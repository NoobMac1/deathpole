#include "Viewmodel.h"

void __fastcall Viewmodel::Hook(void* ecx, void* edx, CBaseEntity* owner, Vec3& eyePosition, Vec3& eyeAngles) {
	auto pLocal = g_EntityCache.m_pLocal;

	if (pLocal && pLocal->IsAlive())
	{
		Func.Original<fn>()(ecx, edx, owner, eyePosition, eyeAngles);
	}
	else {
		Func.Original<fn>()(ecx, edx, owner, eyePosition, eyeAngles);
	}
}

void Viewmodel::Init() {
	auto Src = reinterpret_cast<void*>(g_Pattern.Find(
		L"client.dll", L"55 8B EC 83 EC 70 8B 55 0C 53 8B 5D 08 89 4D FC 8B 02 89 45 E8 8B 42 04 89 45 EC 8B 42 08 89 45 F0 56 57"));
	Func.Hook(Src, Hook);
}

void __fastcall CalcViewModelView::Hook(void* ecx, void* edx, CBaseEntity* owner, Vec3& eyePosition, Vec3& eyeAngles)
{
    static auto originalFn = Func.Original<fn>();

    if (auto &pLocal = g_EntityCache.m_pLocal) {
        if (pLocal->IsAlive() && !g_GlobalInfo.m_vAimPos.IsZero() && Vars::Visuals::AimbotViewmodel.m_Var) {
            if (g_GlobalInfo.m_WeaponType == EWeaponType::PROJECTILE) {            
                eyeAngles = Math::CalcAngle(pLocal->GetEyePosition(), g_GlobalInfo.m_vPredictedPos);
            }
            else {
                eyeAngles = Math::CalcAngle(pLocal->GetEyePosition(), g_GlobalInfo.m_vAimPos);
            }
        }
    }

    originalFn(ecx, edx, owner, eyePosition, eyeAngles);
}

void CalcViewModelView::Init()
{
    fn FN = reinterpret_cast<fn>(g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC 83 EC 3C 8B 55 10")));
    Func.Hook(FN, Hook);
}