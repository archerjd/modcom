//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hl2mp_gamerules.h"
#include "viewport_panel_names.h"
#include "gameeventdefs.h"
#include <KeyValues.h>
#include "ammodef.h"
#include "hl2_shareddefs.h"
#include "modcom/modules.h"
#include "modcom/mc_shareddefs.h"
#include "modcom/mcconvar.h"
#include "basegrenade_shared.h"
#include <string>

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_baseentity.h"
	#include "engine/IEngineSound.h"
	#include "c_ai_basenpc.h"
	#include "c_playerresource.h"
#else
	#include "eventqueue.h"
	#include "player.h"
	#include "gamerules.h"
	#include "game.h"
	#include "items.h"
	#include "entitylist.h"
	#include "mapentities.h"
	#include "in_buttons.h"
	#include <ctype.h>
	#include "voice_gamemgr.h"
	#include "iscorer.h"
	#include "hl2mp_player.h"
	#include "weapon_hl2mpbasehlmpcombatweapon.h"
	#include "team.h"
	#include "voice_gamemgr.h"
	#include "hl2mp_gameinterface.h"
	#include "hl2mp_cvars.h"
	#include "utlbuffer.h"
	#include "filesystem.h"
	#include "modcom/teleport_blocker.h"

//	#include "modcom/monsters.h"
	#include "util/sqlite/dbhandler.h"
	#include "tier0/icommandline.h"
	#include "engine/IEngineSound.h"
#undef time
	#include <time.h>
#ifdef DEBUG	
	#include "hl2mp_bot_temp.h"
#endif

#define VarArgs UTIL_VarArgs
extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);

ConVar sv_report_client_settings("sv_report_client_settings", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );
#ifndef _WIN32
	#define min(a,b) (a) < (b) ? a : b
	#define max(a,b) (a) > (b) ? a : b
#endif

extern ConVar mp_chattime;

extern CBaseEntity	 *g_pLastCombineSpawn;
extern CBaseEntity	 *g_pLastResistanceSpawn;
extern CBaseEntity	 *g_pLastApertureSpawn;

extern void LoadMonsterStats();

#define WEAPON_MAX_DISTANCE_FROM_SPAWN 64
#define DB_PLAYERDATA	"PlayerData.db"
#define DB_GAMESTATS	"GameStats.db"
#define DB_BACKUP "Backup.db"

extern McConVar gamemode_hoarder_exp_scale, gamemode_hoarder_monster_score_token_expiry_time, gamemode_hoarder_token_add_interval, gamemode_hoarder_victory_exp, mc_player_startlevel, mc_aux_batteries_min, mc_aux_batteries_per_armor_battery, mc_aux_batteries_per_armor_charger, mc_aux_batteries_per_health_charger, mc_aux_batteries_per_health_kit, mc_aux_batteries_per_health_vial, mc_experience_minion, mc_experience_monster, mc_experience_spree_bonus, mc_faction_win_experience, mc_spree_start, mc_spreewar_start, mc_perlevel_modulepoints;

extern float g_flRandomSpawnFrequencySum;

ConVar gamestats_commit_interval("mc_dev_gamestats_commit_interval", "5", FCVAR_GAMEDLL);

extern CUtlVector<CBaseEntity*> g_Nodes;
extern CUtlVector<CNPCTypeInfo*> g_pNPCInfo;
extern float g_flRandomSpawnFrequencySum;

// update this any time there are changes to be made to the database to bring it up to date (either to the data model or to the data itself)
#define LATEST_DATABASE_VERSION	4

ConVar mc_server_update_check("mc_server_update_check", "0", FCVAR_GAMEDLL, "Whether dedicated servers will check for available updates", true, 0, true, 1);
#ifdef RELEASE
	extern void CheckForUpdates();
#endif

#endif

extern McConVar ally_experience_boost_fraction, gamemode_hoarder_respawn_delay_no_tokens, mc_max_level, mc_respawn_delay, pvm_buff_combine_scale, pvm_buff_interval_aperture, pvm_buff_interval_resistance, gamemode_hoarder_min_players, sv_hl2mp_weapon_respawn_time, sv_hl2mp_item_respawn_time;;

