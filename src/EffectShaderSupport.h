#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class EffectShaderSupport
{
public:
	static EffectShaderSupport* GetSingleton()
	{
		static EffectShaderSupport handler;
		return &handler;
	}

	static void InstallHooks()
	{
		Hooks::Install();
	}

	std::set<RE::TESEffectShader*> fireFX;
	void GetFireFXForms();

	json JSONSettings;
	bool Enabled;

	float OpacityDay;
	float OpacityNight;
	float OpacityInterior;

	float IntensityDay;
	float IntensityNight;
	float IntensityInterior;

	float ParticleIntensityDay;
	float ParticleIntensityNight;
	float ParticleIntensityInterior;

	float FireIntensityDay;
	float FireIntensityNight;
	float FireIntensityInterior;

	bool enbEnabled = false;

	float currentOpacity = 1.0f;
	float currentIntensity = 1.0f;
	float currentParticleIntensity = 1.0f;
	float currentFireIntensity = 1.0f;

	void LoadJSON();
	void SaveJSON();

	RE::NiColorA mult(RE::NiColorA a_color, float a_multiplier);
	void UpdateEffect(RE::ShaderReferenceEffect* a_effect);

	float GetENBParameterFloat(const char* a_filename, const char* a_category, const char* a_keyname);
	bool  GetENBParameterBool(const char* a_filename, const char* a_category, const char* a_keyname);
	void  UpdateENBTimeOfDay();
	float GetCurrentIntensity(const char* a_category);
	void  Update();
	void  UpdateUI();

	struct ENBTimeOfDay
	{
		float dawn;
		float sunrise;
		float day;
		float sunset;
		float dusk;
		float night;
		float interiorDay;
		float interiorNight;
		float interiorFactor;
		float nightFactor;
	};
	ENBTimeOfDay TimeOfDay;

protected:
	struct Hooks
	{
		struct ShaderReference_Update
		{
			static bool thunk(RE::ShaderReferenceEffect* a_effect, float a_delta)
			{
				bool ret = func(a_effect, a_delta);
				GetSingleton()->UpdateEffect(a_effect);
				return ret;
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_vfunc<RE::ShaderReferenceEffect, 0x28, ShaderReference_Update>();
		}
	};

private:
	EffectShaderSupport()
	{
		ZeroMemory(&TimeOfDay, sizeof(ENBTimeOfDay));
		LoadJSON();
	};

	EffectShaderSupport(const EffectShaderSupport&) = delete;
	EffectShaderSupport(EffectShaderSupport&&) = delete;

	~EffectShaderSupport() = default;

	EffectShaderSupport& operator=(const EffectShaderSupport&) = delete;
	EffectShaderSupport& operator=(EffectShaderSupport&&) = delete;
};
