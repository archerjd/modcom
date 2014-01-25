#include "cbase.h"
#include "teleport_blocker.h"

#include "tier0/memdbgon.h"

#define THINK_INTERVAL 0.5f

LINK_ENTITY_TO_CLASS( info_teleport_node, CTeleportBlocker );

BEGIN_DATADESC( CTeleportBlocker )
	DEFINE_THINKFUNC( VisibleThink ),
END_DATADESC()

CTeleportBlocker::CTeleportBlocker()
{

}

void CTeleportBlocker::Spawn( void )
{
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);
	SetVisible(false);
}

void CTeleportBlocker::SetVisible(bool bVisible)
{
	if ( bVisible )
	{
		SetThink ( &CTeleportBlocker::VisibleThink );
		SetNextThink( gpGlobals->curtime );
	}
	else
		SetThink(NULL);
}

void CTeleportBlocker::VisibleThink ( void )
{
	if ( m_bHasRadius )
		NDebugOverlay::Sphere( GetAbsOrigin(), vec3_angle, m_flRadius, 255, 0, 0, 10, false, THINK_INTERVAL );

	//NDebugOverlay::Cross3D(GetAbsOrigin(), Vector(-4,-4,-4), Vector(4,4,4), 255, 160, 0, false, THINK_INTERVAL );
	NDebugOverlay::Circle(GetAbsOrigin(), 8, 255, 255, 255, 255, false, THINK_INTERVAL );
	NDebugOverlay::Circle(GetAbsOrigin(), 6, 255, 0, 0, 255, true, THINK_INTERVAL );

	SetNextThink( gpGlobals->curtime + THINK_INTERVAL );
}