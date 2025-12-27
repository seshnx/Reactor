#include "PresetManager.h"
#include "Parameters.h"

namespace Reactor
{

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : valueTreeState(apvts)
{
    createFactoryPresets();
}

void PresetManager::createFactoryPresets()
{
    // Helper to create a preset with specific parameter values
    auto createPreset = [this](const juce::String& name, const juce::String& category,
                               float threshold, float ratio, float attack, float release,
                               float knee, float makeup, float mix,
                               int coreMaterial, float responseDensity, bool rdEnabled,
                               float heatSink, bool hsEnabled,
                               float scFreq, bool scEnabled,
                               float inputGain, float outputGain)
    {
        auto xml = std::make_unique<juce::XmlElement>("PARAMETERS");

        xml->setAttribute(ParamIDs::threshold, threshold);
        xml->setAttribute(ParamIDs::ratio, ratio);
        xml->setAttribute(ParamIDs::attack, attack);
        xml->setAttribute(ParamIDs::release, release);
        xml->setAttribute(ParamIDs::knee, knee);
        xml->setAttribute(ParamIDs::makeupGain, makeup);
        xml->setAttribute(ParamIDs::mix, mix);
        xml->setAttribute(ParamIDs::coreMaterial, coreMaterial);
        xml->setAttribute(ParamIDs::responseDensity, responseDensity);
        xml->setAttribute(ParamIDs::responseDensityEnabled, rdEnabled ? 1 : 0);
        xml->setAttribute(ParamIDs::heatSink, heatSink);
        xml->setAttribute(ParamIDs::heatSinkEnabled, hsEnabled ? 1 : 0);
        xml->setAttribute(ParamIDs::sidechainFreq, scFreq);
        xml->setAttribute(ParamIDs::sidechainEnabled, scEnabled ? 1 : 0);
        xml->setAttribute(ParamIDs::inputGain, inputGain);
        xml->setAttribute(ParamIDs::outputGain, outputGain);

        factoryPresets.push_back({ name, category, std::move(xml) });
    };

    // Factory Presets
    // Format: name, category, threshold, ratio, attack, release, knee, makeup, mix,
    //         coreMaterial (0=VCA,1=FET,2=OPTO), responseDensity, rdEnabled,
    //         heatSink, hsEnabled, scFreq, scEnabled, inputGain, outputGain

    // 1. Init - Default neutral settings
    createPreset("Init", "Default",
        -18.0f, 4.0f, 10.0f, 100.0f, 6.0f, 0.0f, 100.0f,
        0, 50.0f, false, 0.0f, true, 100.0f, false, 0.0f, 0.0f);

    // 2. Vocal Glue - Smooth vocal compression
    createPreset("Vocal Glue", "Vocals",
        -20.0f, 3.0f, 15.0f, 150.0f, 8.0f, 3.0f, 100.0f,
        2, 40.0f, false, 15.0f, true, 100.0f, false, 0.0f, 0.0f);

    // 3. Drum Punch - Punchy drums with FET character
    createPreset("Drum Punch", "Drums",
        -15.0f, 6.0f, 5.0f, 80.0f, 4.0f, 4.0f, 100.0f,
        1, 70.0f, true, 20.0f, true, 80.0f, true, 0.0f, 0.0f);

    // 4. Bass Control - Tight bass compression
    createPreset("Bass Control", "Bass",
        -18.0f, 4.0f, 20.0f, 200.0f, 10.0f, 2.0f, 100.0f,
        2, 30.0f, false, 10.0f, true, 80.0f, true, 0.0f, 0.0f);

    // 5. Mix Bus - Gentle mix bus glue
    createPreset("Mix Bus", "Master",
        -12.0f, 2.5f, 30.0f, 300.0f, 12.0f, 1.5f, 100.0f,
        0, 25.0f, false, 5.0f, true, 100.0f, false, 0.0f, 0.0f);

    // 6. Brick Wall - Limiting
    createPreset("Brick Wall", "Master",
        -6.0f, 20.0f, 0.5f, 50.0f, 0.0f, 6.0f, 100.0f,
        0, 90.0f, true, 0.0f, true, 100.0f, false, 0.0f, 0.0f);

    // 7. Parallel Crush - Heavy parallel compression
    createPreset("Parallel Crush", "Creative",
        -30.0f, 10.0f, 1.0f, 40.0f, 2.0f, 12.0f, 40.0f,
        1, 85.0f, true, 40.0f, true, 100.0f, false, 3.0f, 0.0f);

    // 8. Opto Smooth - LA-2A style smooth compression
    createPreset("Opto Smooth", "Vocals",
        -25.0f, 3.5f, 50.0f, 500.0f, 15.0f, 4.0f, 100.0f,
        2, 20.0f, false, 0.0f, true, 100.0f, false, 0.0f, 0.0f);

    // 9. FET Attack - 1176-style aggressive compression
    createPreset("FET Attack", "Drums",
        -20.0f, 8.0f, 0.5f, 60.0f, 3.0f, 6.0f, 100.0f,
        1, 80.0f, true, 25.0f, true, 80.0f, true, 0.0f, 0.0f);

    // 10. Clean Mastering - Transparent VCA mastering
    createPreset("Clean Master", "Master",
        -10.0f, 2.0f, 25.0f, 250.0f, 18.0f, 1.0f, 100.0f,
        0, 35.0f, false, 0.0f, false, 100.0f, false, 0.0f, 0.0f);

    // 11. Snare Crack - Snappy snare compression
    createPreset("Snare Crack", "Drums",
        -18.0f, 5.0f, 2.0f, 50.0f, 2.0f, 5.0f, 100.0f,
        1, 75.0f, true, 30.0f, true, 150.0f, true, 0.0f, 0.0f);

    // 12. Warm Saturation - Gentle warmth with Heat Sink
    createPreset("Warm Saturation", "Creative",
        -16.0f, 3.0f, 15.0f, 120.0f, 8.0f, 2.0f, 100.0f,
        2, 40.0f, false, 50.0f, true, 100.0f, false, 0.0f, 0.0f);
}

void PresetManager::loadFactoryPreset(int index)
{
    if (index < 0 || index >= static_cast<int>(factoryPresets.size()))
        return;

    const auto& preset = factoryPresets[index];
    if (preset.state != nullptr)
    {
        loadPresetFromXml(*preset.state);
        currentPresetName = preset.name;
        currentPresetIndex = index;
        presetModified = false;
    }
}

void PresetManager::loadPresetFromXml(const juce::XmlElement& xml)
{
    // Load each parameter from XML
    auto loadParam = [&](const juce::String& paramId)
    {
        if (xml.hasAttribute(paramId))
        {
            if (auto* param = valueTreeState.getParameter(paramId))
            {
                float value = static_cast<float>(xml.getDoubleAttribute(paramId));
                param->setValueNotifyingHost(param->convertTo0to1(value));
            }
        }
    };

    loadParam(ParamIDs::threshold);
    loadParam(ParamIDs::ratio);
    loadParam(ParamIDs::attack);
    loadParam(ParamIDs::release);
    loadParam(ParamIDs::knee);
    loadParam(ParamIDs::makeupGain);
    loadParam(ParamIDs::mix);
    loadParam(ParamIDs::responseDensity);
    loadParam(ParamIDs::heatSink);
    loadParam(ParamIDs::sidechainFreq);
    loadParam(ParamIDs::inputGain);
    loadParam(ParamIDs::outputGain);

    // Handle choice/bool parameters
    if (xml.hasAttribute(ParamIDs::coreMaterial))
    {
        if (auto* param = valueTreeState.getParameter(ParamIDs::coreMaterial))
        {
            int value = xml.getIntAttribute(ParamIDs::coreMaterial);
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(value)));
        }
    }

    auto loadBoolParam = [&](const juce::String& paramId)
    {
        if (xml.hasAttribute(paramId))
        {
            if (auto* param = valueTreeState.getParameter(paramId))
            {
                bool value = xml.getIntAttribute(paramId) != 0;
                param->setValueNotifyingHost(value ? 1.0f : 0.0f);
            }
        }
    };

    loadBoolParam(ParamIDs::responseDensityEnabled);
    loadBoolParam(ParamIDs::heatSinkEnabled);
    loadBoolParam(ParamIDs::sidechainEnabled);
}

