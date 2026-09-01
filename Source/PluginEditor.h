#pragma once
#include <JuceHeader.h>
#include <string>
#include "CustomLookAndFeel.h"
#include "DataBuffer.h"
#include "InOutGrid.h"
#include "LinearPhaseCrossover.h"
#include "Vectorscope.h"

using namespace juce;

enum RadioGroup : int {
	//0 is reserved
	//1 to (x~<50) are automatically made by JUCE for buttons I haven't explicitly listed
	//1000 to ??? are for the automated parameters
	LeftRightMode = 50,//Default is MidSideMode
	ProtectedMode,//Default is UnprotectedMode
	LogarithmicSkew,//Default is LinearSkew
	ShapeRangeSwapMode,//Default is StandardMode
	CrossoverMode,//Default is FullOctave
	InputShape,//Default is None
	OutputShape//Default is None
};

struct GridItemPanel  : public Component {
    GridItemPanel(Colour colourToUse) : colour (colourToUse){}
    void paint(Graphics& g) override {
        g.fillAll(colour.withAlpha (0.5f));
    }
    Colour colour;
};

class LightUpButton : public ImageButton {
	public:
		LightUpButton(Image enabledImage, Image disabledImage);
		void mouseDown(const MouseEvent& event) override;
		void mouseUp(const MouseEvent &event) override;
		void toggleTo(bool value);
		std::function<void()> onRightClick = [](){};
		Image enabledImage;
		Image disabledImage;
		~LightUpButton();
};

class LinearParameterSlider : public Slider {
	public:
		LinearParameterSlider(void* fb, Slider::SliderStyle sliderStyle, int sliderIndex);
		void mouseDown(const MouseEvent& event) override;
		void connectToDataBuffer(DataBuffer* db);
		void valueChanged() override;
		DataBuffer* dataBuffer = nullptr;//These are set with the connectToDataBuffer function.
		SliderData* sliderData = nullptr;
		void* frameBuffer;
		int sliderIndex;
		~LinearParameterSlider();
};

class ThreeValueParameterSlider : public Slider {
	public:
		ThreeValueParameterSlider(void* fb, Slider::SliderStyle sliderStyle, int sliderIndex);
		void mouseDown(const MouseEvent& event) override;
		void connectToDataBuffer(DataBuffer* db);
		void valueChanged() override;
		DataBuffer* dataBuffer = nullptr;
		SliderData* sliderData = nullptr;
		void* frameBuffer;
		int sliderIndex;
		~ThreeValueParameterSlider();
};

class TabButton : public TextButton {
	public:
		TabButton(int tabIndex,String text);
		int tabIndex;
		~TabButton();
};

