#pragma once
#include <JuceHeader.h>
#include "LinearPhaseCrossover.h"
#include "SideShaper.h"

using namespace juce;

enum TabName : int {
	Pre,
	Low,
	MidLow,
	MidHigh,
	High,
	Post,
	Settings,
	About
};

enum ShaperName : int {
	GlobalMidPre,
	GlobalSidePre,
	LowMid,
	LowSide,
	MidLowMid,
	MidLowSide,
	MidHighMid,
	MidHighSide,
	HighMid,
	HighSide,
	GlobalMidPost,
	GlobalSidePost
};

enum SliderName : int {
	MidSliderMain,
	MidSliderInflection,
	MidSliderZone1,
	MidSliderZone2,
	SideSliderMain,
	SideSliderInflection,
	SideSliderZone1,
	SideSliderZone2,
	UpperFrequencySlider,
	LowerFrequencySlider
};

enum SplitBufferValuesBufferName : int {
	PrePreGlobal,
	PostPreGlobal,
	PreLow,
	PostLow,
	PreMidLow,
	PostMidLow,
	PreMidHigh,
	PostMidHigh,
	PreHigh,
	PostHigh,
	PrePostGlobal,
	PostPostGlobal
};

enum VectorscopeShape : int {
	None,
	Circle,
	Diamond,
	Square
};

class AudioParameterFloatListener : public AudioParameterFloat {
public:
	AudioParameterFloatListener(const ParameterID& parameterID,const String& parameterName,float minValue,float maxValue,float defaultValue);
	void valueChanged (float newValue) override;
	AudioParameterFloatListener& operator= (float);
	~AudioParameterFloatListener();
};

template<typename SampleType>
class SplitBufferValues {
public:
	SampleType prePreGlobal[2] = {0};//One mid value, one side value.
	SampleType postPreGlobal[2] = {0};
	SampleType preLow[2] = {0};
	SampleType postLow[2] = {0};
	SampleType preMidLow[2] = {0};
	SampleType postMidLow[2] = {0};
	SampleType preMidHigh[2] = {0};
	SampleType postMidHigh[2] = {0};
	SampleType preHigh[2] = {0};
	SampleType postHigh[2] = {0};
	SampleType prePostGlobal[2] = {0};
	SampleType postPostGlobal[2] = {0};
	SampleType* operator[](int i) {
		switch (i) {
		case 0:
			return prePreGlobal;
			break;
		case 1:
			return postPreGlobal;
			break;
		case 2:
			return preLow;
			break;
		case 3:
			return postLow;
			break;
		case 4:
			return preMidLow;
			break;
		case 5:
			return postMidLow;
			break;
		case 6:
			return preMidHigh;
			break;
		case 7:
			return postMidHigh;
			break;
		case 8:
			return preHigh;
			break;
		case 9:
			return postHigh;
			break;
		case 10:
			return prePostGlobal;
			break;
		case 11:
			return postPostGlobal;
			break;
		}
		return prePreGlobal;
	}
};

class SliderData {
	public:
		SliderData();
		Atomic<double> atomicMinValue = 0.0;
		Atomic<double> atomicMidValue = 0.5;
		Atomic<double> atomicMaxValue = 1.0;
		Atomic<bool> hasChangeNeedingToBeRecognizedByProcessorMin = false;
		Atomic<bool> hasChangeNeedingToBeRecognizedByProcessorMid = false;
		Atomic<bool> hasChangeNeedingToBeRecognizedByProcessorMax = false;
		Atomic<bool> hasChangeNeedingToBeRecognizedByEditorMin = false;
		Atomic<bool> hasChangeNeedingToBeRecognizedByEditorMid = false;
		Atomic<bool> hasChangeNeedingToBeRecognizedByEditorMax = false;
		double prevMinValue = 0.0;
		double prevMidValue = 0.5;
		double prevMaxValue = 1.0;
		void setBySideShaperParameterName(int sideShaperParameterName, double val){
			switch(sideShaperParameterName){
				case SideShaperParameter::StartDistance:
				case SideShaperParameter::InflectionPointCompression:
				case SideShaperParameter::Zone1Saturation:
				case SideShaperParameter::Zone2Saturation:
					atomicMinValue.set(val);
					break;
				case SideShaperParameter::Zone1Percentage:
					atomicMidValue.set(val);
					break;
				case SideShaperParameter::EndDistance:
					atomicMaxValue.set(val);
					break;
			}
		}
		~SliderData();
};

