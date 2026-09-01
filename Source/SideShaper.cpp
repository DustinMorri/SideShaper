#include <cmath>
#include "SideShaper.h"

template<typename SampleType>
SideShaper<SampleType>::SideShaper() {
	updateDistances(startDistance, endDistance);
	update();
}
//If you want to see what this function does, paste this LaTeX formula into the Desmos online graphing calculator.
//f\left(x\right)=\frac{x\left(a+1\right)}{a\left|x\right|+1}
//in - 0.0 to 1.0
//tension - 0.0 to 1.0 where 0.0 is a linear slope and 1.0 is a right angle
//returns values 0.0 to 1.0 closer to the bottom right of the graph depending on the tension amount
template<typename SampleType>
SampleType SideShaper<SampleType>::curveCompress(SampleType in, SampleType tension) {
	tension *= mo;
	return (in * (tension + po)) / (tension * fabs(in) + po);
}

//y=1-f\left(1-x\right)
//in - 0.0 to 1.0
//tension - 0.0 to 1.0 where 0.0 is a linear slope and 1.0 is a right angle
//returns values 0.0 to 1.0 closer to the top left of the graph depending on the tension amount
template<typename SampleType>
SampleType SideShaper<SampleType>::curveSaturate(SampleType in, SampleType tension) {
	return po - SideShaper::curveCompress(po - in, tension);
}

//This function just does the opposite of normalize.
//in - 0.0 to 1.0
//startDistance - 0.0 to 1.0
//length - 0.0 to 1.0
//returns values 0.0 to 1.0
template<typename SampleType>
SampleType SideShaper<SampleType>::denormalize(SampleType in, SampleType startDistance, SampleType length) {
	return startDistance + in * length;
}

//This function normalizes to a value 0.0 to 1.0 any value in between startDistance and startDistance + length.
//This is used so that the curveCompress function's tension amount is relative to the size of the zone.
//in - 0.0 to 1.0
//startDistance - 0.0 to 1.0
//length - 0.0 to 1.0
//returns values 0.0 to 1.0
template<typename SampleType>
SampleType SideShaper<SampleType>::normalize(SampleType in, SampleType startDistance, SampleType length) {
	return (in - startDistance) / length;
}

template<typename SampleType>
void SideShaper<SampleType>::prepare(const juce::dsp::ProcessSpec&) {
	//jassert(spec.numChannels == 2);
	updateDistances(startDistance, endDistance);
	update();
	reset();
}

