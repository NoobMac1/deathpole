#include "CalcViewModelViewHook.h"
#include "../../Hooks/Viewmodel/Viewmodel.h"

void __fastcall CalcViewModelView::Hook(void* ecx, void* edx, CBaseEntity* owner, Vec3& eyePosition, Vec3& eyeAngles)
{
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
    }

    originalFn(ecx, edx, owner, eyePosition, eyeAngles);
}

void CalcViewModelView::Init()
{
    fn FN = reinterpret_cast<fn>(g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC 83 EC 3C 8B 55 10")));
    Func.Hook(FN, Hook);
}