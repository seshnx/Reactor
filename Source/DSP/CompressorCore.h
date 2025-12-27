#pragma once

#include "LevelDetector.h"
#include "GainComputer.h"
#include "../Utils/Parameters.h"
#include <juce_dsp/juce_dsp.h>

namespace Reactor
{

//==============================================================================
/**
 * Compressor Core with switchable analog modeling
 *
 * Three distinct compression characters:
 *
 * VCA (Voltage Controlled Amplifier):
 * - Clean, transparent, precise
 * - Fast response, accurate gain reduction
 * - Minimal coloration, subtle odd harmonics
 *
 * FET (Field Effect Transistor):
 * - Fast, aggressive, punchy
 * - Characteristic "bite" on transients
 * - Aggressive saturation, odd harmonics emphasis
 *
 * Opto (Optical):
 * - Smooth, musical, program-dependent
 * - Slow attack, frequency-dependent release
 * - Warm saturation, even harmonics
 */
class CompressorCore
{
public:
    CompressorCore() = default;

    void prepare(double sr, int maxBlockSize)
    {
        sampleRate = sr;
        blockSize = maxBlockSize;

        levelDetector.prepare(sr);
        gainComputer.setThreshold(-18.0f);
        gainComputer.setRatio(4.0f);
        gainComputer.setKnee(6.0f);

        // Smoothing for gain reduction (anti-zipper)
        smoothedGainReduction.reset(sr, 0.005); // 5ms smoothing

        reset();
    }

    void reset()
    {
        levelDetector.reset();
        smoothedGainReduction.setCurrentAndTargetValue(0.0f);
        currentGainReductionDb = 0.0f;
    }

    void setCoreMaterial(CoreMaterial material)
    {
        coreMaterial = material;

        // Configure level detector mode based on material
        switch (coreMaterial)
        {
            case CoreMaterial::VCA:
                levelDetector.setMode(LevelDetector::Mode::TruePeak);
                break;

            case CoreMaterial::FET:
                levelDetector.setMode(LevelDetector::Mode::Peak);
                break;

            case CoreMaterial::Opto:
                levelDetector.setMode(LevelDetector::Mode::RMS);
                break;
        }
    }

    void setThreshold(float thresholdDb)
    {
        gainComputer.setThreshold(thresholdDb);
    }

    void setRatio(float ratio)
    {
        gainComputer.setRatio(ratio);
    }

    void setKnee(float kneeDb)
    {
        gainComputer.setKnee(kneeDb);
    }

    void setAttack(float attackMs)
    {
        attackTime = attackMs;
        levelDetector.setAttackTime(getModifiedAttack());
    }

    void setRelease(float releaseMs)
    {
        releaseTime = releaseMs;
        levelDetector.setReleaseTime(getModifiedRelease());
    }

    // Get current gain reduction in dB (for metering)
    float getGainReductionDb() const
    {
        return currentGainReductionDb;
    }

    // Process a stereo buffer
    void process(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>* sidechainBuffer = nullptr)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Get input level from sidechain or main signal
            float leftIn = buffer.getSample(0, sample);
            float rightIn = (numChannels > 1) ? buffer.getSample(1, sample) : leftIn;

            float scLeft = leftIn;
            float scRight = rightIn;

            if (sidechainBuffer != nullptr && sidechainBuffer->getNumSamples() > sample)
            {
                scLeft = sidechainBuffer->getSample(0, sample);
                scRight = (sidechainBuffer->getNumChannels() > 1)
                        ? sidechainBuffer->getSample(1, sample) : scLeft;
            }

            // Detect level
            float inputLevelDb = levelDetector.processStereo(scLeft, scRight);

            // Compute gain reduction
            float gainReductionDb = gainComputer.computeGainReduction(inputLevelDb);

            // Apply model-specific modifications
            gainReductionDb = applyModelCharacter(gainReductionDb, inputLevelDb);

            // Smooth gain reduction
            smoothedGainReduction.setTargetValue(gainReductionDb);
            float smoothedGR = smoothedGainReduction.getNextValue();

            // Store for metering
            currentGainReductionDb = smoothedGR;

