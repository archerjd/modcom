//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "hl2mp_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"
#include "KeyValues.h"
#include "team.h"
#include "weapon_hl2mpbase.h"
#include "grenade_satchel.h"
#include "eventqueue.h"
#include "GameStats.h"
#include "filesystem.h"
#include "modcom/mcconvar.h"
#include <string>

#include <cdll_int.h>

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"
#include "particle_parse.h"

#include "modcom/monsters.h"
#include "util/sqlite/dbhandler.h"

#ifdef USE_OMNIBOT
#include "../omnibot/omnibot_interface.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CBaseEntity	 *g_pLastCombineSpawn = NULL;
CBaseEntity	 *g_pLastResistanceSpawn = NULL;
CBaseEntity	 *g_pLastApertureSpawn = NULL;
bool g_bAllowCheckForScoreTokenVictory = true;

extern CBaseEntity				*g_pLastSpawn;
extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);

#ifndef _WIN32
	#define min(a,b) (a) < (b) ? a : b
	#define max(a,b) (a) > (b) ? a : b
#endif

#define AUTO_SAVE_INTERVAL 20.0f // in seconds

#define HL2MP_COMMAND_MAX_RATE 0.3

void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );

LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

LINK_ENTITY_TO_CLASS( info_player_combine, CPointEntity );
LINK_ENTITY_TO_CLASS( info_player_rebel, CPointEntity );

// Linux compile needs this
extern void SendProxy_Origin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

void* SendProxy_SendNonLocalDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->SetAllRecipients();
	pRecipients->ClearRecipient( objectID - 1 );
	return ( void * )pVarData;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendNonLocalDataTable );

BEGIN_SEND_TABLE_NOBASE( CHL2MP_Player, DT_HL2MPLocalPlayerExclusive )
	// send a hi-res origin to the local player for use in prediction
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),

	SendPropArray3( SENDINFO_ARRAY3(m_iModules), SendPropInt( SENDINFO_ARRAY(m_iModules), 4, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_bModulesActive), SendPropBool( SENDINFO_ARRAY(m_bModulesActive) ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_flModuleCooldownTime), SendPropFloat( SENDINFO_ARRAY(m_flModuleCooldownTime), 0, SPROP_NOSCALE ) ),
	SendPropInt( SENDINFO(m_iTotalExp), 19, SPROP_UNSIGNED ), // down to 19 for a 500k (level 25) limit
//	SendPropInt( SENDINFO(m_iGameExp), 19, SPROP_UNSIGNED ),  // if the client remembers the initial value of m_iTotalExp (telling it when to remember might be hard) then it could recalculate m_iGameExp without networking it!
	SendPropInt( SENDINFO( m_iModulePoints ), 7, SPROP_UNSIGNED ),
// 	SendPropInt( SENDINFO( m_iWeaponPoints ), 12, SPROP_UNSIGNED ), 
// 	SendPropInt( SENDINFO( m_iCredits ), 12, SPROP_UNSIGNED ), 

	SendPropInt( SENDINFO( m_iMaxAuxPower ), 8, SPROP_UNSIGNED ),

	SendPropInt( SENDINFO( m_iNumMinions ), 3, SPROP_UNSIGNED ), // only used for power drain display
	//SendPropBool( SENDINFO( m_bInSpawnProtect ) ),

	SendPropInt( SENDINFO( m_iLoggedOn ), 2, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iFaction ), 2, SPROP_UNSIGNED ),

	// send level and end time info on buffs to myself - for display on the buff bar
	SendPropArray3( SENDINFO_ARRAY3(m_iBuffs), SendPropInt( SENDINFO_ARRAY(m_iBuffs), BITS_FOR_MAX_BUFF_LEVEL, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_flBuffEndTimes), SendPropFloat( SENDINFO_ARRAY(m_flBuffEndTimes), 0, SPROP_NOSCALE ) ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CHL2MP_Player, DT_HL2MPNonLocalPlayerExclusive )
	// send a lo-res origin to other players
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),

	// only send a bool about active buffs to other players
	SendPropArray3( SENDINFO_ARRAY3(m_bBuffs), SendPropInt( SENDINFO_ARRAY(m_bBuffs) ) ),
END_SEND_TABLE()


IMPLEMENT_SERVERCLASS_ST(CHL2MP_Player, DT_HL2MP_Player)
	SendPropDataTable( "playerlocaldata", 0, &REFERENCE_SEND_TABLE(DT_HL2MPLocalPlayerExclusive), SendProxy_SendLocalDataTable ),
	SendPropDataTable( "playernonlocaldata", 0, &REFERENCE_SEND_TABLE(DT_HL2MPNonLocalPlayerExclusive), SendProxy_SendNonLocalDataTable ),

	SendPropInt( SENDINFO( m_iSprintSpeed ), 10, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iNumScoreTokens ), 7, SPROP_UNSIGNED ),

	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11, SPROP_CHANGES_OFTEN ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11, SPROP_CHANGES_OFTEN ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropInt( SENDINFO( m_iSpawnInterpCounter), 4 ),
	SendPropInt( SENDINFO( m_iPlayerSoundType), 3 ),
	
//	SendPropBool( SENDINFO( m_fIsWalking ) ),

	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),

	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
	
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
END_DATADESC()

const char *g_ppszRandomRebelModels[] = 
{
	"models/humans/group03/male_01.mdl",
	"models/humans/group03/male_01_head.mdl",
	"models/humans/group03/male_02.mdl",
	"models/humans/group03/female_01.mdl",
	"models/humans/group03/male_03.mdl",
	"models/humans/group03/female_02.mdl",
	"models/humans/group03/female_02_head.mdl",
	"models/humans/group03/male_04.mdl",
	"models/humans/group03/female_03.mdl",
	"models/humans/group03/female_03_head.mdl",
	"models/humans/group03/male_05.mdl",
	"models/humans/group03/female_04.mdl",
	"models/humans/group03/male_06.mdl",
	"models/humans/group03/female_06.mdl",
	"models/humans/group03/male_07.mdl",
	"models/humans/group03/male_07_head.mdl",
	"models/humans/group03/female_07.mdl",
	"models/humans/group03/male_08.mdl",
	"models/humans/group03/male_09.mdl",
};

const char *g_ppszRandomCombineModels[] =
{
	"models/combine_soldier.mdl",
	"models/combine_soldier2.mdl",
	"models/combine_soldier_prisonguard.mdl",
	"models/combine_soldier_prisonguard2.mdl",
	"models/combine_super_soldier.mdl",
	"models/police.mdl",
};

const char *g_ppszRandomApertureModels[] = 
{
	"models/humans/group01/male_01.mdl",
	"models/humans/group01/male_01_head_ap.mdl",
	"models/humans/group01/male_02.mdl",
	"models/humans/group01/female_01.mdl",
	"models/humans/group01/male_03.mdl",
	"models/humans/group01/female_02.mdl",
	"models/humans/group01/female_02_head_ap.mdl",
	"models/humans/group01/male_04.mdl",
	"models/humans/group01/female_03.mdl",
	"models/humans/group01/female_03_head_ap.mdl",
	"models/humans/group01/male_05.mdl",
	"models/humans/group01/female_04.mdl",
	"models/humans/group01/male_06.mdl",
	"models/humans/group01/female_06.mdl",
	"models/humans/group01/male_07.mdl",
	"models/humans/group01/male_07_head_ap.mdl",
	"models/humans/group01/female_07.mdl",
	"models/humans/group01/male_08.mdl",
	"models/humans/group01/male_09.mdl",
};

extern ConVar mc_gamemode;
extern ConVar mc_vote_gamemode_enabled;
extern ConVar mc_vote_map_enabled;

extern McConVar mc_perlevel_modulepoints, pvm_buff_interval_aperture, pvm_buff_interval_resistance, pvm_buff_combine_scale, mc_combo_kill_max_interval, mc_combo_kill_experience;
ConVar mc_votes_per_map("mc_votes_per_map", "3", FCVAR_GAMEDLL | FCVAR_NOTIFY, "How many times each player can start a vote, with the counter reset every map change. Set to 0 to disable", true, 0, false, 0 );

#define MODEL_CHANGE_INTERVAL 5.0f
#define TEAM_CHANGE_INTERVAL 30.0f

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

extern LEVEL_EXTERN(mod_vitality);
extern LEVEL_EXTERN(mod_armorcap);
extern LEVEL_EXTERN(mod_auxpower);
//extern LEVEL_EXTERN(mod_runningman);
extern McConVar mc_player_base_sprint_speed;

CHL2MP_Player::CHL2MP_Player() : m_PlayerAnimState( this )
{
	m_angEyeAngles.Init();

	m_iLastWeaponFireUsercmd = 0;

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;
	m_flLastPlayerKillTime = 0.0f;

	m_iSpawnInterpCounter = 0;

	Q_snprintf(m_szCharacterName,sizeof(m_szCharacterName), ""); // leaving character ... therefore have no character/suit name
	
    m_bEnterObserver = false;
	m_bReady = false;
	m_bInSpawnProtect = false;
#ifdef USE_OMNIBOT
	m_bIsOmnibot = false;
#endif
	m_iChangedBits = 0;
	m_iAccountID = m_iCharacterID = -1;
	LogOff(); // ONLY clears m_bLogon and m_szLogon
	
	BaseClass::ChangeTeam( 0 );
	
//	UseClientSideAnimation();
	RemoveAllModules();
	limitedQuantities = new LimitedQuantities(this);
	
	m_vecMinionTarget = vec3_origin;
	m_hMinionTarget.Set(NULL);
	m_iNumMinions = 0;
	m_iMinionStance = 2;
	m_flLastUsePress = 0;
	m_flPrevTimePlayed = m_flPlayStart = 0;

	m_iNumVotesThisMap = 0;
	m_bHasNominated = false;
	m_iFaction = 0;

	m_iSprintSpeed = mc_player_base_sprint_speed.GetInt();
}

extern bool		g_fGameOver;

CHL2MP_Player::~CHL2MP_Player( void )
{
	if ( IsLoggedOn() )
		LogOff(g_fGameOver || !HL2MPRules()); // be sure to save (and free the memory of) our character info
	GetLimitedQuantities()->ResetAll(); // just to be totally sure that VAC kicks don't stop this
	delete limitedQuantities;
}

void CHL2MP_Player::UpdateOnRemove( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

extern CUtlVector<CNPCTypeInfo*> g_pNPCInfo;

void CHL2MP_Player::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel ( "sprites/glow01.vmt" );

	//Precache Rebel models
	int nHeads = ARRAYSIZE( g_ppszRandomRebelModels );
	int i;	

	for ( i = 0; i < nHeads; ++i )
	   	 PrecacheModel( g_ppszRandomRebelModels[i] );

	//Precache Combine Models
	nHeads = ARRAYSIZE( g_ppszRandomCombineModels );

	for ( i = 0; i < nHeads; ++i )
		if ( i != 1 && i != 3 )
			PrecacheModel( g_ppszRandomCombineModels[i] );

	//Precache Rebel models
	nHeads = ARRAYSIZE( g_ppszRandomApertureModels );
	
	for ( i = 0; i < nHeads; ++i )
	   	 PrecacheModel( g_ppszRandomApertureModels[i] );

	PrecacheFootStepSounds();

	PrecacheScriptSound( "NPC_MetroPolice.Die" );
	PrecacheScriptSound( "NPC_CombineS.Die" );
	PrecacheScriptSound( "NPC_Citizen.die" ); 
	PrecacheScriptSound( "BoSS.Hello" );
	PrecacheScriptSound( "Headcrab.JumpedOn" );

	// precache all abilities & afflictions
	for ( int i=0; i<NUM_MODULES; i++ )
		GetModule(i)->Precache();
	for ( int i=0; i<NUM_BUFFS; i++ )
		GetBuff(i)->Precache();

	// precache the pvm monsters, as doing so in the gamerules screws up
	int nNpcs = g_pNPCInfo.Count();
	for ( int i=0; i<nNpcs; i++ )
	{
		UTIL_PrecacheOther(g_pNPCInfo[i]->m_szClassname);
		if ( g_pNPCInfo[i]->m_szModelName[0] != '\0' )
			PrecacheModel( g_pNPCInfo[i]->m_szModelName );
	}
	UTIL_PrecacheOther("info_teleport_node");
}

void CHL2MP_Player::GiveAllItems( void )
{
	if ( !IsInCharacter() )
		return;

	EquipSuit();

	CBasePlayer::GiveAmmo( 255,	"Pistol");
	CBasePlayer::GiveAmmo( 255,	"AR2" );
	CBasePlayer::GiveAmmo( 5,	"AR2AltFire" );
	CBasePlayer::GiveAmmo( 255,	"SMG1");
	CBasePlayer::GiveAmmo( 1,	"smg1_grenade");
	CBasePlayer::GiveAmmo( 255,	"Buckshot");
	CBasePlayer::GiveAmmo( 32,	"357" );
	CBasePlayer::GiveAmmo( 3,	"rpg_round");

	CBasePlayer::GiveAmmo( 1,	"grenade" );
	CBasePlayer::GiveAmmo( 2,	"slam" );

	GiveNamedItem( "weapon_crowbar" );
	GiveNamedItem( "weapon_stunstick" );
	GiveNamedItem( "weapon_camera" );
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_357" );

	GiveNamedItem( "weapon_smg1" );
	GiveNamedItem( "weapon_ar2" );
	
	GiveNamedItem( "weapon_shotgun" );
	GiveNamedItem( "weapon_frag" );
	
	GiveNamedItem( "weapon_crossbow" );
	
	GiveNamedItem( "weapon_rpg" );

	GiveNamedItem( "weapon_slam" );

	GiveNamedItem( "weapon_physcannon" );

	SetHealth( GetMaxHealth() );
}

