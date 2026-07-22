#include "Filter.h"
void FilterUnit::prepare(double sampleRate){ sr = sampleRate; }
void FilterUnit::setParameters(float cutoff, float q, const juce::String& type){ juce::dsp::IIR::Coefficients<float>::Ptr coeffs;
    if(type=="lowpass") coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, cutoff, q);
    else if(type=="highpass") coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, cutoff, q);
    else coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, cutoff, q);
    iir.coefficients = *coeffs;
}
void FilterUnit::process(juce::AudioBuffer<float>& buffer){ juce::dsp::AudioBlock<float> block(buffer); juce::dsp::ProcessContextReplacing<float> ctx(block); iir.process(ctx); }
