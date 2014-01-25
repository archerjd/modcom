//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD Target ID element
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_hl2mp_player.h"
#include "c_playerresource.h"
#include "vgui_EntityPanel.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "hl2mp_gamerules.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PLAYER_HINT_DISTANCE	150
#define PLAYER_HINT_DISTANCE_SQ	(PLAYER_HINT_DISTANCE*PLAYER_HINT_DISTANCE)

static ConVar hud_centerid( "hud_centerid", "1" );
static ConVar hud_showtargetid( "hud_showtargetid", "1" );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTargetID : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CTargetID, vgui::Panel );

public:
	CTargetID( const char *pElementName );
	void Init( void );
	virtual void	ApplySchemeSettings( vgui::IScheme *scheme );
	virtual void	Paint( void );
	void VidInit( void );

private:
	Color			GetColorForTargetTeam( int iTeamNumber );

	vgui::HFont		m_hFont;
	int				m_iLastEntIndex;
	float			m_flLastChangeTime;
};

DECLARE_HUDELEMENT( CTargetID );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTargetID::CTargetID( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "TargetID" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_hFont = g_hFontTrebuchet24;
	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;

	SetHiddenBits( HIDEHUD_MISCSTATUS );
}

//-----------------------------------------------------------------------------
// Purpose: Setup
//-----------------------------------------------------------------------------
void CTargetID::Init( void )
{
};

void CTargetID::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_hFont = scheme->GetFont( "TargetID", IsProportional() );

	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: clear out string etc between levels
//-----------------------------------------------------------------------------
void CTargetID::VidInit()
{
	CHudElement::VidInit();

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}

extern ConVar mc_gamemode;
Color CTargetID::GetColorForTargetTeam( int iTeamNumber )
{
	if ( HL2MPRules()->IsTeamplay() )
		return GameResources()->GetTeamColor( iTeamNumber );
	else
		return GameResources()->GetTeamColor( FACTION_NONE );
} 

// convert int to unicode string
void itow(wchar_t out[],int i)
{
	_snwprintf(out,sizeof(out),L"%i",i); // possibly try numLen*sizeof(wchar_t) for these?
}

