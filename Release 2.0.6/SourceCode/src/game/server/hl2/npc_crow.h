//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef NPC_CROW_H
#define NPC_CROW_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_default.h"
#include "ai_task.h"
#include "ai_condition.h"
#include "cbase.h"
#include "ai_basenpc.h"
#include "smoke_trail.h"
#define BIRDTYPE_CROW 1
#define BIRDTYPE_PIGEON 2
#define BIRDTYPE_SEAGULL 3

//
// Spawnflags.
//
#define SF_CROW_FLYING		16

#define	CROW_EXPLODE_DISTANCE	40
#define CROW_PREFERRED_HEIGHT	16
#define CROW_HEIGHT_GAIN_DISTANCE 320 // try to gain height when more than this far away from target
#define SEEK_INTERVAL			0.2f

//
// Custom schedules.
//
enum
{
	SCHED_CROW_IDLE_WALK = LAST_SHARED_SCHEDULE,
	SCHED_CROW_IDLE_STAND,
	SCHED_CROW_IDLE_FLY,

	//
	// Various levels of wanting to get away from something, selected
	// by current value of m_nMorale.
	//
	SCHED_CROW_WALK_AWAY,
	SCHED_CROW_RUN_AWAY,
	SCHED_CROW_HOP_AWAY,
	SCHED_CROW_FLY_AWAY,

	SCHED_CROW_EXPLODE,
	SCHED_CROW_FLY,
	SCHED_CROW_FLY_FAIL,
	SCHED_CROW_START_TO_FLY_KAMAKAZE,
	SCHED_CROW_KAMAKAZE,

	SCHED_CROW_BARNACLED,
};


//
// Custom tasks.
//
enum 
{
	TASK_CROW_FIND_FLYTO_NODE = LAST_SHARED_TASK,
	//TASK_CROW_PREPARE_TO_FLY,
	TASK_CROW_TAKEOFF,
	//TASK_CROW_LAND,
	TASK_CROW_FLY,
	TASK_CROW_FLY_TO_HINT,
	TASK_CROW_PICK_RANDOM_GOAL,
	TASK_CROW_PICK_EVADE_GOAL,
	TASK_CROW_HOP,
	TASK_CROW_EXPLODE,
	TASK_CROW_COO,
	TASK_CHECK_FOR_ENEMY,
	TASK_FLY_KAMAKAZE,
	TASK_FIX_ANGLES,

	TASK_CROW_FALL_TO_GROUND,
	TASK_CROW_PREPARE_TO_FLY_RANDOM,

	TASK_CROW_WAIT_FOR_BARNACLE_KILL,
};


//
// Custom conditions.
//
enum
{
	COND_CROW_ENEMY_TOO_CLOSE = LAST_SHARED_CONDITION,
	COND_CROW_ENEMY_WAY_TOO_CLOSE,
	COND_CROW_FORCED_FLY,
	COND_CROW_BARNACLED,
	COND_CROW_NEW_TARGET,
};

enum FlyState_t
{
	FlyState_Walking = 0,
	FlyState_Flying,
	FlyState_Falling,
	FlyState_Landing,
};


//-----------------------------------------------------------------------------
// The crow class.
//-----------------------------------------------------------------------------
class CNPC_Crow : public CAI_BaseNPC
{
	DECLARE_CLASS( CNPC_Crow, CAI_BaseNPC );

public:
#ifdef USE_OMNIBOT
	int GetOmnibotClass() const;
#endif

	//
	// CBaseEntity:
	//
	CNPC_Crow(void) {}
	virtual void Spawn( void );
	virtual void Precache( void );

	virtual Vector BodyTarget( const Vector &posSrc, bool bNoisy = true );

	virtual int DrawDebugTextOverlays( void );

	//
	// CBaseCombatCharacter:
	//
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual bool CorpseGib( const CTakeDamageInfo &info );
	bool	BecomeRagdollOnClient( const Vector &force );
	virtual void Event_Killed(const CTakeDamageInfo &info);

	//
	// CAI_BaseNPC:
	//
	virtual float MaxYawSpeed( void ) { return 120.0f; }
	
