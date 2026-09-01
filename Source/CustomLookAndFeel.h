#pragma once
#include <JuceHeader.h>

using namespace juce;

struct CustomLookAndFeel : public LookAndFeel_V4 {
	public:
		CustomLookAndFeel();
		Font getLabelFont(juce::Label&) override;
		Font getTextButtonFont(TextButton &,int	buttonHeight) override;
		Font interFont = Font(FontOptions(Typeface::createSystemTypefaceFor(BinaryData::Inter_18ptRegular_ttf,BinaryData::Inter_18ptRegular_ttfSize)).withPointHeight(12.0f));
		~CustomLookAndFeel();
};