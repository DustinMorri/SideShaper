#include "PluginProcessor.h"

#if defined(__clang__)
	#define COMPILING_WITH_CLANG 1
#else
	#define COMPILING_WITH_CLANG 0
#endif

/*template<typename SampleType>
void analyzeBuffer(juce::AudioBuffer<SampleType>& buffer){
	const SampleType pt = 2.0;//positive two
	int bufferSampleCount = buffer.getNumSamples();
	const SampleType* leftChannel = buffer.getReadPointer(0);
	const SampleType* rightChannel = buffer.getReadPointer(1);
	for (int sampleNumber = 0; sampleNumber < bufferSampleCount; sampleNumber++) {
		SampleType mid = (leftChannel[sampleNumber] + rightChannel[sampleNumber]) / pt;
		if(mid > 0.8){
			std::cout << mid << std::endl;
		}
	}
}

template<typename SampleType>
void analyzeBufferBlock(juce::dsp::AudioBlock<SampleType>& block){
	const SampleType pt = 2.0;//positive two
	const auto numSamples = block.getNumSamples();
	const SampleType* leftChannel = block.getChannelPointer(0);
	const SampleType* rightChannel = block.getChannelPointer(1);
	for (int sampleNumber = 0; sampleNumber < numSamples; sampleNumber++) {
		SampleType mid = (leftChannel[sampleNumber] + rightChannel[sampleNumber]) / pt;
		if(mid > 0.8){
			std::cout << mid << std::endl;
		}
	}
}*/

template<typename SampleType>
std::array<SampleType,2> mapTriangleToSquareRayTracing(SampleType mid,SampleType  side) {
	if(mid == 0.0 && side == 0.0){
		return {0.0,0.0};
	}
	const SampleType oneOverSqrt2 = (SampleType) 1.0 / juce::MathConstants<SampleType>::sqrt2;
	const SampleType depth = mid + side;
	SampleType boundaryMid, boundarySide;
	if (mid >= side) {
		boundaryMid = oneOverSqrt2;
		boundarySide = oneOverSqrt2 * (side / mid);
	} else {
		boundaryMid = oneOverSqrt2 * (mid / side);
		boundarySide = oneOverSqrt2;
	}
	return {
		boundaryMid * depth,
		boundarySide * depth
	};
}

template<typename SampleType>
std::array<SampleType,2> mapTriangleToCircleRayTracing(SampleType mid,SampleType  side) {
	if(mid == 0.0 && side == 0.0){
		return {0.0,0.0};
	}
	const SampleType R = pow((SampleType)2.0 / juce::MathConstants<SampleType>::pi,(SampleType)0.25);
	const SampleType r = hypot(mid,side);
	const SampleType depth = mid + side;
	return {
		R * depth * (mid / r),
		R * depth * (side / r)
	};
}

template<typename SampleType>
SampleType getBoundaryRadius(int shape,SampleType theta){
	switch(shape){
		case VectorscopeShape::Circle:
			return std::sqrt((SampleType) 2.0 / juce::MathConstants<SampleType>::pi);
			break;
		case VectorscopeShape::Diamond:
			return (SampleType) 1.0 / (std::fabs(std::cos(theta)) + std::fabs(std::sin(theta)));
			break;
		case VectorscopeShape::Square:
			const SampleType oneOverSqrt2 = (SampleType) 1.0 / juce::MathConstants<SampleType>::sqrt2;
			return oneOverSqrt2 / std::max(std::fabs(std::cos(theta)),std::fabs(std::sin(theta)));
			break;
	}
	return (SampleType)0.0;
}

template<typename SampleType>
std::array<SampleType,2> mapShapeToShapeRayTracing(SampleType radius,SampleType theta,int inputVectorscopeShape,int outputVectorscopeShape) {
	if(radius == 0.0 && theta == 0.0){
		return {0.0,0.0};
	}
	SampleType sourceBoundary = getBoundaryRadius(inputVectorscopeShape,theta);
	SampleType depth = radius / sourceBoundary;
	SampleType targetBoundary = getBoundaryRadius(outputVectorscopeShape,theta);
	SampleType targetRadius = depth * targetBoundary;
	return {
		targetRadius,
		theta
	};
}

template<typename SampleType>
std::array<SampleType,2> cartesianToPolar(SampleType x,SampleType  y) {
	return {
		std::hypot(x,y),//outRadius
		std::atan2(y,x)//outTheta
	};
}

template<typename SampleType>
std::array<SampleType,2> polarToCartesian(SampleType radius,SampleType theta) {
	return {
		radius * std::cos(theta),//outX
		radius * std::sin(theta)//outY
	};
}

template<typename SampleType>
std::array<SampleType,2> leftRightToMidSide(SampleType left,SampleType  right) {
	return {
		(left + right) / (SampleType)2.0,//outMid
		(left - right) / (SampleType)2.0,//outSide
	};
}

template<typename SampleType>
std::array<SampleType,2> midSideToLeftRight(SampleType mid,SampleType  side) {
	return {
		mid + side,//outLeft
		mid - side//outRight
	};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
	return new SideShaperAudioProcessor();
}

SideShaperAudioProcessor::SideShaperAudioProcessor() : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
	dataBuffer = new DataBuffer(this);
	oneSplitFrequencyRange.setSkewForCentre(2205.0f);
	freqLowMid = dataBuffer->freqLowMid;
	freqMidMid = dataBuffer->freqMidMid;
	freqMidHigh = dataBuffer->freqMidHigh;
	dataBuffer->fid.setLowSplitterCutoffFrequency(static_cast<float>(0.5) * 330.0f + 70.0f,false);
	dataBuffer->did.setLowSplitterCutoffFrequency(static_cast<double>(0.5) * 330.0f + 70.0f,false);
	dataBuffer->fid.setMainSplitterCutoffFrequency(static_cast<float>(0.5) * 1190.0f + 410.0f);
	dataBuffer->did.setMainSplitterCutoffFrequency(static_cast<double>(0.5) * 1190.0f + 410.0f);
	dataBuffer->fid.setHighSplitterCutoffFrequency(static_cast<float>(0.5) * 7350.0f + 2650.0f);
	dataBuffer->did.setHighSplitterCutoffFrequency(static_cast<double>(0.5) * 7350.0f + 2650.0f);
	*freqLowMid = 0.5;
	*freqMidMid = 0.5;
	*freqMidHigh = 0.5;
	sideShaperFloats = &(dataBuffer->sideShaperFloats);
}

bool SideShaperAudioProcessor::acceptsMidi() const {
	return false;
}

void SideShaperAudioProcessor::changeProgramName(int, const juce::String&) {
	//Do not allow program renaming.
}

juce::AudioProcessorEditor* SideShaperAudioProcessor::createEditor() {
	//editor = new juce::GenericAudioProcessorEditor(*this);
	editor = juce::Component::SafePointer<SideShaperAudioProcessorEditor>(new SideShaperAudioProcessorEditor(this,dataBuffer));
	return editor.getComponent();
}

int SideShaperAudioProcessor::getCurrentProgram() {
	return currentProgram;
}

const juce::String SideShaperAudioProcessor::getName() const {
	return JucePlugin_Name;
}

int SideShaperAudioProcessor::getNumPrograms() {
	return 2;
}

const juce::String SideShaperAudioProcessor::getProgramName(int index) {
	switch (index) {
		case 0:
			return juce::String("host init");
		case 1:
			return juce::String("default");
	}
	return juce::String("unknown");
}

