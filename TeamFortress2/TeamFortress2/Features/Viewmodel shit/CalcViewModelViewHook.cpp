#include "CalcViewModelViewHook.h"
#include "../Vars.h"

void __fastcall CalcViewModelView::Hook(void* ecx, void* edx, CBaseEntity* owner, Vec3& eyePosition, Vec3& eyeAngles){
    static auto originalFn = Func.Original<fn>();
    if (auto& pLocal = g_EntityCache.m_pLocal) {
        if (pLocal->IsAlive() && !g_GlobalInfo.m_vAimPos.IsZero() && Vars::Visuals::AimbotViewmodel.m_Var) {
            if (g_GlobalInfo.m_WeaponType == EWeaponType::PROJECTILE) {
                eyeAngles = Math::CalcAngle(pLocal->GetEyePosition(), g_GlobalInfo.m_vPredictedPos);
            }
            else {
                eyeAngles = Math::CalcAngle(pLocal->GetEyePosition(), g_GlobalInfo.m_vAimPos);
            }
        }
        else if (pLocal->IsAlive())
        {
            eyeAngles = g_Interfaces.Engine->GetViewAngles();
        }
    }
    //VM Offset Shit
    
    Vec3 vForward = {}, vRight = {}, vUp = {};
    Math::AngleVectors(eyeAngles, &vForward, &vRight, &vUp);

    Vec3 vEyePosition = eyePosition + (
        (vRight * Vars::Visuals::XOffset.m_Var) +
        (vForward * Vars::Visuals::YOffset.m_Var) +
        (vUp * Vars::Visuals::ZOffset.m_Var)
        );
    
    eyeAngles.z += Vars::Visuals::VMRoll.m_Var; //VM Roll

    originalFn(ecx, edx, owner, vEyePosition, eyeAngles);
}

void CalcViewModelView::Init() {
	auto Src = reinterpret_cast<void*>(g_Pattern.Find(
		L"client.dll", L"55 8B EC 83 EC 70 8B 55 0C 53 8B 5D 08 89 4D FC 8B 02 89 45 E8 8B 42 04 89 45 EC 8B 42 08 89 45 F0 56 57"));
	Func.Hook(Src, Hook);
}