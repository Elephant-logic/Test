#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "Oscillator.h"

class BeefVoice : public juce::SynthesiserVoice
{
public:
    BeefVoice();
    ~BeefVoice() override;

    bool canPlaySound (juce::SynthesiserSound*) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void prepareVoice(double sampleRate, int samplesPerBlock, int outputChannels);

    // Parameter setters
    void setDetuneCents(float cents) { osc.setDetuneCents(cents); }
    void setWaveform(int w) { osc.setWaveform(static_cast<Oscillator::Wave>(w)); }

private:
    Oscillator osc;
    juce::ADSR env;
    juce::ADSR::Parameters envParams{};
    double currentSampleRate = 44100.0;
    float level = 0.0f;
    int currentMidiNote = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeefVoice)
};
