#pragma once

// Explicit JUCE module headers used by the DSP code
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class Oscillator
{
public:
    enum Wave { Sine = 0, Saw, Square };

    Oscillator();
    void prepare(double sampleRate);
    void setFrequency(float frequency);
    void setDetuneCents(float detuneCents);
    void setWaveform(Wave w);
    float getNextSample();

private:
    juce::dsp::Oscillator<float> osc;
    double sampleRate = 44100.0;
    float baseFrequency = 440.0f;
    float detuneCents = 0.0f;
    Wave waveform = Sine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oscillator)
};
