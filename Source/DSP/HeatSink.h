#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace Reactor
{

//==============================================================================
/**
 * Heat Sink - Soft Clipper / Limiter
 *
 * Provides gentle to aggressive saturation before the output stage.
 * Uses a combination of polynomial and tanh waveshaping for
 * musical harmonic content.
 *
 * Amount: 0% = bypass, 100% = aggressive saturation
 */
class HeatSink
{
public:
    HeatSink() = default;

    void prepare(double sr)
    {
        sampleRate = sr;

        // DC blocker filter
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sr;
        spec.maximumBlockSize = 512;
        spec.numChannels = 2;

        dcBlocker.prepare(spec);
        updateDCBlocker();
    }

    void reset()
    {
        dcBlocker.reset();
    }

    void setAmount(float amountPercent)
    {
        // 0-100% maps to drive amount
        amount = juce::jlimit(0.0f, 100.0f, amountPercent) / 100.0f;
    }

    void setEnabled(bool enabled)
    {
        isEnabled = enabled;
    }

    // Process a single sample
    float process(float input)
    {
        if (!isEnabled || amount < 0.001f)
            return input;

        // Input drive based on amount
        float drive = 1.0f + amount * 4.0f; // 1x to 5x drive
        float driven = input * drive;

        // Soft clipping using combined waveshaper
        float clipped = softClip(driven);

        // Mix based on amount (more amount = more effect)
        float wetDry = 0.3f + amount * 0.7f; // At least 30% wet when enabled
        float output = input * (1.0f - wetDry) + clipped * wetDry;

        // Compensate for level increase
        float compensation = 1.0f / (1.0f + amount * 0.5f);
        output *= compensation;

        return output;
    }

    // Process stereo buffer
    void process(juce::AudioBuffer<float>& buffer)
    {
        if (!isEnabled || amount < 0.001f)
            return;

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                data[i] = process(data[i]);
            }
        }

        // Apply DC blocker
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        dcBlocker.process(context);
    }

    bool getEnabled() const { return isEnabled; }
    float getAmount() const { return amount * 100.0f; }

private:
    // Combined soft clipper: polynomial for low levels, tanh for high
    float softClip(float x) const
    {
        // Polynomial soft clipping for subtle saturation
        // y = 1.5x - 0.5x^3 (normalized cubic)
        if (std::abs(x) < 1.0f)
        {
            float x2 = x * x;
            return x * (1.5f - 0.5f * x2);
        }

        // Tanh for harder clipping at extremes
        return std::tanh(x);
    }

    void updateDCBlocker()
    {
        // High-pass at 10Hz to remove DC offset from saturation
        *dcBlocker.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, 10.0f, 0.707f);
    }

    double sampleRate = 44100.0;
    float amount = 0.0f;        // 0-1
    bool isEnabled = true;

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> dcBlocker;
};

} // namespace Reactor
