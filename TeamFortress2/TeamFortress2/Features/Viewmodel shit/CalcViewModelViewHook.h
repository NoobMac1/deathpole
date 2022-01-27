#include "../../Utils/DPHook/DPHook.h"
#include "../../Utils/Math/Math.h"
#include "../../SDK/Main/BaseEntity/BaseEntity.h"
namespace CalcViewModelView {
    inline DPHook::Func Func;
    using fn = void(__fastcall*)(void*, void*, CBaseEntity*, Vec3&, Vec3&);
    void __fastcall Hook(void* ecx, void* edx, CBaseEntity* owner, Vec3& eyePosition, Vec3& eyeAngles);
    void Init();
}