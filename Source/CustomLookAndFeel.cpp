#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel(){
	
}

Font CustomLookAndFeel::getLabelFont(juce::Label&){
	return interFont;
}

Font CustomLookAndFeel::getTextButtonFont(TextButton &,int){
	return interFont;
}

CustomLookAndFeel::~CustomLookAndFeel(){

}