//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Crows. Simple ambient birds that fly away when they hear gunfire or
//			when anything gets too close to them.
//
// TODO: landing
// TODO: death
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "game.h"
#include "ai_basenpc.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_motor.h"
#include "ai_navigator.h"
#include "hl2_shareddefs.h"
#include "ai_route.h"
#include "npcevent.h"
#include "gib.h"
#include "ai_interactions.h"
#include "ndebugoverlay.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"
#include "npc_crow.h"
#include "ai_moveprobe.h"
#include "explode.h"
#include "smoke_trail.h"
#include "hl2mp_player.h"
#include "ai_memory.h"
#include "ai_senses.h"
#include "modcom/mcconvar.h"
#include "modcom/mc_shareddefs.h"

#ifdef USE_OMNIBOT
#include "../omnibot/omnibot_interface.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//
// Custom activities.
//
static int ACT_CROW_TAKEOFF;
static int ACT_CROW_SOAR;
static int ACT_CROW_LAND;

//
// Animation events.
//
static int AE_CROW_TAKEOFF;
static int AE_CROW_FLY;
static int AE_CROW_HOP;

//
// Skill settings.
//
ConVar sk_crow_health( "sk_crow_health","1");
ConVar sk_crow_melee_dmg( "sk_crow_melee_dmg","0");

extern LEVEL_EXTERN(mod_crow_airspeed);
extern LEVEL_EXTERN(mod_crow_explode_damage);
extern LEVEL_EXTERN(mod_crow_takeoff_speed);
extern LEVEL_EXTERN(mod_crow_explode_radius);

#define CROW_AIRSPEED LEVEL(mod_crow_airspeed, GetLevel())
#define CROW_TAKEOFF_SPEED LEVEL(mod_crow_explode_radius, GetLevel())
#define CROW_EXPLODE_DAMAGE LEVEL(mod_crow_explode_damage, GetLevel())
#define CROW_EXPLODE_RADIUS LEVEL(mod_crow_explode_radius, GetLevel())

LINK_ENTITY_TO_CLASS( npc_crow, CNPC_Crow );
LINK_ENTITY_TO_CLASS( npc_seagull, CNPC_Seagull );
LINK_ENTITY_TO_CLASS( npc_pigeon, CNPC_Pigeon );

BEGIN_DATADESC( CNPC_Crow )

	DEFINE_FIELD( m_flGroundIdleMoveTime, FIELD_TIME ),
	DEFINE_FIELD( m_bOnJeep, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nMorale, FIELD_INTEGER ),
	DEFINE_FIELD( m_bReachedMoveGoal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flHopStartZ, FIELD_FLOAT ),
	DEFINE_FIELD( m_vDesiredTarget, FIELD_VECTOR ),
	DEFINE_FIELD( m_vCurrentTarget, FIELD_VECTOR ),
	DEFINE_FIELD( m_flSoarTime, FIELD_TIME ),
	DEFINE_FIELD( m_bSoar, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bPlayedLoopingSound, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iBirdType, FIELD_INTEGER ),
	DEFINE_FIELD( m_vLastStoredOrigin, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_flLastStuckCheck, FIELD_TIME ),
	DEFINE_FIELD( m_flDangerSoundTime, FIELD_TIME ),
	DEFINE_KEYFIELD( m_bIsDeaf, FIELD_BOOLEAN, "deaf" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_STRING, "FlyAway", InputFlyAway ),

END_DATADESC()

static ConVar birds_debug( "birds_debug", "0" );

#ifdef USE_OMNIBOT
int CNPC_Crow::GetOmnibotClass() const {
	return Omnibot::MC_CLASSEX_CROW;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Crow::Spawn( void )
{
	BaseClass::Spawn();

	// Always fade the corpse
	AddSpawnFlags( SF_NPC_FADE_CORPSE );

	char *szModel = (char *)STRING( GetModelName() );
	if (!szModel || !*szModel)
	{
		szModel = "models/crow.mdl";
		SetModelName( AllocPooledString(szModel) );
	}

	Precache();
	SetModel( szModel );

	if ( GetLevel() < 1 )
		m_iHealth = 1;
	else
		m_iHealth = -1 + 2 * GetLevel(); // 1, 3, 5...

	SetHullType(HULL_TINY);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_STEP );

	m_flFieldOfView = VIEW_FIELD_FULL;
	SetViewOffset( Vector(6, 0, 11) );		// Position of the eyes relative to NPC's origin.

	m_flGroundIdleMoveTime = gpGlobals->curtime + random->RandomFloat( 0.0f, 5.0f );

	SetBloodColor( BLOOD_COLOR_RED );
	m_NPCState = NPC_STATE_NONE;

	m_nMorale = random->RandomInt( 0, 12 );
	
	SetCollisionGroup( HL2COLLISION_GROUP_CROW );

	CapabilitiesClear();

//	bool bFlying = ( ( m_spawnflags & SF_CROW_FLYING ) != 0 );
//	SetFlyingState( bFlying ? FlyState_Flying : FlyState_Walking );
	SetFlyingState( FlyState_Flying );

	// We don't mind zombies so much. They smell good!
//	AddClassRelationship( CLASS_ZOMBIE, D_NU, 0 );

	m_bSoar = false;
	m_bOnJeep = false;
	m_flSoarTime = gpGlobals->curtime;

	NPCInit();

	m_iBirdType = BIRDTYPE_CROW;

	m_vLastStoredOrigin = vec3_origin;
	m_flLastStuckCheck = gpGlobals->curtime;

	m_flDangerSoundTime = gpGlobals->curtime;
	m_flLastRecalcEnemyTime = gpGlobals->curtime;
}


