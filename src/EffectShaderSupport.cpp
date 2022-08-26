#include "EffectShaderSupport.h"

#include "API/ENBSeriesAPI.h"
extern ENB_API::ENBSDKALT1001* g_ENB;

#include "SimpleIni.h"

void EffectShaderSupport::GetFireFXForms()
{
	std::ifstream i(L"Data\\SKSE\\Plugins\\ENBEffectShaderSupport.json");
	json          formregistry;
	i >> formregistry;
	fireFX.clear();
	for (auto& editorID : formregistry["FireFX"]) {
		if (RE::TESForm* form = RE::TESForm::LookupByEditorID(editorID)) {
			if (auto shader = form->As<RE::TESEffectShader>()) {
				fireFX.insert(shader);
			}
		}
	}
}

#define GetSettingBool(a_setting) ini.GetBoolValue("EFFECTSHADERS", a_setting, true);
#define GetSettingFloat(a_setting) (float)ini.GetDoubleValue("EFFECTSHADERS", a_setting, 1.0f);
#define SetSettingBool(a_value, a_setting) ini.SetBoolValue("EFFECTSHADERS", a_setting, a_value);
#define SetSettingFloat(a_value, a_setting) ini.SetDoubleValue("EFFECTSHADERS", a_setting, a_value);

void EffectShaderSupport::LoadJSON()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);

	CSimpleIniA ini;
	ini.SetUnicode();
	SI_Error    rc = ini.LoadFile(L"enbseries\\enbeffectshaders.ini");
	if (rc < 0) {
		Enabled = true;
		OpacityDay = 1.0f;
		OpacityNight = 1.0f;
		OpacityInterior = 1.0f;
		IntensityDay = 1.0f;
		IntensityNight = 1.0f;
		IntensityInterior = 1.0f;
		ParticleIntensityDay = 1.0f;
		ParticleIntensityNight = 1.0f;
		ParticleIntensityInterior = 1.0f;
		ParticleIntensityNight = 1.0f;
		FireIntensityDay = 1.0f;
		FireIntensityNight = 1.0f;
		FireIntensityInterior = 1.0f;
		return;
	}

	Enabled = GetSettingBool("Enabled");

	OpacityDay = GetSettingFloat("OpacityDay");
	OpacityNight = GetSettingFloat("OpacityNight");
	OpacityInterior = GetSettingFloat("OpacityInterior");

	IntensityDay = GetSettingFloat("IntensityDay");
	IntensityNight = GetSettingFloat("IntensityNight");
	IntensityInterior = GetSettingFloat("IntensityInterior");

	ParticleIntensityDay = GetSettingFloat("ParticleIntensityDay");
	ParticleIntensityNight = GetSettingFloat("ParticleIntensityNight");
	ParticleIntensityInterior = GetSettingFloat("ParticleIntensityInterior");

	FireIntensityDay = GetSettingFloat("FireIntensityDay");
	FireIntensityNight = GetSettingFloat("FireIntensityNight");
	FireIntensityInterior = GetSettingFloat("FireIntensityInterior");
}

void EffectShaderSupport::SaveJSON()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();
	SetSettingFloat(OpacityDay, "OpacityDay");
	SetSettingFloat(OpacityNight, "OpacityNight");
	SetSettingFloat(OpacityInterior, "OpacityInterior");

	SetSettingFloat(IntensityDay, "IntensityDay");
	SetSettingFloat(IntensityNight, "IntensityNight");
	SetSettingFloat(IntensityInterior, "IntensityInterior");

	SetSettingFloat(ParticleIntensityDay, "ParticleIntensityDay");
	SetSettingFloat(ParticleIntensityNight, "ParticleIntensityNight");
	SetSettingFloat(ParticleIntensityInterior, "ParticleIntensityInterior");

	SetSettingFloat(FireIntensityDay, "FireIntensityDay");
	SetSettingFloat(FireIntensityNight, "FireIntensityNight");
	SetSettingFloat(FireIntensityInterior, "FireIntensityInterior");

	ini.SaveFile(L"enbseries\\enbeffectshaders.ini");
}

//RE::NiColorA pow(RE::NiColorA a_color, float a_power)
//{
//	RE::NiColorA newColor;
//	newColor.red = pow(a_color.red, a_power);
//	newColor.green = pow(a_color.red, a_power);
//	newColor.blue = pow(a_color.red, a_power);
//	return newColor;
//}

RE::NiColorA EffectShaderSupport::mult(RE::NiColorA a_color, float a_multiplier)
{
	RE::NiColorA newColor;
	newColor.red = std::clamp(a_color.red * a_multiplier, 0.0f, 1.0f);
	newColor.green = std::clamp(a_color.green * a_multiplier, 0.0f, 1.0f);
	newColor.blue = std::clamp(a_color.blue * a_multiplier, 0.0f, 1.0f);
	newColor.alpha = std::clamp(a_color.alpha * currentOpacity, 0.0f, 1.0f);
	return newColor;
}

