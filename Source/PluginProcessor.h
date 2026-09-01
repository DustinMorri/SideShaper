#pragma once
#include <JuceHeader.h>
#include "DataBuffer.h"
#include "PluginEditor.h"

class SideShaperAudioProcessor : public juce::AudioProcessor {
	public:
		SideShaperAudioProcessor();
		bool acceptsMidi() const override;
		void changeProgramName (int index, const juce::String& newName) override;
		juce::AudioProcessorEditor* createEditor() override;
		int getCurrentProgram() override;
		const juce::String getName() const override;
		int getNumPrograms() override;
		const juce::String getProgramName(int index) override;
		void getStateInformation(juce::MemoryBlock& destData) override;
		double getTailLengthSeconds() const override;
		bool hasEditor() const override;
		bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
		bool isMidiEffect() const override;
		void prepareToPlay(double sampleRate, int samplesPerBlock) override;
		void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override {
			if(dataBuffer->bypassAll.get()) dataBuffer->bypassAll.set(false);
			doProcess<float>(buffer, midiMessages, dataBuffer->fid, dataBuffer->fGraphicsData);
			dataBuffer->lastUpdatedDataType.set(0);
		}
		void processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages) override {
			if(dataBuffer->bypassAll.get()) dataBuffer->bypassAll.set(false);
			doProcess<double>(buffer, midiMessages, dataBuffer->did, dataBuffer->dGraphicsData);
			dataBuffer->lastUpdatedDataType.set(1);
		}
		void processBlockBypassed(AudioBuffer<float> &buffer, MidiBuffer &midiMessages) override {
			if(!dataBuffer->bypassAll.get()) dataBuffer->bypassAll.set(true);
			doProcess<float>(buffer, midiMessages, dataBuffer->fid, dataBuffer->fGraphicsData);
			dataBuffer->lastUpdatedDataType.set(0);
		}
		void processBlockBypassed(AudioBuffer<double> &buffer, MidiBuffer &midiMessages) override {
			if(!dataBuffer->bypassAll.get()) dataBuffer->bypassAll.set(true);
			doProcess<double>(buffer, midiMessages, dataBuffer->did, dataBuffer->dGraphicsData);
			dataBuffer->lastUpdatedDataType.set(1);
		}
		bool producesMidi() const override;
		void releaseResources() override;
		void reset() override;
		template<typename SampleType>
		void sendVectorscopeData(std::vector<SplitBufferValues<SampleType>>& dest, int bufferName, int numSamples, juce::AudioBuffer<SampleType>& src);
		void setCurrentProgram(int index) override;
		void setStateInformation(const void* data, int sizeInBytes) override;
		int shaperIndexAndParameterIndexToSliderIndexConversion(int sideShaperIndex, int sideShaperParameterIndex);
		int shaperIndexToTabIndexConversion(int sideShaperIndex);
		~SideShaperAudioProcessor() override;
	private:
		template<typename SampleType>
		void doProcess(juce::AudioBuffer<SampleType>& buffer, juce::MidiBuffer& midiMessages, SideShaperAudioProcessorInternalData<SampleType>& data, SplitBufferValues<SampleType>* editorData);
		//juce::AudioProcessorEditor* editor;
		int currentProgram = 0;
		DataBuffer* dataBuffer;
		juce::Component::SafePointer<SideShaperAudioProcessorEditor> editor = nullptr;
		juce::AudioParameterFloat* freqLowMid;//This parameter is owned by the data buffer.
		juce::AudioParameterFloat* freqMidMid;//This parameter is owned by the data buffer.
		juce::AudioParameterFloat* freqMidHigh;//This parameter is owned by the data buffer.
		float freqLowMidPrevious, freqMidMidPrevious, freqMidHighPrevious;
		juce::NormalisableRange<float> oneSplitFrequencyRange = juce::NormalisableRange<float>(2.205f,22050.0f);
		const int oneSplitLatencySamples = 128;//256/2
		const int threeSplitLatencySamples = 320;//256/2 + 384/2
		std::array<std::array<AudioParameterFloatListener*,6>,12>* sideShaperFloats;//This vector is owned by the data buffer.
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SideShaperAudioProcessor)
};
