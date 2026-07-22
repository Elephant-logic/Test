#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/Voice.h" // use beef_core's voice

// Implementation details kept private in the .cpp
struct BEEFAudioProcessor::Impl
{
    juce::Synthesiser synth;
    juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>
    > filter;

    juce::dsp::Reverb reverb;
    juce::dsp::Gain<float> masterGain;

    Impl()
        : filter(
            juce::dsp::IIR::Filter<float>(),
            juce::dsp::IIR::Coefficients<float>::makeLowPass(
                44100.0,
                15000.0f
            )
        )
    {
    }
};

BEEFAudioProcessor::BEEFAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters",
      {
          std::make_unique<juce::AudioParameterFloat>("masterGain", "Master Gain", juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), -6.0f),
          std::make_unique<juce::AudioParameterFloat>("oscDetune", "Osc Detune (cents)", -1200.0f, 1200.0f, 0.0f),
          std::make_unique<juce::AudioParameterChoice>("waveform", "Waveform", juce::StringArray{"Sine","Saw","Square"}, 0)
      }),
      impl(std::make_unique<Impl>())
{
}

BEEFAudioProcessor::~BEEFAudioProcessor() = default;

const juce::String BEEFAudioProcessor::getName() const { return "BEEF"; }

void BEEFAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare synth voices and DSP if needed
    // Ensure voices are prepared if synth populated elsewhere
    for (int i = 0; i < impl->synth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<BeefVoice*>(impl->synth.getVoice(i)))
            v->prepareVoice(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    }

    // Prepare master gain and reverb if used
    juce::dsp::ProcessSpec spec{ sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)getTotalNumOutputChannels() };
    impl->masterGain.prepare(spec);
    impl->reverb.prepare(spec);
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

    // Minimal synth processing could be added here using impl->synth if voices and sounds are setup.
}

bool BEEFAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* BEEFAudioProcessor::createEditor()
{
    return new BEEFPluginEditor (*this);
}

// Program and state implementations as requested

double BEEFAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BEEFAudioProcessor::getNumPrograms()
{
    return 1;
}

int BEEFAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BEEFAudioProcessor::setCurrentProgram(int)
{
}

const juce::String BEEFAudioProcessor::getProgramName(int)
{
    return {};
}

void BEEFAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void BEEFAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void BEEFAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

// createPluginFilter as requested
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BEEFAudioProcessor();
}


bool BEEFAudioProcessor::acceptsMidi() const { return true; }
bool BEEFAudioProcessor::producesMidi() const { return false; }
bool BEEFAudioProcessor::isMidiEffect() const { return false; }
