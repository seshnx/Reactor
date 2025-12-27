#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ReactorLookAndFeel.h"

namespace Reactor
{

//==============================================================================
/**
 * Core Material Switch
 *
 * Three-position selector for VCA/FET/OPTO compression modes
 * with industrial toggle switch aesthetic.
 */
class CoreMaterialSwitch : public juce::Component
{
public:
    CoreMaterialSwitch()
    {
        // Mode buttons
        vcaButton.setButtonText("VCA");
        vcaButton.setRadioGroupId(1);
        vcaButton.setClickingTogglesState(true);
        vcaButton.setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(vcaButton);

        fetButton.setButtonText("FET");
        fetButton.setRadioGroupId(1);
        fetButton.setClickingTogglesState(true);
        addAndMakeVisible(fetButton);

        optoButton.setButtonText("OPTO");
        optoButton.setRadioGroupId(1);
        optoButton.setClickingTogglesState(true);
        addAndMakeVisible(optoButton);

        // Title
        titleLabel.setText("CORE MATERIAL", juce::dontSendNotification);
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setColour(juce::Label::textColourId, ReactorLookAndFeel::Colors::textNormal);
        addAndMakeVisible(titleLabel);

        // Description
        descLabel.setJustificationType(juce::Justification::centred);
        descLabel.setColour(juce::Label::textColourId, ReactorLookAndFeel::Colors::textDim);
        addAndMakeVisible(descLabel);

        updateDescription();

        vcaButton.onClick = [this]() { updateDescription(); if (onChange) onChange(); };
        fetButton.onClick = [this]() { updateDescription(); if (onChange) onChange(); };
        optoButton.onClick = [this]() { updateDescription(); if (onChange) onChange(); };
    }

    int getSelectedMode() const
    {
        if (vcaButton.getToggleState()) return 0;
        if (fetButton.getToggleState()) return 1;
        return 2;
    }

    void setSelectedMode(int mode)
    {
        vcaButton.setToggleState(mode == 0, juce::dontSendNotification);
        fetButton.setToggleState(mode == 1, juce::dontSendNotification);
        optoButton.setToggleState(mode == 2, juce::dontSendNotification);
        updateDescription();
    }

    std::function<void()> onChange;

    // Access buttons for parameter attachment
    juce::Button& getVCAButton() { return vcaButton; }
    juce::Button& getFETButton() { return fetButton; }
    juce::Button& getOptoButton() { return optoButton; }

    void resized() override
    {
        auto bounds = getLocalBounds();

        titleLabel.setBounds(bounds.removeFromTop(18));

        auto buttonArea = bounds.removeFromTop(32);
        int buttonWidth = buttonArea.getWidth() / 3;
        vcaButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
        fetButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
        optoButton.setBounds(buttonArea.reduced(2));

        descLabel.setBounds(bounds);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Panel background
        g.setColour(ReactorLookAndFeel::Colors::panelDark.withAlpha(0.5f));
        g.fillRoundedRectangle(bounds, 6.0f);

        // Border with mode-specific color
        juce::Colour borderColor = ReactorLookAndFeel::Colors::alertGreen;
        if (fetButton.getToggleState())
            borderColor = ReactorLookAndFeel::Colors::alertOrange;
        else if (optoButton.getToggleState())
            borderColor = ReactorLookAndFeel::Colors::alertYellow;

        g.setColour(borderColor.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    }

private:
    void updateDescription()
    {
        juce::String desc;
        if (vcaButton.getToggleState())
            desc = "Clean & Precise";
        else if (fetButton.getToggleState())
            desc = "Fast & Aggressive";
        else
            desc = "Smooth & Musical";

        descLabel.setText(desc, juce::dontSendNotification);
        repaint();
    }

    juce::TextButton vcaButton;
    juce::TextButton fetButton;
    juce::TextButton optoButton;

    juce::Label titleLabel;
    juce::Label descLabel;
};

} // namespace Reactor
