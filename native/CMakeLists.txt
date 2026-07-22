cmake_minimum_required(VERSION 3.22)
project(BEEF VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(BEEF
    VERSION 1.0.0
    COMPANY_NAME "Codem8s"
    BUNDLE_ID "com.codem8s.beef"
    PLUGIN_MANUFACTURER_CODE C8ms
    PLUGIN_CODE C8s1
    FORMATS VST3 Standalone
    PRODUCT_NAME "Codem8s BEEF"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(BEEF)

target_sources(BEEF PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
    dsp/Oscillator.h
    dsp/Oscillator.cpp
    dsp/Voice.h
    dsp/Voice.cpp
    dsp/Effects.h
    dsp/Effects.cpp
)

target_compile_definitions(BEEF
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(BEEF
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
