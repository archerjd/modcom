//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2MP_WEAPON_CAMERA_H
#define HL2MP_WEAPON_CAMERA_H
#pragma once


#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"


#ifdef CLIENT_DLL
#define CWeaponCamera C_WeaponCamera
#endif

//-----------------------------------------------------------------------------
// CWeaponCamera
//-----------------------------------------------------------------------------

class CWeaponCamera : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponCamera, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif

	CWeaponCamera();

	float		GetRange( void );
	float		GetFireRate( void );

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack( void )	{	return;	}

	void		Drop( const Vector &vecVelocity );

	virtual int GetPrimaryAttackDamage();
	virtual float GetPrimaryAttackRefire();
	virtual int GetSecondaryAttackDamage() { return 0; }
	virtual float GetSecondaryAttackRefire() { return 0; }
	virtual float GetReloadDuration() { return 0; }
	virtual int GetClipSize() { return -1; }
	
	// Animation event
#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	int WeaponMeleeAttack1Condition( float flDot, float flDist );
#ifdef USE_OMNIBOT
	int GetOmnibotClass() const;
#endif
#endif

	CWeaponCamera( const CWeaponCamera & );

private:
		
};


#endif // HL2MP_WEAPON_CAMERA_H

