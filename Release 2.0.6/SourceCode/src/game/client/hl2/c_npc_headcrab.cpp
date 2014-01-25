#include "cbase.h"
#include "c_AI_BaseNPC.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_NPC_Headcrab : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_NPC_Headcrab, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();
 	DECLARE_DATADESC();

	virtual bool CanBeBouncedOn() { return true; }

	C_NPC_Headcrab() {}
	C_NPC_Headcrab( const C_NPC_Headcrab & );
};

BEGIN_DATADESC( C_NPC_Headcrab )
END_DATADESC()

IMPLEMENT_CLIENTCLASS_DT(C_NPC_Headcrab, DT_NPC_Headcrab, CBaseHeadcrab)
END_RECV_TABLE()