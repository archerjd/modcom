//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hl2mpclientscoreboard.h"
#include "c_team.h"
#include "c_playerresource.h"
#include "c_hl2mp_player.h"
#include "hl2mp_gamerules.h"

#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/SectionedListPanel.h>
#include <vgui_controls/ImageList.h>
#include "modcom/mc_shareddefs.h"

#include "voice_status.h"

using namespace vgui;

#define TEAM_MAXCOUNT			6

// id's of sections used in the scoreboard
enum EScoreboardSections
{
	SCORESECTION_EMPTYPART = FACTION_NONE,
	SCORESECTION_COMBINE = FACTION_COMBINE,
	SCORESECTION_RESISTANCE = FACTION_RESISTANCE,
	SCORESECTION_APERTURE = FACTION_APERTURE,
	SCORESECTION_FREEFORALL = NUM_FACTIONS+1,
	SCORESECTION_SPECTATOR = NUM_FACTIONS+2,
};

const int NumSegments = 7;
static int coord[NumSegments+1] = {
	0,
	1,
	2,
	3,
	4,
	6,
	9,
	10
};

//-----------------------------------------------------------------------------
// Purpose: Konstructor
//-----------------------------------------------------------------------------
CHL2MPClientScoreBoardDialog::CHL2MPClientScoreBoardDialog(IViewPort *pViewPort):CClientScoreBoardDialog(pViewPort)
{
	m_iDeadImageIndex = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHL2MPClientScoreBoardDialog::~CHL2MPClientScoreBoardDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: Paint background for rounded corners
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::PaintBackground()
{
	m_pPlayerList->SetBgColor( Color(0, 0, 0, 0) );
	m_pPlayerList->SetBorder(NULL);

	int x1, x2, y1, y2;
	surface()->DrawSetColor(m_bgColor);
	surface()->DrawSetTextColor(m_bgColor);

	int wide, tall;
	GetSize( wide, tall );

	int i;

	// top-left corner --------------------------------------------------------
	int xDir = 1;
	int yDir = -1;
	int xIndex = 0;
	int yIndex = NumSegments - 1;
	int xMult = 1;
	int yMult = 1;
	int x = 0;
	int y = 0;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = y + coord[NumSegments];
		surface()->DrawFilledRect( x1, y1, x2, y2 );

		xIndex += xDir;
		yIndex += yDir;
	}

	// top-right corner -------------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = wide;
	y = 0;
	xMult = -1;
	yMult = 1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = y + coord[NumSegments];
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// bottom-right corner ----------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = wide;
	y = tall;
	xMult = -1;
	yMult = -1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = y - coord[NumSegments];
		y2 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// bottom-left corner -----------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = 0;
	y = tall;
	xMult = 1;
	yMult = -1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = y - coord[NumSegments];
		y2 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// paint between top left and bottom left ---------------------------------
	x1 = 0;
	x2 = coord[NumSegments];
	y1 = coord[NumSegments];
	y2 = tall - coord[NumSegments];
	surface()->DrawFilledRect( x1, y1, x2, y2 );

	// paint between left and right -------------------------------------------
	x1 = coord[NumSegments];
	x2 = wide - coord[NumSegments];
	y1 = 0;
	y2 = tall;
	surface()->DrawFilledRect( x1, y1, x2, y2 );
	
	// paint between top right and bottom right -------------------------------
	x1 = wide - coord[NumSegments];
	x2 = wide;
	y1 = coord[NumSegments];
	y2 = tall - coord[NumSegments];
	surface()->DrawFilledRect( x1, y1, x2, y2 );
}

