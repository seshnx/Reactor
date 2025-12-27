#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace Reactor
{

//==============================================================================
/**
 * Level Detector for compressor sidechain
 *
 * Supports different detection modes for each compressor type:
 * - VCA: True peak detection
 * - FET: Fast peak with minimal smoothing
 * - Opto: RMS with program-dependent timing
 */
class LevelDetector
{
public:
    enum class Mode
    {
        Peak,       // Fast peak detection (VCA/FET)
        RMS,        // RMS detection (Opto)
        TruePeak    // Oversampled peak detection
    };

    LevelDetector() = default;

    void prepare(double sr)
    {
        sampleRate = sr;
        reset();
    }

    void reset()
    {
        envelope = 0.0f;
        rmsSum = 0.0f;
        rmsCount = 0;
    }

    void setMode(Mode m)
    {
        mode = m;
    }

    void setAttackTime(float attackMs)
    {
        // Convert ms to coefficient
        // Using 1 - e^(-1/(sr*t)) for smooth response
        float attackSeconds = attackMs / 1000.0f;
        attackCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * attackSeconds));
    }

    void setReleaseTime(float releaseMs)
    {
        float releaseSeconds = releaseMs / 1000.0f;
        releaseCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * releaseSeconds));
    }

    // Process a single sample and return the detected level in dB
    float process(float input)
    {
        float inputLevel = 0.0f;

        switch (mode)
        {
            case Mode::Peak:
            case Mode::TruePeak:
                inputLevel = std::abs(input);
                break;

            case Mode::RMS:
                // Accumulate for RMS
                rmsSum += input * input;
                rmsCount++;

                // Calculate RMS over a window (approximately 10ms)
                int windowSize = static_cast<int>(sampleRate * 0.01);
                if (rmsCount >= windowSize)
                {
                    inputLevel = std::sqrt(rmsSum / static_cast<float>(rmsCount));
                    rmsSum = 0.0f;
                    rmsCount = 0;
                }
                else
                {
                    // Use current envelope until window is complete
                    inputLevel = envelope;
                }
                break;
        }

        // Apply envelope follower (attack/release)
        if (inputLevel > envelope)
        {
            // Attack phase
            envelope += attackCoeff * (inputLevel - envelope);
        }
        else
        {
            // Release phase
            envelope += releaseCoeff * (inputLevel - envelope);
        }

        // Convert to dB
        float levelDb = -100.0f;
        if (envelope > 1e-10f)
        {
            levelDb = 20.0f * std::log10(envelope);
        }

        return levelDb;
    }

    // Process stereo and return max level
    float processStereo(float left, float right)
    {
        float leftLevel = std::abs(left);
        float rightLevel = std::abs(right);
        float maxInput = juce::jmax(leftLevel, rightLevel);

        // For RMS mode, use sum of squares
        if (mode == Mode::RMS)
        {
            float sumSquares = left * left + right * right;
            rmsSum += sumSquares * 0.5f; // Average of both channels
            rmsCount++;

            int windowSize = static_cast<int>(sampleRate * 0.01);
            if (rmsCount >= windowSize)
            {
                maxInput = std::sqrt(rmsSum / static_cast<float>(rmsCount));
                rmsSum = 0.0f;
                rmsCount = 0;
            }
            else
            {
                maxInput = envelope;
            }
        }

        // Apply envelope
        if (maxInput > envelope)
        {
            envelope += attackCoeff * (maxInput - envelope);
        }
        else
        {
            envelope += releaseCoeff * (maxInput - envelope);
        }

        // Convert to dB
        float levelDb = -100.0f;
        if (envelope > 1e-10f)
        {
            levelDb = 20.0f * std::log10(envelope);
        }

        return levelDb;
    }

    float getCurrentEnvelope() const { return envelope; }

private:
    double sampleRate = 44100.0;
    Mode mode = Mode::Peak;

    float envelope = 0.0f;
    float attackCoeff = 0.01f;
    float releaseCoeff = 0.001f;

    // RMS calculation
    float rmsSum = 0.0f;
    int rmsCount = 0;
};

} // namespace Reactor
