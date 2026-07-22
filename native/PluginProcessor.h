#pragma once
#include <JuceHeader.h>
#include "dsp/Oscillator.h"
#include "dsp/Wavetable.h"
#include "dsp/Filter.h"
#include "dsp/Envelope.h"
#include "dsp/LFO.h"
#include "dsp/Multiband.h"
#include "dsp/ModMatrix.h"

class WavetableDualSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    WavetableDualSynthAudioProcessor();
    ~WavetableDualSynthAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "WavetableDualSynth"; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // AudioProcessor MIDI capability overrides
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

private:
    // DSP units
    WavetableOscillator oscA, oscB;
    FilterUnit filter;
    Envelope ampEnv, filterEnv;
    LFO lfo1, lfo2;
    MultibandProcessor multiband;
    ModMatrix modMatrix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableDualSynthAudioProcessor)
};
