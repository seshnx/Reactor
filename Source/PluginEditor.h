#pragma once

#include "PluginProcessor.h"
#include "UI/ReactorLookAndFeel.h"
#include "UI/CriticalityMeter.h"
#include "UI/ResponseDensityKnob.h"
#include "UI/CoreMaterialSwitch.h"
#include "UI/LevelMeter.h"
#include "UI/PresetSelector.h"
#include "Utils/Parameters.h"
#include "BinaryData.h"

//==============================================================================
/**
 * SeshNx Reactor Plugin Editor
 *
 * Nuclear industrial themed UI with:
 * - Large Criticality Meter for gain reduction
 * - Core Material switch (VCA/FET/OPTO)
 * - Response Density macro control
 * - Heat Sink soft clipper control
 */
class ReactorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
public:
    ReactorAudioProcessorEditor(ReactorAudioProcessor&);
    ~ReactorAudioProcessorEditor() override;

    //==========================================================================
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    ReactorAudioProcessor& audioProcessor;

    // Company logo
    juce::Image companyLogo;

    // Custom look and feel
    Reactor::ReactorLookAndFeel reactorLookAndFeel;

    // Preset selector
    Reactor::PresetSelector presetSelector { audioProcessor.getPresetManager() };

    // Main meter
    Reactor::CriticalityMeter criticalityMeter;

    // Input/Output level meters
    Reactor::LevelMeter inputMeter { "IN" };
    Reactor::LevelMeter outputMeter { "OUT" };

    // Core Material switch
    Reactor::CoreMaterialSwitch coreMaterialSwitch;

    // Response Density
    Reactor::ResponseDensityKnob responseDensityKnob;

    // Standard knobs
    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider kneeSlider;
    juce::Slider makeupSlider;
    juce::Slider mixSlider;

    // Heat Sink
    juce::Slider heatSinkSlider;
    juce::TextButton heatSinkButton { "HEAT SINK" };

    // Sidechain
    juce::Slider sidechainSlider;
    juce::TextButton sidechainButton { "SC FILTER" };

    // I/O
    juce::Slider inputSlider;
    juce::Slider outputSlider;

    // Labels
    juce::Label titleLabel { {}, "REACTOR" };
    juce::Label subtitleLabel { {}, "CHARACTER COMPRESSOR" };
    juce::Label thresholdLabel { {}, "THRESHOLD" };
    juce::Label ratioLabel { {}, "RATIO" };
    juce::Label attackLabel { {}, "ATTACK" };
    juce::Label releaseLabel { {}, "RELEASE" };
    juce::Label kneeLabel { {}, "KNEE" };
    juce::Label makeupLabel { {}, "MAKEUP" };
    juce::Label mixLabel { {}, "MIX" };
    juce::Label inputLabel { {}, "INPUT" };
    juce::Label outputLabel { {}, "OUTPUT" };

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> makeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> responseDensityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> responseDensityEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> heatSinkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> heatSinkEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sidechainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sidechainEnabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> coreMaterialAttachment;

    // For core material (using combo internally)
    juce::ComboBox coreMaterialCombo;

    //==========================================================================
    void setupSliders();
    void setupLabels();
    void setupTooltips();
    void attachParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReactorAudioProcessorEditor)
};
