#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace Reactor
{

//==============================================================================
// Compression Model Types
//==============================================================================
enum class CoreMaterial
{
    VCA = 0,    // Clean, transparent, precise
    FET,        // Fast, aggressive, punchy
    Opto        // Smooth, musical, program-dependent
};

//==============================================================================
// Parameter IDs
//==============================================================================
namespace ParamIDs
{
    // Core Compression Controls
    inline const juce::String threshold { "threshold" };
    inline const juce::String ratio { "ratio" };
    inline const juce::String attack { "attack" };
    inline const juce::String release { "release" };
    inline const juce::String knee { "knee" };
    inline const juce::String makeupGain { "makeupGain" };
    inline const juce::String mix { "mix" };

    // Unique Controls
    inline const juce::String coreMaterial { "coreMaterial" };
    inline const juce::String responseDensity { "responseDensity" };
    inline const juce::String responseDensityEnabled { "responseDensityEnabled" };
    inline const juce::String heatSink { "heatSink" };
    inline const juce::String heatSinkEnabled { "heatSinkEnabled" };

    // Sidechain
    inline const juce::String sidechainFreq { "sidechainFreq" };
    inline const juce::String sidechainEnabled { "sidechainEnabled" };

    // I/O
    inline const juce::String inputGain { "inputGain" };
    inline const juce::String outputGain { "outputGain" };
}

//==============================================================================
// Default Values
//==============================================================================
namespace Defaults
{
    // Core
    constexpr float threshold = -18.0f;     // dB
    constexpr float ratio = 4.0f;           // :1
    constexpr float attack = 10.0f;         // ms
    constexpr float release = 100.0f;       // ms
    constexpr float knee = 6.0f;            // dB
    constexpr float makeupGain = 0.0f;      // dB
    constexpr float mix = 100.0f;           // %

    // Unique
    constexpr int coreMaterial = 0;         // VCA
    constexpr float responseDensity = 50.0f;// %
    constexpr bool responseDensityEnabled = false;
    constexpr float heatSink = 0.0f;        // 0-100%
    constexpr bool heatSinkEnabled = true;

    // Sidechain
    constexpr float sidechainFreq = 100.0f; // Hz
    constexpr bool sidechainEnabled = false;

    // I/O
    constexpr float inputGain = 0.0f;
    constexpr float outputGain = 0.0f;
}

//==============================================================================
// Parameter Ranges
//==============================================================================
namespace Ranges
{
    // Threshold: -60 to 0 dB
    constexpr float thresholdMin = -60.0f;
    constexpr float thresholdMax = 0.0f;

    // Ratio: 1:1 to 20:1 (inf approximated at 20)
    constexpr float ratioMin = 1.0f;
    constexpr float ratioMax = 20.0f;

    // Attack: 0.1ms to 100ms
    constexpr float attackMin = 0.1f;
    constexpr float attackMax = 100.0f;

    // Release: 10ms to 2000ms
    constexpr float releaseMin = 10.0f;
    constexpr float releaseMax = 2000.0f;

    // Knee: 0 to 24 dB
    constexpr float kneeMin = 0.0f;
    constexpr float kneeMax = 24.0f;

    // Makeup Gain: -12 to +24 dB
    constexpr float makeupMin = -12.0f;
    constexpr float makeupMax = 24.0f;

    // Sidechain filter: 20Hz to 500Hz
    constexpr float sidechainFreqMin = 20.0f;
    constexpr float sidechainFreqMax = 500.0f;

    // General gain
    constexpr float gainMin = -24.0f;
    constexpr float gainMax = 12.0f;
}

//==============================================================================
// Response Density Timing Curves
//==============================================================================
namespace ResponseDensity
{
    // At 0% (Slow/Opto-style): Long attack, long release
    constexpr float slowAttackMs = 80.0f;
    constexpr float slowReleaseMs = 1500.0f;

    // At 100% (Fast/Transient-crushing): Ultra-fast attack, fast release
    constexpr float fastAttackMs = 0.1f;
    constexpr float fastReleaseMs = 30.0f;

    // Calculate attack from density (0-1)
    inline float getAttack(float density)
    {
        // Exponential curve for more natural feel
        float t = 1.0f - density;
        t = t * t; // Square for exponential feel
        return fastAttackMs + t * (slowAttackMs - fastAttackMs);
    }

    // Calculate release from density (0-1)
    inline float getRelease(float density)
    {
        float t = 1.0f - density;
        t = t * t;
        return fastReleaseMs + t * (slowReleaseMs - fastReleaseMs);
    }
}

//==============================================================================
// Core Material Names
//==============================================================================
namespace CoreMaterialNames
{
    inline const juce::StringArray names = {
        "VCA",
        "FET",
        "OPTO"
    };
}

//==============================================================================
// Parameter Layout Creation
//==============================================================================
inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Threshold
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::threshold, 1 },
        "Threshold",
        juce::NormalisableRange<float>(Ranges::thresholdMin, Ranges::thresholdMax, 0.1f),
        Defaults::threshold,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Ratio (skewed for more resolution at lower ratios)
    auto ratioRange = juce::NormalisableRange<float>(Ranges::ratioMin, Ranges::ratioMax, 0.1f);
    ratioRange.setSkewForCentre(4.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::ratio, 1 },
        "Ratio",
        ratioRange,
        Defaults::ratio,
        juce::AudioParameterFloatAttributes().withLabel(":1")));

    // Attack (logarithmic)
    auto attackRange = juce::NormalisableRange<float>(Ranges::attackMin, Ranges::attackMax, 0.01f);
    attackRange.setSkewForCentre(10.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::attack, 1 },
        "Attack",
        attackRange,
        Defaults::attack,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    // Release (logarithmic)
    auto releaseRange = juce::NormalisableRange<float>(Ranges::releaseMin, Ranges::releaseMax, 1.0f);
    releaseRange.setSkewForCentre(200.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::release, 1 },
        "Release",
        releaseRange,
        Defaults::release,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    // Knee
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::knee, 1 },
        "Knee",
        juce::NormalisableRange<float>(Ranges::kneeMin, Ranges::kneeMax, 0.1f),
        Defaults::knee,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Makeup Gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::makeupGain, 1 },
        "Makeup Gain",
        juce::NormalisableRange<float>(Ranges::makeupMin, Ranges::makeupMax, 0.1f),
        Defaults::makeupGain,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::mix, 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        Defaults::mix,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Core Material (VCA/FET/Opto)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParamIDs::coreMaterial, 1 },
        "Core Material",
        CoreMaterialNames::names,
        Defaults::coreMaterial));

    // Response Density
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::responseDensity, 1 },
        "Response Density",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        Defaults::responseDensity,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Response Density Enabled
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParamIDs::responseDensityEnabled, 1 },
        "Response Density Enable",
        Defaults::responseDensityEnabled));

    // Heat Sink Amount
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::heatSink, 1 },
        "Heat Sink",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        Defaults::heatSink,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Heat Sink Enabled
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParamIDs::heatSinkEnabled, 1 },
        "Heat Sink Enable",
        Defaults::heatSinkEnabled));

    // Sidechain Filter Frequency
    auto scRange = juce::NormalisableRange<float>(Ranges::sidechainFreqMin, Ranges::sidechainFreqMax, 1.0f);
    scRange.setSkewForCentre(100.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::sidechainFreq, 1 },
        "Sidechain HP",
        scRange,
        Defaults::sidechainFreq,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // Sidechain Enabled
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParamIDs::sidechainEnabled, 1 },
        "Sidechain Enable",
        Defaults::sidechainEnabled));

    // Input Gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::inputGain, 1 },
        "Input Gain",
        juce::NormalisableRange<float>(Ranges::gainMin, Ranges::gainMax, 0.1f),
        Defaults::inputGain,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Output Gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::outputGain, 1 },
        "Output Gain",
        juce::NormalisableRange<float>(Ranges::gainMin, Ranges::gainMax, 0.1f),
        Defaults::outputGain,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    return { params.begin(), params.end() };
}

} // namespace Reactor
