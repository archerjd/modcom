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

#ifndef HL2MP_GAMERULES_H
#define HL2MP_GAMERULES_H
#pragma once

#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "gamevars_shared.h"
#include "modcom/mc_shareddefs.h"

#ifndef CLIENT_DLL
#include "hl2mp_player.h"
#include "modcom/monsters.h"
#include "doors.h" // for teleport blocker, i put it here for some reason
#endif

#define VEC_CROUCH_TRACE_MIN	HL2MPRules()->GetHL2MPViewVectors()->m_vCrouchTraceMin
#define VEC_CROUCH_TRACE_MAX	HL2MPRules()->GetHL2MPViewVectors()->m_vCrouchTraceMax
/*
enum
{
	TEAM_COMBINE = 2,
	TEAM_REBELS,
};
*/

enum VoteType_t
{
	VOTE_NONE = 0,
	VOTE_ALLOW_EARLY_SELECTION,
	VOTE_GAMEMODE_PART1,
	VOTE_GAMEMODE_PART2,
	VOTE_GAMEMODE_SINGLE_STEP,
	VOTE_GAMEMODE_YESNO,		// change to specific mode. Yes / no?
	VOTE_MAP,
	FORCE_EARLY_MAP_CHANGE,
};
#define MAX_VOTE_STEPS 5
#define MAX_VOTE_OPTIONS NUM_GAME_MODES
#define MAX_MAP_NOMINATIONS 4 // don't want anything more than a 4-way map vote

#ifdef CLIENT_DLL
	#define CHL2MPRules C_HL2MPRules
	#define CHL2MPGameRulesProxy C_HL2MPGameRulesProxy
#else
	class CTeleportBlocker;
#endif

class CHL2MPGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CHL2MPGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

class HL2MPViewVectors : public CViewVectors
{
public:
	HL2MPViewVectors( 
		Vector vView,
		Vector vHullMin,
		Vector vHullMax,
		Vector vDuckHullMin,
		Vector vDuckHullMax,
		Vector vDuckView,
		Vector vObsHullMin,
		Vector vObsHullMax,
		Vector vDeadViewHeight,
		Vector vCrouchTraceMin,
		Vector vCrouchTraceMax ) :
			CViewVectors( 
				vView,
				vHullMin,
				vHullMax,
				vDuckHullMin,
				vDuckHullMax,
				vDuckView,
				vObsHullMin,
				vObsHullMax,
				vDeadViewHeight )
	{
		m_vCrouchTraceMin = vCrouchTraceMin;
		m_vCrouchTraceMax = vCrouchTraceMax;
	}

	Vector m_vCrouchTraceMin;
	Vector m_vCrouchTraceMax;	
};

class CHL2MPRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CHL2MPRules, CTeamplayRules );

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
#endif
	
	CHL2MPRules();
	virtual ~CHL2MPRules();

	virtual void Precache( void );
	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );
	virtual bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );

	virtual float FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon );
	virtual float FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon );
	virtual int WeaponShouldRespawn( CBaseCombatWeapon *pWeapon );
	virtual void Think( void );
	virtual void CreateStandardEntities( void );
	virtual void ClientSettingsChanged( CBasePlayer *pPlayer );
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual void GoToIntermission( void );
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual const char *GetGameDescription( void );
	// derive this function if you mod uses encrypted weapon info files
	virtual const unsigned char *GetEncryptionKey( void ) { return (unsigned char *)"x9Ke0BY7"; }
	virtual const CViewVectors* GetViewVectors() const;
	const HL2MPViewVectors* GetHL2MPViewVectors() const;

	float GetMapRemainingTime();
	void CleanUpMap();
	void CheckRestartGame();
	void RestartGame();
	virtual bool IsFriendly( CBaseEntity *pChecker, CBaseEntity *pTarget );

