#pragma once
#include <JuceHeader.h>
#define maximumOrder 640

enum RolloffSize : int {
	Semitone,
	HalfOctave,
	FullOctave
};

namespace juce::dsp::FIR {
    template <typename SampleType>
    class SimdFilter {
        public:
            alignas(32) std::array<std::array<SampleType,maximumOrder>,2> internalRingBuffer{0.0};
            alignas(32) std::array<std::array<SampleType,maximumOrder>,2> bufAligned{0.0};
	        alignas(32) std::array<SampleType,maximumOrder> firAligned{0.0};
            alignas(32) std::array<SampleType,8> accumulator{0.0};//8 here represents the standard 256-bit register width which can hold 8 32-bit floats in one operation
            using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;
            using CoefficientsPtr = typename Coefficients<NumericType>::Ptr;
            typename Coefficients<NumericType>::Ptr coefficients;
            SimdFilter(CoefficientsPtr coefficientsToUse);
			void recognizeCoefficientsChanged();
            inline void prepare ([[maybe_unused]] const ProcessSpec& spec) noexcept;
            template <typename ProcessContext>
            void process (const ProcessContext& context) noexcept;
            void reset();
			size_t order = maximumOrder;//order must be greater than 16
            size_t pos[2] {0};//a cursor that can be values 0 through order - 1
        JUCE_LEAK_DETECTOR(SimdFilter)
    };
}

using namespace juce;

template <typename SampleType>
class LinearPhaseCrossover {
	public:
		LinearPhaseCrossover();
		void prepare (const dsp::ProcessSpec& spec);
		void process(const dsp::ProcessContextNonReplacing<SampleType>& lowContext, const dsp::ProcessContextNonReplacing<SampleType>& highContext);
		void reset();
		void setCutoffFrequency(SampleType newCutoffFrequencyHz);
		~LinearPhaseCrossover();
		ReferenceCountedObjectPtr<dsp::FIR::Coefficients<SampleType>> kaiserCoefficients;
		ReferenceCountedObjectPtr<dsp::FIR::Coefficients<SampleType>> firCoefficients;
		double currentSampleRate = 44100.0;
		double currentHalfSampleRate = 22050.0;
		SampleType currentCutoff = static_cast<SampleType>(1000.0);
        //dsp::ProcessorDuplicator<dsp::FIR::Filter<SampleType>, dsp::FIR::Coefficients<SampleType>> lowPassSlow;
		dsp::FIR::SimdFilter<SampleType> lowPassSimd = dsp::FIR::SimdFilter<SampleType>(std::move(new dsp::FIR::Coefficients<SampleType>()));
		dsp::DelayLine<SampleType, dsp::DelayLineInterpolationTypes::None> delayLine {640};//maximum number of samples the delay line could have
		size_t filterOrder = 128;//must be an even number
		int latencySamples = static_cast<int>((int)filterOrder / 2);
		Atomic<int> rolloffSize = RolloffSize::FullOctave;
};