//Called when the user wans to save a preset.
void SideShaperAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
	//Copy the current state of the parameters exposed to the DAW into a ValueTree.
	juce::ValueTree tree = dataBuffer->apvts->copyState();
	//Add the variables that aren't AudioParameters (mostly to record the GUI state).
	tree.setProperty("appVersion",ProjectInfo::versionString,nullptr);
	tree.setProperty("appWidth",dataBuffer->appWidth.get(),nullptr);
	tree.setProperty("appHeight",dataBuffer->appHeight.get(),nullptr);
	tree.setProperty("currentTabIndex",dataBuffer->currentTabIndex.get(),nullptr);
	tree.setProperty("bypassAll",dataBuffer->bypassAll.get(),nullptr);
	tree.setProperty("bypassSplitters",dataBuffer->bypassSplitters.get(),nullptr);
	tree.setProperty("oneSplit",dataBuffer->oneSplit.get(),nullptr);
	tree.setProperty("fullFrequencyCrossoverRanges",dataBuffer->fullFrequencyCrossoverRanges.get(),nullptr);
	tree.setProperty("leftRightMode",dataBuffer->leftRightMode.get(),nullptr);
	tree.setProperty("protectedMode",dataBuffer->protectedMode.get(),nullptr);
	tree.setProperty("crossoverMode",dataBuffer->crossoverMode.get(),nullptr);
	tree.setProperty("shapeRangeSwapMode",dataBuffer->shapeRangeSwapMode.get(),nullptr);
	tree.setProperty("inputShape",dataBuffer->inputShape.get(),nullptr);
	tree.setProperty("outputShape",dataBuffer->outputShape.get(),nullptr);
	for(int i=0;i<dataBuffer->tabCount;i++){
		tree.setProperty("gateBeforeStartMid"+String(i),dataBuffer->gateBeforeStart[i][0].get(),nullptr);
		tree.setProperty("gateBeforeStartSide"+String(i),dataBuffer->gateBeforeStart[i][1].get(),nullptr);
		tree.setProperty("flatAfterEndMid"+String(i),dataBuffer->flatAfterEnd[i][0].get(),nullptr);
		tree.setProperty("flatAfterEndSide"+String(i),dataBuffer->flatAfterEnd[i][1].get(),nullptr);
		tree.setProperty("solo"+String(i),dataBuffer->solo[i].get(),nullptr);
		tree.setProperty("soloBeforeOrAfter"+String(i),dataBuffer->soloBeforeOrAfter[i].get(),nullptr);
		tree.setProperty("leftVscopeStateCenterX"+String(i),dataBuffer->leftVscopeStates[i].centerX.get(),nullptr);
		tree.setProperty("leftVscopeStateCenterXOnMouseDown"+String(i),dataBuffer->leftVscopeStates[i].centerXOnMouseDown.get(),nullptr);
		tree.setProperty("leftVscopeStateCenterY"+String(i),dataBuffer->leftVscopeStates[i].centerY.get(),nullptr);
		tree.setProperty("leftVscopeStateCenterYOnMouseDown"+String(i),dataBuffer->leftVscopeStates[i].centerYOnMouseDown.get(),nullptr);
		tree.setProperty("leftVscopeStateProbabilityDistributionMode"+String(i),dataBuffer->leftVscopeStates[i].probabilityDistributionMode.get(),nullptr);
		tree.setProperty("leftVscopeStateProbabilityDistributionMidOrSide"+String(i),dataBuffer->leftVscopeStates[i].probabilityDistributionMidOrSide.get(),nullptr);
		tree.setProperty("leftVscopeStateZoomX"+String(i),dataBuffer->leftVscopeStates[i].zoomX.get(),nullptr);
		tree.setProperty("leftVscopeStateZoomY"+String(i),dataBuffer->leftVscopeStates[i].zoomY.get(),nullptr);
		tree.setProperty("rightVscopeStateCenterX"+String(i),dataBuffer->rightVscopeStates[i].centerX.get(),nullptr);
		tree.setProperty("rightVscopeStateCenterXOnMouseDown"+String(i),dataBuffer->rightVscopeStates[i].centerXOnMouseDown.get(),nullptr);
		tree.setProperty("rightVscopeStateCenterY"+String(i),dataBuffer->rightVscopeStates[i].centerY.get(),nullptr);
		tree.setProperty("rightVscopeStateCenterYOnMouseDown"+String(i),dataBuffer->rightVscopeStates[i].centerYOnMouseDown.get(),nullptr);
		tree.setProperty("rightVscopeStateProbabilityDistributionMode"+String(i),dataBuffer->rightVscopeStates[i].probabilityDistributionMode.get(),nullptr);
		tree.setProperty("rightVscopeStateProbabilityDistributionMidOrSide"+String(i),dataBuffer->rightVscopeStates[i].probabilityDistributionMidOrSide.get(),nullptr);
		tree.setProperty("rightVscopeStateZoomX"+String(i),dataBuffer->rightVscopeStates[i].zoomX.get(),nullptr);
		tree.setProperty("rightVscopeStateZoomY"+String(i),dataBuffer->rightVscopeStates[i].zoomY.get(),nullptr);
		tree.setProperty("inOutGridMidSideMemory"+String(i),dataBuffer->inOutGridMidSideMemory[i].get(),nullptr);
	}
	//Convert the ValueTree into an XML Element
	std::unique_ptr<juce::XmlElement> xml (tree.createXml());
	//Serialize the XML element into the destination MemoryBlock
	if (xml != nullptr) {
		copyXmlToBinary (*xml, destData);
	}
}

double SideShaperAudioProcessor::getTailLengthSeconds() const {
	return 0.0;
}

bool SideShaperAudioProcessor::hasEditor() const {
	return true;
}

bool SideShaperAudioProcessor::isBusesLayoutSupported(const BusesLayout&) const {
	return true;
}

bool SideShaperAudioProcessor::isMidiEffect() const {
	return false;
}

void SideShaperAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
	//Let the splitters know what the sample rate is, what the maximum number of samples they can expect per block to be, and the number of output channels they should have.
	juce::dsp::ProcessSpec spec = {
		sampleRate,
		juce::uint32(samplesPerBlock),
		juce::uint32(getTotalNumOutputChannels())
	};
	dataBuffer->fid.mainSplitter.filterOrder = 256; dataBuffer->did.mainSplitter.filterOrder = 256;
	dataBuffer->fid.mainSplitter.prepare(spec); dataBuffer->did.mainSplitter.prepare(spec);
	dataBuffer->fid.lowSplitter.filterOrder = 384; dataBuffer->did.lowSplitter.filterOrder = 384;
	dataBuffer->fid.lowSplitter.prepare(spec); dataBuffer->did.lowSplitter.prepare(spec);
	dataBuffer->fid.highSplitter.filterOrder = 64; dataBuffer->did.highSplitter.filterOrder = 64;
	dataBuffer->fid.highSplitter.prepare(spec); dataBuffer->did.highSplitter.prepare(spec);
	dataBuffer->fid.highSplitterDelayLine.prepare(spec); dataBuffer->did.highSplitterDelayLine.prepare(spec);
	dataBuffer->fid.highSplitterDelayLine.setDelay(static_cast<float>(160)); dataBuffer->did.highSplitterDelayLine.setDelay(static_cast<double>(160));//(384 - 64) / 2
	setLatencySamples(threeSplitLatencySamples);
	//Set the size of the buffers the splitters will be writing values to.
	dataBuffer->fid.lowEndBuffer.setSize(spec.numChannels, samplesPerBlock); dataBuffer->did.lowEndBuffer.setSize(spec.numChannels, samplesPerBlock);
	dataBuffer->fid.highEndBuffer.setSize(spec.numChannels, samplesPerBlock); dataBuffer->did.highEndBuffer.setSize(spec.numChannels, samplesPerBlock);
	dataBuffer->fid.highEndPostDelayBuffer.setSize(spec.numChannels, samplesPerBlock); dataBuffer->did.highEndPostDelayBuffer.setSize(spec.numChannels, samplesPerBlock);
	dataBuffer->fid.lowBuffer.setSize(spec.numChannels, samplesPerBlock); dataBuffer->did.lowBuffer.setSize(spec.numChannels, samplesPerBlock);
	dataBuffer->fid.midLowBuffer.setSize(spec.numChannels, samplesPerBlock); dataBuffer->did.midLowBuffer.setSize(spec.numChannels, samplesPerBlock);
	dataBuffer->fid.midHighBuffer.setSize(spec.numChannels, samplesPerBlock); dataBuffer->did.midHighBuffer.setSize(spec.numChannels, samplesPerBlock);
	dataBuffer->fid.highBuffer.setSize(spec.numChannels, samplesPerBlock); dataBuffer->did.highBuffer.setSize(spec.numChannels, samplesPerBlock);

	for (size_t i = 0; i < dataBuffer->fid.sideShapers.size(); i++) {
		dataBuffer->fid.sideShapers[i].prepare(spec);
	}
	for (size_t i = 0; i < dataBuffer->did.sideShapers.size(); i++) {
		dataBuffer->did.sideShapers[i].prepare(spec);
	}

	//Initialize this class' buffers with values of zero.
	reset();
}