juce::StringArray PresetManager::getFactoryPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : factoryPresets)
        names.add(preset.name);
    return names;
}

int PresetManager::getNumFactoryPresets() const
{
    return static_cast<int>(factoryPresets.size());
}

juce::File PresetManager::getUserPresetsDirectory() const
{
    auto userDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                       .getChildFile("SeshNx")
                       .getChildFile("Reactor")
                       .getChildFile("Presets");

    if (!userDir.exists())
        userDir.createDirectory();

    return userDir;
}

juce::StringArray PresetManager::getUserPresetNames() const
{
    juce::StringArray names;
    auto presetDir = getUserPresetsDirectory();

    for (const auto& file : presetDir.findChildFiles(juce::File::findFiles, false, "*.xml"))
        names.add(file.getFileNameWithoutExtension());

    return names;
}

int PresetManager::getNumUserPresets() const
{
    return getUserPresetNames().size();
}

juce::StringArray PresetManager::getAllPresetNames() const
{
    auto names = getFactoryPresetNames();
    names.addArray(getUserPresetNames());
    return names;
}

void PresetManager::savePreset(const juce::String& presetName)
{
    auto state = valueTreeState.copyState();
    auto xml = state.createXml();

    if (xml != nullptr)
    {
        auto file = getUserPresetsDirectory().getChildFile(presetName + ".xml");
        xml->writeTo(file);
        currentPresetName = presetName;
        presetModified = false;
    }
}

void PresetManager::loadPreset(const juce::String& presetName)
{
    auto file = getUserPresetsDirectory().getChildFile(presetName + ".xml");

    if (file.existsAsFile())
    {
        if (auto xml = juce::XmlDocument::parse(file))
        {
            valueTreeState.replaceState(juce::ValueTree::fromXml(*xml));
            currentPresetName = presetName;
            presetModified = false;
        }
    }
}

void PresetManager::deletePreset(const juce::String& presetName)
{
    auto file = getUserPresetsDirectory().getChildFile(presetName + ".xml");
    if (file.existsAsFile())
        file.deleteFile();
}

void PresetManager::initializeDefaultPreset()
{
    loadFactoryPreset(0);
}

} // namespace Reactor
