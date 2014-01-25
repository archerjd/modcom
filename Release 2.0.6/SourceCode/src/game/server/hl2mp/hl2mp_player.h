//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef HL2MP_PLAYER_H
#define HL2MP_PLAYER_H
#pragma once

class CHL2MP_Player;

#include "basemultiplayerplayer.h"
#include "hl2_playerlocaldata.h"
#include "hl2_player.h"
#include "simtimer.h"
#include "soundenvelope.h"
#include "hl2mp_player_shared.h"
#include "hl2mp_gamerules.h"
#include "utldict.h"
#include "modcom/mc_shareddefs.h"
#include "modcom/modules.h"
#include "modcom/player_limited_quantities.h"
#include "util/sqlite/dbhandler.h"
#include "recipientfilter.h"

extern int ExpForLevelUp( int level );
extern int TotExpForLevelUp( int level );

#define BITS_CHANGED_CHARACTER			0x0001
#define BITS_CHANGED_MODULES			0x0002
#define BITS_CHANGED_LEAVING_CHAR		0x0004
#define BITS_CHANGED_LOGGING_OFF		0x0008

//=============================================================================
// >> HL2MP_Player
//=============================================================================
class CHL2MPPlayerStateInfo
{
public:
	HL2MPPlayerState m_iPlayerState;
	const char *m_pStateName;

	void (CHL2MP_Player::*pfnEnterState)();	// Init and deinit the state.
	void (CHL2MP_Player::*pfnLeaveState)();

	void (CHL2MP_Player::*pfnPreThink)();	// Do a PreThink() in this state.
};

#define MINION_FORMATION_LIMIT 8
#define SERIALIZED_MODULE_LENGTH	MAX_DB_STRING
#define SERIALIZED_WEAPON_UPGRADE_LENGTH	MAX_DB_STRING

// Struct that contains information about attacks (used for 2fers and shared XP) 
struct playerattacks_t
{
	float flDamage; // damage dealt
	EHANDLE hVictim; // the entity we hurt
	//int iDamageType; // damage type
	//int iHitGroup; // hit box hit (in source, this usually only applies to bullet damage)
	//float flTime; // time the damage took place

	playerattacks_t()
	{
		flDamage = 0.0f;
		hVictim = NULL;
		//iDamageType = DMG_GENERIC;
		//flTime = gpGlobals->curtime;
		//iHitGroup = HITGROUP_GENERIC;
	}
};

class CHL2MP_Player : public CHL2_Player
{
public:
	DECLARE_CLASS( CHL2MP_Player, CHL2_Player );

	CHL2MP_Player();
	~CHL2MP_Player( void );
	
	static CHL2MP_Player *CreatePlayer( const char *className, edict_t *ed )
	{
		CHL2MP_Player::s_PlayerEdict = ed;
		return (CHL2MP_Player*)CreateEntityByName( className );
	}

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void InitialSpawn( void );
	virtual void Spawn( void );
	virtual entity_effect_t GetSpawnEffect() { return SPAWN_HUMAN; }

