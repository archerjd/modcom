//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "c_AI_BaseNPC.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

wchar_t customStatus[32];
class C_NPC_FloorTurret : public C_AI_BaseNPC
{
public:
	C_NPC_FloorTurret() { m_iBullets = 0; }

	DECLARE_CLASS( C_NPC_FloorTurret, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();
 	DECLARE_DATADESC();

	virtual wchar_t *CustomStatusMessage()
	{
		//return VarArgs(", Ammo: %i", m_iBullets);
		_snwprintf(customStatus, sizeof(customStatus), L", Ammo: %i", m_iBullets);
		return customStatus;
	}

private:
	int m_iBullets;

	C_NPC_FloorTurret( const C_NPC_FloorTurret & );

};


//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
BEGIN_DATADESC( C_NPC_FloorTurret )

//	DEFINE_SOUNDPATCH( m_pEngineSound1 ),
//	DEFINE_SOUNDPATCH( m_pEngineSound2 ),
//	DEFINE_SOUNDPATCH( m_pBladeSound ),

//	DEFINE_FIELD( m_nEnginePitch1, FIELD_INTEGER ),
//	DEFINE_FIELD( m_nEnginePitch2, FIELD_INTEGER ),
//	DEFINE_FIELD( m_flEnginePitch1Time, FIELD_FLOAT ),
//	DEFINE_FIELD( m_flEnginePitch2Time, FIELD_FLOAT ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_DT(C_NPC_FloorTurret, DT_NPC_FloorTurret, CNPC_FloorTurret)
	RecvPropInt( RECVINFO( m_iBullets ) ),
END_RECV_TABLE()



