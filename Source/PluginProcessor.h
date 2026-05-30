#pragma once
#include <JuceHeader.h>

//==============================================================================
// A single grain of audio
struct Grain
{
    std::vector<float> buffer;   // recorded samples (left then right interleaved)
    int     writePos    = 0;
    int     readPos     = 0;
    float   pitch       = 1.0f;  // playback speed (pitch shift ratio)
    bool    recording   = false;
    bool    playing     = false;
    float   env         = 0.0f;  // amplitude envelope 0‥1
    int     grainSize   = 0;     // total samples in grain
};

//==============================================================================
class CrystallizerAudioProcessor  : public juce::AudioProcessor,
                                    public juce::AudioProcessorValueTreeState::Listener
{
public:
    CrystallizerAudioProcessor();
    ~CrystallizerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Crystallizer"; }
    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    void parameterChanged (const juce::String& paramID, float newValue) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Parameters (atomic copies for audio thread)
    std::atomic<float> grainSizeMs  { 120.0f };
    std::atomic<float> pitchParam   { 0.0f   };   // semitones, –12 to +12
    std::atomic<float> feedbackParam{ 0.40f  };
    std::atomic<float> mixParam     { 0.50f  };
    std::atomic<float> spreadParam  { 0.50f  };   // pitch spread between grains

    // Grain pool
    static constexpr int kNumGrains = 6;
    Grain grains[kNumGrains];

    int   grainTriggerCounter = 0;
    int   nextGrainIndex      = 0;
    double currentSampleRate  = 44100.0;

    // Feedback delay line (stereo)
    juce::AudioBuffer<float> feedbackBuffer;
    int feedbackWritePos = 0;

    void triggerNewGrain (const juce::AudioBuffer<float>& input, int startSample);
    float readGrainSample (Grain& g, int channel);
    float hannEnvelope (float phase); // phase 0‥1

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrystallizerAudioProcessor)
};
