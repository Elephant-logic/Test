#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class BEEFPluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit BEEFPluginEditor (BEEFAudioProcessor&);
    ~BEEFPluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BEEFAudioProcessor& processorRef;

    juce::Slider masterGainSlider;
    juce::Slider detuneSlider;
    juce::ComboBox waveformBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BEEFPluginEditor)
};
