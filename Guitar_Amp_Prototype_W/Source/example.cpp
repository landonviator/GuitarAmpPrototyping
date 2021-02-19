void Guitar_Amp_PrototypeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    juce::dsp::AudioBlock<float> audioBlock {buffer};
    
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

//            updateHighPassFilter(200);
//            highPassFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
//
//            updatePreClipFilter(1420);
//            preClipFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

            for (int sample = 0; sample < buffer.getNumSamples(); sample++) {

                float diodeClippingAlgorithm = exp((0.1 * inputData[sample]) / (0.0253 * 1.68)) - 1;
                outputData[sample] = piDivisor * atan(diodeClippingAlgorithm * (driveScaled * 16));
               // outputData[sample] = inputData[sample];
            }


            //convolutionProcessor.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        
            updateLowFilter(*rawLow);
            lowFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

            updateMidFilter(*rawMid);
            midFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

            updateHighFilter(*rawHigh);
            highFilter.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        
            outputGainProcessor.setGainDecibels(*rawOutput);
            outputGainProcessor.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        }
}

void Guitar_Amp_PrototypeAudioProcessor::updateHighPassFilter(const float &freq){
    *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(lastSampleRate, 200);
}

void Guitar_Amp_PrototypeAudioProcessor::updatePreClipFilter(const float &freq){
    *preClipFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, 1420, 0.5, 6.0);
}

void Guitar_Amp_PrototypeAudioProcessor::updateLowFilter(const float &gain){
    *lowFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(lastSampleRate, 200, 1.3, pow(10, gain * 0.05));
}

void Guitar_Amp_PrototypeAudioProcessor::updateMidFilter(const float &gain){
    *midFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, 815, 0.3, pow(10, gain * 0.05));
}

void Guitar_Amp_PrototypeAudioProcessor::updateHighFilter(const float &gain){
    *highFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(lastSampleRate, 6000, 0.2, pow(10, gain * 0.05));
}