	virtual Class_T Classify( void );

	virtual void HandleAnimEvent( animevent_t *pEvent );
	virtual int GetSoundInterests( void );
	virtual int SelectSchedule( void );
	virtual void StartTask( const Task_t *pTask );
	virtual void RunTask( const Task_t *pTask );
//	virtual void RunAI();

	virtual bool HandleInteraction( int interactionType, void *data, CBaseCombatCharacter *sourceEnt );

	virtual void OnChangeActivity( Activity eNewActivity );

	virtual bool OverrideMove( float flInterval );

	virtual bool FValidateHintType( CAI_Hint *pHint );
	virtual Activity GetHintActivity( short sHintType, Activity HintsActivity );

	virtual void PainSound( const CTakeDamageInfo &info );
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual void IdleSound( void );
	virtual void AlertSound( void );
	virtual void StopLoopingSounds( void );
	virtual void UpdateEfficiency( bool bInPVS );

	virtual bool QueryHearSound( CSound *pSound );

	void InputFlyAway( inputdata_t &inputdata );
	
	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	void StartTargetHandling( CBaseEntity *pTargetEnt );

	void Explode();

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

	int			m_iBirdType;
	bool		m_bOnJeep;

protected:
	void SetFlyingState( FlyState_t eState );
	inline bool IsFlying( void ) const { return GetNavType() == NAV_FLY; }

	void Takeoff( const Vector &vGoal );
	void FlapSound( void );

	void MoveCrowFly( float flInterval );
	bool Probe( const Vector &vecMoveDir, float flSpeed, Vector &vecDeflect );

	bool IsDeaf() { return m_bIsDeaf; }
	CBaseEntity *BestEnemy();

protected:
	float m_flGroundIdleMoveTime;
	int m_nMorale;				// Used to determine which avoidance schedule to pick. Degrades as I pick avoidance schedules.
	
	bool m_bReachedMoveGoal;

	float m_flHopStartZ;		// Our Z coordinate when we started a hop. Used to check for accidentally hopping off things.

	bool		m_bPlayedLoopingSound;

private:

	Activity NPC_TranslateActivity( Activity eNewActivity );

	float				m_flSoarTime;
	bool				m_bSoar;
	Vector				m_vLastStoredOrigin;
	float				m_flLastStuckCheck;
	
	float				m_flDangerSoundTime;

	float				m_flLastRecalcEnemyTime;

	Vector				m_vDesiredTarget;
	Vector				m_vCurrentTarget;
	Vector				m_vFakeHintNode;
	bool				hasFakeHintNode;

	bool				m_bIsDeaf;
};

//-----------------------------------------------------------------------------
// Purpose: Seagull. Crow with a different model.
//-----------------------------------------------------------------------------
class CNPC_Seagull : public CNPC_Crow
{
	DECLARE_CLASS( CNPC_Seagull, CNPC_Crow );

public:
	
	void Spawn( void )
	{
		SetModelName( AllocPooledString("models/seagull.mdl") );
		BaseClass::Spawn();

		m_iBirdType = BIRDTYPE_SEAGULL;
	}

	void PainSound( const CTakeDamageInfo &info )
	{
		EmitSound( "NPC_Seagull.Pain" );
	}

	void DeathSound( const CTakeDamageInfo &info )
	{
		EmitSound( "NPC_Seagull.Pain" );
	}

	void IdleSound( void )
	{
		EmitSound( "NPC_Seagull.Idle" );
	}
};

//-----------------------------------------------------------------------------
// Purpose: Pigeon. Crow with a different model.
//-----------------------------------------------------------------------------
class CNPC_Pigeon : public CNPC_Crow
{
	DECLARE_CLASS( CNPC_Pigeon, CNPC_Crow );

public:
	void Spawn( void )
	{
		SetModelName( AllocPooledString("models/pigeon.mdl") );
		BaseClass::Spawn();

		m_iBirdType = BIRDTYPE_PIGEON;
	}

	void IdleSound( void )
	{
		EmitSound( "NPC_Pigeon.Idle" );
	}
};

#endif // NPC_CROW_H