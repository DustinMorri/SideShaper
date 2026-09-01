#include "PluginEditor.h"

LightUpButton::LightUpButton(Image enabledImage, Image disabledImage) : ImageButton(){
	this->enabledImage = enabledImage;
	this->disabledImage = disabledImage;
}

void LightUpButton::mouseDown(const juce::MouseEvent& event){
	if(event.mods == ModifierKeys::rightButtonModifier){
		onRightClick();
	}else{
		ImageButton::mouseDown(event);
	}
}

void LightUpButton::mouseUp(const MouseEvent &event){
	if(event.mods != ModifierKeys::rightButtonModifier){
		ImageButton::mouseUp(event);
	}
}

void LightUpButton::toggleTo(bool value){
	if(value){
		setImages(false,true,true,enabledImage,1.0f,Colours::transparentBlack,enabledImage,1.0f,Colours::transparentBlack,enabledImage,1.0f,Colours::transparentBlack);
	}else{
		setImages(false,true,true,disabledImage,1.0f,Colours::transparentBlack,disabledImage,1.0f,Colours::transparentBlack,disabledImage,1.0f,Colours::transparentBlack);
	}
}

LightUpButton::~LightUpButton(){
	
}

LinearParameterSlider::LinearParameterSlider(void* fb, Slider::SliderStyle sliderStyle, int sliderIndex) : Slider(sliderStyle,Slider::TextEntryBoxPosition::NoTextBox){
	frameBuffer = fb;
	this->sliderIndex = sliderIndex;
	setRange(0.0,1.0);
	setDoubleClickReturnValue(true, 0.5);
}

void LinearParameterSlider::mouseDown(const juce::MouseEvent& event){
	if(event.mods.isPopupMenu()){
		juce::PopupMenu menu;
		menu.addItem(1, "Copy");
		menu.addItem(2, "Paste");
		menu.addItem(3, "Paste Inverse");
		if(sliderIndex == SliderName::MidSliderInflection){
			menu.addItem(4, "Copy All From Side");
			menu.addItem(5, "Reset All");
		}else if(sliderIndex == SliderName::SideSliderInflection){
			menu.addItem(4, "Copy All From Mid");
			menu.addItem(5, "Reset All");
		}
		menu.showMenuAsync (juce::PopupMenu::Options(),[this](int result){
			FrameBuffer* fb = reinterpret_cast<FrameBuffer*>(frameBuffer);
			int currentTabIndex = dataBuffer->currentTabIndex.get();
			switch(result){
				case 1:
					fb->clipboardValue = getValue();
					break;
				case 2:
					setValue(fb->clipboardValue);
					break;
				case 3:
					setValue(static_cast<double>(1.0) - fb->clipboardValue);
					break;
				case 4:
					if(sliderIndex == SliderName::MidSliderInflection){
						fb->midSliderInflection.setValue(fb->sideSliderInflection.getValue(),NotificationType::sendNotificationSync);
						//To make sure that there will be no error in logic with setting the mid value, temporarily reset the min and max to 0.0 and 1.0.
						fb->midSliderMain.setMinAndMaxValues(0.0,1.0,NotificationType::dontSendNotification);
						fb->midSliderMain.setValue(fb->sideSliderMain.getValue(),NotificationType::dontSendNotification);
						fb->midSliderMain.setMinAndMaxValues(fb->sideSliderMain.getMinValue(),fb->sideSliderMain.getMaxValue(),NotificationType::sendNotificationSync);
						fb->midSliderZone1.setValue(fb->sideSliderZone1.getValue(),NotificationType::sendNotificationSync);
						fb->midSliderZone2.setValue(fb->sideSliderZone2.getValue(),NotificationType::sendNotificationSync);
						bool newToggleState = fb->gateBeforeStartSide.getToggleState();
						fb->gateBeforeStartMid.setToggleState(newToggleState,NotificationType::dontSendNotification);//sets the button toggle state
						fb->gateBeforeStartMid.toggleTo(newToggleState);//toggles the images
						dataBuffer->gateBeforeStart[currentTabIndex][0].set(newToggleState);//updates the data buffer
						dataBuffer->gateBeforeStartChangesNeedingToBeRecognizedByProcessor[currentTabIndex][0].set(true);//ChangesNeedingToBeRecognizedByProcessor
						newToggleState = fb->flatAfterEndSide.getToggleState();
						fb->flatAfterEndMid.setToggleState(newToggleState,NotificationType::dontSendNotification);
						fb->flatAfterEndMid.toggleTo(newToggleState);
						dataBuffer->flatAfterEnd[currentTabIndex][0].set(newToggleState);
						dataBuffer->flatAfterEndChangesNeedingToBeRecognizedByProcessor[currentTabIndex][0].set(true);
						fb->alertInOutGrid(0);
					}else if(sliderIndex == SliderName::SideSliderInflection){
						fb->sideSliderInflection.setValue(fb->midSliderInflection.getValue(),NotificationType::sendNotificationSync);
						fb->sideSliderMain.setMinAndMaxValues(0.0,1.0,NotificationType::dontSendNotification);
						fb->sideSliderMain.setValue(fb->midSliderMain.getValue(),NotificationType::dontSendNotification);
						fb->sideSliderMain.setMinAndMaxValues(fb->midSliderMain.getMinValue(),fb->midSliderMain.getMaxValue(),NotificationType::sendNotificationSync);
						fb->sideSliderZone1.setValue(fb->midSliderZone1.getValue(),NotificationType::sendNotificationSync);
						fb->sideSliderZone2.setValue(fb->midSliderZone2.getValue(),NotificationType::sendNotificationSync);
						bool newToggleState = fb->gateBeforeStartMid.getToggleState();
						fb->gateBeforeStartSide.setToggleState(newToggleState,NotificationType::dontSendNotification);
						fb->gateBeforeStartSide.toggleTo(newToggleState);
						dataBuffer->gateBeforeStart[currentTabIndex][1].set(newToggleState);
						dataBuffer->gateBeforeStartChangesNeedingToBeRecognizedByProcessor[currentTabIndex][1].set(true);
						newToggleState = fb->flatAfterEndMid.getToggleState();
						fb->flatAfterEndSide.setToggleState(newToggleState,NotificationType::dontSendNotification);
						fb->flatAfterEndSide.toggleTo(newToggleState);
						dataBuffer->flatAfterEnd[currentTabIndex][1].set(newToggleState);
						dataBuffer->flatAfterEndChangesNeedingToBeRecognizedByProcessor[currentTabIndex][1].set(true);
						fb->alertInOutGrid(1);
					}
					break;
				case 5:
					if(sliderIndex == SliderName::MidSliderInflection){
						fb->midSliderInflection.setValue(0.5,NotificationType::sendNotificationSync);
						fb->midSliderMain.setMinAndMaxValues(0.0,1.0,NotificationType::dontSendNotification);
						fb->midSliderMain.setValue(0.5,NotificationType::sendNotificationSync);
						fb->midSliderZone1.setValue(0.5,NotificationType::sendNotificationSync);
						fb->midSliderZone2.setValue(0.5,NotificationType::sendNotificationSync);
						fb->gateBeforeStartMid.setToggleState(false,NotificationType::dontSendNotification);
						fb->gateBeforeStartMid.toggleTo(false);
						dataBuffer->gateBeforeStart[currentTabIndex][0].set(false);
						fb->flatAfterEndMid.setToggleState(false,NotificationType::dontSendNotification);
						fb->flatAfterEndMid.toggleTo(false);
						dataBuffer->flatAfterEnd[currentTabIndex][0].set(false);
						fb->alertInOutGrid(0);
					}else if(sliderIndex == SliderName::SideSliderInflection){
						fb->sideSliderInflection.setValue(0.5,NotificationType::sendNotificationSync);
						fb->sideSliderMain.setMinAndMaxValues(0.0,1.0,NotificationType::dontSendNotification);
						fb->sideSliderMain.setValue(0.5,NotificationType::sendNotificationSync);
						fb->sideSliderZone1.setValue(0.5,NotificationType::sendNotificationSync);
						fb->sideSliderZone2.setValue(0.5,NotificationType::sendNotificationSync);
						fb->gateBeforeStartSide.setToggleState(false,NotificationType::dontSendNotification);
						fb->gateBeforeStartSide.toggleTo(false);
						dataBuffer->gateBeforeStart[currentTabIndex][1].set(false);
						fb->flatAfterEndSide.setToggleState(false,NotificationType::dontSendNotification);
						fb->flatAfterEndSide.toggleTo(false);
						dataBuffer->flatAfterEnd[currentTabIndex][1].set(false);
						fb->alertInOutGrid(1);
					}
					break;
			}
		});
	}else{
		Slider::mouseDown(event);
	}
}

void LinearParameterSlider::connectToDataBuffer(DataBuffer* db){
	dataBuffer = db;
	sliderData = db->sliderData[sliderIndex];
}

void LinearParameterSlider::valueChanged(){
	double newValue = getValue();
	sliderData->atomicMinValue.set(newValue);
	sliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(true);
	FrameBuffer* fb = reinterpret_cast<FrameBuffer*>(frameBuffer);
	if(sliderIndex == SliderName::UpperFrequencySlider || sliderIndex == SliderName::LowerFrequencySlider){
		fb->updateCrossoverLabels(dataBuffer->currentTabIndex.get());
	}else{
		double newValues[3] = {newValue,0.0f,0.0f};
		fb->alertInOutGrid(sliderIndex,newValues);
	}
}

LinearParameterSlider::~LinearParameterSlider(){

}

ThreeValueParameterSlider::ThreeValueParameterSlider(void* fb, Slider::SliderStyle sliderStyle, int sliderIndex) : Slider(sliderStyle,Slider::TextEntryBoxPosition::NoTextBox){
	frameBuffer = fb;
	this->sliderIndex = sliderIndex;
	setRange(0.0,1.0);
	setDoubleClickReturnValue(true, 0.5);
}

