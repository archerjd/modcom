//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "grenade_spit.h"
#include "soundent.h"
#include "decals.h"
#include "smoke_trail.h"
#include "hl2_shareddefs.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "soundenvelope.h"
#include "ai_utils.h"
#include "te_effect_dispatch.h"
#include "hl2mp/hl2mp_gamerules.h"
#include "ai_basenpc.h"
#include "modcom/mcconvar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
ConVar	  sk_antlion_worker_spit_grenade_radius		  ( "sk_antlion_worker_spit_grenade_radius","40", FCVAR_NONE, "Radius of effect for an antlion worker spit grenade.");
extern McConVar sk_antlion_worker_spit_grenade_poison_ratio;
extern LEVEL_EXTERN(mod_poisonspit_damagescale_start);
extern LEVEL_EXTERN(mod_poisonspit_damagescale_end);
extern LEVEL_EXTERN(mod_poisonspit_damagescale_limit);

LINK_ENTITY_TO_CLASS( grenade_spit, CGrenadeSpit );

BEGIN_DATADESC( CGrenadeSpit )

	DEFINE_FIELD( m_bPlaySound, FIELD_BOOLEAN ),

	// Function pointers
	DEFINE_ENTITYFUNC( GrenadeSpitTouch ),

END_DATADESC()

CGrenadeSpit::CGrenadeSpit( void ) : m_bPlaySound( true ), m_pHissSound( NULL )
{
	m_bDamageIncreases = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeSpit::Spawn( void )
{
	Precache( );
	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	SetSolidFlags( FSOLID_NOT_STANDABLE );

	SetModel( "models/spitball_large.mdl" );
	UTIL_SetSize( this, vec3_origin, vec3_origin );

	SetUse( &CBaseGrenade::DetonateUse );
	SetTouch( &CGrenadeSpit::GrenadeSpitTouch );
	SetNextThink( gpGlobals->curtime + 0.1f );

	m_flDamage		= 10;
	m_DmgRadius		= sk_antlion_worker_spit_grenade_radius.GetFloat();
	m_takedamage	= DAMAGE_NO;
	m_iHealth		= 1;
	m_flStartTime	= gpGlobals->curtime;

	SetGravity( UTIL_ScaleForGravity( SPIT_GRAVITY ) );
	SetFriction( 0.8f );

	SetCollisionGroup( COLLISION_GROUP_PROJECTILE ); // HL2COLLISION_GROUP_SPIT
	
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	// We're self-illuminating, so we don't take or give shadows
	AddEffects( EF_NOSHADOW|EF_NORECEIVESHADOW );

	// Create the dust effect in place
	m_hSpitEffect = (CParticleSystem *) CreateEntityByName( "info_particle_system" );
	if ( m_hSpitEffect != NULL )
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue( "start_active", "1" );
		m_hSpitEffect->KeyValue( "effect_name", "antlion_spit_trail" );
		m_hSpitEffect->SetParent( this );
		m_hSpitEffect->SetLocalOrigin( vec3_origin );
		DispatchSpawn( m_hSpitEffect );
		if ( gpGlobals->curtime > 0.5f )
			m_hSpitEffect->Activate();
	}
}


void CGrenadeSpit::SetSpitSize( int nSize )
{
	switch (nSize)
	{
		case SPIT_LARGE:
		{
			m_bPlaySound = true;
			SetModel( "models/spitball_large.mdl" );
			SetSize( -Vector(4,4,4), Vector(4,4,4) );
			break;
		}
		case SPIT_MEDIUM:
		{
			m_bPlaySound = true;
			m_flDamage *= 0.5f;
			SetModel( "models/spitball_medium.mdl" );
			SetSize( -Vector(2,2,2), Vector(2,2,2) );
			break;
		}
		case SPIT_SMALL:
		{
			m_bPlaySound = false;
			m_flDamage *= 0.25f;
			SetModel( "models/spitball_small.mdl" );
			SetSize( -Vector(1,1,1), Vector(1,1,1) );
			break;
		}
	}
}

