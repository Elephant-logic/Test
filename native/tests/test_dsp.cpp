#include <catch2/catch_test_macros.hpp>
#include "../dsp/Wavetable.h"
#include "../dsp/Oscillator.h"
#include "../dsp/Filter.h"

TEST_CASE("Wavetable generation produces expected size and range"){
    auto w = WavetableGenerator::generateSine(1024);
    REQUIRE(w.size()==1024);
    float minv = 1e9f, maxv=-1e9f;
    for(auto v: w){ minv = std::min(minv,v); maxv = std::max(maxv,v); }
    REQUIRE(minv < -0.9f);
    REQUIRE(maxv > 0.9f);
}

TEST_CASE("Oscillator render basic sanity"){
    WavetableOscillator o; o.prepare(48000);
    auto wt = WavetableGenerator::generateSaw(512); o.setWavetable(wt); o.setFrequency(440);
    std::vector<float> buf(256); o.render(buf.data(),256);
    REQUIRE(buf.size()==256);
}
