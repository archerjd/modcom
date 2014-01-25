//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "grenade_frag.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "soundent.h"
#include "hl2mp_player.h"
#include "beam_flags.h"
#include "modcom/mcconvar.h"
#include "modcom/particle_effects.h"
#include "particle_parse.h"
#include "smoke_trail.h"

#ifdef USE_OMNIBOT
#include "../omnibot/omnibot_interface.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FRAG_GRENADE_BLIP_FREQUENCY			1.0f
#define FRAG_GRENADE_BLIP_FAST_FREQUENCY	0.3f

#define FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP 1.5f
#define FRAG_GRENADE_WARN_TIME 1.5f

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

ConVar sk_plr_dmg_fraggrenade	( "sk_plr_dmg_fraggrenade","0");
ConVar sk_npc_dmg_fraggrenade	( "sk_npc_dmg_fraggrenade","0");
ConVar sk_fraggrenade_radius	( "sk_fraggrenade_radius", "0");

#define GRENADE_MODEL "models/Weapons/w_grenade.mdl"
#define MIRVLET_MODEL "models/Weapons/ar2_grenade.mdl"
extern int s_nExplosionTexture;

class CGrenadeFrag : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeFrag, CBaseGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
	
public:
	CGrenadeFrag( void );
	~CGrenadeFrag( void );

	void	Spawn( void );
	void	OnRestore( void );
	void	Precache( void );
	bool	CreateVPhysics( void );
	void	CreateEffects( void );
	void	SetTimer( float detonateDelay, float warnDelay );
	void	SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	void	BlipSound() { EmitSound( "Grenade.Blip" ); }
	void	DelayThink();
	virtual void Detonate();
	void	DetonateTouch(CBaseEntity *pOther);
	void	VPhysicsUpdate( IPhysicsObject *pPhysics );
	void	OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	void	SetCombineSpawned( bool combineSpawned ) { m_combineSpawned = combineSpawned; }
	bool	IsCombineSpawned( void ) const { return m_combineSpawned; }
	void	SetPunted( bool punt ) { m_punted = punt; }
	bool	WasPunted( void ) const { return m_punted; }

	void	SetType( grenade_type type ) { m_type = type; }
	grenade_type GetType() { return m_type; }
	
	// this function only used in episodic.
#ifdef HL2_EPISODIC
	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
#endif 

	void	InputSetTimer( inputdata_t &inputdata );

#ifdef USE_OMNIBOT
	int GetOmnibotClass() const { return Omnibot::MC_CLASSEX_WEAPON + Omnibot::MC_WP_FRAG_GREN; }
#endif

protected:
	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;

	CBaseEntity *pHitEnt;

	float	m_flNextBlipTime;
	bool	m_inSolid;
	bool	m_combineSpawned;
	bool	m_punted;
	grenade_type m_type;
	int		m_iLevel; // only used by derivatives of this class (freeze, mirv)
};

LINK_ENTITY_TO_CLASS( npc_grenade_frag, CGrenadeFrag );

