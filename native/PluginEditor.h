#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class WavetableDualSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    WavetableDualSynthAudioProcessorEditor (WavetableDualSynthAudioProcessor&);
    ~WavetableDualSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    WavetableDualSynthAudioProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavetableDualSynthAudioProcessorEditor)
};
