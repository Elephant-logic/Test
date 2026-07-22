#include "PluginEditor.h"

WavetableDualSynthAudioProcessorEditor::WavetableDualSynthAudioProcessorEditor (WavetableDualSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (600, 400);
}

WavetableDualSynthAudioProcessorEditor::~WavetableDualSynthAudioProcessorEditor(){}

void WavetableDualSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Wavetable Dual Synth (Native Plugin) - GUI Skeleton", getLocalBounds(), juce::Justification::centredTop, 1);
}

void WavetableDualSynthAudioProcessorEditor::resized(){}