void CHL2MP_Player::GiveDefaultItems( void )
{
	if ( !IsInCharacter() )
		return;

	EquipSuit();

	CBasePlayer::GiveAmmo( 255,	"Pistol");
	CBasePlayer::GiveAmmo( 45,	"SMG1");
	CBasePlayer::GiveAmmo( 1,	"grenade" );
	CBasePlayer::GiveAmmo( 6,	"Buckshot");
	CBasePlayer::GiveAmmo( 6,	"357" );

	if ( GetFaction() == FACTION_COMBINE )// GetPlayerModelType() == PLAYER_SOUNDS_METROPOLICE || GetPlayerModelType() == PLAYER_SOUNDS_COMBINESOLDIER )
		GiveNamedItem( "weapon_stunstick" );
	else if ( GetFaction() == FACTION_APERTURE )
		GiveNamedItem( "weapon_camera" );
	else
		GiveNamedItem( "weapon_crowbar" );
	
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_smg1" );
	GiveNamedItem( "weapon_frag" );
	GiveNamedItem( "weapon_physcannon" );

	const char *szDefaultWeaponName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_defaultweapon" );

	CBaseCombatWeapon *pDefaultWeapon = Weapon_OwnsThisType( szDefaultWeaponName );

	if ( pDefaultWeapon )
	{
		Weapon_Switch( pDefaultWeapon );
	}
	else
	{
		Weapon_Switch( Weapon_OwnsThisType( "weapon_physcannon" ) );
	}
}

void CHL2MP_Player::PickDefaultSpawnTeam( void )
{
	ChangeTeam(TEAM_SPECTATOR);
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2MP_Player::InitialSpawn()
{
	m_flRespawnBlockedTil = gpGlobals->curtime;
	m_flNextSaveTime = gpGlobals->curtime + AUTO_SAVE_INTERVAL;
	PickDefaultSpawnTeam();

//	if they disconnected a game while on a spree, it will still be red unless we do this
	CSingleUserRecipientFilter user( this );
	user.MakeReliable();
	UserMessageBegin( user, "Spree" );
		WRITE_BYTE( 0 ); // stop spree
	MessageEnd();
}

extern McConVar mc_player_base_health, mc_player_base_armor_capacity, mc_player_base_sprint_speed, mc_player_base_aux_capacity, mc_player_base_aux_recharge, mc_spawn_protect_time;

void CHL2MP_Player::Spawn(void)
{	
	m_flNextModelChangeTime = 0.0f;

	BaseClass::Spawn();
	Extinguish();

	m_flNextFactionBenefit = gpGlobals->curtime + 2.5f;

	if ( !IsObserver() )
	{
		pl.deadflag = false;
		RemoveSolidFlags( FSOLID_NOT_SOLID );

		RemoveEffects( EF_NODRAW );
		
		GiveDefaultItems();
	}

	RemoveEffects( EF_NOINTERP );

	SetNumAnimOverlays( 3 );
	ResetAnimation();

	m_nRenderFX = kRenderNormal;

	m_Local.m_iHideHUD = 0;
	
	AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	m_impactEnergyScale = HL2MPPLAYER_PHYSDAMAGE_SCALE;

	if ( HL2MPRules()->IsIntermission() )
	{
		AddFlag( FL_FROZEN );
	}
	else
	{
		RemoveFlag( FL_FROZEN );
	}

	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);

	m_bReady = false;

	if ( IsInCharacter() )
	{
		RemoveFlag(FL_NOTARGET);
		
		m_flSpawnProtectTime = gpGlobals->curtime + mc_spawn_protect_time.GetFloat();
		m_bInSpawnProtect = true;
	}
	else
	{
		AddEffects(EF_NODRAW);
		SetSolid(SOLID_NONE);
		SetMoveType(MOVETYPE_NOCLIP);
		AddFlag(FL_NOTARGET);
		m_bInSpawnProtect = false;
	}

	RemoveAllBuffs(); // won't be active, but still needs set the first time around
	ResetDamageScales(); // put our damage received & dealt scalers to 1.0
//	SetDisorientated(false);
	SetStartArmor(0);
	Extinguish();

	ApplyBuff(BUFF_SPAWN); // fade in, particles, etc

	/*if ( HL2MPRules()->IsPreGame() )
		SetBaseMaxHealth(25);
	else*/
		SetBaseMaxHealth( mc_player_base_health.GetInt() );
	SetMaxHealth( GetBaseMaxHealth() );
	SetMaxArmor( mc_player_base_armor_capacity.GetInt() );
	SetMaxAuxPower( mc_player_base_aux_capacity.GetInt() );
	m_iSprintSpeed = mc_player_base_sprint_speed.GetInt();

	m_pMyLastAttacker = NULL;
	for ( int i=0; i<NUM_MODULES; i++ )
	{
		// clear all ability cooldowns
		m_flModuleCooldownTime.Set(i,0);
		m_flModuleStartTime[i] = 0;
		m_bEnqueuedModules[i] = false;

		// start any abilities that should be always on
		Module *a = GetModule(i);
		if ( HasModule(a) )
		{
			if ( GetModule(i)->AutoStarts() )
				StartModule( GetModule(i) );
			else
				m_flModuleCooldownTime.Set(i, gpGlobals->curtime + a->GetInitialCooldown(GetModuleLevel(a)));
			Buff *aff = a->GetAssociatedBuff();
			if ( aff != NULL )
				ApplyBuff(aff->GetBuffIndex(), NULL, GetModuleLevel(a));
		}
	}
	
	
	SetHealth( GetMaxHealth() ); // max health may have changed now that abilities are turned on (eg vitality)
	SetArmorValue( min( GetStartArmor(), GetMaxArmor() ) ); // the fiddly bit just stops it being set higher than their max
	SetAuxPower( GetMaxAuxPower() );
	
	ResetSpree(); //dying ends our spree

	// make sure we CAN cloak, but aren't cloaked
	SetRenderMode(kRenderTransColor);
	//SetRenderColorA(255);

	if ( IsInCharacter() && HL2MPRules()->IsInSpreeWar() )
		ApplyBuff(BUFF_SPREE_WAR);
		
	for ( int i=0; i<MINION_FORMATION_LIMIT; i++ )
		m_hFormationMinions[i] = NULL;
	m_vecMinionTarget = vec3_origin;
	m_hMinionTarget.Set(NULL);
	m_iNumMinions = 0;
	m_flLastUsePress = 0;
}

void CHL2MP_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	
}

int CHL2MP_Player::DetermineModelFaction( const char *pModel )
{
	int iModels = ARRAYSIZE( g_ppszRandomRebelModels );
	int i;	
	for ( i = 0; i < iModels; ++i )
		if ( !Q_stricmp( g_ppszRandomRebelModels[i], pModel ) )
			return FACTION_RESISTANCE;

	iModels = ARRAYSIZE( g_ppszRandomCombineModels );
	for ( i = 0; i < iModels; ++i )
	   	if ( !Q_stricmp( g_ppszRandomCombineModels[i], pModel ) )
			return FACTION_COMBINE;

	iModels = ARRAYSIZE( g_ppszRandomApertureModels );
	for ( i = 0; i < iModels; ++i )
	   	if ( !Q_stricmp( g_ppszRandomApertureModels[i], pModel ) )
			return FACTION_APERTURE;

	return FACTION_NONE;
}

const char *CHL2MP_Player::GetRandomModelForFaction( int faction )
{
	if ( faction == FACTION_COMBINE )
	{
		int nHeads = ARRAYSIZE( g_ppszRandomCombineModels );
		return g_ppszRandomCombineModels[random->RandomInt(0,nHeads-1)];
	}
	else if ( faction == FACTION_RESISTANCE )
	{
		int nHeads = ARRAYSIZE( g_ppszRandomRebelModels );
		return g_ppszRandomRebelModels[random->RandomInt(0,nHeads-1)];
	}
	else if ( faction == FACTION_APERTURE )
	{
		int nHeads = ARRAYSIZE( g_ppszRandomApertureModels );
		return g_ppszRandomApertureModels[random->RandomInt(0,nHeads-1)];
	}

	return NULL;
}

char mat[MODEL_LENGTH];
void CHL2MP_Player::SetModel( const char *szModelName )
{
	//szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_playermodel" );

	int skin = 0;
	if ( FStrEq(szModelName, "models/combine_soldier2.mdl") || FStrEq(szModelName, "models/combine_soldier_prisonguard2.mdl") )
	{
		skin = 1;
		const char *last2 = strrchr(szModelName,'2');
		Q_StrLeft(szModelName, last2-szModelName, mat, sizeof(mat));
		Q_snprintf(mat, sizeof(mat), "%s.mdl", mat);
		szModelName = mat;
	}

	int modelIndex = modelinfo->GetModelIndex( szModelName );
	if ( modelIndex == -1 || DetermineModelFaction( szModelName ) == FACTION_NONE )
	{
		szModelName = "models/Combine_Soldier.mdl";
		char szReturnString[512];
		Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", szModelName );
		engine->ClientCommand ( edict(), szReturnString );
	}

	BaseClass::SetModel(szModelName);
	SetupPlayerSoundsByModel(szModelName);
	m_nSkin = skin;
}

void CHL2MP_Player::SetupPlayerSoundsByModel( const char *pModelName )
{
	if ( Q_stristr( pModelName, "models/human") )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_CITIZEN;
	}
	else if ( Q_stristr(pModelName, "police" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_METROPOLICE;
	}
	else if ( Q_stristr(pModelName, "combine" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_COMBINESOLDIER;
	}
}

void CHL2MP_Player::ResetAnimation( void )
{
	if ( IsAlive() )
	{
		SetSequence ( -1 );
		SetActivity( ACT_INVALID );

		if (!GetAbsVelocity().x && !GetAbsVelocity().y)
			SetAnimation( PLAYER_IDLE );
		else if ((GetAbsVelocity().x || GetAbsVelocity().y) && ( GetFlags() & FL_ONGROUND ))
			SetAnimation( PLAYER_WALK );
		else if (GetWaterLevel() > 1)
			SetAnimation( PLAYER_WALK );
	}
}


bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	if ( bRet == true )
	{
		ResetAnimation();
	}

	return bRet;
}

void CHL2MP_Player::PreThink( void )
{
	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
	SetLocalAngles( vOldAngles );
}

#define CLOAK_WEAPON_HIDE_LIMIT 24.0f	// weapon can be quite noticable when almost fully invisible

void CHL2MP_Player::PostThink( void )
{
	BaseClass::PostThink();

	if ( m_bInSpawnProtect && m_flSpawnProtectTime <= gpGlobals->curtime )
	{
		m_bInSpawnProtect = false;
		//ClientPrint(this, HUD_PRINTCENTER, "Go!");
	}
	
	if ( GetFlags() & FL_DUCKING )
	{
		SetCollisionBounds( VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX );
	}

	m_PlayerAnimState.Update();

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );

	ManageModules();
	ManageBuffs();


	// rpg won't be hidden altogether, so that the muzzle laser is always visible
	bool bHasRpg = ( GetActiveWeapon() && FClassnameIs(GetActiveWeapon(),"weapon_rpg") );

	// apply any coloring afflictions to my weapon & viewmodel too
	int r = GetRenderColor().r, g = GetRenderColor().g, b = GetRenderColor().b, a = GetRenderColor().a;

	// first the weapon
	if ( GetActiveWeapon() )
	{
		GetActiveWeapon()->SetRenderMode( kRenderTransColor );
		if ( a >= CLOAK_WEAPON_HIDE_LIMIT || bHasRpg )
			GetActiveWeapon()->SetRenderColor( r, g, b, a );
		else// weapons showed up too much without forcing their visibility to 0
			GetActiveWeapon()->SetRenderColor( r, g, b, 0 );
	}

	// and then the view model, so that they can see the effect on themselves
	if ( GetViewModel() )
	{
		GetViewModel()->SetRenderMode( kRenderTransColor );
		GetViewModel()->SetRenderColor( r, g, b, a < CLOAK_WEAPON_HIDE_LIMIT && !bHasRpg ? 0 : a );
	}

	// passive health and regen gain from being near faction-mates in PVM, depending on faction. Combine get extra damage instead.
	if ( HL2MPRules()->IsTeamplay() )
	{// we're in a relevant game mode
		if ( GetFaction() == FACTION_APERTURE )
		{
			if ( m_flNextFactionBenefit <= gpGlobals->curtime )
			{
				AddAuxPower(NumAlliesInRange());
				//IncrementArmorValue(NumAlliesInRange(), GetMaxArmor() );
				m_flNextFactionBenefit = gpGlobals->curtime + pvm_buff_interval_aperture.GetFloat();
			}
		}
		else if ( GetFaction() == FACTION_RESISTANCE )
		{
			if ( m_flNextFactionBenefit <= gpGlobals->curtime )
			{
				TakeHealth(NumAlliesInRange(), DMG_GENERIC);
				m_flNextFactionBenefit = gpGlobals->curtime + pvm_buff_interval_resistance.GetFloat();
			}
		}
	}

	if ( IsInCharacter() && m_flNextSaveTime <= gpGlobals->curtime )
	{
		m_flNextSaveTime = gpGlobals->curtime + AUTO_SAVE_INTERVAL;
		HL2MPRules()->SavePlayerData( this );
	}


	m_bJustBeenHurt = false;
}

void CHL2MP_Player::PlayerDeathThink()
{
	if( !IsObserver() )
	{
		BaseClass::PlayerDeathThink();
		if ( gpGlobals->curtime < m_flRespawnBlockedTil )
			return;

		int fAnyButtonDown = (m_nButtons & ~IN_SCORE);
		if ( (fAnyButtonDown & IN_DUCK) && GetToggledDuckState())
			fAnyButtonDown &= ~IN_DUCK;

		// wait for any button down,  or mp_forcerespawn is set and the respawn time is up
		if (!fAnyButtonDown && !( g_pGameRules->IsMultiplayer() && forcerespawn.GetInt() > 0 ) )
			return;

		m_nButtons = 0;
		m_iRespawnFrames = 0;

		respawn( this, !IsObserver() );// don't copy a corpse if we're in deathcam.
		SetNextThink( TICK_NEVER_THINK );
	}
}