//-----------------------------------------------------------------------------
// Purpose: Returns this monster's classification in the relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Crow::Classify( void )
{
	return( CLASS_EARTH_FAUNA ); 
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : posSrc - 
// Output : Vector
//-----------------------------------------------------------------------------
Vector CNPC_Crow::BodyTarget( const Vector &posSrc, bool bNoisy ) 
{ 
	Vector vecResult;
	vecResult = GetAbsOrigin();
	vecResult.z += 6;
	return vecResult;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Crow::StopLoopingSounds( void )
{
	//
	// Stop whatever flap sound might be playing.
	//
	if ( m_bPlayedLoopingSound )
	{
		StopSound( "NPC_Crow.Flap" );
	}
	BaseClass::StopLoopingSounds();
}


//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : pEvent - 
//-----------------------------------------------------------------------------
void CNPC_Crow::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_CROW_TAKEOFF )
	{
		if ( GetNavigator()->GetPath()->GetCurWaypoint() )
		{
			Takeoff( GetNavigator()->GetCurWaypointPos() );
		}
		return;
	}

	if( pEvent->event == AE_CROW_HOP )
	{
		SetGroundEntity( NULL );

		//
		// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
		//
		UTIL_SetOrigin( this, GetLocalOrigin() + Vector( 0 , 0 , 1 ));

		//
		// How fast does the crow need to travel to reach the hop goal given gravity?
		//
		float flHopDistance = ( m_vSavePosition - GetLocalOrigin() ).Length();
		float gravity = sv_gravity.GetFloat();
		if ( gravity <= 1 )
		{
			gravity = 1;
		}

		float height = 0.25 * flHopDistance;
		float speed = sqrt( 2 * gravity * height );
		float time = speed / gravity;

		//
		// Scale the sideways velocity to get there at the right time
		//
		Vector vecJumpDir = m_vSavePosition - GetLocalOrigin();
		vecJumpDir = vecJumpDir / time;

		//
		// Speed to offset gravity at the desired height.
		//
		vecJumpDir.z = speed;

		//
		// Don't jump too far/fast.
		//
		float distance = vecJumpDir.Length();
		if ( distance > 650 )
		{
			vecJumpDir = vecJumpDir * ( 650.0 / distance );
		}

		m_nMorale -= random->RandomInt( 1, 6 );
		if ( m_nMorale <= 0 )
		{
			m_nMorale = 0;
		}

		// Play a hop flap sound.
		EmitSound( "NPC_Crow.Hop" );

		SetAbsVelocity( vecJumpDir );
		return;
	}

	if( pEvent->event == AE_CROW_FLY )
	{
		//
		// Start flying.
		//
		SetActivity( ACT_FLY );

		m_bSoar = false;
		m_flSoarTime = gpGlobals->curtime + random->RandomFloat( 3, 5 );

		return;
	}

	CAI_BaseNPC::HandleAnimEvent( pEvent );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : eNewActivity - 
//-----------------------------------------------------------------------------
void CNPC_Crow::OnChangeActivity( Activity eNewActivity )
{
//	if ( eNewActivity == ACT_FLY )
//	{
//		m_flGroundSpeed = CROW_AIRSPEED;
//	}
//
	bool fRandomize = false;
	if ( eNewActivity == ACT_FLY )
	{
		fRandomize = true;
	}

	BaseClass::OnChangeActivity( eNewActivity );
	if ( fRandomize )
	{
		SetCycle( random->RandomFloat( 0.0, 0.75 ) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Input handler that makes the crow fly away.
//-----------------------------------------------------------------------------
void CNPC_Crow::InputFlyAway( inputdata_t &inputdata )
{
	string_t sTarget = MAKE_STRING( inputdata.value.String() );

	if ( sTarget != NULL_STRING )// this npc has a target
	{
		CBaseEntity *pEnt = gEntList.FindEntityByName( NULL, sTarget );

		if ( pEnt )
		{
			trace_t tr;
			AI_TraceLine ( EyePosition(), pEnt->GetAbsOrigin(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr );

			if ( tr.fraction != 1.0f )
				 return;

			// Find the npc's initial target entity, stash it
			SetGoalEnt( pEnt );
		}
	}
	else
		SetGoalEnt( NULL );

	SetCondition( COND_CROW_FORCED_FLY );
	SetCondition( COND_PROVOKED );

}

void CNPC_Crow::UpdateEfficiency( bool bInPVS )	
{
	if ( IsFlying() )
	{
		SetEfficiency( ( GetSleepState() != AISS_AWAKE ) ? AIE_DORMANT : AIE_NORMAL ); 
		SetMoveEfficiency( AIME_NORMAL ); 
		return;
	}

	BaseClass::UpdateEfficiency( bInPVS );
}

//-----------------------------------------------------------------------------
// Purpose: Implements "deafness"
//-----------------------------------------------------------------------------
bool CNPC_Crow::QueryHearSound( CSound *pSound )
{
	if( IsDeaf() )
		return false;

	return BaseClass::QueryHearSound( pSound );
}

//-----------------------------------------------------------------------------
// Purpose: Handles all flight movement because we don't ever build paths when
//			when we are flying.
// Input  : flInterval - Seconds to simulate.
//-----------------------------------------------------------------------------
bool CNPC_Crow::OverrideMove( float flInterval )
{
	if ( GetNavigator()->GetPath()->CurWaypointNavType() == NAV_FLY && GetNavigator()->GetNavType() != NAV_FLY )
	{
		SetNavType( NAV_FLY );
	}

	if ( IsFlying() )
	{
		if ( GetNavigator()->GetPath()->GetCurWaypoint() )
		{
			if ( m_flLastStuckCheck <= gpGlobals->curtime )
			{
				if ( m_vLastStoredOrigin == GetAbsOrigin() )
				{
					if ( GetAbsVelocity() == vec3_origin )
					{
						float flDamage = m_iHealth;
						
						CTakeDamageInfo	dmgInfo( this, this, flDamage, DMG_GENERIC );
						GuessDamageForce( &dmgInfo, vec3_origin - Vector( 0, 0, 0.1 ), GetAbsOrigin() );
						TakeDamage( dmgInfo );

						return false;
					}
					else
					{
						m_vLastStoredOrigin = GetAbsOrigin();
					}
				}
				else
				{
					m_vLastStoredOrigin = GetAbsOrigin();
				}
				
				m_flLastStuckCheck = gpGlobals->curtime + 1.0f;
			}

			if (m_bReachedMoveGoal )
			{
				SetIdealActivity( (Activity)ACT_CROW_LAND );
				SetFlyingState( FlyState_Landing );
				TaskMovementComplete();
			}
			else
			{
				SetIdealActivity ( ACT_FLY );
				MoveCrowFly( flInterval );
			}

		}
		else //if ( !GetTask() || GetTask()->iTask == TASK_WAIT_FOR_MOVEMENT )
		{
			SetSchedule( SCHED_CROW_IDLE_FLY );
			SetFlyingState( FlyState_Flying );
			SetIdealActivity ( ACT_FLY );
		}
		return true;
	}
	
	return false;
}

Activity CNPC_Crow::NPC_TranslateActivity( Activity eNewActivity )
{
	if ( IsFlying() && eNewActivity == ACT_IDLE )
	{
		return ACT_FLY;
	}

	if ( eNewActivity == ACT_FLY )
	{
		if ( m_flSoarTime < gpGlobals->curtime )
		{
			//Adrian: This should be revisited.
			if ( random->RandomInt( 0, 100 ) <= 50 && m_bSoar == false && GetAbsVelocity().z < 0 )
			{
				m_bSoar = true;
				m_flSoarTime = gpGlobals->curtime + random->RandomFloat( 1, 4 );
			}
			else
			{
				m_bSoar = false;
				m_flSoarTime = gpGlobals->curtime + random->RandomFloat( 3, 5 );
			}
		}

		if ( m_bSoar == true )
		{
			return (Activity)ACT_CROW_SOAR;
		}
		else
			return ACT_FLY;
	}

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}


//-----------------------------------------------------------------------------
// Purpose: Handles all flight movement.
// Input  : flInterval - Seconds to simulate.
//-----------------------------------------------------------------------------
void CNPC_Crow::MoveCrowFly( float flInterval )
{
	//
	// Bound interval so we don't get ludicrous motion when debugging
	// or when framerate drops catastrophically.  
	//
	if (flInterval > 1.0)
	{
		flInterval = 1.0;
	}

	m_flDangerSoundTime = gpGlobals->curtime + 5.0f;

	//
	// Determine the goal of our movement.
	//
	Vector vecMoveGoal = GetAbsOrigin();

	if ( GetNavigator()->IsGoalActive() )
	{
		vecMoveGoal = GetNavigator()->GetCurWaypointPos();

		if ( GetNavigator()->CurWaypointIsGoal() == false  )
		{
  			AI_ProgressFlyPathParams_t params( MASK_NPCSOLID );
			params.bTrySimplify = false;

			GetNavigator()->ProgressFlyPath( params ); // ignore result, crow handles completion directly

			// Fly towards the hint.
			if ( GetNavigator()->GetPath()->GetCurWaypoint() )
			{
				vecMoveGoal = GetNavigator()->GetCurWaypointPos();
			}
		}
	}
	else
	{
		// No movement goal.
		vecMoveGoal = GetAbsOrigin();
		SetAbsVelocity( vec3_origin );
		return;
	}

	Vector vecMoveDir = ( vecMoveGoal - GetAbsOrigin() );
	Vector vForward;
	AngleVectors( GetAbsAngles(), &vForward );
	
	//
	// Fly towards the movement goal.
	//
	float flDistance = ( vecMoveGoal - GetAbsOrigin() ).Length();

	if ( vecMoveGoal != m_vDesiredTarget )
	{
		m_vDesiredTarget = vecMoveGoal;
	}
	else
	{
		m_vCurrentTarget = ( m_vDesiredTarget - GetAbsOrigin() );
		VectorNormalize( m_vCurrentTarget );
	}

	float flLerpMod = 0.25f;

	if ( flDistance <= 256.0f )
	{
		flLerpMod = 1.0f - ( flDistance / 256.0f );
	}


	VectorLerp( vForward, m_vCurrentTarget, flLerpMod, vForward );


	if ( flDistance < CROW_AIRSPEED * flInterval )
	{
		if ( GetNavigator()->IsGoalActive() )
		{
			if ( GetNavigator()->CurWaypointIsGoal() )
			{
				m_bReachedMoveGoal = true;
			}
			else
			{
				GetNavigator()->AdvancePath();
			}
		}
		else
			m_bReachedMoveGoal = true;
	}

	if ( GetHintNode() )
	{
		AIMoveTrace_t moveTrace;
		GetMoveProbe()->MoveLimit( NAV_FLY, GetAbsOrigin(), GetNavigator()->GetCurWaypointPos(), MASK_NPCSOLID, GetNavTargetEntity(), &moveTrace );

		//See if it succeeded
		if ( IsMoveBlocked( moveTrace.fStatus ) )
		{
			Vector vNodePos = vecMoveGoal;
			GetHintNode()->GetPosition(this, &vNodePos);
			
			GetNavigator()->SetGoal( vNodePos );
		}
	}

	//
	// Look to see if we are going to hit anything.
	//
	VectorNormalize( vForward );
	Vector vecDeflect;
	if ( Probe( vForward, CROW_AIRSPEED * flInterval, vecDeflect ) )
	{
		vForward = vecDeflect;
		VectorNormalize( vForward );
	}

	SetAbsVelocity( vForward * CROW_AIRSPEED );

	if ( GetAbsVelocity().Length() > 0 && GetNavigator()->CurWaypointIsGoal() && flDistance < CROW_AIRSPEED )
	{
		SetIdealActivity( (Activity)ACT_CROW_LAND );
	}


	//Bank and set angles.
	Vector vRight;
	QAngle vRollAngle;
	
	VectorAngles( vForward, vRollAngle );
	vRollAngle.z = 0;

	AngleVectors( vRollAngle, NULL, &vRight, NULL );

	float flRoll = DotProduct( vRight, vecMoveDir ) * 45;
	flRoll = clamp( flRoll, -45, 45 );

	vRollAngle[ROLL] = flRoll;
	SetAbsAngles( vRollAngle );
}

//-----------------------------------------------------------------------------
// Purpose: Looks ahead to see if we are going to hit something. If we are, a
//			recommended avoidance path is returned.
// Input  : vecMoveDir - 
//			flSpeed - 
//			vecDeflect - 
// Output : Returns true if we hit something and need to deflect our course,
//			false if all is well.
//-----------------------------------------------------------------------------
bool CNPC_Crow::Probe( const Vector &vecMoveDir, float flSpeed, Vector &vecDeflect )
{
	//
	// Look 1/2 second ahead.
	//
	trace_t tr;
	AI_TraceHull( GetAbsOrigin(), GetAbsOrigin() + vecMoveDir * flSpeed, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, HL2COLLISION_GROUP_CROW, &tr );
	if ( tr.fraction < 1.0f )
	{
		//
		// If we hit something, deflect flight path parallel to surface hit.
		//
		Vector vecUp;
		CrossProduct( vecMoveDir, tr.plane.normal, vecUp );
		CrossProduct( tr.plane.normal, vecUp, vecDeflect );
		VectorNormalize( vecDeflect );
		return true;
	}

	vecDeflect = vec3_origin;
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Switches between flying mode and ground mode.
//-----------------------------------------------------------------------------
void CNPC_Crow::SetFlyingState( FlyState_t eState )
{
	if ( eState == FlyState_Flying )
	{
		// Flying
		SetGroundEntity( NULL );
		AddFlag( FL_FLY );
		SetNavType( NAV_FLY );
		CapabilitiesRemove( bits_CAP_MOVE_GROUND );
		CapabilitiesAdd( bits_CAP_MOVE_FLY );
		SetMoveType( MOVETYPE_STEP );
		m_vLastStoredOrigin = GetAbsOrigin();
		m_flLastStuckCheck = gpGlobals->curtime + 3.0f;
		m_flGroundIdleMoveTime = gpGlobals->curtime + random->RandomFloat( 5.0f, 10.0f );
	}
	else if ( eState == FlyState_Walking )
	{
		// Walking
		QAngle angles = GetAbsAngles();
		angles[PITCH] = 0.0f;
		angles[ROLL] = 0.0f;
		SetAbsAngles( angles );

		RemoveFlag( FL_FLY );
		SetNavType( NAV_GROUND );
		CapabilitiesRemove( bits_CAP_MOVE_FLY );
		CapabilitiesAdd( bits_CAP_MOVE_GROUND );
		SetMoveType( MOVETYPE_STEP );
		m_vLastStoredOrigin = vec3_origin;
		m_flGroundIdleMoveTime = gpGlobals->curtime + random->RandomFloat( 5.0f, 10.0f );
	}
	else
	{
		// Falling
		RemoveFlag( FL_FLY );
		SetNavType( NAV_GROUND );
		CapabilitiesRemove( bits_CAP_MOVE_FLY );
		CapabilitiesAdd( bits_CAP_MOVE_GROUND );
		SetMoveType( MOVETYPE_STEP );
		m_flGroundIdleMoveTime = gpGlobals->curtime + random->RandomFloat( 5.0f, 10.0f );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Performs a takeoff. Called via an animation event at the moment
//			our feet leave the ground.
// Input  : pGoalEnt - The entity that we are going to fly toward.
//-----------------------------------------------------------------------------
void CNPC_Crow::Takeoff( const Vector &vGoal )
{
	if ( vGoal != vec3_origin )
	{
		//
		// Lift us off ground so engine doesn't instantly reset FL_ONGROUND.
		//
		UTIL_SetOrigin( this, GetAbsOrigin() + Vector( 0 , 0 , 1 ));

		//
		// Fly straight at the goal entity at our maximum airspeed.
		//
		Vector vecMoveDir = vGoal - GetAbsOrigin();
		VectorNormalize( vecMoveDir );
		
		// FIXME: pitch over time

		SetFlyingState( FlyState_Flying );

		QAngle angles;
		VectorAngles( vecMoveDir, angles );
		SetAbsAngles( angles );

		SetAbsVelocity( vecMoveDir * CROW_TAKEOFF_SPEED );
	}
}

void CNPC_Crow::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	CTakeDamageInfo	newInfo = info;

	if ( info.GetDamageType() & DMG_PHYSGUN )
	{
		Vector	puntDir = ( info.GetDamageForce() * 5000.0f );

		newInfo.SetDamage( m_iMaxHealth );

		PainSound( newInfo );
		newInfo.SetDamageForce( puntDir );
	}

	BaseClass::TraceAttack( newInfo, vecDir, ptr );
}


void CNPC_Crow::StartTargetHandling( CBaseEntity *pTargetEnt )
{
	AI_NavGoal_t goal( GOALTYPE_PATHCORNER, pTargetEnt->GetAbsOrigin(),
					   ACT_FLY,
					   AIN_DEF_TOLERANCE, AIN_YAW_TO_DEST);

	if ( !GetNavigator()->SetGoal( goal ) )
	{
		DevWarning( 2, "Can't Create Route!\n" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CNPC_Crow::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
		//
		// This task enables us to build a path that requires flight.
		//
//		case TASK_CROW_PREPARE_TO_FLY:
//		{
//			SetFlyingState( FlyState_Flying );
//			TaskComplete();
//			break;
//		}

		case TASK_CROW_TAKEOFF:
		{
			if ( random->RandomInt( 1, 2 ) == 1 )
			{
				AlertSound();
			}

			FlapSound();

			SetIdealActivity( ( Activity )ACT_CROW_TAKEOFF );
			break;
		}

		case TASK_CROW_PICK_EVADE_GOAL:
		{
			if ( GetEnemy() != NULL )
			{
				//
				// Get our enemy's position in x/y.
				//
				Vector vecEnemyOrigin = GetEnemy()->GetAbsOrigin();
				vecEnemyOrigin.z = GetAbsOrigin().z;

				//
				// Pick a hop goal a random distance along a vector away from our enemy.
				//
				m_vSavePosition = GetAbsOrigin() - vecEnemyOrigin;
				VectorNormalize( m_vSavePosition );
				m_vSavePosition = GetAbsOrigin() + m_vSavePosition * ( 32 + random->RandomInt( 0, 32 ) );

				GetMotor()->SetIdealYawToTarget( m_vSavePosition );
				TaskComplete();
			}
			else
			{
				TaskFail( "No enemy" );
			}
			break;
		}

		case TASK_CROW_FALL_TO_GROUND:
		{
			SetFlyingState( FlyState_Falling );
			break;
		}

		case TASK_FIND_HINTNODE:
		{
			if ( GetGoalEnt() )
			{
				TaskComplete();
				return;
			}
			// Overloaded because we search over a greater distance.
			if ( !hasFakeHintNode )
			{
//				SetHintNode(CAI_HintManager::FindHint( this, HINT_CROW_FLYTO_POINT, bits_HINT_NODE_NEAREST | bits_HINT_NODE_USE_GROUP, 10000 ));
//				this is deathmatch, there are no hint nodes! Instead, we'll do two traces in opposite directions, and see which gets further,
//				then fly a random distance in that direction.
				trace_t tr1, tr2;
				m_vFakeHintNode = Vector(random->RandomFloat(-1.0f,1.0f),random->RandomFloat(-1.0f,1.0f),0);
				AI_TraceLine ( EyePosition(), EyePosition() + m_vFakeHintNode * 10000.0f, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr1 );
				AI_TraceLine ( EyePosition(), EyePosition() - m_vFakeHintNode * 10000.0f, MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr2 );
				float f = random->RandomFloat(0.0f,1.0f);
				m_vFakeHintNode = f * tr1.endpos + (1.0f-f) * tr2.endpos;

				// only now think about height
				AI_TraceLine( m_vFakeHintNode, m_vFakeHintNode + Vector(0,0,256), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr1 );
				AI_TraceLine( m_vFakeHintNode, m_vFakeHintNode + Vector(0,0,-256), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr2 );
				m_vFakeHintNode.z = random->RandomFloat( tr1.endpos.z, tr2.endpos.z ); // go up or down, pick somewhere!

				hasFakeHintNode = true;
				TaskComplete();
			}
/*
			if ( GetHintNode() )
			{
				TaskComplete();
			}
			else
			{
				TaskFail( FAIL_NO_HINT_NODE );
			}*/
			break;
		}

		case TASK_GET_PATH_TO_HINTNODE:
		{
			//How did this happen?!
			if ( GetGoalEnt() == this )
			{
				SetGoalEnt( NULL );
			}

			if ( GetGoalEnt() )
			{
				SetFlyingState( FlyState_Flying );
				StartTargetHandling( GetGoalEnt() );
			
				m_bReachedMoveGoal = false;
				TaskComplete();
				hasFakeHintNode = false;
				//SetHintNode( NULL );
				return;
			}

			if ( hasFakeHintNode )
			{
//				Vector vHintPos;
//				GetHintNode()->GetPosition(this, &vHintPos);
		
				SetNavType( NAV_FLY );
				CapabilitiesAdd( bits_CAP_MOVE_FLY );
				// @HACKHACK: Force allow triangulation. Too many HL2 maps were relying on this feature WRT fly nodes (toml 8/1/2007)
/*				NPC_STATE state = GetState();
				m_NPCState = NPC_STATE_SCRIPT;
				bool bFoundPath = GetNavigator()->SetGoal( vHintPos );
				m_NPCState = state;
				if ( !bFoundPath )
				{
					GetHintNode()->DisableForSeconds( .3 );
					SetHintNode(NULL);
				}
*/
				if ( !GetNavigator()->SetGoal( m_vFakeHintNode ) )
					hasFakeHintNode = false;//SetHintNode(NULL);
				CapabilitiesRemove( bits_CAP_MOVE_FLY );
			}

			if ( hasFakeHintNode ) //GetHintNode() )
			{
				m_bReachedMoveGoal = false;
				TaskComplete();
			}
			else
			{
				TaskFail( FAIL_NO_ROUTE );
			}
			break;
		}

		//
		// We have failed to fly normally. Pick a random "up" direction and fly that way.
		//
		case TASK_CROW_FLY:
		{
			float flYaw = UTIL_AngleMod( random->RandomInt( -180, 180 ) );

			Vector vecNewVelocity( cos( DEG2RAD( flYaw ) ), sin( DEG2RAD( flYaw ) ), random->RandomFloat( 0.1f, 0.5f ) );
			vecNewVelocity *= CROW_AIRSPEED;
			SetAbsVelocity( vecNewVelocity );

			SetIdealActivity( ACT_FLY );

			m_bSoar = false;
			m_flSoarTime = gpGlobals->curtime + random->RandomFloat( 2, 5 );

			break;
		}

		case TASK_CROW_PICK_RANDOM_GOAL:
		{
			m_vSavePosition = GetLocalOrigin() + Vector( random->RandomFloat( -48.0f, 48.0f ), random->RandomFloat( -48.0f, 48.0f ), 0 );
			TaskComplete();
			break;
		}

		case TASK_CROW_HOP:
		{
			SetIdealActivity( ACT_HOP );
			m_flHopStartZ = GetLocalOrigin().z;
			break;
		}

		case TASK_CROW_EXPLODE:
		{
			Explode();
			break;
		}
		case TASK_CROW_COO:
		{
			IdleSound();
			TaskComplete();
		}

		case TASK_FLY_KAMAKAZE:
		{
			break;
		}

		case TASK_FIX_ANGLES:
		{
			QAngle a = GetAbsAngles();
			a[PITCH] = 0;
			a[ROLL] = 0;
			SetAbsAngles(a);
			TaskComplete();
			break;
		}

		case TASK_CROW_WAIT_FOR_BARNACLE_KILL:
		{
			break;
		}

		default:
		{
			BaseClass::StartTask( pTask );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CNPC_Crow::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
		case TASK_CROW_TAKEOFF:
		{
			if ( GetEnemy() )
				GetMotor()->SetIdealYawToTargetAndUpdate( GetEnemy()->GetAbsOrigin(), AI_KEEP_YAW_SPEED );
			else
				TaskFail( FAIL_NO_ENEMY );

			if ( IsActivityFinished() )
			{
				TaskComplete();
				SetIdealActivity( ACT_FLY );

				m_bSoar = false;
				m_flSoarTime = gpGlobals->curtime + random->RandomFloat( 2, 5 );
			}
			
			break;
		}

		case TASK_CROW_HOP:
		{
			if ( IsActivityFinished() )
			{
				TaskComplete();
				SetIdealActivity( ACT_IDLE );
			}

			if ( ( GetAbsOrigin().z < m_flHopStartZ ) && ( !( GetFlags() & FL_ONGROUND ) ) )
			{
				//
				// We've hopped off of something! See if we're going to fall very far.
				//
				trace_t tr;
				AI_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, -32 ), MASK_SOLID, this, HL2COLLISION_GROUP_CROW, &tr );
				if ( tr.fraction == 1.0f )
				{
					//
					// We're falling! Better fly away. SelectSchedule will check ONGROUND and do the right thing.
					//
					TaskComplete();
				}
				else
				{
					//
					// We'll be okay. Don't check again unless what we're hopping onto moves
					// out from under us.
					//
					m_flHopStartZ = GetAbsOrigin().z - ( 32 * tr.fraction );
				}
			}

			break;
		}

		case TASK_CROW_EXPLODE:
		{

			break;
		}
		//
		// Face the direction we are flying.
		//
		case TASK_CROW_FLY:
		{
			GetMotor()->SetIdealYawToTargetAndUpdate( GetAbsOrigin() + GetAbsVelocity(), AI_KEEP_YAW_SPEED );

			break;
		}

		case TASK_CROW_FALL_TO_GROUND:
		{
			if ( GetFlags() & FL_ONGROUND )
			{
				SetFlyingState( FlyState_Walking );
				TaskComplete();
			}
			break;
		}

		case TASK_FLY_KAMAKAZE:
		{
/*
Possible future AI upgrade:

if has a target
	if can see it
		fly to it
	else
		if can see target's last known position
			fly to that
		else
			if we have a target
				set it to NULL
				fly to position from where target could last be scene
else
	fail
*/
			if ( !GetEnemy() || m_flLastRecalcEnemyTime < gpGlobals->curtime - SEEK_INTERVAL )
			{
				SetEnemy( BestEnemy() );
				m_flLastRecalcEnemyTime = gpGlobals->curtime;
			}
			CBaseEntity *pTarget = GetEnemy();

			if ( pTarget == NULL )
			{
				TaskFail("Enemy lost");
				break;
			}
//			Msg("Kamakaze flight task running: ");
			Vector target = pTarget->BodyTarget( GetAbsOrigin() );
			Vector seperation = target - GetAbsOrigin();
			float dist = seperation.Length();
			if ( dist < CROW_EXPLODE_DISTANCE )
			{
//				Msg("Close to target, flight complete!\n");
				TaskComplete();
				break;
			}

			trace_t tr;
			AI_TraceLine( EyePosition(), target, MASK_SOLID, this, HL2COLLISION_GROUP_CROW, &tr );
			if ( ( tr.endpos - target).Length() > CROW_EXPLODE_DISTANCE )
			{
				TaskFail( FAIL_NO_ROUTE ); // can't see him
			}

			Vector dir = seperation;
			VectorNormalize(dir);
						
			if ( seperation.Length2D() < CROW_HEIGHT_GAIN_DISTANCE )
			{
				AI_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector(0,0,-CROW_PREFERRED_HEIGHT), MASK_SOLID, this, HL2COLLISION_GROUP_CROW, &tr );
				if ( tr.DidHit() ) // I'm very low to the ground, climb slightly
				{
					dir.z += 0.30;
					VectorNormalize(dir);
				}
			}

			Vector vel = dir * CROW_AIRSPEED;
			SetAbsVelocity( vel );
			GetMotor()->SetIdealYawToTargetAndUpdate( GetAbsOrigin() + GetAbsVelocity(), AI_KEEP_YAW_SPEED );
			break;
		}

		case TASK_CROW_WAIT_FOR_BARNACLE_KILL:
		{
			if ( m_flNextFlinchTime < gpGlobals->curtime )
			{
				m_flNextFlinchTime = gpGlobals->curtime + random->RandomFloat( 0.5f, 2.0f );
				// dvs: TODO: squirm
				// dvs: TODO: spawn feathers
				EmitSound( "NPC_Crow.Squawk" );
			}
			break;
		}

		default:
		{
			CAI_BaseNPC::RunTask( pTask );
		}
	}
}

void CNPC_Crow::Explode()
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetMasterPlayer() );
	ExplosionCreate( GetAbsOrigin() + Vector(0,0,8), GetAbsAngles(), pPlayer,
		CROW_EXPLODE_DAMAGE, CROW_EXPLODE_RADIUS, 0x00000000, 0.0f, this, DMG_BLAST);
	UTIL_ScreenShake( GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START );
	if ( pPlayer )
		pPlayer->GetLimitedQuantities()->Remove(LQ_CROW);
	UTIL_Remove(this);
}

CBaseEntity *CNPC_Crow::BestEnemy()
{
//	return BaseClass::BestEnemy();
	CBaseEntity *pBestEnemy	= NULL;
	int	iBestDistSq	= MAX_COORD_RANGE * MAX_COORD_RANGE;	// so first visible entity will become the closest.
	AIEnemiesIter_t iter;
	Vector vecFacing;
	AngleVectors(GetAbsAngles(),&vecFacing,NULL,NULL);

	for( AI_EnemyInfo_t *pEMemory = GetEnemies()->GetFirst( &iter ); pEMemory != NULL; pEMemory = GetEnemies()->GetNext( &iter ) )
	{
		CBaseEntity *pEnemy = pEMemory->hEnemy;
		if ( !pEnemy || !pEnemy->IsAlive() )
			continue;
		
		if ( ( pEnemy->GetFlags() & FL_NOTARGET ) || HL2MPRules()->IsFriendly(this,pEnemy) )
			continue;

		if ( pEnemy->MyNPCPointer() && !pEnemy->MyNPCPointer()->CanBeAnEnemyOf(this) )
			continue;

//		if ( pEMemory->timeLastSeen < gpGlobals->curtime - 0.15f )
//			continue;

		int lengthSq = ( pEnemy->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr();
		if ( lengthSq > iBestDistSq )
			continue;

		Vector dir = pEnemy->GetAbsOrigin() - GetAbsOrigin();
		VectorNormalize(dir);
		if ( IsFlying() && DotProduct( dir, vecFacing ) <= 0.1f )
			continue; // when flying, only those in front of us will do

		trace_t tr;
		AI_TraceLine(EyePosition(), pEnemy->WorldSpaceCenter(),MASK_NPCSOLID_BRUSHONLY, this, HL2COLLISION_GROUP_CROW, &tr); 
		if ( !tr.DidHit() || tr.m_pEnt == pEnemy )
		{
			pBestEnemy = pEnemy;
			iBestDistSq = lengthSq;
		}
	}

	return pBestEnemy;
}

//------------------------------------------------------------------------------
// Purpose: Override to do crow specific gibs.
// Output : Returns true to gib, false to not gib.
//-----------------------------------------------------------------------------
bool CNPC_Crow::CorpseGib( const CTakeDamageInfo &info )
{
	EmitSound( "NPC_Crow.Gib" );

	// TODO: crow gibs?
	//CGib::SpawnSpecificGibs( this, CROW_GIB_COUNT, 300, 400, "models/gibs/crow_gibs.mdl");

	return true;
}

//-----------------------------------------------------------------------------
// Don't allow ridiculous forces to be applied to the crow. It only weighs
// 1.5kg, so extreme forces will give it ridiculous velocity.
//-----------------------------------------------------------------------------
#define CROW_RAGDOLL_SPEED_LIMIT	600.0f  // Crow ragdoll speed limit in inches per second.
bool CNPC_Crow::BecomeRagdollOnClient( const Vector &force )
{
	Vector newForce = force;
	
	if( VPhysicsGetObject() )
	{
		float flMass = VPhysicsGetObject()->GetMass();
		float speed = VectorNormalize( newForce );
		speed = min( speed, (CROW_RAGDOLL_SPEED_LIMIT * flMass) );
		newForce *= speed;
	}

	return BaseClass::BecomeRagdollOnClient( newForce );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_Crow::FValidateHintType( CAI_Hint *pHint )
{
	return true;//return( pHint->HintType() == HINT_CROW_FLYTO_POINT );
}


//-----------------------------------------------------------------------------
// Purpose: Returns the activity for the given hint type.
// Input  : sHintType - 
//-----------------------------------------------------------------------------
Activity CNPC_Crow::GetHintActivity( short sHintType, Activity HintsActivity )
{
	if ( sHintType == HINT_CROW_FLYTO_POINT )
	{
		return ACT_FLY;
	}

	return BaseClass::GetHintActivity( sHintType, HintsActivity );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pevInflictor - 
//			pevAttacker - 
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
int CNPC_Crow::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	CTakeDamageInfo outInfo = info;

/*	if( outInfo.GetDamageType() & DMG_BLAST )
	{
		outInfo.SetDamage( 0 ); // TEMPORARY CRASH-STOPPER
		Ignite(30);
	}*/

	// TODO: spew a feather or two
	return BaseClass::OnTakeDamage_Alive( outInfo );
}
/*
void CNPC_Crow::RunAI()
{
	if (GetEnemy() != NULL && 
		!GetEnemy()->IsAlive())
	{
		ClearEnemyMemory();
		SetEnemy( NULL );
	}

	GetSenses()->Look( 4092 );
	SetEnemy(BestEnemy());
	
	SetNextThink( gpGlobals->curtime + 0.05 );
	
//	BaseClass::RunAI();
}*/

//-----------------------------------------------------------------------------
// Purpose: Returns the best new schedule for this NPC based on current conditions.
//-----------------------------------------------------------------------------
int CNPC_Crow::SelectSchedule( void )
{
	if ( HasCondition( COND_CROW_BARNACLED ) )
	{
		// Caught by a barnacle!
		return SCHED_CROW_BARNACLED;
	}

	if ( HasCondition( COND_SEE_ENEMY ) || HasCondition( COND_SEE_DISLIKE ) || HasCondition( COND_SEE_HATE ) || HasCondition( COND_SEE_FEAR ) || HasCondition( COND_SEE_NEMESIS ) )
	{
		SetEnemy( BestEnemy() );
		if ( IsFlying() )
			return SCHED_CROW_KAMAKAZE;
		return SCHED_CROW_START_TO_FLY_KAMAKAZE;
	}

	if ( !IsFlying() )
	{
		//
		// If we are hanging out on the ground, see if it is time to pick a new place to walk to.
		//
		if ( gpGlobals->curtime > m_flGroundIdleMoveTime )
		{
			m_flGroundIdleMoveTime = gpGlobals->curtime + random->RandomFloat( 2.0f, 10.0f );
			return SCHED_CROW_IDLE_WALK;
		}

		return SCHED_CROW_IDLE_STAND;
	}

	return SCHED_CROW_FLY_FAIL;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Crow::Precache( void )
{
	BaseClass::Precache();
	
	PrecacheModel( "models/crow.mdl" );
	PrecacheModel( "models/pigeon.mdl" );
	PrecacheModel( "models/seagull.mdl" );

	//Crow
	PrecacheScriptSound( "NPC_Crow.Hop" );
	PrecacheScriptSound( "NPC_Crow.Squawk" );
	PrecacheScriptSound( "NPC_Crow.Gib" );
	PrecacheScriptSound( "NPC_Crow.Idle" );
	PrecacheScriptSound( "NPC_Crow.Alert" );
	PrecacheScriptSound( "NPC_Crow.Die" );
	PrecacheScriptSound( "NPC_Crow.Pain" );
	PrecacheScriptSound( "NPC_Crow.Flap" );

	//Seagull
	PrecacheScriptSound( "NPC_Seagull.Pain" );
	PrecacheScriptSound( "NPC_Seagull.Idle" );

	//Pigeon
	PrecacheScriptSound( "NPC_Pigeon.Idle");
}


//-----------------------------------------------------------------------------
// Purpose: Sounds.
//-----------------------------------------------------------------------------
void CNPC_Crow::IdleSound( void )
{
	if ( m_iBirdType != BIRDTYPE_CROW )
		 return;

	EmitSound( "NPC_Crow.Idle" );
}


void CNPC_Crow::AlertSound( void )
{
	if ( m_iBirdType != BIRDTYPE_CROW )
		 return;

	EmitSound( "NPC_Crow.Alert" );
}


void CNPC_Crow::PainSound( const CTakeDamageInfo &info )
{
	if ( m_iBirdType != BIRDTYPE_CROW )
		 return;

	EmitSound( "NPC_Crow.Pain" );
}


void CNPC_Crow::DeathSound( const CTakeDamageInfo &info )
{
	if ( m_iBirdType != BIRDTYPE_CROW )
		 return;

	EmitSound( "NPC_Crow.Die" );
}

void CNPC_Crow::FlapSound( void )
{
	EmitSound( "NPC_Crow.Flap" );
	m_bPlayedLoopingSound = true;
}


//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CNPC_Crow::HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt )
{
	if ( interactionType == g_interactionBarnacleVictimDangle )
	{
		// Die instantly
		return false;
	}
	else if ( interactionType == g_interactionBarnacleVictimGrab )
	{
		if ( GetFlags() & FL_ONGROUND )
		{
			SetGroundEntity( NULL );
		}

		// return ideal grab position
		if (data)
		{
			// FIXME: need a good way to ensure this contract
			*((Vector *)data) = GetAbsOrigin() + Vector( 0, 0, 5 );
		}

		StopLoopingSounds();

		SetThink( NULL );
		return true;
	}

	return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_Crow::DrawDebugTextOverlays( void )
{
	int nOffset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[512];
		Q_snprintf( tempstr, sizeof( tempstr ), "morale: %d", m_nMorale );
		EntityText( nOffset, tempstr, 0 );
		nOffset++;

/*		if ( GetEnemy() != NULL )
		{
			Q_snprintf( tempstr, sizeof( tempstr ), "enemy (dist): %s (%g)", GetEnemy()->GetClassname(), ( double )m_flEnemyDist );
			EntityText( nOffset, tempstr, 0 );
			nOffset++;
		}
*/	}

	return nOffset;
}


//-----------------------------------------------------------------------------
// Purpose: Determines which sounds the crow cares about.
//-----------------------------------------------------------------------------
int CNPC_Crow::GetSoundInterests( void )
{
	return	SOUND_WORLD | SOUND_COMBAT | SOUND_PLAYER | SOUND_DANGER;
}


void CNPC_Crow::Event_Killed(const CTakeDamageInfo &info)
{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetMasterPlayer() );
		if ( pPlayer )
		{
			pPlayer->GetLimitedQuantities()->Remove(LQ_CROW);
		}

		if ( info.GetAttacker() != this && info.GetAttacker() != pPlayer && ((info.GetDamageType() & DMG_SLASH) || (info.GetDamageType() & DMG_CLUB)) )
			Explode(); // explode when killed by melee - thats nicer than having melee characters ignore me

		BaseClass::Event_Killed(info);
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_crow, CNPC_Crow )

	DECLARE_TASK( TASK_CROW_FIND_FLYTO_NODE )
	//DECLARE_TASK( TASK_CROW_PREPARE_TO_FLY )
	DECLARE_TASK( TASK_CROW_TAKEOFF )
	DECLARE_TASK( TASK_CROW_FLY )
	DECLARE_TASK( TASK_CROW_PICK_RANDOM_GOAL )
	DECLARE_TASK( TASK_CROW_HOP )
	DECLARE_TASK( TASK_CROW_EXPLODE )
	DECLARE_TASK( TASK_FLY_KAMAKAZE )
	DECLARE_TASK( TASK_FIX_ANGLES )
	DECLARE_TASK( TASK_CROW_COO )
	DECLARE_TASK( TASK_CROW_PICK_EVADE_GOAL )
	DECLARE_TASK( TASK_CROW_WAIT_FOR_BARNACLE_KILL )

	// experiment
	DECLARE_TASK( TASK_CROW_FALL_TO_GROUND )
	DECLARE_TASK( TASK_CROW_PREPARE_TO_FLY_RANDOM )

	DECLARE_ACTIVITY( ACT_CROW_TAKEOFF )
	DECLARE_ACTIVITY( ACT_CROW_SOAR )
	DECLARE_ACTIVITY( ACT_CROW_LAND )

	DECLARE_ANIMEVENT( AE_CROW_HOP )
	DECLARE_ANIMEVENT( AE_CROW_FLY )
	DECLARE_ANIMEVENT( AE_CROW_TAKEOFF )
	

	DECLARE_CONDITION( COND_CROW_ENEMY_TOO_CLOSE )
	DECLARE_CONDITION( COND_CROW_ENEMY_WAY_TOO_CLOSE )
	DECLARE_CONDITION( COND_CROW_FORCED_FLY )
	DECLARE_CONDITION( COND_CROW_BARNACLED )
	DECLARE_CONDITION( COND_CROW_NEW_TARGET )

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_IDLE_STAND,

		"	Tasks"
		"		TASK_STOP_MOVING		1"
		"		TASK_SET_ACTIVITY		ACTIVITY:ACT_IDLE"
		"		TASK_WAIT				5"
		"		TASK_WAIT_PVS			0"
		""
		"	Interrupts"
		"		COND_SEE_DISLIKE"
		"		COND_SEE_HATE"
		"		COND_SEE_FEAR"
		"		COND_NEW_ENEMY"
		"		COND_SEE_FEAR"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_SMELL"
		"		COND_PROVOKED"
		"		COND_GIVE_WAY"
		"		COND_HEAR_PLAYER"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_COMBAT"
		"		COND_HEAR_BULLET_IMPACT"
		"		COND_IDLE_INTERRUPT"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_IDLE_WALK,
		
		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_IDLE_STAND"
		"		TASK_CROW_PICK_RANDOM_GOAL		0"
		"		TASK_GET_PATH_TO_SAVEPOSITION	0"
		"		TASK_FIX_ANGLES					0"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_WAIT_PVS					0"
		"		"
		"	Interrupts"
		"		COND_SEE_DISLIKE"
		"		COND_SEE_HATE"
		"		COND_SEE_FEAR"
		"		COND_SEE_ENEMY"
		"		COND_CROW_FORCED_FLY"
		"		COND_PROVOKED"
		"		COND_CROW_ENEMY_TOO_CLOSE"
		"		COND_NEW_ENEMY"
		"		COND_HEAVY_DAMAGE"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_COMBAT"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_WALK_AWAY,
		
		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROW_FLY_AWAY"
		"		TASK_CROW_PICK_EVADE_GOAL		0"
		"		TASK_GET_PATH_TO_SAVEPOSITION	0"
		"		TASK_FIX_ANGLES					0"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		"
		"	Interrupts"
		"		COND_SEE_DISLIKE"
		"		COND_SEE_HATE"
		"		COND_SEE_FEAR"
		"		COND_SEE_ENEMY"
		"		COND_CROW_FORCED_FLY"
		"		COND_CROW_ENEMY_WAY_TOO_CLOSE"
		"		COND_NEW_ENEMY"
		"		COND_HEAVY_DAMAGE"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_COMBAT"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_RUN_AWAY,
		
		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROW_FLY_AWAY"
		"		TASK_CROW_PICK_EVADE_GOAL		0"
		"		TASK_GET_PATH_TO_SAVEPOSITION	0"
		"		TASK_FIX_ANGLES					0"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		"
		"	Interrupts"
		"		COND_SEE_DISLIKE"
		"		COND_SEE_HATE"
		"		COND_SEE_FEAR"
		"		COND_SEE_ENEMY"
		"		COND_CROW_FORCED_FLY"
		"		COND_CROW_ENEMY_WAY_TOO_CLOSE"
		"		COND_NEW_ENEMY"
		"		COND_HEAVY_DAMAGE"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_COMBAT"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_HOP_AWAY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROW_FLY_AWAY"
		"		TASK_STOP_MOVING				0"
		"		TASK_FIX_ANGLES					0"
		"		TASK_CROW_PICK_EVADE_GOAL		0"
		"		TASK_FACE_IDEAL					0"
		"		TASK_CROW_HOP					0"
		"	"
		"	Interrupts"
		"		COND_CROW_FORCED_FLY"
		"		COND_HEAVY_DAMAGE"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_COMBAT"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_COMBAT"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_IDLE_FLY,
		
		"	Tasks"
		"		TASK_FIND_HINTNODE				0"
		"		TASK_GET_PATH_TO_HINTNODE		0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		"
		"	Interrupts"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_FLY_AWAY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROW_FLY_FAIL"
		"		TASK_STOP_MOVING				0"
		"		TASK_FIND_HINTNODE				0"
		"		TASK_GET_PATH_TO_HINTNODE		0"
		"		TASK_CROW_TAKEOFF				0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"	"
		"	Interrupts"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_FLY,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROW_FLY_FAIL"
		"		TASK_STOP_MOVING				0"
		"		TASK_CROW_TAKEOFF				0"
		"		TASK_CROW_FLY					0"
		"	"
		"	Interrupts"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_FLY_FAIL,

		"	Tasks"
		"		TASK_CROW_FALL_TO_GROUND		0"
		"		TASK_FIX_ANGLES					0"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_CROW_IDLE_WALK"
		"	"
		"	Interrupts"
	)
	
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_START_TO_FLY_KAMAKAZE,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROW_FLY_FAIL"
		"		TASK_STOP_MOVING				0"
//		"		TASK_FACE_ENEMY					0"
		"		TASK_CROW_TAKEOFF				0"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_CROW_KAMAKAZE"
		"	"
		"	Interrupts"
	)

	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_CROW_KAMAKAZE,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CROW_FLY_FAIL"
		"		TASK_FLY_KAMAKAZE				0"
		"		TASK_CROW_EXPLODE				0"
		"	"
		"	Interrupts"
	)

	//=========================================================
	// Crow is in the clutches of a barnacle
	DEFINE_SCHEDULE
	(
		SCHED_CROW_BARNACLED,

		"	Tasks"
		"		TASK_STOP_MOVING						0"
		"		TASK_SET_ACTIVITY						ACTIVITY:ACT_HOP"
		"		TASK_CROW_WAIT_FOR_BARNACLE_KILL		0"

		"	Interrupts"
	)


AI_END_CUSTOM_NPC()
