//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements the tripmine grenade.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "beam_shared.h"
#include "shake.h"
#include "hl2mp/grenade_tripmine.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "explode.h"
#include "takedamageinfo.h"
#include "hl2mp_gamerules.h"
#include "ai_basenpc.h"
#include "hl2mp_player.h"
#include "particle_parse.h"
#include "modcom/mc_shareddefs.h"
#include "modcom/mcconvar.h"
#include "modcom/particle_effects.h"
#include "ai_senses.h"

#ifdef USE_OMNIBOT
#include "../omnibot/omnibot_interface.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern const char* g_pModelNameLaser;
extern const char *g_szEntityEffectNames[LAST_ENTITY_EFFECT];


ConVar    sk_plr_dmg_tripmine		( "sk_plr_dmg_tripmine","0");
ConVar    sk_npc_dmg_tripmine		( "sk_npc_dmg_tripmine","0");
ConVar    sk_tripmine_radius		( "sk_tripmine_radius","0");

#define	LASER_BRIGHTNESS_OFF	48
#define LASER_BRIGHTNESS_ON		172
#define LASER_SOUND_INTERVAL	0.5f

LINK_ENTITY_TO_CLASS( npc_tripmine, CTripmineGrenade );

BEGIN_DATADESC( CTripmineGrenade )

	DEFINE_FIELD( m_hOwner,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_flPowerUp,	FIELD_TIME ),
	DEFINE_FIELD( m_vecDir,		FIELD_VECTOR ),
	DEFINE_FIELD( m_vecEnd,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flBeamLength, FIELD_FLOAT ),
	DEFINE_FIELD( m_pBeam,		FIELD_CLASSPTR ),
	DEFINE_FIELD( m_posOwner,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_angleOwner,	FIELD_VECTOR ),

	// Function Pointers
	DEFINE_THINKFUNC( WarningThink ),
	DEFINE_THINKFUNC( PowerupThink ),
	DEFINE_THINKFUNC( FadeThink ),
	DEFINE_THINKFUNC( BeamBreakThink ),
	DEFINE_THINKFUNC( DelayDeathThink ),

END_DATADESC()

#ifdef USE_OMNIBOT
int CTripmineGrenade::GetOmnibotClass() const {
	return Omnibot::MC_CLASSEX_TRIPMINE;
}
#endif

CTripmineGrenade::CTripmineGrenade()
{
	m_vecDir.Init();
	m_vecEnd.Init();
	m_posOwner.Init();
	m_angleOwner.Init();
}

extern LEVEL_EXTERN(mod_lasers_lifetime);

void CTripmineGrenade::Spawn( void )
{
	Precache( );
	// motor
	SetMoveType( MOVETYPE_FLY );
	SetSolid( SOLID_BBOX );
	SetModel( "models/Weapons/w_slam.mdl" );

    IPhysicsObject *pObject = VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, true );
	pObject->EnableMotion( false );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );

	SetCycle( 0.0f );
	m_nBody			= 3;
	m_flDamage		= sk_plr_dmg_tripmine.GetFloat();
	m_DmgRadius		= sk_tripmine_radius.GetFloat();

	ResetSequenceInfo( );
	m_flPlaybackRate	= 0;
	
	UTIL_SetSize(this, Vector( -4, -4, -2), Vector(4, 4, 2));

	m_flPowerUp = gpGlobals->curtime + 2.0;
	
	SetThink( &CTripmineGrenade::PowerupThink );
	SetNextThink( gpGlobals->curtime + 0.2 );

	if ( m_bIsModuleLaser )
	{
		m_iCaps	= FCAP_IMPULSE_USE;
		m_bShownFadeWarning = false;
	}
	else
		m_iCaps = 0;

	m_takedamage		= DAMAGE_YES;

	m_iHealth = 1;

	EmitSound( "TripmineGrenade.Place" );
	SetDamage ( 200 );

	// Tripmine sits at 90 on wall so rotate back to get m_vecDir
	QAngle angles = GetAbsAngles();
	angles.x -= 90;

	AngleVectors( angles, &m_vecDir );
	m_vecEnd = GetAbsOrigin() + m_vecDir * 2048;

	AddEffects( EF_NOSHADOW );
}


