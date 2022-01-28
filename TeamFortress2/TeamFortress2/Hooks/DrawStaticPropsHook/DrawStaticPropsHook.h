#pragma once
#include "../../SDK/SDK.h"

namespace DrawStaticPropsHook
{
	inline DPHook::Func Func;
	using fn = void(__fastcall*)(void*, void*, IClientRenderable**, int, bool, bool);
	void __fastcall Hook(void* ecx, void* edx, IClientRenderable** pProps, int count, bool bShadowDepth, bool drawVCollideWireframe);
}

namespace SetColorModulationHook
{
	inline DPHook::Func Func;
	using fn = void(__fastcall*)(void*, void*, float const*);
	void __fastcall Hook(void* ecx, void* edx, float const* pColor);
}

namespace SetAlphaModulationHook
{
	inline DPHook::Func Func;
	using fn = void(__fastcall*)(void*, void*, float);
	void __fastcall Hook(void* ecx, void* edx, float alpha);
}