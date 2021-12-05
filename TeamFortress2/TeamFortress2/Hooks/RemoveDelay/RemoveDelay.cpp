#include "RemoveDelay.h"

float __fastcall RemoveDelay::Hook(void* ecx, void* edx)
{
    static DWORD ReturnAddress = g_Pattern.Find(_(L"engine.dll"), _(L"D9 43 ? DF F1"));
 
    if (reinterpret_cast<DWORD>(_ReturnAddress()) == ReturnAddress)
        return FLT_MAX;

    Func.Original<fn>()(ecx, edx);
}

void RemoveDelay::Init() {

}