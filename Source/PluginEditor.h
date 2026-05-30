#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class CrystallizerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit CrystallizerAudioProcessorEditor (CrystallizerAudioProcessor&);
    ~CrystallizerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CrystallizerAudioProcessor& processorRef;

    // Knobs
    juce::Slider grainSizeSlider, pitchSlider, feedbackSlider, mixSlider, spreadSlider;
    juce::Label  grainSizeLabel,  pitchLabel,  feedbackLabel,  mixLabel,  spreadLabel;

    // Attachments keep knobs in sync with APVTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        grainSizeAttach, pitchAttach, feedbackAttach, mixAttach, spreadAttach;

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrystallizerAudioProcessorEditor)
};