//-----------------------------------------------------------------------------
// Purpose: Paint border for rounded corners
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::PaintBorder()
{
	int x1, x2, y1, y2;
	surface()->DrawSetColor(m_borderColor);
	surface()->DrawSetTextColor(m_borderColor);

	int wide, tall;
	GetSize( wide, tall );

	int i;

	// top-left corner --------------------------------------------------------
	int xDir = 1;
	int yDir = -1;
	int xIndex = 0;
	int yIndex = NumSegments - 1;
	int xMult = 1;
	int yMult = 1;
	int x = 0;
	int y = 0;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );

		xIndex += xDir;
		yIndex += yDir;
	}

	// top-right corner -------------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = wide;
	y = 0;
	xMult = -1;
	yMult = 1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// bottom-right corner ----------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = wide;
	y = tall;
	xMult = -1;
	yMult = -1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// bottom-left corner -----------------------------------------------------
	xDir = 1;
	yDir = -1;
	xIndex = 0;
	yIndex = NumSegments - 1;
	x = 0;
	y = tall;
	xMult = 1;
	yMult = -1;
	for ( i=0; i<NumSegments; ++i )
	{
		x1 = min( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		x2 = max( x + coord[xIndex]*xMult, x + coord[xIndex+1]*xMult );
		y1 = min( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		y2 = max( y + coord[yIndex]*yMult, y + coord[yIndex+1]*yMult );
		surface()->DrawFilledRect( x1, y1, x2, y2 );
		xIndex += xDir;
		yIndex += yDir;
	}

	// top --------------------------------------------------------------------
	x1 = coord[NumSegments];
	x2 = wide - coord[NumSegments];
	y1 = 0;
	y2 = 1;
	surface()->DrawFilledRect( x1, y1, x2, y2 );

	// bottom -----------------------------------------------------------------
	x1 = coord[NumSegments];
	x2 = wide - coord[NumSegments];
	y1 = tall - 1;
	y2 = tall;
	surface()->DrawFilledRect( x1, y1, x2, y2 );

	// left -------------------------------------------------------------------
	x1 = 0;
	x2 = 1;
	y1 = coord[NumSegments];
	y2 = tall - coord[NumSegments];
	surface()->DrawFilledRect( x1, y1, x2, y2 );

	// right ------------------------------------------------------------------
	x1 = wide - 1;
	x2 = wide;
	y1 = coord[NumSegments];
	y2 = tall - coord[NumSegments];
	surface()->DrawFilledRect( x1, y1, x2, y2 );
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	// resize "dead" image at start of image list
	BaseClass::ApplySchemeSettings( pScheme );

	m_bgColor = GetSchemeColor("SectionedListPanel.BgColor", GetBgColor(), pScheme);
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBgColor( Color(0, 0, 0, 0) );
	SetBorder( pScheme->GetBorder( "BaseBorder" ) );
}

extern ConVar mc_gamemode;

//-----------------------------------------------------------------------------
// Purpose: sets up base sections
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::InitScoreboardSections()
{
	m_pPlayerList->SetBgColor( Color(0, 0, 0, 0) );
	m_pPlayerList->SetBorder(NULL);

	IImage *image = scheme()->GetImage("gfx/dead", true);
	int wide, tall;
	image->GetSize(wide, tall);
	image->SetSize(wide*0.85f, tall*0.85f);
	m_iDeadImageIndex = m_pImageList->AddImage(image);
	
	// fill out the structure of the scoreboard
	AddHeader();

	if ( HL2MPRules()->IsTeamplay() )
	{
		// add the team sections
		AddSection( TYPE_TEAM, SCORESECTION_COMBINE );
		AddSection( TYPE_TEAM, SCORESECTION_RESISTANCE );
		AddSection( TYPE_TEAM, SCORESECTION_APERTURE );
	}
	else
	{
		AddSection( TYPE_TEAM, SCORESECTION_FREEFORALL );
	}
	AddSection( TYPE_SPECTATORS, SCORESECTION_SPECTATOR );
}

int CalcFactionPing(int faction)
{
	int total = 0, num = 0;
	for ( int i=1; i<=MAX_PLAYERS; i++ )
		if ( g_PR->IsConnected(i) && g_PR->GetFaction(i) == faction )
		{
			num++;
			total += g_PR->GetPing(i);
		}

	if ( num > 0 )
		return (int)((float)total / (float)num);
	return 0;
}

int CalcPlayersOnFaction(int faction)
{
	int num = 0;
	for ( int i=1; i<=MAX_PLAYERS; i++ )
		if ( g_PR->IsConnected(i) && g_PR->GetFaction(i) == faction )
			num++;

	return num;
}

//-----------------------------------------------------------------------------
// Purpose: resets the scoreboard team info
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::UpdateTeamInfo()
{
	if ( g_PR == NULL )
		return;

	/*int iNumPlayersInGame = 0;

	for ( int j = 1; j <= gpGlobals->maxClients; j++ )
		if ( g_PR->IsConnected( j ) )
			iNumPlayersInGame++;*/

	// update the team sections in the scoreboard
	for ( int i = 0; i <= NUM_FACTIONS+2; i++ )
	{
		wchar_t *teamName = NULL;
		//int sectionID = i;
		/*C_Team *team = GetGlobalTeam(i);

		if ( team )
		*/{
			//sectionID = GetSectionFromTeamNumber( i );
	
			// update team name
			wchar_t name[64];
			wchar_t string1[1024];
			wchar_t wNumPlayers[6];

			int playerCount = i == SCORESECTION_SPECTATOR ? CalcPlayersOnFaction(0) : i == SCORESECTION_FREEFORALL ? CalcPlayersOnFaction(FACTION_COMBINE) + CalcPlayersOnFaction(FACTION_RESISTANCE) + CalcPlayersOnFaction(FACTION_APERTURE): CalcPlayersOnFaction(i);
			_snwprintf(wNumPlayers, 6, L"%i", playerCount);

			if ( HL2MPRules()->IsTeamplay() == false )
			{
				//_snwprintf(wNumPlayers, 6, L"%i", iNumPlayersInGame );
				//_snwprintf( name, sizeof(name), L"%s", g_pVGuiLocalize->Find("#ScoreBoard_Deathmatch") );
				if ( i == SCORESECTION_FREEFORALL )
					_snwprintf( name, sizeof(name), L"Active Players");
				else
					_snwprintf( name, sizeof(name), L"Unassigned");
									
				teamName = name;

				if ( playerCount == 1)
					g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find("#ScoreBoard_Player"), 2, teamName, wNumPlayers );
				else
					g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find("#ScoreBoard_Players"), 2, teamName, wNumPlayers );

				/*if ( i > 0 )
				{
					m_pPlayerList->ModifyColumn(i, "exp", L"");
					m_pPlayerList->ModifyColumn(i, "deaths", L"");
					m_pPlayerList->ModifyColumn(i, "level", L"");
					m_pPlayerList->ModifyColumn(i, "frags", L"");
					m_pPlayerList->ModifyColumn(i, "ping", L"");
				}*/
			}
			else
			{
				switch ( i )
				{
				case SCORESECTION_COMBINE:
					_snwprintf( name, sizeof(name), L"Combine"); break;
				case SCORESECTION_RESISTANCE:
					_snwprintf( name, sizeof(name), L"Resistance"); break;
				case SCORESECTION_APERTURE:
					_snwprintf( name, sizeof(name), L"Aperture"); break;
				case 0: // nothing for the very top column, please!
					_snwprintf( name, sizeof(name), L""); break;
				default:
					_snwprintf( name, sizeof(name), L"Unassigned"); break;
				}

				teamName = name;

				if (playerCount == 1)
				{
					g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find("#ScoreBoard_Player"), 2, teamName, wNumPlayers );
				}
				else
				{
					g_pVGuiLocalize->ConstructString( string1, sizeof(string1), g_pVGuiLocalize->Find("#ScoreBoard_Players"), 2, teamName, wNumPlayers );
				}

				if ( i > 0 )
				{
					wchar_t val[8];
					
					int iNum = HL2MPRules()->GetFactionExperience(i);
					_snwprintf(val, 8, L"%i", iNum );

					m_pPlayerList->ModifyColumn(i, "exp", val);

					int factionPing = CalcFactionPing(i);
					if (factionPing < 1)
					{
						m_pPlayerList->ModifyColumn(i, "ping", L"");
					}
					else
					{
						swprintf(val, L"%d", factionPing);
						m_pPlayerList->ModifyColumn(i, "ping", val);
					}
				}
			}
		
			if ( i > 0 )
				m_pPlayerList->ModifyColumn(i, "name", string1);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::AddHeader()
{
	// add the top header
	m_pPlayerList->AddSection(0, "");
	m_pPlayerList->SetSectionAlwaysVisible(0);
	HFont hFallbackFont = scheme()->GetIScheme( GetScheme() )->GetFont( "DefaultVerySmallFallBack", false );
	m_pPlayerList->AddColumnToSection(0, "name", "#PlayerName", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_NAME_WIDTH ), hFallbackFont );
	m_pPlayerList->AddColumnToSection(0, "isdead", "", SectionedListPanel::COLUMN_IMAGE, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_ISDEAD_WIDTH ) );	
	m_pPlayerList->AddColumnToSection(0, "suit", "#PlayerSuit", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_SUIT_WIDTH ) );
	m_pPlayerList->AddColumnToSection(0, "exp", "#PlayerExp", 0 | SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_EXP_WIDTH ) );
	m_pPlayerList->AddColumnToSection(0, "frags", "#PlayerScore", 0 | SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_SCORE_WIDTH ) );
	m_pPlayerList->AddColumnToSection(0, "deaths", "#PlayerDeath", 0 | SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_DEATH_WIDTH ) );
	m_pPlayerList->AddColumnToSection(0, "level", "#PlayerLevel", 0 | SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_LEVEL_WIDTH ) );
	m_pPlayerList->AddColumnToSection(0, "ping", "#PlayerPing", 0 | SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_PING_WIDTH ) );
	
