#include "Vectorscope.h"

Vectorscope::Vectorscope(DataBuffer* db, bool leftOrRightVal) : Component(){
	leftOrRight = leftOrRightVal;
	dataBuffer = db;
	setSize(250,250);
	vsPdSwitchButton.setImages(true,true,true,barChartImage,0.70f,Colours::transparentBlack,barChartImage,1.00f,Colours::transparentBlack,barChartImage,1.00f,Colours::transparentBlack);
	vsPdSwitchButton.onClick = [this](){
		//If we were in probability distribution mode and we are now going back to vectorscope mode,
		if(probabilityDistributionMode){
			probabilityDistributionMode = false;
			vsPdSwitchButton.setImages(true,true,true,barChartImage,0.70f,Colours::transparentBlack,barChartImage,1.00f,Colours::transparentBlack,barChartImage,1.00f,Colours::transparentBlack);
			//Changing the image resets the width and height to the width and height of the image.
			vsPdSwitchButton.setBounds(vsPdSwitchButton.getX(),vsPdSwitchButton.getY(),buttonSize,buttonSize);
			midSideSwitchButton.setVisible(false);
		}else{
			probabilityDistributionMode = true;
			vsPdSwitchButton.setImages(true,true,true,scatterPlotImage,0.70f,Colours::transparentBlack,scatterPlotImage,1.00f,Colours::transparentBlack,scatterPlotImage,1.00f,Colours::transparentBlack);
			vsPdSwitchButton.setBounds(vsPdSwitchButton.getX(),vsPdSwitchButton.getY(),buttonSize,buttonSize);
			midSideSwitchButton.setVisible(true);
		}
		uploadStateToDataBuffer();
	};
	midSideSwitchButton.setImages(true,true,true,arrowTopRightImage,0.70f,Colours::transparentBlack,arrowTopRightImage,1.00f,Colours::transparentBlack,arrowTopRightImage,1.00f,Colours::transparentBlack);
	midSideSwitchButton.onClick = [this](){
		probabilityDistributionMidOrSide = !probabilityDistributionMidOrSide;
		uploadStateToDataBuffer();
	};
	zoomInButton.setImages(true,true,true,zoomInImage,0.70f,Colours::transparentBlack,zoomInImage,1.00f,Colours::transparentBlack,zoomInImage,1.00f,Colours::transparentBlack);
	zoomInButton.onClick = [this](){
		zoomX += 33.3f;
		zoomY += 33.3f;	
		uploadStateToDataBuffer();
	};
	zoomOutButton.setImages(true,true,true,zoomOutImage,0.70f,Colours::transparentBlack,zoomOutImage,1.00f,Colours::transparentBlack,zoomOutImage,1.00f,Colours::transparentBlack);
	zoomOutButton.onClick = [this](){
		zoomX -= 33.3f;
		zoomY -= 33.3f;
		uploadStateToDataBuffer();
	};
	zoomToQuadrant1Button.setImages(true,true,true,positionTopRightImage,0.70f,Colours::transparentBlack,positionTopRightImage,1.00f,Colours::transparentBlack,positionTopRightImage,1.00f,Colours::transparentBlack);
	zoomToQuadrant1Button.onClick = [this](){
		centerX = 0.0f;
		centerXOnMouseDown = 0.0f;
		centerY = 248.0f;
		centerYOnMouseDown = 248.0f;
		zoomX = 248.0f;
		zoomY = 248.0f;
		uploadStateToDataBuffer();
	};
	recenterButton.setImages(true,true,true,recenterImage,0.70f,Colours::transparentBlack,recenterImage,1.00f,Colours::transparentBlack,recenterImage,1.00f,Colours::transparentBlack);
	recenterButton.onClick = [this](){
		centerX = 123.0f;
		centerXOnMouseDown = 123.0f;
		centerY = 123.0f;
		centerYOnMouseDown = 123.0f;
		zoomX = 123.0f;
		zoomY = 123.0f;
		uploadStateToDataBuffer();
	};
	copyButton.setImages(true,true,true,copyImage,0.70f,Colours::transparentBlack,copyImage,1.00f,Colours::transparentBlack,copyImage,1.00f,Colours::transparentBlack);
	//The onClick method for the copyButton is defined in the FrameBuffer constructor in the PluginEditor.cpp file.
	repositionButtons();
	addAndMakeVisible(vsPdSwitchButton);
	addChildComponent(midSideSwitchButton);
	addAndMakeVisible(zoomInButton);
	addAndMakeVisible(zoomOutButton);
	addAndMakeVisible(zoomToQuadrant1Button);
	addAndMakeVisible(recenterButton);
	addAndMakeVisible(copyButton);
}

