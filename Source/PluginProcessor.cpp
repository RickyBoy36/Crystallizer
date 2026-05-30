#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CrystallizerAudioProcessor::CrystallizerAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    apvts.addParameterListener ("grainSize",  this);
    apvts.addParameterListener ("pitch",      this);
    apvts.addParameterListener ("feedback",   this);
    apvts.addParameterListener ("mix",        this);
    apvts.addParameterListener ("spread",     this);
}

CrystallizerAudioProcessor::~CrystallizerAudioProcessor()
{
    apvts.removeParameterListener ("grainSize",  this);
    apvts.removeParameterListener ("pitch",      this);
    apvts.removeParameterListener ("feedback",   this);
    apvts.removeParameterListener ("mix",        this);
    apvts.removeParameterListener ("spread",     this);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
    CrystallizerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "grainSize", "Grain Size", juce::NormalisableRange<float>(20.f, 500.f, 1.f), 120.f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "pitch", "Pitch", juce::NormalisableRange<float>(-24.f, 24.f, 0.1f), 0.f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "feedback", "Feedback", juce::NormalisableRange<float>(0.f, 0.95f, 0.01f), 0.40f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "mix", "Mix", juce::NormalisableRange<float>(0.f, 1.f, 0.01f), 0.50f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "spread", "Spread", juce::NormalisableRange<float>(0.f, 12.f, 0.1f), 0.f));

    return { params.begin(), params.end() };
}

void CrystallizerAudioProcessor::parameterChanged (const juce::String& paramID, float newValue)
{
    if      (paramID == "grainSize") grainSizeMs   = newValue;
    else if (paramID == "pitch")     pitchParam    = newValue;
    else if (paramID == "feedback")  feedbackParam = newValue;
    else if (paramID == "mix")       mixParam      = newValue;
    else if (paramID == "spread")    spreadParam   = newValue;
}

//==============================================================================
void CrystallizerAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;

    // Allocate grain buffers (max 500 ms stereo)
    int maxGrainSamples = (int)(sampleRate * 0.5) * 2; // *2 for stereo interleaved
    for (auto& g : grains)
    {
        g.buffer.assign ((size_t)maxGrainSamples, 0.0f);
        g.recording = false;
        g.playing   = false;
    }

    // Feedback delay line: 2 seconds stereo
    int fbSize = (int)(sampleRate * 2.0);
    feedbackBuffer.setSize (2, fbSize, false, true, false);
    feedbackWritePos = 0;
    grainTriggerCounter = 0;
}

void CrystallizerAudioProcessor::releaseResources() {}

bool CrystallizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
    if (layouts.getMainInputChannelSet()  != juce::AudioChannelSet::stereo()) return false;
    return true;
}

//==============================================================================
float CrystallizerAudioProcessor::hannEnvelope (float phase)
{
    // phase: 0‥1
    return 0.5f * (1.0f - std::cos (juce::MathConstants<float>::twoPi * phase));
}

