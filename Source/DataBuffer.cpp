#include "DataBuffer.h"

AudioParameterFloatListener::AudioParameterFloatListener(const ParameterID& parameterID,const String& parameterName,float minValue,float maxValue,float defaultValue) : AudioParameterFloat(parameterID,parameterName,minValue,maxValue,defaultValue){

}

void AudioParameterFloatListener::valueChanged(float){
	
};

AudioParameterFloatListener& AudioParameterFloatListener::operator= (float newValue){
	AudioParameterFloat::operator=(newValue);
	return *this;
}

AudioParameterFloatListener::~AudioParameterFloatListener(){

}

SliderData::SliderData(){
	
}

SliderData::~SliderData(){
	
}

template<typename SampleType>
SideShaperAudioProcessorInternalData<SampleType>::SideShaperAudioProcessorInternalData(){
	
}

template<typename SampleType>
void SideShaperAudioProcessorInternalData<SampleType>::resetSplitterCutoffFrequencies(bool oneSplit){
	setMainSplitterCutoffFrequency(mainSplitter.currentCutoff);
	setLowSplitterCutoffFrequency(lowSplitter.currentCutoff,oneSplit);
	setHighSplitterCutoffFrequency(highSplitter.currentCutoff);
}

template<typename SampleType>
void SideShaperAudioProcessorInternalData<SampleType>::setMainSplitterCutoffFrequency(SampleType newCutoffFrequencyHz){
	mainSplitter.setCutoffFrequency(newCutoffFrequencyHz);
}

template<typename SampleType>
void SideShaperAudioProcessorInternalData<SampleType>::setLowSplitterCutoffFrequency(SampleType newCutoffFrequencyHz, bool oneSplit){
	lowSplitter.setCutoffFrequency(newCutoffFrequencyHz);
	if(oneSplit){
		setMainSplitterCutoffFrequency(newCutoffFrequencyHz);
	}
}

template<typename SampleType>
void SideShaperAudioProcessorInternalData<SampleType>::setHighSplitterCutoffFrequency(SampleType newCutoffFrequencyHz){
	highSplitter.setCutoffFrequency(newCutoffFrequencyHz);
}

template<typename SampleType>
SideShaperAudioProcessorInternalData<SampleType>::~SideShaperAudioProcessorInternalData(){
	
}

template class SideShaperAudioProcessorInternalData<float>;
template class SideShaperAudioProcessorInternalData<double>;

DataBuffer::DataBuffer(juce::AudioProcessor* ap){
	apvts = std::make_unique<juce::AudioProcessorValueTreeState>(*ap,nullptr,juce::Identifier("PARAMETERS"),createParameterLayout());
	for(int i=0;i<sliderCount;i++){
		sliderData[i] = new SliderData();
	}
}

template<typename SampleType>
void DataBuffer::addSideShaper(juce::String parameterId, juce::String parameterName, bool midMode, SideShaperAudioProcessorInternalData<SampleType>& data, int shaperIndex){
	data.sideShapers[shaperIndex] = SideShaper<SampleType>();
	data.sideShapers[shaperIndex].setMidMode(midMode);
}