//RE::NiColor pow(RE::NiColor a_color, float a_power)
//{
//	RE::NiColor newColor;
//	newColor.red = pow(a_color.red, a_power);
//	newColor.green = pow(a_color.red, a_power);
//	newColor.blue = pow(a_color.red, a_power);
//	return newColor;
//}

//RE::NiColor mult(RE::NiColor a_color, float a_multiplier)
//{
//	RE::NiColor newColor;
//	newColor.red = a_color.red * a_multiplier;
//	newColor.green = a_color.green * a_multiplier;
//	newColor.blue = a_color.blue * a_multiplier;
//	return newColor;
//}

void EffectShaderSupport::UpdateEffect(RE::ShaderReferenceEffect* a_effect)
{
	if (g_ENB) {
		if (enbEnabled && Enabled) {
			if (auto& shaderData = a_effect->effectShaderData) {
				shaderData->fillColor = mult(shaderData->fillColor, currentIntensity);
				shaderData->rimColor = mult(shaderData->rimColor, currentIntensity);
			}
			bool  isFire = fireFX.contains(a_effect->effectData);
			float particleIntensity = isFire ? currentFireIntensity : currentParticleIntensity;
			for (auto& particleShader : a_effect->particleShaders) {
				particleShader.particleShaderProp->colorScale = a_effect->effectData->data.colorScale * particleIntensity;
			}
		} else {
			if (auto& shaderData = a_effect->effectShaderData) {
				shaderData->fillColor = shaderData->fillColor;
				shaderData->rimColor = shaderData->rimColor;
			}
			for (auto& particleShader : a_effect->particleShaders) {
				particleShader.particleShaderProp->colorScale = a_effect->effectData->data.colorScale;
			}
		}
	}
}

float EffectShaderSupport::GetENBParameterFloat(const char* a_filename, const char* a_category, const char* a_keyname)
{
	float                 fvalue;
	ENB_SDK::ENBParameter param;
	if (g_ENB->GetParameter(a_filename, a_category, a_keyname, &param)) {
		if (param.Type == ENB_SDK::ENBParameterType::ENBParam_FLOAT) {
			memcpy(&fvalue, param.Data, ENBParameterTypeToSize(ENB_SDK::ENBParameterType::ENBParam_FLOAT));
			return fvalue;
		}
	}
	logger::debug("Could not find ENB parameter {}:{}:{}", a_filename, a_category, a_keyname);
	return 1.0f;
}

bool EffectShaderSupport::GetENBParameterBool(const char* a_filename, const char* a_category, const char* a_keyname)
{
	BOOL                  bvalue;
	ENB_SDK::ENBParameter param;
	if (g_ENB->GetParameter(a_filename, a_category, a_keyname, &param)) {
		if (param.Type == ENB_SDK::ENBParameterType::ENBParam_BOOL) {
			memcpy(&bvalue, param.Data, ENBParameterTypeToSize(ENB_SDK::ENBParameterType::ENBParam_BOOL));
			return bvalue;
		}
	}
	logger::debug("Could not find ENB parameter {}:{}:{}", a_filename, a_category, a_keyname);
	return false;
}


void EffectShaderSupport::UpdateENBTimeOfDay()
{
	// ENB SDK currently does not seem to support giving TOD information to plugins.
	//TimeOfDay.dawn = GetENBParameterFloat("enbseries.ini", "Statistics", "TOD Dawn");
	//TimeOfDay.sunrise = GetENBParameterFloat("enbseries.ini", "Statistics", "TOD Sunrise");
	//TimeOfDay.day = GetENBParameterFloat("enbseries.ini", "Statistics", "TOD Day");
	//TimeOfDay.sunset = GetENBParameterFloat("enbseries.ini", "Statistics", "TOD Sunset");
	//TimeOfDay.dusk = GetENBParameterFloat("enbseries.ini", "Statistics", "TOD Dusk");
	//TimeOfDay.night = GetENBParameterFloat("enbseries.ini", "Statistics", "TOD Night");
	//TimeOfDay.interiorDay = GetENBParameterFloat("enbseries.ini", "Statistics", "TOD interior Day");
	//TimeOfDay.interiorNight = GetENBParameterFloat("enbseries.ini", "Statistics", "TOD interior Night");
	//TimeOfDay.interiorFactor = GetENBParameterFloat("enbseries.ini", "Statistics", "Interior factor (output)");

	float nightFactor = 0.0f;
	if (auto calendar = RE::Calendar::GetSingleton()) {
		auto hour = calendar->GetHour();
		if (hour > 12)
			hour = 24 - hour;
		nightFactor = hour / 12;
	}

	TimeOfDay.day = 1 - nightFactor;
	TimeOfDay.day = nightFactor;
	TimeOfDay.interiorDay = 1 - nightFactor;
	TimeOfDay.interiorNight = nightFactor;

	float interiorFactor = false;
	if (auto player = RE::PlayerCharacter::GetSingleton()) {
		if (auto cell = player->GetParentCell()) {
			if (cell->IsInteriorCell()) {
				interiorFactor = 1.0f;
			}
		}
	}
	TimeOfDay.interiorFactor = interiorFactor;
}