void CrystallizerAudioProcessor::triggerNewGrain (const juce::AudioBuffer<float>& input, int startSample)
{
    int grainSamples = (int)(currentSampleRate * grainSizeMs.load() / 1000.0);
    grainSamples     = juce::jmax (64, grainSamples);

    Grain& g = grains[nextGrainIndex % kNumGrains];
    nextGrainIndex++;

    // Stop any previous use of this slot
    g.recording = false;
    g.playing   = false;

    g.grainSize = grainSamples;
    g.writePos  = 0;

    // Pitch: base + random spread
    float semitones = pitchParam.load();
    float spread    = spreadParam.load();
    if (spread > 0.0f)
    {
        float randomOffset = (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * spread;
        semitones += randomOffset;
    }
    g.pitch = std::pow (2.0f, semitones / 12.0f);

    // Pre-fill grain buffer with current input + feedback
    int numInputSamples = juce::jmin (grainSamples, input.getNumSamples() - startSample);
    for (int i = 0; i < numInputSamples; ++i)
    {
        int fbReadPos = (feedbackWritePos - grainSamples + i + feedbackBuffer.getNumSamples())
                        % feedbackBuffer.getNumSamples();
        for (int ch = 0; ch < 2; ++ch)
        {
            float inSample = (ch < input.getNumChannels()) ? input.getSample (ch, startSample + i) : 0.0f;
            float fbSample = feedbackBuffer.getSample (ch, fbReadPos);
            g.buffer[(size_t)(i * 2 + ch)] = inSample + fbSample * feedbackParam.load();
        }
    }
    g.writePos  = numInputSamples;
    g.recording = (numInputSamples < grainSamples); // continue recording if needed
    g.readPos   = grainSamples - 1; // start reading from END (reverse)
    g.playing   = true;
    g.env       = 0.0f;
}

float CrystallizerAudioProcessor::readGrainSample (Grain& g, int channel)
{
    if (!g.playing) return 0.0f;

    // Envelope based on reversed read position
    float phase = 1.0f - (float)g.readPos / (float)(g.grainSize - 1);
    float env   = hannEnvelope (phase);

    // Linear interpolation for pitch shifting
    float fPos  = (float)g.readPos;
    int   iPos0 = (int)fPos;
    int   iPos1 = iPos0 - 1; // reverse: next sample is earlier in time
    iPos0 = juce::jlimit (0, g.grainSize - 1, iPos0);
    iPos1 = juce::jlimit (0, g.grainSize - 1, iPos1);

    float s0 = g.buffer[(size_t)(iPos0 * 2 + channel)];
    float s1 = g.buffer[(size_t)(iPos1 * 2 + channel)];
    float frac = fPos - (float)(int)fPos;
    float sample = s0 + frac * (s1 - s0);

    // Advance read position (reversed, pitch-shifted)
    g.readPos = (int)((float)g.readPos - g.pitch);

    if (g.readPos < 0)
        g.playing = false;

    return sample * env;
}

//==============================================================================
void CrystallizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& /*midi*/)
{
    juce::ScopedNoDenormals noDenormals;

    int numSamples  = buffer.getNumSamples();
    int grainSamples = (int)(currentSampleRate * grainSizeMs.load() / 1000.0);
    grainSamples     = juce::jmax (64, grainSamples);

    // Trigger interval: new grain every grainSize/2 samples (50% overlap)
    int triggerInterval = grainSamples / 2;

    float wet = mixParam.load();
    float dry = 1.0f - wet;

    // Keep a dry copy
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf (buffer);

    // Clear output (we'll accumulate wet signal)
    buffer.clear();

    for (int s = 0; s < numSamples; ++s)
    {
        // Trigger a new grain?
        if (grainTriggerCounter <= 0)
        {
            triggerNewGrain (dryBuffer, s);
            grainTriggerCounter = triggerInterval;
        }
        --grainTriggerCounter;

        // Sum all active grains
        float outL = 0.0f, outR = 0.0f;
        for (auto& g : grains)
        {
            if (!g.playing) continue;

            // Continue recording if grain buffer isn't full yet
            if (g.recording && g.writePos < g.grainSize)
            {
                for (int ch = 0; ch < 2; ++ch)
                {
                    float inSamp = (ch < dryBuffer.getNumChannels()) ? dryBuffer.getSample (ch, s) : 0.0f;
                    g.buffer[(size_t)(g.writePos * 2 + ch)] = inSamp;
                }
                ++g.writePos;
                if (g.writePos >= g.grainSize)
                    g.recording = false;
            }

            outL += readGrainSample (g, 0);
            outR += readGrainSample (g, 1);
        }

        // Soft-clip
        outL = std::tanh (outL);
        outR = std::tanh (outR);

        // Write wet to output
        buffer.setSample (0, s, outL);
        buffer.setSample (1, s, outR);

        // Update feedback delay line
        feedbackBuffer.setSample (0, feedbackWritePos, outL);
        feedbackBuffer.setSample (1, feedbackWritePos, outR);
        feedbackWritePos = (feedbackWritePos + 1) % feedbackBuffer.getNumSamples();
    }

    // Mix dry + wet
    for (int ch = 0; ch < 2; ++ch)
    {
        buffer.applyGain (ch, 0, numSamples, wet);
        buffer.addFrom   (ch, 0, dryBuffer, ch, 0, numSamples, dry);
    }
}

//==============================================================================
juce::AudioProcessorEditor* CrystallizerAudioProcessor::createEditor()
{
    return new CrystallizerAudioProcessorEditor (*this);
}

void CrystallizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CrystallizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CrystallizerAudioProcessor();
}