void CTripmineGrenade::Precache( void )
{
	PrecacheModel("models/Weapons/w_slam.mdl"); 

	PrecacheScriptSound( "TripmineGrenade.Place" );
	PrecacheScriptSound( "TripmineGrenade.Activate" );
	PrecacheScriptSound( "TripmineGrenade.BeamOn" );
	
//	PrecacheParticleSystem("laser_shoot");
}

void CTripmineGrenade::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	if ( !m_bIsModuleLaser || !pActivator )
		return;

	if ( !pActivator->IsPlayer() || pActivator != m_hOwner )
		return;

//	CBasePlayer *pPlayer = dynamic_cast<CBasePlayer *>(pActivator);
	StartFadeOut();
}

void CTripmineGrenade::StartFadeOut()
{
	// i'm an ability laser and have been used by my owner, fade and die
	SetThink( &CTripmineGrenade::FadeThink );
	SetNextThink( gpGlobals->curtime + 0.25f );
	SetRenderMode(kRenderTransColor);
	SetRenderColorA(255);
	KillBeam();
}

void CTripmineGrenade::FadeThink()
{
	AddSolidFlags( FSOLID_NOT_SOLID );
	int a = GetRenderColor().a - 16;

	if ( a > 0 )
	{
		SetRenderColorA(a);
		SetNextThink( gpGlobals->curtime + 0.07f );
	}
	else
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( m_hOwner );
		if ( pPlayer )
			pPlayer->GetLimitedQuantities()->Remove(LQ_LASER);
		UTIL_Remove( this );
	}
}

void CTripmineGrenade::WarningThink( void  )
{
	// set to power up
	SetThink( &CTripmineGrenade::PowerupThink );
	SetNextThink( gpGlobals->curtime + 1.0f );
}


void CTripmineGrenade::PowerupThink( void  )
{
	if (gpGlobals->curtime > m_flPowerUp)
	{
		MakeBeam( );
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		m_bIsLive			= true;

		// play enabled sound
		EmitSound( "TripmineGrenade.Activate" );
	}
	
	SetNextThink( gpGlobals->curtime + 0.1f );
	if ( m_bIsModuleLaser )
	{
		if ( gpGlobals->curtime >= m_flFadeOutTime )
		{
			CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetThrower() );
			if ( pPlayer )
				ClientPrint( pPlayer, HUD_PRINTCENTER, UTIL_VarArgs("Your laser faded away.\n") );
			StartFadeOut();
		}
		else if ( !m_bShownFadeWarning && gpGlobals->curtime >= m_flFadeOutTime - 10.0f )
		{
			m_bShownFadeWarning = true;
			CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetThrower() );
			if ( pPlayer )
				ClientPrint( pPlayer, HUD_PRINTCENTER, UTIL_VarArgs("Your laser will fade away in 10s!\n") );
		}
	}
}


void CTripmineGrenade::KillBeam( void )
{
	if ( m_pBeam )
	{
		UTIL_Remove( m_pBeam );
		m_pBeam = NULL;
	}
}


void CTripmineGrenade::MakeBeam( void )
{
	trace_t tr;
	int mask = MASK_SHOT;
	if ( m_bIsModuleLaser )
		mask = CONTENTS_SOLID;

	UTIL_TraceLine( GetAbsOrigin(), m_vecEnd, mask, this, COLLISION_GROUP_NONE, &tr );

	m_flBeamLength = tr.fraction;



	// If I hit a living thing, send the beam through me so it turns on briefly
	// and then blows the living thing up
	CBaseEntity *pEntity = tr.m_pEnt;
	CBaseCombatCharacter *pBCC  = ToBaseCombatCharacter( pEntity );

	// Draw length is not the beam length if entity is in the way
	float drawLength = tr.fraction;
	if ( pBCC )
	{
		SetOwnerEntity( pBCC );
		UTIL_TraceLine( GetAbsOrigin(), m_vecEnd, mask, this, COLLISION_GROUP_NONE, &tr );
		m_flBeamLength = tr.fraction;
		SetOwnerEntity( NULL );

	}

	// set to follow laser spot
	SetThink( &CTripmineGrenade::BeamBreakThink );

	// Delay first think slightly so beam has time
	// to appear if person right in front of it
	SetNextThink( gpGlobals->curtime + 1.0f );

	Vector vecTmpEnd = GetLocalOrigin() + m_vecDir * 2048 * drawLength;

	m_pBeam = CBeam::BeamCreate( g_pModelNameLaser, 0.35 );
	m_pBeam->PointEntInit( vecTmpEnd, this );
	if ( m_bIsModuleLaser )
	{
		m_pBeam->SetColor( 55, 255, 52 );
		m_pBeam->SetBrightness(LASER_BRIGHTNESS_OFF);
	}
	else 
	{
		m_pBeam->SetColor( 255, 55, 52 );
		m_pBeam->SetBrightness( 64 );
	}
	m_pBeam->SetScrollRate( 25.6 );
	
	int beamAttach = LookupAttachment("beam_attach");
	m_pBeam->SetEndAttachment( beamAttach );
}


