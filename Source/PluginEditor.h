/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class GraphicEqualiserAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Slider::Listener
{
public:
    GraphicEqualiserAudioProcessorEditor (GraphicEqualiserAudioProcessor&);
    ~GraphicEqualiserAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GraphicEqualiserAudioProcessor& audioProcessor;
    juce::Slider lowGainSlider;
    juce::Slider midGainSlider;
    juce::Slider highGainSlider;

    juce::Label lowGainLabel;
    juce::Label midGainLabel;
    juce::Label highGainLabel;

    juce::Label titleLabel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphicEqualiserAudioProcessorEditor)
};
