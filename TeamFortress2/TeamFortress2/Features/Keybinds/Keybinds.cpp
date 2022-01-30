#include "Keybinds.h"
#include "../Vars.h"
#include "../Menu/Menu.h"

constexpr Color_t clrBlack = { 0, 0, 0, 255 };
constexpr Color_t clrWhite = { 255, 255, 255, 255 };

void CKeybinds::Run()
{
	if (!ShouldRun())
		return;
	m_nKeybindsSize = 120;
	DrawKeybinds(); // draw keybinds window
}

void CKeybinds::DrawFeature(const char* label, int x, int y, bool active)
{
	if (active) { g_Draw.String(FONT_INDICATORS, x, y, Colors::FeatureText, ALIGN_DEFAULT, label); }
}

void CKeybinds::DrawKeybinds()
{
	if (g_Menu.m_bOpen)
	{
		g_Interfaces.Surface->DrawSetAlphaMultiplier(0.6f);
		g_Draw.Rect(m_nKeybindsX - m_nKeybindsSize, m_nKeybindsY - m_nKeybindsSize - 10, m_nKeybindsSize, 10, { 36, 34, 54, 150}); // fuck you i like this colour
		g_Interfaces.Surface->DrawSetAlphaMultiplier(1.f);
		DragKeybinds();
	}
	// if menu open, allow us to drag, render bar

	
	Color_t clrBack = { 30, 30, 30, static_cast<byte>(Vars::Radar::Main::BackAlpha.m_Var) }; // using radar colour

	DrawFeature(_("AIM"), m_nKeybindsX + 5 - m_nKeybindsSize, m_nKeybindsY + 5 - m_nKeybindsSize, GetAsyncKeyState(Vars::Aimbot::Global::AimKey.m_Var));
	DrawFeature(_("TRG"), m_nKeybindsX + 5 - m_nKeybindsSize, m_nKeybindsY + 25 - m_nKeybindsSize, (GetAsyncKeyState(Vars::Triggerbot::Global::TriggerKey.m_Var) || !Vars::Triggerbot::Global::TriggerKey.m_Var));
	DrawFeature(_("DT"), m_nKeybindsX + 5 - m_nKeybindsSize, m_nKeybindsY + 45 - m_nKeybindsSize, (GetAsyncKeyState(Vars::Misc::CL_Move::DoubletapKey.m_Var) || Vars::Misc::CL_Move::DTMode.m_Var == 1));
	DrawFeature(_("RCG"), m_nKeybindsX + 5 - m_nKeybindsSize, m_nKeybindsY + 65 - m_nKeybindsSize, (g_GlobalInfo.m_bRecharging));
	DrawFeature(_("ANT"), m_nKeybindsX + 5 - m_nKeybindsSize, m_nKeybindsY + 85 - m_nKeybindsSize, (Vars::AntiHack::AntiAim::Active.m_Var));
	DrawFeature(_("FLG"), m_nKeybindsX + 5 - m_nKeybindsSize, m_nKeybindsY + 105 - m_nKeybindsSize, (g_GlobalInfo.m_bChoking));
	DrawFeature(_("EDG"), m_nKeybindsX + 5 - m_nKeybindsSize, m_nKeybindsY + 125 - m_nKeybindsSize, GetAsyncKeyState(Vars::Misc::EdgeJumpKey.m_Var));
} // this is fucking retarded

// if we have the esc-menu open do not draw, supa simple
bool CKeybinds::ShouldRun()
{
	if (g_Interfaces.EngineVGui->IsGameUIVisible())
		return false;

	return true;
}

// paste, og @ Radar.cpp
void CKeybinds::DragKeybinds()
{
	int mousex, mousey;
	g_Interfaces.Surface->GetCursorPos(mousex, mousey);

	static POINT pCorrect;
	static bool m_bDrag = false;
	static bool m_bMove = false;
	bool bHeld = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);

	if ((mousex > (m_nKeybindsX - m_nKeybindsSize) &&
		mousex < (m_nKeybindsX - m_nKeybindsSize) + (m_nKeybindsSize) &&
		mousey >(m_nKeybindsY - m_nKeybindsSize) - 10 &&
		mousey < (m_nKeybindsY - m_nKeybindsSize)) && bHeld)
	{
		m_bDrag = true;

		if (!m_bMove)
		{
			pCorrect.x = mousex - m_nKeybindsX;
			pCorrect.y = mousey - (m_nKeybindsY - m_nKeybindsSize);
			m_bMove = true;
		}
	}

	if (m_bDrag)
	{
		m_nKeybindsX = (mousex + m_nKeybindsSize) - (m_nKeybindsSize)-pCorrect.x;
		m_nKeybindsY = (mousey + m_nKeybindsSize) - pCorrect.y;
	}

	if (!bHeld)
	{
		m_bDrag = false;
		m_bMove = false;
	}
}