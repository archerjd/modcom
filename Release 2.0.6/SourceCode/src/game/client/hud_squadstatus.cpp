//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "hl2mp/c_hl2mp_player.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/AnimationController.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>
#include "modcom/mc_shareddefs.h"
#include "modcom/mcconvar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//ConVar player_squad_transient_commands( "player_squad_transient_commands", "1", FCVAR_REPLICATED );

//-----------------------------------------------------------------------------
// Purpose: Shows the sprint power bar
//-----------------------------------------------------------------------------
class CHudSquadStatus : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudSquadStatus, vgui::Panel );

public:
	CHudSquadStatus( const char *pElementName );
	virtual void Init( void );
	virtual void Reset( void );
	virtual void OnThink( void );
	bool ShouldDraw();

	void MsgFunc_SquadMemberDied(bf_read &msg);

protected:
	virtual void Paint();

private:
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, text_xpos, "text1_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text1_ypos", "6", "proportional_float" );
	CPanelAnimationVarAliasType( float, text2_xpos, "text2_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text2_ypos, "text2_ypos", "34", "proportional_float" );

	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "HudIcons" );
	CPanelAnimationVarAliasType( float, m_flIconInsetX, "IconInsetX", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flIconInsetY, "IconInsetY", "3", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flIconGap, "IconGap", "20", "proportional_float" );

	CPanelAnimationVar( Color, m_SquadIconColor, "SquadIconColor", "255 220 0 160" );
	CPanelAnimationVar( Color, m_LastMemberColor, "LastMemberColor", "255 220 0 0" );
	CPanelAnimationVar( Color, m_SquadTextColor, "SquadTextColor", "255 220 0 160" );
	
	int m_iSquadMembers;
	int m_iSquadMedics;
	bool m_bSquadMembersFollowing;
	bool m_bSquadMemberAdded;
	bool m_bSquadMemberJustDied;
};	


