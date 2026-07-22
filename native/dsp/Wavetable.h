#pragma once
#include <vector>

class WavetableGenerator
{
public:
    static std::vector<float> generateSaw(int size);
    static std::vector<float> generateSquare(int size);
    static std::vector<float> generateSine(int size);
    static std::vector<float> generateTriangle(int size);
    static std::vector<float> generateCustom(int size, float seed);
};
