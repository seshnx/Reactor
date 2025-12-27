#include "PluginEditor.h"

//==============================================================================
ReactorAudioProcessorEditor::ReactorAudioProcessorEditor(ReactorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Load company logo
    if (BinaryData::company_logo_png != nullptr && BinaryData::company_logo_pngSize > 0)
    {
        companyLogo = juce::ImageCache::getFromMemory(BinaryData::company_logo_png,
                                                       BinaryData::company_logo_pngSize);
    }

    setLookAndFeel(&reactorLookAndFeel);

    // Add main components
    addAndMakeVisible(presetSelector);
    addAndMakeVisible(criticalityMeter);
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(coreMaterialSwitch);
    addAndMakeVisible(responseDensityKnob);

    // Hidden combo for parameter attachment
    coreMaterialCombo.addItemList(Reactor::CoreMaterialNames::names, 1);
    coreMaterialCombo.setVisible(false);
    addAndMakeVisible(coreMaterialCombo);

    // Sync combo with switch
    coreMaterialSwitch.onChange = [this]() {
        coreMaterialCombo.setSelectedId(coreMaterialSwitch.getSelectedMode() + 1,
                                         juce::sendNotification);
    };

    coreMaterialCombo.onChange = [this]() {
        coreMaterialSwitch.setSelectedMode(coreMaterialCombo.getSelectedId() - 1);
    };

    setupSliders();
    setupLabels();
    setupTooltips();
    attachParameters();

    setSize(800, 500);
    setResizable(true, true);
    setResizeLimits(700, 450, 1000, 600);

    startTimerHz(30);
}

ReactorAudioProcessorEditor::~ReactorAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void ReactorAudioProcessorEditor::setupSliders()
{
    auto setupRotary = [this](juce::Slider& slider, const juce::String& name) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        slider.setName(name);
        addAndMakeVisible(slider);
    };

    setupRotary(thresholdSlider, "Threshold");
    setupRotary(ratioSlider, "Ratio");
    setupRotary(attackSlider, "Attack");
    setupRotary(releaseSlider, "Release");
    setupRotary(kneeSlider, "Knee");
    setupRotary(makeupSlider, "Makeup");
    setupRotary(mixSlider, "Mix");
    setupRotary(heatSinkSlider, "Heat Sink");
    setupRotary(sidechainSlider, "Sidechain");
    setupRotary(inputSlider, "Input");
    setupRotary(outputSlider, "Output");

    // Heat Sink button
    heatSinkButton.setClickingTogglesState(true);
    addAndMakeVisible(heatSinkButton);

    // Sidechain button
    sidechainButton.setClickingTogglesState(true);
    addAndMakeVisible(sidechainButton);
}

