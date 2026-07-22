#pragma once
#include <JuceHeader.h>

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
