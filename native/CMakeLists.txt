cmake_minimum_required(VERSION 3.16)
project(WavetableDualSynth LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch JUCE
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(juce)

# Fetch VST3 SDK
FetchContent_Declare(
  vst3sdk
  GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(vst3sdk)

# Add plugin target using JUCE CMake helper
juce_add_plugin(WavetableDualSynth
  PLUGIN_MANUFACTURER_CODE Wavt
  PLUGIN_CODE        WDS1
  FORMATS VST3
  PRODUCT_NAME "Wavetable Dual Synth"
)

target_sources(WavetableDualSynth PRIVATE
  PluginProcessor.cpp
  PluginProcessor.h
  PluginEditor.cpp
  PluginEditor.h
  dsp/Oscillator.cpp
  dsp/Oscillator.h
  dsp/Wavetable.cpp
  dsp/Wavetable.h
  dsp/Filter.cpp
  dsp/Filter.h
  dsp/Envelope.cpp
  dsp/Envelope.h
  dsp/LFO.cpp
  dsp/LFO.h
  dsp/Multiband.cpp
  dsp/Multiband.h
  dsp/ModMatrix.cpp
  dsp/ModMatrix.h
)

target_compile_definitions(WavetableDualSynth PRIVATE JUCE_WEB_BROWSER=0)

target_link_libraries(WavetableDualSynth PRIVATE juce::juce_audio_utils)

# Tests
enable_testing()
add_executable(dsp_tests tests/test_dsp.cpp dsp/Oscillator.cpp dsp/Wavetable.cpp dsp/Filter.cpp dsp/Envelope.cpp dsp/LFO.cpp dsp/Multiband.cpp dsp/ModMatrix.cpp)
target_include_directories(dsp_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dsp)
# Fetch Catch2 for tests
FetchContent_Declare(catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v2.13.10)
FetchContent_MakeAvailable(catch2)
target_link_libraries(dsp_tests PRIVATE Catch2::Catch2)
add_test(NAME dsp_tests COMMAND dsp_tests)
