#include "InOutGrid.h"

InOutGrid::InOutGrid(SideShaper<float>* sideShaper) : Component(){
	this->sideShaper = sideShaper;
	for(int i=0;i<98;i++){
		inValues[i] = ((float)(i+1)) / 100.0f;
	}
	inValues[98] = 1.0f;
}

void InOutGrid::paint(Graphics& g){
	g.fillAll(Colours::blue);
	g.setColour(Colours::white);
	float width = (float)getWidth();
	float height = (float)getHeight();
	Path p;
	p.startNewSubPath(0.0f * width,1.0f * height);
	for(int i=0;i<99;i++){
		outValues[i] = sideShaper->processSample({inValues[i],inValues[i]}).data[0];
		p.lineTo(inValues[i] * width,(1.0f - outValues[i]) * height);
	}
	g.strokePath(p,PathStrokeType(1.0f));
}

InOutGrid::~InOutGrid(){

}