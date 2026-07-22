#pragma once

#include <vector>

class WavetableOscillator
{
public:
    WavetableOscillator();
    void prepare(double sampleRate);
    void setFrequency(float f);
    void setWavetable(const std::vector<float>& table);
    void render(float* output, int numSamples);
private:
    double sampleRate = 44100.0;
    double phase = 0.0;
    double phaseIncrement = 0.0;
    std::vector<float> wavetable;
};
