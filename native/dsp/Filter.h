#pragma once
#include <JuceHeader.h>

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
