#include "LFO.h"
#include <cmath>
void LFO::prepare(double sampleRate){ sr=sampleRate; }
void LFO::setRate(float r){ rate=r; }
void LFO::setDepth(float d){ depth=d; }
void LFO::setWave(int w){ waveType=w; }
float LFO::getNextSample(){ double inc = rate/sr; phase += inc; if(phase>=1.0) phase -= 1.0; double v=0.0; if(waveType==0) v = sin(2.0*M_PI*phase); else if(waveType==1) v = 1.0 - 4.0*fabs(phase-0.5); return (float)(v*depth); }
