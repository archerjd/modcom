//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Expose an IsAHunter function
//
//=============================================================================//

#ifndef NPC_HUNTER_H
#define NPC_HUNTER_H

#if defined( _WIN32 )
#pragma once
#endif

class CBaseEntity;

/// true if given entity pointer is a hunter.
bool Hunter_IsHunter(CBaseEntity *pEnt);

// call throughs for member functions

void Hunter_StriderBusterAttached( CBaseEntity *pHunter, CBaseEntity *pAttached );
void Hunter_StriderBusterDetached( CBaseEntity *pHunter, CBaseEntity *pAttached );
void Hunter_StriderBusterLaunched( CBaseEntity *pBuster );


class CHunterFlechette : public CBaseCombatCharacter, public IParentPropInteraction
{
	DECLARE_CLASS( CHunterFlechette, CBaseCombatCharacter );

public:

	CHunterFlechette();
	~CHunterFlechette();

	Class_T Classify() { return CLASS_NONE; }
	
	bool IsPlayerOwned() { return m_bPlayerOwned; }
	void SetPlayerOwned(bool b) { m_bPlayerOwned = b; }

public:

	void Spawn();
	void Activate();
	void Precache();
	void Shoot( Vector &vecVelocity, bool bBright );
	void SetSeekTarget( CBaseEntity *pTargetEntity );
	void Explode();

	bool CreateVPhysics();

	unsigned int PhysicsSolidMaskForEntity() const;
	static CHunterFlechette *FlechetteCreate( const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner = NULL );

	// IParentPropInteraction
	void OnParentCollisionInteraction( parentCollisionInteraction_t eType, int index, gamevcollisionevent_t *pEvent );
	void OnParentPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason );

	void SetDamage(int blastDamage, int hitDamage) { m_iBlastDamage = blastDamage; m_iHitDamage = hitDamage; }

protected:

	void SetupGlobalModelData();

	void StickTo( CBaseEntity *pOther, trace_t &tr );

	void BubbleThink();
	void DangerSoundThink();
	void ExplodeThink();
	void DopplerThink();
	void SeekThink();

	bool CreateSprites( bool bBright );

	void FlechetteTouch( CBaseEntity *pOther );

	Vector m_vecShootPosition;
	EHANDLE m_hSeekTarget;
	bool m_bPlayerOwned;
	int m_iBlastDamage, m_iHitDamage;

	DECLARE_DATADESC();
	//DECLARE_SERVERCLASS();
};

#endif
