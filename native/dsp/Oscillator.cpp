#include "Oscillator.h"
WavetableOscillator::WavetableOscillator(){ }
void WavetableOscillator::prepare(double sr){ sampleRate = sr; }
void WavetableOscillator::setFrequency(float f){ phaseIncrement = f / sampleRate; }
void WavetableOscillator::setWavetable(const std::vector<float>& table){ wavetable = table; }
void WavetableOscillator::render(float* output, int numSamples){ if(wavetable.empty()) return; const int N = (int)wavetable.size(); for(int s=0;s<numSamples;s++){ double idx = phase * N; int i0 = (int)idx % N; int i1 = (i0+1)%N; double frac = idx - floor(idx); output[s] = (float)(wavetable[i0]*(1-frac) + wavetable[i1]*frac); phase += phaseIncrement; if(phase>=1.0) phase -= 1.0; } }
