#pragma once
#include <JuceHeader.h>
#include "DataBuffer.h"

using namespace juce;

struct VectorscopeState {
	float centerX = 123.0f;
	float centerXOnMouseDown = 123.0f;
	float centerY = 123.0f;
	float centerYOnMouseDown = 123.0f;
	bool probabilityDistributionMode = false;
	bool probabilityDistributionMidOrSide = false;
	float zoomX = 123.0f;
	float zoomY = 123.0f;
};

class Vectorscope : public Component {
	public:
		Vectorscope(DataBuffer* db, bool leftOrRightVal);
		bool leftOrRight = false;
		DataBuffer* dataBuffer;
		//Component Overrides
		void mouseDown(const MouseEvent &event) override;
		void mouseDrag(const MouseEvent &event) override;
		void mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel) override;
		void paint(Graphics& g) override;
		void repositionButtons();
		void resized() override;
		//Custom Functions
		void loadState(VectorscopeState state);
		VectorscopeState saveState();
		void downloadStateFromDataBuffer();
		void uploadStateToDataBuffer();
		//Size Variables
		int buttonSize = 12;
		float widthScaling = 1.0f;//Imagine that the standard size of the vectorscope is a 250px by 250px square. When the component gets resized, the data being displayed just multiplies this ratio so that the zoom and translation can be normalized to be saved and loaded.
		float heightScaling = 1.0f;
		//Translation And Zoom View Variables
		float centerX = 123.0f;
		float centerXOnMouseDown = 123.0f;
		float centerY = 123.0f;
		float centerYOnMouseDown = 123.0f;
		float zoomX = 123.0f;
		float zoomY = 123.0f;
		//Data Variables
		std::array<float,736> midValues{0.0};
		std::array<float,736> midValuesTransformed{0.0};
		std::array<float,736> sideValues{0.0};
		std::array<float,736> sideValuesTransformed{0.0};
		bool probabilityDistributionMode = false;
		bool probabilityDistributionMidOrSide = true;//true - mid, false - side
		std::array<std::array<int,5>,30> probabilityDistributionFrames = {0};
		std::array<int,5> probabilityDistributionSums = {0};
		int probabilityDistributionCounter = 0;
		//Images For Image Buttons
		Image arrowTopRightImage = ImageCache::getFromMemory(BinaryData::arrow_top_right_png, BinaryData::arrow_top_right_pngSize);
		Image barChartImage = ImageCache::getFromMemory(BinaryData::bar_chart_png, BinaryData::bar_chart_pngSize);
		Image copyImage = ImageCache::getFromMemory(BinaryData::copy_png, BinaryData::copy_pngSize);
		Image positionTopRightImage = ImageCache::getFromMemory(BinaryData::position_top_right_png, BinaryData::position_top_right_pngSize);
		Image scatterPlotImage = ImageCache::getFromMemory(BinaryData::scatter_plot_png, BinaryData::scatter_plot_pngSize);
		Image recenterImage = ImageCache::getFromMemory(BinaryData::recenter_png, BinaryData::recenter_pngSize);
		Image zoomInImage = ImageCache::getFromMemory(BinaryData::zoom_in_png, BinaryData::zoom_in_pngSize);
		Image zoomOutImage = ImageCache::getFromMemory(BinaryData::zoom_out_png, BinaryData::zoom_out_pngSize);
		//Image Buttons
		ImageButton copyButton = ImageButton("copyButton");
		ImageButton midSideSwitchButton = ImageButton("midSideSwitchButton");
		ImageButton recenterButton = ImageButton("recenterButton");
		ImageButton vsPdSwitchButton = ImageButton("vsPdSwitchButton");
		ImageButton zoomInButton = ImageButton("zoomInButton");
		ImageButton zoomOutButton = ImageButton("zoomOutButton");
		ImageButton zoomToQuadrant1Button = ImageButton("zoomToQuadrant1Button");
	private:

};