#include "LinearPhaseCrossover.h"

template <typename SampleType>
dsp::FIR::SimdFilter<SampleType>::SimdFilter(CoefficientsPtr coefficientsToUse) : coefficients(std::move(coefficientsToUse)){
	reset();
}

template <typename SampleType>
void dsp::FIR::SimdFilter<SampleType>::recognizeCoefficientsChanged(){
	size_t previousOrder = order;
	size_t newOrder = coefficients->getFilterOrder();
	if(newOrder != previousOrder){
		reset();
		pos[0] = 0;
		pos[1] = 0;
	}
	auto* fir = coefficients->getRawCoefficients();
	for(size_t i=0;i<newOrder;i++){
		firAligned[i] = fir[i];
	}
	order = newOrder;
}

template <typename SampleType>
inline void dsp::FIR::SimdFilter<SampleType>::prepare([[maybe_unused]] const ProcessSpec& spec) noexcept{
	reset();
}

template <typename SampleType>
template <typename ProcessContext>
void dsp::FIR::SimdFilter<SampleType>::process (const ProcessContext& context) noexcept{
	static_assert(std::is_same_v<typename ProcessContext::SampleType, SampleType>,"The sample-type of the FIR filter must match the sample-type supplied to this process callback");
	auto&& inputBlock  = context.getInputBlock();
	auto&& outputBlock = context.getOutputBlock();
	auto numSamples = inputBlock.getNumSamples();
	auto numChannels = inputBlock.getNumChannels();
	for(int channel=0;channel<numChannels;channel++){
		auto* src = inputBlock.getChannelPointer(channel);
		auto* dst = outputBlock.getChannelPointer(channel);
		size_t p = pos[channel];
		if(context.isBypassed){
			for (size_t i = 0; i < numSamples; ++i) {
				internalRingBuffer[channel][p] = dst[i] = src[i];
				p = (p == 0 ? order - 1 : p - 1);
			}
		}else{
			for (size_t i = 0; i < numSamples; ++i){
				SampleType out (0);
				internalRingBuffer[channel][p] = src[i];
				size_t k = order - p;
				FloatVectorOperationsBase<SampleType,size_t>::copy(bufAligned[channel].data(),internalRingBuffer[channel].data()+p,k);
				FloatVectorOperationsBase<SampleType,size_t>::copy(bufAligned[channel].data()+k,internalRingBuffer[channel].data(),p);
				FloatVectorOperationsBase<SampleType,size_t>::multiply(bufAligned[channel].data(),firAligned.data(),order);
				size_t l;
				for(l=8;l<order-8;l+=8){
					FloatVectorOperationsBase<SampleType,size_t>::add(bufAligned[channel].data(),bufAligned[channel].data()+l,8);
				}
				for(size_t m=0;l+m<order;m++){
					bufAligned[channel][m] += bufAligned[channel][l+m];
				}
				for(l=0;l<8;l++){
					out += bufAligned[channel][l];
				}
				p = (p == 0 ? order - 1 : p - 1);
				dst[i] = out;
			}
		}
		pos[channel] = p;
	}
}

template <typename SampleType>
void dsp::FIR::SimdFilter<SampleType>::reset(){
	FloatVectorOperationsBase<SampleType,size_t>::clear(internalRingBuffer[0].data(),order);
	FloatVectorOperationsBase<SampleType,size_t>::clear(internalRingBuffer[1].data(),order);
}

template<typename SampleType>
LinearPhaseCrossover<SampleType>::LinearPhaseCrossover(){
	
}

template <typename SampleType>
void LinearPhaseCrossover<SampleType>::prepare(const dsp::ProcessSpec& spec) {
	jassert(spec.sampleRate > 0);
    jassert(spec.numChannels > 0);
	currentSampleRate = spec.sampleRate;
	currentHalfSampleRate = static_cast<double>(currentSampleRate * 0.5);
	lowPassSimd.prepare(spec);
	latencySamples = static_cast<int>(filterOrder / 2);
	delayLine.prepare(spec);
	delayLine.setDelay(static_cast<SampleType>(latencySamples));
}

template <typename SampleType>
void LinearPhaseCrossover<SampleType>::process(const dsp::ProcessContextNonReplacing<SampleType>& lowContext, const dsp::ProcessContextNonReplacing<SampleType>& highContext){
	lowPassSimd.process(lowContext);
	delayLine.process(highContext);
	if(!lowContext.isBypassed){
		dsp::AudioBlock<SampleType>& outLow = lowContext.getOutputBlock();
		dsp::AudioBlock<SampleType>& outHigh = highContext.getOutputBlock();
		outHigh.subtract(outLow);
	}
}

