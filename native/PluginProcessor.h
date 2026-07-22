#pragma once

#include <JuceHeader.h>
#include <memory>

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

    // Program management
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    // State handling
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // state
    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS& getValueTreeState() { return parameters; }

private:
    APVTS parameters;

    struct Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BEEFAudioProcessor)
};