void CGrenadeSpit::Event_Killed( const CTakeDamageInfo &info )
{
	Detonate( );
}

//-----------------------------------------------------------------------------
// Purpose: Handle spitting
//-----------------------------------------------------------------------------
ConVar mc_dev_debug_spit("mc_dev_debug_spit", "0", FCVAR_NOTIFY);
void CGrenadeSpit::GrenadeSpitTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsSolid() )
	{
		if ( pOther->GetCollisionGroup() == COLLISION_GROUP_WEAPON )
		{
			if ( mc_dev_debug_spit.GetInt() )
				UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "Spit hit weapon 1, force detonating\n");
			DispatchParticleEffect("antlion_spit_impact",GetAbsOrigin(),QAngle(0,0,0));
			Detonate();
		}
		return;
	}

	if ( pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER | FSOLID_NOT_SOLID) )
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		//if ( ( pOther->m_takedamage == DAMAGE_NO ) || ( pOther->m_takedamage == DAMAGE_EVENTS_ONLY ) )

		if ( pOther->GetCollisionGroup() == COLLISION_GROUP_WEAPON )
		{
			if ( mc_dev_debug_spit.GetInt() )
				UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "Spit hit weapon 2, force detonating\n");
			DispatchParticleEffect("antlion_spit_impact",GetAbsOrigin(),QAngle(0,0,0));
			Detonate();
		}

		return;
	}

	// Don't hit other spit
	if ( pOther->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE ) // HL2COLLISION_GROUP_SPIT
		return;

	if ( mc_dev_debug_spit.GetInt() )
		UTIL_ClientPrintAll(HUD_PRINTCONSOLE, UTIL_VarArgs("Spit collided with %s\n", pOther ? pOther->GetClassname() : "NULL"));

	// We want to collide with water
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();

	// copy out some important things about this trace, because the first TakeDamage
	// call below may cause another trace that overwrites the one global pTrace points
	// at.
	bool bHitWater = ( ( pTrace->contents & CONTENTS_WATER ) != 0 );
	CBaseEntity *const pTraceEnt = pTrace->m_pEnt;
	const Vector tracePlaneNormal = pTrace->plane.normal;

	if ( bHitWater )
	{
		// Splash!
		CEffectData data;
		data.m_fFlags = 0;
		data.m_vOrigin = pTrace->endpos;
		data.m_vNormal = Vector( 0, 0, 1 );
		data.m_flScale = 8.0f;

		DispatchEffect( "watersplash", data );
	}
	else
	{
		// Make a splat decal
		trace_t *pNewTrace = const_cast<trace_t*>( pTrace );
		UTIL_DecalTrace( pNewTrace, "BeerSplash" );
	}

	// Part normal damage, part poison damage
	float poisonratio = sk_antlion_worker_spit_grenade_poison_ratio.GetFloat();

	// Take direct damage if hit
	// NOTE: assume that pTrace is invalidated from this line forward!
	if ( pTraceEnt )
	{
		CBaseCombatCharacter *pAttacker;
		if ( GetThrower() == NULL )
			pAttacker = NULL;
		else if ( GetThrower()->MyNPCPointer() != NULL )
			pAttacker = GetThrower()->MyNPCPointer()->MyAttacker();
		else
			pAttacker = GetThrower();
			
		float damage = m_flDamage;
		if ( m_bDamageIncreases )
		{
			float lifetime = gpGlobals->curtime - m_flStartTime;
			float scaleStart = LEVEL(mod_poisonspit_damagescale_start, GetLevel());
			float fraction = max(min((lifetime - scaleStart) / (LEVEL(mod_poisonspit_damagescale_end, GetLevel()) - scaleStart), 1.0f),0.0f);
			
			// lerp damage to fraction amount between damage & damage * mod_poisonspit_damagescale_limit
			//Msg(UTIL_VarArgs("Damage scaling fraction = %.2f, damage gone from %.0f to ", fraction, damage));
			//damage = Lerp(fraction, damage, damage * LEVEL(mod_poisonspit_damagescale_limit, GetLevel()));
			//Msg(UTIL_VarArgs("%.0f\n", damage));
		}
		pTraceEnt->TakeDamage( CTakeDamageInfo( this, pAttacker, damage * (1.0f-poisonratio), DMG_ACID ) );
		pTraceEnt->TakeDamage( CTakeDamageInfo( this, pAttacker, damage * poisonratio, DMG_POISON ) );
	}

	CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), m_DmgRadius * 2.0f, 0.5f, GetThrower() );

	QAngle vecAngles;
	VectorAngles( tracePlaneNormal, vecAngles );