ConVar forcerespawn( "mp_forcerespawn","0", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar mc_gamemode("mc_gamemode", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Game mode 1 is Deathmatch, with no monsters\nGame mode 2 is cooperative PvM\nGame mode 3 is free-for-all, or 'PvPvM'\nGame mode 4 is team deathmatch\nGame mode 5 is \"Hoarder\"", true, DEATHMATCH, true, NUM_GAME_MODES );

ConVar mc_vote_duration("mc_vote_duration", "15", FCVAR_NOTIFY | FCVAR_REPLICATED, "The duration of all votes.", true, 1, true, 60 );
ConVar mc_vote_map_enabled("mc_vote_map_enabled", "2", FCVAR_NOTIFY | FCVAR_REPLICATED, "Controls when map votes can occur.\n0: Map never voted for, cycles automatically\n1: Map votes at end of each game, and after any game mode votes. Nominations disabled.\n2: Map votes at end of each game, and after any game mode votes. Nominations enabled.", true, 0, true, 2);
ConVar mc_vote_gamemode_enabled("mc_vote_gamemode_enabled", "2", FCVAR_NOTIFY | FCVAR_REPLICATED, "Controls when game mode votes can occur.\n0: Game mode never voted for\n1: Game mode voted for at end of map only\n2: Game mode voted for at end of map, and can be triggered early if players agree", true, 0, true, 2);
ConVar mc_vote_gamemode_type("mc_vote_gamemode_type", "2", FCVAR_NOTIFY | FCVAR_REPLICATED, "Controls the gamemode vote type.\n1: Single stage vote, shows all game modes as options\n2: Two stage vote, asks whether teamplay wanted then whether monsters wanted", true, 1, true, 2);

//ConVar mc_vote_type_gamemode("mc_vote_type_gamemode", "7", FCVAR_NOTIFY | FCVAR_REPLICATED, "Controls how and when mode votes will be called.\n0: never\n1: flag anytime\n2: flag end of round\n4: flag single (only works if flag 1 is enabled)\n8: flag preliminary vote", true, 0, true, 15 );
//ConVar mc_vote_type_map("mc_vote_type_map", "7", FCVAR_NOTIFY | FCVAR_REPLICATED, "Controls how and when map votes will be called.\n0: never\n1: flag anytime\n2: flag end of round\n4: flag nominations\n8: flag preliminary vote", true, 0, true, 15 );

REGISTER_GAMERULES_CLASS( CHL2MPRules );

BEGIN_NETWORK_TABLE_NOBASE( CHL2MPRules, DT_HL2MPRules )

	#ifdef CLIENT_DLL
		//RecvPropBool( RECVINFO( m_bTeamPlayEnabled ) ),
		RecvPropEHandle( RECVINFO( m_hSpreer ) ),
		RecvPropArray3( RECVINFO_ARRAY(m_hGravityWells), RecvPropEHandle( RECVINFO( m_hGravityWells[0] ) ) ),
		RecvPropBool( RECVINFO( m_bInVote ) ),
		RecvPropFloat( RECVINFO( m_flExpFactionCombine ) ),
		RecvPropFloat( RECVINFO( m_flExpFactionResistance ) ),
		RecvPropFloat( RECVINFO( m_flExpFactionAperture ) ),
		RecvPropFloat( RECVINFO( m_flGameStartTime ) ),
		RecvPropArray3( RECVINFO_ARRAY(m_iFactionScoreTokens), RecvPropInt( RECVINFO( m_iFactionScoreTokens[0] ) ) ),

		RecvPropInt( RECVINFO( m_iMaxLevel ) ),

	#else
		//SendPropBool( SENDINFO( m_bTeamPlayEnabled ) ),
		SendPropEHandle( SENDINFO( m_hSpreer ) ),
		SendPropArray3( SENDINFO_ARRAY3(m_hGravityWells), SendPropEHandle( SENDINFO_ARRAY(m_hGravityWells) ) ),
		SendPropBool( SENDINFO( m_bInVote ) ),
		SendPropFloat( SENDINFO( m_flExpFactionCombine ) ),
		SendPropFloat( SENDINFO( m_flExpFactionResistance ) ),
		SendPropFloat( SENDINFO( m_flExpFactionAperture ) ),
		SendPropFloat( SENDINFO( m_flGameStartTime ) ),
		SendPropArray3( SENDINFO_ARRAY3(m_iFactionScoreTokens), SendPropInt( SENDINFO_ARRAY(m_iFactionScoreTokens), 7, SPROP_UNSIGNED ) ),

		SendPropInt( SENDINFO( m_iMaxLevel ), 7, SPROP_UNSIGNED ),
	#endif

END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( hl2mp_gamerules, CHL2MPGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( HL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )

static HL2MPViewVectors g_HL2MPViewVectors(
	Vector( 0, 0, 64 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-16, -16, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  72 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-16, -16, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  36 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 28 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-16, -16, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 16,  16,  60 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

static const char *s_PreserveEnts[] =
{
	"ai_network",
	"ai_hint",
	"hl2mp_gamerules",
	"team_manager",
	"player_manager",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"func_brush",
	"func_wall",
	"func_buyzone",
	"func_illusionary",
	"infodecal",
	"info_projecteddecal",
	"info_node",
	"info_target",
	"info_node_hint",
	"info_player_deathmatch",
	"info_player_combine",
	"info_player_rebel",
	"info_map_parameters",
	"keyframe_rope",
	"move_rope",
	"info_ladder",
	"player",
	"point_viewcontrol",
	"scene_manager",
	"shadow_control",
	"sky_camera",
	"soundent",
	"trigger_soundscape",
	"viewmodel",
	"predicted_viewmodel",
	"worldspawn",
	"point_devshot_camera",
	"", // END Marker
};

void gamemode_hoarder_min_players_changed( IConVar *var, const char *pOldValue, float flOldValue )
{
#ifndef CLIENT_DLL
	if ( gamemode_hoarder_min_players.GetInt() > flOldValue )
	{// requirement increased. check we still have enough players
		if ( !HL2MPRules()->ShouldUseScoreTokens() )
		{
			SHOW_HINT_ALWAYS_ALL("Hint_ModeHoarder2");
			HL2MPRules()->RemoveAllPlayerScoreTokens();
		}
	}
	else
	{// requirement decreased. see if we now have enough players
		if ( HL2MPRules()->ShouldUseScoreTokens() )
		{
			SHOW_HINT_ALWAYS_ALL("Hint_ModeHoarder1");
			HL2MPRules()->HoarderGameFinished(FACTION_NONE); // give everyone one token and get going
		}
	}
#endif
}

#ifdef CLIENT_DLL
	void RecvProxy_HL2MPRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CHL2MPRules *pRules = HL2MPRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CHL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )
		RecvPropDataTable( "hl2mp_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_HL2MPRules ), RecvProxy_HL2MPRules )
	END_RECV_TABLE()
#else
	void* SendProxy_HL2MPRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CHL2MPRules *pRules = HL2MPRules();
		Assert( pRules );
		return pRules;
	}

BEGIN_SEND_TABLE( CHL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )
	SendPropDataTable( "hl2mp_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_HL2MPRules ), SendProxy_HL2MPRules )
END_SEND_TABLE()

class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
	{
		return ( pListener->GetTeamNumber() == pTalker->GetTeamNumber() );
	}
};
CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

extern INetworkStringTableContainer *networkstringtable;
bool AddDownload(const char *file)
{
	INetworkStringTable *pDownloadablesTable = networkstringtable->FindTable("downloadables");
	if (!pDownloadablesTable)
		return false;

	if (pDownloadablesTable->FindStringIndex(file) == INVALID_STRING_INDEX)
	{
#ifdef DEBUG
		Msg("Adding download: %s\n", file);
#endif
		bool save = engine->LockNetworkStringTables(false);
		pDownloadablesTable->AddString(true, file, strlen(file)+1);
		engine->LockNetworkStringTables(save);
	}
	return true;
}

extern void DoFTPUpload(const char *filePath);
//extern const char *GetServerIP();

int GetNumEntities(const char *classname)
{
	int num = 0;
	CBaseEntity* pPrev = gEntList.FindEntityByClassname(NULL,classname);
	while (pPrev)
	{
		num++;
		pPrev = gEntList.FindEntityByClassname(pPrev,classname);
	}
	return num;
}

bool firstThink;


// Random location function
#define NUM_RANDOM_START_TYPES 4
bool bDisabledRandomSpawnStartTypes[NUM_RANDOM_START_TYPES];
CBaseEntity *pLastRandomSpawnStarts[NUM_RANDOM_START_TYPES];
int iNumRandomStartTypes_OnThisMap = NUM_RANDOM_START_TYPES;
#endif

#ifndef CLIENT_DLL
bool cheatsHaveBeenOnThisGame = false, nonDefaultConvarsHaveBeenOnThisGame = false;
void mc_use_defaults_changed( IConVar *var, const char *pOldValue, float flOldValue );
extern int g_iNextGameMode, g_iNumMapNominations;
extern float g_flNextVoteAllowed;
ConVar mc_use_defaults("mc_use_defaults", "1", FCVAR_REPLICATED|FCVAR_NOTIFY, "When 1, server is forced to use standard gameplay, and relevant convars are locked to their default values. Others can still be changed (such as game mode, etc)", true, 0, true, 1, mc_use_defaults_changed);
#else
ConVar mc_use_defaults("mc_use_defaults", "1", FCVAR_REPLICATED|FCVAR_NOTIFY, "When 1, server is forced to use standard gameplay, and relevant convars are locked to their default values. Others can still be changed (such as game mode, etc)", true, 0, true, 1);
#endif

std::string q1 = "'";
std::string q2 = "''";

std::string Escape(const char *instr)
{
	std::string source = instr;
	std::string::size_type pos = 0;
	while ( (pos = source.find(q1, pos)) != std::string::npos ) {
		source.replace( pos, q1.size(), q2 );
        pos += q2.size();
    }
    return source;
}

CHL2MPRules::CHL2MPRules()
{
#ifndef CLIENT_DLL
	firstThink = true;
	m_iNextKillID = 0;
	if ( g_iNextGameMode != -1 )
	{
		mc_gamemode.SetValue(g_iNextGameMode);
		g_iNextGameMode = -1;
	}
	long sysTime = (long)time(NULL);
	Q_snprintf(m_szGameID, sizeof(m_szGameID), "Stats_%ld_%s_%s", sysTime, cvar->FindVar("hostip")->GetString(), cvar->FindVar("hostport")->GetString() );
	
	if ( filesystem->FileExists(DB_GAMESTATS) ) // game stats db exists from previous game, rename it to something else and decide we want to upload it!
	{// rename file, then flag for upload. Will be deleted when done uploading!
		const char *newFilename = UTIL_VarArgs("%s.db", m_szGameID);
		if ( filesystem->RenameFile(DB_GAMESTATS, newFilename, "MOD") )
			DoFTPUpload(newFilename);
			
		else
			Msg("Game stats rename failed - cannot upload!\n");
	}

#ifdef RELEASE
	if ( mc_server_update_check.GetBool() && engine->IsDedicatedServer() )
		CheckForUpdates();
#endif

	if ( filesystem->FileExists(DB_PLAYERDATA) )
	{// if player database file exists, check that its not corrupt.
		dbHandler *tempDB = new dbHandler(DB_PLAYERDATA);
		const char* output = tempDB->ReadString("PRAGMA quick_check(1)");
		bool corrupt = !FStrEq(output, "ok");
		delete tempDB;
		
		if ( !corrupt )
		{// database is not corrupt, take a backup of it
			if ( filesystem->FileExists(DB_BACKUP) )
				filesystem->RemoveFile(DB_BACKUP);
			
			// copy file, as filesystem doesn't have a CopyFile method
			bool copyOk;
			CUtlBuffer buf;
			if(filesystem->ReadFile(DB_PLAYERDATA, "MOD", buf))
				copyOk = filesystem->WriteFile(DB_BACKUP, "MOD", buf); // Write out copy
			else
				copyOk = false;
			if ( copyOk )
			//if ( filesystem->CopyFile(DB_PLAYERDATA, DB_BACKUP, "MOD") )
				Msg("Player database is healthy and backed up\n");
			else
				Warning("Player database is healthy, but backup failed\n");
		}
		else if ( filesystem->FileExists(DB_BACKUP) )
		{// database is corrupt, restore backup if we have one
			if ( filesystem->RenameFile(DB_BACKUP, DB_PLAYERDATA, "MOD") )
				Warning("Player database corrupted, restoring backup\n");
			else
				Warning("Player database is corrupt, failed to restore backup\n");
		}
		else
			Warning("Player database is corrupt, no backup available!\n");
	}

	if ( g_pDB == NULL )
		g_pDB = new dbHandler(DB_PLAYERDATA);
	if ( g_pDB2 == NULL )
		g_pDB2 = new dbHandler(DB_GAMESTATS);
	//m_bTeamPlayEnabled = teamplay.GetBool();
	m_flIntermissionEndTime = 0.0f;
	m_flGameStartTime = 0;

	m_hRespawnableItemsAndWeapons.RemoveAll();
	m_tmNextPeriodicThink = 0;
	m_flRestartGameTime = 0;
	m_bCompleteReset = false;
	m_bHeardAllPlayersReady = false;
	m_bAwaitingReadyRestart = false;
	m_bInVote = false;
	g_flNextVoteAllowed = 0;
	g_iNumMapNominations = 0;
	m_iNumBossNpcs = 0;
	m_iNumTotalScoreTokens = 0;
	m_flNextScoreTokenSpawnTime = gpGlobals->curtime + gamemode_hoarder_token_add_interval.GetFloat();

	for ( int i=0; i<NUM_FACTIONS; i++ )
		m_iFactionScoreTokens.Set(i, 0);

	m_iMaxLevel = mc_max_level.GetInt();
	
	m_iNumCombinePlayers = m_iNumResistancePlayers = m_iNumAperturePlayers = 0;
	m_flExpFactionCombine = m_flExpFactionResistance = m_flExpFactionAperture = 0;
	InitDefaultAIRelationships();

	// intialise database, create all our tables if they're not already there...

	// handle database versions - if database is on a previous version, update it
	//g_pDB->BeginTransaction();
	g_pDB->Command("CREATE TABLE IF NOT EXISTS DatabaseInfo (version integer)");
	int currentVersion = g_pDB->ReadInt("SELECT version FROM DatabaseInfo");
	//g_pDB->CommitTransaction();

	if ( currentVersion != LATEST_DATABASE_VERSION )
	{
		g_pDB->BeginTransaction();
		// it runs through each of these ... if its v1 and it needs to be v3, it does v1 -> v2 then v2 -> v3, but if its v2 already, it just does v2 -> v3
		switch ( currentVersion )
		{
		case -1:
			g_pDB->Command("create table if not exists Accounts (ID integer primary key asc, Name varchar collate nocase, Pass varchar, Created date, LastActive date)");
			g_pDB->Command("create unique index if not exists IX_Accounts ON Accounts (Name)");
			g_pDB->Command("create table if not exists Characters (ID integer primary key asc, AccountID integer, Name varchar, Faction integer, Level integer, Exp integer, AP integer, Modules varchar, DefaultModel varchar, ResistanceModel varchar, CombineModel varchar, ApertureModel varchar, Created date, LastActive date, PlayerKills integer, PlayerDeaths integer, MonsterKills integer, MonsterDeaths integer, Sprees integer, SpreeWars integer, BestSpree integer, TimePlayed float)");
			g_pDB->Command("create index if not exists IX_Characters ON Characters (AccountID)");
			
			g_pDB->Command("insert into DatabaseInfo (version) values (%i)", LATEST_DATABASE_VERSION);
			break; // skip the updates for a new database
		case 1:
			g_pDB->Command("update Characters set AP = AP + (Level+1) * %i + 12", mc_perlevel_modulepoints.GetInt());
			g_pDB->Command("drop index IX_Modules");
			g_pDB->Command("drop table CharacterModules");
			// seems bizarre to add all these seperately, but never mind.
			g_pDB->Command("alter table Characters add column Modules varchar default ''");
			g_pDB->Command("alter table Characters add column PlayerKills integer default '0'");
			g_pDB->Command("alter table Characters add column PlayerDeaths integer default '0'");
			g_pDB->Command("alter table Characters add column MonsterKills integer default '0'");
			g_pDB->Command("alter table Characters add column MonsterDeaths integer default '0'");
			g_pDB->Command("alter table Characters add column Sprees integer default '0'");
			g_pDB->Command("alter table Characters add column SpreeWars integer default '0'");
			g_pDB->Command("alter table Characters add column BestSpree integer default '0'");
			g_pDB->Command("alter table Characters add column TimePlayed float default '0'");

			g_pDB->Command("update Characters set PlayerKills = 0, PlayerDeaths = 0, MonsterKills = 0, MonsterDeaths = 0, Sprees = 0, SpreeWars = 0, BestSpree = 0, TimePlayed = 0");

			// also delete all old-format pvm script files
			FileFindHandle_t findHandle;
			for (const char *pszName=filesystem->FindFirst("pvm/npc_*.txt",&findHandle); pszName; pszName=filesystem->FindNext(findHandle))
				filesystem->RemoveFile(pszName);

		case 2:
			// module point reset for all characters
			g_pDB->Command("update Characters set Modules = '', AP = Level * %i + 15", mc_perlevel_modulepoints.GetInt());

		case 3:
			// module point reset for all characters ... removed starting module points, so DON'T add the extra 15 to account for those!
			g_pDB->Command("update Characters set Modules = '', AP = Level * %i", mc_perlevel_modulepoints.GetInt());



			g_pDB->Command("update DatabaseInfo set version = %i", LATEST_DATABASE_VERSION);
			g_pDB->CommitTransaction();
			
			g_pDB->BeginTransaction();
			g_pDB->Command("vacuum");
		}

		g_pDB->CommitTransaction();	
	}

	// we  have to recreate the game stats database every time, now
	g_pDB2->BeginTransaction();
	g_pDB2->Command("create table if not exists game (id varchar primary key, gamemode int, combinescore int, rebelscore int, aperturescore int, mapname varchar, cheatsenabled int, defaultconvars int)");
	g_pDB2->Command("create table if not exists gameplayer(gameid varchar, characterid int, faction int, plevel int, modules varchar, weapons varchar, defaultweapon varchar, gameexp int, duration float, playerkills int, playerdeaths int, monsterkills int, monsterdeaths int, rank int)");
	g_pDB2->Command("create table if not exists kills(killid int, gameid varchar, victimid int, victimtype varchar, victimlevel int)");
	g_pDB2->Command("create table if not exists killparts (killid int, gameid varchar, killerid int, killertype varchar, inflictor varchar, damageamount real, iskillingblow int)");
	//g_pDB2->CommitTransaction();

	CreateModules(); // the global list must be populated ... on the client, its done in CHLClient::Init
	
	// load extra map data
	mapData = new KeyValues("MapData");
	char szDataFile[512];
	Q_snprintf(szDataFile,sizeof(szDataFile), "maps/graphs/%s.txt", STRING( gpGlobals->mapname ));
	if ( mapData->LoadFromFile( g_pFullFileSystem, szDataFile, "MOD" ) )
	{
		Msg("Loading map data file...\n");
		AddDownload(szDataFile); // when playing a map on a server, players should auto-download its data file if they don't have it.
		
		// use the graph text files instead of the .ain, cos something was up with that.
		char szNodeGraph[512];
		Q_snprintf(szNodeGraph,sizeof(szNodeGraph), "maps/graphs/%s_graph.txt", STRING( gpGlobals->mapname ));
		AddDownload(szNodeGraph);
		Q_snprintf(szNodeGraph,sizeof(szNodeGraph), "maps/graphs/%s_airgraph.txt", STRING( gpGlobals->mapname ));
		AddDownload(szNodeGraph);

		KeyValues *npcSpawns = mapData->FindKey("NpcSpawns");
		if ( npcSpawns != NULL )
		{
			int num = 0;
			
			// all map info files *should* have NPC spawns... most other stuff is fairly optional
			for ( KeyValues *sub = npcSpawns->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
			{//	sub->GetName(), sub->GetString();
				CUtlVector<char*> floats;
				V_SplitString( sub->GetName(), " ", floats );
				Vector origin( atof( floats[0] ), atof( floats[1] ), atof( floats[2] ) );
				floats.PurgeAndDeleteElements();

				CPointEntity *pEnt = (CPointEntity*)CreateEntityByName("info_npc_start"); // is just a regular CPointEntity
				pEnt->SetAbsOrigin(origin);
				m_pNpcSpawns.AddToTail(pEnt);

				num ++;
			}
			Msg("Read in %i npc spawn points\n",num);
		}
		else
			Warning( "Map data file contains no NPC spawn points\n");

		KeyValues *largeNpcSpawns = mapData->FindKey("LargeNpcSpawns");
		if ( largeNpcSpawns != NULL )
		{
			int num = 0;
			
			// maps don't have to have large npc spawn points, only if they feel its needed.
			for ( KeyValues *sub = largeNpcSpawns->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
			{//	sub->GetName(), sub->GetString();
				CUtlVector<char*> floats;
				V_SplitString( sub->GetName(), " ", floats );
				Vector origin( atof( floats[0] ), atof( floats[1] ), atof( floats[2] ) );
				floats.PurgeAndDeleteElements();

				CPointEntity *pEnt = (CPointEntity*)CreateEntityByName("info_npc_start"); // is just a regular CPointEntity
				pEnt->SetAbsOrigin(origin);
				m_pLargeNpcSpawns.AddToTail(pEnt);

				num ++;
			}
			Msg("Read in %i large npc spawn points\n",num);
		}
		//else
			//Warning( "Map data file contains no large NPC spawn points\n");
		
		// teleporter blockers
		KeyValues *teleportBlockers = mapData->FindKey("TeleportBlockers");
		if ( teleportBlockers != NULL )
		{
			int num = 0;
			for ( KeyValues *sub = teleportBlockers->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
			{//	sub->GetName(), sub->GetString();
				CUtlVector<char*> floats;
				V_SplitString( sub->GetName(), " ", floats );
				Vector origin( atof( floats[0] ), atof( floats[1] ), atof( floats[2] ) );
				float radius = floats.Count() > 3 ? atof( floats[3] ) : 1600; // default them all to 1600 for now, that's what it was at before
				floats.PurgeAndDeleteElements();

				CTeleportBlocker *pEnt = static_cast<CTeleportBlocker*>( CBaseEntity::Create("info_teleport_node", origin, vec3_angle, NULL) );
				pEnt->SetRadius(radius);
				m_pTeleportBlockers.AddToTail(pEnt);

				num ++;
			}
			if ( num > 0 )
				Msg("Read in %i teleport blockers\n",num);
		}
		
		// ... and ultimately player spawn overrides, weapon & ammo spawn overrides...
		
		
		Msg("Map data loading complete\n");
	}
	else
		Warning( "Unable to load map data file: maps/graphs/%s.txt\n", STRING( gpGlobals->mapname ) );

	// setup pvm vars - it must be off at the start of the map, so we can detect whether the map has spawns or not
	m_flNextWave = 0;
	m_hSpreer = NULL;

	for ( int i=0; i<MAX_MAGMINES; i++ )
		m_hGravityWells.Set(i, NULL);

	//g_pDB2->BeginTransaction();
	g_pDB2->Command("insert into game (id, gamemode, rebelscore, combinescore, aperturescore, mapname, cheatsenabled, defaultconvars) values ('%s', %i, 0, 0, 0, '%s', %i, %i)", m_szGameID, mc_gamemode.GetInt(), STRING( gpGlobals->mapname ), sv_cheats->GetInt(), mc_use_defaults.GetInt());
	McConVar::CheckAllDefault(); // this will change update the defaultconvars value if they are not set to 1
	g_pDB2->CommitTransaction();
	cheatsHaveBeenOnThisGame = sv_cheats->GetBool();
	nonDefaultConvarsHaveBeenOnThisGame = mc_use_defaults.GetBool();
		
	// random spawn location stuff setup
	for ( int i=0; i<NUM_RANDOM_START_TYPES; i++ )
	{
		bDisabledRandomSpawnStartTypes[i] = false;
		pLastRandomSpawnStarts[i] = NULL;
	}
	iNumRandomStartTypes_OnThisMap = NUM_RANDOM_START_TYPES;

	PrepareVoteSequence(false, NULL); // set up the end of map votes so we know how many to expect... as that affects how soon before the end we call the first one
	PopulateMapList();
#endif
	m_iRememberGameMode = 0;
}

const CViewVectors* CHL2MPRules::GetViewVectors()const
{
	return &g_HL2MPViewVectors;
}

const HL2MPViewVectors* CHL2MPRules::GetHL2MPViewVectors()const
{
	return &g_HL2MPViewVectors;
}
	
CHL2MPRules::~CHL2MPRules( void )
{
#ifndef CLIENT_DLL
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	//g_Teams.Purge();
	g_Nodes.Purge(); // as above
	//DeleteModules(); // also deletes afflictions and weapon upgrades
#endif
}

#ifndef CLIENT_DLL
extern CNPCTypeInfo *g_pNPCErrorType;
extern CNPCTypeInfo *randomType;

void CHL2MPRules::LevelShutdown()
{
	g_pNPCInfo.PurgeAndDeleteElements();
	if ( g_pNPCErrorType != NULL )
	{
		delete g_pNPCErrorType;
		g_pNPCErrorType = NULL;
	}

	mapData->deleteThis();
	
	m_pNpcSpawns.Purge();
	m_pLargeNpcSpawns.Purge();
	m_pTeleportBlockers.Purge();
}
#endif

void CHL2MPRules::CreateStandardEntities( void )
{
#ifndef CLIENT_DLL
	// Create the entity that will send our data to the client.

	BaseClass::CreateStandardEntities();

	g_pLastCombineSpawn = NULL;
	g_pLastResistanceSpawn = NULL;
	g_pLastApertureSpawn = NULL;

#ifdef _DEBUG
	CBaseEntity *pEnt =  
#endif
	CBaseEntity::Create( "hl2mp_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
#endif
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CHL2MPRules::FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( weaponstay.GetInt() > 0 )
	{
		// make sure it's only certain weapons
		if ( !(pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
		{
			return 0;		// weapon respawns almost instantly
		}
	}

	return sv_hl2mp_weapon_respawn_time.GetFloat();
#endif

	return 0;		// weapon respawns almost instantly
}


bool CHL2MPRules::IsIntermission( void )
{
#ifndef CLIENT_DLL
	return m_flIntermissionEndTime > gpGlobals->curtime;
#endif

	return false;
}

void CHL2MPRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL
	if ( IsIntermission() )
		return;

	BaseClass::PlayerKilled( pVictim, info );
	
	AwardExpForKill(pVictim,info);

	CHL2MP_Player *pVictimPlayer = ToHL2MPPlayer(pVictim);

	pVictimPlayer->PlayDeathTaunt();

	if ( IsInSpreeWar() && pVictimPlayer == GetSpreer() )
		EndSpreeWar();

#endif
}

#ifndef CLIENT_DLL
// separated from playerkilled so it can also give experience for monsters killed
extern LEVEL_EXTERN(mod_bruteforce_buff_stacks);
extern LEVEL_EXTERN(mod_adrenaline_buff_stacks);
void CHL2MPRules::AwardExpForKill( CBaseCombatCharacter *pVictim, const CTakeDamageInfo &info )
{
	if( !pVictim )
		return;

	if ( (info.GetDamageType() & DMG_CLUB) && info.GetAttacker() && info.GetAttacker()->IsPlayer() )
	{// if killed by a player with a club, give them the brute force buff if they have the brute force ability
		CHL2MP_Player *pAttacker = ToHL2MPPlayer( info.GetAttacker() );
		int level = pAttacker->GetModuleLevel(GetModule(BRUTEFORCE));
		if ( level > 0 )
		{
			int maxStacks = LEVEL(mod_bruteforce_buff_stacks, level);
			pAttacker->ApplyBuff(BUFF_BRUTEFORCE,pAttacker,min(maxStacks,pAttacker->GetBuffLevel(BUFF_BRUTEFORCE)+1));
		}
	}

	if ( info.GetDamageType() && info.GetAttacker() && info.GetAttacker()->IsPlayer() )
	{// if killed by a player give them the running man buff if they have that ability
		CHL2MP_Player *pAttacker = ToHL2MPPlayer( info.GetAttacker() );
		int level = pAttacker->GetModuleLevel(GetModule(ADRENALINE));
		if ( level > 0 )
		{
			int maxBuffStacks = LEVEL(mod_adrenaline_buff_stacks, level);
			pAttacker->ApplyBuff(BUFF_ADRENALINE,pAttacker,min(maxBuffStacks,pAttacker->GetBuffLevel(BUFF_ADRENALINE)+1));
		}
	}

	
	float fTotalDamage = 0.0f; // used to calculate percentage
	float damageTotals[MAX_PLAYERS+1] = {};
	
	// Find all the players that have attacked the NPC
	for( int i=1; i<=MAX_PLAYERS; i++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( i ) );
		if( pPlayer && !IsFriendly(pPlayer,pVictim) && pPlayer->m_DamageGivenThisLife.Count() > 0 )
		{
			for( int j = 0; j < pPlayer->m_DamageGivenThisLife.Count(); j++ )
			{
				playerattacks_t attack = pPlayer->m_DamageGivenThisLife[j];

				if( attack.hVictim && attack.hVictim == pVictim )
				{
					damageTotals[pPlayer->entindex()] += attack.flDamage;
					fTotalDamage += attack.flDamage;
					
					// must now remove this so that if the target respawns, its not used when they're killed again
					pPlayer->m_DamageGivenThisLife.FastRemove(j);
					j--;
				}
			}
		}
	}

	// we've got our list of people to give shared xp to... now give them it!
	if( fTotalDamage > 0 ) // we have some attackers... go go go
	{
		for( int i=1; i<=MAX_PLAYERS; i++ )
		{
			if( damageTotals[i] > 0 )
			{
				CHL2MP_Player *pLoopPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));
				if( pLoopPlayer )
				{
					float flPercent = damageTotals[ i ] / fTotalDamage;

					int expAwarded = CalculateExperience( pLoopPlayer, pVictim, flPercent);

					if (expAwarded <= 0)
					{//no reason in continuing or showing message
						 continue;
					}

					AwardFactionExp( pLoopPlayer, expAwarded);
					pLoopPlayer->AddExp( expAwarded ) ;

					const char *message_location = engine->GetClientConVarValue( engine->IndexOfEdict( pLoopPlayer->edict() ), "cl_showexpgained" );
					int location = 0;
					if ( message_location )
						location = atoi( message_location );
						
					if ( location > 0 )
					{// 0 = none, 1 = talk, 2 = notify, 3 = center
						const char *message;
						
						int typenum = 1;
						const char *message_type = engine->GetClientConVarValue( engine->IndexOfEdict( pLoopPlayer->edict() ), "cl_expgainedmessage" );
						if ( message_type )
							typenum = atoi( message_type );
						

						switch ( typenum )
						{
						case 4: // tiny
							message = UTIL_VarArgs("+%i exp - did %.0f%% to %s (%i)\n", expAwarded, flPercent > 100.0f ? 100.0f : flPercent * 100, pVictim->GetName(), pVictim->GetLevel());
							break;
						case 3: // short
							message = UTIL_VarArgs("+%i exp - %.0f%% damage to %s (lvl %i)\n", expAwarded, flPercent > 100.0f ? 100.0f : flPercent * 100, pVictim->GetName(), pVictim->GetLevel());
							break;
						case 2: // shorter
							message = UTIL_VarArgs("+%i experience - did %.0f%% damage to %s (lvl %i)\n", expAwarded, flPercent > 100.0f ? 100.0f : flPercent * 100, pVictim->GetName(), pVictim->GetLevel());
							break;
						case 1: // verbose
						default:
							message = UTIL_VarArgs("Gained %i exp for dealing %.0f%% damage to lvl %i %s\n", expAwarded, flPercent > 100.0f ? 100.0f : flPercent * 100, pVictim->GetLevel(), pVictim->GetName());
							break;
						}	
						ClientPrint( pLoopPlayer, location == 2 ? HUD_PRINTNOTIFY : location == 3 ? HUD_PRINTCENTER : HUD_PRINTTALK, message );
					}
				}
			}
		}
	}
	/*else // we didn't get any attackers, just revert back to normal old-style rewarding the killer
	{
		int expAwarded = CalculateExperience( pKiller, pVictim );
		AwardFactionExp(pKiller, expAwarded);
		pKiller->AddExp( expAwarded ) ;

		const char *cl_showexpgained = engine->GetClientConVarValue( engine->IndexOfEdict( pKiller->edict() ), "cl_showexpgained" );
		if ( cl_showexpgained )
		{
			int value = atoi( cl_showexpgained );
			if ( value > 0 )
			{// 0 = none, 1 = talk, 2 = notify, 3 = center
				ClientPrint( pKiller, value == 2 ? HUD_PRINTNOTIFY : value == 3 ? HUD_PRINTCENTER : HUD_PRINTTALK, UTIL_VarArgs("Gained +%i experience.\n", expAwarded ) );
			}
		}
	}*/

	CHL2MP_Player *pKiller = ToHL2MPPlayer( GetDeathScorer( info.GetAttacker(), info.GetInflictor() ) );
	CHL2MP_Player *pVictimPlayer = ToHL2MPPlayer(pVictim);

	if ( pKiller )
	{
		if ( pVictim->IsNPC() )
		{// player killed minion or monster
			pKiller->NoteKilledMonster();
		}
		else if ( pVictim->IsPlayer() )
		{// player kills player
			if ( info.GetInflictor() && info.GetInflictor()->IsNPC() ) // killed with A MINION
				pVictimPlayer->NoteKilledByMonster();
			else
				pVictimPlayer->NoteKilledByPlayer();
			if ( pKiller != pVictim ) // only award a kill if not a suicide
				pKiller->NoteKilledPlayer();
		}
	}
	else
	{
		if ( pVictim->IsPlayer() )
		{// monster killed a player
			pVictimPlayer->NoteKilledByMonster();
		}
		return;
	}
	
	if ( pKiller != pVictim && PlayerRelationship( pKiller, pVictim ) == GR_NOTTEAMMATE )
	{
		if ( pVictimPlayer )
		{// update the killer's spree only if they killed a player
			pKiller->IncrementSpree(); // add one to the killer's spree (victim's spree reset handled in their Event_Killed)
			if ( pKiller->GetSpree() % mc_spree_start.GetInt() == 0 ) // if the spree size is divisible by SPREE_START, update everyone
			{
				if ( pKiller->GetSpree() == mc_spree_start.GetInt() )
				{// start of our spree, adjust our hud
					CSingleUserRecipientFilter user( pKiller );
					user.MakeReliable();
					UserMessageBegin( user, "Spree" );
						WRITE_BYTE( 1 ); // start spree
					MessageEnd();
				}
				UTIL_ClientPrintAll( HUD_PRINTALL, UTIL_VarArgs("%s is on a %i-kill spree!\n", pKiller->GetPlayerName(),pKiller->GetSpree()) );


				// event for stat and omni-bot usage
				IGameEvent * event = gameeventmanager->CreateEvent( "spree" );
				if ( event )
				{
					event->SetInt("userid", pKiller->GetUserID() );
					event->SetInt("kills",  pKiller->GetSpree() );
					gameeventmanager->FireEvent( event );
				}
			}
		}
	}
}

extern McConVar mc_killexp_leveldiff_00, mc_killexp_leveldiff_n01, mc_killexp_leveldiff_n02, mc_killexp_leveldiff_n03, mc_killexp_leveldiff_n04, mc_killexp_leveldiff_n05, mc_killexp_leveldiff_n06, mc_killexp_leveldiff_n07, mc_killexp_leveldiff_n08, mc_killexp_leveldiff_n09, mc_killexp_leveldiff_n10, mc_killexp_leveldiff_n11, mc_killexp_leveldiff_n12, mc_killexp_leveldiff_n13, mc_killexp_leveldiff_n14, mc_killexp_leveldiff_n15, mc_killexp_leveldiff_n16, mc_killexp_leveldiff_n17, mc_killexp_leveldiff_n18, mc_killexp_leveldiff_n19, mc_killexp_leveldiff_n20, mc_killexp_leveldiff_n21, mc_killexp_leveldiff_n22, mc_killexp_leveldiff_n23, mc_killexp_leveldiff_n24, mc_killexp_leveldiff_p01, mc_killexp_leveldiff_p02, mc_killexp_leveldiff_p03, mc_killexp_leveldiff_p04, mc_killexp_leveldiff_p05, mc_killexp_leveldiff_p06, mc_killexp_leveldiff_p07, mc_killexp_leveldiff_p08, mc_killexp_leveldiff_p09, mc_killexp_leveldiff_p10, mc_killexp_leveldiff_p11, mc_killexp_leveldiff_p12, mc_killexp_leveldiff_p13, mc_killexp_leveldiff_p14, mc_killexp_leveldiff_p15, mc_killexp_leveldiff_p16, mc_killexp_leveldiff_p17, mc_killexp_leveldiff_p18, mc_killexp_leveldiff_p19, mc_killexp_leveldiff_p20, mc_killexp_leveldiff_p21, mc_killexp_leveldiff_p22, mc_killexp_leveldiff_p23, mc_killexp_leveldiff_p24;

int CHL2MPRules::CalculateExperience( CHL2MP_Player *pKiller, CBaseCombatCharacter *pVictim, float flPercent /* = 1.0f */)
{
	if( !pKiller || !pVictim )
		return 1;
	if(flPercent <= 0.00f)
		return 0;
	else if ( flPercent > 1.0f )
		flPercent = 1.0f;

	int pKillerLvl = pKiller->GetLevel(); // Get Killer's level
	int pVictimLvl = pVictim->GetLevel(); // Get Victim's level
	int clampedDif = min(max(pVictimLvl - pKillerLvl, -24),24); // keep it within range -24 to 24
	float calcExp;
	switch ( clampedDif )
	{
	case -24:	calcExp = mc_killexp_leveldiff_n24.GetInt(); break;
	case -23:	calcExp = mc_killexp_leveldiff_n23.GetInt(); break;
	case -22:	calcExp = mc_killexp_leveldiff_n22.GetInt(); break;
	case -21:	calcExp = mc_killexp_leveldiff_n21.GetInt(); break;
	case -20:	calcExp = mc_killexp_leveldiff_n20.GetInt(); break;
	case -19:	calcExp = mc_killexp_leveldiff_n19.GetInt(); break;
	case -18:	calcExp = mc_killexp_leveldiff_n18.GetInt(); break;
	case -17:	calcExp = mc_killexp_leveldiff_n17.GetInt(); break;
	case -16:	calcExp = mc_killexp_leveldiff_n16.GetInt(); break;
	case -15:	calcExp = mc_killexp_leveldiff_n15.GetInt(); break;
	case -14:	calcExp = mc_killexp_leveldiff_n14.GetInt(); break;
	case -13:	calcExp = mc_killexp_leveldiff_n13.GetInt(); break;
	case -12:	calcExp = mc_killexp_leveldiff_n12.GetInt(); break;
	case -11:	calcExp = mc_killexp_leveldiff_n11.GetInt(); break;
	case -10:	calcExp = mc_killexp_leveldiff_n10.GetInt(); break;
	case -9:	calcExp = mc_killexp_leveldiff_n09.GetInt(); break;
	case -8:	calcExp = mc_killexp_leveldiff_n08.GetInt(); break;
	case -7:	calcExp = mc_killexp_leveldiff_n07.GetInt(); break;
	case -6:	calcExp = mc_killexp_leveldiff_n06.GetInt(); break;
	case -5:	calcExp = mc_killexp_leveldiff_n05.GetInt(); break;
	case -4:	calcExp = mc_killexp_leveldiff_n04.GetInt(); break;
	case -3:	calcExp = mc_killexp_leveldiff_n03.GetInt(); break;
	case -2:	calcExp = mc_killexp_leveldiff_n02.GetInt(); break;
	case -1:	calcExp = mc_killexp_leveldiff_n01.GetInt(); break;
	case 0:		calcExp = mc_killexp_leveldiff_00.GetInt();  break;
	case 1:		calcExp = mc_killexp_leveldiff_p01.GetInt(); break;
	case 2:		calcExp = mc_killexp_leveldiff_p02.GetInt(); break;
	case 3:		calcExp = mc_killexp_leveldiff_p03.GetInt(); break;
	case 4:		calcExp = mc_killexp_leveldiff_p04.GetInt(); break;
	case 5:		calcExp = mc_killexp_leveldiff_p05.GetInt(); break;
	case 6:		calcExp = mc_killexp_leveldiff_p06.GetInt(); break;
	case 7:		calcExp = mc_killexp_leveldiff_p07.GetInt(); break;
	case 8:		calcExp = mc_killexp_leveldiff_p08.GetInt(); break;
	case 9:		calcExp = mc_killexp_leveldiff_p09.GetInt(); break;
	case 10:	calcExp = mc_killexp_leveldiff_p10.GetInt(); break;
	case 11:	calcExp = mc_killexp_leveldiff_p11.GetInt(); break;
	case 12:	calcExp = mc_killexp_leveldiff_p12.GetInt(); break;
	case 13:	calcExp = mc_killexp_leveldiff_p13.GetInt(); break;
	case 14:	calcExp = mc_killexp_leveldiff_p14.GetInt(); break;
	case 15:	calcExp = mc_killexp_leveldiff_p15.GetInt(); break;
	case 16:	calcExp = mc_killexp_leveldiff_p16.GetInt(); break;
	case 17:	calcExp = mc_killexp_leveldiff_p17.GetInt(); break;
	case 18:	calcExp = mc_killexp_leveldiff_p18.GetInt(); break;
	case 19:	calcExp = mc_killexp_leveldiff_p19.GetInt(); break;
	case 20:	calcExp = mc_killexp_leveldiff_p20.GetInt(); break;
	case 21:	calcExp = mc_killexp_leveldiff_p21.GetInt(); break;
	case 22:	calcExp = mc_killexp_leveldiff_p22.GetInt(); break;
	case 23:	calcExp = mc_killexp_leveldiff_p23.GetInt(); break;
	case 24:	calcExp = mc_killexp_leveldiff_p24.GetInt(); break;
	default:	calcExp = mc_killexp_leveldiff_00.GetInt();	 break;
	}
	
	// now rescale this base value, based on the experience modifying convars
	//calcExp = ((100 + mc_experience_level_falloff.GetFloat() * (calcExp - 100)) * (mc_experience_base.GetFloat()/100.0f));

	// now modify our calculated experience if either the killer or victim are on a spree
	CHL2MP_Player *pVictimPlayer = ToHL2MPPlayer(pVictim);			
	if ( pKiller->GetSpree() >= mc_spree_start.GetInt() && pVictimPlayer )
	{// Killer is on a spree, and killed a player, should give them extra experience
		// Currently 5% per kill, so 5th kill gives 25% extra, 6th gives 30%, 10th gives 50%, 20th gives 100% (double)
		calcExp *= 1.0f + pKiller->GetSpree()*mc_experience_spree_bonus.GetFloat();
	}
	if ( pVictimPlayer && pVictimPlayer->GetSpree() >= mc_spree_start.GetInt() )
	{// we've just killed someone on a spree, give us extra experience
		// currently 5% per kill (of victim's spree), so 25% extra for 5 kills, 50% for 10, 100% for 20
		calcExp *= 1.0f + pVictimPlayer->GetSpree()*mc_experience_spree_bonus.GetFloat();
	}

	if ( pVictim->IsNPC() )
	{// we just killed an NPC, so don't give as much experience
		CAI_BaseNPC *pNPC = (CAI_BaseNPC*)pVictim;
		if ( pNPC->LikesMaster() )
			calcExp *= mc_experience_minion.GetFloat();
		else
			calcExp *= mc_experience_monster.GetFloat();

		calcExp *= pNPC->GetExperienceScale();
	}

	// in PVM, we award 1% more experience for players that are close to members of their own faction
	calcExp += calcExp * ally_experience_boost_fraction.GetFloat() * pKiller->NumAlliesInRange();

	// in the Hoarder gamemode, kill experience is reduced, to compensate for significant victory experience
	if ( ShouldUseScoreTokens() )
		calcExp *= gamemode_hoarder_exp_scale.GetFloat();

	//calcExp *= mc_experience_scale.GetFloat();
	calcExp *= flPercent;

	return max(1, (int)calcExp); // never give them 0 exp after all of that calculation
}

#ifndef RELEASE
extern ConVar mc_dev_debug_factions("mc_dev_debug_factions", "0", FCVAR_NOTIFY | FCVAR_CHEAT, "Disables limits on faction experience, so it can be applied with any number of players");
#endif
void CHL2MPRules::AwardFactionExp(CHL2MP_Player *pPlayer, int amount)
{
	// check game mode allows it, and check we have multiple factions in-game...
	if ( IsTeamplay() )
	{
		int numFactionsPresent = 0;
		if ( m_iNumCombinePlayers > 0 )
			numFactionsPresent ++;
		if ( m_iNumResistancePlayers > 0 )
			numFactionsPresent ++;
		if ( m_iNumAperturePlayers > 0 )
			numFactionsPresent ++;

		if ( numFactionsPresent < 2
#ifndef RELEASE
		&& mc_dev_debug_factions.GetInt() == 0
#endif
		)
			return; // need multiple factions to consider bonus experience

		switch ( pPlayer->GetFaction() )
		{
		case FACTION_COMBINE:
			if ( m_iNumCombinePlayers <= 0 ) return; // no divide by zero, just in case!

			m_flExpFactionCombine += (float)amount / (float)m_iNumCombinePlayers;
			break;
		case FACTION_RESISTANCE:
			if ( m_iNumResistancePlayers <= 0 ) return; // no divide by zero, just in case!

			m_flExpFactionResistance += (float)amount / (float)m_iNumResistancePlayers;
			break;
		case FACTION_APERTURE:
			if ( m_iNumAperturePlayers <= 0 ) return; // no divide by zero, just in case!

			m_flExpFactionAperture += (float)amount / (float)m_iNumAperturePlayers;
			break;
		}
	}
}

void CHL2MPRules::AdjustFactionCount(int faction, int adjustment)
{
	switch ( faction )
	{
		case FACTION_COMBINE:
			m_iNumCombinePlayers += adjustment;
			if ( m_iNumCombinePlayers <= 0 )
				m_flExpFactionCombine = 0; // clear this when no one is on the team
			break;
		case FACTION_RESISTANCE:
			m_iNumResistancePlayers += adjustment;
			if ( m_iNumResistancePlayers <= 0 )
				m_flExpFactionResistance = 0; // clear this when no one is on the team
			break;
		case FACTION_APERTURE:
			m_iNumAperturePlayers += adjustment;
			if ( m_iNumAperturePlayers <= 0 )
				m_flExpFactionAperture = 0; // clear this when no one is on the team
			break;
	}

#ifndef RELEASE
	if ( mc_dev_debug_factions.GetInt() )
		UTIL_ClientPrintAll(HUD_PRINTCONSOLE,UTIL_VarArgs("%i combine players\n%i resistance players\n%i aperture players\n", m_iNumCombinePlayers, m_iNumResistancePlayers, m_iNumAperturePlayers));
#endif
}

int CHL2MPRules::GetTargetScoreTokenLimit()
{
	return NumPlayers() * 2;
}

extern bool g_bAllowCheckForScoreTokenVictory;
void CHL2MPRules::AdjustFactionScoreTokenCount(int factionScored, int adjustment)
{
	if ( factionScored <= 0 || factionScored > NUM_FACTIONS || g_fGameOver )
		return;

	int factionTokens = m_iFactionScoreTokens.Get(factionScored-1) + adjustment;
	m_iFactionScoreTokens.Set(factionScored-1, factionTokens);

	if ( !g_bAllowCheckForScoreTokenVictory || !HL2MPRules()->ShouldUseScoreTokens() )
		return;

	if ( factionTokens >= GetTargetScoreTokenLimit() )
	{
		UTIL_ClientPrintAll(HUD_PRINTTALK,UTIL_VarArgs("%s aquired all of the score tokens, and win the round!\n", GetFactionName(factionScored)));
		HoarderGameFinished(factionScored);
	}

	else if ( factionTokens >= GetFactionScoreTokenTarget(factionScored) )
	{
		UTIL_ClientPrintAll(HUD_PRINTTALK,UTIL_VarArgs("%s gathered enough score tokens to win the round!\n", GetFactionName(factionScored)));
		HoarderGameFinished(factionScored);
	}
}

void CHL2MPRules::HoarderGameFinished(int winningFaction)
{
	// reset all players to 1 score token, and award experience to all players on winningFaction
	g_bAllowCheckForScoreTokenVictory = false;
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
		if ( !pPlayer || !pPlayer->IsInCharacter() )
			continue;

		pPlayer->RemoveScoreToken(pPlayer->GetNumScoreTokens());
		pPlayer->AddScoreToken();

		if ( winningFaction == FACTION_NONE || pPlayer->GetFaction() != winningFaction )
			continue;

		pPlayer->AddExp(gamemode_hoarder_victory_exp.GetInt());
	}
	g_bAllowCheckForScoreTokenVictory = true;

	// remove all score tokens from monsters
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	for ( int i = 0; i < nAIs; i++ )
	{
		CAI_BaseNPC *pNPC = ppAIs[i];
		if ( !pNPC )
			continue;
		pNPC->RemoveScoreToken(pNPC->GetNumScoreTokens());
	}

	// remove all "dropped" score tokens


	// reset the "next score token spawn" counter
	m_flNextScoreTokenSpawnTime = gpGlobals->curtime + gamemode_hoarder_token_add_interval.GetFloat();
}

void CHL2MPRules::RemoveAllPlayerScoreTokens()
{
	g_bAllowCheckForScoreTokenVictory = false;
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
		if ( !pPlayer )
			continue;

		pPlayer->RemoveScoreToken(pPlayer->GetNumScoreTokens());
	}
	g_bAllowCheckForScoreTokenVictory = true;
}

bool g_bEnoughPlayersToEnableScoreTokens = true;
void CHL2MPRules::PlayerEnteredCharacter(CHL2MP_Player *pPlayer)
{
	AdjustFactionCount(pPlayer->GetFaction(), 1);

	if ( !g_bEnoughPlayersToEnableScoreTokens && ShouldUseScoreTokens() )
	{
		SHOW_HINT_ALWAYS_ALL("Hint_ModeHoarder1");
		HoarderGameFinished(FACTION_NONE); // give everyone one token and get going
	}
}

void CHL2MPRules::PlayerLeftCharacter(CHL2MP_Player *pPlayer)
{
	AdjustFactionCount(pPlayer->GetFaction(), -1);
	
	int scoreTokensToRemove = mc_gamemode.GetInt() == HOARDER ? max(0,GetTotalNumScoreTokens() - GetTargetScoreTokenLimit()) : 0;
	
	if ( g_bEnoughPlayersToEnableScoreTokens && !ShouldUseScoreTokens() )
	{
		SHOW_HINT_ALWAYS_ALL("Hint_ModeHoarder2");
		RemoveAllPlayerScoreTokens();
	}
	
	// any monsters with this player as a master should have it set to null
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	for ( int i = 0; i < nAIs; i++ )// look through all the AIs and check each
	{
		CAI_BaseNPC *pNPC = ppAIs[i];
		if ( !pNPC || pNPC->GetMasterPlayer() != pPlayer )
			continue;

		pNPC->SetLikesMaster(false); // stop calling them "unknown's minion"
		pNPC->SetMasterPlayer(NULL); // stop this npc looking for them & crashing
		
		// remove score tokens from monsters if this player's exit means there are now too many of them in play
		int numToRemove = scoreTokensToRemove == 0 ? 0 : min(scoreTokensToRemove, pNPC->GetNumScoreTokens());
		if ( numToRemove > 0 )
		{
			pNPC->RemoveScoreToken(numToRemove);
			scoreTokensToRemove -= numToRemove;
		}
	}
}
#endif

bool CHL2MPRules::ShouldUseScoreTokens()
{
	bool retVal = mc_gamemode.GetInt() == HOARDER && NumPlayers() >= gamemode_hoarder_min_players.GetInt();
#ifndef CLIENT_DLL
	g_bEnoughPlayersToEnableScoreTokens = retVal;
#endif
	return retVal;
}

int CHL2MPRules::GetFactionScoreTokenCount(int faction)
{
	if ( faction <= 0 || faction > NUM_FACTIONS )
		return 0;
	return m_iFactionScoreTokens[faction-1];
}

int CHL2MPRules::GetFactionScoreTokenTarget(int faction)
{
	if ( faction <= 0 || faction > NUM_FACTIONS )
		return 0;

	return NumPlayers() + NumPlayersOnFaction(faction);
}

const char *CHL2MPRules::GetFactionName(int faction)
{
	switch ( faction )
	{
	case FACTION_COMBINE:
		return "The Combine";
	case FACTION_RESISTANCE:
		return "The Resistance";
	case FACTION_APERTURE:
		return "Aperture";
	case FACTION_NONE:
		return "Nobody";
	default:
		return "Unknown";
	}
}

void CHL2MPRules::Think( void )
{
#ifndef CLIENT_DLL
	CGameRules::Think();

	if ( firstThink )
	{
		firstThink = false;
		
		// create aux battery pickups
		int batteries = GetNumEntities("item_battery");
		int healthKits = GetNumEntities("item_healthkit");
		int healthVials = GetNumEntities("item_healthvial");
		int healthChargers = GetNumEntities("func_healthcharger") + GetNumEntities("item_healthcharger");
		int armorChargers = GetNumEntities("func_recharge") + GetNumEntities("item_suitcharger");
		int numToMake = batteries * mc_aux_batteries_per_armor_battery.GetFloat()
			          + healthVials * mc_aux_batteries_per_health_kit.GetFloat()
					  + healthKits * mc_aux_batteries_per_health_vial.GetFloat()
					  + healthChargers * mc_aux_batteries_per_health_charger.GetFloat()
					  + armorChargers * mc_aux_batteries_per_armor_charger.GetFloat();
		if ( numToMake < mc_aux_batteries_min.GetInt() )
			numToMake = mc_aux_batteries_min.GetInt();
#ifdef _DEBUG		
		Msg(UTIL_VarArgs("Got %i batteries, %i health kits, %i health vials, %i health chargers and %i armor chargers. Creating %i AUX batteries based on these.\n", batteries, healthKits, healthVials, healthChargers, armorChargers, numToMake));
#endif

		for ( int i=0; i<numToMake; i++ )
		{
			CBaseEntity *pAux = CreateEntityByName("item_aux_battery");
			pAux->Spawn(); // randomizes its location
			pAux->AddFlag(SF_NORESPAWN);
		}

		g_pDB2->BeginTransaction(); // begin global cycle of commits to stats database at regular intervals
		m_flNextGameStatsCommit = gpGlobals->curtime + gamestats_commit_interval.GetFloat();
	}

	int gameMode = mc_gamemode.GetInt();
	bool useMonsterSpawns = GameModeUsesMonsters(gameMode);
	if ( m_iRememberGameMode != gameMode )
	{
		m_flExpFactionCombine = m_flExpFactionResistance = m_flExpFactionAperture = 0; // reset faction experience counters when mode changes
		bool didUseMonsterSpawns = GameModeUsesMonsters(m_iRememberGameMode);
		if ( didUseMonsterSpawns != useMonsterSpawns ) // pvm turned on or off
		{
			if ( useMonsterSpawns && m_pNpcSpawns.Count() == 0 )
			{
				UTIL_ClientPrintAll(HUD_PRINTNOTIFY,"Cannot enter monster-dependent game mode, as no npc spawn point file is present.\nYou can create one using the startnpcspawns cheat ConCommand\n");
				if(gameMode == FFA)					//they voted for solo kind of game mode
					gameMode = DEATHMATCH;
				else								//They voted for a teamplay game mode
					gameMode = TEAM_DEATHMATCH;
				m_iRememberGameMode = gameMode;
			}
			else
			{
				m_iRememberGameMode = gameMode;
				ResetPVM(); //It was turned off or on, call it
				if(useMonsterSpawns) // if it dosn't error our and we are in pvm, set the next wave time
					m_flNextWave = gpGlobals->curtime + 6.0f;
			}
		}

		if ( gameMode == RANDOM_PVM )
		{
			UTIL_ClientPrintAll(HUD_PRINTTALK,"BoSS: Players vs. Monsters gamemode enabled.\n");
			
			// Send pvm sound to all players
			CRecipientFilter filter;
			filter.AddAllPlayers( );
			UserMessageBegin( filter, "SendAudio" );
			WRITE_STRING( "BoSS.Execution" );
			MessageEnd();
			
			m_flNextWave = gpGlobals->curtime + 6.0f;
		}
		else if (gameMode == FFA)
		{
			UTIL_ClientPrintAll(HUD_PRINTTALK,"BoSS: Free For All gamemode enabled.\n");
		}
		else if ( gameMode == DEATHMATCH )
		{
			ResetPVM();
		}
		else if ( gameMode == TEAM_DEATHMATCH )
		{
			ResetPVM();
		}
		else if ( gameMode == HOARDER )
		{
			HoarderGameFinished(FACTION_NONE); // ensure everyone is allocated a single score token at the start of a hoarder game round
		}

		m_iRememberGameMode = gameMode;
		mc_gamemode.SetValue(gameMode);
		
		if ( IsTeamplay() && IsInSpreeWar() )
			EndSpreeWar(gameMode == RANDOM_PVM); // no spree wars in team games, and no sprees at all in PVM
	}

	if ( useMonsterSpawns && m_flNextWave <= gpGlobals->curtime && !IsIntermission() )
	{
		SpawnMonsterWave();	
	}

	if ( g_fGameOver )   // someone else quit the game already
	{
		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			ChangeLevel(); // intermission is over
		}

		return;
	}

	if ( m_flNextGameStatsCommit <= gpGlobals->curtime )
	{
		g_pDB2->CommitTransaction();
		g_pDB2->BeginTransaction();
		m_flNextGameStatsCommit = gpGlobals->curtime + gamestats_commit_interval.GetFloat();
	}
	
	if ( !cheatsHaveBeenOnThisGame && sv_cheats->GetBool() )
	{// cheats have just been turned on for the first time this game. Be sure we record this game as "cheating"
		cheatsHaveBeenOnThisGame = true;
		g_pDB2->Command("update game set cheatsenabled = 1 where id = '%s'", m_szGameID);
	}
	if ( nonDefaultConvarsHaveBeenOnThisGame && !mc_use_defaults.GetBool() )
	{	// log that defaults have been turned off ... don't do this in the callback, as the gamerules might not yet have been initialized
		g_pDB2->Command("update game set defaultconvars = 0 where id = '%s'", HL2MPRules()->GetGameID());
	}

	if ( IsInVote() && gpGlobals->curtime >= m_flVoteEndTime )
	{
		VoteFinished();
		return; // no more thinking this tick, in case a collision causes problems
	}


	/*
	float flFragLimit = fraglimit.GetFloat();
	if ( flFragLimit )
	{
		if( IsTeamplay() == true )
		{
			CTeam *pCombine = g_Teams[TEAM_COMBINE];
			CTeam *pRebels = g_Teams[TEAM_REBELS];

			if ( pCombine->GetScore() >= flFragLimit || pRebels->GetScore() >= flFragLimit )
			{
				GoToIntermission();
				return;
			}
		}
		else
		{
			// check if any player is over the frag limit
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

				if ( pPlayer && pPlayer->FragCount() >= flFragLimit )
				{
					GoToIntermission();
					return;
				}
			}
		}
	}*/

	if ( gpGlobals->curtime > m_tmNextPeriodicThink )
	{
		float timeRemaining = GetMapRemainingTime();
		if ( timeRemaining != 0 ) // if == 0, probably means no time limit
		{
			// if next map not set, not already in a vote, and map remaining time is less than the duration of the number of votes already set up, run start vote sequence
			if ( (!nextlevel.GetString() || FStrEq(nextlevel.GetString(), "")) && !IsInVote() && timeRemaining < NumQueuedVotes() * mc_vote_duration.GetFloat() + 7)
			{
				RunNextVote(); // PrepareVoteSequence will have been called at the start of the map, or at the end of the last unsuccessful mid-map vote
			}
			else if ( timeRemaining < 0 )
			{
				GoToIntermission();
				return;
			}
		}

		CheckAllPlayersReady();
		CheckRestartGame();
		m_tmNextPeriodicThink = gpGlobals->curtime + 1.0;
	}

	if ( m_flRestartGameTime > 0.0f && m_flRestartGameTime <= gpGlobals->curtime )
	{
		RestartGame();
	}

	if( m_bAwaitingReadyRestart && m_bHeardAllPlayersReady )
	{
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "All players ready. Game will restart in 5 seconds" );
		UTIL_ClientPrintAll( HUD_PRINTCONSOLE, "All players ready. Game will restart in 5 seconds" );

		m_flRestartGameTime = gpGlobals->curtime + 5;
		m_bAwaitingReadyRestart = false;
	}

	ManageObjectRelocation();
#endif
}

void CHL2MPRules::GoToIntermission( void )
{
#ifndef CLIENT_DLL
	if ( g_fGameOver )
		return;

#define NUM_ENDGAME_SOUNDS	8
	g_fGameOver = true;
	// randomly select one here, when we have more than one to work from :)
	const char *speechSound = UTIL_VarArgs("BoSS.Endgame%i", random->RandomInt(1,NUM_ENDGAME_SOUNDS));

	float speechDuration = 0.0f;
	CSoundParameters params;
	if ( CBaseEntity::GetParametersForSound( speechSound, params, NULL ) )
	{
		// Get the duration so we know when it finishes
		speechDuration = enginesound->GetSoundDuration( params.soundname ) ;
		//Msg(UTIL_VarArgs("Speech duration: %.2f\n", speechDuration));
	}
	CRecipientFilter filter;
	filter.AddAllPlayers( );
	UserMessageBegin( filter, "SendAudio" );
		WRITE_STRING( speechSound );
	MessageEnd();

	// if the speech is longer than chat time, extend it so that we hear the whole thing
	m_flIntermissionEndTime = gpGlobals->curtime + max(speechDuration, mp_chattime.GetInt());
	//ResetPVM();
	
	// freeze all players
	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( !pPlayer )
			continue;

		pPlayer->ShowViewPortPanel( PANEL_SCOREBOARD );
		pPlayer->AddFlag( FL_FROZEN );
	}

	// freeze all NPCs
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	for ( int i = 0; i < nAIs; i++ )
	{
		CAI_BaseNPC *pNPC = ppAIs[i];
		if ( !pNPC )
			continue;
		pNPC->ClearCondition(COND_NPC_UNFREEZE);
		pNPC->SetCondition(COND_NPC_FREEZE);
	}

	if ( IsTeamplay() )
	{
		int numFactionsPresent = 0;
		if ( m_iNumCombinePlayers > 0 )
			numFactionsPresent ++;
		if ( m_iNumResistancePlayers > 0 )
			numFactionsPresent ++;
		if ( m_iNumAperturePlayers > 0 )
			numFactionsPresent ++;

		if ( numFactionsPresent < 2
#ifndef RELEASE
		&& mc_dev_debug_factions.GetInt() == 0
#endif
		)
			return; // need multiple factions to consider bonus experience

		int winningFaction = FACTION_NONE;
		if ( m_flExpFactionCombine > m_flExpFactionResistance && m_flExpFactionCombine > m_flExpFactionAperture )
			winningFaction = FACTION_COMBINE;
		else if ( m_flExpFactionResistance > m_flExpFactionCombine && m_flExpFactionResistance > m_flExpFactionAperture )
			winningFaction = FACTION_RESISTANCE;
		else if ( m_flExpFactionAperture > m_flExpFactionCombine && m_flExpFactionAperture > m_flExpFactionResistance )
			winningFaction = FACTION_APERTURE;

		int winExp = mc_faction_win_experience.GetInt();

		UTIL_ClientPrintAll(HUD_PRINTTALK,UTIL_VarArgs("Combine earned %.0f experience per player\nResistance earned %.0f experience per player\nAperture earned %.0f experience per player\n%s is awarded %i exp for winning!\n", m_flExpFactionCombine.Get(), m_flExpFactionResistance.Get(), m_flExpFactionAperture.Get(), GetFactionName(winningFaction), winExp ));
		if ( mc_faction_win_experience.GetInt() > 0 && winningFaction != FACTION_NONE )
			for ( int i=1; i<gpGlobals->maxClients; i++ )
			{
				CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex(i) );
				if ( pPlayer && pPlayer->IsInCharacter() && pPlayer->GetFaction() == winningFaction )
					pPlayer->AddExp( winExp );
			}
	}
	
	g_pDB2->Command("update game set gamemode = %i, rebelscore = %i, combinescore = %i, aperturescore = %i where id = '%s'", mc_gamemode.GetInt(), (int)m_flExpFactionResistance, (int)m_flExpFactionCombine, (int)m_flExpFactionAperture, m_szGameID);
	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( i ) );
		if ( !pPlayer || !pPlayer->IsInCharacter() )
			continue;

		int rank = 0;
		if ( pPlayer->GetChangedBits() & BITS_CHANGED_MODULES )
			pPlayer->SerializeModuleData();
		g_pDB2->Command("insert into gameplayer (gameid, characterid, faction, plevel, modules, gameexp, duration, playerkills, playerdeaths, monsterkills, monsterdeaths, rank) values ('%s', %i, %i, %i, '%s', %i, %.2f, %i, %i, %i, %i, %i)", m_szGameID, pPlayer->GetCharacterID(), pPlayer->GetFaction(), pPlayer->GetLevel(), pPlayer->GetSerializedModules(), pPlayer->GetGameExp(), pPlayer->GameTimePlayed(), pPlayer->GetPlayerKills(), pPlayer->GetPlayerDeaths(), pPlayer->GetMonsterKills(), pPlayer->GetMonsterDeaths(), rank);
	}
	g_pDB2->CommitTransaction();
