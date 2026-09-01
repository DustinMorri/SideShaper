#pragma once
#include <JuceHeader.h>
#include "SideShaper.h"

using namespace juce;

class InOutGrid : public Component {
	public:
		InOutGrid(SideShaper<float>* sideShaper);
		void paint(Graphics& g) override;
		float inValues[99];
		float outValues[99] = {0};
		SideShaper<float>* sideShaper;
		~InOutGrid();
	private:
		
};