void CTripmineGrenade::BeamBreakThink( void  )
{
	if ( m_bIsModuleLaser )
		m_pBeam->SetBrightness(LASER_BRIGHTNESS_OFF); // turn it off when not broken

	// See if I can go solid yet (has dropper moved out of way?)
	if (IsSolidFlagSet( FSOLID_NOT_SOLID ))
	{
		trace_t tr;
		Vector	vUpBit = GetAbsOrigin();
		vUpBit.z += 5.0;

		UTIL_TraceEntity( this, GetAbsOrigin(), vUpBit, MASK_SHOT, &tr );
		if ( !tr.startsolid && (tr.fraction == 1.0) )
		{
			RemoveSolidFlags( FSOLID_NOT_SOLID );
		}
	}

	trace_t tr;

	// NOT MASK_SHOT because we want only simple hit boxes
	UTIL_TraceLine( GetAbsOrigin(), m_vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	// ALERT( at_console, "%f : %f\n", tr.flFraction, m_flBeamLength );

	// respawn detect. 
	if ( !m_pBeam )
	{
		MakeBeam( );
		if ( tr.m_pEnt )
			m_hOwner = tr.m_pEnt;	// reset owner too
	}

	CBaseEntity *pEntity = tr.m_pEnt;
	CBaseCombatCharacter *pBCC  = ToBaseCombatCharacter( pEntity );
	
	if ( pBCC || fabs( m_flBeamLength - tr.fraction ) > 0.001)
	{
		if ( m_bIsModuleLaser )
		{
			if ( pBCC && pBCC != m_hOwner && !HL2MPRules()->IsFriendly(m_hOwner,pBCC) ) // Do not hurt owner.
			{
				CTakeDamageInfo info( this, m_hOwner, m_iLaserDamage, DMG_ENERGYBEAM | DMG_ALWAYSGIB  );
				pBCC->TakeDamage(info);
				m_pBeam->SetBrightness(LASER_BRIGHTNESS_ON);
				if ( gpGlobals->curtime >= m_flPowerUp )
				{
					EmitSound( "TripmineGrenade.BeamOn" );
//					DispatchParticleEffect( "laser_shoot", GetAbsOrigin(), GetAbsAngles() );
					m_flPowerUp = gpGlobals->curtime + LASER_SOUND_INTERVAL;
				}

				m_iLaserDamageRemaining -= m_iLaserDamage;
				if ( m_iLaserDamageRemaining <= 0 )
				{
					// fade out or die with a bang?
					StartFadeOut();
					return;
				}
			}
		}
		else
		{
			m_iHealth = 0;
			Event_Killed( CTakeDamageInfo( (CBaseEntity*)m_hOwner, this, 100, GIB_NORMAL ) ); 
			return;
		}
	}

	SetNextThink( gpGlobals->curtime + 0.05f );
}

int CTripmineGrenade::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	CBaseCombatCharacter *pBCC = ToBaseCombatCharacter( info.GetAttacker() );

	if ( m_bIsModuleLaser && HL2MPRules()->IsFriendly(m_hOwner,pBCC) )
		return 0;

	if (!m_bIsLive && info.GetDamage() < m_iHealth)
	{
		// disable
		// Create( "weapon_tripmine", GetLocalOrigin() + m_vecDir * 24, GetAngles() );
		SetThink( &CTripmineGrenade::SUB_Remove );
		SetNextThink( gpGlobals->curtime + 0.1f );
		KillBeam();
		return FALSE;
	}
	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CTripmineGrenade::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage		= DAMAGE_NO;

	SetThink( &CTripmineGrenade::DelayDeathThink );
	SetNextThink( gpGlobals->curtime + 0.25 );

	EmitSound( "TripmineGrenade.StopSound" );
}


