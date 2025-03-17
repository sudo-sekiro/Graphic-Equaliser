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
    addParameter(low_gainParameter = new juce::AudioParameterFloat("lowgain", "Low Gain", juce::NormalisableRange<float>(-20.f, 20.f, 0.1f), 1.f));
    addParameter(mid_gainParameter = new juce::AudioParameterFloat("midgain", "Mid Gain", juce::NormalisableRange<float>(-20.f, 20.f, 0.1f), 1.f));
    addParameter(high_gainParameter = new juce::AudioParameterFloat("highgain", "High Gain", juce::NormalisableRange<float>(-20.f, 20.f, 0.1f), 1.f));
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
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;

    lowBandBuffer.setSize(2, samplesPerBlock);
    highBandBuffer.setSize(2, samplesPerBlock);
    midBandBuffer.setSize(2, samplesPerBlock);

    lowBandChain.prepare(spec);
    midBandChain.prepare(spec);
    highBandChain.prepare(spec);

    // chain.prepare(spec);
    /* Base code for preparing filters without processor chain
     *
      lowCutFilter.prepare(spec);
      highCutFilter.prepare(spec);
      midFilter.prepare(spec);
    */
    *lowBandChain.get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, low_frequency, low_q, 0.1f); // cut everything above low frequency
    *highBandChain.get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, high_frequency, high_q, 0.1f); // cut everything below high frequency
    // cut everything except mid frequency
    *midBandChain.get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, low_frequency, mid_q, 0.1f);
    *midBandChain.get<2>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, high_frequency, mid_q, 0.1f);
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

void GraphicEqualiserAudioProcessor::updateParameters(float sampleRate) {
    low_gain = pow(10, low_gainParameter->get() / 20);
    mid_gain = pow(10, mid_gainParameter->get() / 20);
    high_gain = pow(10, high_gainParameter->get() / 20);

    lowBandChain.get<0>().setGainLinear(low_gain);
    // lowBandChain.get<0>().setGainLinear(low_gain);
    midBandChain.get<0>().setGainLinear(mid_gain);
    highBandChain.get<0>().setGainLinear(high_gain);

    /* Base code for updating filter parameters
     *
    *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, low_frequency, low_q, low_gain);
    *highCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, mid_frequency, mid_q, mid_gain);
    *midFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, high_frequency, high_q, high_gain);
    */
}

void GraphicEqualiserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Update user parameters
    auto sampleRate = getSampleRate();
    updateParameters(sampleRate);

    // Process filters in parallel
    lowBandBuffer.makeCopyOf(buffer);
    midBandBuffer.makeCopyOf(buffer);
    highBandBuffer.makeCopyOf(buffer);
    juce::dsp::AudioBlock<float> lowBandBlock(lowBandBuffer);
    auto lowBandContext = juce::dsp::ProcessContextReplacing<float>(lowBandBlock);
    lowBandChain.process(lowBandContext);

    juce::dsp::AudioBlock<float> midBandBlock(midBandBuffer);
    auto midBandContext = juce::dsp::ProcessContextReplacing<float>(midBandBlock);
    midBandChain.process(midBandContext);

    juce::dsp::AudioBlock<float> highBandBlock(highBandBuffer);
    auto highBandContext = juce::dsp::ProcessContextReplacing<float>(highBandBlock);
    highBandChain.process(highBandContext);

    buffer.clear();

    for (int i = 0; i < buffer.getNumChannels(); i++) {
        for (int j = 0; j < buffer.getNumSamples(); j++) {
            buffer.setSample(i, j, lowBandBuffer.getSample(i, j) + midBandBuffer.getSample(i, j) + highBandBuffer.getSample(i, j));
        }
    }
    // Process filters in series
    /* Base code for processing filters without processor chain
     *
    lowCutFilter.process(context);
    midFilter.process(context);
    highCutFilter.process(context);
    */

    // Clear any remaining output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
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