//	m_pPlayerList->AddColumnToSection(0, "voice", "#PlayerVoice", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::HEADER_TEXT| SectionedListPanel::COLUMN_CENTER, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_VOICE_WIDTH ) );
//	m_pPlayerList->AddColumnToSection(0, "tracker", "#PlayerTracker", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::HEADER_TEXT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_FRIENDS_WIDTH ) );
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new section to the scoreboard (i.e the team header)
//-----------------------------------------------------------------------------
void CHL2MPClientScoreBoardDialog::AddSection(int teamType, int sectionID)
{
	HFont hFallbackFont = scheme()->GetIScheme( GetScheme() )->GetFont( "DefaultVerySmallFallBack", false );

	if ( teamType == TYPE_TEAM )
	{
 		m_pPlayerList->AddSection(sectionID, "", StaticPlayerSortFunc);

		// setup the columns
		m_pPlayerList->AddColumnToSection(sectionID, "name", "", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_NAME_WIDTH ), hFallbackFont );
		m_pPlayerList->AddColumnToSection(sectionID, "isdead", "", SectionedListPanel::COLUMN_IMAGE, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_ISDEAD_WIDTH ) );	
		m_pPlayerList->AddColumnToSection(sectionID, "suit", "" , 0, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_SUIT_WIDTH ), hFallbackFont );
		m_pPlayerList->AddColumnToSection(sectionID, "exp", "", SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_EXP_WIDTH ) );
		m_pPlayerList->AddColumnToSection(sectionID, "frags", "", SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_SCORE_WIDTH ) );
		m_pPlayerList->AddColumnToSection(sectionID, "deaths", "", SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_DEATH_WIDTH ) );
		m_pPlayerList->AddColumnToSection(sectionID, "level", "", SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_LEVEL_WIDTH ) );
		m_pPlayerList->AddColumnToSection(sectionID, "ping", "", SectionedListPanel::COLUMN_RIGHT, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_PING_WIDTH ) );

		// set the section to have the team color
		if ( HL2MPRules()->IsTeamplay() || sectionID == SCORESECTION_SPECTATOR )
		{
			if ( GameResources() )
				m_pPlayerList->SetSectionFgColor(sectionID,  GameResources()->GetTeamColor(sectionID));
		}

		m_pPlayerList->SetSectionAlwaysVisible(sectionID);
	}
	else if ( teamType == TYPE_SPECTATORS )
	{
		m_pPlayerList->AddSection(sectionID, "");
		m_pPlayerList->AddColumnToSection(sectionID, "name", "#Spectators", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), CSTRIKE_NAME_WIDTH ), hFallbackFont );
	}

	m_pPlayerList->SetSectionAlwaysVisible(sectionID);
}