#endif
}

bool CHL2MPRules::CheckGameOver()
{
#ifndef CLIENT_DLL
	if ( g_fGameOver )   // someone else quit the game already
	{
		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->curtime )
		{
			ChangeLevel(); // intermission is over			
		}

		return true;
	}
#endif

	return false;
}

// when we are within this close to running out of entities,  items 
// marked with the ITEM_FLAG_LIMITINWORLD will delay their respawn
#define ENTITY_INTOLERANCE	100

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CHL2MPRules::FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( pWeapon && (pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
	{
		if ( gEntList.NumberOfEntities() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE) )
			return 0;

		// we're past the entity tolerance level,  so delay the respawn
		return FlWeaponRespawnTime( pWeapon );
	}
#endif
	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHL2MPRules::VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	CWeaponHL2MPBase *pHL2Weapon = dynamic_cast< CWeaponHL2MPBase*>( pWeapon );

	if ( pHL2Weapon )
	{
		return pHL2Weapon->GetOriginalSpawnOrigin();
	}
#endif
	
	return pWeapon->GetAbsOrigin();
}

#ifndef CLIENT_DLL

CItem* IsManagedObjectAnItem( CBaseEntity *pObject )
{
	return dynamic_cast< CItem*>( pObject );
}

CWeaponHL2MPBase* IsManagedObjectAWeapon( CBaseEntity *pObject )
{
	return dynamic_cast< CWeaponHL2MPBase*>( pObject );
}

