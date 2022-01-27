#include "EngineClientHook.h"

#include "../../Features/Menu/Menu.h"

bool __stdcall EngineClientHook::IsPlayingTimeDemo::Hook()
{
	static DWORD dwInterpolateServerEntities = g_Pattern.Find(XorStr(L"client.dll").c_str(), XorStr(L"55 8B EC 83 EC 30 8B 0D ? ? ? ? 53 33 DB 89 5D DC 89 5D E0").c_str());

	if (Vars::Misc::DisableInterpolation.m_Var)
	{
		if (reinterpret_cast<DWORD>(_ReturnAddress()) == (dwInterpolateServerEntities + 0xB8))
			return true;
	}

	return Table.Original<fn>(index)(g_Interfaces.Engine);
}

float __fastcall EngineClientHook::RemoveEventDelay::Hook()
{
	static DWORD dwGetTime = g_Pattern.Find(XorStr(L"engine.dll").c_str(), XorStr(L"E8 ? ? ? ? 83 EC 10").c_str());

	if (reinterpret_cast<DWORD>(_ReturnAddress()) == (dwGetTime + 0x3D))
		return FLT_MAX;
}