//-----------------------------------------------------------------------------
// Purpose: Draw function for the element
//-----------------------------------------------------------------------------
void CTargetID::Paint()
{
	if ( hud_showtargetid.GetInt() == 0 )
		return;

#define MAX_ID_LINE 96
	wchar_t sLine1[ MAX_ID_LINE ];
	wchar_t sLine2[ MAX_ID_LINE ];
	wchar_t sLine3[ MAX_ID_LINE ];
	wchar_t sLine4[ MAX_ID_LINE ];
	sLine1[0] = 0;
	sLine2[0] = 0;
	sLine3[0] = 0;
	sLine4[0] = 0;

	C_HL2MP_Player *pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();

	if ( !pLocalPlayer )
		return;

	Color c;

	// Get our target's ent index
	int iEntIndex = pLocalPlayer->GetIDTarget();
	// Didn't find one?
	if ( !iEntIndex )
	{
		// Check to see if we should clear our ID
		if ( m_flLastChangeTime && (gpGlobals->curtime > (m_flLastChangeTime + 0.5)) )
		{
			m_flLastChangeTime = 0;
			sLine1[0] = sLine2[0] = sLine3[0] = sLine4[0] = 0;
			m_iLastEntIndex = 0;
		}
		else
		{
			// Keep re-using the old one
			iEntIndex = m_iLastEntIndex;
		}
	}
	else
	{
		m_flLastChangeTime = gpGlobals->curtime;
	}

	// Is this an entindex sent by the server?
	if ( iEntIndex )
	{
		C_BaseCombatCharacter *pOther = dynamic_cast<C_BaseCombatCharacter*>(cl_entitylist->GetEnt( iEntIndex ));
		if ( !pOther )
			return;

		bool isFriendly = HL2MPRules()->IsFriendly(pLocalPlayer, pOther);
		if ( pOther->GetRenderColor().a < 50 && !isFriendly )
			return; // abort if they're cloaked and hostile

		int level = pOther->GetLevel();
		int health;
		wchar_t extra[32];
		bool hasExtra = false;
		const char *line2Format;
		if ( pOther->IsPlayer() )
		{
			C_HL2MP_Player *pPlayer = static_cast<C_HL2MP_Player*>(pOther);
			c = GetColorForTargetTeam( pPlayer->GetFaction() );
			g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(),  sLine1, sizeof(sLine1) );
			line2Format = isFriendly ? "#Playerid_sameteam" : "#Playerid_diffteam";
			health = isFriendly ? pPlayer->GetHealth() : -1;
		}
		else if ( pOther->IsNPC() )
		{
			C_AI_BaseNPC *pNPC = pOther->MyNPCPointer();
			g_pVGuiLocalize->ConvertANSIToUnicode( pNPC->GetTypeName(),  sLine1, sizeof(sLine1) );
			if ( pNPC->LikesMaster() )
			{
				if ( pNPC->GetMasterPlayer() == pLocalPlayer && isFriendly )
				{
					line2Format = "#Minionid_mine";
					health = pNPC->GetHealth();
					_snwprintf(extra, sizeof(extra), L"%s", pNPC->CustomStatusMessage());
					hasExtra = true;
					c = GetColorForTargetTeam( pLocalPlayer->GetFaction() );
				}
				else if ( pNPC->GetMasterPlayer() == NULL ) // handle UNKNOWN MINIONS just in case they come up again
				{
					line2Format = "#Minionid_diffteam";
					g_pVGuiLocalize->ConstructString( sLine4, sizeof(sLine4), g_pVGuiLocalize->Find("Minion_ownerid"), 1, "Unknown" );
					health = -1;
					c = GameResources()->GetTeamColor( FACTION_NONE );
				}
				else
				{
					if ( isFriendly )
					{
						wchar_t wMasterName[64];
						g_pVGuiLocalize->ConvertANSIToUnicode( UTIL_SafeName( ToHL2MPPlayer(pNPC->GetMasterPlayer())->GetPlayerName() ), wMasterName, sizeof( wMasterName ) );
						g_pVGuiLocalize->ConstructString( sLine4, sizeof(sLine4), g_pVGuiLocalize->Find("Minion_ownerid"), 1, wMasterName );
						line2Format = "#Minionid_sameteam";
					}
					else
						line2Format = "#Minionid_diffteam";
					health = -1;
					c = GetColorForTargetTeam( ToHL2MPPlayer(pNPC->GetMasterPlayer())->GetFaction() );
				}
			}
			else
			{
				line2Format = isFriendly ? "#Monsterid_sameteam" : "#Monsterid_diffteam";
				health = isFriendly ? pNPC->GetHealth() : -1;
				c = GameResources()->GetTeamColor( FACTION_NONE );
			}
		}
		else
		{
			return; // ???
		}

		// line 2 - type info
		g_pVGuiLocalize->ConstructString( sLine2, sizeof(sLine2), g_pVGuiLocalize->Find(line2Format), 0 );

		// fill out level on line 3
		wchar_t szLevel[4];
		itow(szLevel,level);
		g_pVGuiLocalize->ConstructString( sLine3, sizeof(sLine3), g_pVGuiLocalize->Find("Playerid_level"), 1, szLevel );

		// fill out health on line 4... if its showing minion owner, this is set already
		if ( health > -1 )
		{
			wchar_t szHealth[6];
			itow(szHealth,health);
			g_pVGuiLocalize->ConstructString( sLine4, sizeof(sLine4), g_pVGuiLocalize->Find("Playerid_health"), 2, szHealth, hasExtra ? extra : L"" );
		}


		int wide=0, tall=0;
		int ypos = YRES(260);
		int xpos = XRES(10);
		if ( sLine1[0] )
		{
			vgui::surface()->GetTextSize( m_hFont, sLine1, wide, tall );

			if( hud_centerid.GetInt() == 0 )
				ypos = YRES(350);
			xpos = (ScreenWidth() - wide) / 2;
			
			vgui::surface()->DrawSetTextFont( m_hFont );
			vgui::surface()->DrawSetTextPos( xpos, ypos );
			vgui::surface()->DrawSetTextColor( c );
			vgui::surface()->DrawPrintText( sLine1, wcslen(sLine1) );
		}
		if ( sLine2[0] )
		{
			ypos += tall + 3;
			vgui::surface()->GetTextSize( m_hFont, sLine2, wide, tall );
			xpos = (ScreenWidth() - wide) / 2;
			vgui::surface()->DrawSetTextPos( xpos, ypos );
			vgui::surface()->DrawPrintText( sLine2, wcslen(sLine2) );
		}
		if ( sLine3[0] )
		{
			ypos += tall + 3;
			vgui::surface()->GetTextSize( m_hFont, sLine3, wide, tall );
			xpos = (ScreenWidth() - wide) / 2;
			vgui::surface()->DrawSetTextPos( xpos, ypos );
			vgui::surface()->DrawPrintText( sLine3, wcslen(sLine3) );
		}
		if ( sLine4[0] )
		{
			ypos += tall + 3;
			vgui::surface()->GetTextSize( m_hFont, sLine4, wide, tall );
			xpos = (ScreenWidth() - wide) / 2;
			vgui::surface()->DrawSetTextPos( xpos, ypos );
			vgui::surface()->DrawPrintText( sLine4, wcslen(sLine4) );
		}
	}
}
