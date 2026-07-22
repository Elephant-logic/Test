#pragma once

// Include only the JUCE modules actually used by this header to avoid relying on JuceHeader.h
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

class FilterUnit
{
public:
    void prepare(double sampleRate);
    void setParameters(float cutoff, float q, const juce::String& type);
    void process(juce::AudioBuffer<float>& buffer);
private:
    juce::dsp::IIR::Filter<float> iir;
    double sr = 44100.0;
};
