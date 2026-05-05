#include "CustomAudioEditor.h"

CustomAudioEditor::CustomAudioEditor (RNBO::JuceAudioProcessor* const p, RNBO::CoreObject& rnboObject)
    : AudioProcessorEditor (p)
    , _rnboObject(rnboObject)
    , _audioProcessor(p)
{
    _audioProcessor->AudioProcessor::addListener(this);

    //label for the title of the plugin
    _titleLabel.setText("NeuroBassify", NotificationType::dontSendNotification);
    _titleLabel.setJustificationType(Justification::centred);
    _titleLabel.setColour(Label::textColourId, Colours::white);
    _titleLabel.setFont(Font(26.0f, Font::bold));
    addAndMakeVisible(_titleLabel);

    //label for the example parameter knob, difference in names are explained below
    _exampleLabel.setText("movement", NotificationType::dontSendNotification);
    _exampleLabel.setJustificationType(Justification::centred);
    _exampleLabel.setColour(Label::textColourId, Colour(0xffd8e2ff));
    _exampleLabel.setFont(Font(15.0f, Font::plain));
    addAndMakeVisible(_exampleLabel);

    // The visible label is "movement", but the slider name stays "example"
    // so that it can bind to the RNBO parameter id. Had to do this as I do not want to return to editing source
    // pretty lazy doing this, but reimporting source may have unforseen concicences and would break references
    _exampleSlider.setName("example");
    _exampleSlider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    _exampleSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 96, 24);
    _exampleSlider.setColour(Slider::rotarySliderFillColourId, Colour(0xffff6b35));
    _exampleSlider.setColour(Slider::rotarySliderOutlineColourId, Colour(0xff283049));
    _exampleSlider.setColour(Slider::thumbColourId, Colours::white);
    _exampleSlider.setColour(Slider::textBoxTextColourId, Colours::white);
    _exampleSlider.setColour(Slider::textBoxBackgroundColourId, Colour(0xff111522));
    _exampleSlider.setColour(Slider::textBoxOutlineColourId, Colour(0xff343c56));
    _exampleSlider.setTextValueSuffix(" %");
    addAndMakeVisible(_exampleSlider);

    configureExampleSlider();
    _exampleSlider.addListener(this);

    setSize (360, 300);
}

CustomAudioEditor::~CustomAudioEditor()
{
    _exampleSlider.removeListener(this);
    _audioProcessor->AudioProcessor::removeListener(this);
}

    //adds gradiant to bg, very nice I think
void CustomAudioEditor::paint (Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    ColourGradient background(Colour(0xff121522), bounds.getTopLeft(),
                              Colour(0xff273149), bounds.getBottomRight(), false);

    g.setGradientFill(background);
    g.fillAll();

    g.setColour(Colour(0x22ffffff));
    g.drawRoundedRectangle(bounds.reduced(10.0f), 8.0f, 1.0f);
}

void CustomAudioEditor::audioProcessorParameterChanged (AudioProcessor*, int parameterIndex, float value)
{
    if (parameterIndex == static_cast<int>(_exampleParameterIndex))
    {
        // DAW automation callbacks may arrive off the message thread, so defer UI updates safely.
        Component::SafePointer<CustomAudioEditor> editor(this);
        MessageManager::callAsync([editor, value]
        {
            if (editor != nullptr)
                editor->updateExampleSlider(value);
        });
    }
}

void CustomAudioEditor::resized()
{
    auto area = getLocalBounds().reduced(24);
    _titleLabel.setBounds(area.removeFromTop(42));
    area.removeFromTop(8);

    _exampleSlider.setBounds(area.removeFromTop(180));
    _exampleLabel.setBounds(area.removeFromTop(28));
}

 //updates RNBO code when slider value changes
void CustomAudioEditor::sliderValueChanged(Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved != &_exampleSlider || _exampleParameterIndex == RNBO::INVALID_INDEX)
        return;

    auto parameters = _audioProcessor->getParameters();
    if (_exampleParameterIndex >= static_cast<RNBO::ParameterIndex>(parameters.size()))
        return;

    if (auto* parameter = parameters[static_cast<int>(_exampleParameterIndex)])
    {
        // Hosts expect normalized parameter values, while the GUI slider uses the RNBO display range.
        auto normalizedValue = static_cast<float>(
            _rnboObject.convertToNormalizedParameterValue(_exampleParameterIndex, _exampleSlider.getValue()));

        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(normalizedValue);
        parameter->endChangeGesture();
    }
}

void CustomAudioEditor::configureExampleSlider()
{
    // Look up the exported RNBO parameter and use its min/max/current value for the GUI control.
    _exampleParameterIndex = _rnboObject.getParameterIndexForID("example");

    if (_exampleParameterIndex == RNBO::INVALID_INDEX)
    {
        _exampleSlider.setEnabled(false);
        _exampleSlider.setRange(0.0, 100.0, 0.1);
        _exampleSlider.setValue(0.0, NotificationType::dontSendNotification);
        return;
    }

    RNBO::ParameterInfo parameterInfo;
    _rnboObject.getParameterInfo(_exampleParameterIndex, &parameterInfo);

    _exampleSlider.setRange(parameterInfo.min, parameterInfo.max, 0.01);
    _exampleSlider.setValue(_rnboObject.getParameterValue(_exampleParameterIndex), NotificationType::dontSendNotification);
}
//updates knob when it is changed by automation
//keeps visual up to date with automation so that there isnt a mismash of values
void CustomAudioEditor::updateExampleSlider(float normalizedValue)
{
    if (_exampleParameterIndex == RNBO::INVALID_INDEX || _exampleSlider.getThumbBeingDragged() != -1)
        return;

    // Convert host automation values back into the RNBO parameter range shown by the slider.
    auto value = _rnboObject.convertFromNormalizedParameterValue(_exampleParameterIndex, normalizedValue);
    _exampleSlider.setValue(value, NotificationType::dontSendNotification);
}