template<typename SampleType>
typename SideShaper<SampleType>::TwoSamples SideShaper<SampleType>::processSample(SideShaper<SampleType>::TwoSamples input) {
	//Split the mid and side signals.
	SampleType mid, side;
	bool inputMidIsZero = false;
	bool inputSideIsZero = false;
	if(!leftRightMode){
		mid = (input.data[0] + input.data[1]) / pt;
		side = (input.data[0] - input.data[1]) / pt;
	}else{
		mid = input.data[0];
		side = input.data[1];
	}
	if(mid == 0.0) inputMidIsZero = true;
	if(side == 0.0) inputSideIsZero = true;

	//If mid mode is not active or both mode is active,
	if (!midMode || bothMode) {
		SampleType sideSign = zz;
		if (side > zz) {
			sideSign = po;
		}
		else if (side < zz) {
			sideSign = mo;
		}
		SampleType absMid = fabs(mid);
		SampleType absSide = fabs(side);
		if(shapeRangeSwap){
			if (absMid > startDistance && absMid < endDistance) {
				SampleType outSide = absSide;
				//If we're in zone 1,
				if (absSide <= inflectionPointX) {
					if (zone1saturate) {
						outSide = denormalize(curveSaturate(normalize(absSide, 0.0, zone1xLength), zone1tension), 0.0, zone1yLength);
					} else {
						outSide = denormalize(curveCompress(normalize(absSide, 0.0, zone1xLength), zone1tension), 0.0, zone1yLength);
					}
				}
				else if (absSide > inflectionPointX) {
					if (zone2saturate) {
						outSide = denormalize(curveSaturate(normalize(absSide, inflectionPointX, zone2xLength), zone2tension), inflectionPointY, zone2yLength);
					} else {
						outSide = denormalize(curveCompress(normalize(absSide, inflectionPointX, zone2xLength), zone2tension), inflectionPointY, zone2yLength);
					}
				}
				side = sideSign * outSide;
			}
			if(gateBeforeStart && absMid <= startDistance){
				side = 0.0;
			}
			if(flatAfterEnd && absMid >= endDistance){
				side = 0.0;
			}
		}else{
			if (absSide > startDistance && absSide < endDistance) {
				SampleType outSide = absSide;
				//If we're in zone 1,
				if (absSide <= inflectionPointX) {
					if (zone1saturate) {
						outSide = denormalize(curveSaturate(normalize(absSide, startDistance, zone1xLength), zone1tension), startDistance, zone1yLength);
					} else {
						outSide = denormalize(curveCompress(normalize(absSide, startDistance, zone1xLength), zone1tension), startDistance, zone1yLength);
					}
				}
				else if (absSide > inflectionPointX) {
					if (zone2saturate) {
						outSide = denormalize(curveSaturate(normalize(absSide, inflectionPointX, zone2xLength), zone2tension), inflectionPointY, zone2yLength);
					} else {
						outSide = denormalize(curveCompress(normalize(absSide, inflectionPointX, zone2xLength), zone2tension), inflectionPointY, zone2yLength);
					}
				}
				side = sideSign * outSide;
			}
			if(gateBeforeStart && absSide <= startDistance){
				side = 0.0;
			}
			if(flatAfterEnd && absSide >= endDistance){
				side = sideSign * endDistance;
			}
		}
	}
	//If mid mode is active or both mode is active,
	if (midMode || bothMode) {
		SampleType midSign = zz;
		if (mid > zz) {
			midSign = po;
		}
		else if (mid < zz) {
			midSign = mo;
		}
		SampleType absMid = fabs(mid);
		SampleType absSide = fabs(side);
		if(shapeRangeSwap){
			if (absSide > startDistance && absSide < endDistance) {
				SampleType outMid = absMid;
				//If we're in zone 1,
				if (absMid <= inflectionPointX) {
					if (zone1saturate) {
						outMid = denormalize(curveSaturate(normalize(absMid, 0.0, zone1xLength), zone1tension), 0.0, zone1yLength);
					} else {
						outMid = denormalize(curveCompress(normalize(absMid, 0.0, zone1xLength), zone1tension), 0.0, zone1yLength);
					}
				}
				else if (absMid > inflectionPointX) {
					if (zone2saturate) {
						outMid = denormalize(curveSaturate(normalize(absMid, inflectionPointX, zone2xLength), zone2tension), inflectionPointY, zone2yLength);
					} else {
						outMid = denormalize(curveCompress(normalize(absMid, inflectionPointX, zone2xLength), zone2tension), inflectionPointY, zone2yLength);
					}
				}
				mid = midSign * outMid;
			}
			if(gateBeforeStart && absSide <= startDistance){
				mid = 0.0;
			}
			if(flatAfterEnd && absSide >= endDistance){
				mid = 0.0;
			}
		}else{
			if (absMid > startDistance && absMid < endDistance) {
				SampleType outMid = absMid;
				//If we're in zone 1,
				if (absMid <= inflectionPointX) {
					if (zone1saturate) {
						outMid = denormalize(curveSaturate(normalize(absMid, startDistance, zone1xLength), zone1tension), startDistance, zone1yLength);
					} else {
						outMid = denormalize(curveCompress(normalize(absMid, startDistance, zone1xLength), zone1tension), startDistance, zone1yLength);
					}
				}
				else if (absMid > inflectionPointX) {
					if (zone2saturate) {
						outMid = denormalize(curveSaturate(normalize(absMid, inflectionPointX, zone2xLength), zone2tension), inflectionPointY, zone2yLength);
					} else {
						outMid = denormalize(curveCompress(normalize(absMid, inflectionPointX, zone2xLength), zone2tension), inflectionPointY, zone2yLength);
					}
				}
				mid = midSign * outMid;
			}
			if(gateBeforeStart && absMid <= startDistance){
				mid = 0.0;
			}
			if(flatAfterEnd && absMid >= endDistance){
				mid = midSign * endDistance;
			}
		}
	}

	//Protect the signal.
	if(mid > po){
		mid = po;
	}else if(mid < mo) {
		mid = mo;
	}
	if(side > po) {
		side = po;
	}else if(side < mo) {
		side = mo;
	}
	if(inputMidIsZero){
		mid = zz;
	}
	if(inputSideIsZero){
		side = zz;
	}

	//Recombine the mid and side signals.
	SideShaper<SampleType>::TwoSamples output;
	if(!leftRightMode){
		output.data[0] = mid + side;
		output.data[1] = mid - side;
	}else{
		output.data[0] = mid;
		output.data[1] = side;
	}

	if(std::isnan(output.data[0])){
		output.data[0] = zz;
	}
	if(std::isnan(output.data[1])){
		output.data[1] = zz;
	}

	//In protected mode, the left and right channels are both hard limited to values of +1.0 and -1.0.
	if(protectedMode){
		if(output.data[0] > po){
			output.data[0] = po;
		}else if(output.data[0] < mo){
			output.data[0] = mo;
		}
		if(output.data[1] > po){
			output.data[1] = po;
		}else if(output.data[1] < mo){
			output.data[1] = mo;
		}
	}

	return output;
}

