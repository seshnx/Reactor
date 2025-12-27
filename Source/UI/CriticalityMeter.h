#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ReactorLookAndFeel.h"
#include <array>

namespace Reactor
{

//==============================================================================
/**
 * Criticality Meter - VU/LED style Gain Reduction Display
 *
 * Nuclear-themed meter that shows gain reduction with color thresholds:
 * - Green: 0-6 dB GR (normal operation)
 * - Yellow: 6-10 dB GR (caution)
 * - Red: 10+ dB GR (critical)
 */
class CriticalityMeter : public juce::Component,
                         public juce::Timer
{
public:
    static constexpr int NumLEDs = 20;
    static constexpr float MaxGRdB = 24.0f;

    // Thresholds in dB of gain reduction
    static constexpr float YellowThreshold = 6.0f;
    static constexpr float RedThreshold = 10.0f;

    //==========================================================================
    CriticalityMeter()
    {
        setOpaque(false);
        startTimerHz(30);
    }

    ~CriticalityMeter() override
    {
        stopTimer();
    }

    //==========================================================================
    void setGainReduction(float grDb)
    {
        // GR is typically negative, we want to display as positive
        targetGR = std::abs(grDb);
    }

    //==========================================================================
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background panel
        g.setColour(ReactorLookAndFeel::Colors::panelDark);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Border
        g.setColour(ReactorLookAndFeel::Colors::metallic);
        g.drawRoundedRectangle(bounds, 4.0f, 2.0f);

        // Title
        g.setColour(ReactorLookAndFeel::Colors::textBright);
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        g.drawText("CRITICALITY", bounds.removeFromTop(20), juce::Justification::centred);

        // LED meter area
        auto meterArea = bounds.reduced(8, 4);
        meterArea.removeFromBottom(25); // Space for dB labels

        float ledHeight = meterArea.getHeight() / NumLEDs;
        float ledWidth = meterArea.getWidth();

        // Calculate how many LEDs should be lit
        float normalizedGR = juce::jlimit(0.0f, 1.0f, currentGR / MaxGRdB);
        int litLEDs = static_cast<int>(normalizedGR * NumLEDs);

        // Draw LEDs from bottom to top
        for (int i = 0; i < NumLEDs; ++i)
        {
            float ledY = meterArea.getBottom() - (i + 1) * ledHeight;
            auto ledBounds = juce::Rectangle<float>(meterArea.getX(), ledY, ledWidth, ledHeight - 2);

            // Determine LED color based on dB position
            float ledDb = (static_cast<float>(i + 1) / NumLEDs) * MaxGRdB;
            juce::Colour ledColor;

            if (ledDb <= YellowThreshold)
                ledColor = ReactorLookAndFeel::Colors::alertGreen;
            else if (ledDb <= RedThreshold)
                ledColor = ReactorLookAndFeel::Colors::alertYellow;
            else
                ledColor = ReactorLookAndFeel::Colors::alertRed;

            bool isLit = i < litLEDs;

            if (isLit)
            {
                // Lit LED with glow
                g.setColour(ledColor.withAlpha(0.3f));
                g.fillRoundedRectangle(ledBounds.expanded(2), 2.0f);

                g.setColour(ledColor);
                g.fillRoundedRectangle(ledBounds, 2.0f);

                // Highlight
                g.setColour(juce::Colours::white.withAlpha(0.2f));
                g.fillRoundedRectangle(ledBounds.removeFromTop(ledHeight * 0.3f), 2.0f);
            }
            else
            {
                // Unlit LED (dim)
                g.setColour(ledColor.withAlpha(0.15f));
                g.fillRoundedRectangle(ledBounds, 2.0f);
            }

            // LED border
            g.setColour(ReactorLookAndFeel::Colors::panelLight);
            g.drawRoundedRectangle(ledBounds, 2.0f, 0.5f);
        }

        // Draw dB scale
        auto scaleArea = bounds.removeFromBottom(25);
        g.setColour(ReactorLookAndFeel::Colors::textDim);
        g.setFont(juce::Font(juce::FontOptions(10.0f)));

        g.drawText("0", scaleArea.removeFromLeft(25), juce::Justification::centred);
        g.drawText("-6", scaleArea.removeFromLeft(30), juce::Justification::centred);
        g.drawText("-12", scaleArea.removeFromLeft(30), juce::Justification::centred);
        g.drawText("-24", scaleArea, juce::Justification::centredRight);

        // Current GR readout
        g.setColour(ReactorLookAndFeel::Colors::textBright);
        g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));

        juce::String grText = juce::String(-currentGR, 1) + " dB";
        auto readoutBounds = getLocalBounds().removeFromBottom(22).reduced(4, 2);
        g.drawText(grText, readoutBounds, juce::Justification::centred);
    }

    //==========================================================================
    void timerCallback() override
    {
        // Smooth meter movement
        float smoothingFactor = (targetGR > currentGR) ? 0.3f : 0.1f; // Fast attack, slow release
        currentGR = currentGR + (targetGR - currentGR) * smoothingFactor;

        // Decay target slowly when not updated
        targetGR *= 0.95f;

        repaint();
    }

private:
    float targetGR = 0.0f;
    float currentGR = 0.0f;
};

} // namespace Reactor