bool GetObjectsOriginalParameters( CBaseEntity *pObject, Vector &vOriginalOrigin, QAngle &vOriginalAngles )
{
	if ( CItem *pItem = IsManagedObjectAnItem( pObject ) )
	{
		if ( pItem->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;
		
		vOriginalOrigin = pItem->GetOriginalSpawnOrigin();
		vOriginalAngles = pItem->GetOriginalSpawnAngles();

		pItem->m_flNextResetCheckTime = gpGlobals->curtime + sv_hl2mp_item_respawn_time.GetFloat();
		return true;
	}
	else if ( CWeaponHL2MPBase *pWeapon = IsManagedObjectAWeapon( pObject )) 
	{
		if ( pWeapon->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;

		vOriginalOrigin = pWeapon->GetOriginalSpawnOrigin();
		vOriginalAngles = pWeapon->GetOriginalSpawnAngles();

		pWeapon->m_flNextResetCheckTime = gpGlobals->curtime + sv_hl2mp_weapon_respawn_time.GetFloat();
		return true;
	}

	return false;
}

void CHL2MPRules::ManageObjectRelocation( void )
{
	int iTotal = m_hRespawnableItemsAndWeapons.Count();

	if ( iTotal > 0 )
	{
		for ( int i = 0; i < iTotal; i++ )
		{
			CBaseEntity *pObject = m_hRespawnableItemsAndWeapons[i].Get();
			
			if ( pObject )
			{
				Vector vSpawOrigin;
				QAngle vSpawnAngles;

				if ( GetObjectsOriginalParameters( pObject, vSpawOrigin, vSpawnAngles ) == true )
				{
					float flDistanceFromSpawn = (pObject->GetAbsOrigin() - vSpawOrigin ).Length();

					if ( flDistanceFromSpawn > WEAPON_MAX_DISTANCE_FROM_SPAWN )
					{
						bool shouldReset = false;
						IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();

						if ( pPhysics )
						{
							shouldReset = pPhysics->IsAsleep();
						}
						else
						{
							shouldReset = (pObject->GetFlags() & FL_ONGROUND) ? true : false;
						}

						if ( shouldReset )
						{
							pObject->Teleport( &vSpawOrigin, &vSpawnAngles, NULL );
							pObject->EmitSound( "AlyxEmp.Charge" );

							IPhysicsObject *pPhys = pObject->VPhysicsGetObject();

							if ( pPhys )
							{
								pPhys->Wake();
							}
						}
					}
				}
			}
		}
	}
}

//=========================================================
//AddLevelDesignerPlacedWeapon
//=========================================================
void CHL2MPRules::AddLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) == -1 )
	{
		m_hRespawnableItemsAndWeapons.AddToTail( pEntity );
	}
}

