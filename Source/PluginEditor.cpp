#include "PluginEditor.h"

//==============================================================================
CrystallizerAudioProcessorEditor::CrystallizerAudioProcessorEditor (CrystallizerAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (520, 220);

    setupKnob (grainSizeSlider, grainSizeLabel, "Grain Size");
    setupKnob (pitchSlider,     pitchLabel,     "Pitch");
    setupKnob (feedbackSlider,  feedbackLabel,  "Feedback");
    setupKnob (mixSlider,       mixLabel,       "Mix");
    setupKnob (spreadSlider,    spreadLabel,    "Spread");

    // Attach to APVTS
    grainSizeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                        (processorRef.apvts, "grainSize", grainSizeSlider);
    pitchAttach     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                        (processorRef.apvts, "pitch",     pitchSlider);
    feedbackAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                        (processorRef.apvts, "feedback",  feedbackSlider);
    mixAttach       = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                        (processorRef.apvts, "mix",       mixSlider);
    spreadAttach    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                        (processorRef.apvts, "spread",    spreadSlider);
}

CrystallizerAudioProcessorEditor::~CrystallizerAudioProcessorEditor() {}

//==============================================================================
void CrystallizerAudioProcessorEditor::setupKnob (juce::Slider& s, juce::Label& l,
                                                    const juce::String& name)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    s.setColour (juce::Slider::rotarySliderFillColourId,   juce::Colour (0xff66ccff));
    s.setColour (juce::Slider::rotarySliderOutlineColourId,juce::Colour (0xff224455));
    s.setColour (juce::Slider::thumbColourId,              juce::Colours::white);
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (12.0f, juce::Font::bold));
    l.setColour (juce::Label::textColourId, juce::Colour (0xffaaddff));
    addAndMakeVisible (l);
}

void CrystallizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark gradient background
    g.fillAll (juce::Colour (0xff0a0f18));

    // Subtle grid lines
    g.setColour (juce::Colour (0x18aaddff));
    for (int x = 0; x < getWidth(); x += 40)
        g.drawVerticalLine (x, 0.0f, (float)getHeight());
    for (int y = 0; y < getHeight(); y += 40)
        g.drawHorizontalLine (y, 0.0f, (float)getWidth());

    // Title bar
    g.setColour (juce::Colour (0xff112233));
    g.fillRect (0, 0, getWidth(), 36);

    g.setFont (juce::Font (20.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xff66ccff));
    g.drawText ("CRYSTALLIZER", 0, 0, getWidth(), 36, juce::Justification::centred);

    g.setFont (juce::Font (9.0f));
    g.setColour (juce::Colour (0x88aaddff));
    g.drawText ("reverse granular pitch echo", 0, 0, getWidth(), 36,
                juce::Justification::centredRight);
    g.drawText ("                             ", 0, 0, 10, 36,
                juce::Justification::centredLeft);

    // Border
    g.setColour (juce::Colour (0xff224455));
    g.drawRect (getLocalBounds(), 1);
}

void CrystallizerAudioProcessorEditor::resized()
{
    // 5 knobs spread evenly below the title bar
    const int titleH  = 36;
    const int knobW   = 90;
    const int knobH   = 90;
    const int labelH  = 18;
    const int totalW  = getWidth();
    const int spacing = totalW / 5;
    const int startY  = titleH + 10;

    auto placeKnob = [&](juce::Slider& s, juce::Label& l, int index)
    {
        int cx = spacing * index + spacing / 2;
        s.setBounds (cx - knobW / 2, startY,          knobW, knobH);
        l.setBounds (cx - knobW / 2, startY + knobH,  knobW, labelH);
    };

    placeKnob (grainSizeSlider, grainSizeLabel, 0);
    placeKnob (pitchSlider,     pitchLabel,     1);
    placeKnob (feedbackSlider,  feedbackLabel,  2);
    placeKnob (spreadSlider,    spreadLabel,    3);
    placeKnob (mixSlider,       mixLabel,       4);
}
