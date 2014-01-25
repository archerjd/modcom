//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
//  hud_msg.cpp
//
#include "cbase.h"
#include "clientmode.h"
#include "hudelement.h"
#include "KeyValues.h"
#include "vgui_controls/AnimationController.h"
#include "engine/IEngineSound.h"
#include <bitbuf.h>
#include <game/client/iviewport.h>
#include "modcom/vote_panel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/// USER-DEFINED SERVER MESSAGE HANDLERS

void CHud::MsgFunc_ResetHUD( bf_read &msg )
{
	ResetHUD();
}

void CHud::ResetHUD()
{
	// clear all hud data
	g_pClientMode->GetViewportAnimationController()->CancelAllAnimations();

	for ( int i = 0; i < m_HudList.Size(); i++ )
	{
		m_HudList[i]->Reset();
	}

	g_pClientMode->GetViewportAnimationController()->RunAllAnimationsToCompletion();
#ifndef _XBOX
	// reset sensitivity
	m_flMouseSensitivity = 0;
	m_flMouseSensitivityFactor = 0;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Used for BoSS talk
//-----------------------------------------------------------------------------

char szSound[2048];
void CHud::MsgFunc_SendAudio( bf_read &msg )
{
	// first, stop the previous sound if its still running
	C_BaseEntity::StopSound( SOUND_FROM_LOCAL_PLAYER, szSound );

	// now read and play the desired sound
	msg.ReadString( szSound, sizeof(szSound ) );
	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, szSound );
}

void CHud::MsgFunc_Vote( bf_read &msg )
{
	// read the game mode
	int mode = msg.ReadByte();

	char message[256];
	msg.ReadString(message, 256);

	//KeyValues * pMessage = new KeyValues("VoteMessage", "message", message);

	if ( mode == 0 )
		gViewPortInterface->ShowPanel( PANEL_VOTE, false );
	else
	{
		CVotePanel *pPanel = (CVotePanel*)gViewPortInterface->FindPanelByName( PANEL_VOTE );
		if ( pPanel != NULL )
			pPanel->SetMessage(message);
		else
			Error("Vote panel is null, can't find it!\n");
		gViewPortInterface->ShowPanel( PANEL_VOTE, true );
 	}
}