//=========================================================
//RemoveLevelDesignerPlacedWeapon
//=========================================================
void CHL2MPRules::RemoveLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) != -1 )
	{
		m_hRespawnableItemsAndWeapons.FindAndRemove( pEntity );
	}
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHL2MPRules::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->GetOriginalSpawnOrigin();
}

//=========================================================
// What angles should this item use to respawn?
//=========================================================
QAngle CHL2MPRules::VecItemRespawnAngles( CItem *pItem )
{
	return pItem->GetOriginalSpawnAngles();
}

//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CHL2MPRules::FlItemRespawnTime( CItem *pItem )
{
	return sv_hl2mp_item_respawn_time.GetFloat();
}


//=========================================================
// CanHaveWeapon - returns false if the player is not allowed
// to pick up this weapon
//=========================================================
bool CHL2MPRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem )
{
	if ( weaponstay.GetInt() > 0 )
	{
		if ( pPlayer->Weapon_OwnsThisType( pItem->GetClassname(), pItem->GetSubType() ) )
			 return false;
	}

	return BaseClass::CanHavePlayerItem( pPlayer, pItem );
}

#endif

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CHL2MPRules::WeaponShouldRespawn( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( pWeapon->HasSpawnFlags( SF_NORESPAWN ) )
	{
		return GR_WEAPON_RESPAWN_NO;
	}
#endif

	return GR_WEAPON_RESPAWN_YES;
}

//-----------------------------------------------------------------------------
// Purpose: Player has just left the game
//-----------------------------------------------------------------------------
void CHL2MPRules::ClientDisconnected( edict_t *pClient )
{
#ifndef CLIENT_DLL
	// Msg( "CLIENT DISCONNECTED, REMOVING FROM TEAM.\n" );

	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );
	if ( pPlayer )
	{
		// Remove the player from his team
		if ( pPlayer->GetTeam() )
		{
			pPlayer->GetTeam()->RemovePlayer( pPlayer );
		}

		// end a spree war if the spreer disconnects		
		if ( HL2MPRules()->IsInSpreeWar() && ToHL2MPPlayer( pPlayer ) == GetSpreer() )
		{
			EndSpreeWar();
		}
	}

	BaseClass::ClientDisconnected( pClient );

#endif
}


//=========================================================
// Deathnotice. 
//=========================================================
void CHL2MPRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL
	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_ID = 0;

	// Find the killer & the scorer
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = GetDeathScorer( pKiller, pInflictor );
	int ability = info.GetModule();

	/*if ( ability == NO_MODULE && pInflictor->IsNPC() )
	{
		CAI_BaseNPC *pNPC = static_cast<CAI_BaseNPC*>(pInflictor);
		ability = pNPC->GetMyModule();
	}*/

	// Custom kill type?
	if ( info.GetDamageCustom() )
	{
		killer_weapon_name = GetDamageCustomString( info );
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
		}
	}
	else
	{
		// Is the killer a client?
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
			
			if ( ability != NO_MODULE ) // if this kill was due to an ability, be sure to report what one!
				killer_weapon_name = GetModule(ability)->GetCmdName();
			else if ( pInflictor )
			{
				if ( pInflictor == pScorer )
				{
					// If the inflictor is the killer,  then it must be their current weapon doing the damage
					if ( pScorer->GetActiveWeapon() )
					{
						killer_weapon_name = pScorer->GetActiveWeapon()->GetClassname();
					}
				}
				else if ( pInflictor->IsNPC() )
				{
					killer_weapon_name = pInflictor->MyNPCPointer()->GetName();
				}
				else
				{
					killer_weapon_name = pInflictor->GetClassname();  // it's just that easy
				}
			}
		}
		else if ( !pInflictor )
		{
			killer_weapon_name = "Unknown";
		}
		else if ( pInflictor->IsNPC() )
		{
			killer_weapon_name = pInflictor->MyNPCPointer()->GetName();
		}
		else
		{
			killer_weapon_name = pInflictor->GetClassname();
		}

		// strip the NPC_* or weapon_* from the inflictor's classname
		if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
		{
			killer_weapon_name += 7;
		}
		else if ( strncmp( killer_weapon_name, "npc_", 4 ) == 0 )
		{
			killer_weapon_name += 4;
		}
		else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
		{
			killer_weapon_name += 5;
		}
		else if ( strstr( killer_weapon_name, "physics" ) )
		{
			killer_weapon_name = "physics";
		}

		if ( strcmp( killer_weapon_name, "prop_combine_ball" ) == 0 )
		{
			killer_weapon_name = "combine_ball";
		}
		else if ( strcmp( killer_weapon_name, "grenade_ar2" ) == 0 )
		{
			killer_weapon_name = "smg1_grenade";
		}
		else if ( strcmp( killer_weapon_name, "satchel" ) == 0 || strcmp( killer_weapon_name, "tripmine" ) == 0)
		{
			killer_weapon_name = "slam";
		}


	}

	IGameEvent *event = gameeventmanager->CreateEvent( "player_death" );
	if( event )
	{
		event->SetInt("userid", pVictim->GetUserID() );
		event->SetInt("attacker", killer_ID );
		event->SetString("weapon", killer_weapon_name );
		event->SetInt( "priority", 7 );
		gameeventmanager->FireEvent( event );
	}
#endif

}

void CHL2MPRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( pPlayer );

	if ( pHL2Player == NULL )
		return;
	if ( sv_report_client_settings.GetInt() == 1 )
	{
		UTIL_LogPrintf( "\"%s\" cl_cmdrate = \"%s\"\n", pHL2Player->GetPlayerName(), engine->GetClientConVarValue( pHL2Player->entindex(), "cl_cmdrate" ));
	}

	BaseClass::ClientSettingsChanged( pPlayer );