//float EffectShaderFix::GetCurrentIntensity(const char* a_category)
//{
//	float intensity = 0.0f;
//	intensity += TimeOfDay.dawn * GetENBParameterFloat("enbseries.ini", a_category, "IntensityDawn");
//	intensity += TimeOfDay.sunrise * GetENBParameterFloat("enbseries.ini", a_category, "IntensitySunrise");
//	intensity += TimeOfDay.day * GetENBParameterFloat("enbseries.ini", a_category, "IntensityDay");
//	intensity += TimeOfDay.sunset * GetENBParameterFloat("enbseries.ini", a_category, "IntensitySunset");
//	intensity += TimeOfDay.dusk * GetENBParameterFloat("enbseries.ini", a_category, "IntensityDusk");
//	intensity += TimeOfDay.night * GetENBParameterFloat("enbseries.ini", a_category, "IntensityNight");
//	float interiorIntensity = 0.0f;
//	interiorIntensity += TimeOfDay.interiorDay * GetENBParameterFloat("enbseries.ini", a_category, "IntensityInteriorDay");
//	interiorIntensity += TimeOfDay.interiorNight * GetENBParameterFloat("enbseries.ini", a_category, "IntensityInteriorNight");
//	return std::lerp(intensity, interiorIntensity, TimeOfDay.interiorFactor);
//}

void EffectShaderSupport::Update() 
{
	enbEnabled = GetENBParameterBool("enbseries.ini", "GLOBAL", "UseEffect");
	if (enbEnabled) {
		UpdateENBTimeOfDay();
		currentOpacity = std::lerp(std::lerp(OpacityDay, OpacityNight, TimeOfDay.nightFactor), OpacityInterior, TimeOfDay.interiorFactor);
		currentIntensity = std::lerp(std::lerp(IntensityDay, IntensityNight, TimeOfDay.nightFactor), IntensityInterior, TimeOfDay.interiorFactor);
		currentParticleIntensity = std::lerp(std::lerp(ParticleIntensityDay, ParticleIntensityNight, TimeOfDay.nightFactor), ParticleIntensityInterior, TimeOfDay.interiorFactor);
		currentFireIntensity =  std::lerp(std::lerp(FireIntensityDay, FireIntensityNight, TimeOfDay.nightFactor), FireIntensityInterior, TimeOfDay.interiorFactor);
	}
}

#define TWDEF "group=MOD:EFFECTSHADERS min=0.00 step=0.01"

void EffectShaderSupport::UpdateUI()
{
	//int     index = 0;
	//CTwBar* bar = g_ENB->TwGetBarByIndex(index);
	//while (bar) {
	//	logger::debug("Bar index: {}, Bar name: {}", index, g_ENB->TwGetBarName(bar));
	//	index++;
	//	bar = g_ENB->TwGetBarByIndex(index);
	//}

	auto bar = g_ENB->TwGetBarByEnum(ENB_API::ENBWindowType::EditorBarEffects);
	g_ENB->TwAddVarRW(bar, "Enabled", ETwType::TW_TYPE_BOOLCPP, &Enabled, "group=MOD:EFFECTSHADERS");
	g_ENB->TwAddVarRW(bar, "OpacityDay", ETwType::TW_TYPE_FLOAT, &OpacityDay, TWDEF);
	g_ENB->TwAddVarRW(bar, "OpacityNight", ETwType::TW_TYPE_FLOAT, &OpacityNight, TWDEF);
	g_ENB->TwAddVarRW(bar, "OpacityInterior", ETwType::TW_TYPE_FLOAT, &OpacityInterior, TWDEF);
	g_ENB->TwAddVarRW(bar, "IntensityDay", ETwType::TW_TYPE_FLOAT, &IntensityDay, TWDEF);
	g_ENB->TwAddVarRW(bar, "IntensityNight", ETwType::TW_TYPE_FLOAT, &IntensityNight, TWDEF);
	g_ENB->TwAddVarRW(bar, "IntensityInterior", ETwType::TW_TYPE_FLOAT, &IntensityInterior, TWDEF);
	g_ENB->TwAddVarRW(bar, "ParticleIntensityDay", ETwType::TW_TYPE_FLOAT, &ParticleIntensityDay, TWDEF);
	g_ENB->TwAddVarRW(bar, "ParticleIntensityNight", ETwType::TW_TYPE_FLOAT, &ParticleIntensityNight, TWDEF);
	g_ENB->TwAddVarRW(bar, "ParticleIntensityInterior", ETwType::TW_TYPE_FLOAT, &ParticleIntensityInterior, TWDEF);
	g_ENB->TwAddVarRW(bar, "FireIntensityDay", ETwType::TW_TYPE_FLOAT, &FireIntensityDay, TWDEF);
	g_ENB->TwAddVarRW(bar, "FireIntensityNight", ETwType::TW_TYPE_FLOAT, &FireIntensityNight, TWDEF);
	g_ENB->TwAddVarRW(bar, "FireIntensityInterior", ETwType::TW_TYPE_FLOAT, &FireIntensityInterior, TWDEF);
}