template<typename SampleType>
void SideShaperAudioProcessor::doProcess(juce::AudioBuffer<SampleType>& buffer, juce::MidiBuffer&, SideShaperAudioProcessorInternalData<SampleType>& data, SplitBufferValues<SampleType>* editorData) {
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();
	auto bufferChannelCount = buffer.getNumChannels();
	auto bufferSampleCount = buffer.getNumSamples();

	//Clear any output channels that don't have input data.
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
		buffer.clear(i, 0, bufferSampleCount);
	}

	//If we're going to do an internal bypass, we still need to run the normal process so the visuals can still have their what-if.
	//Make a copy of the input buffer if you need to.
	juce::AudioBuffer<SampleType> bufferCopy;
	bool bypassAll = dataBuffer->bypassAll.get();
	if(bypassAll){
		bufferCopy.makeCopyOf(buffer);
	}

	//Get the current tab.
	//Only do this once per call to the doProcess function so that everything in this call is on the same page.
	int currentTab = dataBuffer->currentTabIndex.get();
	//If the user is looking at the settings or about pages, just pretend like they're still looking at the post tab.
	if(currentTab == TabName::Settings || currentTab == TabName:: About){
		currentTab = TabName::Post;
	}

	//Note that the host (aka DAW) can update any of the exposed AudioProcessorParameters at any time, so we have to check for changes on them constantly.
	//Also note that DAW parameters must allow for all parameters to always be able to use the full 0.0 to 1.0 range.
	//That is okay, but we don't want to bring in illegal values from the AudioParameterFloatListeners (aka dataBuffer.sideShaperFloats) into the SideShaperAudioProcessorInternalData (aka dataBuffer.fid).
	//We also want to display those changes the DAW is making in the GUI. So potentially all 10 of the sliders could be moving at the same time.
	//For each side shaper
	for (int sideShaperName=0;sideShaperName<data.sideShapers.size();sideShaperName++) {
		//Determine if there has been a change in the underlying values of the AudioParameterFloatListeners.
		SampleType newStartDistance = *(*sideShaperFloats)[sideShaperName][SideShaperParameter::StartDistance];
		SampleType oldStartDistance = dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::StartDistance];
		SampleType newEndDistance = *(*sideShaperFloats)[sideShaperName][SideShaperParameter::EndDistance];
		SampleType oldEndDistance = dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::EndDistance];
		SampleType newZone1Percentage = *(*sideShaperFloats)[sideShaperName][SideShaperParameter::Zone1Percentage];
		SampleType oldZone1Percentage = dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::Zone1Percentage];
		SampleType newInternalStartDistance = newStartDistance;
		SampleType newInternalEndDistance = newEndDistance;
		SampleType newInternalZone1Percentage = newZone1Percentage;
		//If the DAW is in an invalid state, we just want to reassert that the values are whatever the sliders say they are.
		if(newStartDistance > newEndDistance){
			newInternalStartDistance = data.sideShapers[sideShaperName].getParameter(SideShaperParameter::StartDistance);
			newInternalEndDistance = data.sideShapers[sideShaperName].getParameter(SideShaperParameter::EndDistance);
			newInternalZone1Percentage = data.sideShapers[sideShaperName].getParameter(SideShaperParameter::Zone1Percentage);
		}
		//If the DAW wants to push around the zone 1 percentage knob just like the GUI can, then we will let it.
		if(newStartDistance > newZone1Percentage){
			newInternalZone1Percentage = newStartDistance;
			(*sideShaperFloats)[sideShaperName][SideShaperParameter::Zone1Percentage]->setValueNotifyingHost((float)newInternalZone1Percentage);
		}
		if(newEndDistance < newZone1Percentage){
			newInternalZone1Percentage = newEndDistance;
			(*sideShaperFloats)[sideShaperName][SideShaperParameter::Zone1Percentage]->setValueNotifyingHost((float)newInternalZone1Percentage);
		}
		SampleType newInflectionPointCompression = *(*sideShaperFloats)[sideShaperName][SideShaperParameter::InflectionPointCompression];
		SampleType oldInflectionPointCompression = dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::InflectionPointCompression];
		SampleType newZone1Saturation = *(*sideShaperFloats)[sideShaperName][SideShaperParameter::Zone1Saturation];
		SampleType oldZone1Saturation = dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::Zone1Saturation];
		SampleType newZone2Saturation = *(*sideShaperFloats)[sideShaperName][SideShaperParameter::Zone2Saturation];
		SampleType oldZone2Saturation = dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::Zone2Saturation];

		bool shaperIsBeingPresentedInGUI = shaperIndexToTabIndexConversion(sideShaperName) == currentTab;

		//If there has been a change of value,
		if(newStartDistance != oldStartDistance){
			//remember that value.
			dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::StartDistance] = (float)newStartDistance;
			//Set the SideShaperAudioProcessorInternalData based on the internal value we decided.
			data.sideShapers[sideShaperName].setParameter(SideShaperParameter::StartDistance,newInternalStartDistance);
			//If the shaper is being presented in the GUI and the sliders are going to agree that a change needs to be displayed,
			if(shaperIsBeingPresentedInGUI && newStartDistance == newInternalStartDistance){
				//tell the editor that it has a change that needs to be displayed.
				SliderData* sd = dataBuffer->sliderData[shaperIndexAndParameterIndexToSliderIndexConversion(sideShaperName,SideShaperParameter::StartDistance)];
				sd->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
				sd->setBySideShaperParameterName(SideShaperParameter::StartDistance,(double)newInternalStartDistance);
			}
		}
		if(newEndDistance != oldEndDistance){
			dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::EndDistance] = (float)newEndDistance;
			data.sideShapers[sideShaperName].setParameter(SideShaperParameter::EndDistance,newInternalEndDistance);
			if(shaperIsBeingPresentedInGUI && newEndDistance == newInternalEndDistance){
				SliderData* sd = dataBuffer->sliderData[shaperIndexAndParameterIndexToSliderIndexConversion(sideShaperName,SideShaperParameter::EndDistance)];
				sd->hasChangeNeedingToBeRecognizedByEditorMax.set(true);
				sd->setBySideShaperParameterName(SideShaperParameter::EndDistance,(double)newInternalEndDistance);
			}
		}
		if(newZone1Percentage != oldZone1Percentage){
			dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::Zone1Percentage] = (float)newZone1Percentage;
			data.sideShapers[sideShaperName].setParameter(SideShaperParameter::Zone1Percentage,newInternalZone1Percentage);
			if(shaperIsBeingPresentedInGUI && newZone1Percentage == newInternalZone1Percentage){
				SliderData* sd = dataBuffer->sliderData[shaperIndexAndParameterIndexToSliderIndexConversion(sideShaperName,SideShaperParameter::Zone1Percentage)];
				sd->hasChangeNeedingToBeRecognizedByEditorMid.set(true);
				sd->setBySideShaperParameterName(SideShaperParameter::Zone1Percentage,(double)newInternalZone1Percentage);
			}
		}
		if(newInflectionPointCompression != oldInflectionPointCompression){
			dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::InflectionPointCompression] = (float)newInflectionPointCompression;
			data.sideShapers[sideShaperName].setParameter(SideShaperParameter::InflectionPointCompression,newInflectionPointCompression);
			if(shaperIsBeingPresentedInGUI){
				SliderData* sd = dataBuffer->sliderData[shaperIndexAndParameterIndexToSliderIndexConversion(sideShaperName,SideShaperParameter::InflectionPointCompression)];
				sd->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
				sd->setBySideShaperParameterName(SideShaperParameter::InflectionPointCompression,(double)newInflectionPointCompression);
			}
		}
		if(newZone1Saturation != oldZone1Saturation){
			dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::Zone1Saturation] = (float)newZone1Saturation;
			data.sideShapers[sideShaperName].setParameter(SideShaperParameter::Zone1Saturation,newZone1Saturation);
			if(shaperIsBeingPresentedInGUI){
				SliderData* sd = dataBuffer->sliderData[shaperIndexAndParameterIndexToSliderIndexConversion(sideShaperName,SideShaperParameter::Zone1Saturation)];
				sd->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
				sd->setBySideShaperParameterName(SideShaperParameter::Zone1Saturation,(double)newZone1Saturation);
			}
		}
		if(newZone2Saturation != oldZone2Saturation){
			dataBuffer->sideShaperFloatsPrevious[sideShaperName][SideShaperParameter::Zone2Saturation] = (float)newZone2Saturation;
			data.sideShapers[sideShaperName].setParameter(SideShaperParameter::Zone2Saturation,newZone2Saturation);
			if(shaperIsBeingPresentedInGUI){
				SliderData* sd = dataBuffer->sliderData[shaperIndexAndParameterIndexToSliderIndexConversion(sideShaperName,SideShaperParameter::Zone2Saturation)];
				sd->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
				sd->setBySideShaperParameterName(SideShaperParameter::Zone2Saturation,(double)newZone2Saturation);
			}
		}
	}

	//Acquire pointers to the slider data.
	SliderData* upperFrequencySliderData = dataBuffer->sliderData[SliderName::UpperFrequencySlider];
	SliderData* lowerFrequencySliderData = dataBuffer->sliderData[SliderName::LowerFrequencySlider];
	SliderData* midSliderInflectionSliderData = dataBuffer->sliderData[SliderName::MidSliderInflection];
	SliderData* midSliderMainSliderData = dataBuffer->sliderData[SliderName::MidSliderMain];
	SliderData* midSliderZone1SliderData = dataBuffer->sliderData[SliderName::MidSliderZone1];
	SliderData* midSliderZone2SliderData = dataBuffer->sliderData[SliderName::MidSliderZone2];
	SliderData* sideSliderInflectionSliderData = dataBuffer->sliderData[SliderName::SideSliderInflection];
	SliderData* sideSliderMainSliderData = dataBuffer->sliderData[SliderName::SideSliderMain];
	SliderData* sideSliderZone1SliderData = dataBuffer->sliderData[SliderName::SideSliderZone1];
	SliderData* sideSliderZone2SliderData = dataBuffer->sliderData[SliderName::SideSliderZone2];

	//Update the plugin if the parameter values have changed.
	//Note that this comes before we ask the editor if there are new values to be recognized because the user clicking and dragging the GUI takes precedence over any DAW automation.
	//Note that asking the editor for a new value calls setValueNotifyingHost which ends up running this section.
	bool oneSplit = dataBuffer->oneSplit.get();
	if(dataBuffer->crossoverModeChangeNeedingToBeRecognizedByProcessor.get()){
		int newCrossoverMode = dataBuffer->crossoverMode.get();
		data.mainSplitter.rolloffSize.set(newCrossoverMode);
		data.lowSplitter.rolloffSize.set(newCrossoverMode);
		data.highSplitter.rolloffSize.set(newCrossoverMode);
		data.resetSplitterCutoffFrequencies(oneSplit);
		dataBuffer->crossoverModeChangeNeedingToBeRecognizedByProcessor.set(false);
	}
	bool fullFrequencyCrossoverRanges = dataBuffer->fullFrequencyCrossoverRanges.get();
	if (*freqLowMid != freqLowMidPrevious) {
		freqLowMidPrevious = *freqLowMid;
		if(!oneSplit){
			if(!fullFrequencyCrossoverRanges){
				data.setLowSplitterCutoffFrequency(freqLowMidPrevious * 330.0f + 70.0f,oneSplit);//from 70 to 400
			}else{
				data.setLowSplitterCutoffFrequency(freqLowMidPrevious * 399.0f + 1.0f,oneSplit);//from 1 to 400
			}
		}else{
			data.setLowSplitterCutoffFrequency(oneSplitFrequencyRange.convertFrom0to1(freqLowMidPrevious),oneSplit);//from 2.205 to 22050.0 with a skew towards 2205.0
		}
		if(currentTab == TabName::Low){
			lowerFrequencySliderData->atomicMinValue.set((double)freqLowMidPrevious);
			lowerFrequencySliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
		}else if(currentTab == TabName::MidLow){
			upperFrequencySliderData->atomicMinValue.set((double)freqLowMidPrevious);
			upperFrequencySliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
		}else if(oneSplit && currentTab == TabName::High){
			upperFrequencySliderData->atomicMinValue.set((double)freqLowMidPrevious);
			upperFrequencySliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
		}
	}
	if (*freqMidMid != freqMidMidPrevious) {
		freqMidMidPrevious = *freqMidMid;
		if(!fullFrequencyCrossoverRanges){
			data.setMainSplitterCutoffFrequency(freqMidMidPrevious * 1190.0f + 410.0f);//from 410 to 1600
		}else{
			data.setMainSplitterCutoffFrequency(freqMidMidPrevious * 6950.0f + 400.0f);//from 400 to 7350
		}
		if(currentTab == TabName::MidLow){
			lowerFrequencySliderData->atomicMinValue.set((double)freqMidMidPrevious);
			lowerFrequencySliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
		}else if(currentTab == TabName::MidHigh){
			upperFrequencySliderData->atomicMinValue.set((double)freqMidMidPrevious);
			upperFrequencySliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
		}
	}
	if (*freqMidHigh != freqMidHighPrevious) {
		freqMidHighPrevious = *freqMidHigh;
		if(!fullFrequencyCrossoverRanges){
			data.setHighSplitterCutoffFrequency(freqMidHighPrevious * 7350.0f + 2650.0f);//from 2650 to 7350
		}else{
			data.setHighSplitterCutoffFrequency(freqMidHighPrevious * 14600.0f + 7350.0f);//from 7350 to 21950 (not 22050 because that glitches out the buffer)
		}
		if(currentTab == TabName::MidHigh){
			lowerFrequencySliderData->atomicMinValue.set((double)freqMidHighPrevious);
			lowerFrequencySliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
		}else if(currentTab == TabName::High){
			upperFrequencySliderData->atomicMinValue.set((double)freqMidHighPrevious);
			upperFrequencySliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(true);
		}
	}

	if(midSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get()){
		(*sideShaperFloats)[currentTab*2][SideShaperParameter::StartDistance]->setValueNotifyingHost((float)midSliderMainSliderData->atomicMinValue.get());
		midSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
	}
	if(midSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMid.get()){
		(*sideShaperFloats)[currentTab*2][SideShaperParameter::Zone1Percentage]->setValueNotifyingHost((float)midSliderMainSliderData->atomicMidValue.get());
		midSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMid.set(false);
	}
	if(midSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMax.get()){
		(*sideShaperFloats)[currentTab*2][SideShaperParameter::EndDistance]->setValueNotifyingHost((float)midSliderMainSliderData->atomicMaxValue.get());
		midSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMax.set(false);
	}
	if(midSliderInflectionSliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get()){
		(*sideShaperFloats)[currentTab*2][SideShaperParameter::InflectionPointCompression]->setValueNotifyingHost((float)midSliderInflectionSliderData->atomicMinValue.get());
		midSliderInflectionSliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
	}
	if(midSliderZone1SliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get()){
		(*sideShaperFloats)[currentTab*2][SideShaperParameter::Zone1Saturation]->setValueNotifyingHost((float)midSliderZone1SliderData->atomicMinValue.get());
		midSliderZone1SliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
	}
	if(midSliderZone2SliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get()){
		(*sideShaperFloats)[currentTab*2][SideShaperParameter::Zone2Saturation]->setValueNotifyingHost((float)midSliderZone2SliderData->atomicMinValue.get());
		midSliderZone2SliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
	}

	if(sideSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get()){
		(*sideShaperFloats)[currentTab*2+1][SideShaperParameter::StartDistance]->setValueNotifyingHost((float)sideSliderMainSliderData->atomicMinValue.get());
		sideSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
	}
	if(sideSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMid.get()){
		(*sideShaperFloats)[currentTab*2+1][SideShaperParameter::Zone1Percentage]->setValueNotifyingHost((float)sideSliderMainSliderData->atomicMidValue.get());
		sideSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMid.set(false);
	}
	if(sideSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMax.get()){
		(*sideShaperFloats)[currentTab*2+1][SideShaperParameter::EndDistance]->setValueNotifyingHost((float)sideSliderMainSliderData->atomicMaxValue.get());
		sideSliderMainSliderData->hasChangeNeedingToBeRecognizedByProcessorMax.set(false);
	}
	if(sideSliderInflectionSliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get()){
		(*sideShaperFloats)[currentTab*2+1][SideShaperParameter::InflectionPointCompression]->setValueNotifyingHost((float)sideSliderInflectionSliderData->atomicMinValue.get());
		sideSliderInflectionSliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
	}
	if(sideSliderZone1SliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get()){
		(*sideShaperFloats)[currentTab*2+1][SideShaperParameter::Zone1Saturation]->setValueNotifyingHost((float)sideSliderZone1SliderData->atomicMinValue.get());
		sideSliderZone1SliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
	}
	if(sideSliderZone2SliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get()){
		(*sideShaperFloats)[currentTab*2+1][SideShaperParameter::Zone2Saturation]->setValueNotifyingHost((float)sideSliderZone2SliderData->atomicMinValue.get());
		sideSliderZone2SliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
	}

	bool upperHasChange = upperFrequencySliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get();
	bool lowerHasChange = lowerFrequencySliderData->hasChangeNeedingToBeRecognizedByProcessorMin.get();
	if(currentTab == TabName::Low){
		if(lowerHasChange == 1){
			freqLowMid->setValueNotifyingHost((float)lowerFrequencySliderData->atomicMinValue.get());
			lowerFrequencySliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
		}
	}else if(currentTab == TabName::MidLow){
		if(upperHasChange == 1){
			freqLowMid->setValueNotifyingHost((float)upperFrequencySliderData->atomicMinValue.get());
			upperFrequencySliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
		}
		if(lowerHasChange == 1){
			freqMidMid->setValueNotifyingHost((float)lowerFrequencySliderData->atomicMinValue.get());
			lowerFrequencySliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
		}
	}else if(currentTab == TabName::MidHigh){
		if(upperHasChange == 1){
			freqMidMid->setValueNotifyingHost((float)upperFrequencySliderData->atomicMinValue.get());
			upperFrequencySliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
		}
		if(lowerHasChange == 1){
			freqMidHigh->setValueNotifyingHost((float)lowerFrequencySliderData->atomicMinValue.get());
			lowerFrequencySliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
		}
	}else if(currentTab == TabName::High){
		if(upperHasChange == 1){
			if(!oneSplit){
				freqMidHigh->setValueNotifyingHost((float)upperFrequencySliderData->atomicMinValue.get());
				upperFrequencySliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
			}else{
				freqLowMid->setValueNotifyingHost((float)upperFrequencySliderData->atomicMinValue.get());
				upperFrequencySliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(false);
			}
		}
	}
	if(dataBuffer->fullFrequencyCrossoverRangesNeedingToBeRecognizedByProcessor.get()){
		if(!fullFrequencyCrossoverRanges){
			data.setLowSplitterCutoffFrequency(freqLowMidPrevious * 330.0f + 70.0f,oneSplit);//from 70 to 400
			data.setMainSplitterCutoffFrequency(freqMidMidPrevious * 1190.0f + 410.0f);//from 410 to 1600
			data.setHighSplitterCutoffFrequency(freqMidHighPrevious * 7350.0f + 2650.0f);//from 2650 to 7350
		}else{
			data.setLowSplitterCutoffFrequency(freqLowMidPrevious * 399.0f + 1.0f,oneSplit);//from 1 to 400
			data.setMainSplitterCutoffFrequency(freqMidMidPrevious * 6950.0f + 400.0f);//from 400 to 7350
			data.setHighSplitterCutoffFrequency(freqMidHighPrevious * 14600.0f + 7350.0f);//from 7350 to 21950 (not 22050 because that glitches out the buffer)
		}
		dataBuffer->fullFrequencyCrossoverRangesNeedingToBeRecognizedByProcessor.set(false);
	}
	if(dataBuffer->oneSplitChangeNeedingToBeRecognizedByProcessor.get()){
		float freqLowMidValue = *freqLowMid;
		if(oneSplit){
			data.setLowSplitterCutoffFrequency(oneSplitFrequencyRange.convertFrom0to1(freqLowMidValue),oneSplit);//from 2.205 to 22050.0 with a skew towards 2205.0
			setLatencySamples(oneSplitLatencySamples);
		}else{
			if(!fullFrequencyCrossoverRanges){
				data.setLowSplitterCutoffFrequency(freqLowMidValue * 330.0f + 70.0f,oneSplit);//from 70 to 400
			}else{
				data.setLowSplitterCutoffFrequency(freqLowMidValue * 399.0f + 1.0f,oneSplit);//from 1 to 400
			}
			setLatencySamples(threeSplitLatencySamples);
		}
		dataBuffer->oneSplitChangeNeedingToBeRecognizedByProcessor.set(false);
	}
	if(dataBuffer->leftRightModeChangeNeedingToBeRecognizedByProcessor.get()){
		bool newLeftRightModeValue = dataBuffer->leftRightMode.get();
		for(int i=0;i<6;i++){
			data.sideShapers[i*2].setLeftRightMode(newLeftRightModeValue);
			data.sideShapers[i*2+1].setLeftRightMode(newLeftRightModeValue);
		}
		dataBuffer->leftRightModeChangeNeedingToBeRecognizedByProcessor.set(false);
	}
	if(dataBuffer->protectedModeChangeNeedingToBeRecognizedByProcessor.get()){
		bool newProtectedModeValue = dataBuffer->protectedMode.get();
		for(int i=0;i<6;i++){
			data.sideShapers[i*2].setProtectedMode(newProtectedModeValue);
			data.sideShapers[i*2+1].setProtectedMode(newProtectedModeValue);
		}
		dataBuffer->protectedModeChangeNeedingToBeRecognizedByProcessor.set(false);
	}
	if(dataBuffer->shapeRangeSwapModeChangeNeedingToBeRecognizedByProcessor.get()){
		bool newshapeRangeSwapModeValue = dataBuffer->shapeRangeSwapMode.get();
		for(int i=0;i<6;i++){
			data.sideShapers[i*2].setShapeRangeSwap(newshapeRangeSwapModeValue);
			data.sideShapers[i*2+1].setShapeRangeSwap(newshapeRangeSwapModeValue);
		}
		dataBuffer->protectedModeChangeNeedingToBeRecognizedByProcessor.set(false);
	}
	for(int midSide = 0; midSide < 2; midSide++){
		if(dataBuffer->flatAfterEndChangesNeedingToBeRecognizedByProcessor[currentTab][midSide].get()){
			data.sideShapers[currentTab*2+midSide].setFlatAfterEnd(dataBuffer->flatAfterEnd[currentTab][midSide].get());
			dataBuffer->flatAfterEndChangesNeedingToBeRecognizedByProcessor[currentTab][midSide].set(false);
		}
		if(dataBuffer->gateBeforeStartChangesNeedingToBeRecognizedByProcessor[currentTab][midSide].get()){
			data.sideShapers[currentTab*2+midSide].setGateBeforeStart(dataBuffer->gateBeforeStart[currentTab][midSide].get());
			dataBuffer->gateBeforeStartChangesNeedingToBeRecognizedByProcessor[currentTab][midSide].set(false);
		}
	}

	//Now that we have finished recognizing parameter changes, we can move on to processing the audio.
	juce::dsp::AudioBlock<SampleType> bufferBlock(buffer.getArrayOfWritePointers(),bufferChannelCount,bufferSampleCount);
	juce::AudioBuffer<SampleType> preBuffer;
	preBuffer.makeCopyOf(buffer);
	juce::dsp::AudioBlock<SampleType> preBufferBlock(preBuffer.getArrayOfWritePointers(),bufferChannelCount,bufferSampleCount);

	//Calculate the pre-processed mid and side values that we will send to the editor for the left vectorscope on the Pre tab.
	std::vector<SplitBufferValues<SampleType>> midSideToEditor(bufferSampleCount);

	if(currentTab == TabName::Pre){
		sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PrePreGlobal, bufferSampleCount, buffer);
	}

	//Note that a false value for the soloBeforeOrAfter variable means that the user wants to hear the buffer before the effect of the processing.
	if(dataBuffer->soloBeforeOrAfter[TabName::Pre].get()){
		//This is here mostly so that the right vectorscope can see what the result of processing just the pre shaper looks like.
		juce::dsp::ProcessContextReplacing<SampleType> globalPreBufferBlockPCR(preBufferBlock);
		#if !COMPILING_WITH_CLANG
		data.sideShapers[ShaperName::GlobalMidPre].process<juce::dsp::ProcessContextReplacing<SampleType>>(globalPreBufferBlockPCR);
		data.sideShapers[ShaperName::GlobalSidePre].process<juce::dsp::ProcessContextReplacing<SampleType>>(globalPreBufferBlockPCR);
		#else
		data.sideShapers[ShaperName::GlobalMidPre].template
			process<juce::dsp::ProcessContextReplacing<SampleType>>(globalPreBufferBlockPCR);
		data.sideShapers[ShaperName::GlobalSidePre].template
			process<juce::dsp::ProcessContextReplacing<SampleType>>(globalPreBufferBlockPCR);
		#endif
	}

	//Now that we have processed the effect of the Pre tab, we can calculate the right vectorscope of the Pre tab.
	if(currentTab == TabName::Pre){
		sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PostPreGlobal, bufferSampleCount, preBuffer);
	}

	bool bypassSplitters = dataBuffer->bypassSplitters.get();
	if(!bypassSplitters){
		//Set all of the buffer values to zero with every call of this function.
		//When the DAW playback stops, it might not call the releaseResources function because reverb-type processors are allowed to continue making noise.
		//The number of samples can change with every call of this function and we don't want values from previous blocks to interact with this block.
		data.lowEndBuffer.clear();
		data.highEndBuffer.clear();
		data.highEndPostDelayBuffer.clear();
		data.lowBuffer.clear();
		data.midLowBuffer.clear();
		data.midHighBuffer.clear();
		data.highBuffer.clear();

		//Create containers that have pointers to the buffers.
		juce::dsp::AudioBlock<SampleType> lowEndBufferBlock(data.lowEndBuffer.getArrayOfWritePointers(), bufferChannelCount, bufferSampleCount);
		juce::dsp::AudioBlock<SampleType> highEndBufferBlock(data.highEndBuffer.getArrayOfWritePointers(), bufferChannelCount, bufferSampleCount);
		juce::dsp::AudioBlock<SampleType> highEndPostDelayBufferBlock(data.highEndPostDelayBuffer.getArrayOfWritePointers(), bufferChannelCount, bufferSampleCount);
		juce::dsp::AudioBlock<SampleType> lowBufferBlock(data.lowBuffer.getArrayOfWritePointers(), bufferChannelCount, bufferSampleCount);
		juce::dsp::AudioBlock<SampleType> midLowBufferBlock(data.midLowBuffer.getArrayOfWritePointers(), bufferChannelCount, bufferSampleCount);
		juce::dsp::AudioBlock<SampleType> midHighBufferBlock(data.midHighBuffer.getArrayOfWritePointers(), bufferChannelCount, bufferSampleCount);
		juce::dsp::AudioBlock<SampleType> highBufferBlock(data.highBuffer.getArrayOfWritePointers(), bufferChannelCount, bufferSampleCount);

		//Create containers that have pointers to the audio blocks.
		//This added complexity is used by the dsp namespace for bypassing.
		juce::dsp::ProcessContextNonReplacing<SampleType> mainSplitterLowPCNR(preBufferBlock,lowEndBufferBlock);
		juce::dsp::ProcessContextNonReplacing<SampleType> mainSplitterHighPCNR(preBufferBlock,highEndBufferBlock);
		juce::dsp::ProcessContextNonReplacing<SampleType> highEndPostDelayPCNR(highEndBufferBlock,highEndPostDelayBufferBlock);
		juce::dsp::ProcessContextNonReplacing<SampleType> lowSplitterLowPCNR(lowEndBufferBlock,lowBufferBlock);
		juce::dsp::ProcessContextNonReplacing<SampleType> lowSplitterHighPCNR(lowEndBufferBlock,midLowBufferBlock);
		juce::dsp::ProcessContextNonReplacing<SampleType> highSplitterLowPCNR(highEndPostDelayBufferBlock,midHighBufferBlock);
		juce::dsp::ProcessContextNonReplacing<SampleType> highSplitterHighPCNR(highEndPostDelayBufferBlock,highBufferBlock);

		if(oneSplit){
			highEndPostDelayPCNR.isBypassed = true;
			lowSplitterLowPCNR.isBypassed = true;
			lowSplitterHighPCNR.isBypassed = true;
			highSplitterLowPCNR.isBypassed = true;
			highSplitterHighPCNR.isBypassed = true;
		}

		//Split the lower two frquency zones from the higher two frequency zones.
		data.mainSplitter.process(mainSplitterLowPCNR,mainSplitterHighPCNR);
		//Split the lower buffer.
		data.lowSplitter.process(lowSplitterLowPCNR,lowSplitterHighPCNR);
		//Delay the higher buffer so that the samples line up to match the lower buffer because it takes more taps to filter.
		data.highSplitterDelayLine.process(highEndPostDelayPCNR);
		//Split the higher buffer.
		data.highSplitter.process(highSplitterLowPCNR,highSplitterHighPCNR);

		if(oneSplit){
			switch(currentTab){
				case TabName::Low:
					sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PreLow, bufferSampleCount, data.lowEndBuffer);
					break;
				case TabName::High:
					sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PreHigh, bufferSampleCount, data.highEndBuffer);
					break;
			}	
		}else{
			switch(currentTab){
				case TabName::Low:
					sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PreLow, bufferSampleCount, data.lowBuffer);
					break;
				case TabName::MidLow:
					sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PreMidLow, bufferSampleCount, data.midLowBuffer);
					break;
				case TabName::MidHigh:
					sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PreMidHigh, bufferSampleCount, data.midHighBuffer);
					break;
				case TabName::High:
					sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PreHigh, bufferSampleCount, data.highBuffer);
					break;
			}
		}

		//Process the split buffers with the side shapers.
		//We only need to process the samples if the user wants to hear the "after" which is the default.
		juce::dsp::ProcessContextReplacing<SampleType> lowBufferBlockPostSplitPCR(lowBufferBlock);
		juce::dsp::ProcessContextReplacing<SampleType> midLowBufferBlockPostSplitPCR(midLowBufferBlock);
		juce::dsp::ProcessContextReplacing<SampleType> midHighBufferBlockPostSplitPCR(midHighBufferBlock);
		juce::dsp::ProcessContextReplacing<SampleType> highBufferBlockPostSplitPCR(highBufferBlock);

		if(dataBuffer->soloBeforeOrAfter[TabName::Low].get()){
			#if !COMPILING_WITH_CLANG
			data.sideShapers[ShaperName::LowMid].process<juce::dsp::ProcessContextReplacing<SampleType>>(lowBufferBlockPostSplitPCR);
			data.sideShapers[ShaperName::LowSide].process<juce::dsp::ProcessContextReplacing<SampleType>>(lowBufferBlockPostSplitPCR);
			#else
			data.sideShapers[ShaperName::LowMid].template
				process<juce::dsp::ProcessContextReplacing<SampleType>>(lowBufferBlockPostSplitPCR);
			data.sideShapers[ShaperName::LowSide].template
				process<juce::dsp::ProcessContextReplacing<SampleType>>(lowBufferBlockPostSplitPCR);
			#endif
		}
		if(dataBuffer->soloBeforeOrAfter[TabName::MidLow].get()){
			#if !COMPILING_WITH_CLANG
			data.sideShapers[ShaperName::MidLowMid].process<juce::dsp::ProcessContextReplacing<SampleType>>(midLowBufferBlockPostSplitPCR);
			data.sideShapers[ShaperName::MidLowSide].process<juce::dsp::ProcessContextReplacing<SampleType>>(midLowBufferBlockPostSplitPCR);
			#else
			data.sideShapers[ShaperName::MidLowMid].template
				process<juce::dsp::ProcessContextReplacing<SampleType>>(midLowBufferBlockPostSplitPCR);
			data.sideShapers[ShaperName::MidLowSide].template
				process<juce::dsp::ProcessContextReplacing<SampleType>>(midLowBufferBlockPostSplitPCR);
			#endif
		}
		if(dataBuffer->soloBeforeOrAfter[TabName::MidHigh].get()){
			#if !COMPILING_WITH_CLANG
			data.sideShapers[ShaperName::MidHighMid].process<juce::dsp::ProcessContextReplacing<SampleType>>(midHighBufferBlockPostSplitPCR);
			data.sideShapers[ShaperName::MidHighSide].process<juce::dsp::ProcessContextReplacing<SampleType>>(midHighBufferBlockPostSplitPCR);
			#else
			data.sideShapers[ShaperName::MidHighMid].template
				process<juce::dsp::ProcessContextReplacing<SampleType>>(midHighBufferBlockPostSplitPCR);
			data.sideShapers[ShaperName::MidHighSide].template
				process<juce::dsp::ProcessContextReplacing<SampleType>>(midHighBufferBlockPostSplitPCR);
			#endif
		}
		if(dataBuffer->soloBeforeOrAfter[TabName::High].get()){
			#if !COMPILING_WITH_CLANG
			data.sideShapers[ShaperName::HighMid].process<juce::dsp::ProcessContextReplacing<SampleType>>(highBufferBlockPostSplitPCR);
			data.sideShapers[ShaperName::HighSide].process<juce::dsp::ProcessContextReplacing<SampleType>>(highBufferBlockPostSplitPCR);
			#else
			data.sideShapers[ShaperName::HighMid].template
				process<juce::dsp::ProcessContextReplacing<SampleType>>(highBufferBlockPostSplitPCR);
			data.sideShapers[ShaperName::HighSide].template
				process<juce::dsp::ProcessContextReplacing<SampleType>>(highBufferBlockPostSplitPCR);
			#endif
		}

		switch(currentTab){
			case TabName::Low:
				sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PostLow, bufferSampleCount, data.lowBuffer);
				break;
			case TabName::MidLow:
				sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PostMidLow, bufferSampleCount, data.midLowBuffer);
				break;
			case TabName::MidHigh:
				sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PostMidHigh, bufferSampleCount, data.midHighBuffer);
				break;
			case TabName::High:
				sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PostHigh, bufferSampleCount, data.highBuffer);
				break;
		}
	}

	bool solodOnPre = dataBuffer->solo[TabName::Pre].get();
	//Now that we have all of the copies of the original buffer that we need, we can reuse the original buffer to recombine the split buffers.
	buffer.clear();
	if(bypassSplitters){
		for (int i = 0; i < bufferChannelCount; ++i) {
			buffer.addFrom(i, 0, preBuffer, i, 0, bufferSampleCount);
		}
	}else{
		for (int i = 0; i < bufferChannelCount; ++i) {
			if(dataBuffer->solo[TabName::Post].get()){
				buffer.addFrom(i, 0, data.lowBuffer, i, 0, bufferSampleCount);
				if(!oneSplit){
					buffer.addFrom(i, 0, data.midLowBuffer, i, 0, bufferSampleCount);
					buffer.addFrom(i, 0, data.midHighBuffer, i, 0, bufferSampleCount);
				}
				buffer.addFrom(i, 0, data.highBuffer, i, 0, bufferSampleCount);
			}else if(solodOnPre){
				buffer.addFrom(i, 0, preBuffer, i, 0, bufferSampleCount);
			}else{
				if(oneSplit){
					if(dataBuffer->solo[TabName::Low].get()){
						buffer.addFrom(i, 0, data.lowBuffer, i, 0, bufferSampleCount);
					}
					if(dataBuffer->solo[TabName::High].get()){
						buffer.addFrom(i, 0, data.midHighBuffer, i, 0, bufferSampleCount);
					}
				}else{
					if(dataBuffer->solo[TabName::Low].get()){
						buffer.addFrom(i, 0, data.lowBuffer, i, 0, bufferSampleCount);
					}
					if(dataBuffer->solo[TabName::MidLow].get()){
						buffer.addFrom(i, 0, data.midLowBuffer, i, 0, bufferSampleCount);
					}
					if(dataBuffer->solo[TabName::MidHigh].get()){
						buffer.addFrom(i, 0, data.midHighBuffer, i, 0, bufferSampleCount);
					}
					if(dataBuffer->solo[TabName::High].get()){
						buffer.addFrom(i, 0, data.highBuffer, i, 0, bufferSampleCount);
					}
				}
			}
		}
	}

	//Before processing the buffer with the Post tab side shaper, send the values to the left vectorscope of the Post tab.
	if(currentTab == TabName::Post){
		sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PrePostGlobal, bufferSampleCount, buffer);
	}

	juce::AudioBuffer<SampleType> postBuffer;
	postBuffer.makeCopyOf(buffer);
	juce::dsp::AudioBlock<SampleType> postBufferBlock(postBuffer.getArrayOfWritePointers(), bufferChannelCount, bufferSampleCount);
	
	//Process the output buffer with the post-split side shaper.
	if(dataBuffer->soloBeforeOrAfter[TabName::Post].get()){
		juce::dsp::ProcessContextReplacing<SampleType> postBufferBlockPCR(postBufferBlock);
		#if !COMPILING_WITH_CLANG
		data.sideShapers[ShaperName::GlobalMidPost].process<juce::dsp::ProcessContextReplacing<SampleType>>(postBufferBlockPCR);
		data.sideShapers[ShaperName::GlobalSidePost].process<juce::dsp::ProcessContextReplacing<SampleType>>(postBufferBlockPCR);
		#else
		data.sideShapers[ShaperName::GlobalMidPost].template
			process<juce::dsp::ProcessContextReplacing<SampleType>>(postBufferBlockPCR);
		data.sideShapers[ShaperName::GlobalSidePost].template
			process<juce::dsp::ProcessContextReplacing<SampleType>>(postBufferBlockPCR);
		#endif
	}

	//Now that we have processed the Post tab, we can calculate the right vectorscope of the Post tab.
	if(currentTab == TabName::Post){
		sendVectorscopeData<SampleType>(midSideToEditor, SplitBufferValuesBufferName::PostPostGlobal, bufferSampleCount, postBuffer);
	}

	if(solodOnPre){
		buffer.makeCopyOf(preBuffer);
	}else{
		buffer.makeCopyOf(postBuffer);
	}

	//Perform the simple shape reshaping if the settings to do that are active.
	//This is down here for the scenario where the user wants to reshape only a particular frequency zone in one soloed instance and then pass through the rest in another instance.
	int inputShape = dataBuffer->inputShape.get();
	int outputShape = dataBuffer->outputShape.get();
	if(inputShape != VectorscopeShape::None && outputShape != VectorscopeShape::None && inputShape != outputShape){
		for (int sampleNumber = 0; sampleNumber < bufferSampleCount; sampleNumber++) {
			const SampleType* leftChannelRead = buffer.getReadPointer(0);
			const SampleType* rightChannelRead = buffer.getReadPointer(1);
			SampleType* leftChannelWrite = buffer.getWritePointer(0);
			SampleType* rightChannelWrite = buffer.getWritePointer(1);
			std::array<SampleType,2> midSideValues = leftRightToMidSide(leftChannelRead[sampleNumber],rightChannelRead[sampleNumber]);
			std::array<SampleType,2> polarValues = cartesianToPolar(midSideValues[0],midSideValues[1]);
			std::array<SampleType,2> mappedValues = mapShapeToShapeRayTracing(polarValues[0],polarValues[1],inputShape,outputShape);
			std::array<SampleType,2> cartesianValues = polarToCartesian(mappedValues[0],mappedValues[1]);
			std::array<SampleType,2> newValues = midSideToLeftRight(cartesianValues[0],cartesianValues[1]);
			leftChannelWrite[sampleNumber] = newValues[0];
			rightChannelWrite[sampleNumber] = newValues[1];
		}
	}

	//Only try to send the editor data if it truly exists and is not in the process of being deleted.
	int start1 = 0;
	int size1 = 0;
	int start2 = 0;
	int size2 = 0;
	int freeSpace = dataBuffer->graphicsDataHandler.getFreeSpace();
	if(freeSpace < bufferSampleCount){
		dataBuffer->graphicsDataHandler.prepareToWrite(freeSpace, start1, size1, start2, size2);
	}else{
		dataBuffer->graphicsDataHandler.prepareToWrite(bufferSampleCount, start1, size1, start2, size2);
	}
	if (size1 > 0) {
		for (int i = 0; i < size1; ++i) {
			editorData[start1 + i] = midSideToEditor[i];
		}
	}
	if (size2 > 0) {
		for (int i = 0; i < size2; ++i) {
			editorData[start2 + i] = midSideToEditor[i];
		}
	}
	dataBuffer->graphicsDataHandler.finishedWrite(size1 + size2);

	//This is all the way down here because I still want the graphics to run even if the audio is not affected.
	if(bypassAll){
		buffer.makeCopyOf(bufferCopy,true);
	}
}

