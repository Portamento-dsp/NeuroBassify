#include "RNBO.h"
#include "RNBO_Utils.h"
#include "RNBO_JuceAudioProcessor.h"
#include "RNBO_BinaryData.h"
#include <json/json.hpp>

class CustomAudioProcessor : public RNBO::JuceAudioProcessor {
public:
    static CustomAudioProcessor* CreateDefault();
    CustomAudioProcessor(const nlohmann::json& patcher_desc, const nlohmann::json& presets, const RNBO::BinaryData& data);
    CustomAudioProcessor(const nlohmann::json& patcher_desc, const nlohmann::json& presets, const RNBO::BinaryData& data, RNBO::JuceAudioParameterFactory* paramFactory);
    juce::AudioProcessorEditor* createEditor() override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomAudioProcessor)
};

