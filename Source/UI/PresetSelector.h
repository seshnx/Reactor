#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Utils/PresetManager.h"
#include "ReactorLookAndFeel.h"

namespace Reactor
{

//==============================================================================
/**
 * Preset Selector Component
 *
 * Dropdown for selecting presets with navigation buttons
 */
class PresetSelector : public juce::Component,
                       public juce::ComboBox::Listener
{
public:
    PresetSelector(PresetManager& pm)
        : presetManager(pm)
    {
        presetBox.setTextWhenNoChoicesAvailable("No Presets");
        presetBox.setTextWhenNothingSelected("Select Preset...");
        presetBox.addListener(this);
        addAndMakeVisible(presetBox);

        // Navigation buttons
        prevButton.setButtonText("<");
        prevButton.onClick = [this]() { navigatePreset(-1); };
        addAndMakeVisible(prevButton);

        nextButton.setButtonText(">");
        nextButton.onClick = [this]() { navigatePreset(1); };
        addAndMakeVisible(nextButton);

        // Populate with presets from PresetManager
        refreshPresetList();
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        const int buttonWidth = 28;
        const int spacing = 4;

        prevButton.setBounds(bounds.removeFromLeft(buttonWidth));
        bounds.removeFromLeft(spacing);

        nextButton.setBounds(bounds.removeFromRight(buttonWidth));
        bounds.removeFromRight(spacing);

        presetBox.setBounds(bounds);
    }

    void comboBoxChanged(juce::ComboBox*) override
    {
        int selectedIndex = presetBox.getSelectedId() - 1;
        if (selectedIndex >= 0)
        {
            presetManager.loadFactoryPreset(selectedIndex);
        }
    }

    void refreshPresetList()
    {
        presetBox.clear();

        auto presetNames = presetManager.getFactoryPresetNames();
        for (int i = 0; i < presetNames.size(); ++i)
        {
            presetBox.addItem(presetNames[i], i + 1);
        }

        int currentIndex = presetManager.getCurrentPresetIndex();
        presetBox.setSelectedId(currentIndex + 1, juce::dontSendNotification);
    }

private:
    void navigatePreset(int direction)
    {
        int current = presetBox.getSelectedId();
        int next = current + direction;
        int numItems = presetBox.getNumItems();

        if (next < 1) next = numItems;
        if (next > numItems) next = 1;

        presetBox.setSelectedId(next, juce::sendNotification);
    }

    PresetManager& presetManager;
    juce::ComboBox presetBox;
    juce::TextButton prevButton, nextButton;
};

} // namespace Reactor
