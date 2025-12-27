# SeshNx Reactor

**Character Compressor - Analog Modeling with Response Density**

A character compressor featuring switchable analog modeling (VCA/FET/Opto) and a unique Response Density macro-control for intuitive timing adjustment.

## Features

### Core Material (Compression Models)

**VCA Mode**
- Clean, transparent, precise compression
- True peak detection for accuracy
- Subtle odd harmonic saturation
- Best for: Transparent dynamics control, mastering

**FET Mode**
- Fast, aggressive, punchy compression
- Enhanced transient response (70% faster attack)
- Aggressive odd harmonic saturation with asymmetry
- Best for: Drums, vocals, adding punch and bite

**Opto Mode**
- Smooth, musical, program-dependent compression
- RMS detection for smooth response
- Warm even harmonic saturation (tube-like)
- Best for: Bass, vocals, mix bus, gentle leveling

### Response Density (Macro Control)

Links and sweeps Attack/Release between two extremes:
- **0% (Slow)**: 80ms attack, 1500ms release - Opto-style timing
- **100% (Fast)**: 0.1ms attack, 30ms release - Transient-crushing

The knob dynamically updates Attack/Release values, overriding manual settings when enabled.

### Heat Sink (Soft Clipper)

Polynomial + tanh waveshaping before output:
- 0%: Bypass
- 100%: Aggressive saturation with 5x drive
- Includes DC blocker for clean output
- Adds harmonic richness and prevents digital overs

### Criticality Meter

Large VU/LED-style gain reduction display:
- **Green** (0-6 dB GR): Normal operation
- **Yellow** (6-10 dB GR): Caution
- **Red** (10+ dB GR): Critical compression

## Controls

| Control | Range | Description |
|---------|-------|-------------|
| **Threshold** | -60 to 0 dB | Compression threshold |
| **Ratio** | 1:1 to 20:1 | Compression ratio |
| **Attack** | 0.1 to 100 ms | Attack time |
| **Release** | 10 to 2000 ms | Release time |
| **Knee** | 0 to 24 dB | Soft knee width |
| **Makeup** | -12 to +24 dB | Makeup gain |
| **Mix** | 0 to 100% | Parallel compression |
| **Core Material** | VCA/FET/OPTO | Compression character |
| **Response Density** | 0 to 100% | Attack/Release macro |
| **Heat Sink** | 0 to 100% | Soft clipper amount |
| **SC Filter** | 20 to 500 Hz | Sidechain high-pass |
| **Input/Output** | -24 to +12 dB | Gain staging |

## UI Theme

Nuclear Industrial design:
- Dark high-contrast panels
- Red/Yellow/Green alert colors
- Industrial metallic knobs
- LED-style criticality meter
- Hazard stripe accents on active controls

## Build Instructions

### Requirements
- CMake 3.22+
- C++17 compatible compiler
- JUCE 8.0.0 (downloaded automatically)

### Build Commands

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

### Output Locations
```
Reactor_artefacts/
├── Release/
│   ├── VST3/SeshNx Reactor.vst3
│   ├── AU/SeshNx Reactor.component (macOS)
│   └── Standalone/SeshNx Reactor.exe
```

## Architecture

```
Source/
├── PluginProcessor.cpp/h    # Main audio engine
├── PluginEditor.cpp/h       # UI implementation
├── DSP/
│   ├── LevelDetector.h      # Peak/RMS detection
│   ├── GainComputer.h       # Threshold/ratio/knee
│   ├── CompressorCore.h     # VCA/FET/Opto models
│   ├── HeatSink.h           # Soft clipper
│   └── SidechainFilter.h    # HP filter for sidechain
├── UI/
│   ├── ReactorLookAndFeel.h # Nuclear theme
│   ├── CriticalityMeter.h   # GR meter display
│   ├── ResponseDensityKnob.h # Macro control
│   └── CoreMaterialSwitch.h  # VCA/FET/OPTO selector
└── Utils/
    └── Parameters.h          # Parameter definitions
```

## Technical Notes

- **Model Switching**: Each mode configures detection, timing modifiers, and saturation
- **Response Density**: Uses exponential curves for natural timing feel
- **Heat Sink**: Polynomial clipping for subtle saturation, tanh for harder clipping
- **Thread Safety**: Atomic parameters, smoothed gain changes
- **Metering**: Real-time gain reduction with fast attack, slow release ballistics

---

*SeshNx Reactor v1.0.0*
*Amalia Media LLC*
