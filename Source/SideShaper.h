#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <JuceHeader.h>

enum SideShaperParameter : int {
	StartDistance,
	EndDistance,
	Zone1Percentage,
	InflectionPointCompression,
	Zone1Saturation,
	Zone2Saturation
};

template<typename SampleType>
class SideShaper {
	public:
		struct TwoSamples {
			SampleType data[2];
		};
		SideShaper();
		static SampleType curveCompress(SampleType in, SampleType tension);
		static SampleType curveSaturate(SampleType in, SampleType tension);
		static SampleType denormalize(SampleType in, SampleType startDistance, SampleType length);
		static SampleType normalize(SampleType in, SampleType startDistance, SampleType length);
		void prepare(const juce::dsp::ProcessSpec&);
		template <typename ProcessContext>
		void process(const ProcessContext& context) noexcept {
			const auto& inputBlock = context.getInputBlock();
			auto& outputBlock = context.getOutputBlock();
			//const auto numChannels = outputBlock.getNumChannels();
			const auto numSamples = outputBlock.getNumSamples();
			//Ensure that the input block has the same number of channels and samples as the output block.
			//jassert(inputBlock.getNumChannels() == numChannels);
			jassert(inputBlock.getNumSamples() == numSamples);
			//Bypass if bypassed.
			if (context.isBypassed) {
				outputBlock.copyFrom(inputBlock);
				return;
			}
			//Process the samples.
			auto* inputSamplesL = inputBlock.getChannelPointer(0);
			auto* inputSamplesR = inputBlock.getChannelPointer(1);
			auto* outputSamplesL = outputBlock.getChannelPointer(0);
			auto* outputSamplesR = outputBlock.getChannelPointer(1);
			for (size_t i = 0; i < numSamples; ++i) {
				SideShaper<SampleType>::TwoSamples input{ inputSamplesL[i],inputSamplesR[i] };
				SideShaper<SampleType>::TwoSamples output = processSample(input);
				outputSamplesL[i] = output.data[0];
				outputSamplesR[i] = output.data[1];
			}
#if JUCE_DSP_ENABLE_SNAP_TO_ZERO
			snapToZero();
#endif
		}
		TwoSamples processSample(TwoSamples inputValue);
		void reset();
		SampleType getParameter(int parameter){
			switch(parameter){
				case SideShaperParameter::StartDistance:
					return startDistance;
				case SideShaperParameter::EndDistance:
					return endDistance;
				case SideShaperParameter::Zone1Percentage:
					return zone1percentageRaw;
				case SideShaperParameter::InflectionPointCompression:
					return inflectionPointCompression;
				case SideShaperParameter::Zone1Saturation:
					return zone1saturation;
				case SideShaperParameter::Zone2Saturation:
					return zone2saturation;
			}
			return (SampleType)0.0;
		}
		void setRelativePositioning(bool val, SampleType startDistanceVal, SampleType endDistanceOrRadiusVal) {
			relativePositioning = val;
			startDistance = startDistanceVal;
			endDistance = endDistanceOrRadiusVal;
			updateDistances(startDistance, endDistance);
			update();
		}
		void setMidMode(bool val) {
			midMode = val;
		}
		void setBothMode(bool val) {
			bothMode = val;
		}
		void setLeftRightMode(bool val){
			leftRightMode = val;
		}
		void setProtectedMode(bool val){
			protectedMode = val;
		}
		void setShapeRangeSwap(bool val){
			if(val){
				diameter = (SampleType)1.0;
				update();
			}else{
				updateDistances(startDistance,endDistance);
				update();
			}
			shapeRangeSwap = val;
		}
		void setGateBeforeStart(bool val){
			gateBeforeStart = val;
		}
		void setFlatAfterEnd(bool val){
			flatAfterEnd = val;
		}
		void setStartDistance(SampleType val) {
			if(val > endDistance){
				return;
			}
			//The three value sliders can move the zone 1 percentage, but the end knobs cannot move each other.
			if(val > zone1percentageRaw){
				setZone1PercentageRelative(val);
			}
			if(val < 0.0){
				val = 0.0;
			}
			startDistance = val;
			if(!shapeRangeSwap){
				updateDistances(val, endDistance);
				update();
			}
		}
		void setEndDistance(SampleType val) {
			if(val < startDistance){
				return;
			}
			if(val < zone1percentageRaw){
				setZone1PercentageRelative(val);
			}
			if(val > 1.0){
				val = 1.0;
			}
			endDistance = val;
			if(!shapeRangeSwap){
				updateDistances(startDistance, val);
				update();
			}
		}
		void setZone1Percentage(SampleType val) {
			if(val < startDistance){
				return;
			}
			if(val > endDistance){
				return;
			}
			zone1percentageRaw = val;
			zone1percentage = val;
			update();
		}
		void setZone1PercentageRelative(SampleType val){
			if(val < startDistance){
				return;
			}
			if(val > endDistance){
				return;
			}
			SampleType currentRange = endDistance - startDistance;
			zone1percentageRaw = val;
			zone1percentage = (val - startDistance) / currentRange;
			update();
		}
		void setInflectionPointCompression(SampleType val) {
			inflectionPointCompression = val;
			update();
		}
		void setZone1Saturation(SampleType val) {
			zone1saturation = val;
			update();
		}
		void setZone2Saturation(SampleType val) {
			zone2saturation = val;
			update();
		}
		void setParameter(int parameter, SampleType val){
			switch(parameter){
				case SideShaperParameter::StartDistance:
					setStartDistance(val);
					break;
				case SideShaperParameter::EndDistance:
					setEndDistance(val);
					break;
				case SideShaperParameter::Zone1Percentage:
					setZone1PercentageRelative(val);
					break;
				case SideShaperParameter::InflectionPointCompression:
					setInflectionPointCompression(val);
					break;
				case SideShaperParameter::Zone1Saturation:
					setZone1Saturation(val);
					break;
				case SideShaperParameter::Zone2Saturation:
					setZone2Saturation(val);
					break;
			}
		}
		void snapToZero() noexcept;
		~SideShaper();
	private:
		void update();
		void updateDistances(SampleType startVal, SampleType endVal);
		bool relativePositioning = false;
		bool midMode = false;
		bool bothMode = false;
		bool leftRightMode = false;
		//In protected mode, the left and right channels are both hard limited to values of +1.0 and -1.0.
		bool protectedMode = false;//This value is intentionally false because resolving overcompression and hard limiting requires sending hot signals (i.e. digital float values potentially over 1.0 or below -1.0).
		bool shapeRangeSwap = false;
		bool zone1saturate = false;
		bool zone2saturate = false;
		bool gateBeforeStart = false;
		bool flatAfterEnd = false;
		SampleType startDistance = 0.0;
		SampleType endDistance = 1.0;
		SampleType diameter;
		SampleType zone1percentage = 0.5;//This value is 0.0 to 1.0 normalized for the startDistance and endDistance.
		SampleType zone1percentageRaw = 0.5;
		SampleType inflectionPointCompression = 0.5;
		SampleType zone1saturation = 0.5;
		SampleType zone2saturation = 0.5;
		SampleType inflectionPointX, inflectionPointY;
		SampleType zone1xLength, zone2xLength, zone1yLength, zone2yLength;
		SampleType zone1tension, zone2tension;
		//Because we don't know whether SampleType will be a float or double, we need these defined here so that no implicit conversions will be performed leading to quality loss.
		static inline const SampleType zz = 0.0;//zero (point) zero
		static inline const SampleType zf = 0.5;//zero (point) five
		static inline const SampleType po = 1.0;//positive one
		static inline const SampleType mo = -1.0;//minus one
		static inline const SampleType pt = 2.0;//positive two

};