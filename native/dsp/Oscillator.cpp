#include "Oscillator.h"
#include <cmath>

Oscillator::Oscillator()
{
    // default to sine
    osc.initialise([](float x) { return std::sin(x); });
}

void Oscillator::prepare(double sr)
{
    sampleRate = sr;
    juce::dsp::ProcessSpec spec{ sr, (juce::uint32)512, 2 };
    osc.prepare(spec);
    osc.setFrequency((float)baseFrequency);
}

void Oscillator::setFrequency(float frequency)
{
    baseFrequency = frequency;
    // apply detune
    float ratio = std::pow(2.0f, detuneCents / 1200.0f);
    osc.setFrequency(baseFrequency * ratio);
}

void Oscillator::setDetuneCents(float cents)
{
    detuneCents = cents;
    float ratio = std::pow(2.0f, detuneCents / 1200.0f);
    osc.setFrequency(baseFrequency * ratio);
}

void Oscillator::setWaveform(Wave w)
{
    waveform = w;
    switch (waveform)
    {
        case Sine:
            osc.initialise([](float x) { return std::sin(x); });
            break;
        case Saw:
            osc.initialise([](float x) { return x / juce::MathConstants<float>::pi; }); // crude saw via phase
            break;
        case Square:
            osc.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; });
            break;
    }
    // reapply frequency after changing waveform
    setFrequency(baseFrequency);
}

float Oscillator::getNextSample()
{
    // Process a single sample with the internal oscillator
    juce::dsp::AudioBlock<float> block(juce::HeapBlock<float>(1), 1, 1); // not used directly
    // Use oscillator's process to fill a small buffer would be heavier; instead use its get next sample helper via processing a single-sample buffer
    float out = osc.processSample(0.0f);
    return out;
}