BEGIN_DATADESC( CGrenadeFrag )

	// Fields
	DEFINE_FIELD( m_pMainGlow, FIELD_EHANDLE ),
	DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( pHitEnt, FIELD_CLASSPTR ),
	DEFINE_FIELD( m_flNextBlipTime, FIELD_TIME ),
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_combineSpawned, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_punted, FIELD_BOOLEAN ),
	
	// Function Pointers
	DEFINE_THINKFUNC( DelayThink ),
	DEFINE_ENTITYFUNC( DetonateTouch ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTimer", InputSetTimer ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeFrag::CGrenadeFrag( void )
{
	m_type = GRENADE_FRAG;
}

CGrenadeFrag::~CGrenadeFrag( void )
{
}

void CGrenadeFrag::Spawn( void )
{
	Precache( );

	SetModel( GRENADE_MODEL );

	if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() )
	{
		m_flDamage		= sk_plr_dmg_fraggrenade.GetFloat();
		m_DmgRadius		= sk_fraggrenade_radius.GetFloat();
	}
	else
	{
		m_flDamage		= sk_npc_dmg_fraggrenade.GetFloat();
		m_DmgRadius		= sk_fraggrenade_radius.GetFloat();
	}

	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	if ( GetType() == GRENADE_FREEZE || GetType() == GRENADE_INCENDIARY || GetType() == GRENADE_MIRV )
	{
		SetCollisionGroup( COLLISION_GROUP_PROJECTILE ); // blow up on impact
		SetSolid(SOLID_BBOX);
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	}
	else
	{
		SetCollisionGroup( COLLISION_GROUP_WEAPON );
		CreateVPhysics();
	}
	
	BlipSound();
	m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;

	AddSolidFlags( FSOLID_NOT_STANDABLE );

	m_combineSpawned	= false;
	m_punted			= false;

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeFrag::OnRestore( void )
{
	// If we were primed and ready to detonate, put FX on us.
	if (m_flDetonateTime > 0)
		CreateEffects();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: Color the trail left by a thrown grenade
//-----------------------------------------------------------------------------
void CGrenadeFrag::CreateEffects( void )
{
	int r, g, b;
	switch ( GetType() )
	{
		case GRENADE_FREEZE:
		{
			r = 50; g = 50; b = 255; // Royal blue
			break;
		}
/*		case GRENADE_DISORIENTATE:
		{
			r = 0; g = 255; b = 0;
			break;
		}
*/		case GRENADE_MIRV:
		{
			r = 255; g = 255; b = 0;
			break;
		}
		case GRENADE_MIRVLET:
		{
			r = 96; g = 96; b = 0;
			break;
		}
		case GRENADE_INCENDIARY:
		{
			r = 255; g = 128; b = 0;
			break;
		}
		case GRENADE_FRAG:
		default:
		{
			r = 255; g = 0; b = 0;
			break;
		}
	}

	// Start up the eye glow
	m_pMainGlow = CSprite::SpriteCreate( "sprites/glow01.vmt", GetLocalOrigin(), false );

	int	nAttachment = LookupAttachment( "fuse" );

	if ( m_pMainGlow != NULL )
	{
		m_pMainGlow->FollowEntity( this );
		m_pMainGlow->SetAttachment( this, nAttachment );
		m_pMainGlow->SetTransparency( kRenderGlow, r, g, b, 200, kRenderFxNoDissipation );
		m_pMainGlow->SetScale( 0.2f );
		m_pMainGlow->SetGlowProxySize( 4.0f );
	}

	// Start up the eye trail
	m_pGlowTrail	= CSpriteTrail::SpriteTrailCreate( "sprites/laser2.vmt", GetLocalOrigin(), false );

	if ( m_pGlowTrail != NULL )
	{
		m_pGlowTrail->FollowEntity( this );
		m_pGlowTrail->SetAttachment( this, nAttachment );
		m_pGlowTrail->SetTransparency( kRenderTransAdd, r, g, b, 255, kRenderFxNone );
		m_pGlowTrail->SetStartWidth( 8.0f );
		m_pGlowTrail->SetEndWidth( 1.0f );
		m_pGlowTrail->SetLifeTime( 0.5f );
	}
}

void MakeMirvlet(Vector origin, Vector dir, CBaseEntity *thrower,int level)
{
	float time, speed;
	time = random->RandomFloat(0.40,0.45);
	speed = random->RandomInt(500,400);

	CBaseGrenade *pGrenade = Mirvlet_Create( origin + dir*2, vec3_angle, dir*speed, AngularImpulse(600,random->RandomInt(-1200,1200),0), thrower, time, level );
	if ( pGrenade )
		pGrenade->SetControllingModule(MIRV);
}


extern LEVEL_EXTERN(mod_freeze_grenade_radius);
extern LEVEL_EXTERN(mod_incendiary_grenade_radius);

void CGrenadeFrag::Detonate()
{
	if ( GetType() == GRENADE_FRAG || GetType() == GRENADE_INCENDIARY || GetType() == GRENADE_MIRV || GetType() == GRENADE_MIRVLET )
		BaseClass::Detonate();

	if ( GetType() == GRENADE_FRAG || GetType() == GRENADE_MIRVLET )
		return;

	if ( GetType () == GRENADE_MIRV )
	{
		// create 4 mirvlets
		QAngle ang1 = QAngle(0, random->RandomFloat(-180.0f, 180.0f), 0);
		QAngle ang2 = QAngle(0, ang1.y + 90, 0);
		Vector dir1, dir2;
		AngleVectors(ang1, &dir1); 
		AngleVectors(ang2, &dir2);
		MakeMirvlet(GetAbsOrigin(),dir1,GetThrower(),GetLevel());
		MakeMirvlet(GetAbsOrigin(),-dir1,GetThrower(),GetLevel());
		MakeMirvlet(GetAbsOrigin(),dir2,GetThrower(),GetLevel());
		MakeMirvlet(GetAbsOrigin(),-dir2,GetThrower(),GetLevel());
		return;
	}

	// freeze (or disorientation) if we got this far
	trace_t		tr;
	Vector		vecSpot;// trace starts here!

	SetThink( NULL );

	vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -32 ), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, & tr);

#if !defined( CLIENT_DLL )
	
	SetModelName( NULL_STRING );//invisible
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( tr.fraction != 1.0 )
	{
		SetAbsOrigin( tr.endpos + (tr.plane.normal * 0.6) );
	}
/*
	Vector vecAbsOrigin = GetAbsOrigin();
	int contents = UTIL_PointContents ( vecAbsOrigin );

	CPASFilter filter( vecAbsOrigin );
	te->Explosion( filter, -1.0, // don't apply cl_interp delay
		&vecAbsOrigin, 
		!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
		m_DmgRadius * .03, 
		25,
		TE_EXPLFLAG_NONE,
		m_DmgRadius,
		500 );
*/
	bool hurtCaster = true;
	bool doWave = false;

	int r=0, g=0, b=0, width=0, shockRadius=0, effectRadius=0, affliction=BUFF_NONE;
	float life=0.0f;
	int level = GetLevel();
	if ( GetType() == GRENADE_FREEZE )
	{
		doWave = true; // The wave effect created by grenade
		r = 50; g = 50;	b = 255; // Royal blue
		width = 100; effectRadius = LEVEL(mod_freeze_grenade_radius,GetLevel());
		shockRadius = effectRadius - 8;
		affliction = DEBUFF_FREEZE;
		life = 0.0952f + 0.0322f*GetLevel(); // 0.35f;
		hurtCaster = false;

		// do a little bit of damage too
		CTakeDamageInfo info(this,GetThrower(),1*GetLevel(),DMG_FREEZE);
		//info.SetModule(FREEZE_GRENADE);
		RadiusDamage(info,GetAbsOrigin(),effectRadius,CLASS_NONE, NULL);
		DispatchParticleEffect("freeze explode", GetAbsOrigin(), GetAbsAngles());

		//level = max(1,random->RandomInt(GetLevel()*0.6f,GetLevel())); // chance of lower level affliction being cast
		// from now on, the randomisation only happens for higher levels
		if ( level > 6 )
			level -= random->RandomInt(0,2);
		else if ( level > 3 )
			level -= random->RandomInt(0,1);
	}
	else if ( GetType() == GRENADE_INCENDIARY )
	{
		affliction = DEBUFF_BURN;
		effectRadius = LEVEL(mod_incendiary_grenade_radius, GetLevel());

		// we want an explosion, but not a normal one... this hopefully makes it look flamey
		DispatchParticleEffect("incendiary", GetAbsOrigin(), GetAbsAngles());
	}
/*	else if ( GetType() == GRENADE_DISORIENTATE )
	{
		r = 96; g = 255; b = 96;
		width = 10; shockRadius = 450; effectRadius = 500;
		affliction = BUFF_DISORIENTATE;
		life = 0.5f;
	}
*/
	if ( doWave )
	{
		//Shockring
		CBroadcastRecipientFilter filter2;
		te->BeamRingPoint( filter2, 0, GetAbsOrigin(),	//origin
			16,			//start radius
			shockRadius,//end radius
			s_nExplosionTexture, //texture
			0,			//halo index
			0,			//start frame
			3,			//framerate
			life,		//life
			width,		//width
			0,			//spread
			0,			//amplitude
			r,	//r
			g,	//g
			b,	//b
			220,	//a
			0,		//speed
			FBEAM_FADEOUT
			);
	}

	CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );

	// apply affliction to all players and npcs within line of sight within effectRadius
	CBaseEntity *pEntity = NULL;
	for ( CEntitySphereQuery sphere( GetAbsOrigin(), effectRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		if( !pEntity || ( pEntity == GetThrower() && hurtCaster == false ) )
			continue;

		if ( ( pEntity->IsNPC() || ( pEntity->IsPlayer() && ToHL2MPPlayer(pEntity)->IsInCharacter() && pEntity->IsAlive() ) ) && !HL2MPRules()->IsFriendly(GetThrower(),pEntity) )
		{
			trace_t tr; // MASK_NPCSOLID_BRUSHONLY means someone standing between me and the grenade wont stop me freezing
			UTIL_TraceLine(GetAbsOrigin(),pEntity->MyCombatCharacterPointer()->WorldSpaceCenter(),MASK_NPCSOLID_BRUSHONLY,this,COLLISION_GROUP_NONE,&tr);
			if ( tr.m_pEnt == pEntity || tr.fraction == 1.0f ) // line of sight
				pEntity->MyCombatCharacterPointer()->ApplyBuff(affliction,ToHL2MPPlayer(GetThrower()),level);
		}
	}

	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );
