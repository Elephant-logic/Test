#pragma once

class Envelope
{
public:
    void prepare(double sampleRate);
    void setADSR(float a, float d, float s, float r);
    float getSample();
    void noteOn();
    void noteOff();
private:
    double sr=44100.0;
    float A=0.01f,D=0.1f,S=0.9f,R=0.2f;
    int state=0; // 0 idle,1 attack,2 decay,3 sustain,4 release
    double envVal=0.0; double t=0.0;
};