class FrameBuffer : public AnimatedAppComponent {
	public:
		FrameBuffer(DataBuffer* db);
		void alertInOutGrid(int midOrSide);
		void alertInOutGrid(int sliderIndex,double newValues[3]);
		void determineTabButtonText();
		void paint (Graphics& g) override;
		void resized() override;
		void setFilterTabVisibility(bool visible);
		void setSettingTabVisibility(bool visible);
		void settingButtonSetup(TextButton& textButton, TextButton* previousButton, int connectedEdges, int y, int radioGroupId);
		void tabChange(int newTabIndex, bool reloadOfSamePage = false);
		void update() override;
		void updateCrossoverLabels(int tabIndex);
		DataBuffer* dataBuffer;
		//GUI
		CustomLookAndFeel claf = CustomLookAndFeel();
		double clipboardValue = 0.5f;
		//Vectorscopes
		Vectorscope leftVscope;
		Vectorscope rightVscope;
		//Mid Stereo Shaper Slider Section
		Grid midGrid = Grid();
		ThreeValueParameterSlider midSliderMain = ThreeValueParameterSlider(this,Slider::SliderStyle::ThreeValueVertical,0);
		LinearParameterSlider midSliderInflection = LinearParameterSlider(this,Slider::SliderStyle::LinearVertical,1);
		LinearParameterSlider midSliderZone1 = LinearParameterSlider(this,Slider::SliderStyle::LinearVertical,2);
		LinearParameterSlider midSliderZone2 = LinearParameterSlider(this,Slider::SliderStyle::LinearVertical,3);
		LightUpButton gateBeforeStartMid = LightUpButton(
			ImageCache::getFromMemory(BinaryData::gate_vertical_enabled_png,BinaryData::gate_vertical_enabled_pngSize),
			ImageCache::getFromMemory(BinaryData::gate_vertical_disabled_png,BinaryData::gate_vertical_disabled_pngSize)
		);
		LightUpButton flatAfterEndMid = LightUpButton(
			ImageCache::getFromMemory(BinaryData::limit_vertical_enabled_png,BinaryData::limit_vertical_enabled_pngSize),
			ImageCache::getFromMemory(BinaryData::limit_vertical_disabled_png,BinaryData::limit_vertical_disabled_pngSize)
		);
		LightUpButton soloButton = LightUpButton(
			ImageCache::getFromMemory(BinaryData::solo_enabled_png, BinaryData::solo_enabled_pngSize),
			ImageCache::getFromMemory(BinaryData::solo_disabled_png, BinaryData::solo_disabled_pngSize)
		);
		//Lower Left Side
		Grid leftSideGrid = Grid();
		//  Pre
		ToggleButton internalBypassButton = ToggleButton();//Labeled as "Graphics Only" in the GUI.
		Label internalBypassLabel = Label("InternalBypassLabel","Bypass All");
		ToggleButton splitterBypassButton = ToggleButton();
		Label splitterBypassLabel = Label("SplitterBypassLabel","Bypass Splitters");
		//  Low
		ToggleButton oneSplitterButton = ToggleButton();
		Label oneSplitterLabel = Label("OneSplitterLabel","One Split");
		NormalisableRange<float> oneSplitFrequencyRange = NormalisableRange<float>(2.205f,22050.0f);
		//  Low, Mid Low, Mid High, High
		LinearParameterSlider upperFrequencySlider = LinearParameterSlider(this,Slider::SliderStyle::LinearHorizontal,8);
		LinearParameterSlider lowerFrequencySlider = LinearParameterSlider(this,Slider::SliderStyle::LinearHorizontal,9);
		Label upperFrequencyLabel = Label("x Hz");
		Label lowerFrequencyLabel = Label("x Hz");
		//  High
		ToggleButton fullFrequencyCrossoverButton = ToggleButton();
		Label fullFrequencyCrossoverLabel = Label("FullFrequencyCrossoverLabel","Full Frequency Range For Crossovers");
		//  Post
		//    Settings
		TextButton settingsButton = TextButton("Settings");
		TextButton midSideModeSettingButton = TextButton("Mid Side Mode");
		TextButton leftRightModeSettingButton = TextButton("Left Right Mode");
		TextButton protectedModeSettingButton = TextButton("Protected Mode");
		TextButton unprotectedModeSettingButton = TextButton("Unprotected Mode");
		TextButton shapeRangeStandardSettingButton = TextButton("Shape Range Standard");
		TextButton shapeRangeSwappedSettingButton = TextButton("Shape Range Swapped");
		TextButton semitoneCrossoverSettingButton = TextButton("Semitone Crossover");
		TextButton halfOctaveCrossoverSettingButton = TextButton("Half Octave Crossover");
		TextButton fullOctaveCrossoverSettingButton = TextButton("Full Octave Crossover");
		TextButton inputShapeNoneSettingButton = TextButton("No Input Shape");
		TextButton inputShapeCircleSettingButton = TextButton("Circle Input");
		TextButton inputShapeDiamondSettingButton = TextButton("Diamond Input");
		TextButton inputShapeSquareSettingButton = TextButton("Square Input");
		TextButton outputShapeNoneSettingButton = TextButton("No Output Shape");
		TextButton outputShapeCircleSettingButton = TextButton("Circle Output");
		TextButton outputShapeDiamondSettingButton = TextButton("Diamond Output");
		TextButton outputShapeSquareSettingButton = TextButton("Square Output");
		//    About
		TextButton aboutButton = TextButton("About");
		Label aboutTextLabel = Label("AboutTextLabel","This Side Shaper plugin was programmed by Dustin Morrison in 2026 using the JUCE C++ library.\nIf you enjoy this plugin, please share it with your friends!\n\nThis plugin comes with absolutely no warranty.\nPlease always remember to save your audio projects frequently.\n\nIf you encounter any bugs and you don't know the ins and outs of GitHub, feel free to email me about it at dustin.morri@gmail.com. Also, if you just want to share cool music with me or if you just want to let me know if this plugin has helped you please also email me.");
		//In Out Grid
		int lastTouchedShaper = 1;
		SideShaper<float> midSideShaper = SideShaper<float>();
		SideShaper<float> sideSideShaper = SideShaper<float>();
		InOutGrid inOutGrid = InOutGrid(&midSideShaper);
		//Side Stereo Shaper Slider Section
		Grid sideGrid = Grid();
		LightUpButton beforeOrAfterButton = LightUpButton(
			ImageCache::getFromMemory(BinaryData::after_png,BinaryData::after_pngSize),
			ImageCache::getFromMemory(BinaryData::before_png,BinaryData::before_pngSize)
		);
		ThreeValueParameterSlider sideSliderMain = ThreeValueParameterSlider(this,Slider::SliderStyle::ThreeValueHorizontal,4);
		LinearParameterSlider sideSliderInflection = LinearParameterSlider(this,Slider::SliderStyle::LinearHorizontal,5);
		LinearParameterSlider sideSliderZone1 = LinearParameterSlider(this,Slider::SliderStyle::LinearHorizontal,6);
		LinearParameterSlider sideSliderZone2 = LinearParameterSlider(this,Slider::SliderStyle::LinearHorizontal,7);
		LightUpButton gateBeforeStartSide = LightUpButton(
			ImageCache::getFromMemory(BinaryData::gate_horizontal_enabled_png, BinaryData::gate_horizontal_enabled_pngSize),
			ImageCache::getFromMemory(BinaryData::gate_horizontal_disabled_png, BinaryData::gate_horizontal_disabled_pngSize)
		);
		LightUpButton flatAfterEndSide = LightUpButton(
			ImageCache::getFromMemory(BinaryData::limit_horizontal_enabled_png, BinaryData::limit_horizontal_enabled_pngSize),
			ImageCache::getFromMemory(BinaryData::limit_horizontal_disabled_png, BinaryData::limit_horizontal_disabled_pngSize)
		);
		//Tabs
		std::array<TabButton,6> tabButtons {
			TabButton(TabName::Pre,"Pre"),
			TabButton(TabName::Low,"Low"),
			TabButton(TabName::MidLow,"Mid Low"),
			TabButton(TabName::MidHigh,"Mid High"),
			TabButton(TabName::High,"High"),
			TabButton(TabName::Post,"Post"),
		};
		//Sizes
		int vectorscopeWidth = 250;
		const float vectorscopeInitWidthRatio = 0.4166666667f;//vectorscopeWidth / appWidth
		int vectorscopeHeight = 250;
		const float vectorscopeInitHeightRatio = 0.6648936170f;//vectorscopeHeight / appHeight
		int middleSectionWidth = 100;
		const float middleSectionInitWidthRatio = 0.1666666667f;//middleSectionWidht / appWidth
		int buttonSectionHeight = 100;
		int labelWidth = 75;
		int labelHeight = 16;
		int buttonWidth = 16;
		int buttonHeight = 16;
		int tabBarHeight = 26;
		int tabButtonWidth = 100;//appWidth / 6
		const float tabButtonWidthInitRatio = 0.1666666667f;//tabButtonWidth / appWidth
		~FrameBuffer() override;
	private:
		//These are the default subcomponent sizes assuming the starting and minimum app size which is set in the constructor of the SideShaperAudioProcessorEditor.
		OwnedArray<GridItemPanel> gridItems;
};

class SideShaperAudioProcessorEditor : public AudioProcessorEditor {
	public:
		SideShaperAudioProcessorEditor(AudioProcessor* p,DataBuffer* db);
		void paint (Graphics& g) override;
		void resized() override;
		DataBuffer* dataBuffer;
		FrameBuffer frameBuffer;
		~SideShaperAudioProcessorEditor() override;
	private:
		AudioProcessor* audioProcessor;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SideShaperAudioProcessorEditor)
};