void Vectorscope::mouseDown(const MouseEvent&){
	centerXOnMouseDown = centerX;
	centerYOnMouseDown = centerY;
	uploadStateToDataBuffer();
}

void Vectorscope::mouseDrag(const MouseEvent& event){
	centerX = centerXOnMouseDown + event.getDistanceFromDragStartX() / 2.0f;
	centerY = centerYOnMouseDown + event.getDistanceFromDragStartY() / 2.0f;
	uploadStateToDataBuffer();
}

void Vectorscope::mouseWheelMove(const MouseEvent&, const MouseWheelDetails& wheel){
	zoomX += wheel.deltaY * 100.0f;
	zoomY += wheel.deltaY * 100.0f;
	uploadStateToDataBuffer();
}

void Vectorscope::paint(Graphics& g){
	g.fillAll(Colours::black);
	g.setColour(Colours::red);
	//The mid and side values are coming in normalized from -1.0 to 1.0.
	//We convert those values to 0.0 to 250.0 here.
	FloatVectorOperationsBase<float,size_t>::copyWithMultiply(sideValuesTransformed.data(),sideValues.data(),zoomX,736);
	FloatVectorOperationsBase<float,size_t>::add(sideValuesTransformed.data(),centerX,736);
	FloatVectorOperationsBase<float,size_t>::multiply(sideValuesTransformed.data(),widthScaling,736);
	FloatVectorOperationsBase<float,size_t>::copyWithMultiply(midValuesTransformed.data(),midValues.data(),zoomY,736);
	FloatVectorOperationsBase<float,size_t>::add(midValuesTransformed.data(),centerY,736);
	FloatVectorOperationsBase<float,size_t>::multiply(midValuesTransformed.data(),heightScaling,736);
	if(!probabilityDistributionMode){
		for(unsigned int i = 0; i < 736; i++){
			//This fillEllipse function hits the CPU pretty hard, so this balances graph density for CPU performance.
			if(i % 2 == 0){
				g.fillEllipse(sideValuesTransformed[i],midValuesTransformed[i],3.75f,3.75f);
			}
		}
	}else{
		int zoneCountIp1 = 16;//zone count as an integer plus 1
		float zoneCountF = 15.0f;//zone count as a float
		std::vector<int> sampleCounts(zoneCountIp1,0);
		int width = getWidth();
		int height = getHeight();
		if(probabilityDistributionMidOrSide){
			float zoneHeight = height / zoneCountF;
			for(unsigned int i = 0; i < 736; i++){
				float midValueScoped = midValuesTransformed[i];
				if(midValueScoped > height || midValueScoped < 0.0) continue;
				int zone = int(midValueScoped / zoneHeight);
				sampleCounts[zone]++;
			}
			int countOfAcceptedSamples = 0;
			for(int zone=0;zone<zoneCountIp1;zone++){
				countOfAcceptedSamples += sampleCounts[zone];
			}
			for(int zone=0;zone<zoneCountIp1;zone++){
				if(countOfAcceptedSamples > 0){
					g.fillRoundedRectangle(0,zone*zoneHeight,width*(sampleCounts[zone]/float(countOfAcceptedSamples)),zoneHeight,3.0f);
				}
			}
		}else{
			float zoneWidth = width / zoneCountF;
			for(unsigned int i = 0; i < 736; i++){
				float sideValueScoped = sideValuesTransformed[i];
				if(sideValueScoped > width || sideValueScoped < 0.0) continue;
				int zone = int(sideValueScoped / zoneWidth);
				sampleCounts[zone]++;
			}
			int countOfAcceptedSamples = 0;
			for(int zone=0;zone<zoneCountIp1;zone++){
				countOfAcceptedSamples += sampleCounts[zone];
			}
			for(int zone=0;zone<zoneCountIp1;zone++){
				if(countOfAcceptedSamples > 0){
					float barHeight = height*(sampleCounts[zone]/float(countOfAcceptedSamples));
					g.fillRoundedRectangle(zone*zoneWidth,height-barHeight,zoneWidth,barHeight,3.0f);
				}
			}
		}
	}
}