bool SideShaperAudioProcessor::producesMidi() const {
	return false;
}

//This is called when the DAW decides to stop calling the processBlock function.
//Note that FL Studios' green button mute, for example, calls this instead of calling the processBlockBypassed function.
//Note that once the plugin is unmuted, the prepareToPlay function will be called again.
void SideShaperAudioProcessor::releaseResources() {
	reset();
}

void SideShaperAudioProcessor::reset() {
	dataBuffer->fid.mainSplitter.reset(); dataBuffer->did.mainSplitter.reset();
	dataBuffer->fid.lowSplitter.reset(); dataBuffer->did.lowSplitter.reset();
	dataBuffer->fid.highSplitter.reset(); dataBuffer->did.highSplitter.reset();
	dataBuffer->fid.highSplitterDelayLine.reset(); dataBuffer->did.highSplitterDelayLine.reset();
	dataBuffer->fid.lowEndBuffer.clear(); dataBuffer->did.lowEndBuffer.clear();
	dataBuffer->fid.highEndBuffer.clear(); dataBuffer->did.highEndBuffer.clear();
	dataBuffer->fid.highEndPostDelayBuffer.clear(); dataBuffer->did.highEndPostDelayBuffer.clear();
	dataBuffer->fid.lowBuffer.clear(); dataBuffer->did.lowBuffer.clear();
	dataBuffer->fid.midLowBuffer.clear(); dataBuffer->did.midLowBuffer.clear();
	dataBuffer->fid.midHighBuffer.clear(); dataBuffer->did.midHighBuffer.clear();
	dataBuffer->fid.highBuffer.clear(); dataBuffer->did.highBuffer.clear();
}