template <typename SampleType>
void LinearPhaseCrossover<SampleType>::setCutoffFrequency(SampleType newCutoffFrequencyHz){
	//If we're using a sample rate of 44,100hz for example, assert that the cutoff frequency must be between 0 and 22,050hz.
	jassert(isPositiveAndBelow(newCutoffFrequencyHz,currentHalfSampleRate));
	currentCutoff = newCutoffFrequencyHz;
	/*The normalisedTransitionWidth, for a low pass filter for example, is defined as the difference between the starting frequency of the stop band and the ending frequency of the pass band divided by the sample rate.
	  (fs - fp) / sr
	Since the sample rate is twice as high as the highest frequency that can be represented by that sample rate, this is why the designFIRLowpassTransitionMethod function asserts that value for the normalisedTransitionWidth parameter must come in between 0.0 and 0.5.
	Here, I define the transition width to be one octave wide.
	One octave above any given frequency is 2 times greater than the given frequency.
	for example
	if the pass band end is 2205 hertz
	and the stop band start is 4410 hertz, which is one octave up
	then the midpoint of the transition band is 3307.5 hertz
	if we call this midpoint the cutoff frequency then
	this means that the pass band end is 1/3 of the cutoff frequency below the cutoff frequency (3307.5 - 2205) / 3307.5 = 0.3333
	  3/3cf - 1/3cf = 2/3cf = fp
	and the stop band start is 1/3 the cutoff frequency above the cutoff frequency (4410 - 3307.5) / 3307.5 = 0.3333
	  3/3cf + 1/3cf = 4/3cf = fs
	so the transition width must be
	  (4/3cf - 2/3cf) / sr
	which can be simplified to
	  2/3cf / sr
	passBandEnd = 1
	transitionBandMultiplier = 2^(1/12)
	transitionBandMidpoint = ((stopBandStart - passBandEnd) / 2) + passBandEnd
	stopBandStart = passBandEnd * transitionBandMultiplier
	fpRatio = passBandEnd / transitionBandMidpoint
	fsRatio = stopBandStart / transitionBandMidpoint
	fpRatio = 1 / (((transitionBandMultiplier - 1) / 2) + 1)
	fsRatio = transitionBandMultiplier / (((transitionBandMultiplier - 1) / 2) + 1)*/
	SampleType rolloffMultiplier;
	switch(rolloffSize.get()){
		case RolloffSize::Semitone:
			rolloffMultiplier = static_cast<SampleType>(0.05774621018668413);//calculated in GNU Octave as ((2^(1/12)) / ((((2^(1/12)) - 1) / 2) + 1)) - (1 / ((((2^(1/12)) - 1) / 2) + 1))
			break;
		case RolloffSize::HalfOctave:
			rolloffMultiplier = static_cast<SampleType>(0.34314575050762);
			break;
		case RolloffSize::FullOctave:
			rolloffMultiplier = static_cast<SampleType>(0.66666666666666666);
			break;
		default:
			rolloffMultiplier = static_cast<SampleType>(0.66666666666666666);
			break;
	}
	SampleType normalisedTransitionWidth = rolloffMultiplier * currentCutoff / static_cast<SampleType>(currentSampleRate);
	SampleType limitedNormalisedTransitionWidth = juce::jlimit(static_cast<SampleType>(0.0001), static_cast<SampleType>(0.1), normalisedTransitionWidth);
	firCoefficients = dsp::FilterDesign<SampleType>::designFIRLowpassTransitionMethod(
		currentCutoff,
		currentSampleRate,
		filterOrder,
		limitedNormalisedTransitionWidth,
		static_cast<SampleType>(1.0)//linear rolloff
	);
	*lowPassSimd.coefficients = *firCoefficients;
	lowPassSimd.recognizeCoefficientsChanged();
	latencySamples = static_cast<int>((int)filterOrder / 2);
	delayLine.setDelay(static_cast<SampleType>(latencySamples));
}

template <typename SampleType>
void LinearPhaseCrossover<SampleType>::reset(){
	lowPassSimd.reset();
	delayLine.reset();
}

template<typename SampleType>
LinearPhaseCrossover<SampleType>::~LinearPhaseCrossover(){
	
}

template class LinearPhaseCrossover<float>;
template class LinearPhaseCrossover<double>;