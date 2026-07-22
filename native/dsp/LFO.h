#pragma once

class LFO
{
public:
    void prepare(double sampleRate);
    void setRate(float r);
    void setDepth(float d);
    void setWave(int wave);
    float getNextSample();
private:
    double sr = 44100.0; double phase = 0.0; float rate=1.0f, depth=1.0f; int waveType=0; // 0 sine
};
