#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ReactorLookAndFeel.h"

namespace Reactor
{

//==============================================================================
/**
 * Response Density Knob
 *
 * Special macro control that shows the linked Attack/Release timing
 * as the density is adjusted. Displays effective timing values.
 */
class ResponseDensityKnob : public juce::Component
{
public:
    ResponseDensityKnob()
    {
        // Main slider
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setName("Response Density");
        addAndMakeVisible(slider);

        // Enable toggle
        enableButton.setButtonText("LINK");
        enableButton.setClickingTogglesState(true);
        addAndMakeVisible(enableButton);

        // Labels
        titleLabel.setText("RESPONSE DENSITY", juce::dontSendNotification);
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setColour(juce::Label::textColourId, ReactorLookAndFeel::Colors::alertRed);
        addAndMakeVisible(titleLabel);

        attackLabel.setJustificationType(juce::Justification::centred);
        attackLabel.setColour(juce::Label::textColourId, ReactorLookAndFeel::Colors::textDim);
        addAndMakeVisible(attackLabel);

        releaseLabel.setJustificationType(juce::Justification::centred);
        releaseLabel.setColour(juce::Label::textColourId, ReactorLookAndFeel::Colors::textDim);
        addAndMakeVisible(releaseLabel);

        densityLabel.setJustificationType(juce::Justification::centred);
        densityLabel.setColour(juce::Label::textColourId, ReactorLookAndFeel::Colors::textBright);
        addAndMakeVisible(densityLabel);

        updateLabels();
        slider.onValueChange = [this]() { updateLabels(); };
    }

    juce::Slider& getSlider() { return slider; }
    juce::Button& getEnableButton() { return enableButton; }

    void setEffectiveTiming(float attackMs, float releaseMs)
    {
        effectiveAttack = attackMs;
        effectiveRelease = releaseMs;
        updateLabels();
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        titleLabel.setBounds(bounds.removeFromTop(18));
        enableButton.setBounds(bounds.removeFromBottom(24).reduced(20, 2));

        auto timingArea = bounds.removeFromBottom(30);
        attackLabel.setBounds(timingArea.removeFromLeft(timingArea.getWidth() / 2));
        releaseLabel.setBounds(timingArea);

        densityLabel.setBounds(bounds.removeFromBottom(20));
        slider.setBounds(bounds.reduced(5));
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Panel background
        g.setColour(ReactorLookAndFeel::Colors::panelDark.withAlpha(0.5f));
        g.fillRoundedRectangle(bounds, 6.0f);

        // Hazard stripes at top if enabled
        if (enableButton.getToggleState())
        {
            auto hazardArea = bounds.removeFromTop(4);
            for (float x = 0; x < hazardArea.getWidth(); x += 10)
            {
                g.setColour((static_cast<int>(x / 10) % 2 == 0)
                            ? ReactorLookAndFeel::Colors::hazardStripe
                            : ReactorLookAndFeel::Colors::panelDark);
                g.fillRect(hazardArea.getX() + x, hazardArea.getY(), 5.0f, hazardArea.getHeight());
            }
        }

        // Border
        g.setColour(enableButton.getToggleState()
                    ? ReactorLookAndFeel::Colors::alertRed.withAlpha(0.5f)
                    : ReactorLookAndFeel::Colors::panelLight);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);
    }

private:
    void updateLabels()
    {
        float density = static_cast<float>(slider.getValue());
        densityLabel.setText(juce::String(static_cast<int>(density)) + "%",
                             juce::dontSendNotification);

        attackLabel.setText("ATK: " + juce::String(effectiveAttack, 1) + "ms",
                            juce::dontSendNotification);
        releaseLabel.setText("REL: " + juce::String(effectiveRelease, 0) + "ms",
                             juce::dontSendNotification);
    }

    juce::Slider slider;
    juce::TextButton enableButton;

    juce::Label titleLabel;
    juce::Label densityLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;

    float effectiveAttack = 10.0f;
    float effectiveRelease = 100.0f;
};

} // namespace Reactor
