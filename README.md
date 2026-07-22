Wavetable Dual Synth - Browser Prototype + Native VST3 (JUCE/CMake)

This repository contains two parts:

1) Browser Web Audio prototype (Preview) - folder: /web
   - Playable onscreen keyboard and computer-keyboard input
   - 2 wavetable oscillators with procedural wavetable generation and morphing
   - Unison, detune, stereo width, phase, pan, sub oscillator, procedural noise
   - Multimode filter, 3-band split processing with crossover, per-band gain + drive
   - ADSR envelopes, 2 LFOs, modulation matrix, master gain and limiter
   - MIDI input support, preset save/load (localStorage / export-import)
   - Responsive dark UI, mobile-first

2) Native VST3 plugin project (JUCE + CMake) - folder: /native
   - JUCE/CMake-based VST3 instrument target
   - Clean DSP architecture with oscillators, wavetables, noise, filter, envelopes, LFOs
   - Multiband processing, parameter/state management, MIDI processing
   - Plugin editor skeleton, preset structure (JSON-based)
   - DSP unit tests (Catch2) and CI workflow to fetch JUCE and VST3 SDK, build, run tests

Notes
- No audio assets are required: all waveforms, wavetables, noise, test tones are generated procedurally.
- CI fetches JUCE and VST3 SDK automatically; toolchains/SDKs are treated as build dependencies.
- This repo is a starting point and aims to be buildable in CI (see .github/workflows/ci.yml).

Quick start (browser prototype):
- Open web/index.html in a modern browser (Chrome/Edge/Firefox/Safari). No server required.
- Use mouse/touch to play the onscreen keyboard or use computer keyboard.
- Connect a MIDI keyboard (via Web MIDI) if allowed by your browser.

Native build (high-level):
- See native/README.md for platform-specific build notes. CI is configured to fetch JUCE and VST3 SDK.

Licensing & Third-party: This project does not include third-party SDKs or binary assets. CI will clone required SDKs automatically as part of the build steps.