	virtual void PostThink( void );
	virtual void PreThink( void );
	virtual void PlayerDeathThink( void );
	virtual void SetAnimation( PLAYER_ANIM playerAnim );
	virtual bool ClientCommand( const CCommand &args );
	virtual bool HandleCommand_JoinTeam( int team );
	virtual void CreateViewModel( int viewmodelindex = 0 );
	virtual bool BecomeRagdollOnClient( const Vector &force );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void RemoveActiveStuff();
	virtual int OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual bool WantsLagCompensationOnEntity( const CBaseEntity *pEntity, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const;
	virtual void FireBullets ( const FireBulletsInfo_t &info );
	virtual bool Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0);
	virtual bool BumpWeapon( CBaseCombatWeapon *pWeapon );
	virtual void ChangeTeam( int iTeam );
	virtual void PickupObject ( CBaseEntity *pObject, bool bLimitMassAndSize );
	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
	virtual void Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget = NULL, const Vector *pVelocity = NULL );
	virtual void UpdateOnRemove( void );
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual CBaseEntity* EntSelectSpawnPoint( void );
	
	virtual float GetSprintSpeed() { return m_iSprintSpeed; }
	virtual void SetSprintSpeed(int i) { m_iSprintSpeed = i; }
	
	int FlashlightIsOn( void );
	void FlashlightTurnOn( void );
	void FlashlightTurnOff( void );
	void PrecacheFootStepSounds( void );

	QAngle GetAnimEyeAngles( void ) { return m_angEyeAngles.Get(); }

	Vector GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget = NULL );

	void CheatImpulseCommands( int iImpulse );
	void CreateRagdollEntity( void );
	void GiveAllItems( void );
	void GiveDefaultItems( void );

	void NoteWeaponFired( void );

	void ResetAnimation( void );
	Activity TranslateTeamActivity( Activity ActToTranslate );
	
	float GetNextModelChangeTime( void ) { return m_flNextModelChangeTime; }
	float GetNextTeamChangeTime( void ) { return m_flNextTeamChangeTime; }
	void  PickDefaultSpawnTeam( void );
	
	static int DetermineModelFaction( const char *pModel );
	static const char *GetRandomModelForFaction( int faction );
	virtual void SetModel( const char *szModelName );
	void  SetupPlayerSoundsByModel( const char *pModelName );
	const char *GetPlayerModelSoundPrefix( void );
	int	  GetPlayerModelType( void ) { return m_iPlayerSoundType;	}
	
	void  DetonateTripmines( bool playSound );

	void Reset();

	bool IsReady();
	void SetReady( bool bReady );

	void CheckChatText( char *p, int bufsize );

	void State_Transition( HL2MPPlayerState newState );
	void State_Enter( HL2MPPlayerState newState );
	void State_Leave();
	void State_PreThink();
	CHL2MPPlayerStateInfo *State_LookupInfo( HL2MPPlayerState state );

	void State_Enter_ACTIVE();
	void State_PreThink_ACTIVE();
	void State_Enter_OBSERVER_MODE();
	void State_PreThink_OBSERVER_MODE();

	virtual int GetPlayerIndex() { return entindex(); }

	virtual bool StartObserverMode( int mode );
	virtual void StopObserverMode( void );

	Vector m_vecTotalBulletForce;	//Accumulator for bullet force in a single frame

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 

	virtual bool	CanHearAndReadChatFrom( CBasePlayer *pPlayer );

	// abilities
	virtual void BuyModule(Module *a);
	virtual void UseModule(Module *a); // do / start / stop, as appropriate
	virtual void StopModule(Module *a); // this has to be public so that firing weapons can stop cloaking

	// Level & Exp Functions
	int GetTotalExp() { return m_iTotalExp; }
	int GetGameExp() { return m_iGameExp; }
	void AddExp( int add );
	void ResetGameExp() { m_iGameExp = 0; };

	void CheckLevel();
	void LevelUp();
	const char *GetName( void ) { return GetPlayerName(); }

	void AddModulePoints( int add = 1 ){ m_iModulePoints += add; }
 	int GetModulePoints() { return m_iModulePoints; }
	void SetModulePoints(int ap) { m_iModulePoints = ap; }

	// innate aux recharge per second (based on recharge module)
	float GetAuxRechargeRate();

	// functions used restoring player data
	void SetLevelAndExp(int level, int exp) { SetLevel(level); m_iTotalExp = exp; }
	void SetStats(int playerKills, int playerDeaths, int monsterKills, int monsterDeaths, int sprees, int spreeWars, int bestSpree, float timePlayed, int progression)
	{
		m_iPlayerKills = playerKills; m_iPlayerDeaths = playerDeaths;
		m_iMonsterKills = monsterKills; m_iMonsterDeaths = monsterDeaths;
		m_iSprees = sprees; m_iSpreeWars = spreeWars; m_iBiggestSpree = bestSpree;
		m_flPrevTimePlayed = timePlayed; m_iProgressionModel = progression;

		m_iGamePlayerKills = m_iGamePlayerDeaths = m_iGameMonsterKills = m_iGameMonsterDeaths = 0;
	}

	// this game only
	int GetPlayerKills() { return m_iGamePlayerKills; }
	int GetPlayerDeaths() { return m_iGamePlayerDeaths; }
	int GetMonsterKills() { return m_iGameMonsterKills; }
	int GetMonsterDeaths() { return m_iGameMonsterDeaths; }

	// Ever!
	int GetStatPlayerKills() { return m_iPlayerKills; }
	int GetStatPlayerDeaths() { return m_iPlayerDeaths; }
	int GetStatMonsterKills() { return m_iMonsterKills; }
	int GetStatMonsterDeaths() { return m_iMonsterDeaths; }
	int GetStatNumSprees() { return m_iSprees; }
	int GetStatNumSpreeWars() { return m_iSpreeWars; }
	int GetStatBestSpree() { return m_iBiggestSpree; }
	int GetProgressionModel() { return m_iProgressionModel; }

	void NoteKilledPlayer() { m_iPlayerKills ++; m_iGamePlayerKills ++; }
	void NoteKilledByPlayer() { m_iPlayerDeaths ++; m_iGamePlayerDeaths ++; }
	void NoteKilledMonster() { m_iMonsterKills ++; m_iGameMonsterKills ++; }
	void NoteKilledByMonster() { m_iMonsterDeaths ++; m_iGameMonsterDeaths ++; }

	// aux power accessors
	virtual float GetAuxPower() { return SuitPower_GetCurrentPercentage(); }
	virtual void DrainAuxPower(float f) { SuitPower_Drain(f); }
	virtual void AddAuxPower(float f) { SuitPower_Charge(f); }
	virtual void SetAuxPower(float f) { SuitPower_SetCharge(f); }
	virtual float GetMaxAuxPower() { return m_iMaxAuxPower; }
	virtual void SetMaxAuxPower(int i) { m_iMaxAuxPower = i; }
	
	virtual bool ShouldCollide( int collisionGroup, int contentsMask, int playerIndex ) const;
	int NumAlliesInRange();

	// minion formation handling
	int AddMinionToFormation(CAI_BaseNPC *pMinion);
	void RemoveMinionFromFormation(CAI_BaseNPC *pMinion);
	int NumMinionsInFormation();
	void ForceMinionCount(int num) { m_iNumMinions = num; }
	Vector GetFormationPosition(CAI_BaseNPC *pMinion);	
	bool MinionsOrderedToPosition() { return m_vecMinionTarget != vec3_origin; }
	int MinionStance() { return m_iMinionStance; }
	virtual void PlayerUse ( void );

	// damage trackers, for eg Headcrab Revenge (although thats not serious)
	bool WasHurtThisFrame() { return m_bJustBeenHurt; }
	CBaseEntity *GetLastAttacker() { return m_pMyLastAttacker; }
	
	// damage scalers, for Strength, Damage Amp, Weaken
	virtual void ScaleDamageDealt(float f);
	virtual void ScaleDamageReceived(float f);

	// get the most likely thing I'm aiming at
	virtual CBaseEntity *GetAimTarget(bool bFriendly = false);

	// how many times have i killed without being killed myself?
	int GetSpree() { return m_iSpreeLength; }
	void ResetSpree();
	void IncrementSpree();


	// logon control
	int GetAccountID() { return m_iAccountID; }
	int GetCharacterID() { return m_iCharacterID; }
	bool IsLoggedOn() { return m_iLoggedOn > 0; }
	bool IsInCharacter() { return m_iLoggedOn > 1; }
	void SetAccount(int accountID);
	const char *GetCharacterName();
	void SetCharacterName(const char *name);
	
	void EnterCharacter(int characterID);
	void LeaveCharacter(bool bEndOfGame = false);
	void LogOff(bool bEndOfGame = false);
	int GetFaction() { return m_iFaction; }
	void SetFaction(int faction) { m_iFaction = faction; }
	float GetLastPlayerKillTime() { return m_flLastPlayerKillTime; }
	void SetLastPlayerKillTime(float f) { m_flLastPlayerKillTime = f; }
	
	void ShowCharacterSelection(int page=0);
	void PlayDeathTaunt();
	void PlayWelcomeMessage();

	bool CanVote() { return m_bCanVote; }
	void AllowVote(bool val) { m_bCanVote = val; }

	int GetChangedBits() { return m_iChangedBits; }
	void ClearChangedBits() { m_iChangedBits = 0; }

	LimitedQuantities *GetLimitedQuantities() { return limitedQuantities; }
	
	virtual void AddScoreToken(int num=1);
	virtual bool RemoveScoreToken(int num=1);

	float TimePlayed()
	{
		return m_flPrevTimePlayed + (gpGlobals->curtime - m_flPlayStart);
	}

	float GameTimePlayed()
	{
		return gpGlobals->curtime - m_flPlayStart;
	}

	float GetFactionalDamageScaleBoost(); // its just easier having this here than re-doing all the code

	// damage logging (and shared experience)
	virtual void RegisterAttackOnEntity( CTakeDamageInfo &info, CBaseCombatCharacter *pVictim, bool isKillingBlow );

	// Damage given to stuff storage.
	CUtlVector< playerattacks_t > m_DamageGivenThisLife; // this is purged each spawn

