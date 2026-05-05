#include "CustomAudioProcessor.h"
#include "CustomAudioEditor.h"
#include <json/json.hpp>
#include <cstring>
#include <memory>

#ifdef RNBO_INCLUDE_DESCRIPTION_FILE
#include <rnbo_description.h>
#endif

namespace
{
    // Filters RNBO parameters before JUCE exposes them to the DAW as VST parameters.
    // Returning nullptr keeps the RNBO parameter functional internally, but removes it
    // from host automation and parameter lists because users do not need to edit this.
    // it was a parameter for debugging that was not used after being exported.
    // should have been removed before compiling to c++
    class HostParameterFactory : public RNBO::JuceAudioParameterFactory
    {
    public:
        explicit HostParameterFactory(const nlohmann::json& patcherDescription)
            : RNBO::JuceAudioParameterFactory(patcherDescription)
        {
        }

    protected:
        juce::AudioProcessorParameter* create(RNBO::CoreObject& rnboObject,
                                              RNBO::ParameterIndex index,
                                              const RNBO::ParameterInfo& info,
                                              int versionHint,
                                              const nlohmann::json& meta) override
        {
            // "f" is used by the RNBO patch, but should not be user-facing in the DAW.
            // Looking back at this comment i am proud of my proffesinalness, not anymore i guess
            if (std::strcmp(rnboObject.getParameterId(index), "f") == 0)
                return nullptr;

            return RNBO::JuceAudioParameterFactory::create(rnboObject, index, info, versionHint, meta);
        }
    };

    HostParameterFactory* createHostParameterFactory(const nlohmann::json& patcherDescription)
    {
        // The RNBO processor only needs this factory while its constructor builds parameters.
        static thread_local std::unique_ptr<HostParameterFactory> factory;
        factory = std::make_unique<HostParameterFactory>(patcherDescription);
        return factory.get();
    }
}

//create an instance of our custom plugin, optionally set description, presets and binary data (datarefs)
CustomAudioProcessor* CustomAudioProcessor::CreateDefault() {
	nlohmann::json patcher_desc, presets;

#ifdef RNBO_BINARY_DATA_STORAGE_NAME
	extern RNBO::BinaryDataImpl::Storage RNBO_BINARY_DATA_STORAGE_NAME;
	RNBO::BinaryDataImpl::Storage dataStorage = RNBO_BINARY_DATA_STORAGE_NAME;
#else
	RNBO::BinaryDataImpl::Storage dataStorage;
#endif
	RNBO::BinaryDataImpl data(dataStorage);

#ifdef RNBO_INCLUDE_DESCRIPTION_FILE
	patcher_desc = RNBO::patcher_description;
	presets = RNBO::patcher_presets;
#endif
  return new CustomAudioProcessor(patcher_desc, presets, data);
}

CustomAudioProcessor::CustomAudioProcessor(
    const nlohmann::json& patcher_desc,
    const nlohmann::json& presets,
    const RNBO::BinaryData& data
    ) 
  : CustomAudioProcessor(patcher_desc, presets, data, createHostParameterFactory(patcher_desc))
{
}

CustomAudioProcessor::CustomAudioProcessor(
    const nlohmann::json& patcher_desc,
    const nlohmann::json& presets,
    const RNBO::BinaryData& data,
    RNBO::JuceAudioParameterFactory* paramFactory
    )
  : RNBO::JuceAudioProcessor(patcher_desc, presets, data, paramFactory)
{
}

AudioProcessorEditor* CustomAudioProcessor::createEditor()
{
    //Change this to use your CustomAudioEditor
    return new CustomAudioEditor (this, this->_rnboObject);
    //return RNBO::JuceAudioProcessor::createEditor();
}

int CustomAudioProcessor::getNumPrograms()
{
    // Keep JUCE's required single program, but do not expose RNBO presets as host programs.
    return 1;
}

int CustomAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CustomAudioProcessor::setCurrentProgram(int index)
{
    // Program changes from the host are ignored because preset selection is hidden.
    RNBO_UNUSED(index)
}

const juce::String CustomAudioProcessor::getProgramName(int index)
{
    RNBO_UNUSED(index)
    return {};
}

void CustomAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    RNBO_UNUSED(index)
    RNBO_UNUSED(newName)
}
// EXTRA NOTE SURROUNDING PRESETS
// This plugin is random, the use of presets doesnt make sense, although would have been nice
// the architechre of the randomness would have made at least 40 parameters to save
// thats too many and would have been a massive bloat for little reason
// as the second example parameter was moved it would be entirely random again
// presets may have been nice for use a ?filter table? for sustain basses
