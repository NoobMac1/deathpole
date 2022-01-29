#pragma once

#include "../../SDK/SDK.h"

#include <fstream>
#include <filesystem>

enum AttributeID
{
	set_item_tint_rgb_override = 1004,// (?)
	halloween_footstep_type = 1005,
	halloween_voice_modulation = 1006,
	halloween_pumpkin_explosions = 1007,
	halloween_green_flames = 1008,	// weird
	halloween_death_ghosts = 1009,	// ?
	IsAustraliumItem = 2027,
	LootRarity = 2022,
	ItemStyleOverride = 542,
	AncientPowers = 150,
	IsFestive = 2053,
	Sheen = 2014,
	UnusualEffect = 134,
	ParticleEffect = 370,
	bombinomicon_effect_on_death = 344,
};

struct AttributeInfo_t
{
	int m_nItemDefIndex;
	int m_nEffect;
	int m_nParticle;
	int m_nSheen;
	bool m_bAncient;
	bool m_bStyleOverride;
	bool voice;
	bool pumpkins;
	bool foot;
	bool australium;
};

class CAttributChanger
{
public:
	void Run();

	bool m_bSave = false, m_bLoad = true, m_bSet = false;

private:
	void SetAttribute();
	void SaveConfig();
	void LoadConfig();

	std::map<int, AttributeInfo_t> m_mapAttributes;

	std::wifstream m_Read;
	std::wofstream m_Write;
	std::wstring m_szAttributePath = L"";

private:
	__inline void SaveInt(const wchar_t* szSection, const wchar_t* szItem, int value) {
		std::wstring szToSave = L"";
		szToSave += std::to_wstring(value);
		WritePrivateProfileStringW(szSection, szItem, szToSave.c_str(), m_szAttributePath.c_str());
	}

	__inline void SaveBool(const wchar_t* szSection, const wchar_t* szItem, bool value) {
		std::wstring szToSave = L"";
		szToSave += value ? L"true" : L"false";
		WritePrivateProfileStringW(szSection, szItem, szToSave.c_str(), m_szAttributePath.c_str());
	}

	__inline int LoadInt(const wchar_t* szSection, const wchar_t* szItem) {
		wchar_t szReturn[69];
		GetPrivateProfileStringW(szSection, szItem, L"0", szReturn, 69, m_szAttributePath.c_str());
		return std::stoi(szReturn);
	}

	__inline bool LoadBool(const wchar_t* szSection, const wchar_t* szItem) {
		wchar_t szReturn[69];
		GetPrivateProfileStringW(szSection, szItem, L"false", szReturn, 69, m_szAttributePath.c_str());
		return (wcscmp(szReturn, L"true") == 0) ? true : false;
	}
};

inline CAttributChanger g_AttributeChanger;