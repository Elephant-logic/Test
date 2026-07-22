#include "Envelope.h"
void Envelope::prepare(double sampleRate){ sr = sampleRate; }
void Envelope::setADSR(float a,float d,float s,float r){ A=a;D=d;S=s;R=r; }
void Envelope::noteOn(){ state=1; t=0; }
void Envelope::noteOff(){ state=4; t=0; }
float Envelope::getSample(){ double dt = 1.0/sr; t += dt; switch(state){ case 0: envVal=0; break; case 1: envVal = t/A; if(t>=A){ state=2; t=0;} break; case 2: envVal = 1.0 - (1.0-S)*(t/D); if(t>=D){ state=3; t=0; } break; case 3: envVal = S; break; case 4: envVal = S * (1.0 - t/R); if(t>=R){ state=0; envVal=0; } break; } return (float)envVal; }