void CTripmineGrenade::DelayDeathThink( void )
{
	KillBeam();
	trace_t tr;

	if ( m_bIsModuleLaser )
	{
		ExplosionCreate( GetAbsOrigin() + m_vecDir * 8, GetAbsAngles(), m_hOwner, m_iLaserDamage * 10, 45,
			SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this );
		
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( m_hOwner );
		if ( pPlayer )
		{
			pPlayer->GetLimitedQuantities()->Remove(LQ_LASER);
			ClientPrint( pPlayer, HUD_PRINTNOTIFY, UTIL_VarArgs("Your laser was destroyed (%i remaining).\n", pPlayer->GetLimitedQuantities()->GetCount(LQ_LASER) ) );
		}
	}
	else
	{
		UTIL_TraceLine ( GetAbsOrigin() + m_vecDir * 8, GetAbsOrigin() - m_vecDir * 64,  MASK_SOLID, this, COLLISION_GROUP_NONE, & tr);
		UTIL_ScreenShake( GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START );

		ExplosionCreate( GetAbsOrigin() + m_vecDir * 8, GetAbsAngles(), m_hOwner, GetDamage(), 200, 
			SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this);
	}
	UTIL_Remove( this );
}


// ----------------------------------------------------

LINK_ENTITY_TO_CLASS( grenade_magmine, CMagMine );

BEGIN_DATADESC( CMagMine )
END_DATADESC()

CMagMine::CMagMine()
{
//	m_posOwner.Init();
//	m_angleOwner.Init();
}

#ifdef USE_OMNIBOT
int CMagMine::GetOmnibotClass() const {
	return Omnibot::MC_CLASSEX_MAGMINE;
}
#endif

extern LEVEL_EXTERN(mod_magmine_damage_radius);
extern LEVEL_EXTERN(mod_magmine_damage);
extern LEVEL_EXTERN(mod_magmine_health);
void CMagMine::Spawn( void )
{
	Precache( );
	// motor
	SetMoveType( MOVETYPE_FLY );
	SetSolid( SOLID_BBOX );
	SetModel( "models/props_combine/combine_mine01.mdl" );

    IPhysicsObject *pObject = VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, true );
	pObject->EnableMotion( false );
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	SetCycle( 0.0f );
	m_flDamage		= LEVEL(mod_magmine_damage, GetLevel());
	m_DmgRadius		= LEVEL(mod_magmine_damage_radius, GetLevel());

	ResetSequenceInfo( );
	m_flPlaybackRate	= 0;

	// make sure that NPCs can see me
	g_AI_SensedObjectsManager.AddEntity( this );
	
	UTIL_SetSize(this, Vector( -4, -4, -2), Vector(4, 4, 2));
	m_bIsLive			= false;
	
	SetThink( &CMagMine::PowerupThink );
	SetNextThink( gpGlobals->curtime + 5 );
	m_flFadeOutTime = gpGlobals->curtime + 120.0f;

	m_iCaps	= FCAP_IMPULSE_USE;

	m_takedamage		= DAMAGE_YES;

	m_iHealth = (int)LEVEL(mod_magmine_health, GetLevel());
	m_iMaxHealth = m_iHealth;

	EmitSound( "NPC_CombineMine.CloseHooks" );

	// MAGD sits at 90 on wall so rotate back to get m_vecDir
	QAngle angles = GetAbsAngles();
	angles.x -= 90;

	AddEffects( EF_NOSHADOW );
}


void CMagMine::Precache( void )
{
	PrecacheModel("models/props_combine/combine_mine01.mdl"); 

	PrecacheScriptSound( "NPC_CombineMine.CloseHooks" );
	PrecacheScriptSound( "Module.MagdLoop" );

	PrecacheParticleSystem( g_szEntityEffectNames[MAGD_EFFECT] );
}

void CMagMine::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	if ( !pActivator || !pActivator->IsPlayer() || pActivator != GetThrower() )
		return;

	StartFadeOut();
}

void CMagMine::StartFadeOut()
{
	// i'm a MAGD and have been used by my owner, fade and die
	SetThink( &CMagMine::FadeThink );
	SetNextThink( gpGlobals->curtime + 0.25f );
	SetRenderMode(kRenderTransColor);
	SetRenderColorA(255);

	m_iCaps = 0; // don't let me be used again as I fade out
}

