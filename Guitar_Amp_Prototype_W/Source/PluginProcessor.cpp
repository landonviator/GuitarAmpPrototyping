/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Guitar_Amp_Prototype_WAudioProcessor::Guitar_Amp_Prototype_WAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    treeState(*this, nullptr, "PARAMETER", createParameterLayout()),
    highPassFilter(juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 20000, 1.0)),
    preClipFilter(juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 1420, 0.5, 6.0)),
    lowFilter(juce::dsp::IIR::Coefficients<float>::makeLowShelf(44100.0, 200.0f, 1.3f, 1.0f)),
    midFilter(juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 815, 0.3, 1.0)),
    highFilter(juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 6000, 0.2, 6.0))
#endif
{
}

Guitar_Amp_Prototype_WAudioProcessor::~Guitar_Amp_Prototype_WAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Guitar_Amp_Prototype_WAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    params.reserve(6);


    auto inputGainParam = std::make_unique<juce::AudioParameterFloat>(inputGainSliderId, inputGainSliderName, -36.0f, 36.0f, 0.0f);
    auto driveParam = std::make_unique<juce::AudioParameterFloat>(driveSliderId, driveSliderName, 0.0f, 24.0f, 0.0f);
    auto lowParam = std::make_unique<juce::AudioParameterFloat>(lowSliderId, lowSliderName, -6.0f, 6.0f, 0.0f);
    auto midParam = std::make_unique<juce::AudioParameterFloat>(midSliderId, midSliderName, -6.0f, 6.0f, 0.0f);
    auto highParam = std::make_unique<juce::AudioParameterFloat>(highSliderId, highSliderName, -6.0f, 6.0f, 0.0f);
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
const juce::String Guitar_Amp_Prototype_WAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Guitar_Amp_Prototype_WAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool Guitar_Amp_Prototype_WAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool Guitar_Amp_Prototype_WAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double Guitar_Amp_Prototype_WAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Guitar_Amp_Prototype_WAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Guitar_Amp_Prototype_WAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Guitar_Amp_Prototype_WAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String Guitar_Amp_Prototype_WAudioProcessor::getProgramName(int index)
{
    return {};
}

void Guitar_Amp_Prototype_WAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void Guitar_Amp_Prototype_WAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();

    highPassFilter.prepare(spec);
    highPassFilter.reset();
    preClipFilter.prepare(spec);
    preClipFilter.reset();

    lowFilter.prepare(spec);
    lowFilter.reset();
    midFilter.prepare(spec);
    midFilter.reset();
    highFilter.prepare(spec);
    highFilter.reset();

    inputGainProcessor.prepare(spec);
    convolutionProcessor.prepare(spec);
    outputGainProcessor.prepare(spec);

    convolutionProcessor.loadImpulseResponse(BinaryData::metalOne_wav, BinaryData::metalOne_wavSize, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::yes, 0, juce::dsp::Convolution::Normalise::yes);
}

void Guitar_Amp_Prototype_WAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Guitar_Amp_Prototype_WAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void Guitar_Amp_Prototype_WAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> audioBlock{ buffer };

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* inputData = buffer.getReadPointer(channel);
        auto* outputData = buffer.getWritePointer(channel);
        auto* rawInput = treeState.getRawParameterValue(inputGainSliderId);
        auto* rawDrive = treeState.getRawParameterValue(driveSliderId);
        float driveScaled = pow(10, *rawDrive * 0.05);
        auto* rawLow = treeState.getRawParameterValue(lowSliderId);
        auto* rawMid = treeState.getRawParameterValue(midSliderId);
        auto* rawHigh = treeState.getRawParameterValue(highSliderId);
        auto* rawOutput = treeState.getRawParameterValue(outputGainSliderId);

        inputGainProcessor.setGainDecibels(*rawInput);
        inputGainProcessor.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

        updateHighPassFilter(200);
        updatePreClipFilter(1420);

        updateLowFilter(*rawLow);
        updateMidFilter(*rawMid);
        updateHighFilter(*rawHigh);

        highPassFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        preClipFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

        for (int sample = 0; sample < buffer.getNumSamples(); sample++) {
            float diodeClippingAlgorithm = exp((0.1 * inputData[sample]) / (0.0253 * 1.68)) - 1;
            outputData[sample] = piDivisor * atan(diodeClippingAlgorithm * (driveScaled * 16));
        }

        convolutionProcessor.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

        outputGainProcessor.setGainDecibels(*rawOutput);
        outputGainProcessor.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

        lowFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        midFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        highFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    }
}

void Guitar_Amp_Prototype_WAudioProcessor::updateHighPassFilter(const float& freq) {
    *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(lastSampleRate, freq);
}

void Guitar_Amp_Prototype_WAudioProcessor::updatePreClipFilter(const float& freq) {
    *preClipFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, freq, 0.5, 6.0);
}

void Guitar_Amp_Prototype_WAudioProcessor::updateLowFilter(const float& gain) {
    *lowFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(lastSampleRate, 200, 1.3, pow(10, gain * 0.05));
}

void Guitar_Amp_Prototype_WAudioProcessor::updateMidFilter(const float& gain) {
    *midFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, 815, 0.3, pow(10, gain * 0.05));
}

void Guitar_Amp_Prototype_WAudioProcessor::updateHighFilter(const float& gain) {
    *highFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, 6000, 0.2, pow(10, gain * 0.05));
}

//==============================================================================
bool Guitar_Amp_Prototype_WAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Guitar_Amp_Prototype_WAudioProcessor::createEditor()
{
    return new Guitar_Amp_Prototype_WAudioProcessorEditor(*this);
}

//==============================================================================
void Guitar_Amp_Prototype_WAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream(stream);
}

void Guitar_Amp_Prototype_WAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid()) {
        treeState.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Guitar_Amp_Prototype_WAudioProcessor();
}
