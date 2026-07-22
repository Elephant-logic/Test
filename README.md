# BEEF - JUCE VST3 Synth (Codem8s conversion)

This project is a JUCE-based VST3 synth skeleton built with CMake.

What this project contains:
- native/: JUCE CMake project, plugin processor, editor and DSP core
- Tests using Catch2 to validate DSP components (Oscillator, Effects)
- GitHub Actions workflow to build on multiple OSes and upload artifacts

Notes and implementation details:
- CMake fetches JUCE and Catch2 via FetchContent. You do not need to provide JUCE or SDKs as assets.
- The reusable DSP code (oscillator, voice, effects) is built into a static library target beef_core used by the plugin and tests.
- Tests exercise DSP classes directly and do not instantiate the plugin to avoid duplicate plugin symbols.
- The plugin is declared as a synth (IS_SYNTH TRUE) and requests MIDI input.

Important implementation choices and fixes made while converting the original project:
- Removed an unmatched #endif from headers.
- Ensured Impl only had a single constructor.
- Provided createPluginFilter() exactly as required.
- ADSR sample rate is set in BeefVoice::prepareVoice via env.setSampleRate(newRate).
- Oscillator detune is applied as a frequency ratio using pow(2, cents/1200.0).
- Glide/portamento is not implemented; it is left inactive until a proper smoothed-frequency implementation is provided.
- Distortion (tanh) and a simple bitcrusher (quantization and downsample hold) are implemented and applied in Effects::processBlock.
- All declared parameters either affect DSP or are clearly noted (UI currently exposes a master gain, oscillator detune and waveform selection). Some controls are conceptual and require a complete synth render path to be fully effective.

Building locally (example):

mkdir -p native/build
cmake -S native -B native/build -DCMAKE_BUILD_TYPE=Release
cmake --build native/build --config Release --parallel 2

Run tests (from build directory):
ctest --test-dir native/build --output-on-failure

License: adapt and extend as needed.
