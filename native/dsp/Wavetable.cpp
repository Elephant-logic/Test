#include "Wavetable.h"
#include <cmath>
#include <random>

std::vector<float> WavetableGenerator::generateSaw(int size){
    std::vector<float> w(size);
    for(int i=0;i<size;i++) w[i] = (float)(2.0*(i/(double)size)-1.0);
    return w;
}
std::vector<float> WavetableGenerator::generateSquare(int size){
    std::vector<float> w(size);
    for(int i=0;i<size;i++) w[i] = (i < size/2) ? 1.0f : -1.0f;
    return w;
}
std::vector<float> WavetableGenerator::generateSine(int size){
    std::vector<float> w(size);
    for(int i=0;i<size;i++) w[i] = (float)std::sin(2.0*M_PI*i/size);
    return w;
}
std::vector<float> WavetableGenerator::generateTriangle(int size){
    std::vector<float> w(size);
    for(int i=0;i<size;i++){
        double t = i/(double)size;
        w[i] = (float)(2.0*std::abs(2.0*(t-std::floor(t+0.5)))-1.0);
    }
    return w;
}
std::vector<float> WavetableGenerator::generateCustom(int size, float seed){
    std::vector<float> w(size);
    std::mt19937 gen((uint32_t)(seed*100000));
    std::uniform_real_distribution<float> d(-1.0f,1.0f);
    for(int i=0;i<size;i++) w[i] = d(gen) * 0.5f + 0.5f*(float)std::sin(2.0*M_PI*i/size);
    return w;
}