void CHL2MP_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		float damage = pWeapon->GetPrimaryAttackDamage() * GetDamageDealtScale();
		damage += damage * GetFactionalDamageScaleBoost();
		modinfo.m_iPlayerDamage = modinfo.m_iDamage = damage;
	}

	NoteWeaponFired();

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );
}

void CHL2MP_Player::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

extern int ExpForLevelUp( int level );
extern int TotExpForLevelUp( int level );
extern McConVar mc_player_startlevel;

#define HINT2_EXP_THRESHOLD	(ExpForLevelUp(mc_player_startlevel.GetInt()) * 0.333f)
#define HINT3_EXP_THRESHOLD	(ExpForLevelUp(mc_player_startlevel.GetInt()) * 0.667f)

void CHL2MP_Player::AddExp( int add )
{
	if ( m_iTotalExp == 0 )
		SHOW_HINT(this,"Hint_FirstExp")
	else if ( m_iTotalExp < HINT2_EXP_THRESHOLD && m_iTotalExp + add >= HINT2_EXP_THRESHOLD )
		SHOW_HINT(this,"Hint_Exp2")
	else if ( m_iTotalExp < HINT3_EXP_THRESHOLD && m_iTotalExp + add >= HINT3_EXP_THRESHOLD )
		SHOW_HINT(this,"Hint_Exp3")
	m_iTotalExp += add;
	m_iGameExp += add;
	m_iChangedBits |= BITS_CHANGED_CHARACTER;
	CheckLevel();
}

extern ConVar sv_maxunlag;

bool CHL2MP_Player::WantsLagCompensationOnEntity( const CBaseEntity *pEntity, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	//if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		//return false;
	
	return BaseClass::WantsLagCompensationOnEntity(pEntity,pCmd,pEntityTransmitBits);
}

Activity CHL2MP_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	if ( GetFaction() == FACTION_COMBINE ) // if ( m_iModelType == TEAM_COMBINE )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

extern ConVar hl2_normspeed;

// Set the activity based on an event or current state
void CHL2MP_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;

	float speed;

	speed = GetAbsVelocity().Length2D();

	
	// bool bRunning = true;

	//Revisit!
/*	if ( ( m_nButtons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
	{
		if ( speed > 1.0f && speed < hl2_normspeed.GetFloat() - 20.0f )
		{
			bRunning = false;
		}
	}*/

	if ( GetFlags() & ( FL_FROZEN | FL_ATCONTROLS ) )
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	Activity idealActivity = ACT_HL2MP_RUN;

	// This could stand to be redone. Why is playerAnim abstracted from activity? (sjb)
	if ( playerAnim == PLAYER_JUMP )
	{
		idealActivity = ACT_HL2MP_JUMP;
	}
	else if ( playerAnim == PLAYER_DIE )
	{
		if ( m_lifeState == LIFE_ALIVE )
		{
			return;
		}
	}
	else if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( GetActivity( ) == ACT_HOVER	|| 
			 GetActivity( ) == ACT_SWIM		||
			 GetActivity( ) == ACT_HOP		||
			 GetActivity( ) == ACT_LEAP		||
			 GetActivity( ) == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity( );
		}
		else
		{
			idealActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
		}
	}
	else if ( playerAnim == PLAYER_RELOAD )
	{
		idealActivity = ACT_HL2MP_GESTURE_RELOAD;
	}
	else if ( playerAnim == PLAYER_IDLE || playerAnim == PLAYER_WALK )
	{
		if ( !( GetFlags() & FL_ONGROUND ) && GetActivity( ) == ACT_HL2MP_JUMP )	// Still jumping
		{
			idealActivity = GetActivity( );
		}
		/*
		else if ( GetWaterLevel() > 1 )
		{
			if ( speed == 0 )
				idealActivity = ACT_HOVER;
			else
				idealActivity = ACT_SWIM;
		}
		*/
		else
		{
			if ( GetFlags() & FL_DUCKING )
			{
				if ( speed > 0 )
				{
					idealActivity = ACT_HL2MP_WALK_CROUCH;
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE_CROUCH;
				}
			}
			else
			{
				if ( speed > 0 )
				{
					/*
					if ( bRunning == false )
					{
						idealActivity = ACT_WALK;
					}
					else
					*/
					{
						idealActivity = ACT_HL2MP_RUN;
					}
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE;
				}
			}
		}

		idealActivity = TranslateTeamActivity( idealActivity );
	}
	
	if ( idealActivity == ACT_HL2MP_GESTURE_RANGE_ATTACK )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );

		// FIXME: this seems a bit wacked
		Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0 );

		return;
	}
	else if ( idealActivity == ACT_HL2MP_GESTURE_RELOAD )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		return;
	}
	else
	{
		SetActivity( idealActivity );

		animDesired = SelectWeightedSequence( Weapon_TranslateActivity ( idealActivity ) );

		if (animDesired == -1)
		{
			animDesired = SelectWeightedSequence( idealActivity );

			if ( animDesired == -1 )
			{
				animDesired = 0;
			}
		}
	
		// Already using the desired animation?
		if ( GetSequence() == animDesired )
			return;

		m_flPlaybackRate = 1.0;
		ResetSequence( animDesired );
		SetCycle( 0 );
		return;
	}

	// Already using the desired animation?
	if ( GetSequence() == animDesired )
		return;

	//Msg( "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	ResetSequence( animDesired );
	SetCycle( 0 );
}


extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

void CHL2MP_Player::ChangeTeam( int iTeam )
{
/*	if ( GetNextTeamChangeTime() >= gpGlobals->curtime )
	{
		char szReturnString[128];
		Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch teams again.\n", (int)(GetNextTeamChangeTime() - gpGlobals->curtime) );

		ClientPrint( this, HUD_PRINTTALK, szReturnString );
		return;
	}*/

	bool bKill = false;

	if ( HL2MPRules()->IsTeamplay() != true && iTeam != TEAM_SPECTATOR )
	{
		//don't let them try to join combine or rebels during deathmatch.
		iTeam = TEAM_UNASSIGNED;
	}

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( iTeam != GetTeamNumber() && GetTeamNumber() != TEAM_UNASSIGNED )
		{
			bKill = true;
		}
	}

	BaseClass::ChangeTeam( iTeam );

//	m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;

	/*if ( HL2MPRules()->IsTeamplay() == true )
	{
		SetPlayerTeamModel();
	}
	else
	{
		SetPlayerModel();
	}*/
	if ( !IsInCharacter() )
		SetModel(STRING(GetModelName())); // needed to stop crashing on connect

	if ( iTeam == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );

//		State_Transition( STATE_OBSERVER_MODE ); // we dont want spectator hud, particularly
	}
/*
	if ( bKill == true )
	{
		CommitSuicide();
	}*/
}

bool CHL2MP_Player::HandleCommand_JoinTeam( int team )
{
	/*if ( !GetGlobalTeam( team ) || team == 0 )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}*/

	if ( team == TEAM_SPECTATOR )
	{
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			return false;
		}

		if ( GetTeamNumber() != TEAM_UNASSIGNED && !IsDead() )
		{
			m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work

			CommitSuicide();

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;
	}
	else
	{
		StopObserverMode();
		State_Transition(STATE_ACTIVE);
	}

	// Switch their actual team...
	ChangeTeam( team );

	return true;
}

const char *Model2Material(const char *modelname)
{
	// remove models/ from the start & replace .mdl on the end with .vmt
	const char *lastDot = strrchr(modelname,'.');
	Q_StrSlice(modelname, 7, lastDot-modelname+1, mat, sizeof(mat));
	Q_snprintf(mat, sizeof(mat), "%svmt", mat);
	//Msg("Model2Material trimmed %s into %s\n",modelname,mat);
	return mat;
}

char date[48];
const char *DateTime2Date(const char *datetime)
{
	// because there isn't room on character selection panel to show date AND time
	const char *space = strchr(datetime,' ');
	V_StrLeft(datetime, space-datetime+1, date, sizeof(date));
	//Msg("DateTime2Date trimmed %s into %s\n",datetime,date);
	return date;
}

const char *DateChopSeconds(const char *datetime)
{
	const char *lastColon = strrchr(datetime,':');
	V_StrLeft(datetime, lastColon-datetime, date, sizeof(date));
	//Msg("DateChopSeconds trimmed %s into %s\n",datetime,date);
	return date;
}

/*
void PrintModuleHelp(CHL2MP_Player *pPlayer, const char *cmd, bool bToggleOnly=false)
{
	char help[2048];
	Q_snprintf(help,sizeof(help),"possible options:\n");
	for ( int i=0; i<NUM_MODULES; i++ )
		if ( ( !bToggleOnly || GetModule(i)->IsToggled() ) && !GetModule(i)->AutoStarts() )
		{
			char bit[128];
			Q_snprintf(bit,sizeof(bit),"	%s %s\n",cmd,GetModule(i)->GetCmdName());
			Q_strcat(help,bit,sizeof(help));
		}
	ClientPrint(pPlayer,HUD_PRINTCONSOLE,help);
}*/

void ShowMenu(CHL2MP_Player *pPlayer, int defaultPage=NUM_TABS)
{
	CSingleUserRecipientFilter user( pPlayer );
	user.MakeReliable();

	if ( pPlayer->IsInCharacter() )
	{// show the in-game menu (abilities, etc)
		int id = pPlayer->GetCharacterID();

		UserMessageBegin( user, "MainGameMenu" );
			WRITE_BYTE( defaultPage );

			// send char info for the character tab
			//WRITE_STRING( HL2MPRules()->GetCharacterName(id) );
			WRITE_LONG( pPlayer->GetStatPlayerKills() );
			WRITE_LONG( pPlayer->GetStatPlayerDeaths() );
			WRITE_LONG( pPlayer->GetStatMonsterKills() );
			WRITE_LONG( pPlayer->GetStatMonsterDeaths() );
			WRITE_SHORT( pPlayer->GetStatNumSprees() );
			WRITE_SHORT( pPlayer->GetStatNumSpreeWars() );
			WRITE_SHORT( pPlayer->GetStatBestSpree() );
			WRITE_FLOAT( pPlayer->TimePlayed() );
			WRITE_STRING( Model2Material(HL2MPRules()->GetCharacterModel(id, pPlayer->GetFaction())) );
			WRITE_STRING( DateChopSeconds(g_pDB->ReadString("select Created from Characters where ID = '%i'", id)) );
		MessageEnd();
	}
	else if ( pPlayer->IsLoggedOn() )
	{// show the character selection screen
		pPlayer->ShowCharacterSelection();
	}
	else
	{//	just show the logon panel
		UserMessageBegin( user, "ShowLogon" );
			WRITE_BYTE( MANUAL_LOG_OUT );
		MessageEnd();
	}
}

