//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_FRAG_H
#define GRENADE_FRAG_H
#pragma once

class CBaseGrenade;
struct edict_t;

enum grenade_type
{
	GRENADE_FRAG,
	GRENADE_FREEZE,
	GRENADE_INCENDIARY,
//	GRENADE_DISORIENTATE,
	GRENADE_MIRV,
	GRENADE_MIRVLET,
};

CBaseGrenade *Frag_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned );
CBaseGrenade *Freeze_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int level, bool combineSpawned=false);
CBaseGrenade *Mirv_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int level, bool combineSpawned=false );
CBaseGrenade *Mirvlet_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int level, bool combineSpawned=false );
CBaseGrenade *Incendiary_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int level, bool combineSpawned=false);
//CBaseGrenade *Disorientate_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned=false);
bool	Fraggrenade_WasPunted( const CBaseEntity *pEntity );
bool	Fraggrenade_WasCreatedByCombine( const CBaseEntity *pEntity );

#endif // GRENADE_FRAG_H