int CHL2MPClientScoreBoardDialog::GetSectionFromTeamNumber( int playerNumber )
{
	int teamNumber = g_PR->GetFaction(playerNumber);
	if ( teamNumber <= FACTION_NONE )
		teamNumber = SCORESECTION_SPECTATOR;
	bool inChar = g_PR->IsInCharacter(playerNumber);
	
	if ( HL2MPRules()->IsTeamplay() || !inChar || teamNumber == SCORESECTION_SPECTATOR )
		return teamNumber;
	
	return SCORESECTION_FREEFORALL;
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new row to the scoreboard, from the playerinfo structure
//-----------------------------------------------------------------------------
bool CHL2MPClientScoreBoardDialog::GetPlayerScoreInfo(int playerIndex, KeyValues *kv)
{
	kv->SetInt("playerIndex", playerIndex);
	kv->SetInt("faction", g_PR->GetFaction( playerIndex ) );
	kv->SetInt("inCharacter", g_PR->IsInCharacter( playerIndex ) ? 1 : 0 );
	kv->SetString("name", g_PR->GetPlayerName(playerIndex) );
	kv->SetString("suit", g_PR->GetSuitName(playerIndex) );
	kv->SetInt("deaths", g_PR->GetDeaths( playerIndex ));
	kv->SetInt("level", g_PR->GetLevel( playerIndex));
	kv->SetInt("frags", g_PR->GetPlayerScore( playerIndex ));
	kv->SetInt("exp", g_PR->GetGameExp( playerIndex ));
	
	if (g_PR->GetPing( playerIndex ) < 1)
	{
		if ( g_PR->IsFakePlayer( playerIndex ) )
		{
			kv->SetString("ping", "BOT");
		}
		else
		{
			kv->SetString("ping", "");
		}
	}
	else
	{
		kv->SetInt("ping", g_PR->GetPing( playerIndex ));
	}
	
	kv->SetInt("isdead", g_PR->IsAlive( playerIndex ) ? -1 : m_iDeadImageIndex);
	
	return true;
}

enum {
	MAX_PLAYERS_PER_TEAM = 16,
	MAX_SCOREBOARD_PLAYERS = 32
};
struct PlayerScoreInfo
{
	int index;
	int level;
	int exp;
	int frags;
	int deaths;
	//bool important;
	int faction;
	bool inCharacter;
};

int PlayerScoreInfoSort( const PlayerScoreInfo *p1, const PlayerScoreInfo *p2 )
{
	// check local
/*	if ( p1->important )
		return -1;
	if ( p2->important )
		return 1;
*/
	// check alive
/*	if ( p1->alive && !p2->alive )
		return -1;
	if ( p2->alive && !p1->alive )
		return 1;
*/
	// check exp
	if ( p1->exp > p2->exp )
		return -1;
	if ( p2->exp > p1->exp )
		return 1;

	// check frags
	if ( p1->frags > p2->frags )
		return -1;
	if ( p2->frags > p1->frags )
		return 1;

	// check deaths
	if ( p1->deaths < p2->deaths )
		return -1;
	if ( p2->deaths < p1->deaths )
		return 1;

	// check level
	if ( p1->level > p2->level )
		return -1;
	if ( p2->level > p1->level )
		return 1;

	// check index
	if ( p1->index < p2->index )
		return -1;

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
extern ConVar mc_gamemode;
void CHL2MPClientScoreBoardDialog::UpdatePlayerInfo()
{
	m_iSectionId = 0; // 0'th row is a header
	int selectedRow = -1;
	int i;

	CBasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !g_PR )
		return;

	Label *control = (Label*)FindChildByName( "GameMode" );
	if ( control )
	{
/*		const char *modename;
		if ( mc_gamemode.GetInt() == DEATHMATCH )
			modename = "Deathmatch";
		else if (mc_gamemode.GetInt() == RANDOM_PVM)
			modename = "Players vs Monsters";
		else if (mc_gamemode.GetInt() == FFA)
			modename = "Free-for-All";
		else if (mc_gamemode.GetInt() == TEAM_DEATHMATCH )
			modename = "Team Deathmatch";
		else if (mc_gamemode.GetInt() == TEAM_FFA )
			modename = "Team Free-for-All";
		else
			modename = "Unknown game mode";
*/
		wchar_t *tempString = g_pVGuiLocalize->Find(VarArgs("#mode%i",mc_gamemode.GetInt()));
		if (tempString)
			control->SetText(tempString);
		else
			control->SetText(L"Unknown game mode");
	}

	control = (Label*)FindChildByName( "TimeLeft" );
	if ( control )
	{
		int iTimeRemaining = max(0, (int)HL2MPRules()->GetMapRemainingTime());

		if(iTimeRemaining != 0)
		{
			//int min = (int)(ftime);
			//int sec = (int)((ftime - min) * 60);
			
			int iMinutes, iSeconds;
			iMinutes = iTimeRemaining / 60;
			iSeconds = iTimeRemaining % 60;

			char minutes[8];
			char seconds[8];

			Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
			Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

			char* tempString = VarArgs("%s:%s", minutes, seconds);

			if (tempString)
				control->SetText(tempString);
			else
				control->SetText(L"0:00");
		}
		else
			control->SetText(L"No Limit");
	}

	control = (Label*)FindChildByName( "MapName" );
	if ( control )
	{
		const char* tempString = engine->GetLevelName();

		tempString += 5;

		if(tempString)
			control->SetText(tempString);
		else
			control->SetText(L"mc_map");
	}

	// walk all the players and make sure they're in the scoreboard
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		bool shouldShow = g_PR->IsConnected( i );
		if ( shouldShow )
		{
			// add the player to the list
			KeyValues *playerData = new KeyValues("data");
			GetPlayerScoreInfo( i, playerData );
			int itemID = FindItemIDForPlayerIndex( i );
  			int sectionID = GetSectionFromTeamNumber( i );// just pass in player index, can consider faction and whether they're in character
						
			if (itemID == -1)
			{
				// add a new row
				itemID = m_pPlayerList->AddItem( sectionID, playerData );
			}
			else
			{
				// modify the current row
				m_pPlayerList->ModifyItem( itemID, sectionID, playerData );
			}

			if ( i == pPlayer->entindex() )
			{
				selectedRow = itemID;	// this is the local player, hilight this row
			}

			// set the row color based on the players team
			m_pPlayerList->SetItemFgColor( itemID, g_PR->GetTeamColor( g_PR->GetFaction( i ) ) );

			playerData->deleteThis();
		}
		else
		{
			// remove the player
			int itemID = FindItemIDForPlayerIndex( i );
			if (itemID != -1)
			{
				m_pPlayerList->RemoveItem(itemID);
			}
		}
	}

	if ( selectedRow != -1 )
	{
		m_pPlayerList->SetSelectedItem(selectedRow);
	}

	
}