extern std::string Escape(const char *instr);
extern bool IsValidInputString(const char *test);
extern ConVar mc_vote_duration;
extern int g_iGameModeVotedOn;
extern float g_flNextVoteAllowed;
bool CHL2MP_Player::ClientCommand( const CCommand &args )
{
	if ( FStrEq( args[0], "spectate" ) )
	{
		if ( ShouldRunRateLimitedCommand( args ) )
		{
			// instantly join spectators
			//HandleCommand_JoinTeam( TEAM_SPECTATOR );	
			if ( IsInCharacter() )
				LeaveCharacter();
			//StartObserverMode(OBS_MODE_ROAMING);
			State_Transition( STATE_OBSERVER_MODE );
		}
//		ClientPrint( this, HUD_PRINTCENTER, "Bugger off." );
		return true;
	}
	/*else if ( FStrEq( args[0], "jointeam" ) ) 
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad jointeam syntax\n" );
		}

		if ( ShouldRunRateLimitedCommand( args ) )
		{
			int iTeam = atoi( args[1] );
			HandleCommand_JoinTeam( iTeam );
		}
		ClientPrint( this, HUD_PRINTCENTER, "Bugger off." );
		return true;
	}*/
	else if ( FStrEq( args[0], "joingame" ) )
	{
		return true;
	}
	else if ( FStrEq( args[0], "make_logon" ) )
	{
		if ( !IsLoggedOn() && args.ArgC() >= 3 )
		{
			char logon[NAME_LENGTH];
			Q_snprintf(logon,sizeof(logon),args[1]);
			bool spaceNext = true; // for some reason, apostrophes come through as seperate words ... don't want to space them out
			for ( int i=2; i<args.ArgC()-1; i++ ) // get any additional words of the user name
				if ( FStrEq(args[i],"'") )
				{
					V_strcat( logon, UTIL_VarArgs("%s",args[i]), sizeof( logon ) );
					spaceNext = false;
				}
				else
				{
					if ( spaceNext )
						V_strcat( logon, UTIL_VarArgs(" %s",args[i]), sizeof( logon ) );
					else
						V_strcat( logon, UTIL_VarArgs("%s",args[i]), sizeof( logon ) );
					spaceNext = true;
				}

			// server-side validation in case they do it through the client
			if ( !IsValidInputString(logon) )
				return true;

			const char *pass = args[args.ArgC()-1]; // 2 if the name has no spaces

			if ( HL2MPRules()->GetAccountID(logon) != -1 )
			{// account exists already, can't create
				CSingleUserRecipientFilter user( this );
				user.MakeReliable();
				UserMessageBegin( user, "ShowLogon" );
					WRITE_BYTE( ACCOUNT_ALREADY_EXISTS );
				MessageEnd();
			}
			else
			{// succeeded creating and logging into new account
				int id = HL2MPRules()->AddUser(logon,pass);
				SetAccount(id);
				ShowCharacterSelection();
			}
		}
		return true;
	}
	else if ( FStrEq( args[0], "do_logon" ) )
	{
		if ( !IsLoggedOn() && args.ArgC() >= 3 )
		{
			char logon[NAME_LENGTH];
			Q_snprintf(logon,sizeof(logon),args[1]);
			bool spaceNext = true; // for some reason, apostrophes come through as seperate words ... don't want to space them out
			for ( int i=2; i<args.ArgC()-1; i++ ) // get any additional words of the user name
				if ( FStrEq(args[i],"'") )
				{
					V_strcat( logon, UTIL_VarArgs("%s",args[i]), sizeof( logon ) );
					spaceNext = false;
				}
				else
				{
					if ( spaceNext )
						V_strcat( logon, UTIL_VarArgs(" %s",args[i]), sizeof( logon ) );
					else
						V_strcat( logon, UTIL_VarArgs("%s",args[i]), sizeof( logon ) );
					spaceNext = true;
				}

			// server-side validation in case they do it through the client
			if ( !IsValidInputString(logon) )
				return true;

			const char *pass = args[args.ArgC()-1]; // 2 if the name has no spaces

//			Msg("logon = %s, pass = %s\n",logon,pass);
			int accountID = HL2MPRules()->GetAccountID(logon);
			if ( accountID == -1 )
			{// account doesn't exist, fail
				CSingleUserRecipientFilter user( this );
				user.MakeReliable();
				UserMessageBegin( user, "ShowLogon" );
					WRITE_BYTE( ACCOUNT_DOESNT_EXIST );
				MessageEnd();
			}
			else if ( !HL2MPRules()->IsCorrectPassword(accountID,pass) )
			{// password is wrong, fail
				CSingleUserRecipientFilter user( this );
				user.MakeReliable();
				UserMessageBegin( user, "ShowLogon" );
					WRITE_BYTE( PASSWORD_INCORRECT );
				MessageEnd();

//				Msg("Password failure: given & correct hashes:\n%s\n%s\n");
			}
			else if ( HL2MPRules()->GetPlayerByAccountID(accountID) != NULL )
			{// this account is already logged in, fail
				CSingleUserRecipientFilter user( this );
				user.MakeReliable();
				UserMessageBegin( user, "ShowLogon" );
					WRITE_BYTE( ACCOUNT_ALREADY_LOGGED_IN );
				MessageEnd();
			}
			else
			{// logon succeeded, woo!
				SetAccount(accountID);
				
				// Send audio to player.
				CRecipientFilter filter;
				filter.AddRecipient( this );
				UserMessageBegin( filter, "SendAudio" );
				WRITE_STRING( "BoSS.CharacterSelect"  );
				MessageEnd();
				
				ShowCharacterSelection();
			}
		}
		return true;
	}
	else if ( FStrEq( args[0], "logoff" ) || FStrEq( args[0], "logout" ))
	{
		if ( IsLoggedOn() )
		{
			LogOff();
			SetLevelAndExp(0,0);
			SetModulePoints(0);
			RemoveAllModules();
			RemoveActiveStuff();
			respawn(this,false);
			CSingleUserRecipientFilter user( this );
			user.MakeReliable();
			UserMessageBegin( user, "ShowLogon" );
				WRITE_BYTE( MANUAL_LOG_OUT );
			MessageEnd();
		}
		return true;
	}
	else if ( FStrEq( args[0], "make_char" ) )
	{
		if ( IsLoggedOn() && args.ArgC() >= 3 )
		{
			// no more characters, we're full
			if ( HL2MPRules()->GetNumCharacters(this) >= MAX_CHARS )
				return true;

			char name[NAME_LENGTH];
			Q_snprintf(name,sizeof(name),args[2]);
			bool spaceNext = true; // for some reason, apostrophes come through as seperate words ... don't want to space them out
			for ( int i=3; i<args.ArgC(); i++ ) // get any additional words of the character name
				if ( FStrEq(args[i],"'") )
				{
					V_strcat( name, UTIL_VarArgs("%s",args[i]), sizeof( name ) );
					spaceNext = false;
				}
				else
				{
					if ( spaceNext )
						V_strcat( name, UTIL_VarArgs(" %s",args[i]), sizeof( name ) );
					else
						V_strcat( name, UTIL_VarArgs("%s",args[i]), sizeof( name ) );
					spaceNext = true;
				}

			// server-side validation in case they do it through the client
			if ( !IsValidInputString(name) )
				return true;
				
			std::string escaped = Escape(name);
			
			// don't want to check ALL characters on the server for duplicate?
			// just check that this player doesn't have a character of the same name
			int alreadyExists = g_pDB->ReadInt("select ID from characters where AccountID = %i and name = '%s'", m_iAccountID, escaped.c_str());
			if ( alreadyExists != -1 )
			{
				CSingleUserRecipientFilter user( this );
				user.MakeReliable();
				UserMessageBegin( user, "MakeCharFailed" );
					WRITE_BYTE( DUPLICATE_NAME );
				MessageEnd();
				return true;
			}

			// check the model is valid
			char model[MODEL_LENGTH];
			Q_snprintf(model,sizeof(model),args[1]);

			int faction = DetermineModelFaction(model);
			if ( faction == FACTION_NONE )
			{// model not recognised from our list
				CSingleUserRecipientFilter user( this );
				user.MakeReliable();
				UserMessageBegin( user, "MakeCharFailed" );
					WRITE_BYTE( INVALID_MODEL );
				MessageEnd();
				return true;
			}
	
			HL2MPRules()->AddNewCharacter(this,name,model,faction,true);
		}
		return true;
	}
	else if ( FStrEq( args[0], "use_char" ) )
	{// the command used by the character selection GUI
		if ( args.ArgC() < 2 )
			return true;

		int characterID = atoi(args[1]);
		if ( !HL2MPRules()->PlayerOwnsCharacter(this,characterID) )
		{
			ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("You don't own character #%i\n",characterID));
			return true; // can't use someone else's character
		}

		// stop all abilities (plague, etc) and do cleanup
		RemoveActiveStuff();
		HL2MPRules()->ApplyDataToPlayer(this, characterID);
		return true;
	}
	else if ( FStrEq( args[0], "del_char" ) )
	{
		if ( args.ArgC() < 2 )
			return true;
		int characterID = atoi(args[1]);
		HL2MPRules()->DeleteCharacter(this,characterID); // this checks if i own it first
		return true;
	}
	else if ( FStrEq( args[0], "pickchar" ) )
	{// the command used by the button on the character panel of the MAIN (in-game) menu
		if ( IsInCharacter() )
			LeaveCharacter();

		if ( IsLoggedOn() )
			ShowCharacterSelection(args.ArgC() >= 2 ? atoi(args[1]) : 0);
		return true;
	}
	else if ( FStrEq( args[0], "mcmenu" ) )
	{// toggle the menu
		ShowMenu(this);
		return true;
	}
	else if ( FStrEq( args[0], "buy" ) )
	{// buy a module
		if ( !IsInCharacter() )
		{// if out of character, show the menu
			ShowMenu(this);
			return true;
		}
		if ( args.ArgC() < 2 )
		{// if no params, show the menu, jumping to the modules tab
			ShowMenu(this,TAB_MODULES);
			return true;
		}
		Module *a = GetModule(args[1]);
		if ( a && a->IsPurchasable() )
			BuyModule(a);
		else
		{
			ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("Invalid module: %s\n",args[1]));
			//PrintModuleHelp(this,"buy");
		}
		return true;
	}
	else if ( FStrEq( args[0], "+do" ) || FStrEq( args[0], "do" ) )
	{// do (also start / stop) an ability
		if ( !IsInCharacter() )
			return true;

		if ( args.ArgC() < 2 )
		{
			ClientPrint(this,HUD_PRINTCONSOLE,"Syntax: +do <ability name>\n");
			//PrintModuleHelp(this,"do");
			return true;
		}
		Module *a = GetModule(args[1]);
		if ( a )
		{
			if ( !a->AutoStarts() ) // cannot manually control autostarting abilities
				UseModule(a);
			else
				ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("Cannot manually start/stop %s\n",args[1]));
		}
		else
		{
			ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("Invalid ability: %s\n",args[1]));
			//PrintModuleHelp(this,"do");
		}
		return true;
	}
	else if ( FStrEq( args[0], "-do" ) )
	{// stop an ability, really only for "held" ones (i.e. jetpack)
		if ( !IsInCharacter() )
			return true;

		if ( args.ArgC() < 2 )
			return true;

		Module *a = GetModule(args[1]);
		if ( a && a->IsToggledByHoldingKey() )
			StopModule(a);

		return true;
	}
	else if ( FStrEq( args[0], "begin" ) )
	{// use / start / stop an ability
		if ( !IsInCharacter() )
			return true;

		if ( args.ArgC() < 2 )
		{
			ClientPrint(this,HUD_PRINTCONSOLE,"Syntax: begin <ability name>\n");
			//PrintModuleHelp(this,"begin",true);
			return true;
		}
		Module *a = GetModule(args[1]);
		if ( a )
		{
			if ( !a->AutoStarts() ) // cannot manually control autostarting abilities
				StartModule(a);
			else
				ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("Cannot manually start/stop %s\n",args[1]));
		}
		else
		{
			ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("Invalid ability: %s\n",args[1]));
			//PrintModuleHelp(this,"begin",true);
		}
		return true;
	}
	else if ( FStrEq( args[0], "end" ) )
	{// use / start / stop an ability
		if ( !IsInCharacter() )
			return true;

		if ( args.ArgC() < 2 )
		{
			ClientPrint(this,HUD_PRINTCONSOLE,"Syntax: end <ability name>\n");
			//PrintModuleHelp(this,"end",true);
			return true;
		}
		Module *a = GetModule(args[1]);
		if ( a )
		{
			if ( !a->AutoStarts() ) // cannot manually control autostarting abilities
				StopModule(a);
			else
				ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("Cannot manually start/stop %s\n",args[1]));
		}
		else
		{
			ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("Invalid ability: %s\n",args[1]));
			//PrintModuleHelp(this,"end",true);
		}
		return true;
	}
	else if ( FStrEq( args[0], "minionstance" ) )
	{
		if ( args.ArgC() < 2 )
		{// cycle stance
			if ( m_iMinionStance < 3 )
				m_iMinionStance ++;
			else
				m_iMinionStance = 1;
			if ( NumMinionsInFormation() > 0 )
				switch(m_iMinionStance)
				{
				case 1:
					ClientPrint(this, HUD_PRINTCENTER, "Minions set to Defensive"); break;
				case 2:
					ClientPrint(this, HUD_PRINTCENTER, "Minions set to Balanced"); break;
				case 3:
					ClientPrint(this, HUD_PRINTCENTER, "Minions set to Roaming"); break;
				}
		}
		else
		{// specific stance (1-3)
			int stance = atoi(args[1]);
			if ( stance > 0 && stance < 4 )
			{
				m_iMinionStance = stance;
				if ( NumMinionsInFormation() > 0 )
					switch(m_iMinionStance)
					{
					case 1:
						ClientPrint(this, HUD_PRINTCENTER, "Minions set to Defensive"); break;
					case 2:
						ClientPrint(this, HUD_PRINTCENTER, "Minions set to Balanced"); break;
					case 3:
						ClientPrint(this, HUD_PRINTCENTER, "Minions set to Roaming"); break;
					}
			}
			else
				ClientPrint(this, HUD_PRINTCONSOLE, "Invalid stance, must be 1-3!\n");
		}
		return true;
	}
	else if ( FStrEq( args[0], "startvote") )
	{
		int votesPerMapLimit = mc_votes_per_map.GetInt();
		if ( votesPerMapLimit > 0 && m_iNumVotesThisMap >= votesPerMapLimit )
		{
			ClientPrint(this, HUD_PRINTTALK, "You cannot start another vote until the map changes\n");
			return true;
		}
		if ( g_flNextVoteAllowed > gpGlobals->curtime )
		{
			ClientPrint(this, HUD_PRINTTALK, "You can't start a vote so soon after the previous vote\n");
			return true;
		}

		if ( !HL2MPRules()->IsInVote() )
		{
			float remainingTime = HL2MPRules()->GetMapRemainingTime();
			if ( remainingTime != 0 && remainingTime < (HL2MPRules()->NumQueuedVotes()+1) * mc_vote_duration.GetFloat())
			{// if a vote sequence were started now, and it got voted down at the first step, there wouldn't be time for the full automatic sequence to run before the end of the map!
				ClientPrint(this, HUD_PRINTTALK, "It is too late to start a new vote on this map!\n");
				return true;
			}

			if ( args.ArgC() > 1 && mc_vote_gamemode_enabled.GetInt() >= 2 )
			{// a "do you want this mode" vote
				int mode = atoi(args[1]);
				if ( mode > PREGAME && mode <= NUM_GAME_MODES )
				{
					g_iGameModeVotedOn = mode;
					HL2MPRules()->PrepareVoteSequence(true, this);
					HL2MPRules()->RunNextVote();
					m_iNumVotesThisMap ++;
				}
			}
			else if ( mc_vote_map_enabled.GetInt() > 0 || mc_vote_gamemode_enabled.GetInt() >= 2 )
			{// a "what mode do you want" vote
				HL2MPRules()->PrepareVoteSequence(false, this);
				HL2MPRules()->RunNextVote();
				m_iNumVotesThisMap ++;
			}
		}
		return true;
	}
	else if ( FStrEq( args[0], "nominate") )
	{
		if ( m_bHasNominated )
		{
			ClientPrint( this, HUD_PRINTTALK, "You have already nominated a map this game" );
			return true; // already nominated a map this game. Don't let someone fill the list with pish
		}

		if ( args.ArgC() < 2 || mc_vote_map_enabled.GetInt() < 2 )
			return false;

		m_bHasNominated = HL2MPRules()->Nominate(this, args[1]);
		return true;
	}
	else if ( FStrEq( args[0], "vote") )
	{
		if ( CanVote() && HL2MPRules()->IsInVote() && args.ArgC() > 1 )
		{
			int mode = atoi( args[1] );
			if ( mode > 0 && mode <= MAX_VOTE_OPTIONS )
				HL2MPRules()->Vote(this, mode);
		}
		return true;
	}
	else if ( FStrEq( args[0], "faction") )
	{
		int targetFaction = atoi( args[1] );
		if ( HL2MPRules()->IsFactionChangeAllowed(GetFaction(), targetFaction) )
		{
			if ( GetNextTeamChangeTime() >= gpGlobals->curtime )
			{
				char szReturnString[128];
				Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch factions again.\n", (int)(GetNextTeamChangeTime() - gpGlobals->curtime) );
				ClientPrint( this, HUD_PRINTTALK, szReturnString );
				return true;
			}

			if ( !IsDead() )
			{
				m_fNextSuicideTime = gpGlobals->curtime; // allow the suicide to work
				CommitSuicide();
				IncrementFragCount( 1 ); // add 1 to frags to balance out the 1 subtracted for killing yourself
			}

			SetFaction(targetFaction);
			SetModel(HL2MPRules()->GetCharacterModel(m_iCharacterID, targetFaction));
			m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;
			
			switch ( targetFaction )
			{
			case FACTION_COMBINE:	
				ClientPrint(this, HUD_PRINTTALK, "You have joined the Combine until the end of the round!\n");
				break;
			case FACTION_RESISTANCE:	
				ClientPrint(this, HUD_PRINTTALK, "You have joined the Resistance until the end of the round!\n");
				break;
			case FACTION_APERTURE:	
				ClientPrint(this, HUD_PRINTTALK, "You have joined Aperture until the end of the round!\n");
				break;
			}
		}
		else
			ClientPrint(this, HUD_PRINTTALK, "This faction change is not currently allowed.\n");
		return true;
	}

	return BaseClass::ClientCommand( args );
}

