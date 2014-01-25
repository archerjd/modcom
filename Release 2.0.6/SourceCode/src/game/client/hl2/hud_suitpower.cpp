//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hud_suitpower.h"
#include "hud_macros.h"
#include "c_hl2mp_player.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "hud_hintdisplay.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CHudSuitPower );

#define SUITPOWER_INIT -1

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSuitPower::CHudSuitPower( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudSuitPower" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSuitPower::Init( void )
{
	m_flSuitPower = SUITPOWER_INIT;
	m_nSuitPowerLow = -1;
	m_iActiveSuitDevices = 0;
	m_bShowLowPowerHint = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSuitPower::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudSuitPower::ShouldDraw()
{
	bool bNeedsDraw = false;

	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;

	// needs draw if suit power changed or animation in progress
	bNeedsDraw = ( ( pPlayer->m_HL2Local.m_flSuitPower != m_flSuitPower ) || ( m_AuxPowerColor[3] > 0 ) );

	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ConVar cl_lowpower_hint_delay("cl_lowpower_hint_delay", "18", FCVAR_CLIENTDLL);
void CHudSuitPower::OnThink( void )
{
	C_HL2MP_Player *pPlayer = (C_HL2MP_Player*)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	float flCurrentPower = pPlayer->m_HL2Local.m_flSuitPower;
	float flMaxPower = pPlayer->GetMaxAuxPower();

	if ( m_bShowLowPowerHint )
	{
		if ( m_nSuitPowerLow < 1 )
			m_bShowLowPowerHint = false;
		else if ( m_nSuitPowerLow && m_flLowPowerStartTime + cl_lowpower_hint_delay.GetFloat() <= gpGlobals->curtime )
		{
			m_bShowLowPowerHint = false;
			SHOW_HINT("#Hint_SuitPowerLow")
		}
	}
	// Only update if we've changed suit power, or changed max
	if ( flCurrentPower == m_flSuitPower && flMaxPower == m_flSuitPowerMax)
		return;

	if ( flCurrentPower >= flMaxPower && m_flSuitPower < flMaxPower )
	{
		// we've reached max power
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitAuxPowerMax");
	}
	else if ( flCurrentPower < flMaxPower && (m_flSuitPower >= flMaxPower || m_flSuitPower == SUITPOWER_INIT) )
	{
		// we've lost power
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitAuxPowerNotMax");
	}


	bool flashlightActive = pPlayer->IsFlashlightActive();
	bool sprintActive = pPlayer->IsSprinting();
	bool breatherActive = pPlayer->IsBreatherActive();

	int activeDevices = (int)flashlightActive + (int)sprintActive + (int)breatherActive;

	if ( pPlayer->NumMinions() > 0 )
		activeDevices ++;
	for ( int i=0; i<NUM_MODULES; i++ )
	{
		Module *a = GetModule(i);
		if ( a->ShouldShowDrainOnHud() && pPlayer->IsModuleActive(a) )
			activeDevices ++;
	}

	if (activeDevices != m_iActiveSuitDevices)
	{
		m_iActiveSuitDevices = activeDevices;

		switch ( m_iActiveSuitDevices )
		{
		default:
		case 3:
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitAuxPowerThreeItemsActive");
			break;
		case 2:
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitAuxPowerTwoItemsActive");
			break;
		case 1:
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitAuxPowerOneItemActive");
			break;
		case 0:
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitAuxPowerNoItemsActive");
			break;
		}
	}

	m_flSuitPower = flCurrentPower;
	m_flSuitPowerMax = flMaxPower;
}

//-----------------------------------------------------------------------------
// Purpose: draws the power bar
//-----------------------------------------------------------------------------
#define POINTS_PER_CHUNK 10.0f
void CHudSuitPower::Paint()
{
	C_HL2MP_Player *pPlayer = (C_HL2MP_Player*)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	// draw more chunks when power capacity is higher
	float flMaxPower = pPlayer->GetMaxAuxPower();
	float flBarChunkWidth = m_flBarChunkWidth / ( flMaxPower / 100.0f );

	// get bar chunks
	int chunkCount = m_flBarWidth / (flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_flSuitPower * 1.0f/flMaxPower) + 0.5f );

	// see if we've changed power state
	int lowPower = 0;
	if (enabledChunks <= (chunkCount / 4))
	{
		lowPower = 1;
	}
	if (m_nSuitPowerLow != lowPower)
	{
		if (m_iActiveSuitDevices || m_flSuitPower < flMaxPower)
		{
			if (lowPower)
			{
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitAuxPowerDecreasedBelow25");
				// send "low power" hint message
				m_flLowPowerStartTime = gpGlobals->curtime;
				m_bShowLowPowerHint = true;
			}
			else
			{
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitAuxPowerIncreasedAbove25");
				//CLEAR_HINT();
			}
			m_nSuitPowerLow = lowPower;
		}
	}

	// draw the suit power bar
	surface()->DrawSetColor( m_AuxPowerColor );
	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;
	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + flBarChunkWidth, ypos + m_flBarHeight );
		xpos += (flBarChunkWidth + m_flBarChunkGap);
	}
	// draw the exhausted portion of the bar.
	surface()->DrawSetColor( Color( m_AuxPowerColor[0], m_AuxPowerColor[1], m_AuxPowerColor[2], m_iAuxPowerDisabledAlpha ) );
	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect( xpos, ypos, xpos + flBarChunkWidth, ypos + m_flBarHeight );
		xpos += (flBarChunkWidth + m_flBarChunkGap);
	}

	// draw our name
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_AuxPowerColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);

	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_AUX_POWER");

	if (tempString)
	{
		surface()->DrawPrintText(tempString, wcslen(tempString));
	}
	else
	{
		surface()->DrawPrintText(L"AUX POWER", wcslen(L"AUX POWER"));
	}

	if ( m_iActiveSuitDevices )
	{
		// draw the additional text
		int ypos = text2_ypos;

		// draw any abilities with a maintained drain we should show on the aux power bar
		for ( int i=0; i<NUM_MODULES; i++ )
		{
			Module *a = GetModule(i);
			if ( a->ShouldShowDrainOnHud() && pPlayer->IsModuleActive(a) )
			{
				tempString = a->GetDisplayNameUnicode();

				surface()->DrawSetTextPos(text2_xpos, ypos);

				if (tempString)
				{
					surface()->DrawPrintText(tempString, wcslen(tempString));
				}
				else
				{
					surface()->DrawPrintText(L"MODULE", wcslen(L"MODULE"));
				}
				ypos += text2_gap;
			}
		}

		if (pPlayer->NumMinions() > 0 )
		{
			surface()->DrawSetTextPos(text2_xpos, ypos);

			switch ( pPlayer->NumMinions() )
			{
			case 1:
				surface()->DrawPrintText(L"Minions x1", wcslen(L"Minions x1")); break;
			case 2:
				surface()->DrawPrintText(L"Minions x2", wcslen(L"Minions x2")); break;
			case 3:
				surface()->DrawPrintText(L"Minions x3", wcslen(L"Minions x3")); break;
			case 4:
				surface()->DrawPrintText(L"Minions x4", wcslen(L"Minions x4")); break;
			default:
				surface()->DrawPrintText(L"Minions", wcslen(L"Minions")); break;
			}
			ypos += text2_gap;
		}

		if (pPlayer->IsBreatherActive())
		{
			tempString = g_pVGuiLocalize->Find("#Valve_Hud_OXYGEN");

			surface()->DrawSetTextPos(text2_xpos, ypos);

			if (tempString)
			{
				surface()->DrawPrintText(tempString, wcslen(tempString));
			}
			else
			{
				surface()->DrawPrintText(L"Oxygen", wcslen(L"Oxygen"));
			}
			ypos += text2_gap;
		}

		if (pPlayer->IsFlashlightActive())
		{
			tempString = g_pVGuiLocalize->Find("#Valve_Hud_FLASHLIGHT");

			surface()->DrawSetTextPos(text2_xpos, ypos);

			if (tempString)
			{
				surface()->DrawPrintText(tempString, wcslen(tempString));
			}
			else
			{
				surface()->DrawPrintText(L"Flashlight", wcslen(L"Flashlight"));
			}
			ypos += text2_gap;
		}

		if (pPlayer->IsSprinting())
		{
			tempString = g_pVGuiLocalize->Find("#Valve_Hud_SPRINT");

			surface()->DrawSetTextPos(text2_xpos, ypos);

			if (tempString)
			{
				surface()->DrawPrintText(tempString, wcslen(tempString));
			}
			else
			{
				surface()->DrawPrintText(L"Sprint", wcslen(L"Sprint"));
			}
			ypos += text2_gap;
		}
	}
}