#ifndef CLIENT_DLL	

	virtual Vector VecItemRespawnSpot( CItem *pItem );
	virtual QAngle VecItemRespawnAngles( CItem *pItem );
	virtual float	FlItemRespawnTime( CItem *pItem );
	virtual bool	CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem );
	virtual bool FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon );
	virtual bool FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );

	void	AddLevelDesignerPlacedObject( CBaseEntity *pEntity );
	void	RemoveLevelDesignerPlacedObject( CBaseEntity *pEntity );
	void	ManageObjectRelocation( void );
	void    CheckChatForReadySignal( CHL2MP_Player *pPlayer, const char *chatmsg );
	const char *GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer );

	void InitDefaultAIRelationships( void );
	void AwardExpForKill( CBaseCombatCharacter *pVictim, const CTakeDamageInfo &info );
	int CalculateExperience( CHL2MP_Player *pKiller, CBaseCombatCharacter *pVictim, float flPercent = 1.0f );

	bool IsCorrectPassword(int accountID, const char *testPassword);
	int GetAccountID(const char *accountName);
	int AddUser(const char *logon, const char *pass);
	CHL2MP_Player *GetPlayerByAccountID(int accountID);

	int GetNumCharacters(CHL2MP_Player *pPlayer);
	bool PlayerOwnsCharacter(CHL2MP_Player *pPlayer, int characterID); // is this player allowed to edit this character?
	const char *GetCharacterName(int characterID);
	const char *GetCharacterModel(int characterID, int faction=FACTION_NONE);
	int GetCharacterLevel(int characterID);
	int AddNewCharacter(CHL2MP_Player *pPlayer, const char *name, const char *model, int faction, bool bLoadThisCharacter=false);
	void DeleteCharacter(CHL2MP_Player *pPlayer, int characterID);
	void ApplyDataToPlayer(CHL2MP_Player *pPlayer, int CharacterID);
	void SavePlayerData(CHL2MP_Player *pPlayer);

	void DeleteAllPlayerData();

	// pvm functions
	void SpawnMonsterWave();
	void SpawnMonsterForEachPlayer(CNPCTypeInfo *typeInfo, int num=1);
	bool SpawnMonster(CNPCTypeInfo *typeInfo, CHL2MP_Player *pPlayer, int num=1);
	bool SpawnMonster(CNPCTypeInfo *typeInfo, Vector origin, QAngle angles, CHL2MP_Player *pTarget=NULL);
	void CC_Dev_CreateNPC( const CCommand& args );
	CBaseEntity *GetNpcSpawnPoint(bool largeSpawnOnly=false);
	CNPCTypeInfo *GetRandomMonsterType();
	void GetPlayerLevelDistribution(int &min, int &max, int &mean); // all params are outputs
	CHL2MP_Player *PickPlayerWithLeastMonstersTargetting();
	int CalculateMonsterLevelForPlayer(CHL2MP_Player *pPlayer);
	int NumMonsters(CHL2MP_Player *pPlayer=NULL);
	void ResetPVM();

	int GetNumBossNpcs(){ return m_iNumBossNpcs; }
	void BossNpcAdded() { m_iNumBossNpcs++; }
	void BossNpcRemoved() { m_iNumBossNpcs--; }
	
	// spree functions
	void StartSpreeWar(CHL2MP_Player *pSpreer);
	void EndSpreeWar(bool alsoEndAllSprees=false);

	virtual void LevelShutdown( void );

	Vector GetRandomSpawnPoint(Vector hullMin, Vector hullMax, int triesLeft=-1, CBaseEntity *pEntIgnore=NULL);
	QAngle GetRandomAngle();

	KeyValues *GetMapData() { return mapData; }
	int m_iNextKillID;
#endif
	bool IsInSpreeWar() { return m_hSpreer != NULL; }
	CBaseEntity *GetSpreer() { if ( !IsInSpreeWar() ) return NULL; return m_hSpreer.Get(); }

	virtual void ClientDisconnected( edict_t *pClient );

	bool CheckGameOver( void );
	bool IsIntermission( void );
	bool IsInVote( void ) { return m_bInVote; }

	void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );

	
	bool	IsTeamplay( void );
	void	CheckAllPlayersReady( void );
	int		NumPlayers();
	int		NumPlayersOnFaction(int faction);
	bool	IsFactionChangeAllowed(int fromFaction, int toFaction);

	float GetFactionExperience(int faction);

	int GetMaxLevel() { return m_iMaxLevel; }

