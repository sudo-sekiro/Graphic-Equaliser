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

    //==============================================================================
    void updateParameters(float sampleRate);

    void setLowGain(float newLowGain) {
      low_gain = newLowGain;
    }
    void setMidGain(float newMidGain) {
      mid_gain = newMidGain;
    }
    void setHighGain(float newHighGain) {
      high_gain = newHighGain;
    }

private:
    using Filter = juce::dsp::IIR::Filter<float>;

    using Stereo = dsp::ProcessorDuplicator<Filter, juce::dsp::IIR::Coefficients<float>>;

    /* Base code for defining filters without processor chain
     *
    Stereo lowCutFilter;
    Stereo highCutFilter;
    Stereo midFilter;
    */

    juce::dsp::ProcessorChain<Stereo, Stereo, Stereo> chain;

    enum {
      lowCutIndex,
      midbandIndex,
      highCutIndex
    };

    float low_gain;
    float low_frequency = 300.f;
    float low_q = 1.f ;

    float mid_gain;
    float mid_frequency = 3000;
    float mid_q = 0.5f;

    float high_gain;
    float high_frequency = 5000;
    float high_q = 1.f;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphicEqualiserAudioProcessor)
};
