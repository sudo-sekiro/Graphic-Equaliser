/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define GUI_PADDING 10
#define TITLE_HEIGHT 50
//==============================================================================
GraphicEqualiserAudioProcessorEditor::GraphicEqualiserAudioProcessorEditor (GraphicEqualiserAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    auto bounds = getLocalBounds();
    bounds = bounds.reduced(GUI_PADDING);
    auto width = bounds.getWidth();
    auto textBoxWidth = (width - 20) / 3;

    setSize (500, 450);

    addAndMakeVisible(lowGainSlider);
    addAndMakeVisible(midGainSlider);
    addAndMakeVisible(highGainSlider);

    lowGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    midGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    highGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);

    // lowGainSlider.setMinAndMaxValues(-20.0f, 20.0f);
    lowGainSlider.setRange(-20.0f, 20.0f);
    midGainSlider.setRange(-20.0f, 20.0f);
    highGainSlider.setRange(-20.0f, 20.0f);

    lowGainSlider.setNumDecimalPlacesToDisplay(2);
    midGainSlider.setNumDecimalPlacesToDisplay(2);
    highGainSlider.setNumDecimalPlacesToDisplay(2);

    lowGainSlider.setTextValueSuffix (" dB");
    midGainSlider.setTextValueSuffix (" dB");
    highGainSlider.setTextValueSuffix (" dB");

    addAndMakeVisible(lowGainLabel);
    lowGainLabel.setText("Low Gain", juce::dontSendNotification);
    addAndMakeVisible(midGainLabel);
    midGainLabel.setText("Mid Gain", juce::dontSendNotification);
    addAndMakeVisible(highGainLabel);
    highGainLabel.setText("High Gain", juce::dontSendNotification);

    addAndMakeVisible(titleLabel);
    titleLabel.setText("Graphic Equaliser", juce::dontSendNotification);

    lowGainSlider.addListener(this);
    midGainSlider.addListener(this);
    highGainSlider.addListener(this);
}

GraphicEqualiserAudioProcessorEditor::~GraphicEqualiserAudioProcessorEditor()
{
}

//==============================================================================
void GraphicEqualiserAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void GraphicEqualiserAudioProcessorEditor::resized()
{
  auto bounds = getLocalBounds();
  auto titleBounds = bounds.removeFromTop(TITLE_HEIGHT);
  bounds = bounds.reduced(GUI_PADDING);

  auto width = bounds.getWidth();
  auto height = bounds.getHeight();

  titleLabel.setBounds(titleBounds);
  titleLabel.setJustificationType(juce::Justification::centred);
  titleLabel.setFont(juce::Font(45.0f));

  lowGainSlider.setBounds(bounds.getX(), bounds.getY(), width / 3, height * 5/6);
  midGainSlider.setBounds(bounds.getX() + (width / 3), bounds.getY(), width / 3, height * 5/6);
  highGainSlider.setBounds(bounds.getX() + (2 * width / 3), bounds.getY(), width / 3, height * 5/6);

  lowGainLabel.setBounds(lowGainSlider.getX(), lowGainSlider.getY() + lowGainSlider.getHeight(), lowGainSlider.getWidth(), 30);
  midGainLabel.setBounds(midGainSlider.getX(), midGainSlider.getY() + midGainSlider.getHeight(), midGainSlider.getWidth(), 30);
  highGainLabel.setBounds(highGainSlider.getX(), highGainSlider.getY() + highGainSlider.getHeight(), highGainSlider.getWidth(), 30);

  lowGainLabel.setJustificationType(juce::Justification::centred);
  midGainLabel.setJustificationType(juce::Justification::centred);
  highGainLabel.setJustificationType(juce::Justification::centred);

  lowGainLabel.setFont(juce::Font(30.0f));
  midGainLabel.setFont(juce::Font(30.0f));
  highGainLabel.setFont(juce::Font(30.0f));
}

void GraphicEqualiserAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) {
  if (slider == &lowGainSlider) {
    audioProcessor.setLowGain(slider->getValue());
  } else if (slider == &midGainSlider) {
    audioProcessor.setMidGain(slider->getValue());
  } else if (slider == &highGainSlider) {
    audioProcessor.setHighGain(slider->getValue());
  }
}