DECLARE_HUDELEMENT( CHudSquadStatus );
DECLARE_HUD_MESSAGE( CHudSquadStatus, SquadMemberDied );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSquadStatus::CHudSquadStatus( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudSquadStatus" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSquadStatus::Init( void )
{
	HOOK_HUD_MESSAGE( CHudSquadStatus, SquadMemberDied );
	m_iSquadMembers = 0;
	m_iSquadMedics = 0;
	m_bSquadMemberAdded = false;
	m_bSquadMembersFollowing = true;
	m_bSquadMemberJustDied = false;
	SetAlpha( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSquadStatus::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudSquadStatus::ShouldDraw( void )
{
	bool bNeedsDraw = false;

	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return false;

	int squadCount = pPlayer->NumAlliesInRange();

	bNeedsDraw = ( squadCount > 0 ||
					( squadCount != m_iSquadMembers ) || 
					//( pPlayer->m_HL2Local.m_fSquadInFollowMode != m_bSquadMembersFollowing ) ||
					( m_iSquadMembers > 0 ) ||
					( m_LastMemberColor[3] > 0 ) );
		
	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: updates hud icons
//-----------------------------------------------------------------------------
void CHudSquadStatus::OnThink( void )
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return;

	int squadMembers = C_HL2MP_Player::GetLocalHL2MPPlayer()->NumAlliesInRange();
	//bool following = pPlayer->m_HL2Local.m_fSquadInFollowMode;
	m_iSquadMedics = 0;

	// Only update if we've changed vars
	if ( squadMembers == m_iSquadMembers )//&& following == m_bSquadMembersFollowing )
		return;

	// update status display
	if ( squadMembers > 0)
	{
		// we have squad members, show the display
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "alpha", 255.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	}
	else
	{
		// no squad members, hide the display
		g_pClientMode->GetViewportAnimationController()->RunAnimationCommand( this, "alpha", 0.0f, 0.0f, 0.4f, AnimationController::INTERPOLATOR_LINEAR);
	}

	if ( squadMembers > m_iSquadMembers )
	{
		// someone is added
		// reset the last icon color and animate
		m_LastMemberColor = m_SquadIconColor;
		m_LastMemberColor[3] = 0;
		m_bSquadMemberAdded = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMemberAdded" ); 
	}
	else if ( squadMembers < m_iSquadMembers )
	{
		// someone has left
		// reset the last icon color and animate
		m_LastMemberColor = m_SquadIconColor;
		m_bSquadMemberAdded = false;
		if (m_bSquadMemberJustDied)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMemberDied" ); 
			m_bSquadMemberJustDied = false;
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMemberLeft" ); 
		}
	}
/*
	if ( following != m_bSquadMembersFollowing )
	{
		if ( following )
		{
			// flash the squad area to indicate they are following
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMembersFollowing" );
		}
		else
		{
			// flash the crosshair to indicate the targeted order is in effect
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SquadMembersStationed" );
		}
	}
*/
	m_iSquadMembers = squadMembers;
//	m_bSquadMembersFollowing = following;
}

//-----------------------------------------------------------------------------
// Purpose: Notification of squad member being killed
//-----------------------------------------------------------------------------
void CHudSquadStatus::MsgFunc_SquadMemberDied(bf_read &msg)
{
	m_bSquadMemberJustDied = true;
}

extern McConVar ally_experience_boost_fraction;
extern McConVar pvm_buff_combine_scale;

//-----------------------------------------------------------------------------
// Purpose: draws the power bar
//-----------------------------------------------------------------------------
void CHudSquadStatus::Paint()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return;

	// draw the suit power bar
	surface()->DrawSetTextColor( m_SquadIconColor );
	surface()->DrawSetTextFont( m_hIconFont );
	int xpos = m_flIconInsetX, ypos = m_flIconInsetY;
	for (int i = 0; i < m_iSquadMembers; i++)
	{
		if (m_bSquadMemberAdded && i == m_iSquadMembers - 1)
		{
			// draw the last added squad member specially
			surface()->DrawSetTextColor( m_LastMemberColor );
		}

		surface()->DrawSetTextPos(xpos, ypos);

		if (i < m_iSquadMedics)
		{
			surface()->DrawUnicodeChar('M');
		}
		else
		{
			surface()->DrawUnicodeChar('C');
		}
		xpos += m_flIconGap;
	}
	if (!m_bSquadMemberAdded && m_LastMemberColor[3])
	{
		// draw the last one in the special color
		surface()->DrawSetTextColor( m_LastMemberColor );
		surface()->DrawSetTextPos(xpos, ypos);
		surface()->DrawUnicodeChar('C');
	}
/*
	// draw our squad status
	wchar_t *text = NULL;
	if (m_bSquadMembersFollowing)
	{
		text = g_pVGuiLocalize->Find("#Valve_Hud_SQUAD_FOLLOWING");

		if (!text)
		{
			text = L"SQUAD FOLLOWING";
		}
	}
	else
	{
		if ( !player_squad_transient_commands.GetBool() )
		{
			text = g_pVGuiLocalize->Find("#Valve_Hud_SQUAD_STATIONED");

			if (!text)
			{
				text = L"SQUAD STATIONED";
			}
		}
	}*/

	wchar_t *text = L"NEARBY ALLIES";
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_SquadTextColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);
	surface()->DrawPrintText(text, wcslen(text));

	int iExpBonus = ally_experience_boost_fraction.GetFloat() * m_iSquadMembers * 100 + 0.5f;
	wchar_t effect[ 32 ];
	if ( m_iSquadMembers == 0 )
		_snwprintf( effect, 32, L"");
	else
		switch ( pPlayer->GetFaction() )
		{
		case FACTION_COMBINE:
		{
			int iDamageBonus = pvm_buff_combine_scale.GetFloat() * m_iSquadMembers * 100;
			_snwprintf( effect, 32, L"+%i%% dmg, +%i%% exp", iDamageBonus, iExpBonus);
			break;
		}
		case FACTION_RESISTANCE:
			_snwprintf( effect, 32, L"+%i health, +%i%% exp", m_iSquadMembers, iExpBonus);
			break;
		case FACTION_APERTURE:
			_snwprintf( effect, 32, L"+%i armor, +%i%% exp", m_iSquadMembers, iExpBonus);
			break;
		default:
			return;
		}

	surface()->DrawSetTextPos(text2_xpos, text2_ypos);
	surface()->DrawPrintText(effect, wcslen(effect));
}