template<typename SampleType>
void SideShaperAudioProcessor::sendVectorscopeData(std::vector<SplitBufferValues<SampleType>>& dest, int bufferName, int numSamples, juce::AudioBuffer<SampleType>& src){
	const SampleType pt = 2.0;//positive two
	const SampleType* leftChannel = src.getReadPointer(0);
	const SampleType* rightChannel = src.getReadPointer(1);
	for (int sampleNumber = 0; sampleNumber < numSamples; sampleNumber++) {
		SampleType mid = (leftChannel[sampleNumber] + rightChannel[sampleNumber]) / pt;
		SampleType side = (leftChannel[sampleNumber] - rightChannel[sampleNumber]) / pt;
		dest[sampleNumber][bufferName][0] = mid;
		dest[sampleNumber][bufferName][1] = side;
	}
}

//Called by the host to change the current program.
void SideShaperAudioProcessor::setCurrentProgram(int index) {
	currentProgram = index;
	switch (index) {
		case 0:
			return;//Program 0 is reserved for the host.
		case 1:
			setStateInformation(BinaryData::init_xml,BinaryData::init_xmlSize);
			break;
	}
	//This is just here because it makes the program change feel more snappy than waiting for the host to realize the values changed.
	updateHostDisplay();
}

//Called when the user wants to load a preset.
void SideShaperAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
	//Parse the binary data into an XML Element.
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
	if(xmlState != nullptr){
		//This allows for backwards compatibility.
		String appVersion = xmlState->getStringAttribute("appVersion");
		if(appVersion == ProjectInfo::versionString){
			//Verify that the XML tag matches your APVTS state type to avoid corruption.
			if(xmlState->hasTagName(dataBuffer->apvts->state.getType())){
				dataBuffer->appWidth.set(xmlState->getIntAttribute("appWidth"));
				dataBuffer->appHeight.set(xmlState->getIntAttribute("appHeight"));
				dataBuffer->stateChangeTabIndex.set(xmlState->getIntAttribute("currentTabIndex"));
				dataBuffer->bypassAll.set(xmlState->getBoolAttribute("bypassAll"));
				dataBuffer->bypassSplitters.set(xmlState->getBoolAttribute("bypassSplitters"));
				dataBuffer->oneSplit.set(xmlState->getBoolAttribute("oneSplit"));
				dataBuffer->fullFrequencyCrossoverRanges.set(xmlState->getBoolAttribute("fullFrequencyCrossoverRanges"));
				dataBuffer->leftRightMode.set(xmlState->getBoolAttribute("leftRightMode"));
				dataBuffer->protectedMode.set(xmlState->getBoolAttribute("protectedMode"));
				dataBuffer->crossoverMode.set(xmlState->getIntAttribute("crossoverMode"));
				dataBuffer->shapeRangeSwapMode.set(xmlState->getBoolAttribute("shapeRangeSwapMode"));
				dataBuffer->inputShape.set(xmlState->getIntAttribute("inputShape"));
				dataBuffer->outputShape.set(xmlState->getIntAttribute("outputShape"));
				for(int i=0;i<dataBuffer->tabCount;i++){
					dataBuffer->gateBeforeStart[i][0].set(xmlState->getBoolAttribute("gateBeforeStartMid"+String(i)));
					dataBuffer->gateBeforeStart[i][1].set(xmlState->getBoolAttribute("gateBeforeStartSide"+String(i)));
					dataBuffer->flatAfterEnd[i][0].set(xmlState->getBoolAttribute("flatAfterEndMid"+String(i)));
					dataBuffer->flatAfterEnd[i][1].set(xmlState->getBoolAttribute("flatAfterEndSide"+String(i)));
					dataBuffer->solo[i].set(xmlState->getBoolAttribute("solo"+String(i)));
					dataBuffer->soloBeforeOrAfter[i].set(xmlState->getBoolAttribute("soloBeforeOrAfter"+String(i)));
					dataBuffer->leftVscopeStates[i].centerX.set((float)xmlState->getDoubleAttribute("leftVscopeStateCenterX"+String(i)));
					dataBuffer->leftVscopeStates[i].centerXOnMouseDown.set((float)xmlState->getDoubleAttribute("leftVscopeStateCenterXOnMouseDown"+String(i)));
					dataBuffer->leftVscopeStates[i].centerY.set((float)xmlState->getDoubleAttribute("leftVscopeStateCenterY"+String(i)));
					dataBuffer->leftVscopeStates[i].centerYOnMouseDown.set((float)xmlState->getDoubleAttribute("leftVscopeStateCenterYOnMouseDown"+String(i)));
					dataBuffer->leftVscopeStates[i].probabilityDistributionMode.set(xmlState->getBoolAttribute("leftVscopeStateProbabilityDistributionMode"+String(i)));
					dataBuffer->leftVscopeStates[i].probabilityDistributionMidOrSide.set(xmlState->getBoolAttribute("leftVscopeStateprobabilityDistributionMidOrSide"+String(i)));
					dataBuffer->leftVscopeStates[i].zoomX.set((float)xmlState->getDoubleAttribute("leftVscopeStateZoomX"+String(i)));
					dataBuffer->leftVscopeStates[i].zoomY.set((float)xmlState->getDoubleAttribute("leftVscopeStateZoomY"+String(i)));
					dataBuffer->rightVscopeStates[i].centerX.set((float)xmlState->getDoubleAttribute("rightVscopeStateCenterX"+String(i)));
					dataBuffer->rightVscopeStates[i].centerXOnMouseDown.set((float)xmlState->getDoubleAttribute("rightVscopeStateCenterXOnMouseDown"+String(i)));
					dataBuffer->rightVscopeStates[i].centerY.set((float)xmlState->getDoubleAttribute("rightVscopeStateCenterY"+String(i)));
					dataBuffer->rightVscopeStates[i].centerYOnMouseDown.set((float)xmlState->getDoubleAttribute("rightVscopeStateCenterYOnMouseDown"+String(i)));
					dataBuffer->rightVscopeStates[i].probabilityDistributionMode.set(xmlState->getBoolAttribute("rightVscopeStateProbabilityDistributionMode"+String(i)));
					dataBuffer->rightVscopeStates[i].probabilityDistributionMidOrSide.set(xmlState->getBoolAttribute("rightVscopeStateprobabilityDistributionMidOrSide"+String(i)));
					dataBuffer->rightVscopeStates[i].zoomX.set((float)xmlState->getDoubleAttribute("rightVscopeStateZoomX"+String(i)));
					dataBuffer->rightVscopeStates[i].zoomY.set((float)xmlState->getDoubleAttribute("rightVscopeStateZoomY"+String(i)));
					dataBuffer->inOutGridMidSideMemory[i].set(xmlState->getIntAttribute("inOutGridMidSideMemory"+String(i)));
				}
				//Convert the XML back into a ValueTree and apply it to your APVTS.
				juce::ValueTree tree = juce::ValueTree::fromXml(*xmlState);
				//This will load all of the values that are exposed to the DAW.
				dataBuffer->apvts->replaceState(tree);
				//Some of the values in the state file are only the GUI values and are not the truly internal values.
				for(int i=0;i<dataBuffer->tabCount;i++){
					dataBuffer->gateBeforeStartChangesNeedingToBeRecognizedByProcessor[i][0].set(true);
					dataBuffer->gateBeforeStartChangesNeedingToBeRecognizedByProcessor[i][1].set(true);
					dataBuffer->flatAfterEndChangesNeedingToBeRecognizedByProcessor[i][0].set(true);
					dataBuffer->flatAfterEndChangesNeedingToBeRecognizedByProcessor[i][1].set(true);
				}
				dataBuffer->oneSplitChangeNeedingToBeRecognizedByProcessor.set(true);
				dataBuffer->fullFrequencyCrossoverRangesNeedingToBeRecognizedByProcessor.set(true);
				dataBuffer->leftRightModeChangeNeedingToBeRecognizedByProcessor.set(true);
				dataBuffer->protectedModeChangeNeedingToBeRecognizedByProcessor.set(true);
				dataBuffer->crossoverModeChangeNeedingToBeRecognizedByProcessor.set(true);
				dataBuffer->shapeRangeSwapModeChangeNeedingToBeRecognizedByProcessor.set(true);
			}
			updateHostDisplay();
			dataBuffer->saveStateChangeNeedingToBeRecognizedByEditor.set(true);
		}
	}
}