#ifndef CLIENT_DLL
	void PrepareVoteSequence(bool isSpecificModeRequest, CHL2MP_Player *pStartingPlayer);
	void Vote(CHL2MP_Player *pPlayer, int value);
	bool Nominate(CHL2MP_Player *pPlayer, const char* szValue);
	void RunNextVote();
	void VoteFinished();
	int NumQueuedVotes();

	int		NumTeleportBlockers() { return m_pTeleportBlockers.Count(); }
	CTeleportBlocker *GetTeleportBlocker(int num) { return m_pTeleportBlockers[num]; }
	
	void	AddGravityWell(CBaseEntity *e);
	void	RemoveGravityWell(CBaseEntity *e);

	int GetTotalNumScoreTokens() { return m_iNumTotalScoreTokens; }
	void AdjustTotalNumScoreTokens(int adjustment) { m_iNumTotalScoreTokens += adjustment; }
	int GetTargetScoreTokenLimit();
	void AdjustFactionScoreTokenCount(int factionScored, int adjustment);
	void HoarderGameFinished(int winningFaction);
	void RemoveAllPlayerScoreTokens();

	const char *GetGameID() { return m_szGameID; }
	
	void	PlayerEnteredCharacter(CHL2MP_Player *pPlayer);
	void	PlayerLeftCharacter(CHL2MP_Player *pPlayer);

	void	AwardFactionExp(CHL2MP_Player *pPlayer, int amount);
	void	AdjustFactionCount(int faction, int adjustment);
	CUtlVector<CTeleportBlocker*> *GetTeleportBlockers() { return &m_pTeleportBlockers; }
#else
	CBaseEntity *GetCachedSpreer() { return m_hCachedSpreer.Get(); }
	void SetCachedSpreer(CBaseEntity *pEnt) { m_hCachedSpreer = pEnt; }

	int GetRememberedGameMode() { return m_iRememberGameMode; }
	void SetRememberedGameMode(int i) { m_iRememberGameMode = i; }
#endif
	bool ShouldUseScoreTokens();
	int GetFactionScoreTokenCount(int faction);
	int GetFactionScoreTokenTarget(int faction);

	CBaseCombatCharacter *GetGravityWell(int i);
	const char *GetFactionName(int faction);

	bool GameModeUsesMonsters(int mode);
	bool GameModeUsesTeamplay(int mode);
	
private:
	
	CNetworkVar( float, m_flGameStartTime );
	CUtlVector<EHANDLE> m_hRespawnableItemsAndWeapons;
	float m_tmNextPeriodicThink;
	float m_flRestartGameTime;
	bool m_bCompleteReset;
	bool m_bAwaitingReadyRestart;
	bool m_bHeardAllPlayersReady;
	CNetworkVar( bool, m_bInVote );
	
	CNetworkVar(float, m_flExpFactionCombine);
	CNetworkVar(float, m_flExpFactionResistance);
	CNetworkVar(float, m_flExpFactionAperture);

	CNetworkVar(int, m_iMaxLevel);

	CNetworkArray(int, m_iFactionScoreTokens, NUM_FACTIONS);

#ifndef CLIENT_DLL
	// Vote vars
	int m_iVotes[MAX_VOTE_OPTIONS], m_iTotalVoters;		//vote counters
	float m_flVoteEndTime;
	char m_szGameID[64]; // unique ID of this game's entry in the database

	VoteType_t m_VoteSequence[MAX_VOTE_STEPS];

	int m_iNumCombinePlayers, m_iNumResistancePlayers, m_iNumAperturePlayers;

	int m_iNumTotalScoreTokens; // for "hoarder" game mode
	float m_flNextScoreTokenSpawnTime;

	// pvm vars
	float m_flNextWave;
	float m_flNextBossWave;
	float m_flNextGameStatsCommit;

	CUtlVector<CBaseEntity*> m_pNpcSpawns;
	CUtlVector<CBaseEntity*> m_pLargeNpcSpawns;
	CUtlVector<CTeleportBlocker*> m_pTeleportBlockers;

	KeyValues *mapData;

	int m_iNumBossNpcs;
#endif
	int m_iRememberGameMode; // so we can catch it turning on / off, or announce that players are / aren't allies ... also used on client
		
	CNetworkArray( EHANDLE, m_hGravityWells, MAX_MAGMINES ); // networking these for prediction
	CNetworkHandle(CBaseEntity, m_hSpreer); // its actually an HL2MP player, but it complains it cant convert otherwise
	EHANDLE m_hCachedSpreer;
};

inline CHL2MPRules* HL2MPRules()
{
	return static_cast<CHL2MPRules*>(g_pGameRules);
}

#endif //HL2MP_GAMERULES_H