void ThreeValueParameterSlider::mouseDown(const juce::MouseEvent& event){
	if(event.mods.isPopupMenu()){
		juce::PopupMenu menu;
		menu.addItem(1, "Copy End Distance");
		menu.addItem(2, "Copy Zone 1 Percentage");
		menu.addItem(3, "Copy Start Distance");
		menu.addItem(4, "Paste End Distance");
		menu.addItem(5, "Paste Zone 1 Percentage");
		menu.addItem(6, "Paste Start Distance");
		menu.showMenuAsync (juce::PopupMenu::Options(),[this](int result){
			FrameBuffer* fb = reinterpret_cast<FrameBuffer*>(frameBuffer);
			switch(result){
				case 1:
					fb->clipboardValue = getMaxValue();
					break;
				case 2:
					fb->clipboardValue = getValue();
					break;
				case 3:
					fb->clipboardValue = getMinValue();
					break;
				case 4:
					setMaxValue(fb->clipboardValue);
					break;
				case 5:
					setValue(fb->clipboardValue);
					break;
				case 6:
					setMinValue(fb->clipboardValue);
					break;
			}
		});
	}else{
		Slider::mouseDown(event);
	}
}

void ThreeValueParameterSlider::connectToDataBuffer(DataBuffer* db){
	dataBuffer = db;
	sliderData = db->sliderData[sliderIndex];
}

void ThreeValueParameterSlider::valueChanged(){
	double newMinValue = getMinValue();
	double newMidValue = getValue();
	double newMaxValue = getMaxValue();
	sliderData->atomicMinValue.set(newMinValue);//1
	sliderData->atomicMidValue.set(newMidValue);//3
	sliderData->atomicMaxValue.set(newMaxValue);//5
	if(newMinValue != sliderData->prevMinValue){
		sliderData->hasChangeNeedingToBeRecognizedByProcessorMin.set(true);
		setDoubleClickReturnValue(true, (newMaxValue - newMinValue) / 2.0 + newMinValue);
	}
	if(newMidValue != sliderData->prevMidValue){
		sliderData->hasChangeNeedingToBeRecognizedByProcessorMid.set(true);
	}
	if(newMaxValue != sliderData->prevMaxValue){
		sliderData->hasChangeNeedingToBeRecognizedByProcessorMax.set(true);
		setDoubleClickReturnValue(true, (newMaxValue - newMinValue) / 2.0 + newMinValue);
	}
	sliderData->prevMinValue = newMinValue;
	sliderData->prevMidValue = newMidValue;
	sliderData->prevMaxValue = newMaxValue;
	FrameBuffer* fb = reinterpret_cast<FrameBuffer*>(frameBuffer);
	double newValues[3] = {newMinValue,newMaxValue,newMidValue};
	fb->alertInOutGrid(sliderIndex,newValues);
}

ThreeValueParameterSlider::~ThreeValueParameterSlider(){

}

TabButton::TabButton(int tabIndex, String text) : TextButton(text){
	this->tabIndex = tabIndex;
}

TabButton::~TabButton(){
	
}