int SideShaperAudioProcessor::shaperIndexAndParameterIndexToSliderIndexConversion(int sideShaperIndex, int sideShaperParameterIndex) {
	switch(sideShaperIndex){
		case ShaperName::GlobalMidPre:
		case ShaperName::LowMid:
		case ShaperName::MidLowMid:
		case ShaperName::MidHighMid:
		case ShaperName::HighMid:
		case ShaperName::GlobalMidPost:
			switch(sideShaperParameterIndex){
				case SideShaperParameter::StartDistance:
					return SliderName::MidSliderMain;
				case SideShaperParameter::EndDistance:
					return SliderName::MidSliderMain;
				case SideShaperParameter::Zone1Percentage:
					return SliderName::MidSliderMain;
				case SideShaperParameter::InflectionPointCompression:
					return SliderName::MidSliderInflection;
				case SideShaperParameter::Zone1Saturation:
					return SliderName::MidSliderZone1;
				case SideShaperParameter::Zone2Saturation:
					return SliderName::MidSliderZone2;
			}
		case ShaperName::GlobalSidePre:
		case ShaperName::LowSide:
		case ShaperName::MidLowSide:
		case ShaperName::MidHighSide:
		case ShaperName::HighSide:
		case ShaperName::GlobalSidePost:
			switch(sideShaperParameterIndex){
				case SideShaperParameter::StartDistance:
					return SliderName::SideSliderMain;
				case SideShaperParameter::EndDistance:
					return SliderName::SideSliderMain;
				case SideShaperParameter::Zone1Percentage:
					return SliderName::SideSliderMain;
				case SideShaperParameter::InflectionPointCompression:
					return SliderName::SideSliderInflection;
				case SideShaperParameter::Zone1Saturation:
					return SliderName::SideSliderZone1;
				case SideShaperParameter::Zone2Saturation:
					return SliderName::SideSliderZone2;
			}
	}
	return 0;
}

int SideShaperAudioProcessor::shaperIndexToTabIndexConversion(int sideShaperIndex){
	switch(sideShaperIndex){
		case ShaperName::GlobalMidPre:
			return TabName::Pre;
		case ShaperName::GlobalSidePre:
			return TabName::Pre;
		case ShaperName::LowMid:
			return TabName::Low;
		case ShaperName::LowSide:
			return TabName::Low;
		case ShaperName::MidLowMid:
			return TabName::MidLow;
		case ShaperName::MidLowSide:
			return TabName::MidLow;
		case ShaperName::MidHighMid:
			return TabName::MidHigh;
		case ShaperName::MidHighSide:
			return TabName::MidHigh;
		case ShaperName::HighMid:
			return TabName::High;
		case ShaperName::HighSide:
			return TabName::High;
		case ShaperName::GlobalMidPost:
			return TabName::Post;
		case ShaperName::GlobalSidePost:
			return TabName::Post;
	}
	return 0;
}

SideShaperAudioProcessor::~SideShaperAudioProcessor(){
	releaseResources();
	delete editor;
	delete dataBuffer;
}