template<typename SampleType>
class SideShaperAudioProcessorInternalData {
	public: 
		SideShaperAudioProcessorInternalData();
		LinearPhaseCrossover<SampleType> mainSplitter;
		LinearPhaseCrossover<SampleType> lowSplitter;
		LinearPhaseCrossover<SampleType> highSplitter;
		dsp::DelayLine<SampleType, dsp::DelayLineInterpolationTypes::None> highSplitterDelayLine {640};//maximum number of samples the delay line could have
		juce::AudioBuffer<SampleType> lowEndBuffer;
		juce::AudioBuffer<SampleType> highEndBuffer;
		juce::AudioBuffer<SampleType> highEndPostDelayBuffer;
		juce::AudioBuffer<SampleType> lowBuffer;
		juce::AudioBuffer<SampleType> midLowBuffer;
		juce::AudioBuffer<SampleType> midHighBuffer;
		juce::AudioBuffer<SampleType> highBuffer;
		std::array<SideShaper<SampleType>,12> sideShapers;
		void resetSplitterCutoffFrequencies(bool oneSplit);
		void setMainSplitterCutoffFrequency(SampleType newCutoffFrequencyHz);
		void setLowSplitterCutoffFrequency(SampleType newCutoffFrequencyHz, bool oneSplit);
		void setHighSplitterCutoffFrequency(SampleType newCutoffFrequencyHz);
		~SideShaperAudioProcessorInternalData();
};

//This is the same as the Vectorscope files's structure called simply the VectorscopeState, but having an additional data type is just simpler than using a ring buffer.
struct VectorscopeStateAtomized {
	Atomic<float> centerX = 123.0f;
	Atomic<float> centerXOnMouseDown = 123.0f;
	Atomic<float> centerY = 123.0f;
	Atomic<float> centerYOnMouseDown = 123.0f;
	Atomic<bool> probabilityDistributionMode = false;
	Atomic<bool> probabilityDistributionMidOrSide = false;
	Atomic<float> zoomX = 123.0f;
	Atomic<float> zoomY = 123.0f;
	VectorscopeStateAtomized() = default;
};

