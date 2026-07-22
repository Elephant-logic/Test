#pragma once

#include <JuceHeader.h>

class BEEFAudioProcessor  : public juce::AudioProcessor
{
public:
    BEEFAudioProcessor();
    ~BEEFAudioProcessor() override;

    // AudioProcessor overrides
    const juce::String getName() const override;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    bool hasEditor() const override;

    // state
    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS& getValueTreeState() { return parameters; }

private:
    APVTS parameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BEEFAudioProcessor)
};
