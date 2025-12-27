#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReactorAudioProcessor::ReactorAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("ReactorParams"),
                 Reactor::createParameterLayout()),
      presetManager(parameters)
{
    // Cache parameter pointers
    thresholdParam = parameters.getRawParameterValue(Reactor::ParamIDs::threshold);
    ratioParam = parameters.getRawParameterValue(Reactor::ParamIDs::ratio);
    attackParam = parameters.getRawParameterValue(Reactor::ParamIDs::attack);
    releaseParam = parameters.getRawParameterValue(Reactor::ParamIDs::release);
    kneeParam = parameters.getRawParameterValue(Reactor::ParamIDs::knee);
    makeupGainParam = parameters.getRawParameterValue(Reactor::ParamIDs::makeupGain);
    mixParam = parameters.getRawParameterValue(Reactor::ParamIDs::mix);
    coreMaterialParam = parameters.getRawParameterValue(Reactor::ParamIDs::coreMaterial);
    responseDensityParam = parameters.getRawParameterValue(Reactor::ParamIDs::responseDensity);
    responseDensityEnabledParam = parameters.getRawParameterValue(Reactor::ParamIDs::responseDensityEnabled);
    heatSinkParam = parameters.getRawParameterValue(Reactor::ParamIDs::heatSink);
    heatSinkEnabledParam = parameters.getRawParameterValue(Reactor::ParamIDs::heatSinkEnabled);
    sidechainFreqParam = parameters.getRawParameterValue(Reactor::ParamIDs::sidechainFreq);
    sidechainEnabledParam = parameters.getRawParameterValue(Reactor::ParamIDs::sidechainEnabled);
    inputGainParam = parameters.getRawParameterValue(Reactor::ParamIDs::inputGain);
    outputGainParam = parameters.getRawParameterValue(Reactor::ParamIDs::outputGain);
}

ReactorAudioProcessor::~ReactorAudioProcessor() {}

//==============================================================================
const juce::String ReactorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReactorAudioProcessor::acceptsMidi() const { return false; }
bool ReactorAudioProcessor::producesMidi() const { return false; }
bool ReactorAudioProcessor::isMidiEffect() const { return false; }
double ReactorAudioProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================================
int ReactorAudioProcessor::getNumPrograms() { return 1; }
int ReactorAudioProcessor::getCurrentProgram() { return 0; }
void ReactorAudioProcessor::setCurrentProgram(int) {}
const juce::String ReactorAudioProcessor::getProgramName(int) { return {}; }
void ReactorAudioProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
void ReactorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare DSP components
    compressor.prepare(sampleRate, samplesPerBlock);
    heatSink.prepare(sampleRate);
    sidechainFilter.prepare(sampleRate, samplesPerBlock);

    // Prepare buffers
    dryBuffer.setSize(2, samplesPerBlock);
    sidechainBuffer.setSize(2, samplesPerBlock);

    // Initialize smoothed values
    smoothedInputGain.reset(sampleRate, 0.02);
    smoothedOutputGain.reset(sampleRate, 0.02);
    smoothedMakeupGain.reset(sampleRate, 0.02);
    smoothedMix.reset(sampleRate, 0.05);

    smoothedInputGain.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(inputGainParam->load()));
    smoothedOutputGain.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(outputGainParam->load()));
    smoothedMakeupGain.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(makeupGainParam->load()));
    smoothedMix.setCurrentAndTargetValue(mixParam->load() / 100.0f);
}

void ReactorAudioProcessor::releaseResources()
{
    compressor.reset();
    heatSink.reset();
    sidechainFilter.reset();
}

