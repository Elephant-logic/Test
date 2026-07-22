#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

// Simple effects: distortion (tanh) and bitcrusher
class Effects
{
public:
    Effects();
    void prepare(double sampleRate);
    void processBlock(juce::AudioBuffer<float>& buffer);

    void setDistortionAmount(float amt); // 0..1
    void setBitDepth(float bits); // 1..32
    void setDownsampleFactor(int factor); // 1..32

private:
    double sr = 44100.0;
    float distortion = 0.0f;
    float bitDepth = 32.0f;
    int downsample = 1;
    int downsampleCounter = 0;
    float lastSample = 0.0f;
};
