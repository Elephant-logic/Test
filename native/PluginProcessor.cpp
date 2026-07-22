#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/Voice.h" // use beef_core's voice

// We keep implementation details private in the .cpp
struct BEEFAudioProcessor::Impl
{
    Impl() {}
};

BEEFAudioProcessor::BEEFAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters",
      {
          std::make_unique<juce::AudioParameterFloat>("masterGain", "Master Gain", juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), -6.0f),
          std::make_unique<juce::AudioParameterFloat>("oscDetune", "Osc Detune (cents)", -1200.0f, 1200.0f, 0.0f),
          std::make_unique<juce::AudioParameterChoice>("waveform", "Waveform", juce::StringArray{"Sine","Saw","Square"}, 0)
      })
{
}

BEEFAudioProcessor::~BEEFAudioProcessor() = default;

const juce::String BEEFAudioProcessor::getName() const { return "BEEF"; }

void BEEFAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare the synth (if present) or other processor resources
}

void BEEFAudioProcessor::releaseResources() {}

bool BEEFAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Mono/stereo supported only
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void BEEFAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float gainDb = *parameters.getRawParameterValue("masterGain");
    float gainLinear = juce::Decibels::decibelsToGain(gainDb);
    buffer.applyGain(gainLinear);

    // This minimal processor currently does not implement a full synth render here.
    // The DSP classes are available through beef_core for tests and for a more complete synth implementation.
}

bool BEEFAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* BEEFAudioProcessor::createEditor()
{
    return new BEEFPluginEditor (*this);
}

// createPluginFilter as requested
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BEEFAudioProcessor();
}


bool BEEFAudioProcessor::acceptsMidi() const { return true; }
bool BEEFAudioProcessor::producesMidi() const { return false; }
bool BEEFAudioProcessor::isMidiEffect() const { return false; }