#endif

	if ( GetShakeAmplitude() )
	{
		UTIL_ScreenShake( GetAbsOrigin(), GetShakeAmplitude(), 150.0, 1.0, GetShakeRadius(), SHAKE_START );
	}
}

void CGrenadeFrag::DetonateTouch( CBaseEntity *pOther )
{
	Assert( pOther );
	if ( !pOther->IsSolid() )
		return;

	pHitEnt = pOther;
	Detonate();
}

bool CGrenadeFrag::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}

// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterCollisionGroupDelta );
	
	CTraceFilterCollisionGroupDelta( const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup )
		: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked( collisionGroupAlreadyChecked ), m_newCollisionGroup( newCollisionGroup )
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if ( pEntity )
		{
			if ( g_pGameRules->ShouldCollide( m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup() ) )
				return false;
			if ( g_pGameRules->ShouldCollide( m_newCollisionGroup, pEntity->GetCollisionGroup() ) )
				return true;
		}

		return false;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	int		m_collisionGroupAlreadyChecked;
	int		m_newCollisionGroup;
};

extern Vector CalcMagdForce(CBaseCombatCharacter *magd, IPhysicsObject *pPhysics, CBaseEntity *sucked, float interval);

void CGrenadeFrag::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );

	if ( GetType() == GRENADE_FREEZE || GetType() == GRENADE_INCENDIARY || GetType() == GRENADE_MIRV )
		return;

	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity( &vel, &angVel );
	
	Vector gravWellForce(0,0,0);

	for ( int i=0; i<MAX_MAGMINES; i++ )
	{
		CBaseGrenade *well = (CBaseGrenade*)HL2MPRules()->GetGravityWell(i);
		if ( !well )
			break; // we shunt them all up, so there's never any after a NULL
		
		//if (this->GetThrower() != well->GetThrower()) // magmines affect all grenades. If we limit it, it shouldn't affect FRIENDLY ones.
			gravWellForce += CalcMagdForce(well,pPhysics,this,gpGlobals->frametime);
	}

	vel += gravWellForce;

	Vector start = GetAbsOrigin();
	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGroupDelta filter( this, GetCollisionGroup(), COLLISION_GROUP_NONE );
	trace_t tr;

	// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
	UTIL_TraceHull( start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#else
	UTIL_TraceLine( start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#endif
	if ( tr.startsolid )
	{
		if ( !m_inSolid )
		{
			// UNDONE: Do a better contact solution that uses relative velocity?
			vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
			pPhysics->SetVelocity( &vel, NULL );
		}
		m_inSolid = true;
		return;
	}
	m_inSolid = false;
	if ( tr.DidHit() )
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info( this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH );
		tr.m_pEnt->TakeDamage( info );

		// reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;
		
		// absorb 80% in impact
		vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity( &vel, &angVel );
	}
	else
		pPhysics->SetVelocity( &vel, NULL );
}


