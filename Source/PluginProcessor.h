#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "DSP/CompressorCore.h"
#include "DSP/HeatSink.h"
#include "DSP/SidechainFilter.h"
#include "Utils/Parameters.h"
#include "Utils/PresetManager.h"

//==============================================================================
/**
 * SeshNx Reactor - Character Compressor
 *
 * A character compressor featuring:
 * - Switchable analog modeling (VCA/FET/Opto)
 * - Response Density macro control
 * - Heat Sink soft clipper
 * - Criticality Meter for gain reduction visualization
 */
class ReactorAudioProcessor : public juce::AudioProcessor
{
public:
    ReactorAudioProcessor();
    ~ReactorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter access
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }
    Reactor::PresetManager& getPresetManager() { return presetManager; }

    // Metering data
    float getGainReductionDb() const { return currentGainReduction.load(); }
    float getInputLevel(int channel) const { return inputLevels[channel].load(); }
    float getOutputLevel(int channel) const { return outputLevels[channel].load(); }

    // Response Density state
    float getEffectiveAttack() const { return effectiveAttack.load(); }
    float getEffectiveRelease() const { return effectiveRelease.load(); }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    Reactor::PresetManager presetManager;

    // Cached parameter pointers
    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* ratioParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* kneeParam = nullptr;
    std::atomic<float>* makeupGainParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* coreMaterialParam = nullptr;
    std::atomic<float>* responseDensityParam = nullptr;
    std::atomic<float>* responseDensityEnabledParam = nullptr;
    std::atomic<float>* heatSinkParam = nullptr;
    std::atomic<float>* heatSinkEnabledParam = nullptr;
    std::atomic<float>* sidechainFreqParam = nullptr;
    std::atomic<float>* sidechainEnabledParam = nullptr;
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;

    // DSP components
    Reactor::CompressorCore compressor;
    Reactor::HeatSink heatSink;
    Reactor::SidechainFilter sidechainFilter;

    // Dry buffer for parallel compression
    juce::AudioBuffer<float> dryBuffer;

    // Sidechain buffer
    juce::AudioBuffer<float> sidechainBuffer;

    // Metering
    std::atomic<float> currentGainReduction { 0.0f };
    std::array<std::atomic<float>, 2> inputLevels = { 0.0f, 0.0f };
    std::array<std::atomic<float>, 2> outputLevels = { 0.0f, 0.0f };

    // Effective timing (for UI display when Response Density is active)
    std::atomic<float> effectiveAttack { 10.0f };
    std::atomic<float> effectiveRelease { 100.0f };

    // Smoothed parameters
    juce::SmoothedValue<float> smoothedInputGain;
    juce::SmoothedValue<float> smoothedOutputGain;
    juce::SmoothedValue<float> smoothedMakeupGain;
    juce::SmoothedValue<float> smoothedMix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReactorAudioProcessor)
};