template<typename SampleType>
void SideShaper<SampleType>::reset() {
	//If I decide at some point to use an internal vector of samples, this is where that vector would be initialized with values of 0.
	//std::fill(s->begin(), s->end(), static_cast<SampleType> (0));
}

template<typename SampleType>
void SideShaper<SampleType>::snapToZero() noexcept {
	//If I decide at some point to use an internal vector of samples, this is where the denormals (i.e. values really close to 0, between -0.00000001 and 0.00000001) would be zeroed.
	//util::snapToZero(element);
}

template<typename SampleType>
void SideShaper<SampleType>::update() {
	//Any variable that only changes with user input (i.e. not changing for every sample) gets updated here to save computation power.
	SampleType z1pOffset = zone1percentage * diameter;
	SampleType ipcDirection = zf - inflectionPointCompression;
	SampleType ipcDiameter = fabs(zone1percentage - zf) * pt;
	ipcDiameter = po - ipcDiameter;
	ipcDiameter *= diameter;
	SampleType ipcOffset = ipcDirection * ipcDiameter;

	if(shapeRangeSwap){
		inflectionPointX = z1pOffset + ipcOffset;
		inflectionPointY = z1pOffset - ipcOffset;
		zone1xLength = inflectionPointX;
		zone1yLength = inflectionPointY;
		zone2xLength = static_cast<SampleType>(1.0) - inflectionPointX;
		zone2yLength = static_cast<SampleType>(1.0) - inflectionPointY;
	}else{
		inflectionPointX = startDistance + z1pOffset + ipcOffset;
		inflectionPointY = startDistance + z1pOffset - ipcOffset;
		zone1xLength = inflectionPointX - startDistance;
		zone1yLength = inflectionPointY - startDistance;
		zone2xLength = endDistance - inflectionPointX;
		zone2yLength = endDistance - inflectionPointY;
	}
	//this is where zone1saturation would be set
	zone1saturate = 0;
	zone1tension = (zf - zone1saturation) * pt;
	if (zone1saturation > zf) {
		zone1saturate = 1;
		zone1tension = (zone1saturation - zf) * pt;
	}
	//this is where zone2saturation would be set
	zone2saturate = 0;
	zone2tension = (zf - zone2saturation) * pt;
	if (zone2saturation > zf) {
		zone2saturate = 1;
		zone2tension = (zone2saturation - zf) * pt;
	}
}

template<typename SampleType>
void SideShaper<SampleType>::updateDistances(SampleType startVal, SampleType endVal) {
	if (relativePositioning) {
		SampleType inflectionPointDistance;
		SampleType radius;
		inflectionPointDistance = startVal;
		radius = endVal / pt;
		startDistance = inflectionPointDistance - radius;
		endDistance = inflectionPointDistance + radius;
		if (startDistance < zz) {
			radius = inflectionPointDistance;
			startDistance = zz;
			endDistance = inflectionPointDistance + radius;
		}
		else if (endDistance > po) {
			radius = po - inflectionPointDistance;
			startDistance = inflectionPointDistance - radius;
			endDistance = po;
		}
	}
	diameter = endDistance - startDistance;
	zone1percentage = (zone1percentageRaw - startDistance) / diameter;
}

template<typename SampleType>
SideShaper<SampleType>::~SideShaper(){
	
}

template class SideShaper<float>;
template class SideShaper<double>;