void CGrenadeFrag::Precache( void )
{
	PrecacheModel( GRENADE_MODEL );
	PrecacheModel( MIRVLET_MODEL );

	PrecacheScriptSound( "Grenade.Blip" );

	PrecacheModel( "sprites/glow01.vmt" );
	PrecacheModel( "sprites/laser2.vmt" );

	PrecacheParticleSystem("incendiary");
	PrecacheParticleSystem("freeze explode");

	BaseClass::Precache();
}

void CGrenadeFrag::SetTimer( float detonateDelay, float warnDelay )
{
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink( &CGrenadeFrag::DelayThink );
	SetNextThink( gpGlobals->curtime );

	CreateEffects();
}

void CGrenadeFrag::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	SetThrower( pPhysGunUser );

#ifdef HL2MP
	SetTimer( FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP, FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP / 2);

	BlipSound();
	m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
	m_bHasWarnedAI = true;
#else
	if( IsX360() )
	{
		// Give 'em a couple of seconds to aim and throw. 
		SetTimer( 2.0f, 1.0f);
		BlipSound();
		m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
	}
#endif

#ifdef HL2_EPISODIC
	SetPunted( true );
#endif

	BaseClass::OnPhysGunPickup( pPhysGunUser, reason );
}

void CGrenadeFrag::DelayThink() 
{
	if( gpGlobals->curtime > m_flDetonateTime )
	{
		Detonate();
		return;
	}

	if( !m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime )
	{
#if !defined( CLIENT_DLL )
		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this );
#endif
		m_bHasWarnedAI = true;
	}
	
	if( gpGlobals->curtime > m_flNextBlipTime )
	{
		BlipSound();
		
		if( m_bHasWarnedAI )
		{
			m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
		}
		else
		{
			m_flNextBlipTime = gpGlobals->curtime + FRAG_GRENADE_BLIP_FREQUENCY;
		}
	}

	SetNextThink( gpGlobals->curtime + 0.1 );
}

