/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class GraphicEqualiserAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    GraphicEqualiserAudioProcessor();
    ~GraphicEqualiserAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    juce::IIRCoefficients* coeffs;

    juce::AudioParameterFloat* low_gainParameter;
    juce::AudioParameterFloat* low_frequencyParameter;
    juce::AudioParameterFloat* low_qParameter;

    juce::AudioParameterFloat* mid_gainParameter;
    juce::AudioParameterFloat* mid_frequencyParameter;
    juce::AudioParameterFloat* mid_qParameter;

    juce::AudioParameterFloat* high_gainParameter;
    juce::AudioParameterFloat* high_frequencyParameter;
    juce::AudioParameterFloat* high_qParameter;

    using Filter = juce::dsp::IIR::Filter<float>;
    juce::dsp::ProcessorChain<juce::dsp::Gain<float>, Filter, Filter, Filter> chain;

    enum {
      gainIndex,
      lowCutIndex,
      midbandIndex,
      highCutIndex
    };

    float low_gain;
    float low_frequency;
    float low_q;

    float mid_gain;
    float mid_frequency;
    float mid_q;

    float high_gain;
    float high_frequency;
    float high_q;

    std::vector<juce::IIRFilter> filters;
    // juce::dsp::ProcessorChain<juce::dsp::gain<float>, juce::IIRFilter> chain;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphicEqualiserAudioProcessor)
};
