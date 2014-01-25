//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_TRIPMINE_H
#define GRENADE_TRIPMINE_H
#ifdef _WIN32
#pragma once
#endif

#include "basegrenade_shared.h"

class CBeam;


class CTripmineGrenade : public CBaseGrenade
{
public:
	DECLARE_CLASS( CTripmineGrenade, CBaseGrenade );
#ifdef USE_OMNIBOT
	int GetOmnibotClass() const;
#endif
	CTripmineGrenade();
	void Spawn( void );
	void Precache( void );

	int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	void WarningThink( void );
	void PowerupThink( void );
	void FadeThink( void );
	void BeamBreakThink( void );
	void DelayDeathThink( void );
	void Event_Killed( const CTakeDamageInfo &info );

	void MakeBeam( void );
	void KillBeam( void );

	virtual int	ObjectCaps( void ) { return (BaseClass::ObjectCaps() | m_iCaps ); }
	
	void StartFadeOut();
	void SetModuleLaunched( void ) { m_bIsModuleLaser = true; }
	void SetLaserDamage(int dmg) { m_iLaserDamage = dmg; }
	void SetMaxLaserDamage(int max) { m_iLaserDamageRemaining = max; }
	void SetLifetime(float lifetime) { m_flFadeOutTime = gpGlobals->curtime + lifetime; }

public:
	EHANDLE		m_hOwner;

private:
	float		m_flPowerUp;
	Vector		m_vecDir;
	Vector		m_vecEnd;
	float		m_flBeamLength;

	CBeam		*m_pBeam;
	Vector		m_posOwner;
	Vector		m_angleOwner;
	int			m_iCaps;

	bool	m_bIsModuleLaser;
	int		m_iLaserDamage;
	int		m_iLaserDamageRemaining;
	float	m_flFadeOutTime;
	bool	m_bShownFadeWarning;

	DECLARE_DATADESC();
};


class CMagMine : public CBaseGrenade
{
public:
	DECLARE_CLASS( CMagMine, CBaseGrenade );
#ifdef USE_OMNIBOT
	int GetOmnibotClass() const;
#endif
public:
	CMagMine();
	void Spawn( void );
	void Precache( void );

	int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	void PowerupThink( void );
	void DelayDeathThink( void );
	void FadeThink( void );
	void Event_Killed( const CTakeDamageInfo &info );
	void StartFadeOut();

	virtual int	ObjectCaps( void ) { return (BaseClass::ObjectCaps() | m_iCaps ); }
	Class_T Classify( void ) { return CLASS_BULLSEYE; }
	virtual bool CanBeAnEnemyOf( CBaseEntity *pEnemy );

private:

	float		m_flPowerUp;
	int			m_iCaps;
	float		m_flFadeOutTime;

	DECLARE_DATADESC();
};

#endif // GRENADE_TRIPMINE_H
