#include "DynamicValueParser.h"

#include "UserSettingParser.h"
#include "WallpaperEngine/Data/Model/DynamicValue.h"

using namespace WallpaperEngine::Data::Parsers;

DynamicValueUniquePtr DynamicValueParser::parse (const json& data, const Properties& properties, bool expectColor) {
    auto value = std::make_unique<DynamicValue> ();
    auto valueIt = data;
    std::optional<std::string> scriptSource = std::nullopt;
    std::optional<json> scriptPropsJson = std::nullopt;

    if (data.is_object ()) {
	const auto user = data.optional ("user");
	const auto script = data.optional ("script");
	valueIt = data.require ("value", "User setting must have a value");

	if (script.has_value () && !script->is_null ()) {
	    scriptSource = script->get<std::string> ();
	    scriptPropsJson = data.optional ("scriptproperties");
	}
    }

    // actual value parsing
    if (valueIt.is_string ()) {
	if (expectColor) {
	    value->update (Builders::ColorBuilder::parse (valueIt), DynamicValue::UpdateSource::Initialization);
	} else {
	    std::string str = valueIt;
	    int size = Builders::VectorBuilder::preparseSize (str);

	    if (size == 1) {
		// scalar? text value?
		std::size_t parsed = 0;
		try {
		    float f = std::stof (str, &parsed);

		    if (parsed == str.size ()) {
			value->update (f, DynamicValue::UpdateSource::Initialization);
		    } else {
			value->update (str, DynamicValue::UpdateSource::Initialization);
		    }
		} catch (const std::exception&) {
		    value->update (str, DynamicValue::UpdateSource::Initialization);
		}
	    } else if (size == 2) {
		value->update (static_cast<glm::vec2> (valueIt), DynamicValue::UpdateSource::Initialization);
	    } else if (size == 3) {
		value->update (static_cast<glm::vec3> (valueIt), DynamicValue::UpdateSource::Initialization);
	    } else {
		value->update (static_cast<glm::vec4> (valueIt), DynamicValue::UpdateSource::Initialization);
	    }
	}
    } else if (valueIt.is_number_integer ()) {
	value->update (valueIt.get<int> (), DynamicValue::UpdateSource::Initialization);
    } else if (valueIt.is_number_float ()) {
	value->update (valueIt.get<float> (), DynamicValue::UpdateSource::Initialization);
    } else if (valueIt.is_boolean ()) {
	value->update (valueIt.get<bool> (), DynamicValue::UpdateSource::Initialization);
    } else if (valueIt.is_null ()) {
	// null value with no connection to property
	value->update (DynamicValue::UpdateSource::Initialization);
    }

    if (data.is_object () && data.contains ("animation") && data["animation"].is_object ()) {
	const auto& animationJson = data["animation"];
	PropertyAnimation animation;
	if (animationJson.contains ("options") && animationJson["options"].is_object ()) {
	    const auto& options = animationJson["options"];
	    if (options.contains ("fps") && options["fps"].is_number ()) animation.fps = options["fps"].get<float> ();
	    if (options.contains ("length") && options["length"].is_number ()) {
		animation.length = options["length"].get<float> ();
	    }
	    if (options.contains ("mode") && options["mode"].is_string ()) {
		animation.mode = options["mode"].get<std::string> ();
	    }
	}
	for (int channel = 0; channel < 4; channel++) {
	    const std::string name = "c" + std::to_string (channel);
	    if (!animationJson.contains (name) || !animationJson[name].is_array ()) break;
	    std::vector<PropertyAnimation::Key> keys;
	    for (const auto& key : animationJson[name]) {
		if (key.is_object () && key.contains ("frame") && key.contains ("value")
		    && key["frame"].is_number () && key["value"].is_number ()) {
		    keys.push_back ({ key["frame"].get<float> (), key["value"].get<float> () });
		}
	    }
	    animation.channels.push_back (std::move (keys));
	}
	value->setAnimation (std::move (animation));
    }

    if (scriptSource.has_value ()) {
	std::map<std::string, UserSettingUniquePtr> scriptProps;

	if (scriptPropsJson.has_value () && scriptPropsJson->is_object ()) {
	    for (const auto& [key, propData] : scriptPropsJson->items ()) {
		scriptProps[key] = UserSettingParser::parse (propData, properties);
	    }
	}

	value->setProperties (std::move (scriptProps));
	value->setScriptSource (scriptSource.value ());
    }

    return value;
}