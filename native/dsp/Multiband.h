#pragma once

// Only include JUCE modules required by this header
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

class MultibandProcessor
{
public:
    void prepare(double sampleRate);
    void setCrossovers(float lowMid, float midHigh);
    void process(juce::AudioBuffer<float>& buffer);
private:
    juce::dsp::IIR::Filter<float> low, mid, high;
    double sr = 44100.0;
};
