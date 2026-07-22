#include "Voice.h"

BeefVoice::BeefVoice()
{
    // default envelope parameters
    envParams.attack = 0.01f;
    envParams.decay = 0.1f;
    envParams.sustain = 0.8f;
    envParams.release = 0.2f;
    env.setParameters(envParams);
}

BeefVoice::~BeefVoice() = default;

bool BeefVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*> (sound) != nullptr;
}

void BeefVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    currentMidiNote = midiNoteNumber;
    float freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    osc.setFrequency(freq);
    env.noteOn();
}

void BeefVoice::stopNote (float, bool allowTailOff)
{
    env.noteOff();
    if (!allowTailOff || !env.isActive())
        clearCurrentNote();
}

void BeefVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float currentSample = osc.getNextSample() * env.getNextSample() * 0.25f; // keep safety gain
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            outputBuffer.addSample(channel, startSample + sample, currentSample);
        }
    }

    if (!env.isActive())
        clearCurrentNote();
}

void BeefVoice::prepareVoice(double sampleRate, int samplesPerBlock, int)
{
    currentSampleRate = sampleRate;
    osc.prepare(sampleRate);
    env.setSampleRate(sampleRate); // ensure ADSR uses correct rate
}