#ifdef USE_OMNIBOT
	inline bool IsOmnibot() const { return m_bIsOmnibot; }
	inline void SetIsOmnibot(bool b) { m_bIsOmnibot = b; }
#endif

	void SerializeModuleData();
	const char *GetSerializedModules();
	void SetSerializedModuleData(const char *data);

protected:
	virtual bool DoModule(Module *a, bool isEnqueued=false);
	virtual bool StartModule(Module *a);
	virtual bool EnqueueModule(Module *a);

private:
	// account control
	char m_szCharacterName[28];
	int m_iAccountID, m_iCharacterID;
	CNetworkVar(int, m_iLoggedOn);

	int m_iChangedBits;

	float	m_flNextSaveTime, m_flNextFactionBenefit, m_flPrevTimePlayed, m_flPlayStart;
	int		m_iPlayerKills, m_iPlayerDeaths, m_iMonsterKills, m_iMonsterDeaths, m_iSprees, m_iSpreeWars, m_iBiggestSpree;
	int 	m_iGamePlayerKills, m_iGamePlayerDeaths, m_iGameMonsterKills, m_iGameMonsterDeaths;
	int		m_iProgressionModel;
	bool	m_bCanVote, m_bHasNominated;

	bool m_bJustBeenHurt;
	CBaseEntity *m_pMyLastAttacker;
	int m_iSpreeLength;

	CNetworkVar(int, m_iSprintSpeed);
	CNetworkVar(int, m_iMaxAuxPower);

	CNetworkQAngle( m_angEyeAngles );
	CPlayerAnimState   m_PlayerAnimState;

	int m_iLastWeaponFireUsercmd;
	//int m_iModelType;
	CNetworkVar( int, m_iSpawnInterpCounter );
	CNetworkVar( int, m_iPlayerSoundType );

	float m_flNextModelChangeTime;
	float m_flNextTeamChangeTime;
	float m_flLastPlayerKillTime;

	float m_flSpawnProtectTime, m_flRespawnBlockedTil;
	CNetworkVar(bool, m_bInSpawnProtect);

	HL2MPPlayerState m_iPlayerState;
	CHL2MPPlayerStateInfo *m_pCurStateInfo;

	bool ShouldRunRateLimitedCommand( const CCommand &args );

	// This lets us rate limit the commands the players can execute so they don't overflow things like reliable buffers.
	CUtlDict<float,int>	m_RateLimitLastCommandTimes;

    bool m_bEnterObserver;
	bool m_bReady;
