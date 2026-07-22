#include "PluginEditor.h"

BEEFPluginEditor::BEEFPluginEditor (BEEFAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    masterGainSlider.setSliderStyle(juce::Slider::Rotary);
    masterGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(masterGainSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.getValueTreeState(), "masterGain", masterGainSlider);

    detuneSlider.setSliderStyle(juce::Slider::Rotary);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    detuneSlider.setRange(-1200.0, 1200.0, 1.0);
    addAndMakeVisible(detuneSlider);
    detuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.getValueTreeState(), "oscDetune", detuneSlider);

    waveformBox.addItem("Sine", 1);
    waveformBox.addItem("Saw", 2);
    waveformBox.addItem("Square", 3);
    addAndMakeVisible(waveformBox);
    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.getValueTreeState(), "waveform", waveformBox);

    setSize (320, 240);
}

BEEFPluginEditor::~BEEFPluginEditor() = default;

void BEEFPluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkslategrey);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawFittedText("BEEF Synth", getLocalBounds(), juce::Justification::centredTop, 1);
}

void BEEFPluginEditor::resized()
{
    auto r = getLocalBounds().reduced(12);
    auto top = r.removeFromTop(32);
    auto area = r;
    masterGainSlider.setBounds(area.removeFromLeft(100).reduced(8));
    detuneSlider.setBounds(area.removeFromLeft(100).reduced(8));
    waveformBox.setBounds(area.removeFromLeft(120).reduced(8));
}