            // Convert to linear gain
            float gainLinear = juce::Decibels::decibelsToGain(smoothedGR);

            // Apply compression
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float input = buffer.getSample(ch, sample);
                float compressed = input * gainLinear;

                // Apply model-specific saturation
                compressed = applyModelSaturation(compressed);

                buffer.setSample(ch, sample, compressed);
            }
        }
    }

private:
    // Modify attack/release based on compressor model
    float getModifiedAttack() const
    {
        switch (coreMaterial)
        {
            case CoreMaterial::VCA:
                return attackTime; // Precise, as set

            case CoreMaterial::FET:
                // FET is slightly faster
                return attackTime * 0.7f;

            case CoreMaterial::Opto:
                // Opto is slower and more program-dependent
                return attackTime * 2.0f;
        }
        return attackTime;
    }

    float getModifiedRelease() const
    {
        switch (coreMaterial)
        {
            case CoreMaterial::VCA:
                return releaseTime;

            case CoreMaterial::FET:
                // FET has slightly faster release
                return releaseTime * 0.8f;

            case CoreMaterial::Opto:
                // Opto has much slower, frequency-dependent release
                return releaseTime * 1.5f;
        }
        return releaseTime;
    }

    // Apply model-specific gain reduction character
    float applyModelCharacter(float gainReductionDb, float inputLevelDb) const
    {
        switch (coreMaterial)
        {
            case CoreMaterial::VCA:
                // VCA: Clean, linear response
                return gainReductionDb;

            case CoreMaterial::FET:
            {
                // FET: More aggressive at higher levels, slight ratio increase
                float excessDb = juce::jmax(0.0f, inputLevelDb - gainComputer.getThreshold());
                float aggressiveness = 1.0f + excessDb * 0.02f; // Increases with level
                return gainReductionDb * aggressiveness;
            }

            case CoreMaterial::Opto:
            {
                // Opto: Softer response, level-dependent timing
                // At low levels, compression is gentler
                float levelFactor = juce::jmap(inputLevelDb,
                                               gainComputer.getThreshold() - 20.0f,
                                               gainComputer.getThreshold() + 20.0f,
                                               0.7f, 1.0f);
                levelFactor = juce::jlimit(0.7f, 1.0f, levelFactor);
                return gainReductionDb * levelFactor;
            }
        }
        return gainReductionDb;
    }

    // Apply model-specific saturation to the signal
    float applyModelSaturation(float input) const
    {
        switch (coreMaterial)
        {
            case CoreMaterial::VCA:
            {
                // VCA: Very subtle odd harmonic saturation
                // Soft asymmetric clipping
                float x = input;
                if (std::abs(x) > 0.9f)
                {
                    float sign = (x > 0) ? 1.0f : -1.0f;
                    x = sign * (0.9f + (std::abs(x) - 0.9f) * 0.1f);
                }
                return x;
            }

            case CoreMaterial::FET:
            {
                // FET: Aggressive odd harmonic saturation (1176-style)
                // Asymmetric soft clipping with more grit
                float x = input * 1.2f; // Slight drive
                float saturation = x / (1.0f + std::abs(x) * 0.3f);

                // Add slight asymmetry for FET character
                if (x > 0)
                    saturation *= 1.02f;
                else
                    saturation *= 0.98f;

                return saturation * 0.9f;
            }

            case CoreMaterial::Opto:
            {
                // Opto: Warm even harmonic saturation (LA-2A style)
                // Tube-like soft clipping
                float x = input;
                float tube = std::tanh(x * 0.8f) * 1.1f;

                // Add subtle even harmonics
                float evenHarmonic = x * x * 0.05f * (x > 0 ? 1.0f : -1.0f);

                return tube + evenHarmonic;
            }
        }
        return input;
    }

    double sampleRate = 44100.0;
    int blockSize = 512;

    CoreMaterial coreMaterial = CoreMaterial::VCA;

    LevelDetector levelDetector;
    GainComputer gainComputer;

    float attackTime = 10.0f;
    float releaseTime = 100.0f;

    juce::SmoothedValue<float> smoothedGainReduction;
    float currentGainReductionDb = 0.0f;
};

} // namespace Reactor