/*	
	if ( pOther->IsPlayer() || bHitWater )
	{
		// Do a lighter-weight effect if we just hit a player
		DispatchParticleEffect( "antlion_spit_player", GetAbsOrigin(), vecAngles );
	}
	else
	{
		DispatchParticleEffect( "antlion_spit", GetAbsOrigin(), vecAngles );
	}
*/	DispatchParticleEffect("antlion_spit_impact",GetAbsOrigin(),vecAngles);
	Detonate();
}

void CGrenadeSpit::Detonate(void)
{
	m_takedamage = DAMAGE_NO;

	EmitSound( "GrenadeSpit.Hit" );	

	// Stop our hissing sound
	if ( m_pHissSound != NULL )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pHissSound );
		m_pHissSound = NULL;
	}

	if ( m_hSpitEffect )
	{
		UTIL_Remove( m_hSpitEffect );
	}

	UTIL_Remove( this );
}

void CGrenadeSpit::InitHissSound( void )
{
	if ( m_bPlaySound == false )
		return;

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	if ( m_pHissSound == NULL )
	{
		CPASAttenuationFilter filter( this );
		m_pHissSound = controller.SoundCreate( filter, entindex(), "NPC_Antlion.PoisonBall" );
		controller.Play( m_pHissSound, 1.0f, 100 );
	}
}

void CGrenadeSpit::Think( void )
{
	InitHissSound();
	if ( m_pHissSound == NULL )
		return;
	
	// Add a doppler effect to the balls as they travel
	CBaseEntity *pPlayer = AI_GetSinglePlayer();
	if ( pPlayer != NULL )
	{
		Vector dir;
		VectorSubtract( pPlayer->GetAbsOrigin(), GetAbsOrigin(), dir );
		VectorNormalize(dir);

		float velReceiver = DotProduct( pPlayer->GetAbsVelocity(), dir );
		float velTransmitter = -DotProduct( GetAbsVelocity(), dir );
		
		// speed of sound == 13049in/s
		int iPitch = 100 * ((1 - velReceiver / 13049) / (1 + velTransmitter / 13049));

		// clamp pitch shifts
		if ( iPitch > 250 )
		{
			iPitch = 250;
		}
		if ( iPitch < 50 )
		{
			iPitch = 50;
		}

		// Set the pitch we've calculated
		CSoundEnvelopeController::GetController().SoundChangePitch( m_pHissSound, iPitch, 0.1f );
	}

	// Set us up to think again shortly
	SetNextThink( gpGlobals->curtime + 0.05f );
}

void CGrenadeSpit::Precache( void )
{
	// m_nSquidSpitSprite = PrecacheModel("sprites/greenglow1.vmt");// client side spittle.

	PrecacheModel( "models/spitball_large.mdl" ); 
	PrecacheModel("models/spitball_medium.mdl"); 
	PrecacheModel("models/spitball_small.mdl"); 

	PrecacheScriptSound( "GrenadeSpit.Hit" );

//	PrecacheParticleSystem( "antlion_spit_player" );
//	PrecacheParticleSystem( "antlion_spit" );
	PrecacheParticleSystem( "antlion_spit_impact" );
}
