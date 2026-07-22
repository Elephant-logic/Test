#include "Effects.h"
#include <cmath>

Effects::Effects()
{
}

void Effects::prepare(double sampleRate)
{
    sr = sampleRate;
}

void Effects::setDistortionAmount(float amt)
{
    distortion = juce::jlimit(0.0f, 1.0f, amt);
}

void Effects::setBitDepth(float bits)
{
    bitDepth = juce::jlimit(1.0f, 32.0f, bits);
}

void Effects::setDownsampleFactor(int factor)
{
    downsample = juce::jlimit(1, 32, factor);
}

void Effects::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    downsampleCounter = 0;
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Downsampling (simple hold)
        if (downsample > 1)
        {
            if (downsampleCounter == 0)
            {
                // compute new sample (use channel 0 as source for hold value)
                lastSample = buffer.getSample(0, sample);
            }
            downsampleCounter = (downsampleCounter + 1) % downsample;
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float in = (downsample > 1) ? lastSample : buffer.getSample(ch, sample);

            // Distortion: tanh-based
            if (distortion > 0.0f)
            {
                float drive = 1.0f + distortion * 20.0f;
                in = std::tanh(in * drive);
            }

            // Bitcrusher: quantize amplitude
            if (bitDepth < 32.0f)
            {
                float levels = std::pow(2.0f, bitDepth);
                in = std::round(in * levels) / levels;
            }

            buffer.setSample(ch, sample, in);
        }
    }
}