void CHL2MP_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
		case 101:
			{
				if( sv_cheats->GetBool() )
				{
					GiveAllItems();
				}
			}
			break;

		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CHL2MP_Player::ShouldRunRateLimitedCommand( const CCommand &args )
{
	int i = m_RateLimitLastCommandTimes.Find( args[0] );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( args[0], gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < HL2MP_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

// send character selection menu info. Format as follows:
// byte:		number of characters being sent
// for each
//	 long:		character ID
//	 byte:		level
//	 string:	model material
//	 string:	date created
//	 string:	last active
void CHL2MP_Player::ShowCharacterSelection(int page)
{
	if ( page < 0 )
		page = 0;
	else if ( page >= MAX_CHARS / CHARS_PER_PAGE )
		page = MAX_CHARS / CHARS_PER_PAGE - 1;
	
	CSingleUserRecipientFilter user( this );
	user.MakeReliable();

	dbReadResult *characterIDs = g_pDB->ReadMultiple("select ID from Characters where AccountID = %i", GetAccountID());
	int offset = page*CHARS_PER_PAGE;
	int numChars = clamp(characterIDs->Count()-offset, 0, CHARS_PER_PAGE);

	UserMessageBegin( user, "PickChar" );
		WRITE_BYTE( characterIDs->Count() ); // how many characters in total do they have?
		WRITE_BYTE( page+1 ); // page number (eg 1/4)
		WRITE_BYTE( numChars ); // how many characters to show on this page?
		for ( int i=0; i<numChars; i++ )
		{
			int charID = characterIDs->Element(i+offset).integer;
			dbReadResult *info = g_pDB->ReadMultiple("select Level, Name, DefaultModel, Created, LastActive from Characters where ID = %i", charID);

			WRITE_LONG( charID ); // ID
			WRITE_BYTE( info->Element(0).integer ); // level
			WRITE_STRING( info->Element(1).text ); // name
			WRITE_STRING( Model2Material(info->Element(2).text) ); // model material
			WRITE_STRING( DateTime2Date(info->Element(3).text) );
			WRITE_STRING( DateTime2Date(info->Element(4).text) );

			info->Purge();
			delete info;
		}
	MessageEnd();

	characterIDs->Purge();
	delete characterIDs;
}

void CHL2MP_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CHL2MP_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CHL2MPRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CHL2MPRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( hl2mp_ragdoll, CHL2MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL2MPRagdoll, DT_HL2MPRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nSkin), 2, SPROP_UNSIGNED ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


void CHL2MP_Player::CreateRagdollEntity( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CHL2MPRagdoll* >( CreateEntityByName( "hl2mp_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nSkin = m_nSkin;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2MP_Player::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOn( void )
{
	if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "HL2Player.FlashlightOn" );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOff( void )
{
	RemoveEffects( EF_DIMLIGHT );
	
	if( IsAlive() )
	{
		EmitSound( "HL2Player.FlashlightOff" );
	}
}

void CHL2MP_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	//Drop a grenade if it's primed.
	if ( GetActiveWeapon() )
	{
		CBaseCombatWeapon *pGrenade = Weapon_OwnsThisType("weapon_frag");

		if ( GetActiveWeapon() == pGrenade )
		{
			if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) )
			{
				DropPrimedFragGrenade( this, pGrenade );
				return;
			}
		}
	}

	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}


void CHL2MP_Player::DetonateTripmines( bool playSound )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	if(playSound)
		EmitSound( "Weapon_SLAM.SatchelDetonate" );
}

// remove all my stuff
void CHL2MP_Player::RemoveActiveStuff()
{
	ResetSpree(); // end my spree
	DetonateTripmines(false); // As it's apprently not being called

	// disable all active effects
	for ( int i=0; i<NUM_MODULES; i++ )
	{
		Module *a = GetModule(i);
		if ( IsModuleActive(a) )
			StopModule(a);
	}
	RemoveAllBuffs(); // players stop being sick when we're dead
	GetLimitedQuantities()->ResetAll();

	for ( int i=0; i<MINION_FORMATION_LIMIT; i++ )
		m_hFormationMinions[i] = NULL;
	m_vecMinionTarget = vec3_origin;
	m_hMinionTarget.Set(NULL);
	m_iNumMinions = 0;
}

extern McConVar mc_spree_start, mc_spreewar_start, mc_respawn_delay, gamemode_hoarder_respawn_delay_no_tokens, mc_death_shared_experience_scaling;

void CHL2MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	// must enable controls when killed
	EnableControl(true);
	StopParticleEffects(this);

	if ( !IsInCharacter() )
		return;

	m_bInSpawnProtect = false;

	if ( GetSpree() >= mc_spree_start.GetInt() )
	{
		if ( info.GetAttacker() && info.GetAttacker()->IsPlayer() && info.GetAttacker() != this )
			UTIL_ClientPrintAll( HUD_PRINTTALK, UTIL_VarArgs("%s's %i-kill spree was ended by %s\n",GetPlayerName(),GetSpree(),ToHL2MPPlayer(info.GetAttacker())->GetPlayerName()));
		else
			UTIL_ClientPrintAll( HUD_PRINTTALK, UTIL_VarArgs("%s's %i-kill spree came to an unfortunate end\n",GetPlayerName(),GetSpree()) );

		// event for stat and omni-bot usage
		IGameEvent * event = gameeventmanager->CreateEvent( "spree_end" );
		if ( event )
		{
			event->SetInt("userid", GetUserID() );
			if ( info.GetAttacker() && info.GetAttacker()->IsPlayer() && info.GetAttacker() != this )
				event->SetInt("endedby",ToHL2MPPlayer(info.GetAttacker())->GetUserID());
			else
				event->SetInt("endedby", 0);
			event->SetInt("kills",GetSpree());
			gameeventmanager->FireEvent( event );
		}
	}
	RemoveActiveStuff();
	float scale = mc_death_shared_experience_scaling.GetFloat();
	for ( int i=0; i<m_DamageGivenThisLife.Count(); i++ )
		m_DamageGivenThisLife[i].flDamage *= scale;

	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	SetNumAnimOverlays( 0 );

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	CreateRagdollEntity();

	m_flRespawnBlockedTil = gpGlobals->curtime + ((HL2MPRules() && HL2MPRules()->ShouldUseScoreTokens() && GetNumScoreTokens() == 0 ) ? gamemode_hoarder_respawn_delay_no_tokens.GetFloat() : mc_respawn_delay.GetFloat());

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			if ( info.GetDamageType() & DMG_DIRECT )
				m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_CORE );
			else
				m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	CBaseEntity *pAttacker = info.GetAttacker();

	if ( pAttacker )
	{
		int iScoreToAdd = 1;

		if ( pAttacker == this )
		{
			iScoreToAdd = -1;
		}
		else if ( pAttacker->IsPlayer() )
		{
			// award combo experience if they got the kill really soon after their last kill
			CHL2MP_Player *pAttackerPlayer = ToHL2MPPlayer(pAttacker);
			if ( pAttackerPlayer->GetLastPlayerKillTime() >= gpGlobals->curtime - mc_combo_kill_max_interval.GetFloat() )
			{
				pAttackerPlayer->AddExp(mc_combo_kill_experience.GetInt());

				// and tell them they got the combo, in the same place as where they get regular experience messages
				const char *cl_showexpgained = engine->GetClientConVarValue( engine->IndexOfEdict( pAttackerPlayer->edict() ), "cl_showexpgained" );
				if ( cl_showexpgained )
				{
					int value = atoi( cl_showexpgained );
					if ( value > 0 )
					{// 0 = none, 1 = talk, 2 = notify, 3 = center
						ClientPrint( pAttackerPlayer, value == 2 ? HUD_PRINTNOTIFY : value == 3 ? HUD_PRINTCENTER : HUD_PRINTTALK, UTIL_VarArgs("Combo! (+%i experience)\n", mc_combo_kill_experience.GetInt() ) );
					}
				}
			}
			pAttackerPlayer->SetLastPlayerKillTime(gpGlobals->curtime);
		}

		//GetGlobalTeam( pAttacker->GetTeamNumber() )->AddScore( iScoreToAdd );
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	StopZooming();

	m_iChangedBits |= BITS_CHANGED_CHARACTER;
}

extern McConVar mod_magmine_downward_force_to_count_as_kill, mod_magmine_gravitykills;
extern Vector CalcMagdForce(CBaseCombatCharacter *magd, CBaseCombatCharacter *sucked, float interval);
int CHL2MP_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//return here if the player is in the respawn grace period 
	if ( m_bInSpawnProtect )//&& (inputInfo.GetDamageType() & DMG_CRUSH|DMG_FALL) == 0 && pAttacker == NULL )
		return 0;

	if ( GetFlags() & FL_FROZEN )
		return 0; // take no damage when frozen

	CTakeDamageInfo info = inputInfo;
	m_bJustBeenHurt = true; // let abilities know i've been hurt this frame
	CBaseEntity *pAttacker = info.GetAttacker(); // remember who hurt me last
	if ( mod_magmine_gravitykills.GetInt() == 1 && /*!pAttacker &&*/ (info.GetDamageType() & DMG_FALL) ) // if hurt by fall damage, and magd gravity kills are allowed
	{// see if a MAGD was pulling me downwards - if it was, it surely contributed to this death
        Vector gravWellForce(0,0,0);
		for ( int i=0; i<MAX_MAGMINES; i++ )
		{
			CBaseCombatCharacter *well = HL2MPRules()->GetGravityWell(i);
			if ( !well )
				break; // we shunt them all up, so there's never any after a NULL

			gravWellForce += CalcMagdForce(well,this,1.0); // calculate the force for a whole second. keeps decision framerate-independent!
		}
		
		if ( gravWellForce.z < -mod_magmine_downward_force_to_count_as_kill.GetFloat() )
		{// set attacker to the owner of the nearest (hostile) MAGD - so we need to calculate which is nearest
			CBaseGrenade *pNearest = NULL;
			float nearestDist = 9999999;
			for ( int i=0; i<MAX_MAGMINES; i++ )
			{
				CBaseCombatCharacter *well = HL2MPRules()->GetGravityWell(i);
				if ( !well )
					break;
				CBaseGrenade *pMagd = (CBaseGrenade*)well;
		        if ( HL2MPRules()->IsFriendly(this,pMagd->GetThrower()) )
					continue; // ignore friendly MAGDs
				
				float dist = (pMagd->GetAbsOrigin() - GetAbsOrigin()).Length();
				if ( dist < nearestDist )
				{
					nearestDist = dist;
					pNearest = pMagd;
				}
			}
			
			if ( pNearest != NULL )
			{
				info.SetAttacker( pNearest->GetThrower() );
				pAttacker = info.GetAttacker();
			}
		}
	}
	
	if ( pAttacker && pAttacker != this && ( !info.GetInflictor() || !info.GetInflictor()->IsNPC() ) )
		m_pMyLastAttacker = pAttacker;
	else
		m_pMyLastAttacker = NULL;

	m_vecTotalBulletForce += info.GetDamageForce();
	
	gamestats->Event_PlayerDamage( this, info );

	bool wasAlive = m_iHealth > 0;

#ifdef USE_OMNIBOT
	Omnibot::Notify_Hurt(this, pAttacker);
#endif
	int retVal = BaseClass::OnTakeDamage( info );

	if ( pAttacker && pAttacker->MyCombatCharacterPointer() )
		pAttacker->MyCombatCharacterPointer()->RegisterAttackOnEntity( info, this, wasAlive && m_iHealth <= 0 );
		
	return retVal;
}