#endif
	
}

int CHL2MPRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	CHL2MP_Player *pHl2Player = ToHL2MPPlayer( pPlayer );
	CHL2MP_Player *pTargetPlayer = ToHL2MPPlayer( pTarget );

	// we only like each other in a spree war if neither of us is the spreer
	if ( IsInSpreeWar() && pHl2Player && pTargetPlayer && GetSpreer() != pHl2Player && GetSpreer() != pTargetPlayer )
		return GR_TEAMMATE;

	if ( !pTargetPlayer || !pHl2Player )
		return GR_NOTTEAMMATE;

	if ( mc_gamemode.GetInt() == RANDOM_PVM ) // in gamemode = 2 mode, all players are allies.
		return GR_TEAMMATE;
	else if ( GameModeUsesTeamplay(mc_gamemode.GetInt()) && pTargetPlayer->GetFaction() == pHl2Player->GetFaction() )
		return GR_TEAMMATE;
	else
		return GR_NOTTEAMMATE;
}

#ifdef CLIENT_DLL
#define CAI_BaseNPC C_AI_BaseNPC
#endif

// this function could be made recursive, and just look at the parent / master / thrower and keep calling itself
// until it has two players or a null, then do player relationship.
bool CHL2MPRules::IsFriendly( CBaseEntity *pChecker, CBaseEntity *pTarget )
{
	if ( !pChecker || !pTarget )
		return false;
	if ( pChecker == pTarget )
		return true;

	if ( pChecker->IsPlayer() )
	{
		if ( pTarget->IsPlayer() )
			return PlayerRelationship(pChecker,pTarget) == GR_TEAMMATE;
		
		else if ( pTarget->IsNPC() )
		{
			CAI_BaseNPC *pNPC = pTarget->MyNPCPointer();
			if ( pNPC && pNPC->LikesMaster() && pNPC->GetMasterPlayer() )
				return pNPC->GetMasterPlayer() == pChecker || PlayerRelationship(pChecker,pNPC->GetMasterPlayer()) == GR_TEAMMATE;
			else
				return false;
		}

		else if ( pTarget->IsGrenade() )
		{
			CBaseGrenade *pGrenade = pTarget->MyGrenadePointer();
			if ( pGrenade && pGrenade->GetThrower() && pGrenade->GetThrower()->IsPlayer() )
				return pGrenade->GetThrower() == pChecker || PlayerRelationship(pChecker,ToHL2MPPlayer(pGrenade->GetThrower())) == GR_TEAMMATE;
			else
				return false;
		}
	}
	else if ( pChecker->IsNPC() )
	{
		CAI_BaseNPC *pCheckerNPC = pChecker->MyNPCPointer();
		bool checkerHasMaster = ( pCheckerNPC && pCheckerNPC->LikesMaster() && pCheckerNPC->GetMasterPlayer() );
		
		if ( pTarget->IsPlayer() )
			return checkerHasMaster && (pCheckerNPC->GetMasterPlayer() == pTarget || PlayerRelationship(pCheckerNPC->GetMasterPlayer(),pTarget) == GR_TEAMMATE);
		
		else if ( pTarget->IsNPC() )
		{
			CAI_BaseNPC *pNPC = pTarget->MyNPCPointer();
			bool targetHasMaster = ( pNPC && pNPC->LikesMaster() && pNPC->GetMasterPlayer() );
			
			if ( checkerHasMaster )
			{
				if ( targetHasMaster ) // both have master, friends if its the same master or masters are friends
					return pCheckerNPC->GetMasterPlayer() == pNPC->GetMasterPlayer() || PlayerRelationship(pCheckerNPC->GetMasterPlayer(),pNPC->GetMasterPlayer()) == GR_TEAMMATE;
				else // target has no master, its a pvm and I'm a minion
					return false; 
			}
			else if ( targetHasMaster ) // its a minion, I'm a PVM
				return false;
			else // its a PVM & so am I; we're friends
				return true;
		}

		else if ( pTarget->IsGrenade() )
		{
			CBaseGrenade *pGrenade = pTarget->MyGrenadePointer();
			if ( pGrenade && pGrenade->GetThrower() && pGrenade->GetThrower()->IsPlayer() && pCheckerNPC->LikesMaster() )
				return pGrenade->GetThrower() == pCheckerNPC->GetMasterPlayer() || PlayerRelationship(pCheckerNPC->GetMasterPlayer(),ToHL2MPPlayer(pGrenade->GetThrower())) == GR_TEAMMATE;
			else
				return false;
		}
	}
	else if ( pChecker->IsGrenade() )
	{
		CBaseGrenade *pCheckerGrenade = pChecker->MyGrenadePointer();
		bool checkerHasMaster = ( pCheckerGrenade && pCheckerGrenade->GetThrower() && pCheckerGrenade->GetThrower()->IsPlayer() );
		
		if ( pTarget->IsPlayer() && checkerHasMaster )
			return pCheckerGrenade->GetThrower() == pTarget || PlayerRelationship(ToHL2MPPlayer(pCheckerGrenade->GetThrower()),pTarget) == GR_TEAMMATE;
		
		else if ( pTarget->IsNPC() )
		{
			CAI_BaseNPC *pNPC = pTarget->MyNPCPointer();
			if ( pNPC && pNPC->LikesMaster() && pNPC->GetMasterPlayer() )
				return pCheckerGrenade->GetThrower() == pNPC->GetMasterPlayer() || PlayerRelationship(ToHL2MPPlayer(pCheckerGrenade->GetThrower()),pNPC->GetMasterPlayer()) == GR_TEAMMATE;
			else
				return true; // we are both PVM monsters
		}

		else if ( pTarget->IsGrenade() )
		{
			CBaseGrenade *pGrenade = pTarget->MyGrenadePointer();
			if ( pGrenade && pGrenade->GetThrower() && pGrenade->GetThrower()->IsPlayer() )
				return pCheckerGrenade->GetThrower() == pGrenade->GetThrower() || PlayerRelationship(ToHL2MPPlayer(pCheckerGrenade->GetThrower()),ToHL2MPPlayer(pGrenade->GetThrower())) == GR_TEAMMATE;
			else
				return false;
		}
	}
	
//	Msg("Unknown stuff not friendly\n");
	return false;
}

bool CHL2MPRules::GameModeUsesMonsters(int mode)
{
	return mode == RANDOM_PVM || mode == FFA || mode == HOARDER;
}

bool CHL2MPRules::GameModeUsesTeamplay(int mode)
{
	return mode == RANDOM_PVM || mode == TEAM_DEATHMATCH || mode == HOARDER;
}
	
bool CHL2MPRules::IsTeamplay( void )
{
	return GameModeUsesTeamplay(mc_gamemode.GetInt());
}

int CHL2MPRules::NumPlayers()
{
	int num = 0;
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
#ifdef CLIENT_DLL
		if ( g_PR == NULL )
			return NULL;

		if ( g_PR->IsConnected(i) && g_PR->GetFaction(i) != FACTION_NONE )
			num ++;
#else
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
		if ( !pPlayer || !pPlayer->IsInCharacter() || pPlayer->GetFaction() == FACTION_NONE )
			continue;
		num ++;
#endif
	}
	return num;
}

int CHL2MPRules::NumPlayersOnFaction(int faction)
{
	int num = 0;
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
#ifdef CLIENT_DLL
		if ( g_PR->IsConnected(i) && g_PR->GetFaction(i) == faction )
			num ++;
#else
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
		if ( !pPlayer || !pPlayer->IsInCharacter() || pPlayer->GetFaction() != faction )
			continue;
		num ++;
#endif
	}
	return num;
}

bool CHL2MPRules::IsFactionChangeAllowed(int fromFaction, int toFaction)
{
	if ( !IsTeamplay() || fromFaction == toFaction || fromFaction <= FACTION_NONE || fromFaction > NUM_FACTIONS || toFaction <= FACTION_NONE || toFaction > NUM_FACTIONS)
		return false;

	return NumPlayersOnFaction(fromFaction) > NumPlayersOnFaction(toFaction);
}

#ifndef CLIENT_DLL
CUtlVector<McConVar*> mcConVarList;
void McConVar::AddMeToList()
{
	mcConVarList.AddToTail(this);
}

void McConVar::RemoveMeFromList()
{
//	mcConVarList.Remove(this);
}

void mc_use_defaults_changed( IConVar *var, const char *pOldValue, float flOldValue )
{
	if ( mc_use_defaults.GetBool() )
	{// force all McConVars to default
		McConVar::SetAllDefault();
	}
	else
		nonDefaultConvarsHaveBeenOnThisGame = true;
}

void McConVar::CheckAllDefault()
{
	for ( int i=0; i<mcConVarList.Size(); i++ )
		if ( !mcConVarList.Element(i)->IsDefault() )
		{
			mc_use_defaults.SetValue(0);
			return;
		}
	mc_use_defaults.SetValue(1);
}

void McConVar::SetAllDefault()
{
	for ( int i=0; i<mcConVarList.Size(); i++ )
	{
		McConVar *m = mcConVarList.Element(i);
		if ( !m->IsDefault() )
		{
			m->SetValue(m->GetDefault());
			Msg("Reset %s to %s\n", m->GetName(), m->GetDefault());
		}
	}
	//mc_use_defaults.SetValue(1); // having this here would cause an infinate loop with mc_use_defaults_changed
}

void McConVar::OnChanged()
{
	if ( AreAllDefault() )
	{
		if ( !IsDefault() )
			mc_use_defaults.SetValue(0);
	}
	else if ( IsDefault() )
		CheckAllDefault();
}
#endif

bool McConVar::AreAllDefault()
{
	return mc_use_defaults.GetBool();
}

#ifdef CLIENT_DLL
	#define UTIL_VarArgs VarArgs
#endif

const char *McConVar::GetHelpText() const
{
	if ( mc_use_defaults.GetBool() )
		return UTIL_VarArgs("[Locked] %s", ConVar::GetHelpText());
	return ConVar::GetHelpText();
}

#ifndef CLIENT_DLL
#define SAY_TO_CONSOLE( text ) if ( pPlayer ) ClientPrint(pPlayer, HUD_PRINTCONSOLE, text ); else Msg( text );
CON_COMMAND( mc_save_convars, "Saves all modified game balance convars to <file> (no spaces!)" )
{
	// is this restricted to rcon / host by default? I'm not actually sure... this command certainly SHOULD be! Clients spamming this would be bad.
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );
	if ( pPlayer != NULL && pPlayer != UTIL_GetListenServerHost() )
	{
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "This command requires rcon access\n" );
		return; // only the host (or the console) can run this command... it should require rcon
	}
	
	bool anyChanged = false;
	char dataDump[32568]; // this was too short at 16284!
	Q_snprintf(dataDump, sizeof(dataDump), "");
	for ( int i=0; i<mcConVarList.Size(); i++ )
	{
		McConVar *m = mcConVarList.Element(i);
		if ( !m->IsDefault() )
		{
			Q_strcat(dataDump, UTIL_VarArgs("%s %s\r\n", m->GetName(), m->GetString()), sizeof(dataDump));
			anyChanged = true;
		}
	}
	if ( !anyChanged )
	{
		SAY_TO_CONSOLE("No changes to save; all restricted convars are set to default!\n");
		return;
	}
	
	// ok, we've got all our data in dataDump... now save it to a file
	//CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
	//filesystem->WriteFile( "cfg/test.cfg", "MOD", buf )
	
	FileHandle_t fh;
	const char *filename = args.ArgC() > 1 ? UTIL_VarArgs("cfg/%s", args[1]) : "cfg/saved_convars.cfg";
	
	fh = filesystem->Open( filename, "w", "MOD");
	if (fh)
	{
	    filesystem->FPrintf(fh, dataDump);
	    filesystem->Close(fh);
		SAY_TO_CONSOLE(UTIL_VarArgs("Saved changes to %s\n", filename));
	}
	else
	{
		SAY_TO_CONSOLE("Unable to open file for writing!\n");
	}
}

CON_COMMAND( mc_save_convar_list, "Saves defaults of all game balance convars to <file> (no spaces!)" )
{
	// is this restricted to rcon / host by default? I'm not actually sure... this command certainly SHOULD be! Clients spamming this would be bad.
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );
	if ( pPlayer != NULL && pPlayer != UTIL_GetListenServerHost() )
	{
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "This command requires rcon access\n" );
		return; // only the host (or the console) can run this command... it should require rcon
	}
   
	char dataDump[32568];
	Q_snprintf(dataDump, sizeof(dataDump), "");
	for ( int i=0; i<mcConVarList.Size(); i++ )
	{
		McConVar *m = mcConVarList.Element(i);
		Q_strcat(dataDump, UTIL_VarArgs("%s %s\r\n", m->GetName(), m->GetDefault()), sizeof(dataDump));
	}
   
	FileHandle_t fh;
	const char *filename = args.ArgC() > 1 ? UTIL_VarArgs("cfg/%s", args[1]) : "cfg/saved_convars.cfg";
   
	fh = filesystem->Open( filename, "w", "MOD");
	if (fh)
	{
		filesystem->FPrintf(fh, dataDump);
		filesystem->Close(fh);
		SAY_TO_CONSOLE(UTIL_VarArgs("Saved convar list to %s\n", filename));
	}
	else
	{
		SAY_TO_CONSOLE("Unable to open file for writing!\n");
	}
}

CON_COMMAND( mc_save_module_list, "Saves command name and display name of every module to <file> (no spaces!)" )
{
    CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );
    if ( pPlayer != NULL && pPlayer != UTIL_GetListenServerHost() )
    {
        ClientPrint(pPlayer, HUD_PRINTCONSOLE, "This command requires rcon access\n" );
        return; // only the host (or the console) can run this command... it should require rcon
    }

    char dataDump[32568];
    Q_snprintf(dataDump, sizeof(dataDump), "");
    for ( int i=0; i<GetNumModules(); i++ )
    {
        Module *m = GetModule(i);
        Q_strcat(dataDump, UTIL_VarArgs("%s %s\r\n", m->GetCmdName(), m->GetDisplayName()), sizeof(dataDump));
    }

    FileHandle_t fh;
    const char *filename = args.ArgC() > 1 ? UTIL_VarArgs("cfg/%s", args[1]) : "cfg/module_list.txt";

    fh = filesystem->Open( filename, "w", "MOD");
    if (fh)
    {
        filesystem->FPrintf(fh, dataDump);
        filesystem->Close(fh);
        SAY_TO_CONSOLE(UTIL_VarArgs("Saved module list to %s\n", filename));
    }
    else
    {
        SAY_TO_CONSOLE("Unable to open file for writing!\n");
    }
}
#endif

const char *CHL2MPRules::GetGameDescription( void )
{
	char mode[48];
	switch ( mc_gamemode.GetInt() )
	{
	case 1: Q_snprintf(mode, sizeof(mode), "MC Deathmatch"); break;
	case 2: Q_snprintf(mode, sizeof(mode), "MC Players vs Monsters"); break;
	case 3: Q_snprintf(mode, sizeof(mode), "MC Free-For-All"); break;
	case 4: Q_snprintf(mode, sizeof(mode), "MC Team Deathmatch"); break;
	case 5: Q_snprintf(mode, sizeof(mode), "MC Hoarder"); break;
	default: Q_snprintf(mode, sizeof(mode), "Modular Combat"); break;
	}
	return UTIL_VarArgs("%s%s", McConVar::AreAllDefault() ? "" : "[Modified] ", mode);
} 


