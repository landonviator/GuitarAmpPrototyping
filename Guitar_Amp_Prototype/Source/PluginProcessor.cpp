/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Guitar_Amp_PrototypeAudioProcessor::Guitar_Amp_PrototypeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
treeState (*this, nullptr, "PARAMETER", createParameterLayout())
#endif
{
}

Guitar_Amp_PrototypeAudioProcessor::~Guitar_Amp_PrototypeAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Guitar_Amp_PrototypeAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    params.reserve(6);
    
    
    auto inputGainParam = std::make_unique<juce::AudioParameterFloat>(inputGainSliderId, inputGainSliderName, -36.0f, 36.0f, 0.0f);
    auto driveParam = std::make_unique<juce::AudioParameterFloat>(driveSliderId, driveSliderName, 0.0f, 24.0f, 0.0f);
    auto lowParam = std::make_unique<juce::AudioParameterFloat>(lowSliderId, lowSliderName, -36.0f, 36.0f, 0.0f);
    auto midParam = std::make_unique<juce::AudioParameterFloat>(midSliderId, midSliderName, -36.0f, 36.0f, 0.0f);
    auto highParam = std::make_unique<juce::AudioParameterFloat>(highSliderId, highSliderName, -36.0f, 36.0f, 0.0f);
    auto outputGainParam = std::make_unique<juce::AudioParameterFloat>(outputGainSliderId, outputGainSliderName, -36.0f, 36.0f, 0.0f);
    
    params.push_back(std::move(inputGainParam));
    params.push_back(std::move(driveParam));
    params.push_back(std::move(lowParam));
    params.push_back(std::move(midParam));
    params.push_back(std::move(highParam));
    params.push_back(std::move(outputGainParam));
    
    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String Guitar_Amp_PrototypeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Guitar_Amp_PrototypeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Guitar_Amp_PrototypeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Guitar_Amp_PrototypeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Guitar_Amp_PrototypeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Guitar_Amp_PrototypeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Guitar_Amp_PrototypeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Guitar_Amp_PrototypeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Guitar_Amp_PrototypeAudioProcessor::getProgramName (int index)
{
    return {};
}

void Guitar_Amp_PrototypeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Guitar_Amp_PrototypeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    inputGainProcessor.prepare(spec);
    convolutionProcessor.prepare(spec);
    outputGainProcessor.prepare(spec);
}

void Guitar_Amp_PrototypeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Guitar_Amp_PrototypeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Guitar_Amp_PrototypeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    juce::dsp::AudioBlock<float> audioBlock {buffer};
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel){
        auto* inputData = buffer.getReadPointer(channel);
        auto* outputData = buffer.getWritePointer(channel);
        auto* rawInput = treeState.getRawParameterValue(inputGainSliderId);
        auto* rawDrive = treeState.getRawParameterValue(driveSliderId);
        auto* rawLow = treeState.getRawParameterValue(lowSliderId);
        auto* rawMid = treeState.getRawParameterValue(midSliderId);
        auto* rawHigh = treeState.getRawParameterValue(highSliderId);
        auto* rawOutput = treeState.getRawParameterValue(outputGainSliderId);
        
        inputGainProcessor.setGainDecibels(*rawInput);
        inputGainProcessor.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
        
//        for (int samples = 0; samples < buffer.getNumSamples(); samples++){
//
//        }
        
        convolutionProcessor.loadImpulseResponse(BinaryData::metalOne_wav, BinaryData::metalOne_wavSize, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, 0, juce::dsp::Convolution::Normalise::yes);
        convolutionProcessor.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
        
        outputGainProcessor.setGainDecibels(*rawOutput);
        outputGainProcessor.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
    }
}

//==============================================================================
bool Guitar_Amp_PrototypeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Guitar_Amp_PrototypeAudioProcessor::createEditor()
{
    return new Guitar_Amp_PrototypeAudioProcessorEditor (*this);
}

//==============================================================================
void Guitar_Amp_PrototypeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream (stream);
}

void Guitar_Amp_PrototypeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData (data, size_t (sizeInBytes));
    if (tree.isValid()) {
        treeState.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Guitar_Amp_PrototypeAudioProcessor();
}