class DataBuffer {
	public:
		DataBuffer(juce::AudioProcessor* ap);
		template<typename SampleType>
		void addSideShaper(juce::String parameterId, juce::String parameterName, bool midMode, SideShaperAudioProcessorInternalData<SampleType>& data, int shaperIndex);
		template<typename SampleType>
		int addSideShaperAndItsParameters(juce::String parameterId, juce::String parameterName, bool midMode, SideShaperAudioProcessorInternalData<SampleType>& data, juce::AudioProcessorValueTreeState::ParameterLayout& apvtsParams, int paramCount, int shaperIndex);
		juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
		//GUI
		Atomic<int> appWidth = 600;//vectorscopeWidth + middleSectionWidth + vectorscopeWidth = 250 + 100 + 250 = 600; see also PluginEditor.cpp SideShaperAudioProcessorEditor constructor
		Atomic<int> appHeight = 376;//vectorscopeHeight + buttonSectionHeight + tabBarHeight = 250 + 100 + 26 = 376;
		Atomic<int> currentTabIndex = 0;
		std::array<Atomic<int>,6> inOutGridMidSideMemory = {0,0,0,0,0,0};
		Atomic<int> stateChangeTabIndex = 0;
		std::array<VectorscopeStateAtomized,6> leftVscopeStates;
		std::array<VectorscopeStateAtomized,6> rightVscopeStates;
		Atomic<bool> saveStateChangeNeedingToBeRecognizedByEditor = false;
		//Vectorscopes
		AbstractFifo graphicsDataHandler = AbstractFifo(736);//This is the object that atomizes both the f and d GraphicsData arrays.
		SplitBufferValues<double> dGraphicsData[736] = {0};//44,100 samples per second / 60 frames per second
		SplitBufferValues<float> fGraphicsData[736] = {0};//44,100 samples per second / 60 frames per second
		SideShaperAudioProcessorInternalData<double> did;
		SideShaperAudioProcessorInternalData<float> fid;
		Atomic<int> lastUpdatedDataType = 0;//0 - float, 1 - double
		//Mid Grid
		const int sliderCount = 10;//the number of distinct sliders that will appear in the GUI; if you change this, also change the sliderData array size
		const int tabCount = 6;
		std::array<SliderData*,10> sliderData;
		std::array<std::array<Atomic<bool>,2>,6> flatAfterEnd {0};
		std::array<std::array<Atomic<bool>,2>,6> flatAfterEndChangesNeedingToBeRecognizedByProcessor {0};
		std::array<std::array<Atomic<bool>,2>,6> gateBeforeStart {0};
		std::array<std::array<Atomic<bool>,2>,6> gateBeforeStartChangesNeedingToBeRecognizedByProcessor {0};
		std::array<Atomic<bool>,6> solo = {0,0,0,0,0,1};//The bool value for the pre tab represents whether there is no solo action going on.
		//Left Side Grid
		//  Pre
		Atomic<bool> bypassAll = false;
		Atomic<bool> bypassSplitters = false;
		//  Low
		Atomic<bool> oneSplit = false;
		Atomic<bool> oneSplitChangeNeedingToBeRecognizedByProcessor = false;
		//  High
		Atomic<bool> fullFrequencyCrossoverRanges = false;
		Atomic<bool> fullFrequencyCrossoverRangesNeedingToBeRecognizedByProcessor = false;
		//  Post
		//    Settings
		Atomic<bool> leftRightMode = false;
		Atomic<bool> leftRightModeChangeNeedingToBeRecognizedByProcessor = false;
		Atomic<bool> protectedMode = false;
		Atomic<bool> protectedModeChangeNeedingToBeRecognizedByProcessor = false;
		Atomic<bool> shapeRangeSwapMode = false;
		Atomic<bool> shapeRangeSwapModeChangeNeedingToBeRecognizedByProcessor = false;
		Atomic<int> crossoverMode = RolloffSize::FullOctave;
		Atomic<bool> crossoverModeChangeNeedingToBeRecognizedByProcessor = false;
		Atomic<int> inputShape = VectorscopeShape::None;
		Atomic<int> outputShape = VectorscopeShape::None;
		//Side Grid
		//note that 4 of the 10 sliders are for the Side Grid
		//note the Mid Grid 2-element arrays in the flatAfterEnd and gateBeforeStart variables
		std::array<Atomic<bool>,6> soloBeforeOrAfter = {1,1,1,1,1,1};//false - before, true - after
		//Supporting Variables
		const int shaperCount = 12;//6 tabs * 2 for mid/side = 12
		std::array<std::array<float,6>,12> sideShaperFloatsPrevious;
		//These variables are exposed to the DAW.
		juce::AudioParameterFloat* freqLowMid;
		juce::AudioParameterFloat* freqMidMid;
		juce::AudioParameterFloat* freqMidHigh;
		std::array<std::array<AudioParameterFloatListener*,6>,12> sideShaperFloats;
		std::unique_ptr<juce::AudioProcessorValueTreeState> apvts;//This has to be at the end of the variable declarations of this class to work.
		~DataBuffer();
};