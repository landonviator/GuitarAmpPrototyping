/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define inputGainSliderId "input"
#define inputGainSliderName "Input"

#define driveSliderId "drive"
#define driveSliderName "Drive"

#define lowSliderId "low"
#define lowSliderName "Low"

#define midSliderId "mid"
#define midSliderName "Mid"

#define highSliderId "high"
#define highSliderName "High"

#define outputGainSliderId "output"
#define outputGainSliderName "Output"

//==============================================================================
/**
*/
class Guitar_Amp_PrototypeAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Guitar_Amp_PrototypeAudioProcessor();
    ~Guitar_Amp_PrototypeAudioProcessor() override;

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
    void updateHighPassFilter(const float &freq);


    juce::AudioProcessorValueTreeState treeState;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
private:
    const float piDivisor = 2 / 3.14;
    float lastSampleRate = 44100;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highPassFilter;
    juce::dsp::Gain<float> inputGainProcessor;
    juce::dsp::Convolution convolutionProcessor;
    juce::dsp::Gain<float> outputGainProcessor;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Guitar_Amp_PrototypeAudioProcessor)
};
