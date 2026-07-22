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
    // Process a single sample with the internal oscillator using a valid temporary AudioBuffer
    // Create a small temporary buffer matching the prepared channel count (2 channels as used in prepare)
    juce::AudioBuffer<float> tempBuffer(2, 1);
    tempBuffer.clear();

    juce::dsp::AudioBlock<float> block(tempBuffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Let the oscillator fill the block (one sample)
    osc.process(context);

    // Return the left channel sample (channel 0)
    return tempBuffer.getSample(0, 0);
}