float CHL2MPRules::GetMapRemainingTime()
{
	// if timelimit is disabled, return 0
	if ( mp_timelimit.GetInt() <= 0 )
		return 0;

	// timelimit is in minutes

	float timeleft = (m_flGameStartTime + mp_timelimit.GetInt() * 60.0f ) - gpGlobals->curtime;

	return timeleft;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPRules::Precache( void )
{
	CBaseEntity::PrecacheScriptSound( "AlyxEmp.Charge" );
	CBaseEntity::PrecacheScriptSound( "BoSS.Execution" );
#ifndef CLIENT_DLL
	LoadMonsterStats();
#endif
}

bool CHL2MPRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		swap(collisionGroup0,collisionGroup1);
	}

	if ( (collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT) &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	// weapons and NPCs don't collide
	if ( collisionGroup0 == COLLISION_GROUP_WEAPON && (collisionGroup1 >= HL2COLLISION_GROUP_FIRST_NPC && collisionGroup1 <= HL2COLLISION_GROUP_LAST_NPC ) )
		return false;

	// Prevent the player movement from colliding with spit globs (caused the player to jump on top of globs while in water)
	if ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT && collisionGroup1 == HL2COLLISION_GROUP_SPIT )
		return false;
/*
	// Spit doesn't touch other spit
	if ( collisionGroup0 == HL2COLLISION_GROUP_SPIT && collisionGroup1 == HL2COLLISION_GROUP_SPIT )
		return false;
*/
	// spit *DOES* touch weapons, so that it hits turrets
	if ( collisionGroup0 == COLLISION_GROUP_WEAPON && collisionGroup1 == COLLISION_GROUP_PROJECTILE )
		return true;

	// crows do not collide
	if ( collisionGroup1 == HL2COLLISION_GROUP_CROW && ( collisionGroup0 == COLLISION_GROUP_PLAYER || COLLISION_GROUP_PLAYER_MOVEMENT ) )
		return false;

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}

bool CHL2MPRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
#ifndef CLIENT_DLL
	if( BaseClass::ClientCommand( pEdict, args ) )
		return true;


	CHL2MP_Player *pPlayer = (CHL2MP_Player *) pEdict;

	if ( pPlayer->ClientCommand( args ) )
		return true;
#endif

	return false;
}

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;
	
	if ( !bInitted )
	{
		bInitted = true;
//																								plr dmg		npc dmg	max carry	impulse
		def.AddAmmoType("AR2",				DMG_ENERGYBEAM,				TRACER_LINE_AND_WHIZ,	0,			35,		90,			BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("AR2AltFire",		DMG_DISSOLVE,				TRACER_NONE,			0,			0,		3,			0,							0 );
		def.AddAmmoType("Pistol",			DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			18,		168,		BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("SMG1",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			10,		270,		BULLET_IMPULSE(200, 1225),	0 );
		def.AddAmmoType("357",				DMG_BULLET,					TRACER_LINE_AND_WHIZ,	0,			0,		18,			BULLET_IMPULSE(800, 5000),	0 );
		def.AddAmmoType("XBowBolt",			DMG_BULLET,					TRACER_LINE,			0,			140,	11,			BULLET_IMPULSE(800, 8000),	0 );
		def.AddAmmoType("Buckshot",			DMG_BULLET | DMG_BUCKSHOT,	TRACER_LINE,			0,			8,		36,			BULLET_IMPULSE(400, 1200),	0 );
		def.AddAmmoType("RPG_Round",		DMG_BURN,					TRACER_NONE,			0,			180,	3,			0,							0 ); // reduced max carry by 1, to compensate for setting clip size = 1
		def.AddAmmoType("SMG1_Grenade",		DMG_BURN,					TRACER_NONE,			0,			115,	3,			0,							0 );
		def.AddAmmoType("Grenade",			DMG_BURN,					TRACER_NONE,			0,			135,	5,			0,							0 );
		def.AddAmmoType("slam",				DMG_BURN,					TRACER_NONE,			0,			0,		5,			0,							0 );

		def.AddAmmoType("AlyxGun",			DMG_BULLET,					TRACER_LINE,			"sk_plr_dmg_alyxgun",		"sk_npc_dmg_alyxgun",		"sk_max_alyxgun",		BULLET_IMPULSE(200, 1225), 0 );
		def.AddAmmoType("SniperRound",		DMG_BULLET | DMG_SNIPER,	TRACER_NONE,			"sk_plr_dmg_sniper_round",	"sk_npc_dmg_sniper_round",	"sk_max_sniper_round",	BULLET_IMPULSE(650, 6000), 0 );
		def.AddAmmoType("SniperPenetratedRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE,			"sk_dmg_sniper_penetrate_plr", "sk_dmg_sniper_penetrate_npc", "sk_max_sniper_round", BULLET_IMPULSE(150, 6000), 0 );
		def.AddAmmoType("Grenade",			DMG_BURN,					TRACER_NONE,			"sk_plr_dmg_grenade",		"sk_npc_dmg_grenade",		"sk_max_grenade",		0, 0);
		def.AddAmmoType("Thumper",			DMG_SONIC,					TRACER_NONE,			10, 10, 2, 0, 0 );
		def.AddAmmoType("Gravity",			DMG_CLUB,					TRACER_NONE,			0,	0, 8, 0, 0 );
		def.AddAmmoType("Battery",			DMG_CLUB,					TRACER_NONE,			NULL, NULL, NULL, 0, 0 );
//		def.AddAmmoType("GaussEnergy",		DMG_SHOCK,					TRACER_NONE,			"sk_jeep_gauss_damage",		"sk_jeep_gauss_damage", "sk_max_gauss_round", BULLET_IMPULSE(650, 8000), 0 ); // hit like a 10kg weight at 400 in/s
		def.AddAmmoType("CombineCannon",	DMG_BULLET,					TRACER_LINE,			"sk_npc_dmg_gunship_to_plr", "sk_npc_dmg_gunship", NULL, 1.5 * 750 * 12, 0 ); // hit like a 1.5kg weight at 750 ft/s
		def.AddAmmoType("AirboatGun",		DMG_AIRBOAT,				TRACER_LINE,			"sk_plr_dmg_airboat",		"sk_npc_dmg_airboat",		NULL,					BULLET_IMPULSE(10, 600), 0 );
		def.AddAmmoType("StriderMinigun",	DMG_BULLET,					TRACER_LINE,			5, 15,15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType("StriderMinigunDirect",	DMG_BULLET,				TRACER_LINE,			2, 2, 15, 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType("HelicopterGun",	DMG_BULLET,					TRACER_LINE_AND_WHIZ,	"sk_npc_dmg_helicopter_to_plr", "sk_npc_dmg_helicopter",	"sk_max_smg1",	BULLET_IMPULSE(400, 1225), AMMO_FORCE_DROP_IF_CARRIED | AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER );
#ifdef HL2_EPISODIC
		def.AddAmmoType("Hopwire",			DMG_BLAST,					TRACER_NONE,			"sk_plr_dmg_grenade",		"sk_npc_dmg_grenade",		"sk_max_hopwire",		0, 0);
		def.AddAmmoType("CombineHeavyCannon",	DMG_BULLET,				TRACER_LINE,			40,	40, NULL, 10 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 10 kg weight at 750 ft/s
		def.AddAmmoType("ammo_proto1",			DMG_BULLET,				TRACER_LINE,			0, 0, 10, 0, 0 );
#endif // HL2_EPISODIC
	}

	return &def;
}

#ifdef CLIENT_DLL
	// Default: 1, set to 0 due to beta-tester complaints.
	ConVar cl_autowepswitch("cl_autowepswitch", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Automatically switch to picked up weapons (if more powerful)" );
	ConVar cl_showexpgained("cl_showexpgained", "1", FCVAR_ARCHIVE | FCVAR_USERINFO, "Show experience gained in hud chat." );
	ConVar cl_expgainedmessage("cl_expgainedmessage", "1", FCVAR_ARCHIVE | FCVAR_USERINFO, "Controls the length of the 'experience gained' message used. 1 is the longest, 4 is the shortest", true, 1, true, 4);
	ConVar cl_show_damage_scale_messages("cl_show_damage_scale_messages", "1", FCVAR_ARCHIVE | FCVAR_USERINFO, "Controls whether messages are shown to indicate the players damage dealt/received scales. 1 shows them in chat, 2 in the console, and 0 disables them", true, 0, true, 2);
#else
	bool CHL2MPRules::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon )
	{		
		if ( pPlayer->GetActiveWeapon() && pPlayer->IsNetClient() )
		{
			// Player has an active item, so let's check cl_autowepswitch.
			const char *cl_autowepswitch = engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), "cl_autowepswitch" );
			if ( cl_autowepswitch && atoi( cl_autowepswitch ) <= 0 )
			{
				return false;
			}
		}

		return BaseClass::FShouldSwitchWeapon( pPlayer, pWeapon );
	}

#endif

#ifndef CLIENT_DLL
char *szRandomSpawnTypes[NUM_RANDOM_START_TYPES] = { "info_player_*", "info_start_*", "info_npc_start", "info_node" };

// find a suitable entity in the world (player spawn, ammo/weapon/health pickup, etc),
// and then shoot off in a random direction from it, horizontally.
// travel up a bit
// travel some distance in another horizontal direction
// drop straight to the ground
// travel up a foot or so
Vector CHL2MPRules::GetRandomSpawnPoint(Vector hullMin, Vector hullMax, int triesLeft, CBaseEntity *pEntIgnore)
{
	if ( iNumRandomStartTypes_OnThisMap <= 0 ) // nothing to start at!
		if ( pEntIgnore != NULL )
			return pEntIgnore->GetAbsOrigin(); // bah, stay put
		else
			return vec3_origin;

	int typeNum;
	do
	{
		typeNum = random->RandomInt(0,NUM_RANDOM_START_TYPES-1);
	}
	while ( bDisabledRandomSpawnStartTypes[typeNum] == true ); // wait til we find a type we haven't already marked as missing from this map
	char *pSpawnType = szRandomSpawnTypes[typeNum];


	int numSteps = 1;
	if ( triesLeft < 0 ) // this is the first attempt, randomize start spawn a little
	{
		numSteps = random->RandomInt(1,4);
		triesLeft = 4;
	}

	// if our last attempt failed, we need to use a different start point just in case this one is unusable
	CBaseEntity *pSpawnEnt = pLastRandomSpawnStarts[typeNum];
	//Msg(UTIL_VarArgs("Random spawn starting at type #%i\n", typeNum));
	for ( int i=0; i<numSteps; i++ )
	{
		pSpawnEnt = gEntList.FindEntityByClassname(pSpawnEnt,pSpawnType);
		if ( pSpawnEnt == NULL )
		{
			pSpawnEnt = gEntList.FindEntityByClassname(pSpawnEnt,pSpawnType);
			if ( pSpawnEnt == NULL )
			{
				// if we can't find anything of this type
				// mark that type as blocked off, and try again
				bDisabledRandomSpawnStartTypes[typeNum] = true;
				iNumRandomStartTypes_OnThisMap--;
				
				return GetRandomSpawnPoint(hullMin, hullMax, triesLeft, pEntIgnore);
			}
		}
	}
	pLastRandomSpawnStarts[typeNum] = pSpawnEnt;


	Vector startPoint = pSpawnEnt->GetAbsOrigin();

	// if we've tried random points too many times and failed, just return the spawn point
	if ( triesLeft == 0 )
	{
		Msg("Tried random position 5 times and failed!\n");
		pSpawnEnt = NULL;
		return startPoint;
	}


	// ok, now we have an entity, need to get a set distance from it.
	// firstly, shoot off 4 horizontal traces, & select the one that travels the furthest distance
	// so they aren't absolute North South East West, make the first one random, & base the others off of it.
	Vector dir1 = vec3_origin;
	while ( dir1 == vec3_origin ) // if (somehow) both randoms were zero, it'll keep trying again
	{// make dir be a random horizontal vector
		dir1.x = random->RandomFloat(-1.0f,1.0f);
		dir1.y = random->RandomFloat(-1.0f,1.0f);
	}
	VectorNormalize( dir1 );
	Vector dir2 = Vector(dir1.y,-dir1.x,0);
	Vector dir3 = -dir1;
	Vector dir4 = -dir2;
	
	trace_t tr1, tr2, tr3, tr4;

	UTIL_TraceLine( startPoint, startPoint + dir1 * 8192.0, MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr1 );
	UTIL_TraceLine( startPoint, startPoint + dir2 * 8192.0, MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr2 );
	UTIL_TraceLine( startPoint, startPoint + dir3 * 8192.0, MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr3 );
	UTIL_TraceLine( startPoint, startPoint + dir4 * 8192.0, MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr4 );

	// see which of these travelled the furthest, force this to be dir1 & tr1
	if ( tr2.fraction > tr1.fraction )
	{
		tr1 = tr2;
		dir1 = dir2;
	}
	if ( tr3.fraction > tr1.fraction )
	{
		tr1 = tr3;
		dir1 = dir3;
	}
	if ( tr4.fraction > tr1.fraction )
	{
		tr1 = tr4;
		dir1 = dir4;
	}

	startPoint = startPoint + dir1 * 8192.0 * tr1.fraction * random->RandomFloat(0.45f,0.9f); // travel some distance in this furthest direction
    

	// go up somewhat
	UTIL_TraceLine( startPoint, startPoint + Vector(0,0,1280), MASK_PLAYERSOLID, NULL, COLLISION_GROUP_PLAYER_MOVEMENT, &tr1 );
	startPoint = startPoint + Vector(0,0,1280) * tr1.fraction * random->RandomFloat(0.0f,0.9f);

	// now select & travel in another random horizontal direction
	dir1 = vec3_origin;
	while ( dir1 == vec3_origin ) // if (somehow) both randoms were zero, it'll keep trying again
	{// make dir be a random horizontal vector
		dir1.x = random->RandomFloat(-1.0f,1.0f);
		dir1.y = random->RandomFloat(-1.0f,1.0f);
	}
	VectorNormalize( dir1 );
	dir2 = Vector(dir1.y,-dir1.x,0);
	dir3 = -dir1;
	dir4 = -dir2;

	UTIL_TraceLine( startPoint, startPoint + dir1 * 2048.0, MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr1 );
	UTIL_TraceLine( startPoint, startPoint + dir2 * 2048.0, MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr2 );
	UTIL_TraceLine( startPoint, startPoint + dir3 * 2048.0, MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr3 );
	UTIL_TraceLine( startPoint, startPoint + dir4 * 2048.0, MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr4 );

	// see which of these travelled the furthest, force this to be dir1 & tr1
	if ( tr2.fraction > tr1.fraction )
	{
		tr1 = tr2;
		dir1 = dir2;
	}
	if ( tr3.fraction > tr1.fraction )
	{
		tr1 = tr3;
		dir1 = dir3;
	}
	if ( tr4.fraction > tr1.fraction )
	{
		tr1 = tr4;
		dir1 = dir4;
	}

	startPoint = startPoint + dir1 * 2048.0 * tr1.fraction * random->RandomFloat(0.1f,0.9f); // travel some distance in this furthest direction


	// now drop straight down to the floor
	UTIL_TraceLine( startPoint, startPoint + Vector(0,0,-8192), MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr1 );
	startPoint = tr1.endpos;


	// lastly, come up just a little
	UTIL_TraceLine( startPoint, startPoint + Vector(0,0,8), MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr1 );

	Vector spawnPoint = tr1.endpos;

	if ( hullMin != vec3_origin || hullMax != vec3_origin )
	{
		UTIL_TraceHull( spawnPoint, spawnPoint, hullMin, hullMax, MASK_PLAYERSOLID, pEntIgnore, COLLISION_GROUP_PLAYER_MOVEMENT, &tr2 );
		if ( tr2.startsolid ) // need to try again
		{
			if ( triesLeft < 0 ) triesLeft = 4;
			spawnPoint = GetRandomSpawnPoint(hullMin, hullMax, triesLeft-1, pEntIgnore);
		}
	}
	
	pSpawnEnt = NULL; // clear this again
	return spawnPoint;
}

// essentially, only the y component should be randomized
QAngle CHL2MPRules::GetRandomAngle()
{
	return QAngle(0, random->RandomInt(0,360), 0);
}

void CHL2MPRules::RestartGame()
{
	// bounds check
	if ( mp_timelimit.GetInt() < 0 )
	{
		mp_timelimit.SetValue( 0 );
	}
	m_flGameStartTime = gpGlobals->curtime;
	if ( !IsFinite( m_flGameStartTime.Get() ) )
	{
		Warning( "Trying to set a NaN game start time\n" );
		m_flGameStartTime.GetForModify() = 0.0f;
	}

	CleanUpMap();
	
	// now respawn all players
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer || pPlayer->IsObserver() )
			continue;

		if ( pPlayer->GetActiveWeapon() )
		{
			pPlayer->GetActiveWeapon()->Holster();
		}
		pPlayer->RemoveAllItems( true );
		respawn( pPlayer, false );
		pPlayer->Reset();
	}

	// Respawn entities (glass, doors, etc..)

	/*CTeam *pRebels = GetGlobalTeam( TEAM_REBELS );
	CTeam *pCombine = GetGlobalTeam( TEAM_COMBINE );

	if ( pRebels )
	{
		pRebels->SetScore( 0 );
	}

	if ( pCombine )
	{
		pCombine->SetScore( 0 );
	}*/

	m_flIntermissionEndTime = 0;
	m_flRestartGameTime = 0.0;		
	m_bCompleteReset = false;

	IGameEvent * event = gameeventmanager->CreateEvent( "round_start" );
	if ( event )
	{
		event->SetInt("fraglimit", 0 );
		event->SetInt( "priority", 6 ); // HLTV event priority, not transmitted

		event->SetString("objective","DEATHMATCH");

		gameeventmanager->FireEvent( event );
	}
}

bool FindInList( const char **pStrings, const char *pToFind )
{
	int i = 0;
	while ( pStrings[i][0] != 0 )
	{
		if ( Q_stricmp( pStrings[i], pToFind ) == 0 )
			return true;
		i++;
	}

	return false;
}

void CHL2MPRules::CleanUpMap()
{
	// Recreate all the map entities from the map data (preserving their indices),
	// then remove everything else except the players.

	// Get rid of all entities except players.
	CBaseEntity *pCur = gEntList.FirstEnt();
	while ( pCur )
	{
		CBaseHL2MPCombatWeapon *pWeapon = dynamic_cast< CBaseHL2MPCombatWeapon* >( pCur );
		// Weapons with owners don't want to be removed..
		if ( pWeapon )
		{
			if ( !pWeapon->GetPlayerOwner() )
			{
				UTIL_Remove( pCur );
			}
		}
		// remove entities that has to be restored on roundrestart (breakables etc)
		else if ( !FindInList( s_PreserveEnts, pCur->GetClassname() ) )
		{
			UTIL_Remove( pCur );
		}

		pCur = gEntList.NextEnt( pCur );
	}

	// Really remove the entities so we can have access to their slots below.
	gEntList.CleanupDeleteList();

	// Cancel all queued events, in case a func_bomb_target fired some delayed outputs that
	// could kill respawning CTs
	g_EventQueue.Clear();

	// Now reload the map entities.
	class CHL2MPMapEntityFilter : public IMapEntityFilter
	{
	public:
		virtual bool ShouldCreateEntity( const char *pClassname )
		{
			// Don't recreate the preserved entities.
			if ( !FindInList( s_PreserveEnts, pClassname ) )
			{
				return true;
			}
			else
			{
				// Increment our iterator since it's not going to call CreateNextEntity for this ent.
				if ( m_iIterator != g_MapEntityRefs.InvalidIndex() )
					m_iIterator = g_MapEntityRefs.Next( m_iIterator );

				return false;
			}
		}


		virtual CBaseEntity* CreateNextEntity( const char *pClassname )
		{
			if ( m_iIterator == g_MapEntityRefs.InvalidIndex() )
			{
				// This shouldn't be possible. When we loaded the map, it should have used 
				// CCSMapLoadEntityFilter, which should have built the g_MapEntityRefs list
				// with the same list of entities we're referring to here.
				Assert( false );
				return NULL;
			}
			else
			{
				CMapEntityRef &ref = g_MapEntityRefs[m_iIterator];
				m_iIterator = g_MapEntityRefs.Next( m_iIterator );	// Seek to the next entity.

				if ( ref.m_iEdict == -1 || engine->PEntityOfEntIndex( ref.m_iEdict ) )
				{
					// Doh! The entity was delete and its slot was reused.
					// Just use any old edict slot. This case sucks because we lose the baseline.
					return CreateEntityByName( pClassname );
				}
				else
				{
					// Cool, the slot where this entity was is free again (most likely, the entity was 
					// freed above). Now create an entity with this specific index.
					return CreateEntityByName( pClassname, ref.m_iEdict );
				}
			}
		}

	public:
		int m_iIterator; // Iterator into g_MapEntityRefs.
	};
	CHL2MPMapEntityFilter filter;
	filter.m_iIterator = g_MapEntityRefs.Head();

	// DO NOT CALL SPAWN ON info_node ENTITIES!

	MapEntity_ParseAllEntities( engine->GetMapEntitiesString(), &filter, true );
}

void CHL2MPRules::CheckChatForReadySignal( CHL2MP_Player *pPlayer, const char *chatmsg )
{
	if( m_bAwaitingReadyRestart && FStrEq( chatmsg, mp_ready_signal.GetString() ) )
	{
		if( !pPlayer->IsReady() )
		{
			pPlayer->SetReady( true );
		}		
	}
}

void CHL2MPRules::CheckRestartGame( void )
{
	// Restart the game if specified by the server
	int iRestartDelay = mp_restartgame.GetInt();

	if ( iRestartDelay > 0 )
	{
		if ( iRestartDelay > 60 )
			iRestartDelay = 60;


		// let the players know
		char strRestartDelay[64];
		Q_snprintf( strRestartDelay, sizeof( strRestartDelay ), "%d", iRestartDelay );
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "Game will restart in %s1 %s2", strRestartDelay, iRestartDelay == 1 ? "SECOND" : "SECONDS" );
		UTIL_ClientPrintAll( HUD_PRINTCONSOLE, "Game will restart in %s1 %s2", strRestartDelay, iRestartDelay == 1 ? "SECOND" : "SECONDS" );

		m_flRestartGameTime = gpGlobals->curtime + iRestartDelay;
		m_bCompleteReset = true;
		mp_restartgame.SetValue( 0 );
	}

	if( mp_readyrestart.GetBool() )
	{
		m_bAwaitingReadyRestart = true;
		m_bHeardAllPlayersReady = false;
		

		const char *pszReadyString = mp_ready_signal.GetString();


		// Don't let them put anything malicious in there
		if( pszReadyString == NULL || Q_strlen(pszReadyString) > 16 )
		{
			pszReadyString = "ready";
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "hl2mp_ready_restart" );
		if ( event )
			gameeventmanager->FireEvent( event );

		mp_readyrestart.SetValue( 0 );

		// cancel any restart round in progress
		m_flRestartGameTime = -1;
	}
}

