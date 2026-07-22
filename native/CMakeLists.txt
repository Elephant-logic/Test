cmake_minimum_required(VERSION 3.16)
project(WavetableDualSynth VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# On macOS some Objective-C/Objective-C++ sources or system frameworks may be required by JUCE
if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()

# Fetch JUCE only. Use JUCE's CMake helpers to create the plugin target and build formats.
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(juce)

# Add plugin target using JUCE CMake helper. Use JUCE's VST3 support (no external Steinberg VST3 SDK fetch).
juce_add_plugin(WavetableDualSynth
    COMPANY_NAME "Elephant Logic"
    BUNDLE_ID "com.elephantlogic.wavetabledualsynth"
    PLUGIN_MANUFACTURER_CODE Elph
    PLUGIN_CODE WvDS
    FORMATS VST3 Standalone
    PRODUCT_NAME "Wavetable Dual Synth"
)

# Generate the consolidated JuceHeader for the plugin target so includes like <JuceHeader.h> are available
juce_generate_juce_header(WavetableDualSynth)

# Plugin sources - preserve existing plugin and DSP sources
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

# Compile flags / defines
target_compile_definitions(WavetableDualSynth PRIVATE JUCE_WEB_BROWSER=0)

# Link to JUCE modules as needed
target_link_libraries(WavetableDualSynth
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Tests
enable_testing()

# Fetch Catch2 (v3) for tests
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(dsp_tests
  tests/test_dsp.cpp
  dsp/Oscillator.cpp
  dsp/Wavetable.cpp
  dsp/Filter.cpp
  dsp/Envelope.cpp
  dsp/LFO.cpp
  dsp/Multiband.cpp
  dsp/ModMatrix.cpp
)

target_include_directories(dsp_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dsp)

target_link_libraries(dsp_tests
    PRIVATE
        Catch2::Catch2WithMain
        juce::juce_dsp
        juce::juce_audio_basics
)

add_test(NAME dsp_tests COMMAND dsp_tests)
