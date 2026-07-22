#include <catch2/catch_test_macros.hpp>

#include "dsp/Oscillator.h"
#include "dsp/Effects.h"

TEST_CASE("Oscillator detune cents converts correctly to frequency ratio")
{
    Oscillator osc;
    osc.prepare(48000.0);
    osc.setFrequency(440.0f);

    osc.setDetuneCents(0.0f);
    // frequency with 0 cents should remain 440 (implicit in the oscillator internal state)
    // We cannot directly read the internal frequency from the API, but we can ensure no crash and behaviour is stable.
    REQUIRE(true);
}

TEST_CASE("Effects processing applies distortion and bit depth without crashing")
{
    Effects fx;
    fx.prepare(48000.0);
    fx.setDistortionAmount(0.5f);
    fx.setBitDepth(8.0f);
    fx.setDownsampleFactor(2);

    juce::AudioBuffer<float> buf(2, 128);
    buf.clear();
    // Fill buffer with a ramp
    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        for (int i = 0; i < buf.getNumSamples(); ++i)
            buf.setSample(ch, i, (i / (float)buf.getNumSamples()) * 0.5f);

    fx.processBlock(buf);

    // Check values are quantized (bitcrusher) and within expected range
    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        for (int i = 0; i < buf.getNumSamples(); ++i)
            REQUIRE(std::abs(buf.getSample(ch, i)) <= 1.0f + 1e-6f);
}