bool ReactorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void ReactorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    // Get parameter values
    float threshold = thresholdParam->load();
    float ratio = ratioParam->load();
    float attack = attackParam->load();
    float release = releaseParam->load();
    float knee = kneeParam->load();
    float makeupGainDb = makeupGainParam->load();
    float mix = mixParam->load() / 100.0f;

    int coreMaterial = static_cast<int>(coreMaterialParam->load());
    float responseDensity = responseDensityParam->load() / 100.0f;
    bool responseDensityEnabled = responseDensityEnabledParam->load() > 0.5f;

    float heatSinkAmount = heatSinkParam->load();
    bool heatSinkEnabled = heatSinkEnabledParam->load() > 0.5f;

    float sidechainFreq = sidechainFreqParam->load();
    bool sidechainEnabled = sidechainEnabledParam->load() > 0.5f;

    float inputGainDb = inputGainParam->load();
    float outputGainDb = outputGainParam->load();

    // Update smoothed values
    smoothedInputGain.setTargetValue(juce::Decibels::decibelsToGain(inputGainDb));
    smoothedOutputGain.setTargetValue(juce::Decibels::decibelsToGain(outputGainDb));
    smoothedMakeupGain.setTargetValue(juce::Decibels::decibelsToGain(makeupGainDb));
    smoothedMix.setTargetValue(mix);

    // Apply input gain and update input meters
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        float maxLevel = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            data[i] *= smoothedInputGain.getNextValue();
            maxLevel = juce::jmax(maxLevel, std::abs(data[i]));
        }

        if (ch < 2)
            inputLevels[static_cast<size_t>(ch)].store(maxLevel);

        smoothedInputGain.setCurrentAndTargetValue(
            juce::Decibels::decibelsToGain(inputGainDb));
    }

    // Store dry signal for parallel compression
    dryBuffer.makeCopyOf(buffer);

    // Prepare sidechain (filtered copy of input)
    sidechainBuffer.makeCopyOf(buffer);
    sidechainFilter.setFrequency(sidechainFreq);
    sidechainFilter.setEnabled(sidechainEnabled);
    sidechainFilter.process(sidechainBuffer);

    // Configure compressor
    compressor.setCoreMaterial(static_cast<Reactor::CoreMaterial>(coreMaterial));
    compressor.setThreshold(threshold);
    compressor.setRatio(ratio);
    compressor.setKnee(knee);

    // Handle Response Density macro
    float effectiveAttackMs = attack;
    float effectiveReleaseMs = release;

    if (responseDensityEnabled)
    {
        effectiveAttackMs = Reactor::ResponseDensity::getAttack(responseDensity);
        effectiveReleaseMs = Reactor::ResponseDensity::getRelease(responseDensity);
    }

    compressor.setAttack(effectiveAttackMs);
    compressor.setRelease(effectiveReleaseMs);

    // Store effective timing for UI
    effectiveAttack.store(effectiveAttackMs);
    effectiveRelease.store(effectiveReleaseMs);

    // Process compression
    compressor.process(buffer, &sidechainBuffer);

    // Update gain reduction meter
    currentGainReduction.store(compressor.getGainReductionDb());

    // Apply makeup gain
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            data[i] *= smoothedMakeupGain.getNextValue();
        }
        smoothedMakeupGain.setCurrentAndTargetValue(
            juce::Decibels::decibelsToGain(makeupGainDb));
    }

    // Apply Heat Sink (soft clipper)
    heatSink.setEnabled(heatSinkEnabled);
    heatSink.setAmount(heatSinkAmount);
    heatSink.process(buffer);

    // Apply parallel mix (wet/dry)
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* wetData = buffer.getWritePointer(ch);
        const float* dryData = dryBuffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float mixValue = smoothedMix.getNextValue();
            wetData[i] = dryData[i] * (1.0f - mixValue) + wetData[i] * mixValue;
        }

        smoothedMix.setCurrentAndTargetValue(mix);
    }

    // Apply output gain and update output meters
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        float maxLevel = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            data[i] *= smoothedOutputGain.getNextValue();
            maxLevel = juce::jmax(maxLevel, std::abs(data[i]));
        }

        if (ch < 2)
            outputLevels[static_cast<size_t>(ch)].store(maxLevel);

        smoothedOutputGain.setCurrentAndTargetValue(
            juce::Decibels::decibelsToGain(outputGainDb));
    }
}

//==============================================================================
bool ReactorAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ReactorAudioProcessor::createEditor()
{
    return new ReactorAudioProcessorEditor(*this);
}

//==============================================================================
void ReactorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ReactorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReactorAudioProcessor();
}