void CHL2MP_Player::DeathSound( const CTakeDamageInfo &info )
{
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		 return;

	char szStepSound[128];

	Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.Die", GetPlayerModelSoundPrefix() );

	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( szStepSound, params, pModelName ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

CBaseEntity* CHL2MP_Player::EntSelectSpawnPoint( void )
{
	CBaseEntity *pSpot = NULL;
	CBaseEntity *pLastSpawnPoint = g_pLastSpawn;
	edict_t		*player = edict();
	const char *pSpawnpointName = "info_player_deathmatch";

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		switch ( GetFaction() )
		{
		case FACTION_COMBINE:
			pSpawnpointName = "info_start_combine";
			pLastSpawnPoint = g_pLastCombineSpawn;
			break;
		case FACTION_RESISTANCE:
			pSpawnpointName = "info_start_resistance";
			pLastSpawnPoint = g_pLastResistanceSpawn;
			break;
		case FACTION_APERTURE:
			pSpawnpointName = "info_start_aperture";
			pLastSpawnPoint = g_pLastApertureSpawn;
			break;
				
/*		default: // not needed - defaults are set at top of the function
			pSpawnpointName = "info_player_deathmatch";
			pLastSpawnPoint = g_pLastSpawn;
			break;*/
		}

		// if map doesn't have team spawns (currently, no maps do!) then use regular DM spawns
		if ( gEntList.FindEntityByClassname( NULL, pSpawnpointName ) == NULL )
		{
			pSpawnpointName = "info_player_deathmatch";
			pLastSpawnPoint = g_pLastSpawn;
		}
	}

	pSpot = pLastSpawnPoint;
	// Randomize the start spot
	for ( int i = random->RandomInt(1,5); i > 0; i-- )
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	if ( !pSpot )  // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );

	CBaseEntity *pFirstSpot = pSpot;

	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	if ( pSpot )
	{
		CBaseEntity *ent = NULL;
		for ( CEntitySphereQuery sphere( pSpot->GetAbsOrigin(), 128 ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
		{
			// if ent is a client, kill em (unless they are ourselves)
			if ( ent->IsPlayer() && !(ent->edict() == player) )
				ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
		}
		goto ReturnSpot;
	}

	if ( !pSpot  )
	{
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_start" );

		if ( pSpot )
			goto ReturnSpot;
	}

ReturnSpot:

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		switch ( GetFaction() )
		{
		case FACTION_COMBINE:
			g_pLastCombineSpawn = pSpot;
			break;
		case FACTION_RESISTANCE:
			g_pLastResistanceSpawn = pSpot;
			break;
		case FACTION_APERTURE:
			g_pLastApertureSpawn = pSpot;
			break;
		}
	}

	g_pLastSpawn = pSpot;
	return pSpot;
} 

CON_COMMAND( welcome, "players welcome message" )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );
	if ( pPlayer )
		pPlayer->PlayWelcomeMessage();
}

CON_COMMAND( timeleft, "prints the time remaining in the match" )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = (int)HL2MPRules()->GetMapRemainingTime();

	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "This game has no timelimit." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in map: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}


void CHL2MP_Player::Reset()
{	
	ResetDeathCount();
	ResetFragCount();
}

bool CHL2MP_Player::IsReady()
{
	return m_bReady;
}

void CHL2MP_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

void CHL2MP_Player::CheckChatText( char *p, int bufsize )
{
	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

	const char *pReadyCheck = p;

	HL2MPRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CHL2MP_Player::State_Transition( HL2MPPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CHL2MP_Player::State_Enter( HL2MPPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CHL2MP_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CHL2MP_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CHL2MPPlayerStateInfo *CHL2MP_Player::State_LookupInfo( HL2MPPlayerState state )
{
	// This table MUST match the 
	static CHL2MPPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CHL2MP_Player::State_Enter_ACTIVE, NULL, &CHL2MP_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CHL2MP_Player::State_Enter_OBSERVER_MODE,	NULL, &CHL2MP_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

bool CHL2MP_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	if ( m_bEnterObserver == true )
	{
		VPhysicsDestroyObject();
		return BaseClass::StartObserverMode( mode );
	}
	return false;
}

void CHL2MP_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CHL2MP_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CHL2MP_Player::State_PreThink_OBSERVER_MODE()
{
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}


void CHL2MP_Player::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	
	// md 8/15/07 - They'll get set back to solid when they actually respawn. If we set them solid now and mp_forcerespawn
	// is false, then they'll be spectating but blocking live players from moving.
	// RemoveSolidFlags( FSOLID_NOT_SOLID );
	
	m_Local.m_iHideHUD = 0;
}


void CHL2MP_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2MP_Player::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Make experience work
//-----------------------------------------------------------------------------

// We have gained experience, check if we should level up.
void CHL2MP_Player::CheckLevel()
{
	while ( GetLevel() < HL2MPRules()->GetMaxLevel() && GetTotalExp() >= TotExpForLevelUp( GetLevel() ) )
		LevelUp();		// increment level, award points, notify them of change
}

void CHL2MP_Player::LevelUp()
{
	SetLevel( GetLevel() + 1 );
	

	int modulepoints = mc_perlevel_modulepoints.GetInt(); 
//	int weaponpoints = 2; 
 	AddModulePoints( modulepoints ); 
// 	AddWP( weaponpoints );

	if ( GetLevel() < HL2MPRules()->GetMaxLevel() )
	{
		int nextExp = TotExpForLevelUp( GetLevel() );
		SHOW_HINT_ALWAYS(this,UTIL_VarArgs( "Welcome to level %i!\nYou have %i module points,\nand need %i more experience to level up again.", GetLevel(), GetModulePoints(), nextExp-GetTotalExp()) );
	}
	else
		SHOW_HINT_ALWAYS(this,UTIL_VarArgs( "Welcome to level %i, THE MAX LEVEL!\nYou have %i module points.", GetLevel(), GetModulePoints() ) );
	UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs("%s is now level %i!", GetPlayerName(), GetLevel()) );
	ClientPrint(this,HUD_PRINTTALK,UTIL_VarArgs( "You received %i ability points for leveling up", modulepoints ) );
// 	ClientPrint(this,HUD_PRINTTALK,UTIL_VarArgs( "You received %i ability points and %i weapon points for leveling up", abilitypoints, weaponpoints ) );

	// Save stats.
	HL2MPRules()->SavePlayerData(this);

	// event for stat and omni-bot usage
	IGameEvent * event = gameeventmanager->CreateEvent( "level_up" );
	if ( event )
	{
		event->SetInt("userid", GetUserID() );
		event->SetInt("level", GetLevel());
		gameeventmanager->FireEvent( event );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Make abilities work.
//-----------------------------------------------------------------------------
bool g_bShowdamageScaleMessages = true;
void CHL2MP_Player::BuyModule(Module *a)
{
	if( !IsInCharacter() )
	{
		ClientPrint( this, HUD_PRINTTALK, "Spectators cannot buy abilities" );
		return;
	}

 	if ( GetModulePoints() < a->GetUpgradeCost() ) 
 	{ 
		ClientPrint(this,HUD_PRINTTALK,UTIL_VarArgs("You don't have enough ability points to buy this! (%i points left)\n", GetModulePoints() ) ); 
 		return; 
 	} 
 	else 
 	{ 
 		AddModulePoints( - a->GetUpgradeCost() ); 
 		ClientPrint(this,HUD_PRINTTALK,UTIL_VarArgs("You spent %i to buy %s. (%i points left)\n", a->GetUpgradeCost(), a->GetDisplayName(), GetModulePoints() ) ); 
 	} 


	if ( GetModuleLevel(a) >= a->GetMaxLevel() )
	{
		ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("You have already maxed out %s!\n",a->GetDisplayName()));
		return;
	}

	m_iChangedBits |= BITS_CHANGED_MODULES;
	BaseClass::BuyModule(a);
	ClientPrint(this,HUD_PRINTCONSOLE,UTIL_VarArgs("Bought %s, level %i\n",a->GetDisplayName(),GetModuleLevel(a)));
}

void CHL2MP_Player::UseModule(Module *a)
{
	BaseClass::UseModule(a);
}

bool CHL2MP_Player::EnqueueModule(Module *a)
{
	if ( !HasModule(a) )
	{
		ClientPrint(this,HUD_PRINTTALK,UTIL_VarArgs("You don't have %s!\n",a->GetDisplayName()));
		return false;
	}

	int level = GetModuleLevel(a);
	if ( !a->UsesSmoothAuxDrain() && GetAuxPower() < a->GetAuxDrain(this, level) )
		return false; // insufficient power

	if ( BaseClass::EnqueueModule(a) )
	{
		if ( !a->UsesSmoothAuxDrain() )
			DrainAuxPower(a->GetAuxDrain(this, level));
		return true;
	}
	else
		return false;
}

bool CHL2MP_Player::DoModule(Module *a, bool isEnqueued) // isEnqueued should default to false
{
	if ( !HasModule(a) )
	{
		ClientPrint(this,HUD_PRINTTALK,UTIL_VarArgs("You don't have %s!\n",a->GetDisplayName()));
		return false;
	}

	int level = GetModuleLevel(a);
	if ( !a->IsToggled() && !a->UsesSmoothAuxDrain() && !isEnqueued && GetAuxPower() < a->GetAuxDrain(this, level) )
	{
		a->FailEffect(this,GetModuleLevel(a));
		return false; // insufficient power
	}

	if ( BaseClass::DoModule(a, isEnqueued) )
	{
		if ( !a->UsesSmoothAuxDrain() )
			DrainAuxPower(a->GetAuxDrain(this, level));
		return true;
	}
	else
		return false;
}

bool CHL2MP_Player::StartModule(Module *a)
{
	if ( !a->IsToggled() )
	{
		ClientPrint(this,HUD_PRINTNOTIFY,UTIL_VarArgs("%s does not toggle on/off!\n",a->GetDisplayName()));
		return false;
	}

	if ( !HasModule(a) )
	{
		ClientPrint(this,HUD_PRINTTALK,UTIL_VarArgs("You don't have %s!\n",a->GetDisplayName()));
		return false;
	}

	if ( !a->UsesSmoothAuxDrain() && GetAuxPower() < a->GetAuxDrain(this, GetModuleLevel(a)) )
		return false;

	return BaseClass::StartModule(a);
}

void CHL2MP_Player::StopModule(Module *a)
{
	if ( !a->IsToggled() )
	{
		ClientPrint(this,HUD_PRINTNOTIFY,UTIL_VarArgs("%s does not toggle on/off!\n",a->GetDisplayName()));
		return;
	}

	if ( !HasModule(a) )
	{
		ClientPrint(this,HUD_PRINTTALK,UTIL_VarArgs("You don't have %s!\n",a->GetDisplayName()));
		return;
	}

	BaseClass::StopModule(a);
//	ClientPrint(this,HUD_PRINTNOTIFY,UTIL_VarArgs("Disabled %s\n",a->GetDisplayName()));
}


// returns who / what we're looking directly at
CBaseEntity *CHL2MP_Player::GetAimTarget(bool bFriendly)
{
	autoaim_params_t params;

	params.m_fScale = AUTOAIM_20DEGREES;
	params.m_fMaxDist = 8096;

	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );
	CBasePlayer::GetAutoaimVector( params, bFriendly, true );
	CBaseEntity *pTarget = params.m_hAutoAimEntity;
	lagcompensation->FinishLagCompensation( this );

	CSingleUserRecipientFilter filter( this );

	if ( pTarget != NULL )
		EmitSound( filter, entindex(), "Weapon_SLAM.SatchelDetonate" );
	else
		EmitSound( filter, entindex(), "HL2Player.UseDeny" );

	return pTarget;
}

void CHL2MP_Player::ScaleDamageDealt(float f)
{
	BaseClass::ScaleDamageDealt(f);
	int val = atoi(engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_show_damage_scale_messages" ));
	if ( g_bShowdamageScaleMessages && val > 0)
		ClientPrint(this,val == 1 ? HUD_PRINTTALK : HUD_PRINTCONSOLE,UTIL_VarArgs("You are now dealing damage at %i%% of normal\n",RoundFloatToInt(GetDamageDealtScale()*100.0f)));
}

void CHL2MP_Player::ScaleDamageReceived(float f)
{
	BaseClass::ScaleDamageReceived(f);
	int val = atoi(engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_show_damage_scale_messages" ));
	if ( g_bShowdamageScaleMessages && val > 0)
		ClientPrint(this,val == 1 ? HUD_PRINTTALK : HUD_PRINTCONSOLE,UTIL_VarArgs("You are now taking damage at %i%% of normal\n",RoundFloatToInt(GetDamageReceivedScale()*100.0f)));
}

void CHL2MP_Player::SetAccount(int accountID)
{
	m_iAccountID = accountID;
	m_iLoggedOn = 1;
}

const char *CHL2MP_Player::GetCharacterName()
{
	return m_szCharacterName;
}

void CHL2MP_Player::SetCharacterName(const char *name)
{
	Q_snprintf(m_szCharacterName,sizeof(m_szCharacterName), name);
}

void CHL2MP_Player::LogOff(bool bEndOfGame)
{
	m_iChangedBits |= BITS_CHANGED_LOGGING_OFF;
	if ( IsInCharacter() )
		LeaveCharacter(bEndOfGame);

	m_iAccountID = -1;
	m_iLoggedOn = 0;
}

void CHL2MP_Player::EnterCharacter(int characterID)
{
	StopObserverMode();
	m_iCharacterID = characterID;
	m_iLoggedOn = 2;
	m_flPlayStart = gpGlobals->curtime;
	HL2MPRules()->PlayerEnteredCharacter(this);

	RemoveAllItems(true);
	ChangeTeam(TEAM_UNASSIGNED);
	respawn(this,false);
	ResetGameExp();
}

void CHL2MP_Player::LeaveCharacter(bool bEndOfGame)
{
	RemoveScoreToken(GetNumScoreTokens()); // remove any score tokens this player may have (must call this before PlayerLeftCharacter()
	
	if ( HL2MPRules() )
		HL2MPRules()->PlayerLeftCharacter(this);

	m_iChangedBits |= BITS_CHANGED_LEAVING_CHAR;
	HL2MPRules()->SavePlayerData(this);
	
	m_iCharacterID = -1;
	m_iLoggedOn = 1;
	m_iFaction = 0;
	ResetScores(); // reset frag count and death count
	m_DamageGivenThisLife.Purge(); // clear my damage record when i exit a character (but not when I die)

	ResetGameExp();

	if ( !bEndOfGame )
	{// we don't care about this stuff at the end of the game, but it caused crashes
		Q_snprintf(m_szCharacterName,sizeof(m_szCharacterName), ""); // leaving character ... therefore have no character/suit name
	
		RemoveActiveStuff();
		RemoveAllItems(true);

		ChangeTeam(TEAM_SPECTATOR);
		respawn(this,true);
		Reset();
	}

	// remove all my abilities and afflictions (stops spectators spreading plague)
	RemoveAllModules();
	RemoveAllBuffs();
}

void CHL2MP_Player::AddScoreToken(int num)
{
	BaseClass::AddScoreToken(num);
	HL2MPRules()->AdjustFactionScoreTokenCount(GetFaction(), num);
}

// this is called even if the player has none to remove, and simply returns false. Only remove from the faction total what we actually remove!
bool CHL2MP_Player::RemoveScoreToken(int num)
{
	int numRemoved = min(num, GetNumScoreTokens());
	bool retVal = BaseClass::RemoveScoreToken(num);
	if ( numRemoved > 0 )
		HL2MPRules()->AdjustFactionScoreTokenCount(GetFaction(), -numRemoved);
	return retVal;
}

void CHL2MP_Player::IncrementSpree()
{
    m_iSpreeLength ++;
    if ( m_iBiggestSpree < m_iSpreeLength )
        m_iBiggestSpree = m_iSpreeLength;
	if ( m_iSpreeLength >= mc_spree_start.GetInt() )
	{
		ApplyBuff(BUFF_SPREE, NULL, m_iSpreeLength);
		if ( m_iSpreeLength == mc_spree_start.GetInt() )
			m_iSprees ++;
		
		if ( m_iSpreeLength == mc_spreewar_start.GetInt() && !HL2MPRules()->IsTeamplay() )
	    {
	        HL2MPRules()->StartSpreeWar(this);
	        m_iSpreeWars ++;
		}
    }

    char *spreeSound = NULL;
    bool localOnly = false;
    switch ( m_iSpreeLength )
    {
    case 3: spreeSound = "BoSS.Spree1"; localOnly = true; break;
    case 4: spreeSound = "BoSS.Spree2"; localOnly = true; break;
    case 5: spreeSound = "BoSS.Spree3"; break;
    case 10: spreeSound = "BoSS.Spree4"; break;
    case 15: spreeSound = "BoSS.Spree5"; break;
    case 20: spreeSound = "BoSS.Spree6"; break;
    case 25: spreeSound = "BoSS.Spree7"; break;
    default: return; // don't continue unless we have a sound
    }
    
    CRecipientFilter filter;
    if ( localOnly )
        filter.AddRecipient( this );
    else
        filter.AddAllPlayers();
    UserMessageBegin( filter, "SendAudio" );
    WRITE_STRING( spreeSound );
    MessageEnd();
}

void CHL2MP_Player::ResetSpree()
{
	if ( GetSpree() >= mc_spree_start.GetInt() )
	{
		CSingleUserRecipientFilter user( this );
		user.MakeReliable();
		UserMessageBegin( user, "Spree" );
			WRITE_BYTE( 0 ); // stop spree
		MessageEnd();
	}

	RemoveBuff(BUFF_SPREE);
	m_iSpreeLength = 0;
}

void CHL2MP_Player::PlayDeathTaunt()
{
	int PlaySound = random->RandomInt(0,100);

	if ( PlaySound > 60 )
	{
		CRecipientFilter filter;
		filter.AddRecipient( this );
		UserMessageBegin( filter, "SendAudio" );
		WRITE_STRING( "BoSS.Deaths" );
		MessageEnd();
	}
}

void CHL2MP_Player::PlayWelcomeMessage()
{
	CRecipientFilter filter;
	filter.AddRecipient( this );
	UserMessageBegin( filter, "SendAudio" );
	WRITE_STRING( "BoSS.Hello" );
	MessageEnd();
}


void CC_DestroyLasers( void )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() ) );
	pPlayer->GetLimitedQuantities()->Reset(LQ_LASER);
}