void CGrenadeFrag::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

int CGrenadeFrag::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage( inputInfo );

	// Grenades only suffer blast damage and burn damage.
	if( !(inputInfo.GetDamageType() & (DMG_BLAST|DMG_BURN) ) )
		return 0;

	return BaseClass::OnTakeDamage( inputInfo );
}

#ifdef HL2_EPISODIC
extern int	g_interactionBarnacleVictimGrab; ///< usually declared in ai_interactions.h but no reason to haul all of that in here.
extern int g_interactionBarnacleVictimBite;
extern int g_interactionBarnacleVictimReleased;
bool CGrenadeFrag::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	// allow fragnades to be grabbed by barnacles. 
	if ( interactionType == g_interactionBarnacleVictimGrab )
	{
		// give the grenade another five seconds seconds so the player can have the satisfaction of blowing up the barnacle with it
		float timer = m_flDetonateTime - gpGlobals->curtime + 5.0f;
		SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );

		return true;
	}
	else if ( interactionType == g_interactionBarnacleVictimBite )
	{
		// detonate the grenade immediately 
		SetTimer( 0, 0 );
		return true;
	}
	else if ( interactionType == g_interactionBarnacleVictimReleased )
	{
		// take the five seconds back off the timer.
		float timer = max(m_flDetonateTime - gpGlobals->curtime - 5.0f,0.0f);
		SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
		return true;
	}
	else
	{
		return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
	}
}
#endif

void CGrenadeFrag::InputSetTimer( inputdata_t &inputdata )
{
	SetTimer( inputdata.value.Float(), inputdata.value.Float() - FRAG_GRENADE_WARN_TIME );
}

CBaseGrenade *Frag_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::CreateNoSpawn( "npc_grenade_frag", position, angles, pOwner );
	pGrenade->SetType(GRENADE_FRAG);
	DispatchSpawn(pGrenade);
	pGrenade->SetDamage(sk_plr_dmg_fraggrenade.GetFloat());
	pGrenade->SetDamageRadius(sk_fraggrenade_radius.GetFloat());

	pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned( combineSpawned );

	return pGrenade;
}

CBaseGrenade *Freeze_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int level, bool combineSpawned )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::CreateNoSpawn( "npc_grenade_frag", position, angles, pOwner );
	pGrenade->SetType(GRENADE_FREEZE);
	DispatchSpawn(pGrenade);
	pGrenade->SetDamage(1);
	pGrenade->SetDamageRadius(1);
	pGrenade->SetLevel(level);

	pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned( combineSpawned );
	pGrenade->SetTouch(&CGrenadeFrag::DetonateTouch);
	pGrenade->SetUse( &CGrenadeFrag::DetonateUse );

	return pGrenade;
}

