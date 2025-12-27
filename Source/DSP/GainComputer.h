#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace Reactor
{

//==============================================================================
/**
 * Gain Computer for dynamic range compression
 *
 * Calculates gain reduction based on:
 * - Input level (dB)
 * - Threshold (dB)
 * - Ratio (x:1)
 * - Knee width (dB)
 */
class GainComputer
{
public:
    GainComputer() = default;

    void setThreshold(float thresholdDb)
    {
        threshold = thresholdDb;
    }

    void setRatio(float r)
    {
        ratio = juce::jmax(1.0f, r);
    }

    void setKnee(float kneeDb)
    {
        kneeWidth = juce::jmax(0.0f, kneeDb);
    }

    // Compute gain reduction in dB for a given input level in dB
    float computeGainReduction(float inputLevelDb) const
    {
        float outputDb = inputLevelDb;

        if (kneeWidth > 0.0f)
        {
            // Soft knee implementation
            float kneeStart = threshold - kneeWidth / 2.0f;
            float kneeEnd = threshold + kneeWidth / 2.0f;

            if (inputLevelDb <= kneeStart)
            {
                // Below knee - no compression
                outputDb = inputLevelDb;
            }
            else if (inputLevelDb >= kneeEnd)
            {
                // Above knee - full compression
                outputDb = threshold + (inputLevelDb - threshold) / ratio;
            }
            else
            {
                // In the knee - gradual transition
                // Quadratic interpolation for smooth knee
                float kneeRatio = (inputLevelDb - kneeStart) / kneeWidth;
                float kneeGain = 1.0f + (1.0f / ratio - 1.0f) * kneeRatio * kneeRatio;

                // Calculate how much above threshold we are (mapped to knee range)
                float aboveThresh = inputLevelDb - threshold;
                outputDb = inputLevelDb - (1.0f - 1.0f / ratio) *
                           (aboveThresh + kneeWidth / 2.0f) *
                           (aboveThresh + kneeWidth / 2.0f) /
                           (2.0f * kneeWidth);
            }
        }
        else
        {
            // Hard knee
            if (inputLevelDb > threshold)
            {
                outputDb = threshold + (inputLevelDb - threshold) / ratio;
            }
        }

        // Gain reduction is the difference between input and output
        float gainReductionDb = outputDb - inputLevelDb;

        return gainReductionDb;
    }

    // Compute output level for visualization
    float computeOutputLevel(float inputLevelDb) const
    {
        return inputLevelDb + computeGainReduction(inputLevelDb);
    }

    float getThreshold() const { return threshold; }
    float getRatio() const { return ratio; }
    float getKnee() const { return kneeWidth; }

private:
    float threshold = -18.0f;   // dB
    float ratio = 4.0f;         // x:1
    float kneeWidth = 6.0f;     // dB
};

} // namespace Reactor
