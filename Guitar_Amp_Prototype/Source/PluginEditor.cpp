/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Guitar_Amp_PrototypeAudioProcessorEditor::Guitar_Amp_PrototypeAudioProcessorEditor (Guitar_Amp_PrototypeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    shadowProperties.radius = 24;
        shadowProperties.offset = juce::Point<int> (-1, 3);
        dialShadow.setShadowProperties (shadowProperties);
        
        sliders.reserve(6);
        sliders = {
            &inputSlider, &driveSlider, &lowSlider, &midSlider, &highSlider, &outputSlider
        };
        
        labels.reserve(6);
        labels = {
                 &inputLabel, &driveLabel, &lowLabel, &midLabel, &highLabel, &outputLabel
            };
            
        labelTexts.reserve(6);
        labelTexts = {
            inputSliderLabelText, driveSliderLabelText, lowSliderLabelText, midSliderLabelText, highSliderLabelText, outputSliderLabelText
        };
            
        
        for (auto i = 0; i < sliders.size(); i++) {
            addAndMakeVisible(sliders[i]);
            sliders[i]->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
            sliders[i]->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 32);
            sliders[i]->setColour(0x1001400, juce::Colour::fromFloatRGBA(1, 1, 1, 0.5f));
            sliders[i]->setColour(0x1001700, juce::Colour::fromFloatRGBA(1, 1, 1, 0.0f));
            sliders[i]->setColour(0x1001500, juce::Colour::fromFloatRGBA(0, 0, 0, 0.25f));
            sliders[i]->setLookAndFeel(&customDial);
            sliders[i]->setComponentEffect(&dialShadow);
            sliders[i]->setRange(-36.0, 36.0, 0.25);
            sliders[i]->setDoubleClickReturnValue(true, 0.0);
        }
        
        driveSlider.setRange(0, 24, 0.5);
        lowSlider.setRange(-6.0, 6.0, 0.25);
        midSlider.setRange(-6.0, 6.0, 0.25);
        highSlider.setRange(-6.0, 6.0, 0.25);
        
        inputSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, inputGainSliderId, inputSlider);
        driveSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, driveSliderId, driveSlider);
        lowSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, lowSliderId, lowSlider);
        midSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, midSliderId, midSlider);
        highSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, highSliderId, highSlider);
        outputSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, outputGainSliderId, outputSlider);
        
        for (auto i = 0; i < labels.size(); i++) {
                addAndMakeVisible(labels[i]);
                labels[i]->setText(labelTexts[i], juce::dontSendNotification);
                labels[i]->setJustificationType(juce::Justification::centred);
                labels[i]->setColour(0x1000281, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
                labels[i]->attachToComponent(sliders[i], false);
            }
        
        addAndMakeVisible(windowBorder);
        windowBorder.setText("Ignorant Diode Amplifier");
        windowBorder.setColour(0x1005400, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
        windowBorder.setColour(0x1005410, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
        
        //Making the window resizable by aspect ratio and setting size
        AudioProcessorEditor::setResizable(true, true);
        AudioProcessorEditor::setResizeLimits(711, 237, 1374, 458);
        AudioProcessorEditor::getConstrainer()->setFixedAspectRatio(3.0);
        setSize (711, 237);
}

Guitar_Amp_PrototypeAudioProcessorEditor::~Guitar_Amp_PrototypeAudioProcessorEditor()
{
}

//==============================================================================
void Guitar_Amp_PrototypeAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::Rectangle<int> background = AudioProcessorEditor::getLocalBounds();
    g.setGradientFill(juce::ColourGradient::vertical(juce::Colour::fromFloatRGBA(0.18f, 0.20f, 0.24f, 1.0), 300 * .01, juce::Colour::fromFloatRGBA(0.12f, 0.14f, 0.18f, 1.0), 300 * .99));
    g.fillRect(background);
}

void Guitar_Amp_PrototypeAudioProcessorEditor::resized()
{
    //Master bounds object
       juce::Rectangle<int> bounds = getLocalBounds();
           
       //first column of gui
       juce::FlexBox flexboxColumnOne;
       flexboxColumnOne.flexDirection = juce::FlexBox::Direction::column;
       flexboxColumnOne.flexWrap = juce::FlexBox::Wrap::noWrap;
       flexboxColumnOne.alignContent = juce::FlexBox::AlignContent::stretch;
           
       juce::Array<juce::FlexItem> itemArrayColumnOne;
       itemArrayColumnOne.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, inputSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .25, 0, 0, 0)));
           
       flexboxColumnOne.items = itemArrayColumnOne;
       flexboxColumnOne.performLayout(bounds.removeFromLeft(bounds.getWidth() / 6));
       /* ============================================================================ */
       
       //second column of gui
       juce::FlexBox flexboxColumnTwo;
       flexboxColumnTwo.flexDirection = juce::FlexBox::Direction::column;
       flexboxColumnTwo.flexWrap = juce::FlexBox::Wrap::noWrap;
       flexboxColumnTwo.alignContent = juce::FlexBox::AlignContent::stretch;
           
       juce::Array<juce::FlexItem> itemArrayColumnTwo;
       itemArrayColumnTwo.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, driveSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .25, 0, 0, 0)));
           
       flexboxColumnTwo.items = itemArrayColumnTwo;
       flexboxColumnTwo.performLayout(bounds.removeFromLeft(bounds.getWidth() / 5));
       /* ============================================================================ */
       
       //third column of gui
       juce::FlexBox flexboxColumnThree;
       flexboxColumnThree.flexDirection = juce::FlexBox::Direction::column;
       flexboxColumnThree.flexWrap = juce::FlexBox::Wrap::noWrap;
       flexboxColumnThree.alignContent = juce::FlexBox::AlignContent::stretch;
           
       juce::Array<juce::FlexItem> itemArrayColumnThree;
       itemArrayColumnThree.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, lowSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .25, 0, 0, 0)));
           
       flexboxColumnThree.items = itemArrayColumnThree;
       flexboxColumnThree.performLayout(bounds.removeFromLeft(bounds.getWidth() / 4));
       /* ============================================================================ */
       
       //fourth column of gui
       juce::FlexBox flexboxColumnFour;
       flexboxColumnFour.flexDirection = juce::FlexBox::Direction::column;
       flexboxColumnFour.flexWrap = juce::FlexBox::Wrap::noWrap;
       flexboxColumnFour.alignContent = juce::FlexBox::AlignContent::stretch;
           
       juce::Array<juce::FlexItem> itemArrayColumnFour;
       itemArrayColumnFour.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, midSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .25, 0, 0, 0)));
           
       flexboxColumnFour.items = itemArrayColumnFour;
       flexboxColumnFour.performLayout(bounds.removeFromLeft(bounds.getWidth() / 3));
       /* ============================================================================ */
       
       //fifth column of gui
       juce::FlexBox flexboxColumnFive;
       flexboxColumnFive.flexDirection = juce::FlexBox::Direction::column;
       flexboxColumnFive.flexWrap = juce::FlexBox::Wrap::noWrap;
       flexboxColumnFive.alignContent = juce::FlexBox::AlignContent::stretch;
           
       juce::Array<juce::FlexItem> itemArrayColumnFive;
       itemArrayColumnFive.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, highSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .25, 0, 0, 0)));
           
       flexboxColumnFive.items = itemArrayColumnFive;
       flexboxColumnFive.performLayout(bounds.removeFromLeft(bounds.getWidth() / 2));
       /* ============================================================================ */
       
       //sixth column of gui
       juce::FlexBox flexboxColumnSix;
       flexboxColumnSix.flexDirection = juce::FlexBox::Direction::column;
       flexboxColumnSix.flexWrap = juce::FlexBox::Wrap::noWrap;
       flexboxColumnSix.alignContent = juce::FlexBox::AlignContent::stretch;
           
       juce::Array<juce::FlexItem> itemArrayColumnSix;
       itemArrayColumnSix.add(juce::FlexItem(bounds.getWidth() / 6, bounds.getHeight() / 2, outputSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .25, 0, 0, 0)));
           
       flexboxColumnSix.items = itemArrayColumnSix;
       flexboxColumnSix.performLayout(bounds.removeFromLeft(bounds.getWidth()));
       /* ============================================================================ */
       
       windowBorder.setBounds(AudioProcessorEditor::getWidth() * .01, AudioProcessorEditor::getHeight() * 0.04, AudioProcessorEditor::getWidth() * .98, AudioProcessorEditor::getHeight() * .90);
}
