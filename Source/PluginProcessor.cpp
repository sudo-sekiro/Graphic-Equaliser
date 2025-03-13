/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GraphicEqualiserAudioProcessor::GraphicEqualiserAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(gainParameter = new juce::AudioParameterFloat("gain", "Gain", juce::NormalisableRange<float>(0.f, 1.f, 0.1f), 0.5f));
    addParameter(frequencyParameter = new juce::AudioParameterFloat("freq", "Frequency", juce::NormalisableRange<float>(20.f, 20000.f, 1.f), 100.f));
    addParameter(qParameter = new juce::AudioParameterFloat("q", "Q", juce::NormalisableRange<float>(0.1f, 10.f, 0.1f), 1.f));
}

GraphicEqualiserAudioProcessor::~GraphicEqualiserAudioProcessor()
{
}

//==============================================================================
const juce::String GraphicEqualiserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GraphicEqualiserAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GraphicEqualiserAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GraphicEqualiserAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GraphicEqualiserAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GraphicEqualiserAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GraphicEqualiserAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GraphicEqualiserAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GraphicEqualiserAudioProcessor::getProgramName (int index)
{
    return {};
}

void GraphicEqualiserAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GraphicEqualiserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    chain.prepare(spec);

    for (int i = 0; i < getTotalNumInputChannels(); i++)
    {
        filters.push_back(juce::IIRFilter());
    }
}

void GraphicEqualiserAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GraphicEqualiserAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GraphicEqualiserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto sampleRate = getSampleRate();

    // Get user parameters
    gain = gainParameter->get();
    // gain = pow(10, gain / 20);
    frequency = frequencyParameter->get();
    q = qParameter->get();

    float normalizedFrequency = juce::MathConstants<float>::twoPi * frequency / sampleRate;
    const float bandwidth = normalizedFrequency / q;
    const double two_cos_wc = -2.0 * cos(normalizedFrequency);
    const double tan_half_bw = tan(bandwidth / 2.0);
    const double g_tan_half_bw = gain * tan_half_bw;
    const double sqrt_g = sqrt(gain);

    // Can just use juce make filter
    auto coefficients = juce::IIRCoefficients(sqrt_g + g_tan_half_bw,  // b0
                                              sqrt_g * two_cos_wc,  // b1
                                              sqrt_g - g_tan_half_bw,  // b2
                                              sqrt_g + tan_half_bw,  // a0
                                              sqrt_g * two_cos_wc,  // a1
                                              sqrt_g - tan_half_bw);  // a2

    juce::dsp::AudioBlock<float> block(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    for (int i = 0; filters.size(); i++)
    {
        filters[i].setCoefficients(coefficients);
    }

    // chain.process(context);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        filters[channel].processSamples (channelData, buffer.getNumSamples());
    }
}

//==============================================================================
bool GraphicEqualiserAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GraphicEqualiserAudioProcessor::createEditor()
{
    // return new GraphicEqualiserAudioProcessorEditor (*this);
    return new GenericAudioProcessorEditor(*this);

}

//==============================================================================
void GraphicEqualiserAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GraphicEqualiserAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GraphicEqualiserAudioProcessor();
}