void ReactorAudioProcessorEditor::setupLabels()
{
    // Title (left-aligned for header)
    titleLabel.setFont(juce::Font(juce::FontOptions(24.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, Reactor::ReactorLookAndFeel::Colors::alertRed);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitleLabel.setColour(juce::Label::textColourId, Reactor::ReactorLookAndFeel::Colors::textDim);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(subtitleLabel);

    // Knob labels
    auto setupLabel = [this](juce::Label& label) {
        label.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        label.setColour(juce::Label::textColourId, Reactor::ReactorLookAndFeel::Colors::textNormal);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupLabel(thresholdLabel);
    setupLabel(ratioLabel);
    setupLabel(attackLabel);
    setupLabel(releaseLabel);
    setupLabel(kneeLabel);
    setupLabel(makeupLabel);
    setupLabel(mixLabel);
    setupLabel(inputLabel);
    setupLabel(outputLabel);
}

void ReactorAudioProcessorEditor::setupTooltips()
{
    // Main compression controls
    thresholdSlider.setTooltip("Compression threshold in dB. Signal above this level will be compressed.");
    ratioSlider.setTooltip("Compression ratio. Higher values = more compression. 20:1 is limiting.");
    attackSlider.setTooltip("How quickly compression engages. Fast = more punch, Slow = more transients.");
    releaseSlider.setTooltip("How quickly compression releases. Fast = pumping, Slow = smoother.");
    kneeSlider.setTooltip("Soft knee width in dB. Higher = gentler transition into compression.");
    makeupSlider.setTooltip("Makeup gain to compensate for volume loss from compression.");
    mixSlider.setTooltip("Parallel compression mix. 100% = fully compressed, lower = blend with dry.");

    // Heat Sink
    heatSinkSlider.setTooltip("Soft clipper amount. Adds warmth and prevents harsh peaks.");
    heatSinkButton.setTooltip("Enable/disable the Heat Sink soft clipper.");

    // Sidechain
    sidechainSlider.setTooltip("Sidechain high-pass filter frequency. Prevents low frequencies from triggering compression.");
    sidechainButton.setTooltip("Enable/disable the sidechain high-pass filter.");

    // I/O
    inputSlider.setTooltip("Input gain in dB. Adjusts signal level before compression.");
    outputSlider.setTooltip("Output gain in dB. Final volume adjustment.");
}

void ReactorAudioProcessorEditor::attachParameters()
{
    auto& params = audioProcessor.getParameters();

    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::threshold, thresholdSlider);

    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::ratio, ratioSlider);

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::attack, attackSlider);

    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::release, releaseSlider);

    kneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::knee, kneeSlider);

    makeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::makeupGain, makeupSlider);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::mix, mixSlider);

    responseDensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::responseDensity, responseDensityKnob.getSlider());

    responseDensityEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, Reactor::ParamIDs::responseDensityEnabled, responseDensityKnob.getEnableButton());

    heatSinkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::heatSink, heatSinkSlider);

    heatSinkEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, Reactor::ParamIDs::heatSinkEnabled, heatSinkButton);

    sidechainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::sidechainFreq, sidechainSlider);

    sidechainEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, Reactor::ParamIDs::sidechainEnabled, sidechainButton);

    inputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::inputGain, inputSlider);

    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Reactor::ParamIDs::outputGain, outputSlider);

    coreMaterialAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, Reactor::ParamIDs::coreMaterial, coreMaterialCombo);
}

//==============================================================================
void ReactorAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(Reactor::ReactorLookAndFeel::Colors::panelDark);

    // Industrial grid pattern
    g.setColour(Reactor::ReactorLookAndFeel::Colors::panelMid.withAlpha(0.3f));
    for (int x = 0; x < getWidth(); x += 20)
        g.drawVerticalLine(x, 0, static_cast<float>(getHeight()));
    for (int y = 0; y < getHeight(); y += 20)
        g.drawHorizontalLine(y, 0, static_cast<float>(getWidth()));

    // Header bar background
    const int headerHeight = 55;
    auto headerArea = getLocalBounds().removeFromTop(headerHeight);
    g.setColour(Reactor::ReactorLookAndFeel::Colors::panelDark.darker(0.3f));
    g.fillRect(headerArea);

    // Header border line
    g.setColour(Reactor::ReactorLookAndFeel::Colors::alertRed.withAlpha(0.5f));
    g.drawLine(0.0f, static_cast<float>(headerHeight), static_cast<float>(getWidth()),
               static_cast<float>(headerHeight), 1.5f);
}

void ReactorAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    // Draw company logo centered in header
    if (!companyLogo.isValid())
        return;

    const int headerHeight = 55;
    const float logoHeight = 35.0f;
    const float logoAspect = static_cast<float>(companyLogo.getWidth()) /
                             static_cast<float>(companyLogo.getHeight());
    const float logoWidth = logoHeight * logoAspect;

    const float logoX = (getWidth() - logoWidth) * 0.5f;
    const float logoY = (headerHeight - logoHeight) * 0.5f;

    juce::Rectangle<float> logoBounds(logoX, logoY, logoWidth, logoHeight);
    g.drawImage(companyLogo, logoBounds, juce::RectanglePlacement::centred);
}

void ReactorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    int padding = 10;
    int knobSize = 75;
    int smallKnobSize = 65;
    const int headerHeight = 55;

    // Header area with title on left, preset selector on right
    auto headerArea = bounds.removeFromTop(headerHeight);

    auto titleSection = headerArea.removeFromLeft(180).reduced(20, 0);
    titleLabel.setBounds(titleSection.removeFromTop(30).withTrimmedTop(8));
    subtitleLabel.setBounds(titleSection.removeFromTop(18));

    // Preset selector on right side of header
    auto presetArea = headerArea.removeFromRight(220).reduced(16, 12);
    presetSelector.setBounds(presetArea);

    // Main layout: Left (meters) | Center (controls) | Right (special controls)

    // Left: Level Meters and Criticality Meter
    auto leftArea = bounds.removeFromLeft(140).reduced(padding);

    // Input meter on top-left
    inputMeter.setBounds(leftArea.removeFromLeft(35));
    leftArea.removeFromLeft(4);

    // Criticality meter in center-left
    criticalityMeter.setBounds(leftArea.removeFromLeft(60));
    leftArea.removeFromLeft(4);

    // Output meter on right of left section
    outputMeter.setBounds(leftArea);

    // Right: Special controls (Core Material, Response Density, Heat Sink)
    auto rightArea = bounds.removeFromRight(180).reduced(padding);

    coreMaterialSwitch.setBounds(rightArea.removeFromTop(75));
    rightArea.removeFromTop(padding);

    responseDensityKnob.setBounds(rightArea.removeFromTop(150));
    rightArea.removeFromTop(padding);

    // Heat Sink section
    auto heatSinkArea = rightArea.removeFromTop(90);
    heatSinkButton.setBounds(heatSinkArea.removeFromTop(24).reduced(10, 0));
    heatSinkSlider.setBounds(heatSinkArea.reduced(10, 0));

    // Sidechain section
    auto sidechainArea = rightArea.removeFromTop(90);
    sidechainButton.setBounds(sidechainArea.removeFromTop(24).reduced(10, 0));
    sidechainSlider.setBounds(sidechainArea.reduced(10, 0));

    // Center: Main compression controls
    auto centerArea = bounds.reduced(padding);

    // Row 1: Threshold, Ratio, Attack, Release
    auto row1 = centerArea.removeFromTop(knobSize + 20);
    int row1KnobWidth = row1.getWidth() / 4;

    auto threshArea = row1.removeFromLeft(row1KnobWidth);
    thresholdLabel.setBounds(threshArea.removeFromTop(16));
    thresholdSlider.setBounds(threshArea);

    auto ratioArea = row1.removeFromLeft(row1KnobWidth);
    ratioLabel.setBounds(ratioArea.removeFromTop(16));
    ratioSlider.setBounds(ratioArea);

    auto attackArea = row1.removeFromLeft(row1KnobWidth);
    attackLabel.setBounds(attackArea.removeFromTop(16));
    attackSlider.setBounds(attackArea);

    auto releaseArea = row1;
    releaseLabel.setBounds(releaseArea.removeFromTop(16));
    releaseSlider.setBounds(releaseArea);

    centerArea.removeFromTop(padding);

    // Row 2: Knee, Makeup, Mix, Input, Output
    auto row2 = centerArea.removeFromTop(smallKnobSize + 20);
    int row2KnobWidth = row2.getWidth() / 5;

    auto kneeArea = row2.removeFromLeft(row2KnobWidth);
    kneeLabel.setBounds(kneeArea.removeFromTop(16));
    kneeSlider.setBounds(kneeArea);

    auto makeupArea = row2.removeFromLeft(row2KnobWidth);
    makeupLabel.setBounds(makeupArea.removeFromTop(16));
    makeupSlider.setBounds(makeupArea);

    auto mixArea = row2.removeFromLeft(row2KnobWidth);
    mixLabel.setBounds(mixArea.removeFromTop(16));
    mixSlider.setBounds(mixArea);

    auto inputArea = row2.removeFromLeft(row2KnobWidth);
    inputLabel.setBounds(inputArea.removeFromTop(16));
    inputSlider.setBounds(inputArea);

    auto outputArea = row2;
    outputLabel.setBounds(outputArea.removeFromTop(16));
    outputSlider.setBounds(outputArea);
}

void ReactorAudioProcessorEditor::timerCallback()
{
    // Update criticality meter
    criticalityMeter.setGainReduction(audioProcessor.getGainReductionDb());

    // Update input/output level meters
    inputMeter.setLevels(audioProcessor.getInputLevel(0), audioProcessor.getInputLevel(1));
    outputMeter.setLevels(audioProcessor.getOutputLevel(0), audioProcessor.getOutputLevel(1));

    // Update response density timing display
    responseDensityKnob.setEffectiveTiming(
        audioProcessor.getEffectiveAttack(),
        audioProcessor.getEffectiveRelease());

    // Sync core material switch with combo
    coreMaterialSwitch.setSelectedMode(coreMaterialCombo.getSelectedId() - 1);
}