template<typename SampleType>
int DataBuffer::addSideShaperAndItsParameters(juce::String parameterId, juce::String parameterName, bool midMode, SideShaperAudioProcessorInternalData<SampleType>& data, juce::AudioProcessorValueTreeState::ParameterLayout& apvtsParams, int paramCount, int shaperIndex) {
	addSideShaper<SampleType>(parameterId, parameterName, midMode, data, shaperIndex);
	//Note that to be AAX compliant, the parameterID must be 31 characters or less.
	std::unique_ptr sd = std::make_unique<AudioParameterFloatListener>(juce::ParameterID{"sd_"+parameterId,paramCount},juce::String("Start Distance "+parameterName), 0.0f, 1.0f, 0.0f);
	*sd = 0.0;
	std::unique_ptr ed = std::make_unique<AudioParameterFloatListener>(juce::ParameterID{"ed_"+parameterId,paramCount+1},juce::String("End Distance "+parameterName), 0.0f, 1.0f, 1.0f);
	*ed = 1.0;
	std::unique_ptr z1p = std::make_unique<AudioParameterFloatListener>(juce::ParameterID{"zop_"+parameterId,paramCount+2},juce::String("Zone 1 Percentage "+parameterName), 0.0f, 1.0f, 0.5f);
	*z1p = 0.5;
	std::unique_ptr ipc = std::make_unique<AudioParameterFloatListener>(juce::ParameterID{"inf_"+parameterId,paramCount+3},juce::String("Inflection Point Compression "+parameterName), 0.0f, 1.0f, 0.5f);
	*ipc = 0.5;
	std::unique_ptr z1s = std::make_unique<AudioParameterFloatListener>(juce::ParameterID{"zos_"+parameterId,paramCount+4},juce::String("Zone 1 Saturation "+parameterName), 0.0f, 1.0f, 0.5f);
	*z1s = 0.5;
	std::unique_ptr z2s = std::make_unique<AudioParameterFloatListener>(juce::ParameterID{"zts_"+parameterId,paramCount+5},juce::String("Zone 2 Saturation "+parameterName), 0.0f, 1.0f, 0.5f);
	*z2s = 0.5;
	sideShaperFloats[shaperIndex] = std::array<AudioParameterFloatListener*,6>{
		sd.get(),
		ed.get(),
		z1p.get(),
		ipc.get(),
		z1s.get(),
		z2s.get()
	};
	apvtsParams.add(std::move(sd));
	apvtsParams.add(std::move(ed));
	apvtsParams.add(std::move(z1p));
	apvtsParams.add(std::move(ipc));
	apvtsParams.add(std::move(z1s));
	apvtsParams.add(std::move(z2s));
	sideShaperFloatsPrevious[shaperIndex] = std::array<float, 6>{
		0.0f,
		1.0f,
		0.5f,
		0.5f,
		0.5f,
		0.5f
	};
	return 6;
}

juce::AudioProcessorValueTreeState::ParameterLayout DataBuffer::createParameterLayout(){
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	int paramCount = 0;
	std::unique_ptr flm = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"freqLowMid",1},juce::String("FreqLowMid"),0.0f,1.0f,0.5f);
	freqLowMid = flm.get();
	layout.add(std::move(flm));
	paramCount++;
	std::unique_ptr fmm = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"freqMidMid",2},juce::String("FreqMidMid"),0.0f,1.0f,0.5f);
	freqMidMid = fmm.get();
	layout.add(std::move(fmm));
	paramCount++;
	std::unique_ptr fmh = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"freqMidHigh",3},juce::String("FreqMidHigh"),0.0f,1.0f,0.5f);
	freqMidHigh = fmh.get();
	layout.add(std::move(fmh));
	paramCount++;
	int shaperIndex = 0;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("gmb"), juce::String("Global Mid Pre"), true, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("gsb"), juce::String("Global Side Pre"), false, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("lm"), juce::String("Low Mid"), true, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("ls"), juce::String("Low Side"), false, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("mlm"), juce::String("Mid Low Mid"), true, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("mls"), juce::String("Mid Low Side"), false, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("mhm"), juce::String("Mid High Mid"), true, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("mhs"), juce::String("Mid High Side"), false, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("hm"), juce::String("High Mid"), true, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("hs"), juce::String("High Side"), false, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("gma"), juce::String("Global Mid Post"), true, fid, layout, paramCount, shaperIndex); shaperIndex++;
	paramCount += addSideShaperAndItsParameters<float>(juce::String("gsa"), juce::String("Global Side Post"), false, fid, layout, paramCount, shaperIndex);
	shaperIndex = 0;
	addSideShaper<double>(juce::String("gmb"), juce::String("Global Mid Pre"), true, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("gsb"), juce::String("Global Side Pre"), false, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("lm"), juce::String("Low Mid"), true, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("ls"), juce::String("Low Side"), false, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("mlm"), juce::String("Mid Low Mid"), true, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("mls"), juce::String("Mid Low Side"), false, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("mhm"), juce::String("Mid High Mid"), true, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("mhs"), juce::String("Mid High Side"), false, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("hm"), juce::String("High Mid"), true, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("hs"), juce::String("High Side"), false, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("gma"), juce::String("Global Mid Post"), true, did, shaperIndex); shaperIndex++;
	addSideShaper<double>(juce::String("gsa"), juce::String("Global Side Post"), false, did, shaperIndex);
	return layout;
}

DataBuffer::~DataBuffer(){
	for(int i=0;i<sliderCount;i++){
		delete sliderData[i];
	}
}