//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CHL2MPCLIENTSCOREBOARDDIALOG_H
#define CHL2MPCLIENTSCOREBOARDDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <clientscoreboarddialog.h>

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CHL2MPClientScoreBoardDialog : public CClientScoreBoardDialog
{
private:
	DECLARE_CLASS_SIMPLE(CHL2MPClientScoreBoardDialog, CClientScoreBoardDialog);
	
public:
	CHL2MPClientScoreBoardDialog(IViewPort *pViewPort);
	~CHL2MPClientScoreBoardDialog();


protected:
	// scoreboard overrides
	virtual void InitScoreboardSections();
	virtual void UpdateTeamInfo();
	virtual bool GetPlayerScoreInfo(int playerIndex, KeyValues *outPlayerInfo);
	virtual void UpdatePlayerInfo();

	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	virtual void AddHeader(); // add the start header of the scoreboard
	virtual void AddSection(int teamType, int teamNumber); // add a new section header for a team

	int GetSectionFromTeamNumber( int teamNumber );
	enum 
	{ 
		CSTRIKE_NAME_WIDTH = 136,
		CSTRIKE_SUIT_WIDTH = 88, 
		CSTRIKE_EXP_WIDTH = 38,
		CSTRIKE_SCORE_WIDTH = 38,
		CSTRIKE_DEATH_WIDTH = 38,
		CSTRIKE_LEVEL_WIDTH = 38,
		CSTRIKE_ISDEAD_WIDTH = 16, // need to get enough width for this, but must remove it from the other columns
		CSTRIKE_PING_WIDTH = 38,
//		CSTRIKE_VOICE_WIDTH = 40, 
//		CSTRIKE_FRIENDS_WIDTH = 24,
	};

	int m_iDeadImageIndex;

	// rounded corners
	Color					 m_bgColor;
	Color					 m_borderColor;
};


#endif // CHL2MPCLIENTSCOREBOARDDIALOG_H