static ConCommand cc_destroylasers("destroylasers", CC_DestroyLasers, "Destroys all of your lasers");

void CC_DestroyMinions( void )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() ) );
	pPlayer->GetLimitedQuantities()->Reset(LQ_MINION);
	pPlayer->ForceMinionCount(0); // recalculate
}

static ConCommand cc_destroyminions("destroyminions", CC_DestroyMinions, "Destroys all your minions.");

#define SAY_TO_CONSOLE( text ) if ( pCommandPlayer ) ClientPrint(pCommandPlayer, HUD_PRINTCONSOLE, text ); else Msg( text );

// this standardises the fiddly code of joining a player name together out of multiple arguments, while (potentially) ignoring the last N parameters - and using yourself if no name parameters are present
#define INIT_PLAYER_NAME_COMMAND(reserveParams) \
	CHL2MP_Player *pCommandPlayer = ToHL2MPPlayer(UTIL_GetCommandClient()); \
	CHL2MP_Player *pTargetPlayer = NULL; \
	\
	if ( args.ArgC() == 1+reserveParams ) \
	{ \
		pTargetPlayer = pCommandPlayer; \
		if ( pTargetPlayer == NULL || !pTargetPlayer->IsInCharacter() ) \
		{ \
			SAY_TO_CONSOLE("Command needs a valid target\n"); \
			return; \
		} \
	} \
	else \
	{ \
		char playerName[NAME_LENGTH]; \
		Q_snprintf(playerName,sizeof(playerName),args[1]); \
		for ( int i=2; i<args.ArgC()-1-reserveParams; i++ ) \
			V_strcat( playerName, UTIL_VarArgs(" %s",args[i]), sizeof( playerName ) ); \
		\
		for (int i = 1; i <= gpGlobals->maxClients; i++ ) \
		{ \
			CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i ); \
			if ( !pPlayer || !FStrEq(playerName,pPlayer->GetPlayerName()) ) \
				continue; \
			\
			pTargetPlayer = pPlayer; \
			break; \
		} \
		\
		if ( pTargetPlayer == NULL || !pTargetPlayer->IsInCharacter() ) \
		{ \
			SAY_TO_CONSOLE(UTIL_VarArgs("Player not recognised: %s\n", playerName)); \
			return; \
		} \
	}

#ifndef RELEASE
extern ConVar mc_dev_debug_factions;
#endif
void CC_AddExp( const CCommand &args )
{
	INIT_PLAYER_NAME_COMMAND(1);

	int amt = atoi(args[args.ArgC()-1]);
	if ( amt < 1 )
	{
		SAY_TO_CONSOLE(UTIL_VarArgs("Invalid amount: %s\n", args[args.ArgC()-1]));
		return;
	}

	pTargetPlayer->AddExp(amt);
#ifndef RELEASE
	if ( mc_dev_debug_factions.GetInt() == 1 )
		HL2MPRules()->AwardFactionExp(pTargetPlayer, amt);
#endif
	SAY_TO_CONSOLE("Success\n");
}

static ConCommand cc_addexp("mc_admin_add_exp", CC_AddExp, "Add experience to a specific player - use one of the following command formats:\n  mc_admin_add_exp <player name> <amount> (to affect the named player)\n  addexp <amount> (to affect yourself)", FCVAR_CHEAT);
static ConCommand addexp("addexp", CC_AddExp, "This command has been renamed. See mc_admin_add_exp instead.", FCVAR_CHEAT|FCVAR_HIDDEN);

extern McConVar mc_max_level;
void CC_SetLevel( const CCommand &args )
{
	INIT_PLAYER_NAME_COMMAND(1)

	int level = atoi(args[args.ArgC()-1]);
	if ( level < 1 || level > mc_max_level.GetInt() )
	{
		SAY_TO_CONSOLE(UTIL_VarArgs("Invalid level: %s\n", args[args.ArgC()-1]));
		return;
	}

	if ( level < pTargetPlayer->GetLevel() )
	{
		SAY_TO_CONSOLE("Cannot reverse player progression, can only be used to level up!\n");
		return;
	}

	pTargetPlayer->AddExp(TotExpForLevelUp(level-1) - pTargetPlayer->GetTotalExp());
	SAY_TO_CONSOLE("Success\n");
}

static ConCommand cc_setlevel("mc_admin_set_level", CC_SetLevel, "Add experience to a specific player, to bring them to the specified level - use one of the following command formats:\n  mc_admin_set_level <player name> <level> (to affect the named player)\n  mc_admin_set_level <level> (to affect yourself)", FCVAR_CHEAT);


void CC_RefundModules( const CCommand &args )
{
	INIT_PLAYER_NAME_COMMAND(0)

	// now need to clear target all player's modules, and ensure that this change will be propagated to the database also
	pTargetPlayer->RemoveAllModules();

	pTargetPlayer->SerializeModuleData();
	int abilitypoints = mc_perlevel_modulepoints.GetInt();
 	pTargetPlayer->SetModulePoints( abilitypoints * pTargetPlayer->GetLevel() );
	
	SAY_TO_CONSOLE("Success\n");
}

static ConCommand cc_refundmodules("mc_admin_refund_modules", CC_RefundModules, "Refunds all module points spent by a specific player - use one of the following command formats:\n  mc_admin_refund_modules <player name> (to affect the named player)\n  mc_admin_refund_modules (to affect yourself)", FCVAR_CHEAT);

void CC_RefundModule( const CCommand &args )
{
	INIT_PLAYER_NAME_COMMAND(1)

	Module *a = GetModule(args[args.ArgC()-1]);
	if ( a == NULL )
	{
		SAY_TO_CONSOLE(UTIL_VarArgs("Module not recognised: %s\nFor a list of all module command names, type 'listmodules'\n", args[args.ArgC()-1]));
		return;
	}	

	// now need to clear target all player's modules, and ensure that this change will be propagated to the database also
	int level = pTargetPlayer->GetModuleLevel(a);
	pTargetPlayer->SetModuleLevel(a,0);
	pTargetPlayer->SerializeModuleData();

	pTargetPlayer->AddModulePoints(a->GetUpgradeCost()*level);
	
	SAY_TO_CONSOLE("Success\n");
}

static ConCommand cc_refundmodule("mc_admin_refund_module", CC_RefundModule, "Refunds all module points spent on a specified module by a specific player - use one of the following command formats:\n  mc_admin_refund_module <player name> <module command name> (to affect the named player)\n  mc_admin_refund_module <module command name> (to affect yourself)", FCVAR_CHEAT);
void CC_NumMonsters( void )
{
	CHL2MP_Player *pCommandPlayer = ToHL2MPPlayer(UTIL_GetCommandClient());
	ClientPrint(pCommandPlayer,HUD_PRINTCONSOLE,UTIL_VarArgs("%i monsters\n",HL2MPRules()->NumMonsters()));
}

static ConCommand cc_nummonsters("nummonsters", CC_NumMonsters, "Reports how many pvm monsters are currently in game");

// ===================================================================================
// minion formation handling
// ===================================================================================

int CHL2MP_Player::AddMinionToFormation(CAI_BaseNPC *pMinion)
{
	int retVal = -1;
	for ( int i=0; i<MINION_FORMATION_LIMIT; i++ )
		if ( m_hFormationMinions[i] == NULL )
		{
			m_hFormationMinions[i].Set(pMinion);
			retVal = i;
			break;
		}
	int newNum = NumMinionsInFormation(); // recalculate .... hmm, this won't show the right number when beyond the limit
	ApplyBuff(BUFF_MINION_DRAIN, NULL, newNum);
	return retVal; // -1 if there isn't an available slot
}

void CHL2MP_Player::RemoveMinionFromFormation(CAI_BaseNPC *pMinion)
{
	for ( int i=0; i<MINION_FORMATION_LIMIT; i++ )
		if ( m_hFormationMinions[i].Get() == pMinion )
		{
			for ( int j=i+1; j<MINION_FORMATION_LIMIT; j++ )
				m_hFormationMinions[j-1].Set(m_hFormationMinions[j].Get());
			m_hFormationMinions[MINION_FORMATION_LIMIT-1].Set(NULL);
			break;
		}

	int newNum = NumMinionsInFormation();
	RemoveBuff(BUFF_MINION_DRAIN);
	if ( newNum > 0 )
		ApplyBuff(BUFF_MINION_DRAIN, NULL, newNum);
}

int CHL2MP_Player::NumMinionsInFormation()
{
	for ( int i=0; i<MINION_FORMATION_LIMIT; i++ )
		if ( m_hFormationMinions[i] == NULL )
		{
			m_iNumMinions = i;
			return i;
		}
	m_iNumMinions = MINION_FORMATION_LIMIT;
	return MINION_FORMATION_LIMIT;
}

