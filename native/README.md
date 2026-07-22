Native VST3 Plugin - Wavetable Dual Synth (JUCE + CMake)

This folder contains a JUCE-based VST3 instrument project configured with CMake.

Overview
- C++ modular DSP code: Oscillator, Wavetable, Noise, Filter, Envelope, LFO, Multiband processing, Modulation Matrix
- PluginProcessor.* and PluginEditor.* files implement plugin and editor skeleton
- Parameter state is exposed via juce::AudioProcessorValueTreeState-compatible layout (simple JSON-style presets)
- Unit tests are in tests/ using Catch2

Building (CI fetches dependencies automatically)
- The included top-level CMakeLists uses FetchContent to clone JUCE and the VST3 SDK when configuring the project.
- You need a C++ toolchain and CMake installed (not included).

Example local build steps (Linux/macOS/Windows with appropriate toolchain):

mkdir build && cd build
cmake -S ../ -B .
cmake --build . --config Release

The GitHub Actions workflow (.github/workflows/ci.yml) demonstrates automated fetching of JUCE and VST3 SDK and building on multiple OSes. The workflow also runs unit tests and uploads compiled artifacts.

Notes
- Do not commit JUCE or VST3 SDK files into the repo; CI clones them.
- For plugin production builds, ensure you have the VST3 SDK license considered.
