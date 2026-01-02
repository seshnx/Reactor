cmake_minimum_required(VERSION 3.22)

project(Reactor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE setup via FetchContent
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Plugin definition
juce_add_plugin(Reactor
    COMPANY_NAME "SeshNx"
    PLUGIN_MANUFACTURER_CODE Sesh
    PLUGIN_CODE Reac
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "SeshNx Reactor"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# Source files
target_sources(Reactor
    PRIVATE
        # Core
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp

        # DSP
        Source/DSP/LevelDetector.cpp
        Source/DSP/GainComputer.cpp
        Source/DSP/CompressorCore.cpp
        Source/DSP/HeatSink.cpp
        Source/DSP/SidechainFilter.cpp

        # UI
        Source/UI/ReactorLookAndFeel.cpp
        Source/UI/CriticalityMeter.cpp
        Source/UI/ResponseDensityKnob.cpp
        Source/UI/CoreMaterialSwitch.cpp

        # Utils
        Source/Utils/Parameters.cpp
        Source/Utils/PresetManager.cpp
)

# Include directories
target_include_directories(Reactor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/UI
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Utils
)

# Binary resources (logo)
juce_add_binary_data(Reactor_BinaryData
    SOURCES
        Resources/company_logo.png
    HEADER_NAME BinaryData.h
)

# Compiler definitions
target_compile_definitions(Reactor
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link JUCE modules
target_link_libraries(Reactor
    PRIVATE
        Reactor_BinaryData
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