#ifdef USE_OMNIBOT
	bool m_bIsOmnibot;
#endif

	// Level and Experience
	CNetworkVar(int, m_iTotalExp);
	CNetworkVar(int, m_iGameExp);
 	CNetworkVar(int, m_iModulePoints); 

	CNetworkVar(int, m_iFaction);

	// disorientation
//	CNetworkVar(bool, m_bDisorientated);

	CHandle<CAI_BaseNPC> m_hFormationMinions[MINION_FORMATION_LIMIT];
	Vector	m_vecMinionTarget;
	EHANDLE m_hMinionTarget; // for when they're targetting an entity 
	int m_iMinionStance;
	float m_flLastUsePress;
	CNetworkVar(int, m_iNumMinions); // not used on server, just for client calcs
	int m_iNumVotesThisMap; // for vote limiting purposes

	LimitedQuantities *limitedQuantities;
	char m_szSerializedModuleInfo[SERIALIZED_MODULE_LENGTH];
};

inline CHL2MP_Player *ToHL2MPPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<CHL2MP_Player*>( pEntity );
}

#define DECLARE_NAMED_HUDELEMENT( className, panelName )						\
	static CHudElement *Create_##panelName##( void )							\
		{																		\
			return new className( #panelName );									\
		};																		\
	static CHudElementHelper g_##panelName##_Helper( Create_##panelName##, 50 );




#define SHOW_HINT(player,message) {								\
	CSingleUserRecipientFilter user( (CBasePlayer *)player );	\
	user.MakeReliable();										\
	UserMessageBegin( user, "HintText" );						\
		WRITE_BYTE( 0 ),									\
		WRITE_STRING( message );								\
	MessageEnd(); }

#define SHOW_HINT_ALWAYS(player,message) {						\
	CSingleUserRecipientFilter user( (CBasePlayer *)player );	\
	user.MakeReliable();										\
	UserMessageBegin( user, "HintText" );						\
		WRITE_BYTE( 1 );										\
		WRITE_STRING( message );								\
	MessageEnd(); }

#define CLEAR_HINT(player) {						\
	CSingleUserRecipientFilter user( (CBasePlayer *)player );	\
	user.MakeReliable();										\
	UserMessageBegin( user, "HintText" );						\
		WRITE_BYTE( 1 );										\
		WRITE_STRING( "" );										\
	MessageEnd(); }

#define SHOW_HINT_ALL(message) {								\
	CRecipientFilter filter;									\
	filter.AddAllPlayers();										\
	filter.MakeReliable();										\
	UserMessageBegin( filter, "HintText" );						\
		WRITE_BYTE( 0 );									\
		WRITE_STRING( message );								\
	MessageEnd(); }

#define SHOW_HINT_ALWAYS_ALL(message) {						\
	CRecipientFilter filter;									\
	filter.AddAllPlayers();										\
	filter.MakeReliable();										\
	UserMessageBegin( filter, "HintText" );						\
		WRITE_BYTE( 0 );									\
		WRITE_STRING( message );								\
	MessageEnd(); }

#endif //HL2MP_PLAYER_H
