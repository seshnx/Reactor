#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ReactorLookAndFeel.h"

namespace Reactor
{

//==============================================================================
/**
 * Stereo Level Meter
 *
 * Vertical LED-style level meter with peak hold
 */
class LevelMeter : public juce::Component,
                   public juce::Timer
{
public:
    LevelMeter(const juce::String& label = "")
        : labelText(label)
    {
        startTimerHz(30);
    }

    void setLevels(float left, float right)
    {
        leftLevel = left;
        rightLevel = right;

        // Peak hold
        if (left > leftPeak)
        {
            leftPeak = left;
            leftPeakHoldTime = peakHoldMs;
        }
        if (right > rightPeak)
        {
            rightPeak = right;
            rightPeakHoldTime = peakHoldMs;
        }
    }

    void timerCallback() override
    {
        // Decay peaks
        const float decayRate = 0.95f;

        if (leftPeakHoldTime > 0)
            leftPeakHoldTime -= 33; // ~30Hz
        else
            leftPeak *= decayRate;

        if (rightPeakHoldTime > 0)
            rightPeakHoldTime -= 33;
        else
            rightPeak *= decayRate;

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(ReactorLookAndFeel::Colors::panelDark.darker(0.3f));
        g.fillRoundedRectangle(bounds, 4.0f);

        // Label
        if (labelText.isNotEmpty())
        {
            g.setColour(ReactorLookAndFeel::Colors::textDim);
            g.setFont(juce::Font(juce::FontOptions(9.0f).withStyle("Bold")));
            g.drawText(labelText, bounds.removeFromTop(14), juce::Justification::centred);
        }

        bounds = bounds.reduced(4, 2);

        // Meter bars area
        float barWidth = (bounds.getWidth() - 4) / 2.0f;
        auto leftBounds = bounds.removeFromLeft(barWidth);
        bounds.removeFromLeft(4); // Gap between meters
        auto rightBounds = bounds;

        // Draw meter backgrounds
        g.setColour(ReactorLookAndFeel::Colors::panelDark);
        g.fillRoundedRectangle(leftBounds, 2.0f);
        g.fillRoundedRectangle(rightBounds, 2.0f);

        // Draw levels
        drawMeterBar(g, leftBounds, leftLevel, leftPeak);
        drawMeterBar(g, rightBounds, rightLevel, rightPeak);

        // Border
        g.setColour(ReactorLookAndFeel::Colors::panelLight.withAlpha(0.5f));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 4.0f, 1.0f);
    }

private:
    void drawMeterBar(juce::Graphics& g, juce::Rectangle<float> bounds, float level, float peak)
    {
        bounds = bounds.reduced(1);
        float height = bounds.getHeight();

        // Clamp level to 0-1 (with some headroom display)
        float displayLevel = juce::jlimit(0.0f, 1.2f, level);
        float displayPeak = juce::jlimit(0.0f, 1.2f, peak);

        // Level bar with gradient colors
        float levelHeight = height * juce::jmin(1.0f, displayLevel);
        auto levelBounds = bounds.removeFromBottom(levelHeight);

        // Color gradient based on level
        juce::Colour barColor;
        if (displayLevel > 1.0f)
            barColor = ReactorLookAndFeel::Colors::alertRed;
        else if (displayLevel > 0.7f)
            barColor = ReactorLookAndFeel::Colors::alertYellow;
        else
            barColor = ReactorLookAndFeel::Colors::alertGreen;

        g.setColour(barColor);
        g.fillRoundedRectangle(levelBounds, 1.0f);

        // Peak indicator
        if (displayPeak > 0.01f)
        {
            float peakY = bounds.getBottom() - height * juce::jmin(1.0f, displayPeak);
            juce::Colour peakColor = displayPeak > 1.0f ? ReactorLookAndFeel::Colors::alertRed :
                                     displayPeak > 0.7f ? ReactorLookAndFeel::Colors::alertYellow :
                                     ReactorLookAndFeel::Colors::alertGreen;
            g.setColour(peakColor.brighter(0.3f));
            g.fillRect(bounds.getX(), peakY, bounds.getWidth(), 2.0f);
        }
    }

    juce::String labelText;
    float leftLevel = 0.0f;
    float rightLevel = 0.0f;
    float leftPeak = 0.0f;
    float rightPeak = 0.0f;
    int leftPeakHoldTime = 0;
    int rightPeakHoldTime = 0;
    static constexpr int peakHoldMs = 1500;
};

} // namespace Reactor