// ideally, this would also output a tolerance, so that for formation positions, tolerance was very low, but for "formation is full, just move to player" positions, tolerance was high
#define FORMATION_FORWARD_DIST	128
#define FORMATION_SIDE_DIST		64
#define FORMATION_REAR_DIST		64
#define FLYING_MINION_FORMATION_HEIGHT 48
extern ConVar ai_debug_follow;
Vector CHL2MP_Player::GetFormationPosition(CAI_BaseNPC *pMinion)
{
	if ( m_hMinionTarget.Get() != NULL )
		if ( m_hMinionTarget.Get()->IsAlive() )
			m_vecMinionTarget = m_hMinionTarget.Get()->WorldSpaceCenter();
		else
			m_hMinionTarget.Set(NULL);

	if ( m_vecMinionTarget != vec3_origin )
	{
		if ( ai_debug_follow.GetBool() )
			NDebugOverlay::Line( pMinion->GetAbsOrigin(), m_vecMinionTarget, 255, 160, 0, true, 0.5f );
		if ( pMinion->GetNavType() == NAV_FLY )
			return m_vecMinionTarget + Vector(0,0,FLYING_MINION_FORMATION_HEIGHT);
		return m_vecMinionTarget;
	}

	
	Vector forward, right, up;
	AngleVectors( EyeAngles(), &forward, &right, &up );	
	
	if ( !pMinion->IsRegularMinionType() )
	{// buzz annoyingly around my eye position - this only means manhacks for now, so not bothering checking if they fly or not
		return GetAbsOrigin() + forward * FORMATION_FORWARD_DIST + Vector(0,0,FLYING_MINION_FORMATION_HEIGHT);
	}

	int minionIndex = -1;
	for ( int i=0; i<MINION_FORMATION_LIMIT; i++ )
		if ( m_hFormationMinions[i].Get() == pMinion )
		{
			minionIndex = i;
			break;
		}
		
	if ( minionIndex == -1 )
	{
	//	can't find me in the list, try to add it
		minionIndex = AddMinionToFormation(pMinion);
		if ( minionIndex == -1 )
		{
			if ( pMinion->GetNavType() == NAV_FLY )
				return GetAbsOrigin() + Vector(0,0,FLYING_MINION_FORMATION_HEIGHT);
			return GetAbsOrigin();
		}
	}
		
	forward.z = 0; right.z = 0;
	VectorNormalize(forward);
	VectorNormalize(right);
	
	Vector offset = vec3_origin;	
	switch ( NumMinionsInFormation() )
	{
		case 1:
			offset = forward * FORMATION_FORWARD_DIST + right * FORMATION_SIDE_DIST * 0.333f; // *slightly* to the side
			break;
		case 2:
			if ( minionIndex == 0 )
				offset = forward * FORMATION_FORWARD_DIST - right * FORMATION_SIDE_DIST;
			else
				offset = forward * FORMATION_FORWARD_DIST + right * FORMATION_SIDE_DIST;
			break;
		case 3:
			if ( minionIndex == 0 )
				offset = forward * FORMATION_FORWARD_DIST - right * FORMATION_SIDE_DIST;
			else if ( minionIndex == 1 )
				offset = forward * FORMATION_FORWARD_DIST + right * FORMATION_SIDE_DIST;
			else
				offset = -forward * FORMATION_REAR_DIST;
			break;
		case 4:
			if ( minionIndex == 0 )
				offset = forward * FORMATION_FORWARD_DIST - right * FORMATION_SIDE_DIST;
			else if ( minionIndex == 1 )
				offset = forward * FORMATION_FORWARD_DIST + right * FORMATION_SIDE_DIST;
			else if ( minionIndex == 2 )
				offset = -forward * FORMATION_REAR_DIST - right * FORMATION_SIDE_DIST;
			else
				offset = -forward * FORMATION_REAR_DIST + right * FORMATION_SIDE_DIST;
			break;
		case 5: // for 5, move the rear 2 forward and outward so they're beside the player, and add a new one rear-center
			if ( minionIndex == 0 )
				offset = forward * FORMATION_FORWARD_DIST - right * FORMATION_SIDE_DIST;
			else if ( minionIndex == 1 )
				offset = forward * FORMATION_FORWARD_DIST + right * FORMATION_SIDE_DIST;
			else if ( minionIndex == 2 )
				offset = -right * FORMATION_SIDE_DIST * 1.5f;
			else if ( minionIndex == 3 )
				offset = right * FORMATION_SIDE_DIST * 1.5f;
			else
				offset = -forward * FORMATION_REAR_DIST;
			break;
		case 6: // for 6, have as per 5, but have two spread out in the rear
		case 7: // for 7, add one forward-center, further forward than the rest
		case 8: // and for 8, add one rear-center, further back than the rest
			if ( minionIndex == 0 )
				offset = forward * FORMATION_FORWARD_DIST - right * FORMATION_SIDE_DIST;
			else if ( minionIndex == 1 )
				offset = forward * FORMATION_FORWARD_DIST + right * FORMATION_SIDE_DIST;
			else if ( minionIndex == 2 )
				offset = -right * FORMATION_SIDE_DIST * 1.5f;
			else if ( minionIndex == 3 )
				offset = right * FORMATION_SIDE_DIST * 1.5f;
			else if ( minionIndex == 4 )
				offset = -forward * FORMATION_REAR_DIST - right * FORMATION_SIDE_DIST;
			else if ( minionIndex == 5 )
				offset = -forward * FORMATION_REAR_DIST + right * FORMATION_SIDE_DIST;
			else if ( minionIndex == 6 )
				offset = forward * FORMATION_FORWARD_DIST * 1.5f;
			else
				offset = -forward * FORMATION_REAR_DIST * 1.5f;
			break;
	}
	
	//return WorldSpaceCenter() + offset;
	// now do a trace from WorldSpaceCenter to WorldSpaceCenter + offset, for world geometry only.
	// then trace down from the end position just as far as WorldSpaceCenter().z - GetAbsOrigin().z, THATS our target position
	trace_t tr1, tr2;
	UTIL_TraceLine(EyePosition(), EyePosition() + offset + GetAbsVelocity(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr1);
	Vector targetPos;
	if ( tr1.DidHit() )
	{
		VectorNormalize(offset);
		targetPos = tr1.endpos - offset * 16;
	}
	else
		targetPos = tr1.endpos;
	UTIL_TraceLine(targetPos, targetPos - Vector(0,0,96), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr2);
	if ( ai_debug_follow.GetBool() )
		NDebugOverlay::Line( pMinion->GetAbsOrigin(), tr2.endpos, 255, 0, 0, true, 0.5f );
//	Msg(UTIL_VarArgs("Current %.1f %.1f %.1f, target %.1f %.1f %.1f\n",VectorExpand(pMinion->GetAbsOrigin()), VectorExpand(tr2.endpos)));

	if ( pMinion->GetNavType() == NAV_FLY )
		return tr2.endpos + Vector(0,0,FLYING_MINION_FORMATION_HEIGHT);
	return tr2.endpos;
}

#define USE_DOUBLETAP_DELAY	0.5f
void CHL2MP_Player::PlayerUse( void )
{
	BaseClass::PlayerUse();

	// currently we allow all USES to affect minion commands where we can't otherwise find a use entity.
	// not tracking whether the use call affected one or not per se, but this should suffice - i hope
//	Msg("Player use\n");
//	if ( (m_afButtonPressed & IN_USE) )
//		Msg("IN_USE pressed\n");
	if ( (m_afButtonPressed & IN_USE) && FindUseEntity() == NULL )
	{
		if ( NumMinionsInFormation() == 0 && GetLimitedQuantities()->GetCount(LQ_MANHACK) == 0 )
			m_vecMinionTarget = vec3_origin;
		else
		{
			if ( m_flLastUsePress + USE_DOUBLETAP_DELAY >= gpGlobals->curtime )
			{// double tap, all minions to FOLLOW mode
				m_vecMinionTarget = vec3_origin;
				m_hMinionTarget.Set(NULL);
				ClientPrint(this,HUD_PRINTCENTER, "Recalling minions");
			}
			else
			{// single tap, send all minions to currently targeted position
				CBaseEntity *pAimTarget = GetAimTarget(false);
 				if ( pAimTarget && (pAimTarget->IsPlayer() || pAimTarget->IsNPC()) ) 
				{// if there's a hostile where we're looking, target it
					m_hMinionTarget.Set(pAimTarget);
					m_vecMinionTarget = pAimTarget->GetAbsOrigin();
				}
				else // otherwise, move to position
				{
					trace_t tr1, tr2;
					Vector forward;
					AngleVectors( EyeAngles(), &forward );
					UTIL_TraceLine(EyePosition(), EyePosition() + forward * MAX_TRACE_LENGTH, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr1);
					Vector targetPos;
					if ( tr1.DidHit() )
						targetPos = tr1.endpos - forward * 16;
					else
						targetPos = tr1.endpos;
					UTIL_TraceLine(targetPos, targetPos - Vector(0,0,96), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr2);
					m_vecMinionTarget = tr2.endpos;
					//ClientPrint(this,HUD_PRINTCENTER, "Sending minions");
				}

				// and get them to play noises
				for ( int i=0; i<NumMinionsInFormation(); i++ )
					m_hFormationMinions[i].Get()->AlertSound();
			}
		}
		m_flLastUsePress = gpGlobals->curtime;
	}
}

float CHL2MP_Player::GetFactionalDamageScaleBoost()
{
	if ( !HL2MPRules()->IsTeamplay() || GetFaction() != FACTION_COMBINE )
		return 0.0f;

	return NumAlliesInRange() * pvm_buff_combine_scale.GetFloat();
}

void CHL2MP_Player::RegisterAttackOnEntity( CTakeDamageInfo &info, CBaseCombatCharacter *pVictim, bool isKillingBlow )
{
	if( !pVictim )
		return;

	// do the data logging stuff firstly ... all the stuff after this is just for shared experience
	BaseClass::RegisterAttackOnEntity(info, pVictim, isKillingBlow);
	
	for ( int i=0; i<m_DamageGivenThisLife.Count(); i++ )
		if ( m_DamageGivenThisLife[i].hVictim == pVictim )//&& m_DamageGivenThisLife[i].flDamage > 0 )
		{
			m_DamageGivenThisLife[i].flDamage += info.GetDamage();
			return;
		}

	playerattacks_t attack;
	attack.flDamage = info.GetDamage();
	//attack.flTime = gpGlobals->curtime;
	//attack.iDamageType = info.GetDamageType();
	//attack.iHitGroup = pVictim->LastHitGroup();
	attack.hVictim = pVictim;

	int attackIndex = m_DamageGivenThisLife.AddToTail();
	m_DamageGivenThisLife[ attackIndex ] = attack;
}

float CHL2MP_Player::GetAuxRechargeRate()
{
	return /*LEVEL(mod_recharge_amount, GetModuleLevel(GetModule(RECHARGE))) / mod_recharge_tick.GetFloat()*/ + mc_player_base_aux_recharge.GetFloat();
}

// used for saving module info in the database, in a single line of text rather than in several seperate entries
void CHL2MP_Player::SerializeModuleData()
{
	// alternates between module name & module level
	int len = sizeof(m_szSerializedModuleInfo);
	Q_strncpy(m_szSerializedModuleInfo, "", len);
	bool first = true;
	for ( int i=0; i<NUM_MODULES; i++ )
	{
		Module *m = GetModule(i);
		int level = m_iModules.Get(i);
		if ( level > 0 )
		{
			Q_strncat(m_szSerializedModuleInfo, UTIL_VarArgs(first ? "%s;%i" : ";%s;%i", m->GetCmdName(), level), len);
			first = false;
		}
	}
	
	if ( Q_strlen(m_szSerializedModuleInfo) > len * 0.8f )
		Warning("Serialized module data length is approaching limit, increase MAX_DB_STRING!\n");
		
	m_iChangedBits &= ~BITS_CHANGED_MODULES; // clear this flag
	m_iChangedBits |= BITS_CHANGED_CHARACTER; // character table now needs saved to database
}

const char *CHL2MP_Player::GetSerializedModules()
{
	return m_szSerializedModuleInfo;
}

void CHL2MP_Player::SetSerializedModuleData(const char *data)
{
	Q_snprintf(m_szSerializedModuleInfo, sizeof(m_szSerializedModuleInfo), data);
}

void CC_MenuView( void )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() ) );

	if ( pPlayer && !pPlayer->IsInCharacter() )
	{
		Vector pos = Vector(1867.65,414.93,226.15); //Vector(1838.82, 283.84, 228.97); 
		QAngle ang = QAngle(10.04,-158.17,0);//QAngle(14.01,-167.67,0);
		
		pPlayer->SetAbsOrigin(pos);
		pPlayer->SnapEyeAngles(ang);
	}
}

static ConCommand cc_menuview("menuview", CC_MenuView, "Snaps eye position to the correct location for recording background menu screenshots");



void CC_ListMonsters( void )
{
	if ( !sv_cheats->GetBool() )
		return;
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() ) );

	if ( pPlayer )
	{
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "All monster types:\n");
		int numNpcTypes = g_pNPCInfo.Count();
		for ( int i=0; i<numNpcTypes; i++ )
			ClientPrint(pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs("%s\n",g_pNPCInfo[i]->m_szTypeName));
	}
}

static ConCommand cc_listmonsters("listmonsters", CC_ListMonsters, "Lists all monster types that can be created using mc_dev_creatnpc");


void CC_BuffMyself( const CCommand& args )
{
	if ( !sv_cheats->GetBool() )
		return;

	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() ) );
	int num = args.ArgC();
	if ( !pPlayer || num < 2 )
		return;
	
	int i = atoi(args[1]);
	if ( i > -1 && i < NUM_BUFFS )
	{
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs("Applying buff: %s\n", GetBuff(i)->GetName()));
		pPlayer->ApplyBuff(i);
	}
	else
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Invalid buff index\n");
}

static ConCommand cc_buffmyself("buffmyself", CC_BuffMyself, "Applies the buff of the relevant number to yourself");

void CC_UnBuffMyself( const CCommand& args )
{
	if ( !sv_cheats->GetBool() )
		return;

	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() ) );
	int num = args.ArgC();
	if ( !pPlayer || num < 2 )
		return;
	
	int i = atoi(args[1]);
	if ( i > -1 && i < NUM_BUFFS )
	{
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs("Removing buff: %s\n", GetBuff(i)->GetName()));
		pPlayer->RemoveBuff(i);
	}
	else
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Invalid buff index\n");
}

static ConCommand cc_unbuffmyself("unbuffmyself", CC_UnBuffMyself, "Removes the buff of the relevant number from yourself");

#ifndef RELEASE
void CC_AddToken( const CCommand& args )
{
	if ( !sv_cheats->GetBool() )
		return;

	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() ) );
	if ( !pPlayer )
		return;
	
	pPlayer->AddScoreToken();
}

static ConCommand cc_addtoken("mc_dev_addtoken", CC_AddToken, "Adds a score token to the current player");

extern CBaseEntity *FindPickerEntity( CBasePlayer *pPlayer );
void CC_AddTokenTarget( const CCommand& args )
{
	if ( !sv_cheats->GetBool() )
		return;

	CBaseEntity *pEntity = FindPickerEntity( UTIL_GetCommandClient() );
	if ( pEntity && pEntity->MyCombatCharacterPointer() )
		pEntity->MyCombatCharacterPointer()->AddScoreToken();
}

static ConCommand cc_addtoken_target("mc_dev_addtoken_target", CC_AddTokenTarget, "Adds a score token to the targetted entity player");
#endif