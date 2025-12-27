#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace Reactor
{

//==============================================================================
/**
 * Nuclear Industrial Look and Feel for SeshNx Reactor
 *
 * Theme: Dark industrial, high-contrast, red/yellow/green alert colors
 * Inspired by: Nuclear power plant control rooms, industrial equipment
 */
class ReactorLookAndFeel : public juce::LookAndFeel_V4
{
public:
    //==========================================================================
    // Color Palette
    //==========================================================================
    struct Colors
    {
        // Background
        static inline const juce::Colour panelDark { 0xff1a1a1a };
        static inline const juce::Colour panelMid { 0xff2a2a2a };
        static inline const juce::Colour panelLight { 0xff3a3a3a };

        // Alert colors
        static inline const juce::Colour alertGreen { 0xff00cc44 };
        static inline const juce::Colour alertYellow { 0xffffcc00 };
        static inline const juce::Colour alertRed { 0xffff3333 };
        static inline const juce::Colour alertOrange { 0xffff8800 };

        // Accents
        static inline const juce::Colour accentBlue { 0xff3399ff };
        static inline const juce::Colour accentCyan { 0xff00cccc };

        // Text
        static inline const juce::Colour textBright { 0xffeeeeee };
        static inline const juce::Colour textNormal { 0xffaaaaaa };
        static inline const juce::Colour textDim { 0xff666666 };

        // Meter colors
        static inline const juce::Colour meterGreen { 0xff00cc44 };
        static inline const juce::Colour meterYellow { 0xffffcc00 };
        static inline const juce::Colour meterRed { 0xffff3333 };
        static inline const juce::Colour meterBackground { 0xff1a1a1a };

        // Knob colors
        static inline const juce::Colour knobBackground { 0xff222222 };
        static inline const juce::Colour knobRing { 0xff444444 };
        static inline const juce::Colour knobPointer { 0xffcccccc };

        // Industrial accents
        static inline const juce::Colour hazardStripe { 0xffff8800 };
        static inline const juce::Colour metallic { 0xff555555 };
    };

    //==========================================================================
    ReactorLookAndFeel()
    {
        // Default colors
        setColour(juce::Slider::backgroundColourId, Colors::knobBackground);
        setColour(juce::Slider::thumbColourId, Colors::alertGreen);
        setColour(juce::Slider::trackColourId, Colors::alertGreen);
        setColour(juce::Slider::rotarySliderFillColourId, Colors::alertGreen);
        setColour(juce::Slider::rotarySliderOutlineColourId, Colors::knobRing);

        setColour(juce::Label::textColourId, Colors::textNormal);

        setColour(juce::TextButton::buttonColourId, Colors::panelMid);
        setColour(juce::TextButton::buttonOnColourId, Colors::alertGreen.withAlpha(0.3f));
        setColour(juce::TextButton::textColourOffId, Colors::textNormal);
        setColour(juce::TextButton::textColourOnId, Colors::alertGreen);

        setColour(juce::ComboBox::backgroundColourId, Colors::panelMid);
        setColour(juce::ComboBox::textColourId, Colors::textBright);
        setColour(juce::ComboBox::outlineColourId, Colors::knobRing);
        setColour(juce::ComboBox::arrowColourId, Colors::alertGreen);

        setColour(juce::PopupMenu::backgroundColourId, Colors::panelDark);
        setColour(juce::PopupMenu::textColourId, Colors::textBright);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::alertGreen.withAlpha(0.3f));
        setColour(juce::PopupMenu::highlightedTextColourId, Colors::alertGreen);

        setColour(juce::ToggleButton::textColourId, Colors::textNormal);
        setColour(juce::ToggleButton::tickColourId, Colors::alertGreen);
    }

    //==========================================================================
    // Rotary Slider (Industrial dial style)
    //==========================================================================
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Determine color based on slider name or value
        juce::Colour accentColor = Colors::alertGreen;
        juce::String name = slider.getName().toLowerCase();

        if (name.contains("threshold") || name.contains("ratio"))
            accentColor = Colors::alertOrange;
        else if (name.contains("heat") || name.contains("density"))
            accentColor = Colors::alertRed;
        else if (name.contains("attack") || name.contains("release"))
            accentColor = Colors::alertYellow;

        // Background circle with metallic look
        juce::ColourGradient bgGradient(
            Colors::panelLight, centreX, centreY - radius,
            Colors::panelDark, centreX, centreY + radius, false);
        g.setGradientFill(bgGradient);
        g.fillEllipse(rx, ry, rw, rw);

        // Outer ring (industrial)
        g.setColour(Colors::metallic);
        g.drawEllipse(rx, ry, rw, rw, 3.0f);

        // Inner shadow
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawEllipse(rx + 2, ry + 2, rw - 4, rw - 4, 2.0f);

        // Value arc
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY,
                               radius - 8.0f, radius - 8.0f,
                               0.0f,
                               rotaryStartAngle,
                               angle,
                               true);

        // Glow
        g.setColour(accentColor.withAlpha(0.3f));
        g.strokePath(valueArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Main arc
        g.setColour(accentColor);
        g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Pointer
        juce::Path pointer;
        auto pointerLength = radius * 0.55f;
        auto pointerThickness = 4.0f;

        pointer.addRoundedRectangle(-pointerThickness / 2, -pointerLength,
                                     pointerThickness, pointerLength, 2.0f);

        g.setColour(Colors::knobPointer);
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        // Center cap
        float capRadius = radius * 0.2f;
        juce::ColourGradient capGradient(
            Colors::metallic, centreX, centreY - capRadius,
            Colors::panelDark, centreX, centreY + capRadius, false);
        g.setGradientFill(capGradient);
        g.fillEllipse(centreX - capRadius, centreY - capRadius,
                      capRadius * 2, capRadius * 2);

        g.setColour(Colors::knobRing);
        g.drawEllipse(centreX - capRadius, centreY - capRadius,
                      capRadius * 2, capRadius * 2, 1.0f);

        // Tick marks
        g.setColour(Colors::textDim);
        int numTicks = 11;
        for (int i = 0; i < numTicks; ++i)
        {
            float tickAngle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * i / (numTicks - 1);
            float tickInnerRadius = radius - 4.0f;
            float tickOuterRadius = radius;

            float x1 = centreX + tickInnerRadius * std::sin(tickAngle);
            float y1 = centreY - tickInnerRadius * std::cos(tickAngle);
            float x2 = centreX + tickOuterRadius * std::sin(tickAngle);
            float y2 = centreY - tickOuterRadius * std::cos(tickAngle);

            g.drawLine(x1, y1, x2, y2, (i % 5 == 0) ? 2.0f : 1.0f);
        }
    }

    //==========================================================================
    // Button styling (industrial push buttons)
    //==========================================================================
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        auto cornerSize = 4.0f;

        juce::Colour baseColor = backgroundColour;
        juce::Colour borderColor = Colors::knobRing;

        if (button.getToggleState())
        {
            baseColor = Colors::alertGreen.withAlpha(0.2f);
            borderColor = Colors::alertGreen;
        }

        if (shouldDrawButtonAsHighlighted)
            baseColor = baseColor.brighter(0.1f);

        if (shouldDrawButtonAsDown)
        {
            baseColor = baseColor.darker(0.1f);
            bounds = bounds.reduced(1.0f);
        }

        // 3D effect
        g.setColour(Colors::panelLight);
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(baseColor);
        g.fillRoundedRectangle(bounds.reduced(1), cornerSize);

        g.setColour(borderColor);
        g.drawRoundedRectangle(bounds, cornerSize, 1.5f);

        // LED indicator for toggle buttons
        if (button.isToggleable())
        {
            auto ledBounds = bounds.removeFromLeft(12.0f).reduced(3.0f, bounds.getHeight() / 2 - 4);
            ledBounds.setWidth(8.0f);
            ledBounds.setHeight(8.0f);
            ledBounds.setCentre(bounds.getX() - 6, bounds.getCentreY());

            g.setColour(button.getToggleState() ? Colors::alertGreen : Colors::panelDark);
            g.fillEllipse(ledBounds);

            if (button.getToggleState())
            {
                g.setColour(Colors::alertGreen.withAlpha(0.5f));
                g.fillEllipse(ledBounds.expanded(2));
            }

            g.setColour(Colors::knobRing);
            g.drawEllipse(ledBounds, 1.0f);
        }
    }

    //==========================================================================
    // ComboBox styling
    //==========================================================================
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        juce::ignoreUnused(buttonX, buttonY, buttonW, buttonH);

        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);
        auto cornerSize = 3.0f;

        // Background
        g.setColour(isButtonDown ? Colors::panelLight : Colors::panelMid);
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(Colors::knobRing);
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

        // Arrow
        auto arrowZone = juce::Rectangle<int>(width - 20, 0, 20, height).toFloat();
        juce::Path arrow;
        arrow.addTriangle(arrowZone.getCentreX() - 4, arrowZone.getCentreY() - 2,
                          arrowZone.getCentreX() + 4, arrowZone.getCentreY() - 2,
                          arrowZone.getCentreX(), arrowZone.getCentreY() + 4);

        g.setColour(Colors::alertGreen);
        g.fillPath(arrow);
    }

    //==========================================================================
    // Label styling
    //==========================================================================
    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(juce::FontOptions(13.0f).withStyle("Bold"));
    }

    //==========================================================================
    // Fonts
    //==========================================================================
    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return juce::Font(juce::FontOptions(12.0f).withStyle("Bold"));
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return juce::Font(juce::FontOptions(12.0f).withStyle("Bold"));
    }
};

} // namespace Reactor