void Vectorscope::resized(){
	widthScaling = (float)getWidth() / 250.0f;
	heightScaling = (float)getHeight() / 250.0f;
	repositionButtons();
}

void Vectorscope::repositionButtons(){
	int width = getWidth();
	int height = getHeight();
	//top left, left to right
	vsPdSwitchButton.setBounds(buttonSize*1,12,12,12);
	midSideSwitchButton.setBounds(buttonSize*3,12,12,12);
	//top right, left to right
	zoomInButton.setBounds(width - (buttonSize*8),12,12,12);
	zoomOutButton.setBounds(width - (buttonSize*6),12,12,12);
	zoomToQuadrant1Button.setBounds(width - (buttonSize*4),12,12,12);
	recenterButton.setBounds(width - (buttonSize*2),12,12,12);
	//bottom right
	copyButton.setBounds(width - (buttonSize*2),height - (buttonSize*2),12,12);
}

void Vectorscope::loadState(VectorscopeState state){
	centerX = state.centerX;
	centerXOnMouseDown = state.centerXOnMouseDown;
	centerY = state.centerY;
	centerYOnMouseDown = state.centerYOnMouseDown;
	probabilityDistributionMode = state.probabilityDistributionMode;
	if(probabilityDistributionMode){
		vsPdSwitchButton.setImages(true,true,true,scatterPlotImage,0.70f,Colours::transparentBlack,scatterPlotImage,1.00f,Colours::transparentBlack,scatterPlotImage,1.00f,Colours::transparentBlack);
		vsPdSwitchButton.setBounds(vsPdSwitchButton.getX(),vsPdSwitchButton.getY(),buttonSize,buttonSize);
		midSideSwitchButton.setVisible(true);
	}else{
		vsPdSwitchButton.setImages(true,true,true,barChartImage,0.70f,Colours::transparentBlack,barChartImage,1.00f,Colours::transparentBlack,barChartImage,1.00f,Colours::transparentBlack);
		vsPdSwitchButton.setBounds(vsPdSwitchButton.getX(),vsPdSwitchButton.getY(),buttonSize,buttonSize);
		midSideSwitchButton.setVisible(false);
	}
	probabilityDistributionMidOrSide = state.probabilityDistributionMidOrSide;
	zoomX = state.zoomX;
	zoomY = state.zoomY;
}

VectorscopeState Vectorscope::saveState(){
	return VectorscopeState {
		centerX,
		centerXOnMouseDown,
		centerY,
		centerYOnMouseDown,
		probabilityDistributionMode,
		probabilityDistributionMidOrSide,
		zoomX,
		zoomY
	};
}

void Vectorscope::downloadStateFromDataBuffer(){
	int currentTab = dataBuffer->currentTabIndex.get();
	VectorscopeStateAtomized vsa;
	if(leftOrRight){
		vsa = dataBuffer->leftVscopeStates[currentTab];
	}else{
		vsa = dataBuffer->rightVscopeStates[currentTab];
	}
	VectorscopeState newState = {
		vsa.centerX.get(),
		vsa.centerXOnMouseDown.get(),
		vsa.centerY.get(),
		vsa.centerYOnMouseDown.get(),
		vsa.probabilityDistributionMode.get(),
		vsa.probabilityDistributionMidOrSide.get(),
		vsa.zoomX.get(),
		vsa.zoomY.get()
	};
	loadState(newState);
}

void Vectorscope::uploadStateToDataBuffer(){
	int currentTab = dataBuffer->currentTabIndex.get();
	VectorscopeStateAtomized* vsa;
	if(leftOrRight){
		vsa = &dataBuffer->leftVscopeStates[currentTab];
	}else{
		vsa = &dataBuffer->rightVscopeStates[currentTab];
	}
	VectorscopeState currentState = this->saveState();
	vsa->centerX.set(currentState.centerX);
	vsa->centerXOnMouseDown.set(currentState.centerXOnMouseDown);
	vsa->centerY.set(currentState.centerY);
	vsa->centerYOnMouseDown.set(currentState.centerYOnMouseDown);
	vsa->probabilityDistributionMode.set(currentState.probabilityDistributionMode);
	vsa->probabilityDistributionMidOrSide.set(currentState.probabilityDistributionMidOrSide);
	vsa->zoomX.set(currentState.zoomX);
	vsa->zoomY.set(currentState.zoomY);
}