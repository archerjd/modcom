#include "cbase.h"
#include "particles_simple.h"
#include "iefx.h"
#include "dlight.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_te_effect_dispatch.h"

#include "c_ai_basenpc.h"


class C_NPC_Hunter : public C_AI_BaseNPC
{
	DECLARE_CLASS( C_NPC_Hunter, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

public:
	virtual void	ReceiveMessage( int classID, bf_read &msg );
};

IMPLEMENT_CLIENTCLASS_DT( C_NPC_Hunter, DT_NPC_Hunter, CNPC_Hunter )
	
END_RECV_TABLE()

#define HUNTERFX_MUZZLEFLASH 0

void C_NPC_Hunter::ReceiveMessage( int classID, bf_read &msg )
{
	// Is the message for a sub-class?
	if ( classID != GetClientClass()->m_ClassID )
	{
		BaseClass::ReceiveMessage( classID, msg );
		return;
	}
	
	int messageType = msg.ReadByte();
	switch( messageType )
	{
	case HUNTERFX_MUZZLEFLASH:
		{
			// Find our attachment point
			unsigned char nAttachment = msg.ReadByte();
			
			// Get our attachment position
			Vector vecStart;
			QAngle vecAngles;
			GetAttachment( nAttachment, vecStart, vecAngles );

			//DispatchParticleEffect( "hunter_muzzle_flash", PATTACH_POINT_FOLLOW, this, nAttachment );
			DispatchParticleEffect( "hunter_muzzle_flash", vecStart, vecStart, vecAngles, NULL );

			// Dispatch the elight	
			CEffectData data;
			data.m_nAttachmentIndex = nAttachment;
			//data.m_nEntIndex = entindex();
			DispatchEffect( "HunterMuzzleFlash", data );
		}
		break;

	default:
		AssertMsg1( false, "Received unknown message %d", messageType);
	}
}