FrameBuffer::FrameBuffer(DataBuffer* db) : leftVscope(Vectorscope(db,false)), rightVscope(Vectorscope(db,true)){
	dataBuffer = db;

	//Make the single splitter mode frequency slider easier to work with.
	oneSplitFrequencyRange.setSkewForCentre(2205.0f);

	//If the computer monitor runs at 60 frames per second, call the update function 60 times per second.
	setSynchroniseToVBlank(true);

	//Set the size of the frame buffer to match the size of the plugin editor.
	int initAppWidth = dataBuffer->appWidth.get();
	int initAppHeight = dataBuffer->appHeight.get();
	setSize(initAppWidth,initAppHeight);
	
	//Add the vectorscopes.
	leftVscope.setTopLeftPosition(0,0);
	addAndMakeVisible(leftVscope);
	addAndMakeVisible(rightVscope);

	//Establish what the vectorscope copy buttons do.
	leftVscope.copyButton.onClick = [this](){
		leftVscope.loadState(rightVscope.saveState());
	};
	rightVscope.copyButton.onClick = [this](){
		rightVscope.loadState(leftVscope.saveState());
	};

	//Mid Stereo Shaper Slider Section
	midGrid.alignContent = Grid::AlignContent::center;
	midGrid.alignItems = Grid::AlignItems::center;
	midGrid.justifyContent = Grid::JustifyContent::center;
	midGrid.justifyItems = Grid::JustifyItems::center;
	midGrid.templateColumns = {Grid::TrackInfo(5_px), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(5_px)};
	midGrid.templateRows = {Grid::TrackInfo(5_px), Grid::TrackInfo(8_fr), Grid::TrackInfo(8_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(5_px)};
	midGrid.columnGap = 5_px;
	/*
	123456 - padding row
	2 - padding column, inflection,         main,      zone 2,    flatAfterEnd, padding column
	3 - padding column, inflection,         main,      zone 1, gateBeforeStart, padding column
	4 - padding column, solo button, solo button, solo button,     solo button, padding column
	5 - padding row
	*/
	midGrid.items.add(GridItem().withArea(1,1,2,7));//A blank 5px row on the top. The first number is inclusive. The second number is exclusive.
	midGrid.items.add(GridItem().withArea(5,1,6,7));//A blank 5px row on the bottom.
	midGrid.items.add(GridItem().withArea(2,1,5,2));//A blank 5px column on the left between the top and bottom.
	midGrid.items.add(GridItem().withArea(2,6,5,7));//A blank 5px column on the right between the top and bottom.

	midSliderInflection.connectToDataBuffer(db);
	//midSliderInflection.setValue(0.5);
	midGrid.items.add(GridItem(midSliderInflection).withArea(2,2,4,3));//.withMargin(GridItem::Margin(0.0f,10.0f,0.0f,10.0f))

	midSliderMain.connectToDataBuffer(db);
	//midSliderMain.setMinAndMaxValues(0.0,1.0);
	//midSliderMain.setValue(0.5);
	midGrid.items.add(GridItem(midSliderMain).withArea(2,3,4,4));

	midSliderZone2.connectToDataBuffer(db);
	//midSliderZone2.setValue(0.5);
	midGrid.items.add(GridItem(midSliderZone2).withArea(2,4,3,5));

	midSliderZone1.connectToDataBuffer(db);
	//midSliderZone1.setValue(0.5);
	midGrid.items.add(GridItem(midSliderZone1).withArea(3,4,4,5));

	flatAfterEndMid.setToggleState(false,NotificationType::dontSendNotification);
	//flatAfterEndMid.toggleTo(false);
	flatAfterEndMid.onClick = [this](){
		int currentTab = dataBuffer->currentTabIndex.get();
		bool newToggleState = !flatAfterEndMid.getToggleState();
		midSideShaper.setFlatAfterEnd(newToggleState);
		dataBuffer->flatAfterEnd[currentTab][0].set(newToggleState);
		dataBuffer->flatAfterEndChangesNeedingToBeRecognizedByProcessor[currentTab][0].set(true);
		flatAfterEndMid.setToggleState(newToggleState,NotificationType::dontSendNotification);
		flatAfterEndMid.toggleTo(newToggleState);
	};
	midGrid.items.add(GridItem(flatAfterEndMid).withArea(2,5,3,6));

	gateBeforeStartMid.setToggleState(false,NotificationType::dontSendNotification);
	//gateBeforeStartMid.toggleTo(false);
	gateBeforeStartMid.onClick = [this](){
		int currentTab = dataBuffer->currentTabIndex.get();
		bool newToggleState = !gateBeforeStartMid.getToggleState();
		midSideShaper.setGateBeforeStart(newToggleState);
		dataBuffer->gateBeforeStart[currentTab][0].set(newToggleState);
		dataBuffer->gateBeforeStartChangesNeedingToBeRecognizedByProcessor[currentTab][0].set(true);
		gateBeforeStartMid.setToggleState(newToggleState,NotificationType::dontSendNotification);
		gateBeforeStartMid.toggleTo(newToggleState);
	};
	midGrid.items.add(GridItem(gateBeforeStartMid).withArea(3,5,4,6));

	soloButton.setToggleState(true,NotificationType::dontSendNotification);
	//soloButton.toggleTo(true);
	soloButton.onClick = [this](){
		int currentTab = dataBuffer->currentTabIndex.get();
		bool newToggleState = !soloButton.getToggleState();
		//if the new toggle state is solo enabled (aka true)
		if(newToggleState){//left click going from false to true
			soloButton.setToggleState(newToggleState,NotificationType::dontSendNotification);
			soloButton.toggleTo(newToggleState);
			dataBuffer->solo[currentTab].set(true);
			switch(currentTab){
				case TabName::Pre:
					dataBuffer->solo[TabName::Low].set(false);
					dataBuffer->solo[TabName::MidLow].set(false);
					dataBuffer->solo[TabName::MidHigh].set(false);
					dataBuffer->solo[TabName::High].set(false);
					dataBuffer->solo[TabName::Post].set(false);
					break;
				case TabName::Low:
				case TabName::MidLow:
				case TabName::MidHigh:
				case TabName::High:
					dataBuffer->solo[TabName::Pre].set(false);
					dataBuffer->solo[TabName::Post].set(false);
					break;
				case TabName::Post:
					dataBuffer->solo[TabName::Pre].set(false);
					dataBuffer->solo[TabName::Low].set(false);
					dataBuffer->solo[TabName::MidLow].set(false);
					dataBuffer->solo[TabName::MidHigh].set(false);
					dataBuffer->solo[TabName::High].set(false);
					break;
			}
		}else{//left click going from true to false
			//do not allow the post solo to be disabled
			if(currentTab != TabName::Post){
				soloButton.setToggleState(newToggleState,NotificationType::dontSendNotification);
				soloButton.toggleTo(newToggleState);
				dataBuffer->solo[currentTab].set(false);
				//If all solos are now false, reset the post solo to true.
				bool anyTrue = false;
				for(int i=0;i<dataBuffer->solo.size();i++){
					anyTrue |= dataBuffer->solo[i].get();
				}
				if(!anyTrue){
					dataBuffer->solo[TabName::Post].set(true);
				}
			}
		}
	};
	soloButton.onRightClick = [this](){
		int currentTab = dataBuffer->currentTabIndex.get();
		bool newToggleState = !soloButton.getToggleState();
		//if the new toggle state is solo enabled (aka true)
		if(newToggleState){
			soloButton.setToggleState(newToggleState,NotificationType::dontSendNotification);
			soloButton.toggleTo(newToggleState);
			dataBuffer->solo[currentTab].set(true);
			for(int i=0;i<dataBuffer->solo.size();i++){
				if(i == currentTab) continue;
				dataBuffer->solo[i].set(false);
			}
		}else{//right click going from true to false
			//Set all solos to false except the post and don't do anything if the user right clicks on the soloed post LightUpButton.
			if(currentTab != TabName::Post){
				soloButton.setToggleState(newToggleState,NotificationType::dontSendNotification);
				soloButton.toggleTo(newToggleState);
				dataBuffer->solo[TabName::Post].set(true);
				for(int i=0;i<dataBuffer->solo.size();i++){
					if(i == TabName::Post) continue;
					dataBuffer->solo[i].set(false);
				}
			}
		}
	};
	midGrid.items.add(GridItem(soloButton).withArea(4,2,5,6));

	addAndMakeVisible(midSliderMain);
	addAndMakeVisible(midSliderInflection);
	addAndMakeVisible(midSliderZone2);
	addAndMakeVisible(midSliderZone1);
	addAndMakeVisible(gateBeforeStartMid);
	addAndMakeVisible(flatAfterEndMid);
	addAndMakeVisible(soloButton);
	
	//Side Slider Section
	sideGrid.alignContent = Grid::AlignContent::center;
	sideGrid.alignItems = Grid::AlignItems::center;
	sideGrid.justifyContent = Grid::JustifyContent::center;
	sideGrid.justifyItems = Grid::JustifyItems::center;
	sideGrid.templateColumns = {Grid::TrackInfo(5_px), Grid::TrackInfo(1_fr), Grid::TrackInfo(8_fr), Grid::TrackInfo(8_fr), Grid::TrackInfo(5_px)};
	sideGrid.templateRows = {Grid::TrackInfo(5_px), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(5_px)};
	sideGrid.rowGap = 5_px;
	/*
	12345 - padding row
	2 - padding column, beforeOrAfter,      inflection,   inflection, padding column
	3 - padding column, beforeOrAfter,            main,         main, padding column
	4 - padding column, beforeOrAfter,          zone 1,       zone 2, padding column
	5 - padding column, beforeOrAfter, gateBeforeStart, flatAfterEnd, padding column
	6 - padding row
	*/
	sideGrid.items.add(GridItem().withArea(1,1,2,6));//A blank 5px row on the top. The first number is inclusive. The second number is exclusive.
	sideGrid.items.add(GridItem().withArea(6,1,7,6));//A blank 5px row on the bottom.
	sideGrid.items.add(GridItem().withArea(2,1,6,2));//A blank 5px column on the left between the top and bottom.
	sideGrid.items.add(GridItem().withArea(2,5,6,6));//A blank 5px column on the right between the top and bottom.

	beforeOrAfterButton.setToggleState(true,NotificationType::dontSendNotification);
	//beforeOrAfterButton.toggleTo(true);
	beforeOrAfterButton.onClick = [this](){
		int currentTab = dataBuffer->currentTabIndex.get();
		bool newToggleState = !dataBuffer->soloBeforeOrAfter[currentTab].get();
		dataBuffer->soloBeforeOrAfter[currentTab].set(newToggleState);
		beforeOrAfterButton.setToggleState(newToggleState,NotificationType::dontSendNotification);
		beforeOrAfterButton.toggleTo(newToggleState);
	};
	sideGrid.items.add(GridItem(beforeOrAfterButton).withArea(2,2,6,3));
	
	sideSliderInflection.connectToDataBuffer(db);
	//sideSliderInflection.setValue(0.5);
	sideGrid.items.add(GridItem(sideSliderInflection).withArea(2,3,3,5));

	sideSliderMain.connectToDataBuffer(db);
	//sideSliderMain.setMinAndMaxValues(0.0,1.0);
	//sideSliderMain.setValue(0.5);
	sideGrid.items.add(GridItem(sideSliderMain).withArea(3,3,4,5));
	
	sideSliderZone1.connectToDataBuffer(db);
	//sideSliderZone1.setValue(0.5);
	sideGrid.items.add(GridItem(sideSliderZone1).withArea(4,3,5,4));
	
	sideSliderZone2.connectToDataBuffer(db);
	//sideSliderZone2.setValue(0.5);
	sideGrid.items.add(GridItem(sideSliderZone2).withArea(4,4,5,5));

	gateBeforeStartSide.setToggleState(false,NotificationType::dontSendNotification);
	//gateBeforeStartSide.toggleTo(false);
	gateBeforeStartSide.onClick = [this](){
		int currentTab = dataBuffer->currentTabIndex.get();
		bool newToggleState = !gateBeforeStartSide.getToggleState();
		sideSideShaper.setGateBeforeStart(newToggleState);
		dataBuffer->gateBeforeStart[currentTab][1].set(newToggleState);
		dataBuffer->gateBeforeStartChangesNeedingToBeRecognizedByProcessor[currentTab][1].set(true);
		gateBeforeStartSide.setToggleState(newToggleState,NotificationType::dontSendNotification);
		gateBeforeStartSide.toggleTo(newToggleState);
	};
	sideGrid.items.add(GridItem(gateBeforeStartSide).withArea(5,3,6,4));
	
	flatAfterEndSide.setToggleState(false,NotificationType::dontSendNotification);
	//flatAfterEndSide.toggleTo(false);
	flatAfterEndSide.onClick = [this](){
		int currentTab = dataBuffer->currentTabIndex.get();
		bool newToggleState = !flatAfterEndSide.getToggleState();
		sideSideShaper.setFlatAfterEnd(newToggleState);
		dataBuffer->flatAfterEnd[currentTab][1].set(newToggleState);
		dataBuffer->flatAfterEndChangesNeedingToBeRecognizedByProcessor[currentTab][1].set(true);
		flatAfterEndSide.setToggleState(newToggleState,NotificationType::dontSendNotification);
		flatAfterEndSide.toggleTo(newToggleState);
	};
	sideGrid.items.add(GridItem(flatAfterEndSide).withArea(5,4,6,5));

	addAndMakeVisible(beforeOrAfterButton);
	addAndMakeVisible(sideSliderInflection);
	addAndMakeVisible(sideSliderMain);
	addAndMakeVisible(sideSliderZone1);
	addAndMakeVisible(sideSliderZone2);
	addAndMakeVisible(gateBeforeStartSide);
	addAndMakeVisible(flatAfterEndSide);

	//Lower Left Side
	leftSideGrid.alignContent = Grid::AlignContent::center;
	leftSideGrid.alignItems = Grid::AlignItems::center;
	leftSideGrid.justifyContent = Grid::JustifyContent::center;
	leftSideGrid.justifyItems = Grid::JustifyItems::center;
	leftSideGrid.templateColumns = {Grid::TrackInfo(10_px), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(5_px)};
	leftSideGrid.templateRows = {Grid::TrackInfo(5_px), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(5_px)};
	/*
	Pre
		1234 - padding row
		2 - padding column, internalBypassLabel, internalBypassButton, padding column
		3 - padding column, splitterBypassLabel, splitterBypassButton, padding column
		4 - padding row
	Low
		1234 - padding row
		2 - padding column, oneSplitterLabel, oneSplitterButton, padding column
		3 - padding column, lowerFrequencySlider, lowerFrequencyLabel, padding column
		4 - padding row
	Mid Low & Mid High
		1234 - padding row
		2 - padding column, upperFrequencySlider, upperFrequencyLabel, padding column
		3 - padding column, lowerFrequencySlider, lowerFrequencyLabel, padding column
		4 - padding row
	High
		1234 - padding row
		2 - padding column,        upperFrequencySlider,          upperFrequencyLabel, padding column
		3 - padding column, fullFrequencyCrossoverLabel, fullFrequencyCrossoverButton, padding column
		4 - padding row
	Post
		1234 - padding row
		2 - padding column, settingsButton, settingsButton, padding column
		3 - padding column,    aboutButton,    aboutButton, padding column
		4 - padding row
	*/
	leftSideGrid.items.add(GridItem().withArea(1,1,2,5));//A blank 5px row on the top.
	leftSideGrid.items.add(GridItem().withArea(4,1,5,5));//A blank 5px row on the bottom.
	leftSideGrid.items.add(GridItem().withArea(2,1,4,2));//A blank 5px column on the left between the top and bottom.
	leftSideGrid.items.add(GridItem().withArea(2,4,4,5));//A blank 5px column on the right between the top and bottom.

	leftSideGrid.items.add(GridItem(internalBypassLabel).withArea(2,2,3,3));
	internalBypassButton.onClick = [this](){
		dataBuffer->bypassAll.set(internalBypassButton.getToggleState());
	};
	leftSideGrid.items.add(GridItem(internalBypassButton).withArea(2,3,3,4));

	leftSideGrid.items.add(GridItem(splitterBypassLabel).withArea(3,2,4,3));
	splitterBypassButton.onClick = [this](){
		dataBuffer->bypassSplitters.set(splitterBypassButton.getToggleState());
		determineTabButtonText();
	};
	leftSideGrid.items.add(GridItem(splitterBypassButton).withArea(3,3,4,4));
	
	addAndMakeVisible(internalBypassLabel);
	addAndMakeVisible(internalBypassButton);
	addAndMakeVisible(splitterBypassLabel);
	addAndMakeVisible(splitterBypassButton);

	oneSplitterButton.onClick = [this](){
		bool newState = oneSplitterButton.getToggleState();
		dataBuffer->oneSplit.set(newState);
		determineTabButtonText();
		dataBuffer->oneSplitChangeNeedingToBeRecognizedByProcessor.set(true);
		updateCrossoverLabels(dataBuffer->currentTabIndex.get());
	};

	addChildComponent(oneSplitterLabel);
	addChildComponent(oneSplitterButton);

	upperFrequencyLabel.setJustificationType(Justification::centred);
	upperFrequencySlider.connectToDataBuffer(db);
	//upperFrequencySlider.setValue(0.5);

	lowerFrequencyLabel.setJustificationType(Justification::centred);
	lowerFrequencySlider.connectToDataBuffer(db);
	//lowerFrequencySlider.setValue(0.5);
	
	addChildComponent(upperFrequencyLabel);
	addChildComponent(upperFrequencySlider);
	addChildComponent(lowerFrequencyLabel);
	addChildComponent(lowerFrequencySlider);

	fullFrequencyCrossoverButton.onClick = [this](){
		dataBuffer->fullFrequencyCrossoverRanges.set(fullFrequencyCrossoverButton.getToggleState());
		dataBuffer->fullFrequencyCrossoverRangesNeedingToBeRecognizedByProcessor.set(true);
		updateCrossoverLabels(dataBuffer->currentTabIndex.get());
	};

	addChildComponent(fullFrequencyCrossoverLabel);
	addChildComponent(fullFrequencyCrossoverButton);
	
	settingsButton.onClick = [this](){
		tabChange(TabName::Settings);
	};
	addChildComponent(settingsButton);

	aboutButton.onClick = [this](){
		tabChange(TabName::About);
	};
	addChildComponent(aboutButton);

	midSideModeSettingButton.onClick = [this](){
		dataBuffer->leftRightMode.set(false);
		dataBuffer->leftRightModeChangeNeedingToBeRecognizedByProcessor.set(true);
	};
	settingButtonSetup(midSideModeSettingButton,nullptr,Button::ConnectedOnRight,35,RadioGroup::LeftRightMode);

	leftRightModeSettingButton.onClick = [this](){
		dataBuffer->leftRightMode.set(true);
		dataBuffer->leftRightModeChangeNeedingToBeRecognizedByProcessor.set(true);
	};
	settingButtonSetup(leftRightModeSettingButton,&midSideModeSettingButton,Button::ConnectedOnLeft,35,RadioGroup::LeftRightMode);

	unprotectedModeSettingButton.onClick = [this](){
		dataBuffer->protectedMode.set(false);
		dataBuffer->protectedModeChangeNeedingToBeRecognizedByProcessor.set(true);
	};
	settingButtonSetup(unprotectedModeSettingButton,nullptr,Button::ConnectedOnRight,70,RadioGroup::ProtectedMode);

	protectedModeSettingButton.onClick = [this](){
		dataBuffer->protectedMode.set(true);
		dataBuffer->protectedModeChangeNeedingToBeRecognizedByProcessor.set(true);
	};
	settingButtonSetup(protectedModeSettingButton,&unprotectedModeSettingButton,Button::ConnectedOnLeft,70,RadioGroup::ProtectedMode);

	shapeRangeStandardSettingButton.onClick = [this](){
		dataBuffer->shapeRangeSwapMode.set(false);
		dataBuffer->shapeRangeSwapModeChangeNeedingToBeRecognizedByProcessor.set(true);
	};
	settingButtonSetup(shapeRangeStandardSettingButton,nullptr,Button::ConnectedOnRight,105,RadioGroup::ShapeRangeSwapMode);

	shapeRangeSwappedSettingButton.onClick = [this](){
		dataBuffer->shapeRangeSwapMode.set(true);
		dataBuffer->shapeRangeSwapModeChangeNeedingToBeRecognizedByProcessor.set(true);
	};
	settingButtonSetup(shapeRangeSwappedSettingButton,&shapeRangeStandardSettingButton,Button::ConnectedOnLeft,105,RadioGroup::ShapeRangeSwapMode);

	semitoneCrossoverSettingButton.onClick = [this](){
		dataBuffer->crossoverMode.set(RolloffSize::Semitone);
		dataBuffer->crossoverModeChangeNeedingToBeRecognizedByProcessor.set(true);
	};
	settingButtonSetup(semitoneCrossoverSettingButton,nullptr,Button::ConnectedOnRight,140,RadioGroup::CrossoverMode);

	halfOctaveCrossoverSettingButton.onClick = [this](){
		dataBuffer->crossoverMode.set(RolloffSize::HalfOctave);
		dataBuffer->crossoverModeChangeNeedingToBeRecognizedByProcessor.set(true);
	};
	settingButtonSetup(halfOctaveCrossoverSettingButton,&semitoneCrossoverSettingButton,Button::ConnectedOnLeft | Button::ConnectedOnRight,140,RadioGroup::CrossoverMode);

	fullOctaveCrossoverSettingButton.onClick = [this](){
		dataBuffer->crossoverMode.set(RolloffSize::FullOctave);
		dataBuffer->crossoverModeChangeNeedingToBeRecognizedByProcessor.set(true);
	};
	settingButtonSetup(fullOctaveCrossoverSettingButton,&halfOctaveCrossoverSettingButton,Button::ConnectedOnLeft,140,RadioGroup::CrossoverMode);

	inputShapeNoneSettingButton.onClick = [this](){
		dataBuffer->inputShape.set(VectorscopeShape::None);
	};
	settingButtonSetup(inputShapeNoneSettingButton,nullptr,Button::ConnectedOnRight,175,RadioGroup::InputShape);

	inputShapeCircleSettingButton.onClick = [this](){
		dataBuffer->inputShape.set(VectorscopeShape::Circle);
	};
	settingButtonSetup(inputShapeCircleSettingButton,&inputShapeNoneSettingButton,Button::ConnectedOnLeft | Button::ConnectedOnRight,175,RadioGroup::InputShape);

	inputShapeDiamondSettingButton.onClick = [this](){
		dataBuffer->inputShape.set(VectorscopeShape::Diamond);
	};
	settingButtonSetup(inputShapeDiamondSettingButton,&inputShapeCircleSettingButton,Button::ConnectedOnLeft | Button::ConnectedOnRight,175,RadioGroup::InputShape);

	inputShapeSquareSettingButton.onClick = [this](){
		dataBuffer->inputShape.set(VectorscopeShape::Square);
	};
	settingButtonSetup(inputShapeSquareSettingButton,&inputShapeDiamondSettingButton,Button::ConnectedOnLeft,175,RadioGroup::InputShape);

	outputShapeNoneSettingButton.onClick = [this](){
		dataBuffer->outputShape.set(VectorscopeShape::None);
	};
	settingButtonSetup(outputShapeNoneSettingButton,nullptr,Button::ConnectedOnRight,210,RadioGroup::OutputShape);

	outputShapeCircleSettingButton.onClick = [this](){
		dataBuffer->outputShape.set(VectorscopeShape::Circle);
	};
	settingButtonSetup(outputShapeCircleSettingButton,&outputShapeNoneSettingButton,Button::ConnectedOnLeft | Button::ConnectedOnRight,210,RadioGroup::OutputShape);

	outputShapeDiamondSettingButton.onClick = [this](){
		dataBuffer->outputShape.set(VectorscopeShape::Diamond);
	};
	settingButtonSetup(outputShapeDiamondSettingButton,&outputShapeCircleSettingButton,Button::ConnectedOnLeft | Button::ConnectedOnRight,210,RadioGroup::OutputShape);

	outputShapeSquareSettingButton.onClick = [this](){
		dataBuffer->outputShape.set(VectorscopeShape::Square);
	};
	settingButtonSetup(outputShapeSquareSettingButton,&outputShapeDiamondSettingButton,Button::ConnectedOnLeft,210,RadioGroup::OutputShape);

	aboutTextLabel.setBounds(35,35,initAppWidth-35,initAppHeight-35);
	aboutTextLabel.setJustificationType(Justification::topLeft);
	aboutTextLabel.setFont(claf.interFont.withPointHeight(18));
	addChildComponent(aboutTextLabel);

	//For the inOutGrid, establish that the dummy side shapers that will be producing the points that draw the curves and lines in the inOutGrid are coming in as mono samples 0.0 to 1.0.
	midSideShaper.setMidMode(true);
	sideSideShaper.setMidMode(true);
	addAndMakeVisible(inOutGrid);

	//Establish what clicking the tab buttons does.
	//Do not do anything if the user is trying to click on a disabled tab.
	for(int i = 0; i < 6; i++){
		tabButtons[i].onClick = [this,i](){
			if(dataBuffer->bypassSplitters.get()){
				if(i != TabName::Low && i != TabName::MidLow && i != TabName::MidHigh && i != TabName::High){
					tabChange(i);
				}
			}else{
				if(dataBuffer->oneSplit.get()){
					if(i != TabName::MidLow && i != TabName::MidHigh){
						tabChange(i);
					}
				}else{
					tabChange(i);
				}
			}
		};
		addAndMakeVisible(tabButtons[i]);
	}

	//Since the whole PluginEditor object is completely destroyed and recreated every time the user closes the window and reopens it, we want to load whatever values the databuffer says.
	tabChange(dataBuffer->currentTabIndex.get(),true);
}

void FrameBuffer::alertInOutGrid(int midOrSide){
	int currentTab = dataBuffer->currentTabIndex.get();
	dataBuffer->inOutGridMidSideMemory[currentTab].set(midOrSide);
	if(midOrSide == 0){
		inOutGrid.sideShaper = &midSideShaper;
		midSideShaper.setStartDistance(static_cast<float>(midSliderMain.getMinValue()));
		midSideShaper.setEndDistance(static_cast<float>(midSliderMain.getMaxValue()));
		midSideShaper.setZone1PercentageRelative(static_cast<float>(midSliderMain.getValue()));
		midSideShaper.setInflectionPointCompression(static_cast<float>(midSliderInflection.getValue()));
		midSideShaper.setZone1Saturation(static_cast<float>(midSliderZone1.getValue()));
		midSideShaper.setZone2Saturation(static_cast<float>(midSliderZone2.getValue()));
		midSideShaper.setGateBeforeStart(gateBeforeStartMid.getToggleState());
		midSideShaper.setFlatAfterEnd(flatAfterEndMid.getToggleState());
	}else{
		inOutGrid.sideShaper = &sideSideShaper;
		sideSideShaper.setStartDistance(static_cast<float>(sideSliderMain.getMinValue()));
		sideSideShaper.setEndDistance(static_cast<float>(sideSliderMain.getMaxValue()));
		sideSideShaper.setZone1PercentageRelative(static_cast<float>(sideSliderMain.getValue()));
		sideSideShaper.setInflectionPointCompression(static_cast<float>(sideSliderInflection.getValue()));
		sideSideShaper.setZone1Saturation(static_cast<float>(sideSliderZone1.getValue()));
		sideSideShaper.setZone2Saturation(static_cast<float>(sideSliderZone2.getValue()));
		sideSideShaper.setGateBeforeStart(gateBeforeStartSide.getToggleState());
		sideSideShaper.setFlatAfterEnd(flatAfterEndSide.getToggleState());
	}
}

void FrameBuffer::alertInOutGrid(int sliderIndex,double newValues[3]){
	int currentTab = dataBuffer->currentTabIndex.get();
	switch(sliderIndex){
		case SliderName::MidSliderMain:
		case SliderName::MidSliderInflection:
		case SliderName::MidSliderZone1:
		case SliderName::MidSliderZone2:
			inOutGrid.sideShaper = &midSideShaper;
			dataBuffer->inOutGridMidSideMemory[currentTab].set(0);
			break;
		case SliderName::SideSliderMain:
		case SliderName::SideSliderInflection:
		case SliderName::SideSliderZone1:
		case SliderName::SideSliderZone2:
			inOutGrid.sideShaper = &sideSideShaper;
			dataBuffer->inOutGridMidSideMemory[currentTab].set(1);
			break;
	}
	switch(sliderIndex){
		case SliderName::MidSliderMain:
			midSideShaper.setStartDistance((float)newValues[0]);
			midSideShaper.setEndDistance((float)newValues[1]);
			midSideShaper.setZone1PercentageRelative((float)newValues[2]);
			break;
		case SliderName::MidSliderInflection:
			midSideShaper.setInflectionPointCompression((float)newValues[0]);
			break;
		case SliderName::MidSliderZone1:
			midSideShaper.setZone1Saturation((float)newValues[0]);
			break;
		case SliderName::MidSliderZone2:
			midSideShaper.setZone2Saturation((float)newValues[0]);
			break;
		case SliderName::SideSliderMain:
			sideSideShaper.setStartDistance((float)newValues[0]);
			sideSideShaper.setEndDistance((float)newValues[1]);
			sideSideShaper.setZone1PercentageRelative((float)newValues[2]);
			break;
		case SliderName::SideSliderInflection:
			sideSideShaper.setInflectionPointCompression((float)newValues[0]);
			break;
		case SliderName::SideSliderZone1:
			sideSideShaper.setZone1Saturation((float)newValues[0]);
			break;
		case SliderName::SideSliderZone2:
			sideSideShaper.setZone2Saturation((float)newValues[0]);
			break;
	}
}

void FrameBuffer::determineTabButtonText(){
	bool bypassSplittersValue = dataBuffer->bypassSplitters.get();
	bool oneSplitValue = dataBuffer->oneSplit.get();
	if(bypassSplittersValue){
		tabButtons[TabName::Low].setButtonText("Low (Disabled)");
		tabButtons[TabName::MidLow].setButtonText("Mid Low (Disabled)");
		tabButtons[TabName::MidHigh].setButtonText("Mid High (Disabled)");
		tabButtons[TabName::High].setButtonText("High (Disabled)");
	}else{
		tabButtons[TabName::Low].setButtonText("Low");
		if(oneSplitValue){
			tabButtons[TabName::MidLow].setButtonText("Mid Low (Disabled)");
			tabButtons[TabName::MidHigh].setButtonText("Mid High (Disabled)");
		}else{
			tabButtons[TabName::MidLow].setButtonText("Mid Low");
			tabButtons[TabName::MidHigh].setButtonText("Mid High");
		}
		tabButtons[TabName::High].setButtonText("High");
	}
}

void FrameBuffer::update(){
	//Pull the vectorscope graphics data from the data buffer.
	//This will occur every frame.
	int currentTab = dataBuffer->currentTabIndex.get();
	int start1 = 0;
	int size1 = 0;
	int start2 = 0;
	int size2 = 0;
	int numReady = dataBuffer->graphicsDataHandler.getNumReady();
	dataBuffer->graphicsDataHandler.prepareToRead(numReady, start1, size1, start2, size2);
	//If we should pull from the float buffer,
	if(dataBuffer->lastUpdatedDataType.get() == 0){
		if(size1 > 0){
			for(int i = 0; i < size1; ++i){
				SplitBufferValues<float> bv = dataBuffer->fGraphicsData[start1 + i];
				leftVscope.midValues[start1 + i] = bv[currentTab*2][0];
				leftVscope.sideValues[start1 + i] = bv[currentTab*2][1];
				rightVscope.midValues[start1 + i] = bv[currentTab*2+1][0];
				rightVscope.sideValues[start1 + i] = bv[currentTab*2+1][1];
			}
		}
		if(size2 > 0){
			for(int i = 0; i < size2; ++i){
				SplitBufferValues<float> bv = dataBuffer->fGraphicsData[start2 + i];
				leftVscope.midValues[start2 + i] = bv[currentTab*2][0];
				leftVscope.sideValues[start2 + i] = bv[currentTab*2][1];
				rightVscope.midValues[start2 + i] = bv[currentTab*2+1][0];
				rightVscope.sideValues[start2 + i] = bv[currentTab*2+1][1];
			}
		}
	}
	//If we should pull from the double buffer,
	else{
		if(size1 > 0){
			for(int i = 0; i < size1; ++i){
				SplitBufferValues<double> bv = dataBuffer->dGraphicsData[start1 + i];
				leftVscope.midValues[start1 + i] = (float)bv[currentTab*2][0];
				leftVscope.sideValues[start1 + i] = (float)bv[currentTab*2][1];
				rightVscope.midValues[start1 + i] = (float)bv[currentTab*2+1][0];
				rightVscope.sideValues[start1 + i] = (float)bv[currentTab*2+1][1];
			}
		}
		if(size2 > 0){
			for(int i = 0; i < size2; ++i){
				SplitBufferValues<double> bv = dataBuffer->dGraphicsData[start2 + i];
				leftVscope.midValues[start2 + i] = (float)bv[currentTab*2][0];
				leftVscope.sideValues[start2 + i] = (float)bv[currentTab*2][1];
				rightVscope.midValues[start2 + i] = (float)bv[currentTab*2+1][0];
				rightVscope.sideValues[start2 + i] = (float)bv[currentTab*2+1][1];
			}
		}
	}
	dataBuffer->graphicsDataHandler.finishedRead(size1 + size2);

	//Update the slider values in case the user is editing the values via DAW parameter.
	//This should only happen when the user is doing this, but will checked every frame.
	int alertNeeded = 0;
	if(midSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		midSliderMain.setMinValue(*dataBuffer->sideShaperFloats[currentTab*2][0],NotificationType::dontSendNotification);
		midSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		alertNeeded = 1;
	}
	if(midSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMax.get()){
		midSliderMain.setMaxValue(*dataBuffer->sideShaperFloats[currentTab*2][1],NotificationType::dontSendNotification);
		midSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMax.set(false);
		alertNeeded = 1;
	}
	if(midSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMid.get()){
		midSliderMain.setValue(*dataBuffer->sideShaperFloats[currentTab*2][2],NotificationType::dontSendNotification);
		midSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMid.set(false);
		alertNeeded = 1;
	}
	if(midSliderInflection.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		midSliderInflection.setValue(*dataBuffer->sideShaperFloats[currentTab*2][3],NotificationType::dontSendNotification);
		midSliderInflection.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		alertNeeded = 1;
	}
	if(midSliderZone1.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		midSliderZone1.setValue(*dataBuffer->sideShaperFloats[currentTab*2][4],NotificationType::dontSendNotification);
		midSliderZone1.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		alertNeeded = 1;
	}
	if(midSliderZone2.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		midSliderZone2.setValue(*dataBuffer->sideShaperFloats[currentTab*2][5],NotificationType::dontSendNotification);
		midSliderZone2.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		alertNeeded = 1;
	}

	if(sideSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		sideSliderMain.setMinValue(*dataBuffer->sideShaperFloats[currentTab*2+1][0],NotificationType::dontSendNotification);
		sideSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		alertNeeded = 2;
	}
	if(sideSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMax.get()){
		sideSliderMain.setMaxValue(*dataBuffer->sideShaperFloats[currentTab*2+1][1],NotificationType::dontSendNotification);
		sideSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMax.set(false);
		alertNeeded = 2;
	}
	if(sideSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMid.get()){
		sideSliderMain.setValue(*dataBuffer->sideShaperFloats[currentTab*2+1][2],NotificationType::dontSendNotification);
		sideSliderMain.sliderData->hasChangeNeedingToBeRecognizedByEditorMid.set(false);
		alertNeeded = 2;
	}
	if(sideSliderInflection.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		sideSliderInflection.setValue(*dataBuffer->sideShaperFloats[currentTab*2+1][3],NotificationType::dontSendNotification);
		sideSliderInflection.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		alertNeeded = 2;
	}
	if(sideSliderZone1.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		sideSliderZone1.setValue(*dataBuffer->sideShaperFloats[currentTab*2+1][4],NotificationType::dontSendNotification);
		sideSliderZone1.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		alertNeeded = 2;
	}
	if(sideSliderZone2.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		sideSliderZone2.setValue(*dataBuffer->sideShaperFloats[currentTab*2+1][5],NotificationType::dontSendNotification);
		sideSliderZone2.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		alertNeeded = 2;
	}

	if(alertNeeded == 1){
		alertInOutGrid(0);
	}else if(alertNeeded == 2){
		alertInOutGrid(1);
	}

	bool frequencyLabelsNeedToBeUpdated = false;
	if(upperFrequencySlider.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		upperFrequencySlider.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		if(currentTab == TabName::MidLow){
			upperFrequencySlider.setValue(*dataBuffer->freqLowMid);
		}else if(currentTab == TabName::MidHigh){
			upperFrequencySlider.setValue(*dataBuffer->freqMidMid);
		}else if(currentTab == TabName::High){
			if(!dataBuffer->oneSplit.get()){
				upperFrequencySlider.setValue(*dataBuffer->freqMidHigh);
			}else{
				upperFrequencySlider.setValue(*dataBuffer->freqLowMid);
			}
		}
		frequencyLabelsNeedToBeUpdated = true;
	}
	if(lowerFrequencySlider.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.get()){
		lowerFrequencySlider.sliderData->hasChangeNeedingToBeRecognizedByEditorMin.set(false);
		if(currentTab == TabName::Low){
			lowerFrequencySlider.setValue(*dataBuffer->freqLowMid);
		}else if(currentTab == TabName::MidLow){
			lowerFrequencySlider.setValue(*dataBuffer->freqMidMid);
		}else if(currentTab == TabName::MidHigh){
			lowerFrequencySlider.setValue(*dataBuffer->freqMidHigh);
		}
		frequencyLabelsNeedToBeUpdated = true;
	}
	if(frequencyLabelsNeedToBeUpdated){
		updateCrossoverLabels(currentTab);
	}
	if(dataBuffer->saveStateChangeNeedingToBeRecognizedByEditor.get()){
		tabChange(currentTab,true);
		dataBuffer->saveStateChangeNeedingToBeRecognizedByEditor.set(false);
	}
	repaint();
}

void FrameBuffer::updateCrossoverLabels(int tabIndex){
	bool oneSplit = dataBuffer->oneSplit.get();
	bool fullFrequencyCrossoverRanges = dataBuffer->fullFrequencyCrossoverRanges.get();
	float upperFrequency = (float)upperFrequencySlider.getValue();
	float lowerFrequency = (float)lowerFrequencySlider.getValue();
	if(tabIndex == TabName::Low){
		float freqLowMidHz;
		if(oneSplit){
			freqLowMidHz = oneSplitFrequencyRange.convertFrom0to1(lowerFrequency);
		}else{
			if(!fullFrequencyCrossoverRanges){
				freqLowMidHz = lowerFrequency * 330.0f + 70.0f;
			}else{
				freqLowMidHz = lowerFrequency * 399.0f + 1.0f;
			}
		}
		lowerFrequencyLabel.setText(String(freqLowMidHz,2) + " Hz",NotificationType::sendNotificationSync);
	}else if(tabIndex == TabName::MidLow){
		float freqLowMidHz, freqMidMidHz;
		if(!fullFrequencyCrossoverRanges){
			freqLowMidHz = upperFrequency * 330.0f + 70.0f;
			freqMidMidHz = lowerFrequency * 1190.0f + 410.0f;
		}else{
			freqLowMidHz = upperFrequency * 399.0f + 1.0f;
			freqMidMidHz = lowerFrequency * 6950.0f + 400.0f;
		}
		upperFrequencyLabel.setText(String(freqLowMidHz,2) + " Hz",NotificationType::sendNotificationSync);
		lowerFrequencyLabel.setText(String(freqMidMidHz,2) + " Hz",NotificationType::sendNotificationSync);
	}else if(tabIndex == TabName::MidHigh){
		float freqMidMidHz, freqMidHighHz;
		if(!fullFrequencyCrossoverRanges){
			freqMidMidHz = upperFrequency * 1190.0f + 410.0f;
			freqMidHighHz = lowerFrequency * 7350.0f + 2650.0f;
		}else{
			freqMidMidHz = upperFrequency * 6950.0f + 400.0f;
			freqMidHighHz = lowerFrequency * 14600.0f + 7350.0f;
		}
		upperFrequencyLabel.setText(String(freqMidMidHz,2) + " Hz",NotificationType::sendNotificationSync);
		lowerFrequencyLabel.setText(String(freqMidHighHz,2) + " Hz",NotificationType::sendNotificationSync);
	}else if(tabIndex == TabName::High){
		float freqMidHighHz;
		if(oneSplit){
			freqMidHighHz = oneSplitFrequencyRange.convertFrom0to1(upperFrequency);
		}else{
			if(!fullFrequencyCrossoverRanges){
				freqMidHighHz = upperFrequency * 7350.0f + 2650.0f;
			}else{
				freqMidHighHz = upperFrequency * 14600.0f + 7350.0f;
			}
		}
		upperFrequencyLabel.setText(String(freqMidHighHz,2) + " Hz",NotificationType::sendNotificationSync);
	}
}

void FrameBuffer::paint (Graphics& g) {
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void FrameBuffer::resized(){
	int resizedAppWidth = dataBuffer->appWidth.get();
	int resizedAppHeight = dataBuffer->appHeight.get();
	setSize(resizedAppWidth,resizedAppHeight);
	vectorscopeWidth = int((float)resizedAppWidth * vectorscopeInitWidthRatio);
	vectorscopeHeight = int((float)resizedAppHeight * vectorscopeInitHeightRatio);
	middleSectionWidth = int((float)resizedAppWidth * middleSectionInitWidthRatio);
	buttonSectionHeight = resizedAppHeight - tabBarHeight - vectorscopeHeight;
	tabButtonWidth = int((float)resizedAppWidth * tabButtonWidthInitRatio);

	//Establish positioning helper variables.
	//x values left to right
	int leftVscopeRightEdge = vectorscopeWidth;
	int rightVscopeLeftEdge = rightVscope.getX();
	//y values top to bottom
	int vscopeBottomEdge = vectorscopeHeight;
	int tabButtonsTopEdge = resizedAppHeight - tabBarHeight;

	leftVscope.setSize(vectorscopeWidth,vectorscopeHeight);
	rightVscope.setTopRightPosition(resizedAppWidth,0);
	rightVscope.setSize(vectorscopeWidth,vectorscopeHeight);
	midGrid.performLayout({leftVscopeRightEdge,0,middleSectionWidth,vectorscopeHeight});
	leftSideGrid.performLayout({0,vscopeBottomEdge,vectorscopeWidth,buttonSectionHeight});
	inOutGrid.setBounds(leftVscopeRightEdge,vscopeBottomEdge,middleSectionWidth,buttonSectionHeight);
	sideGrid.performLayout({rightVscopeLeftEdge,vscopeBottomEdge,vectorscopeWidth,buttonSectionHeight});
	for(int i = 0; i < 6; i++){
		tabButtons[i].setBounds(tabButtonWidth*i,tabButtonsTopEdge,tabButtonWidth,tabBarHeight);
	}
	int currentTabIndex = dataBuffer->currentTabIndex.get();
	if(currentTabIndex == TabName::Post){
		settingsButton.setBounds(settingsButton.getBounds().withTrimmedTop(10).withTrimmedBottom(10));
		aboutButton.setBounds(aboutButton.getBounds().withTrimmedTop(10).withTrimmedBottom(10));
	}
}

void FrameBuffer::setFilterTabVisibility(bool visible){
	leftVscope.setVisible(visible);
	for(GridItem& gi : midGrid.items){
		//the padding rows and columns do not have pointers to associated components
		if(gi.associatedComponent != nullptr){
			gi.associatedComponent->setVisible(visible);
		}
	}
	rightVscope.setVisible(visible);
	for(GridItem& gi : leftSideGrid.items){
		//the padding rows and columns do not have pointers to associated components
		if(gi.associatedComponent != nullptr){
			gi.associatedComponent->setVisible(visible);
		}
	}
	inOutGrid.setVisible(visible);
	for(GridItem& gi : sideGrid.items){
		//the padding rows and columns do not have pointers to associated components
		if(gi.associatedComponent != nullptr){
			gi.associatedComponent->setVisible(visible);
		}
	}
}

void FrameBuffer::setSettingTabVisibility(bool visible){
	midSideModeSettingButton.setVisible(visible);
	leftRightModeSettingButton.setVisible(visible);
	unprotectedModeSettingButton.setVisible(visible);
	protectedModeSettingButton.setVisible(visible);
	shapeRangeStandardSettingButton.setVisible(visible);
	shapeRangeSwappedSettingButton.setVisible(visible);
	semitoneCrossoverSettingButton.setVisible(visible);
	halfOctaveCrossoverSettingButton.setVisible(visible);
	fullOctaveCrossoverSettingButton.setVisible(visible);
	inputShapeNoneSettingButton.setVisible(visible);
	inputShapeCircleSettingButton.setVisible(visible);
	inputShapeDiamondSettingButton.setVisible(visible);
	inputShapeSquareSettingButton.setVisible(visible);
	outputShapeNoneSettingButton.setVisible(visible);
	outputShapeCircleSettingButton.setVisible(visible);
	outputShapeDiamondSettingButton.setVisible(visible);
	outputShapeSquareSettingButton.setVisible(visible);
}

void FrameBuffer::settingButtonSetup(TextButton& textButton, TextButton* previousButton, int connectedEdges, int y, int radioGroupId){
	textButton.setConnectedEdges(connectedEdges);
	if(previousButton == nullptr){
		textButton.setTopLeftPosition(35,y);
	}else{
		textButton.setTopLeftPosition(previousButton->getRight(),previousButton->getY());
	}
	textButton.setSize(18,18);
	textButton.changeWidthToFitText();
	textButton.setClickingTogglesState(true);
	textButton.setRadioGroupId(radioGroupId);
	textButton.setColour(TextButton::textColourOffId, Colours::black);
	textButton.setColour(TextButton::textColourOnId, Colours::white);
	textButton.setColour(TextButton::buttonColourId, Colours::white);
	textButton.setColour(TextButton::buttonOnColourId, Colours::blue);
	addChildComponent(textButton);
}

//Note that this function is also called when the user reopens the window after closing it.
void FrameBuffer::tabChange(int newTabIndex, bool reloadOfSamePage){
	int oldTabIndex = dataBuffer->currentTabIndex.get();
	if(newTabIndex == oldTabIndex && !reloadOfSamePage){
		return;
	}
	dataBuffer->currentTabIndex.set(newTabIndex);
	bool goingToSettingsOrAboutTab = newTabIndex == TabName::Settings || newTabIndex == TabName::About;
	bool goingToFilterTab = !goingToSettingsOrAboutTab;
	
	if(goingToFilterTab){
		//Load the most recent vectorscope states we remember the tab having.
		leftVscope.downloadStateFromDataBuffer();
		rightVscope.downloadStateFromDataBuffer();

		//Load the button toggle states for the tab.
		bool soloState = dataBuffer->solo[newTabIndex].get();
		soloButton.setToggleState(soloState,NotificationType::dontSendNotification);
		soloButton.toggleTo(soloState);
		bool beforeOrAfterState = dataBuffer->soloBeforeOrAfter[newTabIndex].get();
		beforeOrAfterButton.setToggleState(beforeOrAfterState,NotificationType::dontSendNotification);
		beforeOrAfterButton.toggleTo(beforeOrAfterState);

		//Load the slider values.
		std::array<std::array<AudioParameterFloatListener*,6>,12>& sideShaperFloats = dataBuffer->sideShaperFloats;

		midSliderMain.setMinValue(sideShaperFloats[newTabIndex*2][0]->get(),NotificationType::dontSendNotification);
		midSliderMain.setMaxValue(sideShaperFloats[newTabIndex*2][1]->get(),NotificationType::dontSendNotification);
		midSliderMain.setValue(sideShaperFloats[newTabIndex*2][2]->get(),NotificationType::dontSendNotification);
		midSliderInflection.setValue(sideShaperFloats[newTabIndex*2][3]->get(),NotificationType::dontSendNotification);
		midSliderZone1.setValue(sideShaperFloats[newTabIndex*2][4]->get(),NotificationType::dontSendNotification);
		midSliderZone2.setValue(sideShaperFloats[newTabIndex*2][5]->get(),NotificationType::dontSendNotification);

		sideSliderMain.setMinValue(sideShaperFloats[newTabIndex*2+1][0]->get(),NotificationType::dontSendNotification);
		sideSliderMain.setMaxValue(sideShaperFloats[newTabIndex*2+1][1]->get(),NotificationType::dontSendNotification);
		sideSliderMain.setValue(sideShaperFloats[newTabIndex*2+1][2]->get(),NotificationType::dontSendNotification);
		sideSliderInflection.setValue(sideShaperFloats[newTabIndex*2+1][3]->get(),NotificationType::dontSendNotification);
		sideSliderZone1.setValue(sideShaperFloats[newTabIndex*2+1][4]->get(),NotificationType::dontSendNotification);
		sideSliderZone2.setValue(sideShaperFloats[newTabIndex*2+1][5]->get(),NotificationType::dontSendNotification);

		//Load the frequency slider section.
		if(newTabIndex == TabName::Pre){
			internalBypassButton.setToggleState(dataBuffer->bypassAll.get(),NotificationType::dontSendNotification);
			splitterBypassButton.setToggleState(dataBuffer->bypassSplitters.get(),NotificationType::dontSendNotification);
		}
		bool oneSplit = dataBuffer->oneSplit.get();
		if(newTabIndex == TabName::Low){
			oneSplitterButton.setToggleState(oneSplit,NotificationType::dontSendNotification);
		}
		if(newTabIndex == TabName::MidLow){
			upperFrequencySlider.setValue(dataBuffer->freqLowMid->get(),NotificationType::dontSendNotification);
		}else if(newTabIndex == TabName::MidHigh){
			upperFrequencySlider.setValue(dataBuffer->freqMidMid->get(),NotificationType::dontSendNotification);
		}else if(newTabIndex == TabName::High){
			if(!oneSplit){
				upperFrequencySlider.setValue(dataBuffer->freqMidHigh->get(),NotificationType::dontSendNotification);
			}else{
				upperFrequencySlider.setValue(dataBuffer->freqLowMid->get(),NotificationType::dontSendNotification);
			}
		}
		if(newTabIndex == TabName::Low){
			lowerFrequencySlider.setValue(dataBuffer->freqLowMid->get(),NotificationType::dontSendNotification);
		}else if(newTabIndex == TabName::MidLow){
			lowerFrequencySlider.setValue(dataBuffer->freqMidMid->get(),NotificationType::dontSendNotification);
		}else if(newTabIndex == TabName::MidHigh){
			lowerFrequencySlider.setValue(dataBuffer->freqMidHigh->get(),NotificationType::dontSendNotification);
		}
		updateCrossoverLabels(newTabIndex);
		if(newTabIndex == TabName::High){
			fullFrequencyCrossoverButton.setToggleState(dataBuffer->fullFrequencyCrossoverRanges.get(),NotificationType::dontSendNotification);
		}

		bool flatAfterEndMidVal = dataBuffer->flatAfterEnd[newTabIndex][0].get();
		flatAfterEndMid.setToggleState(flatAfterEndMidVal,NotificationType::dontSendNotification);
		flatAfterEndMid.toggleTo(flatAfterEndMidVal);
		midSideShaper.setFlatAfterEnd(flatAfterEndMidVal);
		bool gateBeforeStartMidVal = dataBuffer->gateBeforeStart[newTabIndex][0].get();
		gateBeforeStartMid.setToggleState(gateBeforeStartMidVal,NotificationType::dontSendNotification);
		gateBeforeStartMid.toggleTo(gateBeforeStartMidVal);
		midSideShaper.setGateBeforeStart(gateBeforeStartMidVal);
		bool flatAfterEndSideVal = dataBuffer->flatAfterEnd[newTabIndex][1].get();
		flatAfterEndSide.setToggleState(flatAfterEndSideVal,NotificationType::dontSendNotification);
		flatAfterEndSide.toggleTo(flatAfterEndSideVal);
		sideSideShaper.setFlatAfterEnd(flatAfterEndSideVal);
		bool gateBeforeStateSideVal = dataBuffer->gateBeforeStart[newTabIndex][1].get();
		gateBeforeStartSide.setToggleState(gateBeforeStateSideVal,NotificationType::dontSendNotification);
		gateBeforeStartSide.toggleTo(gateBeforeStateSideVal);
		sideSideShaper.setGateBeforeStart(gateBeforeStateSideVal);

		//Load the settings for the in out grid.
		int inOutMidOrSideBeforeRefresh = dataBuffer->inOutGridMidSideMemory[newTabIndex].get();
		alertInOutGrid(0);
		alertInOutGrid(1);
		if(inOutMidOrSideBeforeRefresh == 0) alertInOutGrid(0);
	}

	determineTabButtonText();

	if(newTabIndex == TabName::Settings){
		bool leftRightMode = dataBuffer->leftRightMode.get();
		if(leftRightMode){
			midSideModeSettingButton.setToggleState(false,NotificationType::dontSendNotification);
			leftRightModeSettingButton.setToggleState(true,NotificationType::dontSendNotification);
		}else{
			midSideModeSettingButton.setToggleState(true,NotificationType::dontSendNotification);
			leftRightModeSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		}
		bool protectedMode = dataBuffer->protectedMode.get();
		if(protectedMode){
			unprotectedModeSettingButton.setToggleState(false,NotificationType::dontSendNotification);
			protectedModeSettingButton.setToggleState(true,NotificationType::dontSendNotification);
		}else{
			unprotectedModeSettingButton.setToggleState(true,NotificationType::dontSendNotification);
			protectedModeSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		}
		bool shapeRangeSwapMode = dataBuffer->shapeRangeSwapMode.get();
		if(shapeRangeSwapMode){
			shapeRangeStandardSettingButton.setToggleState(false,NotificationType::dontSendNotification);
			shapeRangeSwappedSettingButton.setToggleState(true,NotificationType::dontSendNotification);
		}else{
			shapeRangeStandardSettingButton.setToggleState(true,NotificationType::dontSendNotification);
			shapeRangeSwappedSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		}
		int crossoverMode = dataBuffer->crossoverMode.get();
		semitoneCrossoverSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		halfOctaveCrossoverSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		fullOctaveCrossoverSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		if(crossoverMode == RolloffSize::Semitone){
			semitoneCrossoverSettingButton.setToggleState(true,NotificationType::dontSendNotification);
		}else if(crossoverMode == RolloffSize::HalfOctave){
			halfOctaveCrossoverSettingButton.setToggleState(true,NotificationType::dontSendNotification);
		}else if(crossoverMode == RolloffSize::FullOctave){
			fullOctaveCrossoverSettingButton.setToggleState(true,NotificationType::dontSendNotification);
		}
		int inputShape = dataBuffer->inputShape.get();
		inputShapeNoneSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		inputShapeCircleSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		inputShapeDiamondSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		inputShapeSquareSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		switch(inputShape){
			case VectorscopeShape::None:
				inputShapeNoneSettingButton.setToggleState(true,NotificationType::dontSendNotification);
				break;
			case VectorscopeShape::Circle:
				inputShapeCircleSettingButton.setToggleState(true,NotificationType::dontSendNotification);
				break;
			case VectorscopeShape::Diamond:
				inputShapeDiamondSettingButton.setToggleState(true,NotificationType::dontSendNotification);
				break;
			case VectorscopeShape::Square:
				inputShapeSquareSettingButton.setToggleState(true,NotificationType::dontSendNotification);
				break;
		}
		int outputShape = dataBuffer->outputShape.get();
		outputShapeNoneSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		outputShapeCircleSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		outputShapeDiamondSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		outputShapeSquareSettingButton.setToggleState(false,NotificationType::dontSendNotification);
		switch(outputShape){
			case VectorscopeShape::None:
				outputShapeNoneSettingButton.setToggleState(true,NotificationType::dontSendNotification);
				break;
			case VectorscopeShape::Circle:
				outputShapeCircleSettingButton.setToggleState(true,NotificationType::dontSendNotification);
				break;
			case VectorscopeShape::Diamond:
				outputShapeDiamondSettingButton.setToggleState(true,NotificationType::dontSendNotification);
				break;
			case VectorscopeShape::Square:
				outputShapeSquareSettingButton.setToggleState(true,NotificationType::dontSendNotification);
				break;
		}
	}

	//Set the visibility of the components.
	GridItem& upperLeft = leftSideGrid.items.getReference(4);
	GridItem& upperRight = leftSideGrid.items.getReference(5);
	GridItem& lowerLeft = leftSideGrid.items.getReference(6);
	GridItem& lowerRight = leftSideGrid.items.getReference(7);
	if(newTabIndex == TabName::Pre){
		setFilterTabVisibility(true);
		upperFrequencySlider.setVisible(false);
		lowerFrequencySlider.setVisible(false);
		upperFrequencyLabel.setVisible(false);
		lowerFrequencyLabel.setVisible(false);
		internalBypassLabel.setVisible(true);
		internalBypassButton.setVisible(true);
		splitterBypassLabel.setVisible(true);
		splitterBypassButton.setVisible(true);
		oneSplitterLabel.setVisible(false);
		oneSplitterButton.setVisible(false);
		fullFrequencyCrossoverLabel.setVisible(false);
		fullFrequencyCrossoverButton.setVisible(false);
		settingsButton.setVisible(false);
		setSettingTabVisibility(false);
		aboutButton.setVisible(false);
		aboutTextLabel.setVisible(false);
		upperLeft.associatedComponent = &internalBypassLabel;
		upperRight.associatedComponent = &internalBypassButton;
		lowerLeft.associatedComponent = &splitterBypassLabel;
		lowerRight.associatedComponent = &splitterBypassButton;
	}else if(newTabIndex == TabName::Low){
		setFilterTabVisibility(true);
		upperFrequencySlider.setVisible(false);
		lowerFrequencySlider.setVisible(true);
		upperFrequencyLabel.setVisible(false);
		lowerFrequencyLabel.setVisible(true);
		internalBypassLabel.setVisible(false);
		internalBypassButton.setVisible(false);
		splitterBypassLabel.setVisible(false);
		splitterBypassButton.setVisible(false);
		oneSplitterLabel.setVisible(true);
		oneSplitterButton.setVisible(true);
		fullFrequencyCrossoverLabel.setVisible(false);
		fullFrequencyCrossoverButton.setVisible(false);
		settingsButton.setVisible(false);
		setSettingTabVisibility(false);
		aboutButton.setVisible(false);
		aboutTextLabel.setVisible(false);
		upperLeft.associatedComponent = &oneSplitterLabel;
		upperRight.associatedComponent = &oneSplitterButton;
		lowerLeft.associatedComponent = &lowerFrequencySlider;
		lowerRight.associatedComponent = &lowerFrequencyLabel;
	}else if(newTabIndex == TabName::MidLow){
		setFilterTabVisibility(true);
		upperFrequencySlider.setVisible(true);
		lowerFrequencySlider.setVisible(true);
		upperFrequencyLabel.setVisible(true);
		lowerFrequencyLabel.setVisible(true);
		internalBypassLabel.setVisible(false);
		internalBypassButton.setVisible(false);
		splitterBypassLabel.setVisible(false);
		splitterBypassButton.setVisible(false);
		oneSplitterLabel.setVisible(false);
		oneSplitterButton.setVisible(false);
		fullFrequencyCrossoverLabel.setVisible(false);
		fullFrequencyCrossoverButton.setVisible(false);
		settingsButton.setVisible(false);
		setSettingTabVisibility(false);
		aboutButton.setVisible(false);
		aboutTextLabel.setVisible(false);
		upperLeft.associatedComponent = &upperFrequencySlider;
		upperRight.associatedComponent = &upperFrequencyLabel;
		lowerLeft.associatedComponent = &lowerFrequencySlider;
		lowerRight.associatedComponent = &lowerFrequencyLabel;
	}else if(newTabIndex == TabName::MidHigh){
		setFilterTabVisibility(true);
		upperFrequencySlider.setVisible(true);
		lowerFrequencySlider.setVisible(true);
		upperFrequencyLabel.setVisible(true);
		lowerFrequencyLabel.setVisible(true);
		internalBypassLabel.setVisible(false);
		internalBypassButton.setVisible(false);
		splitterBypassLabel.setVisible(false);
		splitterBypassButton.setVisible(false);
		oneSplitterLabel.setVisible(false);
		oneSplitterButton.setVisible(false);
		fullFrequencyCrossoverLabel.setVisible(false);
		fullFrequencyCrossoverButton.setVisible(false);
		settingsButton.setVisible(false);
		setSettingTabVisibility(false);
		aboutButton.setVisible(false);
		aboutTextLabel.setVisible(false);
		upperLeft.associatedComponent = &upperFrequencySlider;
		upperRight.associatedComponent = &upperFrequencyLabel;
		lowerLeft.associatedComponent = &lowerFrequencySlider;
		lowerRight.associatedComponent = &lowerFrequencyLabel;
	}else if(newTabIndex == TabName::High){
		setFilterTabVisibility(true);
		upperFrequencySlider.setVisible(true);
		lowerFrequencySlider.setVisible(false);
		upperFrequencyLabel.setVisible(true);
		lowerFrequencyLabel.setVisible(false);
		internalBypassLabel.setVisible(false);
		internalBypassButton.setVisible(false);
		splitterBypassLabel.setVisible(false);
		splitterBypassButton.setVisible(false);
		oneSplitterLabel.setVisible(false);
		oneSplitterButton.setVisible(false);
		fullFrequencyCrossoverLabel.setVisible(true);
		fullFrequencyCrossoverButton.setVisible(true);
		settingsButton.setVisible(false);
		setSettingTabVisibility(false);
		aboutButton.setVisible(false);
		aboutTextLabel.setVisible(false);
		upperLeft.associatedComponent = &upperFrequencySlider;
		upperRight.associatedComponent = &upperFrequencyLabel;
		lowerLeft.associatedComponent = &fullFrequencyCrossoverLabel;
		lowerRight.associatedComponent = &fullFrequencyCrossoverButton;
	}else if(newTabIndex == TabName::Post){
		setFilterTabVisibility(true);
		upperFrequencySlider.setVisible(false);
		lowerFrequencySlider.setVisible(false);
		upperFrequencyLabel.setVisible(false);
		lowerFrequencyLabel.setVisible(false);
		internalBypassLabel.setVisible(false);
		internalBypassButton.setVisible(false);
		splitterBypassLabel.setVisible(false);
		splitterBypassButton.setVisible(false);
		oneSplitterLabel.setVisible(false);
		oneSplitterButton.setVisible(false);
		fullFrequencyCrossoverLabel.setVisible(false);
		fullFrequencyCrossoverButton.setVisible(false);
		settingsButton.setVisible(true);
		setSettingTabVisibility(false);
		aboutButton.setVisible(true);
		aboutTextLabel.setVisible(false);
		upperLeft.associatedComponent = &settingsButton;
		lowerLeft.associatedComponent = &aboutButton;
	}else if(newTabIndex == TabName::Settings){
		setFilterTabVisibility(false);
		setSettingTabVisibility(true);
	}else if(newTabIndex == TabName::About){
		setFilterTabVisibility(false);
		setSettingTabVisibility(false);
		aboutTextLabel.setVisible(true);
	}
	resized();
	repaint();
}

FrameBuffer::~FrameBuffer(){
	
}

SideShaperAudioProcessorEditor::SideShaperAudioProcessorEditor (juce::AudioProcessor* p,DataBuffer* db) :
AudioProcessorEditor (p),
audioProcessor(p),
dataBuffer(db),
frameBuffer(db){
	setResizable(true, false);
	getConstrainer()->setMinimumSize(600, 376);//see also DataBuffer.h
	int appWidth = db->appWidth.get();
	int appHeight = db->appHeight.get();
	AudioProcessorEditor::setSize(appWidth, appHeight);
	setSize(appWidth,appHeight);
	LookAndFeel::setDefaultLookAndFeel(&frameBuffer.claf);
	addAndMakeVisible(frameBuffer);
}

//This function will be called any time the host user has opened or moved the window.
void SideShaperAudioProcessorEditor::paint (juce::Graphics& g) {
	//DBG("SideShaperAudioProcessorEditor paint called at: " << juce::Time::getMillisecondCounter());
	g.fillAll(AudioProcessorEditor::getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	g.setFont(FontOptions(15.0f));
	g.setColour(Colours::white);
}

void SideShaperAudioProcessorEditor::resized() {
	Rectangle<int> newSize = getLocalBounds();
	dataBuffer->appWidth.set(newSize.getWidth());
	dataBuffer->appHeight.set(newSize.getHeight());
	frameBuffer.resized();
}

SideShaperAudioProcessorEditor::~SideShaperAudioProcessorEditor() {
	LookAndFeel::setDefaultLookAndFeel(nullptr);
}