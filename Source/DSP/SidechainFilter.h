#pragma once

#include <juce_dsp/juce_dsp.h>

namespace Reactor
{

//==============================================================================
/**
 * Sidechain Filter for compressor
 *
 * High-pass filter on the sidechain to reduce low-frequency pumping.
 * Commonly used to prevent kick drums from over-triggering compression.
 */
class SidechainFilter
{
public:
    SidechainFilter() = default;

    void prepare(double sr, int maxBlockSize)
    {
        sampleRate = sr;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sr;
        spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
        spec.numChannels = 2;

        highPassFilter.prepare(spec);
        updateFilter();
    }

    void reset()
    {
        highPassFilter.reset();
    }

    void setFrequency(float freqHz)
    {
        frequency = juce::jlimit(20.0f, 500.0f, freqHz);
        updateFilter();
    }

    void setEnabled(bool enabled)
    {
        isEnabled = enabled;
    }

    // Process the sidechain signal
    void process(juce::AudioBuffer<float>& buffer)
    {
        if (!isEnabled)
            return;

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        highPassFilter.process(context);
    }

    // Process and return filtered copy (doesn't modify input)
    juce::AudioBuffer<float> processAndCopy(const juce::AudioBuffer<float>& input)
    {
        juce::AudioBuffer<float> filtered;
        filtered.makeCopyOf(input);

        if (isEnabled)
        {
            process(filtered);
        }

        return filtered;
    }

    bool getEnabled() const { return isEnabled; }
    float getFrequency() const { return frequency; }

private:
    void updateFilter()
    {
        *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, frequency, 0.707f);
    }

    double sampleRate = 44100.0;
    float frequency = 100.0f;
    bool isEnabled = false;

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> highPassFilter;
};

} // namespace Reactor
