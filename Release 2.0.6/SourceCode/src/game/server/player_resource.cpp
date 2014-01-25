//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "player.h"
#include "player_resource.h"
#include <coordsize.h>
#include "hl2mp_player.h"
#include "modcom/mc_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void SendProxy_Stringt_ToString( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	string_t *pString = (string_t*)pData;
	pOut->m_pString = (char*)STRING( *pString );
}

// Datatable
IMPLEMENT_SERVERCLASS_ST_NOBASE(CPlayerResource, DT_PlayerResource)
	SendPropArray( SendPropString( SENDINFO_ARRAY( m_szSuit ), 0, SendProxy_Stringt_ToString ), m_szSuit ),
	SendPropArray3( SENDINFO_ARRAY3(m_iPing), SendPropInt( SENDINFO_ARRAY(m_iPing), 10, SPROP_UNSIGNED ) ),
//	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iPacketloss), 7, SPROP_UNSIGNED ), m_iPacketloss ),
	SendPropArray3( SENDINFO_ARRAY3(m_iScore), SendPropInt( SENDINFO_ARRAY(m_iScore), 12 ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iDeaths), SendPropInt( SENDINFO_ARRAY(m_iDeaths), 12 ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_bConnected), SendPropInt( SENDINFO_ARRAY(m_bConnected), 1, SPROP_UNSIGNED ) ),
	//SendPropArray3( SENDINFO_ARRAY3(m_iTeam), SendPropInt( SENDINFO_ARRAY(m_iTeam), 4 ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_bAlive), SendPropInt( SENDINFO_ARRAY(m_bAlive), 1, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iHealth), SendPropInt( SENDINFO_ARRAY(m_iHealth), 8, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iLevel), SendPropInt( SENDINFO_ARRAY(m_iLevel), 5, SPROP_UNSIGNED  ) ),
//	SendPropArray3( SENDINFO_ARRAY3(m_iTotalExp), SendPropInt( SENDINFO_ARRAY(m_iTotalExp), 19, SPROP_UNSIGNED  ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iGameExp), SendPropInt( SENDINFO_ARRAY(m_iGameExp), 19, SPROP_UNSIGNED  ) ),

	SendPropArray3( SENDINFO_ARRAY3(m_bInChar), SendPropBool( SENDINFO_ARRAY(m_bInChar) ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iFaction), SendPropInt( SENDINFO_ARRAY(m_iFaction), 3, SPROP_UNSIGNED  ) ),
	SendPropArray( SendPropVector( SENDINFO_ARRAY(m_vecPos), -1, SPROP_COORD_MP_LOWPRECISION), m_vecPos)
END_SEND_TABLE()

BEGIN_DATADESC( CPlayerResource )

	// DEFINE_ARRAY( m_iPing, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_iPacketloss, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_iScore, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_iDeaths, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_bConnected, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_FIELD( m_flNextPingUpdate, FIELD_FLOAT ),
	// DEFINE_ARRAY( m_iTeam, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_bAlive, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_iHealth, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_FIELD( m_nUpdateCounter, FIELD_INTEGER ),

	// Function Pointers
	DEFINE_FUNCTION( ResourceThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( player_manager, CPlayerResource );

CPlayerResource *g_pPlayerResource;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerResource::Spawn( void )
{
	for ( int i=0; i < MAX_PLAYERS+1; i++ )
	{
		m_szSuit.Set( i, NULL_STRING );
		m_iPing.Set( i, 0 );
		m_iScore.Set( i, 0 );
		m_iDeaths.Set( i, 0 );
		m_bConnected.Set( i, 0 );
		m_iTeam.Set( i, 0 );
		m_bAlive.Set( i, 0 );
		m_iLevel.Set( i, 0 );
//		m_iTotalExp.Set( i, 0 );
		m_iGameExp.Set( i, 0 );
	}

	SetThink( &CPlayerResource::ResourceThink );
	SetNextThink( gpGlobals->curtime );
	m_nUpdateCounter = 0;
}

//-----------------------------------------------------------------------------
// Purpose: The Player resource is always transmitted to clients
//-----------------------------------------------------------------------------
int CPlayerResource::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: Wrapper for the virtual GrabPlayerData Think function
//-----------------------------------------------------------------------------
void CPlayerResource::ResourceThink( void )
{
	m_nUpdateCounter++;

	UpdatePlayerData();

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerResource::UpdatePlayerData( void )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
//		CBasePlayer *pPlayer = (CBasePlayer*)UTIL_PlayerByIndex( i );
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( i ) );
		
		if ( pPlayer && pPlayer->IsConnected() )
		{
			m_szSuit.Set( i, MAKE_STRING(pPlayer->GetCharacterName()) );
			m_iScore.Set( i, pPlayer->FragCount() );
			m_iDeaths.Set( i, pPlayer->DeathCount() );
			m_bConnected.Set( i, 1 );
			m_iTeam.Set( i, pPlayer->GetTeamNumber() );
			m_bAlive.Set( i, pPlayer->IsAlive()?1:0 );
			m_iHealth.Set(i, max( 0, pPlayer->GetHealth() ) );
			m_iLevel.Set(i, pPlayer->GetLevel() );
//			m_iTotalExp.Set(i, pPlayer->GetTotalExp() );
			m_iGameExp.Set(i, pPlayer->GetGameExp() );
			m_bInChar.Set(i, pPlayer->IsInCharacter() );
			m_iFaction.Set(i, pPlayer->GetFaction() );
			m_vecPos.Set(i, pPlayer->GetAbsOrigin() + Vector(0,0,ALLY_ICON_VERTICAL_OFFSET));

			// Don't update ping / packetloss everytime

			if ( !(m_nUpdateCounter%20) )
			{
				// update ping all 20 think ticks = (20*0.1=2seconds)
				int ping, packetloss;
				UTIL_GetPlayerConnectionInfo( i, ping, packetloss );
				
				// calc avg for scoreboard so it's not so jittery
				ping = 0.8f * m_iPing.Get(i) + 0.2f * ping;

				
				m_iPing.Set( i, ping );
				// m_iPacketloss.Set( i, packetloss );
			}
		}
		else
		{
			m_bConnected.Set( i, 0 );
		}
	}
}