void CMagMine::FadeThink()
{
	if ( !IsSolidFlagSet( FSOLID_NOT_SOLID ) )
	{
		AddSolidFlags( FSOLID_NOT_SOLID );
		HL2MPRules()->RemoveGravityWell(this);
		StopSound("Module.MagdLoop");
		StopParticleEffects(this);
	}
	int a = GetRenderColor().a - 16;

	if ( a > 0 )
	{
		SetRenderColorA(a);
		SetNextThink( gpGlobals->curtime + 0.07f );
	}
	else
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetThrower() );
		if ( pPlayer )
			pPlayer->GetLimitedQuantities()->Remove(LQ_MAGMINE);
		UTIL_Remove( this );
	}
}

int CMagMine::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// don't take friendly damage
	CBaseCombatCharacter *pBCC  = ToBaseCombatCharacter( info.GetAttacker() );
	if ( HL2MPRules()->IsFriendly(GetThrower(),pBCC) && pBCC != this ) // damaging self is allowed
		return 0;

	return BaseClass::OnTakeDamage_Alive(info);
}

void CMagMine::Event_Killed( const CTakeDamageInfo &info )
{
	m_takedamage		= DAMAGE_NO;

	if ( HL2MPRules() )
		HL2MPRules()->RemoveGravityWell(this);
	StopSound("Module.MagdLoop");
	StopParticleEffects(this);

	SetThink( &CMagMine::DelayDeathThink );
	SetNextThink( gpGlobals->curtime + 0.25 );
}

extern LEVEL_EXTERN(mod_magmine_fullradius);
#define MAGMINE_THINK_INTERVAL	5.33f
#define DANGERSOUND_WITHIN_MAX_RADIUS_DIST	64.0f
void CMagMine::PowerupThink( void )
{
	if (!m_bIsLive)
	{
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		m_bIsLive = true;
		
		// play enabled sound
		EmitSound( "Module.MagdLoop" );
		HL2MPRules()->AddGravityWell(this);
		m_iParticleEffect = MAGD_EFFECT;
	}
	
	SetNextThink( gpGlobals->curtime + MAGMINE_THINK_INTERVAL );
	if ( gpGlobals->curtime >= m_flFadeOutTime )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetThrower() );
		if ( pPlayer )
			ClientPrint( pPlayer, HUD_PRINTCENTER, UTIL_VarArgs("Your magmine faded away.\n") );
		StartFadeOut();
	}
	else
    {
        int radius = LEVEL(mod_magmine_fullradius, GetLevel()) - DANGERSOUND_WITHIN_MAX_RADIUS_DIST;
        CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin() + Vector(0,0,24), radius, MAGMINE_THINK_INTERVAL, GetThrower() );
    }
}

void CMagMine::DelayDeathThink( void )
{
	trace_t tr;

	ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), GetThrower(), m_flDamage, m_DmgRadius,
		SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this );
	
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetThrower() );
	if ( pPlayer )
	{
		if ( pPlayer->GetLimitedQuantities()->GetCount(LQ_MAGMINE) > 0 )
		{// if they manually detonated it, don't show this message
			pPlayer->GetLimitedQuantities()->Remove(LQ_MAGMINE);
			ClientPrint( pPlayer, HUD_PRINTCENTER, UTIL_VarArgs("Your magmine was destroyed.\n") );
		}
	}
	UTIL_Remove( this );
}

bool CMagMine::CanBeAnEnemyOf( CBaseEntity *pEnemy )
{
	if ( !pEnemy->IsPlayer() && !pEnemy->IsNPC() && !pEnemy->IsGrenade() )
		return false;
	
	CBaseCombatCharacter *pBCC = pEnemy->MyCombatCharacterPointer();
	if ( HL2MPRules()->IsFriendly(pBCC,this) )
		return false; // only if its not friendly can we target it
/*
	float distSq = ( GetAbsOrigin().AsVector2D() - pEnemy->GetAbsOrigin().AsVector2D() ).LengthSqr();
	if ( distSq > Square( 160 ) ) // only ever target this if we're really close (stuck in it)
		return false;
*/
	// return BaseClass::CanBeAnEnemyOf( pEnemy );
	return true;
}