void CHL2MPRules::CheckAllPlayersReady( void )
{
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;
		if ( !pPlayer->IsReady() )
			return;
	}
	m_bHeardAllPlayersReady = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CHL2MPRules::GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer )
{
	if ( !pPlayer )  // dedicated server output
	{
		return NULL;
	}

	const char *pszFormat = NULL;

	// team only
	if ( bTeamOnly == TRUE )
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		{
			pszFormat = "HL2MP_Chat_Spec";
		}
		else
		{
			const char *chatLocation = GetChatLocation( bTeamOnly, pPlayer );
			if ( chatLocation && *chatLocation )
			{
				pszFormat = "HL2MP_Chat_Team_Loc";
			}
			else
			{
				pszFormat = "HL2MP_Chat_Team";
			}
		}
	}
	// everyone
	else
	{
		if ( pPlayer->GetTeamNumber() != TEAM_SPECTATOR )
		{
			pszFormat = "HL2MP_Chat_All";	
		}
		else
		{
			pszFormat = "HL2MP_Chat_AllSpec";
		}
	}

	return pszFormat;
}

void CHL2MPRules::InitDefaultAIRelationships( void )
{
	int i, j;

	//  Allocate memory for default relationships
	CBaseCombatCharacter::AllocateDefaultRelationships();

	// By default all relationships are friendly, of priority zero. except players

	// No. Just making everything hating everything else	-- So much pent up anger there mate(Josh)
	for (i=0;i<NUM_AI_CLASSES;i++)
	{
		for (j=0;j<NUM_AI_CLASSES;j++)
		{
			CBaseCombatCharacter::SetDefaultRelationship( (Class_T)i, (Class_T)j, D_HT, 0 );
		}
	}
}

void CHL2MPRules::StartSpreeWar(CHL2MP_Player *pSpreer)
{
	m_hSpreer = pSpreer;

	// go through all players, and apply SpreeWar affliction if they're not the spreer
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
		if ( !pPlayer || !pPlayer->IsInCharacter() || pPlayer == GetSpreer() )
			continue;

		pPlayer->ApplyBuff(BUFF_SPREE_WAR);

	}

	UTIL_ClientPrintAll(HUD_PRINTALL,"%s is on a spree war!\n",pSpreer->GetPlayerName());
	UTIL_ClientPrintAll(HUD_PRINTTALK,"All players are now allied against %s!\n",pSpreer->GetPlayerName());

	IGameEvent * event = gameeventmanager->CreateEvent( "spreewar_start" );
	if ( event )
	{
		event->SetInt("userid", pSpreer->GetUserID() );
		gameeventmanager->FireEvent( event );
	}
}

void CHL2MPRules::EndSpreeWar(bool alsoEndAllSprees)
{
	m_hSpreer = NULL;

	// go through all players, and remove SpreeWar affliction from them all
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
		if ( !pPlayer || !pPlayer->IsInCharacter() )
			continue;

		pPlayer->RemoveBuff(BUFF_SPREE_WAR);
		if ( alsoEndAllSprees )
			pPlayer->ResetSpree();
	}

	UTIL_ClientPrintAll(HUD_PRINTTALKCONSOLE,"Spree war finished\n");

	// event for stat and omni-bot usage
	IGameEvent * event = gameeventmanager->CreateEvent( "spreewar_end" );
	if ( event )
	{
		gameeventmanager->FireEvent( event );
	}
}

// needed so that spree war allies don't hurt each other
bool CHL2MPRules::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
	if( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
	{
		return false;
	}

	if ( pAttacker && PlayerRelationship( pPlayer, pAttacker ) == GR_TEAMMATE )
	{
		if ( pAttacker != pPlayer )
		{
			// this hit came from someone other than myself, and we're friends, don't get hurt
			return false;
		}
	}

	return BaseClass::FPlayerCanTakeDamage( pPlayer, pAttacker );
}
#endif

CBaseCombatCharacter *CHL2MPRules::GetGravityWell(int i)
{
	if ( i < MAX_MAGMINES && i >= 0 )
	{
		CBaseEntity *e = m_hGravityWells[i].Get();
		if ( e == NULL )
			return NULL;
		return e->MyCombatCharacterPointer();
	}
	return NULL;
}

#ifndef CLIENT_DLL
void CHL2MPRules::AddGravityWell(CBaseEntity *e)
{
	int firstSpace = -1;
	for ( int i=0; i<MAX_MAGMINES; i++ )
		if ( m_hGravityWells.Get(i) == NULL )
		{
			firstSpace = i;
			break;
		}

	if ( firstSpace >= 0 )
		m_hGravityWells.Set(firstSpace,e);
	else
		Warning("Too many gravity wells, unable to create another!\n");
}

void CHL2MPRules::RemoveGravityWell(CBaseEntity *e)
{
	for ( int i=0; i<MAX_MAGMINES; i++ )
		if ( m_hGravityWells[i].Get() == e )
		{
			int j;
			for ( j=i; j<MAX_MAGMINES-1; j++ )
				m_hGravityWells.Set(j, m_hGravityWells[j+1].Get());
			m_hGravityWells.Set(j-1, NULL);
			break;
		}
}
#endif

/*
ConVar classtables_showvars( "classtables_showvars", "0", FCVAR_REPLICATED );

#ifndef CLIENT_DLL

extern ServerClass *g_pServerClassHead;
CON_COMMAND(classtables_Server, "Lists the server classes, and also their member variables if classtables_showvars is set")
{
	int start = 0, end = INT_MAX;
	if ( args.ArgC() > 2 )
	{
		start = atoi(args[1]);
		if ( args.ArgC() > 1 )
			end = atoi(args[2]);
	}

	int pos = 0;
	ServerClass *pClass = g_pServerClassHead;
	while ( pClass )
	{
		if ( pos >= start && pos <= end )
		{
			Msg(UTIL_VarArgs("%s (%i)\n",pClass->m_pNetworkName,pClass->m_ClassID));
			if ( classtables_showvars.GetInt()  > 0 )
			{				   
				SendTable *pTable = pClass->m_pTable;
				if ( pTable == NULL )
					continue;

				int num = pTable->GetNumProps();
				for ( int i=0; i<num; i++ )
				{
					SendProp *pProp = pTable->GetProp(i);
					Msg("	%s",pProp->GetName());
				}
				Msg("\n");
			}
		}
		pClass = pClass->m_pNext;
		pos ++;
	}
}

#else

extern ClientClass *g_pClientClassHead;
CON_COMMAND( classtables_client, "Lists the client classes, and also their member variables if classtables_showvars is set" )
{
	int start = 0, end = INT_MAX;
	if ( args.ArgC() > 2 )
	{
		start = atoi(args[1]);
		if ( args.ArgC() > 1 )
			end = atoi(args[2]);
	}

	int pos = 0;
	ClientClass *pClass = g_pClientClassHead;
	while ( pClass )
	{
		if ( pos >= start && pos <= end )
		{
			Msg(VarArgs("%s (%i)\n",pClass->m_pNetworkName,pClass->m_ClassID));
			if ( classtables_showvars.GetInt()  > 0 )
			{
				RecvTable *pTable = pClass->m_pRecvTable;
				if ( pTable == NULL )
					continue;

				int num = pTable->GetNumProps();
				for ( int i=0; i<num; i++ )
				{
					RecvProp *pProp = pTable->GetProp(i);
					Msg("	%s",pProp->GetName());
				}
				Msg("\n");
			}
		}
		pClass = pClass->m_pNext;
		pos ++;
	}
}

#endif
*/

float CHL2MPRules::GetFactionExperience(int faction)
{
	switch ( faction )
	{
	case FACTION_COMBINE:
		return m_flExpFactionCombine;
	case FACTION_RESISTANCE:
		return m_flExpFactionResistance;
	case FACTION_APERTURE:
		return m_flExpFactionAperture;
	default:
		return 0;
	}
}

const char *szPassHash1 = "X'a4%", *szPassHash2 = "@2dR.";
// so that we do not store unencrypted passwords, and neither do we send the same data as we check for on the server
char g_szCombinedPassHash[50];
const char *DoubleHashPassword(const char *passHash)
{
	Q_snprintf(g_szCombinedPassHash,sizeof(g_szCombinedPassHash), "%s%s%s", szPassHash1, passHash, szPassHash2);
	return g_szCombinedPassHash;
}

void ClearStoredPasswordHash()
{
	Q_snprintf(g_szCombinedPassHash, sizeof(g_szCombinedPassHash), "                                                "); // don't keep it in memory
}

// check that this string only uses allowed characters
// since using steam name, we now allow several additional characters
bool IsValidInputString(const char *test)
{
	if ( Q_strlen(test) < 1 )
	{
#ifdef CLIENT_DLL
		Msg("Input string is blank!\n");
#endif		
		return false;
	}

	std::string searchme = test; // std::string makes this much easier
	int size = searchme.size();
	for ( int i=0; i<size; i++ )
	{
		char c = searchme.at(i);
		if ( !( isalnum(c) || c == ' ' || c == '_' || c == '-' || c == '.' || c == ','
			|| c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}'
			|| c == '!' || c == '?' || c == '@' || c == '$' || c == '£' || c == ':' || c == ';'
			|| c == '%' || c == '\"' || c == '\'' || c == '+' || c == '=' || c == '*' ) )
		{// bad character
#ifdef CLIENT_DLL
			Msg("Character is disallowed: %c\n",c);
#endif
			return false;
		}
	}
	return true;
}