extern LEVEL_EXTERN(mod_incendiary_grenade_damage);

CBaseGrenade *Incendiary_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int level, bool combineSpawned )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::CreateNoSpawn( "npc_grenade_frag", position, angles, pOwner );
	pGrenade->SetType(GRENADE_INCENDIARY);
	DispatchSpawn(pGrenade);
	pGrenade->SetLevel(level);
	pGrenade->SetDamage(LEVEL(mod_incendiary_grenade_damage, level));
	pGrenade->SetDamageRadius(96);
	pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned( combineSpawned );
	pGrenade->SetTouch(&CGrenadeFrag::DetonateTouch);
	pGrenade->SetUse( &CGrenadeFrag::DetonateUse );

	return pGrenade;
}

ConVar mod_mirvgrenade_damagebase("mod_mirvgrenade_damagebase", "25", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar mod_mirvgrenade_damagescale("mod_mirvgrenade_damagescale", "5", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar mod_mirvgrenade_radiusbase("mod_mirvgrenade_radiusbase", "100", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar mod_mirvgrenade_radiusscale("mod_mirvgrenadet_radiusscale", "5", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED );
CBaseGrenade *Mirv_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int level, bool combineSpawned )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::CreateNoSpawn( "npc_grenade_frag", position, angles, pOwner );
	pGrenade->SetType(GRENADE_MIRV);
	DispatchSpawn(pGrenade);
	pGrenade->SetDamage(mod_mirvgrenade_damagebase.GetInt()+mod_mirvgrenade_damagescale.GetInt()*level);
	pGrenade->SetDamageRadius(mod_mirvgrenade_radiusbase.GetInt()+mod_mirvgrenade_radiusscale.GetInt()*level);
	pGrenade->SetLevel(level);

	pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
	//pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned( combineSpawned );
	pGrenade->SetTouch(&CGrenadeFrag::DetonateTouch);
	pGrenade->SetUse( &CGrenadeFrag::DetonateUse );

	return pGrenade;
}

ConVar mod_mirvlet_damagebase("mod_mirvlet_damagebase", "15", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar mod_mirvlet_damagescale("mod_mirvlet_damagescale", "3", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar mod_mirvlet_radiusbase("mod_mirvlet_radiusbase", "100", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar mod_mirvlet_radiusscale("mod_mirvlet_radiusscale", "5", FCVAR_GAMEDLL | FCVAR_NOTIFY | FCVAR_REPLICATED );
CBaseGrenade *Mirvlet_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int level, bool combineSpawned )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::CreateNoSpawn( "npc_grenade_frag", position, angles, pOwner );
	pGrenade->SetType(GRENADE_MIRVLET);
	DispatchSpawn(pGrenade);
	pGrenade->SetModel(MIRVLET_MODEL);
	pGrenade->SetDamage(mod_mirvlet_damagebase.GetInt()+mod_mirvlet_damagescale.GetFloat()*level);
	pGrenade->SetDamageRadius(mod_mirvlet_radiusbase.GetInt()+mod_mirvlet_radiusscale.GetInt()*level);
	pGrenade->SetLevel(level);
	pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned( combineSpawned );

	return pGrenade;
}


/*
CBaseGrenade *Disorientate_Grenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int level, bool combineSpawned )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::CreateNoSpawn( "npc_grenade_frag", position, angles, pOwner );
	pGrenade->SetType(GRENADE_DISORIENTATE);
	DispatchSpawn(pGrenade);
	pGrenade->SetDamage(1);
	pGrenade->SetDamageRadius(1);
	pGrenade->SetLevel(level);

	pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned( combineSpawned );

	return pGrenade;
}
*/
bool Fraggrenade_WasPunted( const CBaseEntity *pEntity )
{
	const CGrenadeFrag *pFrag = dynamic_cast<const CGrenadeFrag *>( pEntity );
	if ( pFrag )
	{
		return pFrag->WasPunted();
	}

	return false;
}

bool Fraggrenade_WasCreatedByCombine( const CBaseEntity *pEntity )
{
	const CGrenadeFrag *pFrag = dynamic_cast<const CGrenadeFrag *>( pEntity );
	if ( pFrag )
	{
		return pFrag->IsCombineSpawned();
	}

	return false;
}