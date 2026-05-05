#include "JuceHeader.h"
#include "RNBO.h"
#include "RNBO_JuceAudioProcessor.h"

class CustomAudioEditor : public AudioProcessorEditor, private AudioProcessorListener, private Slider::Listener
{
public:
    CustomAudioEditor(RNBO::JuceAudioProcessor* const p, RNBO::CoreObject& rnboObject);
    ~CustomAudioEditor() override;
    void paint (Graphics& g) override;
    void resized() override;

private:
    void audioProcessorChanged (AudioProcessor*, const ChangeDetails&) override { }
    void audioProcessorParameterChanged(AudioProcessor*, int parameterIndex, float) override;
    void sliderValueChanged(Slider* sliderThatWasMoved) override;
    void configureExampleSlider();
    void updateExampleSlider(float normalizedValue);

protected:
    AudioProcessor                              *_audioProcessor;
    RNBO::CoreObject&                           _rnboObject; 
    Label                                       _titleLabel;
    Label                                       _exampleLabel;
    Slider                                      _exampleSlider;
    RNBO::ParameterIndex                        _exampleParameterIndex = RNBO::INVALID_INDEX;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomAudioEditor)
};
