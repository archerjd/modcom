#include "cbase.h"
#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include <vgui/ILocalize.h>
	#include <vgui_controls/Label.h>	// seemingly needed for g_pVGuiLocalize
#else
	#include "hl2mp_player.h"
	#include "hl2_player.h"
	#include "grenade_frag.h" // needed for the grenade abilities
	#include "basegrenade_shared.h" // likewise
	#include "ai_basenpc.h" // for any monster spawning abilities
	#include "prop_combine_ball.h"
	#include "weapon_slam.h"
	#include "hl2mp/grenade_tripmine.h"
	#include "hl2/npc_turret_floor.h"
	#include "basecombatweapon_shared.h"
	#include "weapon_hl2mpbase.h"
	#include "npc_crow.h"
	#include "particle_parse.h"
	#include "grenade_spit.h"
	#include "props.h"
	#include "episodic/npc_hunter.h"
	#include "shot_manipulator.h"
	#include "modcom/teleport_blocker.h"
	#include "beam_flags.h"
#endif
#include "basegrenade_shared.h"
#include "modules.h"
#include "buffs.h"
#include "mcconvar.h"
#include "particle_effects.h"
#include "hl2mp/hl2mp_gamerules.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
	All abilities are to be created in this file, and must be added into the CreateModules function
	at the end of the file. That is called from the gamerules constructor, on both server and client.

	Modules should extend one of the following base classes:
	PassiveModule, InstantModule, ContinuousModule, ToggledRepeatingModule, or MaintainedRepeatingModule

	For details of each, see the header.
*/

extern int RandomlyRoundFloat(float f);

Module *g_Modules[NUM_MODULES];

#ifndef CLIENT_DLL
#define VarArgs UTIL_VarArgs 
extern CNPCTypeInfo *GetNPCInfo(const char *name);

// these are NOT loaded from the stats files. Servers shouldn't be able to delete / modify these.
CNPCTypeInfo *g_MinionAntlion, *g_MinionAntlionWorker, *g_MinionFastZombie, *g_MinionZombie, *g_MinionTurret, *g_MinionVortigaunt, *g_MinionManhack, *g_Crow;

// forward declaration of useful functions
CAI_BaseNPC *CreateNPC(CNPCTypeInfo *type,Vector origin,QAngle angles,CHL2MP_Player *pOwner,int level,bool fallToGround=true);
#else
wchar_t *percent(float f);
wchar_t *toStr(int i);
wchar_t *toStr(float f);
extern McConVar minion_drain_scale; // used by various descriptions


#define DATA_TABLE_ITEM(paramnum,name,param) \
	virtual wchar_t* GetDataTableValue ## paramnum (int level) \
	{ \
		return param; \
	} \
	  \
	virtual wchar_t* GetDataTableRowName ## paramnum () \
	{ \
		return name; \
	}
#endif

extern McConVar mc_player_max_minions;

#define DECLARE_MODULE(className, type, levelType, cmdname) \
	McConVar mod_##cmdname##_enabled("mod_" #cmdname "_enabled", "1", FCVAR_NOTIFY|FCVAR_REPLICATED, "", true, 0, true, 1); \
	class className : public type , levelType \
	{ \
	public: \
		className () { Init(#cmdname); } \
		virtual bool IsPurchasable() { return mod_##cmdname##_enabled.GetBool(); }

#define END_MODULE() };

//-----------------------------------------------------------------------------
// Purpose: Impact. Player's ranged weapon damage is increased 3.75...37.5%
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_impact);

DECLARE_MODULE(Impact, PassiveModule, TenLevelModule, impact)
	virtual const char *DisplayName() { return "Impact"; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
		pOwner->ScaleDamageDealt( 1 + LEVEL(mod_impact,level) );
	}

	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)
	{
		pOwner->ScaleDamageDealt( 1.0f / (1 + LEVEL(mod_impact,level)) ); // divide by the above amount
	}
	virtual int GetAIDescription() { return PASSIVE; }
#else
	DATA_TABLE_ITEM(1, L"Bonus %", percent(LEVEL(mod_impact,level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Brute Force. melee damage enhancement - Player's melee damage is increased,
// they also get a damage buff on melee kills
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_bruteforce);
extern LEVEL_EXTERN(mod_bruteforce_buff_stacks);
extern LEVEL_EXTERN(mod_bruteforce_buff_duration);
extern LEVEL_EXTERN(mod_bruteforce_buff);

DECLARE_MODULE(BruteForce, PassiveModule, TenLevelModule, bruteforce)
	virtual const char *DisplayName() { return "Brute Force"; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)	{
		// brute force damage is actually handled directly in melee code
		//pOwner->ScaleDamageDealt( BruteForceDamageScale(level) );
	}

	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)	{
		//pOwner->ScaleDamageDealt( 1.0f / BruteForceDamageScale(level) ); // divide by the above amount
	}
#else
	DATA_TABLE_ITEM(1, L"Melee %", percent(LEVEL(mod_bruteforce, level)))
	DATA_TABLE_ITEM(2, L"Buff %", percent(LEVEL(mod_bruteforce_buff, level)))
	DATA_TABLE_ITEM(3, L"Buff time", toStr(LEVEL(mod_bruteforce_buff_duration, level)))
	DATA_TABLE_ITEM(4, L"Max stacks", toStr(LEVEL(mod_bruteforce_buff_stacks, level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Resistance. All damage received is reduced 9...50%
//-----------------------------------------------------------------------------
/*
class Resistance : public PassiveModule, TenLevelModule
{
public:
	virtual const char *DisplayName() { return "Resistance"; }
	virtual const char *CmdName() { return "resistance"; }

	// resist 5% per level
	float ScaleFactor(int level)
	{
		return 1.0f - LEVEL(mod_resistance, level);
	}
#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)	
	{
		pOwner->ScaleDamageReceived( ScaleFactor(level) );
//		ClientPrint(pOwner,HUD_PRINTNOTIFY,UTIL_VarArgs("Starting %s effect\n",DisplayName()));	
	}

	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)	
	{
		pOwner->ScaleDamageReceived( 1.0f / ScaleFactor(level) ); // divide by the above amount
//		ClientPrint(pOwner,HUD_PRINTNOTIFY,UTIL_VarArgs("Stopping %s effect\n",DisplayName()));	
	}
#else
	virtual wchar_t* GetDescriptionParameter1(int level)
	{
		return percent(1.0f-ScaleFactor(level)); // the reduction as a percent (eg 3%)
	}
#endif
};
*/

//-----------------------------------------------------------------------------
// Purpose: Damage Amplifier. Target player takes 60...150% more damage for 0.5...5 seconds.
// increases all damage target takes by 60% + 10% per level, for 0.5s per level
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_damageamp_drain);
extern LEVEL_EXTERN(mod_damageamp_cooldown);
extern LEVEL_EXTERN(mod_damageamp);
extern LEVEL_EXTERN(mod_damageamp_duration);
DECLARE_MODULE(DamageAmp, InstantModule, TenLevelModule, damage_amp)
	virtual const char *DisplayName() { return "Damage Amp"; }
	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_damageamp_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_damageamp_cooldown, level); }

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		// find a player or monster in front of owner, apply affliction
		CBaseCombatCharacter *pTarget = (CBaseCombatCharacter*)(pOwner->GetAimTarget(false));

		if ( pTarget && pTarget->ApplyBuff(DEBUFF_DAMAGE_AMP,pOwner,level) )
			return true;
		else
			return false;
	}
	virtual int GetAIDescription() { return INSTANT_TARGET_EFFECT; }
#else
	DATA_TABLE_ITEM(1, L"Bonus %", percent(LEVEL(mod_damageamp, level) - 1.0f))
	DATA_TABLE_ITEM(2, L"Duration", toStr(LEVEL(mod_damageamp_duration, level)))
#endif
END_MODULE()

extern LEVEL_EXTERN(mod_weaken_drain);
extern LEVEL_EXTERN(mod_weaken_cooldown);
extern LEVEL_EXTERN(mod_weaken_duration);
extern LEVEL_EXTERN(mod_weaken_damagescale);
extern LEVEL_EXTERN(mod_weaken_sprintscale);
DECLARE_MODULE(Weaken, InstantModule, TenLevelModule, weaken)
	virtual const char *DisplayName() { return "Weaken"; }
	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_weaken_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_weaken_cooldown, level); }

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		// find a player or monster in front of owner, apply affliction
		CBaseCombatCharacter *pTarget = (CBaseCombatCharacter*)(pOwner->GetAimTarget(false));

		return ( pTarget && pTarget->ApplyBuff(DEBUFF_WEAKEN,pOwner,level) );
	}
	virtual int GetAIDescription() { return INSTANT_TARGET_EFFECT; }
#else
	DATA_TABLE_ITEM(1, L"Damage %", percent(1.0f-LEVEL(mod_weaken_damagescale, level)))
	DATA_TABLE_ITEM(2, L"Sprint %", percent(1.0f-LEVEL(mod_weaken_sprintscale, level)))
	DATA_TABLE_ITEM(3, L"Duration", toStr(LEVEL(mod_weaken_duration, level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Regeneration (Heals player when player is under 75...100% health.)
//-----------------------------------------------------------------------------
/*
extern LEVEL_EXTERN(mod_regen_interval;
extern LEVEL_EXTERN(mod_regen_amount;
extern LEVEL_EXTERN(mod_regen_maxhealingfraction);
DECLARE_MODULE(Regeneration, PassiveRepeatingModule, TenLevelModule, regeneration)
	virtual const char *DisplayName() { return "Regeneration"; }
	virtual float GetTickInterval(int level) { return LEVEL(mod_regen_interval, level); }
	
	int HealingLimit(CBaseCombatCharacter *pOwner)
	{
		return  pOwner->GetMaxHealth() * LEVEL(mod_regen_maxhealingfraction, pOwner->GetModuleLevel(this)) ;
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		if ( pOwner->IsBuffActive(DEBUFF_ATTRITION) )
			return false;

		if ( pOwner->GetHealth() >= HealingLimit(pOwner) )
			return false;

		int healAmt = LEVEL(mod_regen_amount, level);
		if ( pOwner->GetHealth() + healAmt > HealingLimit(pOwner) )
			healAmt = HealingLimit(pOwner) - pOwner->GetHealth(); // heal only up to the limit

		pOwner->TakeHealth( healAmt, DMG_GENERIC );
//		ClientPrint(pOwner,HUD_PRINTNOTIFY,UTIL_VarArgs("Doing %s effect\n",DisplayName()));
		return true;
	}
	virtual int GetAIDescription() { return PASSIVE; }
#else
	DATA_TABLE_ITEM(1, L"Health / sec", toStr(LEVEL(mod_regen_amount, level)/GetCooldown(level)))
	DATA_TABLE_ITEM(2, L"Max health %", percent(LEVEL(mod_regen_maxhealingfraction, level)))
#endif
END_MODULE()
*/
//-----------------------------------------------------------------------------
// Purpose: Armor regeneration
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_armorregen_interval);
extern LEVEL_EXTERN(mod_armorregen_amount);
extern LEVEL_EXTERN(mod_armorregen_maxfraction);
extern LEVEL_EXTERN(mod_armorregen_minaux);
extern LEVEL_EXTERN(mod_armorregen_cooldown);
DECLARE_MODULE(ArmorRegen, PassiveRepeatingModule, TenLevelModule, armorregen)
	virtual const char *DisplayName() { return "Armor Regen"; }
	virtual bool ShouldTurnOffBeforeUpgrading() { return false; }
	virtual float GetCooldown(int level) { return LEVEL(mod_armorregen_cooldown, level); }
	virtual float GetTickInterval(int level) { return LEVEL(mod_armorregen_interval, level); }
	virtual bool IsSuppressed(CBaseCombatCharacter *pOwner)
	{
		if ( pOwner->IsBuffActive(DEBUFF_SUPPRESS_ARMORREGEN) )
			return true;
		return Module::IsSuppressed(pOwner);
	}
#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pOwner);
		if ( !pOwnerPlayer )
			return false;

		int limit =  pOwnerPlayer->GetMaxArmor() *LEVEL(mod_armorregen_maxfraction, pOwnerPlayer->GetModuleLevel(this)) ;
		if ( pOwnerPlayer->ArmorValue() >= limit )
			return false;

		if ( pOwnerPlayer->IsBuffActive(DEBUFF_ATTRITION) )
			return false;

		if ( pOwnerPlayer->IsBuffActive(DEBUFF_SUPPRESS_ARMORREGEN) )
			return false;

		int minaux = LEVEL(mod_armorregen_minaux, level);
		if ( pOwnerPlayer->GetAuxPower() <= minaux )
			return false;

		pOwnerPlayer->IncrementArmorValue( LEVEL(mod_armorregen_amount, level), limit );
		pOwnerPlayer->DrainAuxPower( LEVEL(mod_armorregen_amount, level) );
		return true;
	}
#else
	DATA_TABLE_ITEM(1, L"Armor / sec", toStr(LEVEL(mod_armorregen_amount, level)/GetCooldown(level)))
	DATA_TABLE_ITEM(2, L"Max armor %", percent(LEVEL(mod_armorregen_maxfraction, level)))
#endif
END_MODULE()
//-----------------------------------------------------------------------------
// Purpose: Ammo Regen - Regenerate pistol, SMG, AR, and Shotgun priamry ammo.
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_ammoregen_interval);
extern LEVEL_EXTERN(mod_ammoregen_pistol);
extern LEVEL_EXTERN(mod_ammoregen_ar2);
extern LEVEL_EXTERN(mod_ammoregen_smg1);
extern LEVEL_EXTERN(mod_ammoregen_buckshot);
DECLARE_MODULE(AmmoRegen, PassiveRepeatingModule, TenLevelModule, ammoregen)
	virtual const char *DisplayName() { return "Ammo Regen"; }

	virtual float GetTickInterval(int level)
	{
		return LEVEL(mod_ammoregen_interval, level);
	}
	
#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		if ( pOwner->IsBuffActive(DEBUFF_ATTRITION) )
			return false;

		if ( pOwner->IsPlayer() )
		{
			CBasePlayer *pPlayer = ToBasePlayer(pOwner);
			pPlayer->GiveAmmo( (int)LEVEL(mod_ammoregen_pistol, level),	 "Pistol", true );
			pPlayer->GiveAmmo( (int)LEVEL(mod_ammoregen_ar2, level),	 "AR2", true );
			pPlayer->GiveAmmo( (int)LEVEL(mod_ammoregen_smg1, level),	 "SMG1", true );
			pPlayer->GiveAmmo( (int)LEVEL(mod_ammoregen_buckshot, level),"Buckshot", true );
		}
		return true;
	}
#else
	DATA_TABLE_ITEM(1, L"Interval", toStr(LEVEL(mod_ammoregen_interval, level)))
	DATA_TABLE_ITEM(2, L"Pistol", toStr((int)LEVEL(mod_ammoregen_pistol, level)))
	DATA_TABLE_ITEM(3, L"SMG1", toStr((int)LEVEL(mod_ammoregen_smg1, level)))
	DATA_TABLE_ITEM(4, L"AR2", toStr((int)LEVEL(mod_ammoregen_ar2, level)))
	DATA_TABLE_ITEM(5, L"Shotgun", toStr((int)LEVEL(mod_ammoregen_buckshot, level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Shockwave
// Drains power from all nearby players
// power is added to the owner's power, and the victim takes damage equal to the power drained.
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_shockwave_victim_drain);
extern LEVEL_EXTERN(mod_shockwave_damage_ratio);
extern LEVEL_EXTERN(mod_shockwave_aux_transfer_ratio);
extern LEVEL_EXTERN(mod_shockwave_cooldown);
extern LEVEL_EXTERN(mod_shockwave_drain);
extern LEVEL_EXTERN(mod_shockwave_radius);
DECLARE_MODULE(Shockwave, InstantModule, TenLevelModule, shockwave)
	virtual const char *DisplayName() { return "Shockwave"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_shockwave_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_shockwave_cooldown, level); }

	virtual const char *ParticleEffect() { return "mind absorb"; }
	virtual const char *LocalParticleEffect() { return "mind absorb local"; }

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		float maxDrain = LEVEL(mod_shockwave_victim_drain, level);
		if ( maxDrain == 0 ) // if we can't drain any power, don't do the effect 
			return false;

		CBaseEntity *pTarget = NULL;
		float radius =  LEVEL(mod_shockwave_radius, level);
		while ( ( pTarget = gEntList.FindEntityInSphere( pTarget, pOwner->WorldSpaceCenter(), radius ) ) != NULL )
		{
			CBaseCombatCharacter *pOther = pTarget->MyCombatCharacterPointer();

			// only NPCs and players should be affected, and only if they're not the attacking player
			if ( pOther == NULL || !(pOther->IsNPC() || (pOther->IsPlayer() && ToHL2MPPlayer(pOther)->IsInCharacter())) || pOther == pOwner )
				continue;

			// shockwave doesn't affect allies of the attacking player
			if ( HL2MPRules()->IsFriendly(pOwner,pOther) )
				continue;

			if ( !HasTargetInLOS(pOwner,pOther) )
				continue;

			float drain = min(maxDrain,pOther->GetAuxPower());
			pOther->DrainAuxPower(drain);
			
			pOwner->AddAuxPower(drain * LEVEL(mod_shockwave_aux_transfer_ratio, level) );
			CTakeDamageInfo info( pOwner, pOwner, drain * LEVEL(mod_shockwave_damage_ratio, level), (DMG_BURN|DMG_NEVERGIB) );
			info.SetModule(GetModuleIndex());
			pOther->TakeDamage( info );
		}

		return true;
	}

	bool HasTargetInLOS(CBaseCombatCharacter *pStartEnt, CBaseCombatCharacter *pTargetEnt )
	{
		trace_t tr;
		UTIL_TraceLine( pStartEnt->EyePosition(),pTargetEnt->WorldSpaceCenter(), MASK_SOLID, pStartEnt, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction == 1.0f || tr.m_pEnt == pTargetEnt )
			return true;

		return false;
	}
	virtual int GetAIDescription() { return PASSIVE; }
#else
	DATA_TABLE_ITEM(1, L"Max drain", toStr(LEVEL(mod_shockwave_victim_drain, level)))
	DATA_TABLE_ITEM(2, L"Cooldown", toStr(GetCooldown(level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Minion Antlion
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_antlion_drain);
extern LEVEL_EXTERN(mod_antlion_cooldown);
extern LEVEL_EXTERN(mod_antlion_health);
extern LEVEL_EXTERN(mod_antlion_dmg_swipe);
extern LEVEL_EXTERN(mod_antlion_dmg_jump);
DECLARE_MODULE(MinionAntlion, InstantModule, TenLevelModule, minion_antlion)
	virtual const char *DisplayName() { return "Antlion"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_antlion_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_antlion_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_antlion" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
//		pOwner->EnableControl(false); // freeze for the cast duration
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		Vector	vForward;
		pOwner->EyeVectors( &vForward, NULL, NULL );
	
		if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_MINION) )
		{
			ClientPrint(pOwnerPlayer, HUD_PRINTNOTIFY, UTIL_VarArgs("You cannot have more than %i minions at a time.\n", pOwnerPlayer->GetLimitedQuantities()->GetLimit(LQ_MINION)) );
			return false; // too many critters, bugger off
		}

		vForward.z = 0;
		VectorNormalize(vForward);
		Vector traceStart = pOwner->EyePosition() + vForward * 48;
		trace_t tr;

		UTIL_TraceHull( traceStart, traceStart - Vector(0,0,72), 
					NAI_Hull::Mins(HULL_MEDIUM), NAI_Hull::Maxs(HULL_MEDIUM),
					MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction == 0.0f )
			return false;

		QAngle angles = pOwner->EyeAngles();
		angles.x = 0;
		angles.z = 0;

		CAI_BaseNPC *pAntlion = CreateNPC(g_MinionAntlion,tr.endpos,angles,pOwnerPlayer,level);
		if ( pAntlion )
		{
			pAntlion->SetControllingModule(GetModuleIndex());
			pAntlion->m_flNextAttack = gpGlobals->curtime + 0.1f;//GetCastTime(level); // should we create it at StartEffect, so we can set this here?

			if ( pOwnerPlayer != NULL )
			{
				pOwnerPlayer->GetLimitedQuantities()->Add(LQ_MINION);

				if ( pOwnerPlayer->GetLimitedQuantities()->GetCount(LQ_MINION) == 1 )
					SHOW_HINT(pOwnerPlayer, random->RandomInt(1,2) == 1 ? "Hint_Minion1" : "Hint_Minion2")
			}
			return true;
		}
		
		return false;
	}

	virtual void FailEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}
	virtual int GetAIDescription() { return MINION; }
#else
	DATA_TABLE_ITEM(1, L"Minion level", toStr(level))
	DATA_TABLE_ITEM(2, L"Maximum", toStr(mc_player_max_minions.GetInt()))
	DATA_TABLE_ITEM(3, L"Drain", toStr(minion_drain_scale.GetFloat()))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Minion Antlion Worker (Poison)
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_antlionworker_drain);
extern LEVEL_EXTERN(mod_antlionworker_cooldown);
extern LEVEL_EXTERN(mod_antlionworker_health);
extern LEVEL_EXTERN(mod_antlionworker_dmg_burst);
extern LEVEL_EXTERN(mod_antlionworker_dmg_swipe);
extern LEVEL_EXTERN(mod_antlionworker_dmg_jump);
extern LEVEL_EXTERN(mod_antlionworker_dmg_spit);
DECLARE_MODULE(MinionAntlionWorker, InstantModule, TenLevelModule, minion_antlion_worker)
	virtual const char *DisplayName() { return "Antlion worker"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_antlionworker_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_antlionworker_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_antlion_worker" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
//		pOwnerPlayer->EnableControl(false); // freeze for the cast duration
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		Vector	vForward;
		pOwner->EyeVectors( &vForward, NULL, NULL );
	
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pOwner);
		if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_MINION) )
		{
			ClientPrint(pOwnerPlayer, HUD_PRINTNOTIFY, UTIL_VarArgs("You cannot have more than %i minions at a time.\n", pOwnerPlayer->GetLimitedQuantities()->GetLimit(LQ_MINION)) );
			return false; // too many critters, bugger off
		}

		vForward.z = 0;
		VectorNormalize(vForward);
		Vector traceStart = pOwner->EyePosition() + vForward * 48;
		trace_t tr;

		UTIL_TraceHull( traceStart, traceStart - Vector(0,0,72), 
					NAI_Hull::Mins(HULL_MEDIUM), NAI_Hull::Maxs(HULL_MEDIUM),
					MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction == 0.0f )
			return false;

		QAngle angles = pOwner->EyeAngles();
		angles.x = 0;
		angles.z = 0;
		
		CAI_BaseNPC *pAntlionWorker = CreateNPC(g_MinionAntlionWorker,tr.endpos,angles,pOwnerPlayer,level);
		if ( pAntlionWorker )
		{
			pAntlionWorker->SetControllingModule(GetModuleIndex());
			pAntlionWorker->m_flNextAttack = gpGlobals->curtime + 0.1f;//GetCastTime(level); // should we create it at StartEffect, so we can set this here?

			if ( pOwnerPlayer )
			{
				pOwnerPlayer->GetLimitedQuantities()->Add(LQ_MINION);
				if ( pOwnerPlayer->GetLimitedQuantities()->GetCount(LQ_MINION) == 1 )
					SHOW_HINT(pOwnerPlayer, random->RandomInt(1,2) == 1 ? "Hint_Minion1" : "Hint_Minion2")
			}
			return true;
		}

		return false;
	}

	virtual void FailEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}
	virtual int GetAIDescription() { return MINION; }
#else
	DATA_TABLE_ITEM(1, L"Minion level", toStr(level))
	DATA_TABLE_ITEM(2, L"Maximum", toStr(mc_player_max_minions.GetInt()))
	DATA_TABLE_ITEM(3, L"Drain", toStr(minion_drain_scale.GetFloat()))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Minion Fast Headcrab
//-----------------------------------------------------------------------------
/*
extern LEVEL_EXTERN(mod_fastheadcrab_drain);
extern LEVEL_EXTERN(mod_fastheadcrab_cooldown);
DECLARE_MODULE(MinionFastHeadcrab, InstantModule, TenLevelModule, minion_fastheadcrab)
	virtual const char *DisplayName() { return "Fast Headcrab"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_fastheadcrab_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_fastheadcrab_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_headcrab_fast" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
//		pOwner->EnableControl(false); // freeze for the cast duration
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		Vector	vForward;
		pOwner->EyeVectors( &vForward, NULL, NULL );
	
		if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_MINION) )
		{
			ClientPrint(pOwnerPlayer, HUD_PRINTNOTIFY, UTIL_VarArgs("You cannot have more than %i minions at a time.\n", pOwnerPlayer->GetLimitedQuantities()->GetLimit(LQ_MINION)) );
			return false; // too many critters, bugger off
		}

		vForward.z = 0;
		VectorNormalize(vForward);
		Vector traceStart = pOwner->EyePosition() + vForward * 48;
		trace_t tr;

		UTIL_TraceHull( traceStart, traceStart - Vector(0,0,72), 
					NAI_Hull::Mins(HULL_TINY), NAI_Hull::Maxs(HULL_TINY),
					MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction == 0.0f )
			return false;

		QAngle angles = pOwner->EyeAngles();
		angles.x = 0;
		angles.z = 0;

		CAI_BaseNPC *pFastHeadCrab = CreateNPC(g_MinionFastHeadcrab,tr.endpos,angles,pOwnerPlayer,level);
		if ( pFastHeadCrab )
		{
			pFastHeadCrab->SetControllingModule(GetModuleIndex());
			pFastHeadCrab->m_flNextAttack = gpGlobals->curtime + 0.1f;//GetCastTime(level); // should we create it at StartEffect, so we can set this here?

			if ( pOwnerPlayer != NULL )
			{
				pOwnerPlayer->GetLimitedQuantities()->Add(LQ_MINION);

				if ( pOwnerPlayer->GetLimitedQuantities()->GetCount(LQ_MINION) == 1 )
					SHOW_HINT(pOwnerPlayer, random->RandomInt(1,2) == 1 ? "Hint_Minion1" : "Hint_Minion2")
			}
			return true;
		}

//		ClientPrint(pOwner,HUD_PRINTNOTIFY,UTIL_VarArgs("Doing %s effect\n",DisplayName()));
		return false;
	}

	virtual void FailEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}
	virtual int GetAIDescription() { return MINION; }
#else
	DATA_TABLE_ITEM(1, L"Minion level", toStr(level))
	DATA_TABLE_ITEM(2, L"Maximum", toStr(mc_player_max_minions.GetInt()))
	DATA_TABLE_ITEM(3, L"Drain", toStr(minion_drain_scale.GetFloat()))
#endif
END_MODULE()
*/
//-----------------------------------------------------------------------------
// Purpose: Minion Fast Zombie
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_fastzombie_drain);
extern LEVEL_EXTERN(mod_fastzombie_cooldown);
extern LEVEL_EXTERN(mod_fastzombie_health);
extern LEVEL_EXTERN(mod_fastzombie_dmg_leap);
extern LEVEL_EXTERN(mod_fastzombie_dmg_claw);
DECLARE_MODULE(MinionFastZombie, InstantModule, TenLevelModule, minion_fastzombie)
	virtual const char *DisplayName() { return "Fast Zombie"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_fastzombie_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_fastzombie_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_fastzombie" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
//		pOwner->EnableControl(false); // freeze for the cast duration
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		Vector	vForward;
		pOwner->EyeVectors( &vForward, NULL, NULL );
	
		if ( pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_MINION) )
		{
			ClientPrint(pOwnerPlayer, HUD_PRINTNOTIFY, UTIL_VarArgs("You cannot have more than %i minions at a time.\n", pOwnerPlayer->GetLimitedQuantities()->GetLimit(LQ_MINION)) );
			return false; // too many critters, bugger off
		}

		vForward.z = 0;
		VectorNormalize(vForward);
		Vector traceStart = pOwner->EyePosition() + vForward * 48;
		trace_t tr;

		UTIL_TraceHull( traceStart, traceStart - Vector(0,0,72), 
					NAI_Hull::Mins(HULL_MEDIUM), NAI_Hull::Maxs(HULL_MEDIUM),
					MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction == 0.0f )
			return false;

		QAngle angles = pOwner->EyeAngles();
		angles.x = 0;
		angles.z = 0;

		CAI_BaseNPC *pFastZombie = CreateNPC(g_MinionFastZombie,tr.endpos,angles,pOwnerPlayer,level);
		if ( pFastZombie )
		{
			pFastZombie->SetControllingModule(GetModuleIndex());
			pFastZombie->m_flNextAttack = gpGlobals->curtime + 0.1f;//GetCastTime(level); // should we create it at StartEffect, so we can set this here?

			if ( pOwnerPlayer != NULL )
			{
				pOwnerPlayer->GetLimitedQuantities()->Add(LQ_MINION);

				if ( pOwnerPlayer->GetLimitedQuantities()->GetCount(LQ_MINION) == 1 )
					SHOW_HINT(pOwnerPlayer, random->RandomInt(1,2) == 1 ? "Hint_Minion1" : "Hint_Minion2")
			}
			return true;
		}

//		ClientPrint(pOwner,HUD_PRINTNOTIFY,UTIL_VarArgs("Doing %s effect\n",DisplayName()));
		return false;
	}

	virtual void FailEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}
	virtual int GetAIDescription() { return MINION; }
#else
	DATA_TABLE_ITEM(1, L"Minion level", toStr(level))
	DATA_TABLE_ITEM(2, L"Maximum", toStr(mc_player_max_minions.GetInt()))
	DATA_TABLE_ITEM(3, L"Drain", toStr(minion_drain_scale.GetFloat()))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Minion Zombie
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_zombie_drain);
extern LEVEL_EXTERN(mod_zombie_cooldown);
extern LEVEL_EXTERN(mod_zombie_health);
extern LEVEL_EXTERN(mod_zombie_dmg_one);
extern LEVEL_EXTERN(mod_zombie_dmg_both);
DECLARE_MODULE(MinionZombie, InstantModule, TenLevelModule, minion_zombie)
	virtual const char *DisplayName() { return "Zombie"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_zombie_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_zombie_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_zombie" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
//		pOwner->EnableControl(false); // freeze for the cast duration
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		Vector	vForward;
		pOwner->EyeVectors( &vForward, NULL, NULL );
	
		if ( pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_MINION) )
		{
			ClientPrint(pOwnerPlayer, HUD_PRINTNOTIFY, UTIL_VarArgs("You cannot have more than %i minions at a time.\n", pOwnerPlayer->GetLimitedQuantities()->GetLimit(LQ_MINION)) );
			return false; // too many critters, bugger off
		}

		vForward.z = 0;
		VectorNormalize(vForward);
		Vector traceStart = pOwner->EyePosition() + vForward * 48;
		trace_t tr;

		UTIL_TraceHull( traceStart, traceStart - Vector(0,0,72), 
					NAI_Hull::Mins(HULL_MEDIUM), NAI_Hull::Maxs(HULL_MEDIUM),
					MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction == 0.0f )
			return false;

		QAngle angles = pOwner->EyeAngles();
		angles.x = 0;
		angles.z = 0;

		CAI_BaseNPC *pZombie = CreateNPC(g_MinionZombie,tr.endpos,angles,pOwnerPlayer,level);
		if ( pZombie )
		{
			pZombie->SetControllingModule(GetModuleIndex());
			pZombie->m_flNextAttack = gpGlobals->curtime + 0.1f;//GetCastTime(level); // should we create it at StartEffect, so we can set this here?

			if ( pOwnerPlayer != NULL )
			{
				pOwnerPlayer->GetLimitedQuantities()->Add(LQ_MINION);

				if ( pOwnerPlayer->GetLimitedQuantities()->GetCount(LQ_MINION) == 1 )
					SHOW_HINT(pOwnerPlayer, random->RandomInt(1,2) == 1 ? "Hint_Minion1" : "Hint_Minion2")
			}
			return true;
		}

//		ClientPrint(pOwner,HUD_PRINTNOTIFY,UTIL_VarArgs("Doing %s effect\n",DisplayName()));
		return false;
	}

	virtual void FailEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}
	virtual int GetAIDescription() { return MINION; }
#else
	DATA_TABLE_ITEM(1, L"Minion level", toStr(level))
	DATA_TABLE_ITEM(2, L"Maximum", toStr(mc_player_max_minions.GetInt()))
	DATA_TABLE_ITEM(3, L"Drain", toStr(minion_drain_scale.GetFloat()))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Minion Vortigaunt
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_vortigaunt_drain);
extern LEVEL_EXTERN(mod_vortigaunt_cooldown);
extern LEVEL_EXTERN(mod_vortigaunt_health);
extern LEVEL_EXTERN(mod_vortigaunt_dmg_claw);
extern LEVEL_EXTERN(mod_vortigaunt_dmg_rake);
extern LEVEL_EXTERN(mod_vortigaunt_dmg_zap);
extern LEVEL_EXTERN(mod_vortigaunt_zap_spread);
DECLARE_MODULE(MinionVortigaunt, InstantModule, TenLevelModule, minion_vortigaunt)
	virtual const char *DisplayName() { return "Vortigaunt"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_vortigaunt_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_vortigaunt_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_vortigaunt" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
//		pOwner->EnableControl(false); // freeze for the cast duration
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		Vector	vForward;
		pOwner->EyeVectors( &vForward, NULL, NULL );
	
		if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_MINION,2) )
		{
			ClientPrint(pOwnerPlayer, HUD_PRINTNOTIFY, UTIL_VarArgs("You cannot have more than %i minions at a time.\n", pOwnerPlayer->GetLimitedQuantities()->GetLimit(LQ_MINION)) );
			return false; // too many critters, bugger off
		}

		vForward.z = 0;
		VectorNormalize(vForward);
		Vector traceStart = pOwner->EyePosition() + vForward * 48;
		trace_t tr;

		UTIL_TraceHull( traceStart, traceStart - Vector(0,0,72), 
					NAI_Hull::Mins(HULL_MEDIUM), NAI_Hull::Maxs(HULL_MEDIUM),
					MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction == 0.0f )
			return false;

		QAngle angles = pOwner->EyeAngles();
		angles.x = 0;
		angles.z = 0;

		CAI_BaseNPC *pVortigaunt = CreateNPC(g_MinionVortigaunt,tr.endpos,angles,pOwnerPlayer,level);
		if ( pVortigaunt )
		{
			pVortigaunt->SetControllingModule(GetModuleIndex());
			pVortigaunt->m_flNextAttack = gpGlobals->curtime + 0.1f;//GetCastTime(level); // should we create it at StartEffect, so we can set this here?

			if ( pOwnerPlayer != NULL )
			{
				pOwnerPlayer->GetLimitedQuantities()->Add(LQ_MINION,2);

				if ( pOwnerPlayer->GetLimitedQuantities()->GetCount(LQ_MINION) == 1 )
					SHOW_HINT(pOwnerPlayer, random->RandomInt(1,2) == 1 ? "Hint_Minion1" : "Hint_Minion2")
			}
			return true;
		}

		return false;
	}

	virtual void FailEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}
	virtual int GetAIDescription() { return MINION; }
#else
	DATA_TABLE_ITEM(1, L"Minion level", toStr(level))
	DATA_TABLE_ITEM(2, L"Maximum", toStr(mc_player_max_minions.GetInt()))
	DATA_TABLE_ITEM(3, L"Drain", toStr(minion_drain_scale.GetFloat()))
	DATA_TABLE_ITEM(4, L"Vort Drain", toStr(minion_drain_scale.GetFloat()*2))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Minion Manhack
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_manhack_drain);
extern LEVEL_EXTERN(mod_manhack_cooldown);
extern LEVEL_EXTERN(mod_manhack_limit);
extern LEVEL_EXTERN(mod_manhack_health);
extern LEVEL_EXTERN(mod_manhack_dmg_slash);
extern LEVEL_EXTERN(mod_manhack_dmg_held);
extern LEVEL_EXTERN(mod_manhack_engine_power);
#ifndef CLIENT_DLL
extern McConVar mod_manhack_engine_power_max;
#endif
DECLARE_MODULE(MinionManhack, InstantModule, TenLevelModule, minion_manhack)
	virtual const char *DisplayName() { return "Manhack"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_manhack_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_manhack_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_manhack" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
//		pOwnerPlayer->EnableControl(false); // freeze for the cast duration
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		Vector	vForward, vRight;
		pOwner->EyeVectors( &vForward, &vRight, NULL );
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pOwner);

		for ( int i=0; i<2; i++ ) // spawn 2 at a time
		{
			if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_MANHACK) )
			{
				ClientPrint(pOwnerPlayer, HUD_PRINTNOTIFY, UTIL_VarArgs("You cannot have more than %i manhacks at a time.\n", pOwnerPlayer->GetLimitedQuantities()->GetLimit(LQ_MANHACK)) );
				return i>0; // too many critters, bugger off	<- "hahahaha, that was beautiful... (Josh)"
			}

			vForward.z = 0;
			VectorNormalize(vForward);
			Vector traceStart = pOwner->EyePosition() + vForward * 48;
			trace_t tr;

			float sideOffset = i == 0 ? -24 : 24;
			UTIL_TraceHull( traceStart, traceStart + vRight * sideOffset, 
						NAI_Hull::Mins(HULL_TINY_CENTERED), NAI_Hull::Maxs(HULL_TINY_CENTERED),
						MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );
			if ( tr.fraction == 0.0f )
				return i>0;

			QAngle angles = pOwner->EyeAngles();
			angles.x = 0;
			angles.z = 0;

			CAI_BaseNPC *pManHack = CreateNPC(g_MinionManhack,tr.endpos,angles,pOwnerPlayer,level);
			if ( pManHack )
			{
				pManHack->RemoveBuff(BUFF_MINION);
				pManHack->SetControllingModule(GetModuleIndex());
				pManHack->m_flNextAttack = gpGlobals->curtime + 0.1f;//GetCastTime(level); // should we create it at StartEffect, so we can set this here?

				if ( pOwnerPlayer )
					pOwnerPlayer->GetLimitedQuantities()->Add(LQ_MANHACK);
			}
			else
				return i>0;
		}

		return true;
	}

	virtual void FailEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}
	virtual int GetAIDescription() { return MINION; }
#else
	DATA_TABLE_ITEM(1, L"Minion level", toStr(level))
	DATA_TABLE_ITEM(2, L"Maximum", toStr((int)LEVEL(mod_manhack_limit, level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Vitality
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_vitality);
extern McConVar mc_player_base_health;
DECLARE_MODULE(Vitality, PassiveModule, TenLevelModule, vitality)
	virtual const char *DisplayName() { return "Vitality"; }

#ifndef CLIENT_DLL
	//virtual bool ShouldTurnOffBeforeUpgrading() { return false; }
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
	    float fraction = (float)pOwner->GetHealth() / (float)pOwner->GetMaxHealth();
		pOwner->SetMaxHealth( pOwner->GetBaseMaxHealth() + LEVEL(mod_vitality, level) - LEVEL(mod_vitality, 0) );
		int newHealth = pOwner->GetMaxHealth() * fraction;
		pOwner->TakeHealth( newHealth - pOwner->GetHealth(), DMG_GENERIC );
	}
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)
	{
		float fraction = (float)pOwner->GetHealth() / (float)pOwner->GetMaxHealth();
		pOwner->SetMaxHealth( pOwner->GetBaseMaxHealth() );
		pOwner->SetHealth(pOwner->GetMaxHealth() * fraction);
	}
	virtual int GetAIDescription() { return PASSIVE; }
#else
	DATA_TABLE_ITEM(1, L"Health", toStr(mc_player_base_health.GetInt() + (int)LEVEL(mod_vitality, level)))
	virtual bool ShouldShowLevel0OnDataTables() { return true; }
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Armor Capacity
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_armorcap);
extern McConVar mc_player_base_armor_capacity;
DECLARE_MODULE(ArmorCapacity, PassiveModule, TenLevelModule, armorcap)
	virtual const char *DisplayName() { return "Armor Capacity"; }

#ifndef CLIENT_DLL
	virtual bool ShouldTurnOffBeforeUpgrading() { return false; }
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pOwner);
		if ( pOwnerPlayer )
			pOwnerPlayer->SetMaxArmor( mc_player_base_armor_capacity.GetInt() + LEVEL(mod_armorcap, level) );
	}
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pOwner);
		if ( pOwnerPlayer )
			pOwnerPlayer->SetMaxArmor( mc_player_base_armor_capacity.GetInt() );
	}
#else
	DATA_TABLE_ITEM(1, L"Armor", toStr(mc_player_base_armor_capacity.GetInt() + (int)LEVEL(mod_armorcap, level)))
	virtual bool ShouldShowLevel0OnDataTables() { return true; }
#endif
END_MODULE()


//-----------------------------------------------------------------------------
// Purpose: Aux Power Tank
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_auxpower);
extern McConVar mc_player_base_aux_capacity;
DECLARE_MODULE(AuxPowerTank, PassiveModule, TenLevelModule, auxpower)
	virtual const char *DisplayName() { return "Aux Power Tank"; }

#ifndef CLIENT_DLL
	virtual bool ShouldTurnOffBeforeUpgrading() { return true; }
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pOwner);
		if ( pOwnerPlayer )
			pOwnerPlayer->SetMaxAuxPower( mc_player_base_aux_capacity.GetInt() + (int)LEVEL(mod_auxpower, level) );
	}
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pOwner);
		if ( pOwnerPlayer )
			pOwnerPlayer->SetMaxAuxPower( mc_player_base_aux_capacity.GetInt() );
	}
#else
	DATA_TABLE_ITEM(1, L"Aux power", toStr( mc_player_base_aux_capacity.GetInt() + (int)LEVEL(mod_auxpower, level) ))
	virtual bool ShouldShowLevel0OnDataTables() { return true; }
#endif
END_MODULE()



//-----------------------------------------------------------------------------
// Purpose: Adrenaline
//-----------------------------------------------------------------------------

extern LEVEL_EXTERN(mod_adrenaline);
extern McConVar mc_player_base_walk_speed;
DECLARE_MODULE(Adrenaline, PassiveModule, TenLevelModule, adrenaline)
	virtual const char *DisplayName() { return "Adrenaline"; }
	//virtual int GetMaxLevel() { return 6; } // ONLY 6 levels of this!!

#ifndef CLIENT_DLL
	// Please see the buffs for this module
	virtual bool ShouldTurnOffBeforeUpgrading() { return false; }
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level){}
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level){}
	virtual int GetAIDescription() { return PASSIVE; } // can we make them faster?
#else
	DATA_TABLE_ITEM(1, L"Speed", toStr( mc_player_base_walk_speed.GetInt() + (int)LEVEL(mod_adrenaline, level)))
	virtual bool ShouldShowLevel0OnDataTables() { return true; }
#endif
END_MODULE()


//-----------------------------------------------------------------------------
// Purpose: Magazine size
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_clipsize_pistol);
extern LEVEL_EXTERN(mod_clipsize_357);
extern LEVEL_EXTERN(mod_clipsize_smg);
extern LEVEL_EXTERN(mod_clipsize_ar2);
extern LEVEL_EXTERN(mod_clipsize_shotgun);
extern LEVEL_EXTERN(mod_clipsize_crossbow);
extern LEVEL_EXTERN(mod_clipsize_rpg);
extern McConVar weapon_pistol_clip_size, weapon_357_clip_size, weapon_smg_clip_size, weapon_ar2_clip_size, weapon_shotgun_clip_size, weapon_crossbow_clip_size, weapon_rpg_clip_size;
DECLARE_MODULE(ClipSize, PassiveModule, TenLevelModule, clipsize)
	virtual const char *DisplayName() { return "Clip Size"; }

#ifndef CLIENT_DLL
	virtual bool ShouldTurnOffBeforeUpgrading() { return false; }
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level) {}
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level) {}
#else
	virtual bool ShouldShowLevel0OnDataTables() { return true; }
	
	DATA_TABLE_ITEM(1, L"Pistol", toStr(weapon_pistol_clip_size.GetInt() + (int)LEVEL(mod_clipsize_pistol, level)))
	DATA_TABLE_ITEM(2, L"357", toStr(weapon_357_clip_size.GetInt() + (int)LEVEL(mod_clipsize_357, level)))
	DATA_TABLE_ITEM(3, L"SMG", toStr(weapon_smg_clip_size.GetInt() + (int)LEVEL(mod_clipsize_smg, level)))
	DATA_TABLE_ITEM(4, L"AR2", toStr(weapon_ar2_clip_size.GetInt() + (int)LEVEL(mod_clipsize_ar2, level)))
	DATA_TABLE_ITEM(5, L"Shotgun", toStr(weapon_shotgun_clip_size.GetInt() + (int)LEVEL(mod_clipsize_shotgun, level)))
	DATA_TABLE_ITEM(6, L"Crossbow", toStr(weapon_crossbow_clip_size.GetInt() + (int)LEVEL(mod_clipsize_crossbow, level)))
	DATA_TABLE_ITEM(7, L"RPG", toStr(weapon_rpg_clip_size.GetInt() + (int)LEVEL(mod_clipsize_rpg, level)))
#endif
END_MODULE()


//-----------------------------------------------------------------------------
// Purpose: Barnacle
//-----------------------------------------------------------------------------
/*
extern LEVEL_EXTERN(mod_barnacle_drain);
extern LEVEL_EXTERN(mod_barnacle_cooldown);
DECLARE_MODULE(Barnacle, InstantModule, TenLevelModule, barnacle)
	virtual const char *DisplayName() { return "Barnacle"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_barnacle_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_barnacle_cooldown, level); }
	
	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_barnacle" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_BARNACLE) )
		{
			Msg( "You cannot have more than %i barnacles out at a time.\n", pOwnerPlayer->GetLimitedQuantities()->GetLimit(LQ_BARNACLE) );
			return false; // too many critters, bugger off
		}

		Vector	vForward;
		pOwner->EyeVectors( &vForward, NULL, NULL );
	
		Vector traceStart = pOwner->EyePosition();
		Vector traceEnd = pOwner->EyePosition() + vForward * 2048;
		trace_t tr;

		UTIL_TraceLine( traceStart, traceEnd, MASK_NPCSOLID, pOwner, COLLISION_GROUP_NONE, &tr );

		if ( tr.plane.normal.z > -0.6 )
		{
			return false;
		}

		UTIL_TraceHull( tr.endpos, tr.endpos,
						NAI_Hull::Mins(HULL_MEDIUM), NAI_Hull::Maxs(HULL_MEDIUM),
						MASK_NPCSOLID, NULL, COLLISION_GROUP_NPC, &tr );

		if ( !tr.DidHit() || ( tr.surface.flags & SURF_SKY ) )
			return false;

		QAngle angles = pOwner->EyeAngles();
		angles.x = 0;
		angles.z = 0;

		CAI_BaseNPC *pBarnacle = CreateNPC(g_MinionBarnacle,tr.endpos,angles,NULL,level);
		if ( pBarnacle )
		{
			pBarnacle->SetMasterPlayer(pOwnerPlayer);
			pBarnacle->SetLikesMaster(true);
			pBarnacle->SetControllingModule(GetModuleIndex());
			pBarnacle->m_flNextAttack = gpGlobals->curtime + 0.1f;//GetCastTime(level); // should we create it at StartEffect, so we can set this here?
			pBarnacle->RemoveBuff(BUFF_MINION); // hack to stop this healing or affecting minion limit on death

			if ( pOwnerPlayer )
				pOwnerPlayer->GetLimitedQuantities()->Add(LQ_BARNACLE);
		}

//		ClientPrint(pOwner,HUD_PRINTNOTIFY,UTIL_VarArgs("Doing %s effect\n",DisplayName()));
		return true;
	}

	virtual void FailEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}
#endif
END_MODULE()
*/
//-----------------------------------------------------------------------------
// Purpose: Starting Armor
//----------------------------------------------------------------------------
/*
extern LEVEL_EXTERN(mod_startarmor);
DECLARE_MODULE(StartArmor, PassiveModule, TenLevelModule, startarmor)
    virtual const char *DisplayName() { return "Start Armor"; }

#ifndef CLIENT_DLL
    virtual bool ShouldTurnOffBeforeUpgrading() { return false; }
    virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
    {
        CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
        if ( !pOwnerPlayer )
            return;
        pOwnerPlayer->SetStartArmor( LEVEL(mod_startarmor, level) );
    }
    virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)
    {
        CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
        if ( pOwnerPlayer )
            pOwnerPlayer->SetStartArmor( 0 );
    }
#else
	DATA_TABLE_ITEM(1, L"Armor", toStr((int)LEVEL(mod_startarmor, level)))
	DATA_TABLE_ITEM(2, L"Capacity", toStr((int)LEVEL(mod_armorcap, C_HL2MP_Player::GetLocalHL2MPPlayer()->GetModuleLevel(GetModule(ARMOR_CAPACITY)))))
#endif
END_MODULE()
*/
//-----------------------------------------------------------------------------
// Purpose: Energy Ball
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_energyball_drain);
extern LEVEL_EXTERN(mod_energyball_cooldown);
extern LEVEL_EXTERN(mod_energyball_speed);
extern LEVEL_EXTERN(mod_energyball_damage);
extern LEVEL_EXTERN(mod_energyball_lifetime);

DECLARE_MODULE(EnergyBall, InstantModule, TenLevelModule, energyball)
	virtual const char *DisplayName() { return "Energy Ball"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_energyball_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_energyball_cooldown, level); }

	int MaxSpeed( int level ) { return (int)LEVEL(mod_energyball_speed, level); }

	virtual const char *UseSound() { return "Module.EnergyBall"; }
#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));

		Vector vecSrc	 = pOwner->Weapon_ShootPosition( );
		Vector forward;
		pOwner->EyeVectors(&forward,NULL,NULL);
		Vector vecVelocity = forward * MaxSpeed(level);
		
		// Fire the combine ball
		CreateCombineBall(	vecSrc,			// Source
							vecVelocity,	// Speed
							50,				// Radius
							50,				// Mass
							(int)LEVEL(mod_energyball_lifetime, level),	// Lifetime (seconds)
							pOwner, // Who is it NOT going to hurt?
							true,
							0, // bounces
							(int)LEVEL(mod_energyball_damage, level) );

		// View effects
		color32 white = {255, 255, 255, 64};
		UTIL_ScreenFade( pOwner, white, 0.1, 0, FFADE_IN  );
	return true;
	}
	virtual int GetAIDescription() { return PROJECTILE_DIRECT; }
#else
	DATA_TABLE_ITEM(1, L"Damage", toStr( (int)LEVEL(mod_energyball_damage, level) ))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Flechette
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_flechette_drain);
extern LEVEL_EXTERN(mod_flechette_refire);
extern LEVEL_EXTERN(mod_flechette_cooldown);
extern LEVEL_EXTERN(mod_flechette_duration);
extern LEVEL_EXTERN(mod_flechette_dmg_hit);
extern LEVEL_EXTERN(mod_flechette_dmg_blast);
extern ConVar hunter_flechette_speed;
DECLARE_MODULE(Flechette, MaintainedRepeatingModule, TenLevelModule, flechette)
	virtual const char *DisplayName() { return "Flechette"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_flechette_drain, level); }
	virtual float GetTickInterval(int level) { return LEVEL(mod_flechette_refire, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_flechette_cooldown, level); }
	virtual float GetMaxDuration(int level) { return LEVEL(mod_flechette_duration, level); }

	virtual const char *TickSound() { return "NPC_Hunter.FlechetteShoot"; }
#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		Vector vecSrc = pOwner->Weapon_ShootPosition();
		if ( pOwner->IsPlayer() )
			vecSrc += Vector(0,0,-8);
		Vector forward;
		pOwner->EyeVectors(&forward,NULL,NULL);

		CShotManipulator manipulator( forward );
		Vector vecShoot = manipulator.ApplySpread( VECTOR_CONE_4DEGREES, 1.0f );
		QAngle angShoot;
		VectorAngles( vecShoot, angShoot );

		CHunterFlechette *pFlechette = CHunterFlechette::FlechetteCreate( vecSrc, angShoot, pOwner );
		pFlechette->SetControllingModule(GetModuleIndex());
		pFlechette->SetDamage( LEVEL(mod_flechette_dmg_blast, level), LEVEL(mod_flechette_dmg_hit, level) );
		pFlechette->SetPlayerOwned(true); // marks this flechette as being able to hurt hunters. They're immune, otherwise.
		pFlechette->AddEffects( EF_NOSHADOW );
		vecShoot *= hunter_flechette_speed.GetFloat();
		pFlechette->Shoot( vecShoot, false );

		return true;
	}
	virtual int GetAIDescription() { return PROJECTILE_DIRECT | MAINTAINED_EFFECT; }
#else
	DATA_TABLE_ITEM(1, L"Impact dmg", toStr(LEVEL(mod_flechette_dmg_hit, level)))
	DATA_TABLE_ITEM(2, L"Explosion dmg", toStr(LEVEL(mod_flechette_dmg_blast, level)))
	DATA_TABLE_ITEM(3, L"Shots", toStr((int)(LEVEL(mod_flechette_duration, level) / LEVEL(mod_flechette_refire, level) + 0.5f)))
	DATA_TABLE_ITEM(4, L"Duration", toStr(LEVEL(mod_flechette_duration, level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Cloak
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_cloak_drain);
extern LEVEL_EXTERN(mod_cloak_cooldown);
extern LEVEL_EXTERN(mod_cloak_movement_drain);
DECLARE_MODULE(Cloak, ToggledRepeatingModule, SingleLevelModule, cloak)
	virtual const char *DisplayName() { return "Cloak"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level)
	{
		// drain extra power, based on their velocity. this is in addition to the 5 per second base use drain
		// clamp the max vertical speed used, so as to not let excessive vertical velocities screw you over too badly
		#define VMAX	160.0f
		Vector velocity = pOwner->GetAbsVelocity();
		velocity.z = max(min(pOwner->GetAbsVelocity().z,VMAX),-VMAX);
		return LEVEL(mod_cloak_drain, level) + velocity.LengthSqr() * LEVEL(mod_cloak_movement_drain, level) * 0.000001f;
	}

	virtual bool UsesSmoothAuxDrain() { return true; } // causes aux drain to be "per second"

	virtual float GetTickInterval(int level) { return 1.0f; }
	virtual float GetCooldown(int level) { return LEVEL(mod_cloak_cooldown, level); } // can't use for 2s after stopping

	virtual bool IsSuppressed(CBaseCombatCharacter *pOwner)
	{
		if ( pOwner->IsBuffActive(DEBUFF_SUPPRESS_CLOAK) )
			return true;
		return Module::IsSuppressed(pOwner);
	}

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
		pOwner->EnableCloaking(true);
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		return true;
	}

	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)
	{
		pOwner->EnableCloaking(false);
	}
	virtual int GetAIDescription() { return PASSIVE; }		// don't want them thinking about cloak. just stay cloaked except when you attack stuff.
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Recharge
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_recharge_interval);
extern LEVEL_EXTERN(mod_recharge_amount);
extern McConVar mc_player_base_aux_recharge;
DECLARE_MODULE(Recharge, PassiveRepeatingModule, TenLevelModule, recharge)
	virtual const char *DisplayName() { return "Recharge"; }

	virtual float GetTickInterval(int level) { return LEVEL(mod_recharge_interval, level); }
	
	// had to split so the description could get at this
	int rechargePerTick(int level)
	{
		return LEVEL(mod_recharge_amount, level);
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		if ( pOwnerPlayer )
			pOwnerPlayer->AddAuxPower(rechargePerTick(level));
		return true;
	}
	virtual int GetAIDescription() { return PASSIVE; }
#else
	DATA_TABLE_ITEM(1, L"Aux / sec", toStr(mc_player_base_aux_recharge.GetFloat()+rechargePerTick(level) * LEVEL(mod_recharge_interval, level)))
	virtual bool ShouldShowLevel0OnDataTables() { return true; }
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Lasers
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_lasers_drain);
extern LEVEL_EXTERN(mod_lasers_cooldown);
extern LEVEL_EXTERN(mod_lasers_damage);
extern LEVEL_EXTERN(mod_lasers_max_damage);
extern LEVEL_EXTERN(mod_lasers_lifetime);
extern LEVEL_EXTERN(mod_lasers_limit);
DECLARE_MODULE(Lasers, InstantModule, TenLevelModule, laser)
	virtual const char *DisplayName() { return "Lasers"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_lasers_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_lasers_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_tripmine" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		Vector vecSrc, vecAiming;

		// Take the eye position and direction
		vecSrc = pOwner->EyePosition();
		
		QAngle angles = pOwner->EyeAngles();

		AngleVectors( angles, &vecAiming );

		trace_t tr;

		UTIL_TraceLine( vecSrc, vecSrc + (vecAiming * 128), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
		
		if (tr.fraction < 1.0)
		{
			CBaseEntity *pEntity = tr.m_pEnt;
			if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR) && (!pOwnerPlayer || !pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_LASER)) )
			{
				QAngle angles;
				VectorAngles(tr.plane.normal, angles);

				angles.x += 90;

				CBaseEntity *pEnt = CBaseEntity::CreateNoSpawn( "npc_tripmine", tr.endpos + tr.plane.normal * 3, angles, NULL );

				CTripmineGrenade *pMine = (CTripmineGrenade *)pEnt;
				pMine->SetControllingModule(GetModuleIndex());
				pMine->m_hOwner = pOwner;
				pMine->SetLaserDamage( LEVEL(mod_lasers_damage, level) );
				pMine->SetMaxLaserDamage( LEVEL(mod_lasers_max_damage, level) );
				pMine->SetModuleLaunched();
				pMine->SetLifetime(LEVEL(mod_lasers_lifetime, level));
				if ( pOwnerPlayer )
					pOwnerPlayer->GetLimitedQuantities()->Add(LQ_LASER);
				DispatchSpawn( pMine );

				return true;
			}
		}
		return false;
	}
	virtual int GetAIDescription() { return DEPLOYABLE; }
#else
	DATA_TABLE_ITEM(1, L"Damage / sec", toStr((int)LEVEL(mod_lasers_damage, level)*20))
	DATA_TABLE_ITEM(2, L"Duration", toStr(LEVEL(mod_lasers_lifetime, level)))
	DATA_TABLE_ITEM(3, L"Max damage", toStr((int)LEVEL(mod_lasers_max_damage, level)))
	DATA_TABLE_ITEM(4, L"Max lasers", toStr((int)LEVEL(mod_lasers_limit, level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Turrets
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_turret_drain);
extern LEVEL_EXTERN(mod_turret_cooldown);
extern LEVEL_EXTERN(mod_turret_limit);
extern LEVEL_EXTERN(mod_turret_health);
extern LEVEL_EXTERN(mod_turret_dmg_shoot);
DECLARE_MODULE(Turret, InstantModule, TenLevelModule, turret)
	virtual const char *DisplayName() { return "Turrets"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_turret_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_turret_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "npc_turret_floor" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		Vector	vForward;
		pOwner->EyeVectors( &vForward, NULL, NULL );

		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_TURRET) )
		{
			ClientPrint(pOwnerPlayer, HUD_PRINTNOTIFY, UTIL_VarArgs("You cannot have more than %i turrets at a time.\n", pOwnerPlayer->GetLimitedQuantities()->GetLimit(LQ_TURRET)) );
			return false; // too many turrets, bugger off
		}

		vForward.z = 0;
		VectorNormalize(vForward);
		Vector traceStart = pOwner->EyePosition() + vForward * 48;
		trace_t tr;

		UTIL_TraceHull( traceStart, traceStart - Vector(0,0,72), 
					NAI_Hull::Mins(HULL_MEDIUM), NAI_Hull::Maxs(HULL_MEDIUM),
					MASK_NPCSOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction == 0.0f )
			return false;

		QAngle angles = pOwner->EyeAngles();
		angles.x = 0;
		angles.z = 0;

		CNPC_FloorTurret *pTurret = static_cast<CNPC_FloorTurret*>(CBaseEntity::CreateNoSpawn(g_MinionTurret->m_szClassname,tr.endpos,angles));
		
		if ( !pTurret )
		{
			Warning( "Can't make turret!\n");
			return false;
		}

		pTurret->SetMasterPlayer( pOwnerPlayer ); // need to attribute their damage
		pTurret->AddSpawnFlags( SF_NPC_FADE_CORPSE );
		pTurret->SetFaction(pOwnerPlayer ? pOwnerPlayer->GetFaction() : FACTION_COMBINE);
		pTurret->SetStats(g_MinionTurret,level); // calls DispatchSpawn

		pTurret->ApplyBuff(BUFF_SPAWN); // fade in with fancy effects

		pTurret->GetMotor()->SetIdealYaw( angles.y );
	
		pTurret->SetActivity( ACT_IDLE );
		pTurret->SetNextThink( gpGlobals->curtime );
		pTurret->PhysicsSimulate();

		pTurret->m_flNextAttack = gpGlobals->curtime + 0.15f;
		pTurret->Activate();

		pTurret->RemoveBuff(BUFF_MINION); // don't affect minion limit

		if ( pOwnerPlayer )
		{
			pTurret->SetMasterPlayer(pOwnerPlayer);
			pTurret->SetLikesMaster(true);
		}
		else
			pTurret->SetLikesMaster(false);
		pTurret->SetControllingModule(GetModuleIndex());
		pTurret->m_flNextAttack = gpGlobals->curtime + 0.1f;

		pOwnerPlayer->GetLimitedQuantities()->Add(LQ_TURRET);
		return true;
	}
	virtual int GetAIDescription() { return DEPLOYABLE; }
#else
	DATA_TABLE_ITEM(1, L"Health", toStr(5*level+20))
	DATA_TABLE_ITEM(2, L"Damage", toStr(level+4))
	DATA_TABLE_ITEM(3, L"Max", toStr((int)LEVEL(mod_turret_limit, level)))
#endif
END_MODULE()
/*
//-----------------------------------------------------------------------------
// Purpose: Repellent mines
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_repmine_drain);
extern LEVEL_EXTERN(mod_repmine_cooldown);
extern LEVEL_EXTERN(mod_repmine_damage);
extern LEVEL_EXTERN(mod_repmine_health);
extern LEVEL_EXTERN(mod_repmine_strength);
extern LEVEL_EXTERN(mod_repmine_fullradius);
extern LEVEL_EXTERN(mod_repmine_damage_radius);
extern McConVar mod_repmine_npc_scale, mod_repmine_allow_upward_force;
extern LEVEL_EXTERN(mod_repmine_limit);
DECLARE_MODULE(Repmine, InstantModule, TenLevelModule, repmine)
	virtual const char *DisplayName() { return "Rep Mine"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_repmine_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_repmine_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "grenade_repmine" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->GetCount(LQ_REPMINE) > 0 )
		{
			pOwnerPlayer->GetLimitedQuantities()->Reset(LQ_REPMINE);
			ClientPrint( pOwnerPlayer, HUD_PRINTCENTER, UTIL_VarArgs("Detonated your repmine\n") );
			return false;
		}

		Vector vecSrc, vecAiming;

		// Take the eye position and direction
		vecSrc = pOwner->EyePosition();
		
		QAngle angles = pOwner->EyeAngles();

		AngleVectors( angles, &vecAiming );

		trace_t tr;

		UTIL_TraceLine( vecSrc, vecSrc + (vecAiming * 128), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
		
		if (tr.fraction < 1.0f && tr.plane.normal.z > 0.4f )
		{
			CBaseEntity *pEntity = tr.m_pEnt;
			if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR) && (!pOwnerPlayer || !pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_REPMINE)) )
			{
				QAngle angles;
				VectorAngles(tr.plane.normal, angles);

				angles.x += 90;

				CMagMine *pMine = (CMagMine*)CBaseEntity::CreateNoSpawn( "grenade_repmine", tr.endpos + tr.plane.normal * 3, angles, NULL );

				pMine->SetControllingModule(GetModuleIndex());
				pMine->SetThrower(pOwner);
				pMine->SetLevel(level);
				DispatchSpawn( pMine );
				if ( pOwnerPlayer )
					pOwnerPlayer->GetLimitedQuantities()->Add(LQ_REPMINE);
				
				return true;
			}
		}
		return false;
	}
	virtual int GetAIDescription() { return DEPLOYABLE; }
#else
	DATA_TABLE_ITEM(1, L"Strength", toStr(LEVEL(mod_repmine_strength, level)))
	DATA_TABLE_ITEM(2, L"Health", toStr((int)LEVEL(mod_repmine_health, level)))
	DATA_TABLE_ITEM(3, L"Damage", toStr((int)LEVEL(mod_repmine_damage, level)))
#endif
END_MODULE()
*/
//-----------------------------------------------------------------------------
// Purpose: Magnetic mines
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_magmine_drain);
extern LEVEL_EXTERN(mod_magmine_cooldown);
extern LEVEL_EXTERN(mod_magmine_damage);
extern LEVEL_EXTERN(mod_magmine_health);
extern LEVEL_EXTERN(mod_magmine_strength);
extern LEVEL_EXTERN(mod_magmine_fullradius);
extern LEVEL_EXTERN(mod_magmine_damage_radius);
extern McConVar mod_magmine_npc_scale, mod_magmine_allow_upward_force;
extern LEVEL_EXTERN(mod_magmine_limit);
DECLARE_MODULE(Magmine, InstantModule, TenLevelModule, magmine)
	virtual const char *DisplayName() { return "Mag Mine"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_magmine_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_magmine_cooldown, level); }

	virtual void Precache()
	{
		#ifndef CLIENT_DLL
			UTIL_PrecacheOther( "grenade_magmine" );
		#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->GetCount(LQ_MAGMINE) > 0 )
		{
			pOwnerPlayer->GetLimitedQuantities()->Reset(LQ_MAGMINE);
			ClientPrint( pOwnerPlayer, HUD_PRINTCENTER, UTIL_VarArgs("Detonated your magmine\n") );
			return false;
		}

		Vector vecSrc, vecAiming;

		// Take the eye position and direction
		vecSrc = pOwner->EyePosition();
		
		QAngle angles = pOwner->EyeAngles();

		AngleVectors( angles, &vecAiming );

		trace_t tr;

		UTIL_TraceLine( vecSrc, vecSrc + (vecAiming * 128), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
		
		if (tr.fraction < 1.0f && tr.plane.normal.z > 0.4f )
		{
			CBaseEntity *pEntity = tr.m_pEnt;
			if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR) && (!pOwnerPlayer || !pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_MAGMINE)) )
			{
				QAngle angles;
				VectorAngles(tr.plane.normal, angles);

				angles.x += 90;

				CMagMine *pMine = (CMagMine*)CBaseEntity::CreateNoSpawn( "grenade_magmine", tr.endpos + tr.plane.normal * 3, angles, NULL );

				pMine->SetControllingModule(GetModuleIndex());
				pMine->SetThrower(pOwner);
				pMine->SetLevel(level);
				DispatchSpawn( pMine );
				if ( pOwnerPlayer )
					pOwnerPlayer->GetLimitedQuantities()->Add(LQ_MAGMINE);
				
				return true;
			}
		}
		return false;
	}
	virtual int GetAIDescription() { return DEPLOYABLE; }
#else
	DATA_TABLE_ITEM(1, L"Strength", toStr(LEVEL(mod_magmine_strength, level)))
	DATA_TABLE_ITEM(2, L"Health", toStr((int)LEVEL(mod_magmine_health, level)))
	DATA_TABLE_ITEM(3, L"Damage", toStr((int)LEVEL(mod_magmine_damage, level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Magnetic Grenade
//-----------------------------------------------------------------------------
#define MAGD_STRENGTH_SCALE 500.0f
Vector CalcMagdForce(CBaseCombatCharacter *magd, CBaseCombatCharacter *sucked, float interval)
{
	CBaseGrenade *pMagd = (CBaseGrenade*)magd;
	if ( HL2MPRules()->IsFriendly(sucked,pMagd->GetThrower()) || sucked->IsBuffActive(BUFF_SPAWN) )
		return vec3_origin;

	Vector direction = magd->GetAbsOrigin() - sucked->GetAbsOrigin();
	float seperation = VectorNormalize( direction );
	int level = magd->GetLevel();
	
	float baseForce = LEVEL(mod_magmine_strength, level) * MAGD_STRENGTH_SCALE * interval;
	float fullEffectRadius = LEVEL(mod_magmine_fullradius, level);
	
	Vector force;

	if ( seperation < fullEffectRadius )
		force = baseForce * direction;
		
	else if ( seperation < fullEffectRadius * 1.1f )
	{
		float scaleDown = 1.0f - ((seperation - fullEffectRadius) / (fullEffectRadius * 0.1f));
		force = baseForce * scaleDown * direction;
	}
	
	else
		force = vec3_origin;
	
	// cannot pull upwards if convar is not set
	if ( mod_magmine_allow_upward_force.GetInt() == 0 && force.z > 0 )
		force.z = 0;

	// only allow suction when in line of sight
	trace_t tr;
	UTIL_TraceLine( magd->WorldSpaceCenter(), sucked->WorldSpaceCenter(), MASK_PLAYERSOLID_BRUSHONLY, magd, COLLISION_GROUP_PLAYER_MOVEMENT, &tr );
	if ( !tr.DidHit() || tr.m_pEnt == sucked )
		return force;

	return vec3_origin;
}

Vector CalcMagdForce(CBaseCombatCharacter *magd, IPhysicsObject *pPhysics, CBaseEntity *sucked, float interval)
{
	Vector physPos;
	pPhysics->GetPosition(&physPos,NULL);

	Vector magdPos = magd->GetAbsOrigin();
	magdPos.z = magdPos.z + 30;

	Vector direction = magd->GetAbsOrigin() - physPos;
	float seperation = VectorNormalize( direction );
	int level = magd->GetLevel();
	
	float baseForce = LEVEL(mod_magmine_strength, level) * MAGD_STRENGTH_SCALE * interval;
	float fullEffectRadius = LEVEL(mod_magmine_fullradius, level);
	
	Vector force;

	if ( seperation < fullEffectRadius )
		force = baseForce * direction;
		
	else if ( seperation < fullEffectRadius * 1.1f )
	{
		float scaleDown = 1.0f - ((seperation - fullEffectRadius) / (fullEffectRadius * 0.1f));
		force = baseForce * scaleDown * direction;
	}
	
	else
		force = vec3_origin;

	// only allow suction when in line of sight
	trace_t tr;
	UTIL_TraceLine( magd->WorldSpaceCenter(), sucked->WorldSpaceCenter(), MASK_PLAYERSOLID_BRUSHONLY, magd, COLLISION_GROUP_PLAYER_MOVEMENT, &tr );
	if ( !tr.DidHit() || tr.m_pEnt == sucked )
		return force;

	return vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose: Freeze grenade. Deals no damage, but immobilizes for a short period
//-----------------------------------------------------------------------------
// /*
extern LEVEL_EXTERN(mod_freezebomb_drain);
extern LEVEL_EXTERN(mod_freezebomb_cooldown);
extern LEVEL_EXTERN(mod_freezebomb_duration);
DECLARE_MODULE(FreezeGrenade, InstantModule, TenLevelModule, freeze_bomb)
	virtual const char *DisplayName() { return "Freeze Bomb"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_freezebomb_drain, level); }
	virtual float GetCooldown(int level)
	{
		return LEVEL(mod_freezebomb_cooldown, level)
#ifndef CLIENT_DLL // Hide random value from buy menu.
		+ ((rand() % 250) / 100.0) * (rand() % 2 == 1 ? -1 : 1) // Randomize the thaw period.
#endif
			; 
	}

	virtual void Precache()
	{
#ifndef CLIENT_DLL
		UTIL_PrecacheOther( "npc_grenade_frag" );
#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{// just lifted from the grenade weapon code
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		Vector	vecEye = pOwner->EyePosition();
		Vector	vForward, vRight;

		pOwner->EyeVectors( &vForward, &vRight, NULL );
		Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
		CheckThrowPosition( pOwner, vecEye, vecSrc );
	//	vForward[0] += 0.1f;
		vForward[2] += 0.1f;

		Vector vecThrow;
		pOwner->GetVelocity( &vecThrow, NULL );
		vecThrow += vForward * 900;

		CBaseGrenade *pGrenade = Freeze_Grenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pOwner, 2.5f, level );

		if ( pGrenade )
		{
			if ( pOwner && pOwner->m_lifeState != LIFE_ALIVE )
			{
				pOwner->GetVelocity( &vecThrow, NULL );

				IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
				if ( pPhysicsObject )
				{
					pPhysicsObject->SetVelocity( &vecThrow, NULL );
				}
			}
			pGrenade->SetControllingModule(GetModuleIndex());
			pGrenade->m_iParticleEffect = FREEZE_EFFECT;
		}

		pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));
		if ( pOwnerPlayer )
			pOwnerPlayer->SetAnimation( PLAYER_ATTACK1 );
		return true;
	}

	// also part of the grenade weapon code
	void CheckThrowPosition( CBaseCombatCharacter *pThrower, const Vector &vecEye, Vector &vecSrc )
	{
		trace_t tr;

		UTIL_TraceHull( vecEye, vecSrc, -Vector(6,6,6), Vector(6,6,6), 
			pThrower->PhysicsSolidMaskForEntity(), pThrower, pThrower->GetCollisionGroup(), &tr );
		
		if ( tr.DidHit() )
		{
			vecSrc = tr.endpos;
		}
	}
	virtual int GetAIDescription() { return PROJECTILE_BALLISTIC; }
#else
	DATA_TABLE_ITEM(1, L"Duration", toStr(LEVEL(mod_freezebomb_duration, level)))
#endif
END_MODULE()
// */

//-----------------------------------------------------------------------------
// Purpose: Incendiary Grenade. Lights shit on fire.
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_incendiary_drain);
extern LEVEL_EXTERN(mod_incendiary_cooldown);
extern LEVEL_EXTERN(mod_incendiary_duration);
extern LEVEL_EXTERN(mod_incendiary_dps);
DECLARE_MODULE(IncendiaryGrenade, InstantModule, TenLevelModule, incendiary)
	virtual const char *DisplayName() { return "Incendiary Grenade"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_incendiary_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_incendiary_cooldown, level); }

	virtual void Precache()
	{
#ifndef CLIENT_DLL
		UTIL_PrecacheOther( "npc_grenade_frag" );
#endif
		PrecacheParticleSystem("incendiary");
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{// just lifted from the grenade weapon code
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		Vector	vecEye = pOwner->EyePosition();
		Vector	vForward, vRight;

		pOwner->EyeVectors( &vForward, &vRight, NULL );
		Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
		CheckThrowPosition( pOwner, vecEye, vecSrc );
	//	vForward[0] += 0.1f;
		vForward[2] += 0.1f;

		Vector vecThrow;
		pOwner->GetVelocity( &vecThrow, NULL );
		vecThrow += vForward * 900;

		CBaseGrenade *pGrenade = Incendiary_Grenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pOwner, 2.5f, level );

		if ( pGrenade )
		{
			if ( pOwner && pOwner->m_lifeState != LIFE_ALIVE )
			{
				pOwner->GetVelocity( &vecThrow, NULL );

				IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
				if ( pPhysicsObject )
				{
					pPhysicsObject->SetVelocity( &vecThrow, NULL );
				}
			}
			pGrenade->SetControllingModule(GetModuleIndex());
			pGrenade->m_iParticleEffect = EMBER_EFFECT;
		}

		pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));
		if ( pOwnerPlayer )
			pOwnerPlayer->SetAnimation( PLAYER_ATTACK1 );
		return true;
	}

	// also part of the grenade weapon code
	void CheckThrowPosition( CBaseCombatCharacter *pThrower, const Vector &vecEye, Vector &vecSrc )
	{
		trace_t tr;

		UTIL_TraceHull( vecEye, vecSrc, -Vector(6,6,6), Vector(6,6,6), 
			pThrower->PhysicsSolidMaskForEntity(), pThrower, pThrower->GetCollisionGroup(), &tr );
		
		if ( tr.DidHit() )
		{
			vecSrc = tr.endpos;
		}
	}
	virtual int GetAIDescription() { return PROJECTILE_BALLISTIC; }
#else
	DATA_TABLE_ITEM(1, L"Duration", toStr(LEVEL(mod_incendiary_duration, level)))
	DATA_TABLE_ITEM(2, L"Damage / sec", toStr((int)LEVEL(mod_incendiary_dps, level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: MIRV grenade. Explodes into a cluster of bomblets
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_mirvgrenade_drain);
extern LEVEL_EXTERN(mod_mirvgrenade_cooldown);
extern LEVEL_EXTERN(mod_mirvgrenade_damage);
extern LEVEL_EXTERN(mod_mirvgrenade_radius);
extern LEVEL_EXTERN(mod_mirvgrenade_bomblet_damage);
extern LEVEL_EXTERN(mod_mirvgrenade_bomblet_radius);
DECLARE_MODULE(Mirv, InstantModule, TenLevelModule, mirv)
	virtual const char *DisplayName() { return "MIRV"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_mirvgrenade_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_mirvgrenade_cooldown, level); }


	virtual void Precache()
	{
#ifndef CLIENT_DLL
		UTIL_PrecacheOther( "npc_grenade_frag" );
#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{// just lifted from the grenade weapon code
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		Vector	vecEye = pOwner->EyePosition();
		Vector	vForward, vRight;

		pOwner->EyeVectors( &vForward, &vRight, NULL );
		Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
		CheckThrowPosition( pOwner, vecEye, vecSrc );
	//	vForward[0] += 0.1f;
		vForward[2] += 0.1f;

		Vector vecThrow;
		pOwner->GetVelocity( &vecThrow, NULL );
		vecThrow += vForward * 900;
		CBaseGrenade *pGrenade = Mirv_Grenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pOwner, 2.5f, level );

		if ( pGrenade )
		{
			if ( pOwner && pOwner->m_lifeState != LIFE_ALIVE )
			{
				pOwner->GetVelocity( &vecThrow, NULL );

				IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
				if ( pPhysicsObject )
				{
					pPhysicsObject->SetVelocity( &vecThrow, NULL );
				}
			}
			pGrenade->SetControllingModule(GetModuleIndex());
		}

		pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));
		if ( pOwnerPlayer )
			pOwnerPlayer->SetAnimation( PLAYER_ATTACK1 );
		return true;
	}

	// also part of the grenade weapon code
	void CheckThrowPosition( CBaseCombatCharacter *pThrower, const Vector &vecEye, Vector &vecSrc )
	{
		trace_t tr;

		UTIL_TraceHull( vecEye, vecSrc, -Vector(6,6,6), Vector(6,6,6), 
			pThrower->PhysicsSolidMaskForEntity(), pThrower, pThrower->GetCollisionGroup(), &tr );
		
		if ( tr.DidHit() )
		{
			vecSrc = tr.endpos;
		}
	}
	virtual int GetAIDescription() { return PROJECTILE_BALLISTIC; }
#else
	DATA_TABLE_ITEM(1, L"Main dmg", toStr((int)LEVEL(mod_mirvgrenade_damage, level)))
	DATA_TABLE_ITEM(2, L"Bomblet dmg", toStr((int)LEVEL(mod_mirvgrenade_bomblet_damage, level)))
#endif
END_MODULE()


//-----------------------------------------------------------------------------
// Purpose: Poison Grenade, throws a grenade poisonous to its surroundings
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_poisonspit_drain);
extern LEVEL_EXTERN(mod_poisonspit_cooldown);
extern LEVEL_EXTERN(mod_poisonspit_damage);
extern LEVEL_EXTERN(mod_poisonspit_amount);
extern LEVEL_EXTERN(mod_poisonspit_speed);
extern LEVEL_EXTERN(mod_poisonspit_spread);
extern LEVEL_EXTERN(mod_poisonspit_damagescale_start);
extern LEVEL_EXTERN(mod_poisonspit_damagescale_end);
extern LEVEL_EXTERN(mod_poisonspit_damagescale_limit);
DECLARE_MODULE(PoisonSpit, InstantModule, TenLevelModule, poisonspit)
	virtual const char *DisplayName() { return "Poison Spit"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_poisonspit_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_poisonspit_cooldown, level); }


	virtual void Precache()
	{
#ifndef CLIENT_DLL
		//UTIL_PrecacheOther( "npc_grenade_frag" );
#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{// just lifted from the grenade weapon code - and spit code
		Vector	vecEye = pOwner->EyePosition();
		Vector	vForward, vRight;

		pOwner->EyeVectors( &vForward, &vRight, NULL );
		Vector vecSrc = vecEye + vForward * 24.0f + vRight * 8.0f;
		CheckThrowPosition( pOwner, vecEye, vecSrc );

		Vector vecOwner;
		pOwner->GetVelocity( &vecOwner, NULL );
		float speed = LEVEL(mod_poisonspit_speed, level);

		int numBalls = (int)LEVEL(mod_poisonspit_amount, level);
		for ( int i = 0; i < numBalls; i++ )
		{
			CGrenadeSpit *pGrenade = (CGrenadeSpit*) CreateEntityByName( "grenade_spit" );
			pGrenade->SetAbsOrigin( vecSrc );
			pGrenade->SetAbsAngles( vec3_angle );
			pGrenade->SetThrower( pOwner );
			pGrenade->SetOwnerEntity( pOwner );
			DispatchSpawn( pGrenade );
			pGrenade->SetDamage( LEVEL(mod_poisonspit_damage, level) );
			pGrenade->SetDamageIncreasesOverTime(true);
			pGrenade->SetControllingModule(GetModuleIndex());
			
			if ( i == 0 )
			{
				pGrenade->SetSpitSize( SPIT_LARGE );
				pGrenade->SetAbsVelocity( vForward * speed + vecOwner);
			}
			else
			{
				float spread = LEVEL(mod_poisonspit_spread, level);
				pGrenade->SetAbsVelocity( ( vForward + RandomVector( -spread, spread ) ) * speed + vecOwner);
				pGrenade->SetSpitSize( random->RandomInt( SPIT_SMALL, SPIT_MEDIUM ) );
			}

			// Tumble through the air
			pGrenade->SetLocalAngularVelocity(
				QAngle( random->RandomFloat( -250, -500 ),
						random->RandomFloat( -250, -500 ),
						random->RandomFloat( -250, -500 ) ) );
		}

		for ( int i = 0; i < 4; i++ )
			DispatchParticleEffect( "blood_impact_yellow_01", vecSrc + RandomVector( -12.0f, 12.0f ), pOwner->EyeAngles() + RandomAngle(-16,16) );

		//if ( mod_poisonspit_sound.GetInt() == 1 )
			//pOwner->EmitSound( "NPC_Antlion.PoisonShoot" );

		pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pOwner);
		if ( pOwnerPlayer )
			pOwnerPlayer->SetAnimation( PLAYER_ATTACK1 );
		return true;
	}

	// also part of the grenade weapon code
	void CheckThrowPosition( CBaseCombatCharacter *pThrower, const Vector &vecEye, Vector &vecSrc )
	{
		trace_t tr;

		UTIL_TraceHull( vecEye, vecSrc, -Vector(6,6,6), Vector(6,6,6), 
			pThrower->PhysicsSolidMaskForEntity(), pThrower, pThrower->GetCollisionGroup(), &tr );
		
		if ( tr.DidHit() )
		{
			vecSrc = tr.endpos;
		}
	}
	virtual int GetAIDescription() { return PROJECTILE_BALLISTIC; }
#else
	DATA_TABLE_ITEM(1, L"Globs", toStr((int)LEVEL(mod_poisonspit_amount, level)))
	DATA_TABLE_ITEM(2, L"Damage", toStr((int)LEVEL(mod_poisonspit_damage, level)))
	DATA_TABLE_ITEM(3, L"Scale max", toStr(LEVEL(mod_poisonspit_damagescale_limit, level)))
	DATA_TABLE_ITEM(4, L"Scale time", toStr(LEVEL(mod_poisonspit_damagescale_end, level)))
#endif
END_MODULE()


//-----------------------------------------------------------------------------
// Purpose: Kamakaze crow launcher. Releases a crow, carrying a small plastic explosive,
// trained to fly at enemies and then detonate it
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_crowlauncher_drain);
extern LEVEL_EXTERN(mod_crowlauncher_cooldown);
extern LEVEL_EXTERN(mod_crow_health);
extern LEVEL_EXTERN(mod_crow_airspeed);
extern LEVEL_EXTERN(mod_crow_explode_damage);
extern LEVEL_EXTERN(mod_crow_limit);
extern McConVar mc_player_max_crows;
DECLARE_MODULE(Crow, InstantModule, TenLevelModule, crow)
	virtual const char *DisplayName() { return "Crow Launcher"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_crowlauncher_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_crowlauncher_cooldown, level); }

	virtual void Precache()
	{
#ifndef CLIENT_DLL
		UTIL_PrecacheOther( "npc_crow" );
#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		if ( pOwnerPlayer && pOwnerPlayer->GetLimitedQuantities()->IsFull(LQ_CROW) )
			return false;

		Vector	vForward,vRight,vUp;
		pOwner->EyeVectors( &vForward, &vRight, &vUp );
		Vector src = pOwner->Weapon_ShootPosition( ) + 56.0f*vForward + 8.0f*vRight - 3.0f*vUp ; // position not confirmed

		Vector vecLaunch;
		pOwner->GetVelocity( &vecLaunch, NULL );
		vecLaunch += vForward * 250;
		
		CAI_BaseNPC *pCrow = CreateNPC(g_Crow,src,QAngle(0,pOwner->GetAbsAngles()[YAW],0),pOwnerPlayer,level,false);
		if ( pCrow )
		{
			pCrow->SetAbsVelocity( vecLaunch );
			pCrow->SetControllingModule(GetModuleIndex());
			pCrow->RemoveBuff(BUFF_SPAWN); // dont fade
			pCrow->RemoveBuff(BUFF_MINION); // crows shouldn't affect minion limit, they're seperate

			if ( pOwnerPlayer )
				pOwnerPlayer->GetLimitedQuantities()->Add(LQ_CROW);
			return true;
		}

		pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));
		pOwner->ViewPunch( QAngle( random->RandomFloat( -2, -1 ), random->RandomFloat( -2, 2 ), 0 ) );
		if ( pOwnerPlayer )
			pOwnerPlayer->SetAnimation( PLAYER_ATTACK1 );
		return true;
	}
	virtual int GetAIDescription() { return DEPLOYABLE; }
#else
	DATA_TABLE_ITEM(1, L"Speed", toStr((int)(LEVEL(mod_crow_airspeed, level)/12.0f)))
	DATA_TABLE_ITEM(2, L"Damage", toStr((int)LEVEL(mod_crow_explode_damage, level)))
	DATA_TABLE_ITEM(3, L"Max", toStr((int)LEVEL(mod_crow_limit, level)))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Disorientation grenade. Deals no damage, but randomizes player controls
//-----------------------------------------------------------------------------
/*
DECLARE_MODULE(Disorientation, InstantModule, SingleLevelModule, disorientation)
	virtual const char *DisplayName() { return "Disorientation"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return 50; }
	virtual float GetCooldown(int level) { return 2.5f; }


	virtual void Precache()
	{
#ifndef CLIENT_DLL
		UTIL_PrecacheOther( "npc_grenade_frag" );
#endif
		Module::Precache();
	}

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{// just lifted from the grenade weapon code
		Vector	vecEye = pOwner->EyePosition();
		Vector	vForward, vRight;

		pOwner->EyeVectors( &vForward, &vRight, NULL );
		Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
		CheckThrowPosition( pOwner, vecEye, vecSrc );
	//	vForward[0] += 0.1f;
		vForward[2] += 0.1f;

		Vector vecThrow;
		pOwner->GetVelocity( &vecThrow, NULL );
		vecThrow += vForward * 1200;
		CBaseGrenade *pGrenade = Disorientate_Grenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pOwner, 2.5f, level );

		if ( pGrenade )
		{
			if ( pOwner && pOwner->m_lifeState != LIFE_ALIVE )
			{
				pOwner->GetVelocity( &vecThrow, NULL );

				IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
				if ( pPhysicsObject )
				{
					pPhysicsObject->SetVelocity( &vecThrow, NULL );
				}
			}
			pGrenade->SetControllingModule(GetModuleIndex());
		}

		pOwner->SetAnimation( PLAYER_ATTACK1 );
		return true;
	}

	// also part of the grenade weapon code
	void CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
	{
		trace_t tr;

		UTIL_TraceHull( vecEye, vecSrc, -Vector(6,6,6), Vector(6,6,6), 
			pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
		
		if ( tr.DidHit() )
		{
			vecSrc = tr.endpos;
		}
	}
#endif
END_MODULE()
*/

//-----------------------------------------------------------------------------
// Purpose: Plague (Applies plague affliction to nearby players, which slowly drains health indefinitely)
//-----------------------------------------------------------------------------
#ifndef CLIENT_DLL
void EmitPlague(CBaseCombatCharacter *pEmitter, CBaseCombatCharacter *pAttacker, int level);
#endif
extern LEVEL_EXTERN(mod_plague_damage);
extern LEVEL_EXTERN(mod_plague_victim_interval);
extern LEVEL_EXTERN(mod_plague_caster_interval);
extern LEVEL_EXTERN(mod_plague_radius);
extern LEVEL_EXTERN(mod_plague_drain);
extern LEVEL_EXTERN(mod_plague_drain_per_victim);

DECLARE_MODULE(Plague, ToggledRepeatingModule, TenLevelModule, plague)
	virtual const char *DisplayName() { return "Plague"; }
	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level)
	{
		return LEVEL(mod_plague_drain, level)
#ifndef CLIENT_DLL // don't have the buy menu show this extra drain on its single-number "aux cost" value
		+ pOwner->GetBuffLevel(BUFF_PLAGUE_STATUS) * LEVEL(mod_plague_drain_per_victim, level)
#endif
;
	}
	virtual bool UsesSmoothAuxDrain() { return true; } // causes aux drain to be "per second"

	virtual float GetTickInterval(int level) { return LEVEL(mod_plague_caster_interval, level); }
	virtual float GetCooldown(int level) { return 0; }
	
	static float GetRadius(int level)
	{
		return LEVEL(mod_plague_radius, level);
	}
#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		EmitPlague(pOwner,pOwnerPlayer,level);
		return true;
	}

	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)
	{
		// remove the plague status buff from myself
		pOwner->RemoveBuff(BUFF_PLAGUE_STATUS);
	
		// and then cure all plague victims
		for (int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
			if ( !pPlayer )
				continue;

			if ( pPlayer->IsBuffActive(DEBUFF_PLAGUE) && pPlayer->GetInflictor(DEBUFF_PLAGUE) == pOwner )
				pPlayer->RemoveBuff(DEBUFF_PLAGUE);
		}

		CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
		int nAIs = g_AI_Manager.NumAIs();
		for ( int i = 0; i < nAIs; i++ )// look through all the AIs and check each
		{
			CAI_BaseNPC *pNPC = ppAIs[i];
			if ( !pNPC )
				continue;
			if ( pNPC->IsBuffActive(DEBUFF_PLAGUE) && pNPC->GetInflictor(DEBUFF_PLAGUE) == pOwner )
				pNPC->RemoveBuff(DEBUFF_PLAGUE);
		}
	}
	virtual int GetAIDescription() { return PASSIVE; }
#else
	DATA_TABLE_ITEM(1, L"Damage / sec", toStr(LEVEL(mod_plague_damage, level) / LEVEL(mod_plague_victim_interval, level)))
#endif
END_MODULE()
#ifndef CLIENT_DLL
// a seperate function because its called by victims too, to spread the ... love
void EmitPlague(CBaseCombatCharacter *pEmitter, CBaseCombatCharacter *pAttacker, int level)
{
	CBaseEntity *pTarget = NULL;
	float radius = Plague::GetRadius(level);
	while ( ( pTarget = gEntList.FindEntityInSphere( pTarget, pEmitter->WorldSpaceCenter(), radius ) ) != NULL )
	{
		CBaseCombatCharacter *pOther = pTarget->MyCombatCharacterPointer();

		// only NPCs and players should be affected, and only if they're not the emitting or (originally) attacking player
		if ( pOther == NULL || !(pOther->IsNPC() || (pOther->IsPlayer() && ToHL2MPPlayer(pOther)->IsInCharacter())) || pOther == pEmitter || pOther == pAttacker )
			continue;

		// currently, plague doesn't affect allies of the attacking player
		if ( HL2MPRules()->IsFriendly(pAttacker,pOther) )
			continue;

		// don't give low-level plague if they already have The Clap, or are healing it (prevents flickering)
		if ( pOther->GetBuffLevel(DEBUFF_PLAGUE) >= level || pOther->IsBuffActive(BUFF_HEALD) )
			continue;

		pOther->ApplyBuff(DEBUFF_PLAGUE,pAttacker,level);
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Piercing Resist. Player takes less damage from melee & crossbow attacks
//-----------------------------------------------------------------------------
/*
McConVar mod_piercingresist_scale("mod_piercingresist_scale", "30", FCVAR_NOTIFY | FCVAR_REPLICATED, "Piercing resist reduces all piercing melee & crossbow damage taken by this percentage", true, 0, true, 100 );
class PiercingResist : public PassiveModule, CheapSingleLevelModule
{
public:
	virtual const char *DisplayName() { return "Piercing Resist"; }
	virtual const char *CmdName() { return "piercingresist"; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level) { }
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level) { }
#else
	virtual wchar_t* GetDescriptionParameter1(int level)
	{
		return percent(mod_piercingresist_scale.GetFloat()/100.0f);
	}
#endif
};
*/
//-----------------------------------------------------------------------------
// Purpose: Bullet Resist. Player takes less damage from bullet attacks
//-----------------------------------------------------------------------------
/*
McConVar mod_bulletresist_scale("mod_bulletresist_scale", "20", FCVAR_NOTIFY | FCVAR_REPLICATED, "Bullet resist reduces all bullet damage taken by this percentage", true, 0, true, 100 );
class BulletResist : public PassiveModule, CheapSingleLevelModule
{
public:
	virtual const char *DisplayName() { return "Bullet Resist"; }
	virtual const char *CmdName() { return "bulletresist"; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level) { }
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level) { }
#else
	virtual wchar_t* GetDescriptionParameter1(int level)
	{
		return percent(mod_bulletresist_scale.GetFloat()/100.0f);
	}
#endif
};
*/
//-----------------------------------------------------------------------------
// Purpose: Crush Resist. Player takes less damage from gravity gun attacks
//-----------------------------------------------------------------------------
/*
McConVar mod_crushresist_scale("mod_crushresist_scale", "45", FCVAR_NOTIFY | FCVAR_REPLICATED, "Crush resist reduces all impact-based damage taken by this percentage", true, 0, true, 100 );
class CrushResist : public PassiveModule, CheapSingleLevelModule
{
public:
	virtual const char *DisplayName() { return "Crush Resist"; }
	virtual const char *CmdName() { return "crushresist"; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level) { }
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level) { }
#else
	virtual wchar_t* GetDescriptionParameter1(int level)
	{
		return percent(mod_crushresist_scale.GetFloat()/100.0f);
	}
#endif
};
*/
//-----------------------------------------------------------------------------
// Purpose: Poison Resist. Player takes less damage from poison attacks
//-----------------------------------------------------------------------------
/*
McConVar mod_poisonresist_scale("mod_poisonresist_scale", "60", FCVAR_NOTIFY | FCVAR_REPLICATED, "Poison resist reduces all poison damage taken by this percentage", true, 0, true, 100 );
class PoisonResist : public PassiveModule, CheapSingleLevelModule
{
public:
	virtual const char *DisplayName() { return "Poison Resist"; }
	virtual const char *CmdName() { return "poisonresist"; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level) { }
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level) { }
#else
	virtual wchar_t* GetDescriptionParameter1(int level)
	{
		return percent(mod_poisonresist_scale.GetFloat()/100.0f);
	}
#endif
};

//-----------------------------------------------------------------------------
// Purpose: Blast Resist. Player takes less damage from explosive attacks
//-----------------------------------------------------------------------------
McConVar mod_blastresist_scale("mod_blastresist_scale", "33", FCVAR_NOTIFY | FCVAR_REPLICATED, "Blast resist reduces all damage taken from explosions by this percentage", true, 0, true, 100 );
class BlastResist : public PassiveModule, CheapSingleLevelModule
{
public:
	virtual const char *DisplayName() { return "Blast Resist"; }
	virtual const char *CmdName() { return "blastresist"; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level) { }
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level) { }
#else
	virtual wchar_t* GetDescriptionParameter1(int level)
	{
		return percent(mod_blastresist_scale.GetFloat()/100.0f);
	}
#endif
};

//-----------------------------------------------------------------------------
// Purpose: Energy Resist. Player takes less damage from laser & electric attacks
//-----------------------------------------------------------------------------
McConVar mod_energyresist_scale("mod_energyresist_scale", "33", FCVAR_NOTIFY | FCVAR_REPLICATED, "Blast resist reduces all damage taken from lasers & other high-energy attacks by this percentage", true, 0, true, 100 );
class EnergyResist : public PassiveModule, CheapSingleLevelModule
{
public:
	virtual const char *DisplayName() { return "Energy Resist"; }
	virtual const char *CmdName() { return "energyresist"; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level) { }
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level) { }
#else
	virtual wchar_t* GetDescriptionParameter1(int level)
	{
		return percent(mod_energyresist_scale.GetFloat()/100.0f);
	}
#endif
};

//-----------------------------------------------------------------------------
// Purpose: Fire Resist. Player takes less damage from fire-based sources
//-----------------------------------------------------------------------------
McConVar mod_fireresist_scale("mod_fireresist_scale", "40", FCVAR_NOTIFY | FCVAR_REPLICATED, "Fire resist reduces all damage from fire-based attacks by this percentage", true, 0, true, 100 );
class FireResist : public PassiveModule, CheapSingleLevelModule
{
public:
	virtual const char *DisplayName() { return "Fire Resist"; }
	virtual const char *CmdName() { return "fireresist"; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level) { }
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level) { }
#else
	virtual wchar_t* GetDescriptionParameter1(int level)
	{
		return percent(mod_fireresist_scale.GetFloat()/100.0f);
	}
#endif
};
*/

//-----------------------------------------------------------------------------
// Purpose: HEALD. Uses energy to regenerate health.
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_heald_drain);
extern LEVEL_EXTERN(mod_heald_cooldown);
extern LEVEL_EXTERN(mod_heald_cast_time);
extern LEVEL_EXTERN(mod_heald_duration);
extern LEVEL_EXTERN(mod_heald_hps);
DECLARE_MODULE(Heald, InstantModule, TenLevelModule, heald)
	virtual const char *DisplayName() { return "HEALD"; }
	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_heald_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_heald_cooldown, level); }
	virtual float GetCastTime(int level) { return LEVEL(mod_heald_cast_time, level); }

	virtual const char *ParticleEffect() { return "heald cast"; }

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		// find a player or monster in front of owner, apply affliction
		CBaseCombatCharacter *pTarget = (CBaseCombatCharacter*)(pOwner->GetAimTarget(true));

		if ( pTarget && pTarget->ApplyBuff(BUFF_HEALD,pOwner,level) )
		{
			pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));
			return true;
		}
		else if ( pOwner->ApplyBuff(BUFF_HEALD,pOwner,level) )
		{
			pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));
			return true;
		}
		return false;
	}
	virtual int GetAIDescription() { return TRIGGERED_BUFF | INSTANT_TARGET_EFFECT; } // or maybe not have them heal allies with this? Thats a bit much probably
#else
	DATA_TABLE_ITEM(1, L"Healing", toStr( (int)(LEVEL(mod_heald_hps, level) * LEVEL(mod_heald_duration, level)) ))
	DATA_TABLE_ITEM(2, L"Duration", toStr( LEVEL(mod_heald_duration, level) ))
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: BIOARMOR. Uses energy to regenerate armor.
//-----------------------------------------------------------------------------
/*
extern LEVEL_EXTERN(mod_bioarmor_drain);
extern LEVEL_EXTERN(mod_bioarmor_cooldown);
extern LEVEL_EXTERN(mod_bioarmor_cast_time);
extern LEVEL_EXTERN(mod_bioarmor_duration);
extern LEVEL_EXTERN(mod_bioarmor_aps);
DECLARE_MODULE(BioArmor, InstantModule, TenLevelModule, bioarmor)
	virtual const char *DisplayName() { return "BioArmor"; }
	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_bioarmor_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_bioarmor_cooldown, level); }
	virtual float GetCastTime(int level) { return LEVEL(mod_bioarmor_cast_time, level); }

	virtual const char *ParticleEffect() { return "bioarmor cast"; }

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		// find a player or monster in front of owner, apply affliction
		CBaseCombatCharacter *pTarget = (CBaseCombatCharacter*)(pOwner->GetAimTarget(true));

		if ( pTarget && pTarget->ApplyBuff(BUFF_BIOARMOR,pOwner,level) )
		{
			pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));
			return true;
		}
		else if ( pOwner->ApplyBuff(BUFF_BIOARMOR,pOwner,level) )
		{
			pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));
			return true;
		}
		return false;
	}
	virtual int GetAIDescription() { return TRIGGERED_BUFF | INSTANT_TARGET_EFFECT; }
#else
	DATA_TABLE_ITEM(1, L"Charging", toStr( (int)(LEVEL(mod_bioarmor_aps, level) * LEVEL(mod_bioarmor_duration, level)) ))
	DATA_TABLE_ITEM(2, L"Duration", toStr( LEVEL(mod_bioarmor_duration, level) ))
#endif
END_MODULE()
*/
//-----------------------------------------------------------------------------
// Purpose: Attrition. Disallows target from picking up items.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Purpose: Attrition. Prevents player from regenerating health, armor, & energy.
//-----------------------------------------------------------------------------
/*
extern LEVEL_EXTERN(mod_attrition_drain);
extern LEVEL_EXTERN(mod_attrition_cooldown);
extern LEVEL_EXTERN(mod_attrition_duration);
DECLARE_MODULE(Attrition, InstantModule, TenLevelModule, attrition)
	virtual const char *DisplayName() { return "Attrition"; }
	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_attrition_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_attrition_cooldown, level); }

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		// find a player or monster in front of owner, apply affliction
		CBaseCombatCharacter *pTarget = (CBaseCombatCharacter*)(pOwner->GetAimTarget(false));

		if ( pTarget && pTarget->ApplyBuff(DEBUFF_ATTRITION,pOwner,level) )
		{
			pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, max(pOwner->GetModuleLevel(CLOAK),1));
			return true;
		}

		return false;
	}
	virtual int GetAIDescription() { return INSTANT_TARGET_EFFECT; }
#else
	DATA_TABLE_ITEM(1, L"Duration", toStr( LEVEL(mod_attrition_duration, level) ))
#endif
END_MODULE()
*/

//-----------------------------------------------------------------------------
// Purpose: Critical damage chance. Controls the chances to get a critical on attacks.
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_crits_chance);
extern LEVEL_EXTERN(mod_crits_drain_proportion);
extern LEVEL_EXTERN(mod_crits_scale);
DECLARE_MODULE(CriticalHits, MaintainedModule, TenLevelModule, crits)
	virtual const char *DisplayName() { return "Critical Hits"; }
	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return 0; }
	virtual float GetCooldown(int level) { return 0; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level) { }
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level) { }
	virtual int GetAIDescription() { return PASSIVE; }
#else
	DATA_TABLE_ITEM(1, L"Chance %", percent(LEVEL(mod_crits_chance, level)/100.0f))
	DATA_TABLE_ITEM(2, L"Scale", toStr( LEVEL(mod_crits_scale, level) ))
	DATA_TABLE_ITEM(3, L"Drain %", percent(LEVEL(mod_crits_drain_proportion, level)*100));
#endif

	virtual void Precache()
	{
		CBaseEntity::PrecacheScriptSound("Crit.Hit");
		CBaseEntity::PrecacheScriptSound("Crit.Shoot");
		Module::Precache();
	}
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Jetpack
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_jetpack_drain);
DECLARE_MODULE(Jetpack, MaintainedRepeatingModule, SingleLevelModule, jetpack)
	virtual const char *DisplayName() { return "Jetpack"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_jetpack_drain, level); }
	virtual bool UsesSmoothAuxDrain() { return true; } // causes aux drain to be "per second"
	virtual float GetTickInterval(int level) { return 1.0f; }
	virtual float GetCooldown(int level) { return 0; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
		pOwner->ApplyBuff(BUFF_JETPACK_FLAME);
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		return true;
	}

	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)
	{
		pOwner->RemoveBuff(BUFF_JETPACK_FLAME);
	}
	virtual int GetAIDescription() { return TRIGGERED_MOVEMENT | MAINTAINED_EFFECT; } // ok i have no idea how this will work really
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Teleport
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_teleport_drain);
extern LEVEL_EXTERN(mod_teleport_cooldown);
extern LEVEL_EXTERN(mod_teleport_range);
DECLARE_MODULE(Teleport, InstantModule, SingleLevelModule, teleport)
	virtual const char *DisplayName() { return "Teleport"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_teleport_drain, level); }
	virtual float GetCooldown(int level) { return LEVEL(mod_teleport_cooldown, level); }
//	virtual float GetCastTime(int level) { return 0.2f; }

	virtual const char *UseSound() { return "Module.Teleport"; }

	virtual void Precache()
	{
		PrecacheParticleSystem("tp_in");
		PrecacheParticleSystem("tp_out");
		PrecacheParticleSystem("tp_in_duck");
		PrecacheParticleSystem("tp_out_duck");
		Module::Precache();
	}

#ifndef CLIENT_DLL
	static bool CheckPosition(CBaseCombatCharacter *pOwner, Vector pos, Vector mins, Vector maxs)
	{
		if ( UTIL_PointContents( pos ) & CONTENTS_SOLID )
			return false;

		trace_t tr;
		Ray_t ray;
		ray.Init( pos, pos, mins, maxs );
		UTIL_TraceRay( ray, MASK_PLAYERSOLID, pOwner, COLLISION_GROUP_PLAYER, &tr );
		if ( (tr.contents & MASK_PLAYERSOLID) && tr.m_pEnt )
			return false;
		
		// now check each teleport blocker
		for ( int i=0; i<HL2MPRules()->NumTeleportBlockers(); i++ )
		{
			CTeleportBlocker *blocker = HL2MPRules()->GetTeleportBlocker(i);
			if ( !blocker->m_bHasRadius || (pos - blocker->GetAbsOrigin()).LengthSqr() <= blocker->GetRadiusSq() )
			{// within this blocker's sphere of influence, check if we have player movement "LOS" to it -
			 // if we do, this point is in a no-go area
				UTIL_TraceLine( pos, blocker->GetAbsOrigin(), MASK_PLAYERSOLID, pOwner, COLLISION_GROUP_PLAYER_MOVEMENT, &tr );
				if ( !tr.DidHit() )
					return false; // if nothing between here and the blocker, its a "blocked" position
			}
		}
		
		return true;
	}

	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level)
	{
		CHL2MP_Player *pOwnerPlayer = pOwner->IsPlayer() ? ToHL2MPPlayer(pOwner) : NULL;
		trace_t tr;
		Vector vForward;
		pOwner->EyeVectors( &vForward, NULL, NULL );
		
		Vector mins = pOwnerPlayer ? pOwnerPlayer->GetPlayerMins() : NAI_Hull::Mins(pOwner->GetHullType());
		Vector maxs = pOwnerPlayer ? pOwnerPlayer->GetPlayerMaxs() : NAI_Hull::Maxs(pOwner->GetHullType());

		bool bTeleport = false;
		Vector vTelePos;

		int traceDist = (int)LEVEL(mod_teleport_range, level);
		while ( traceDist > 128 && bTeleport == false )
		{
			Vector tracePos = pOwner->GetAbsOrigin() + vForward * traceDist;
			Vector tracePos2 = tracePos;

			for ( int i=0; i<6; i++ ) // try increasing the height a couple of times
			{
				UTIL_TraceHull(tracePos, tracePos2, mins, maxs, MASK_PLAYERSOLID, pOwner, COLLISION_GROUP_PLAYER, &tr);
				if ( !tr.DidHit() && CheckPosition(pOwner,tr.endpos,mins,maxs) )
				{
					bTeleport = true;
					vTelePos = tr.endpos;
					break;
				}
				else
				{
					tracePos2 = tracePos;
					tracePos += Vector(0,0,8);
				}
			}

			traceDist -= 32;
		}

		if ( bTeleport )
		{
			if ( pOwnerPlayer && pOwnerPlayer->IsDucked() )
				DispatchParticleEffect("tp_in_duck", pOwner->GetAbsOrigin(), pOwner->GetAbsAngles());
			else
				DispatchParticleEffect("tp_in", pOwner->GetAbsOrigin(), pOwner->GetAbsAngles());

			pOwner->AddEffects( EF_NOINTERP );
			pOwner->SetAbsOrigin(vTelePos);
			pOwner->ApplyBuff(DEBUFF_SUPPRESS_CLOAK, NULL, max(pOwner->GetModuleLevel(CLOAK),1));

			if ( pOwnerPlayer && pOwnerPlayer->IsDucked() )
				DispatchParticleEffect("tp_out_duck", pOwner->GetAbsOrigin(), pOwner->GetAbsAngles());
			else
				DispatchParticleEffect("tp_out", pOwner->GetAbsOrigin(), pOwner->GetAbsAngles());
		}

		return bTeleport;
	}
	virtual int GetAIDescription() { return TRIGGERED_MOVEMENT; }
#endif
END_MODULE()

//-----------------------------------------------------------------------------
// Purpose: Longjump. Allows the player to lunge forward periodically.
//-----------------------------------------------------------------------------
extern LEVEL_EXTERN(mod_longjump_speed);
extern LEVEL_EXTERN(mod_longjump_use_interval);
extern LEVEL_EXTERN(mod_longjump_drain);
extern LEVEL_EXTERN(mod_longjump_vertical_scale_factor);
DECLARE_MODULE(LongJump, PassiveModule, SingleLevelModule, longjump)
	virtual const char *DisplayName() { return "Long Jump"; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return LEVEL(mod_longjump_drain, level); }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}

	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level)
	{
	}
	virtual int GetAIDescription() { return TRIGGERED_MOVEMENT; }
#else
	DATA_TABLE_ITEM(1, L"Speed", toStr((int)LEVEL(mod_longjump_speed, level)))
	DATA_TABLE_ITEM(2, L"Drain", toStr((int)LEVEL(mod_longjump_drain, level)))
#endif
END_MODULE()
//--------------------------------------End Of Module Definitions---------------------------------------

#ifndef CLIENT_DLL

void SetupNPCVar(CNPCTypeInfo *info, int pos, const char *name, float base, float scale, float value)
{
	Q_snprintf(info->m_szVarNames[pos], MAX_NPC_VAR_NAME, name);
	info->m_flVarsBase[pos] = base;
	info->m_flVarsScale[pos] = scale;
	info->m_flVarsPower[pos] = value;
	info->m_iVarLimitType[pos] = 0;
	info->m_flVarsLimit[pos] = 0;	

	if ( info->m_iNumVars <= pos )
		info->m_iNumVars = pos+1;
}
#endif

extern const char *g_szEntityEffectNames[LAST_ENTITY_EFFECT];

void CreateModules()
{
	// core modules
	(new Vitality())->SetModuleIndex(VITALITY, MODULE_CATEGORY_CORE);
	(new ArmorCapacity())->SetModuleIndex(ARMOR_CAPACITY, MODULE_CATEGORY_CORE);
	(new AuxPowerTank())->SetModuleIndex(AUX_CAPACITY, MODULE_CATEGORY_CORE);

	// weapon modules
	(new Impact())->SetModuleIndex(STRENGTH, MODULE_CATEGORY_WEAPON);
	(new BruteForce())->SetModuleIndex(BRUTEFORCE, MODULE_CATEGORY_WEAPON);
	(new CriticalHits())->SetModuleIndex(CRITS, MODULE_CATEGORY_WEAPON);
	(new ClipSize())->SetModuleIndex(CLIP_SIZE, MODULE_CATEGORY_WEAPON);
	(new AmmoRegen())->SetModuleIndex(AMMOREGEN, MODULE_CATEGORY_WEAPON);

	// mobility
	(new Cloak())->SetModuleIndex(CLOAK, MODULE_CATEGORY_MOBILITY);
	(new Jetpack())->SetModuleIndex(JETPACK, MODULE_CATEGORY_MOBILITY);
	(new Teleport())->SetModuleIndex(TELEPORT, MODULE_CATEGORY_MOBILITY);
	(new LongJump())->SetModuleIndex(LONG_JUMP, MODULE_CATEGORY_MOBILITY);
	(new Adrenaline())->SetModuleIndex(ADRENALINE, MODULE_CATEGORY_MOBILITY);

	// projectiles
	(new EnergyBall())->SetModuleIndex(ENERGYBALL, MODULE_CATEGORY_PROJECTILES);
	(new Flechette())->SetModuleIndex(FLECHETTE, MODULE_CATEGORY_PROJECTILES);
	(new PoisonSpit())->SetModuleIndex(POISON_GRENADE, MODULE_CATEGORY_PROJECTILES);
	(new FreezeGrenade())->SetModuleIndex(FREEZE_GRENADE, MODULE_CATEGORY_PROJECTILES);
	(new IncendiaryGrenade())->SetModuleIndex(INCENDIARY_GRENADE, MODULE_CATEGORY_PROJECTILES);
	(new Mirv())->SetModuleIndex(MIRV, MODULE_CATEGORY_PROJECTILES);

	// target effects
	(new Heald())->SetModuleIndex(HEALD, MODULE_CATEGORY_TARGET_EFFECTS);
	//(new Charged())->SetModuleIndex(CHARGED, MODULE_CATEGORY_TARGET_EFFECTS);
	(new DamageAmp())->SetModuleIndex(DAMAGE_AMP, MODULE_CATEGORY_TARGET_EFFECTS);
	//(new Attrition())->SetModuleIndex(ATTRITION, MODULE_CATEGORY_TARGET_EFFECTS);
	(new Weaken())->SetModuleIndex(WEAKEN, MODULE_CATEGORY_TARGET_EFFECTS);
	(new Plague())->SetModuleIndex(PLAGUE, MODULE_CATEGORY_TARGET_EFFECTS);
	(new Shockwave())->SetModuleIndex(SHOCKWAVE, MODULE_CATEGORY_TARGET_EFFECTS);
	//(new BioArmor())->SetModuleIndex(BIOARMOR, MODULE_CATEGORY_TARGET_EFFECTS);

	// deployables
	(new Lasers())->SetModuleIndex(LASERS, MODULE_CATEGORY_DEPLOYABLES);
	(new Turret())->SetModuleIndex(TURRET, MODULE_CATEGORY_DEPLOYABLES);
	(new Magmine())->SetModuleIndex(MAGMINE, MODULE_CATEGORY_DEPLOYABLES);
//	(new Repmine())->SetModuleIndex(REPMINE, MODULE_CATEGORY_DEPLOYABLES);
	(new Crow())->SetModuleIndex(CROW, MODULE_CATEGORY_DEPLOYABLES);

	// minions
	(new MinionZombie())->SetModuleIndex(MINION_ZOMBIE, MODULE_CATEGORY_MINIONS);
	(new MinionFastZombie())->SetModuleIndex(MINION_FASTZOMBIE, MODULE_CATEGORY_MINIONS);
	(new MinionAntlion())->SetModuleIndex(MINION_ANTLION, MODULE_CATEGORY_MINIONS);
	(new MinionAntlionWorker())->SetModuleIndex(MINION_ANTLION_WORKER, MODULE_CATEGORY_MINIONS);
	//(new MinionFastHeadcrab())->SetModuleIndex(MINION_FASTHEADCRAB, MODULE_CATEGORY_MINIONS);
	(new MinionVortigaunt())->SetModuleIndex(MINION_VORTIGAUNT, MODULE_CATEGORY_MINIONS);
	(new MinionManhack())->SetModuleIndex(MINION_MANHACK, MODULE_CATEGORY_MINIONS);
	(new Recharge())->SetModuleIndex(RECHARGE, MODULE_CATEGORY_CORE);

	/*
	(new SprintSpeed())->SetModuleIndex(SPRINT_SPEED, MODULE_CATEGORY_CORE);
	(new Regeneration())->SetModuleIndex(REGENERATION, MODULE_CATEGORY_PASSIVE);
	*/
	(new ArmorRegen())->SetModuleIndex(ARMORREGEN, MODULE_CATEGORY_CORE);
	/*
	(new BulletResist())->SetModuleIndex(BULLET_RESIST, MODULE_CATEGORY_RESISTANCE);
	(new PiercingResist())->SetModuleIndex(PIERCING_RESIST, MODULE_CATEGORY_RESISTANCE);
	(new CrushResist())->SetModuleIndex(CRUSH_RESIST, MODULE_CATEGORY_RESISTANCE);
	(new BlastResist())->SetModuleIndex(BLAST_RESIST, MODULE_CATEGORY_RESISTANCE);
	(new EnergyResist())->SetModuleIndex(ENERGY_RESIST, MODULE_CATEGORY_RESISTANCE);
	(new PoisonResist())->SetModuleIndex(POISON_RESIST, MODULE_CATEGORY_RESISTANCE);
	(new FireResist())->SetModuleIndex(FIRE_RESIST, MODULE_CATEGORY_RESISTANCE);
	(new Barnacle())->SetModuleIndex(BARNACLE);
	*/

	//for ( int i=0; i<GetNumModules(); i++ )
		//GetModule(i)->Init();

	CreateBuffs();
	SetupParticleEffects();
	
#ifndef CLIENT_DLL
// initialize minion NPC type info based on convars rather than text files that may get accidentally left out of a custom monster setup
	g_MinionAntlion = new CNPCTypeInfo("npc_antlion", "Antlion", 0.0f);	
	g_MinionAntlion->m_flExperienceScale = 1.0f;
	SetupNPCVar(g_MinionAntlion, 0, "health",    EXPAND_LEVEL(mod_antlion_health));
	SetupNPCVar(g_MinionAntlion, 1, "dmg_swipe", EXPAND_LEVEL(mod_antlion_dmg_swipe));
	SetupNPCVar(g_MinionAntlion, 2, "dmg_jump",  EXPAND_LEVEL(mod_antlion_dmg_jump));
	SetupNPCVar(g_MinionAntlion, 3, "dmg_in_air",EXPAND_LEVEL(mod_antlion_dmg_jump));
	
	g_MinionAntlionWorker = new CNPCTypeInfo("npc_antlion_worker", "Antlion Worker", 0.0f);
	g_MinionAntlionWorker->m_flExperienceScale = 1.0f;
	SetupNPCVar(g_MinionAntlionWorker, 0, "health",    EXPAND_LEVEL(mod_antlionworker_health));
	SetupNPCVar(g_MinionAntlionWorker, 1, "dmg_burst", EXPAND_LEVEL(mod_antlionworker_dmg_burst));
	SetupNPCVar(g_MinionAntlionWorker, 2, "dmg_swipe", EXPAND_LEVEL(mod_antlionworker_dmg_swipe)); 
	SetupNPCVar(g_MinionAntlionWorker, 3, "dmg_jump",  EXPAND_LEVEL(mod_antlionworker_dmg_jump));
	SetupNPCVar(g_MinionAntlionWorker, 4, "dmg_in_air",EXPAND_LEVEL(mod_antlionworker_dmg_jump));
	SetupNPCVar(g_MinionAntlionWorker, 5, "dmg_spit",  EXPAND_LEVEL(mod_antlionworker_dmg_spit));

	g_MinionFastZombie = new CNPCTypeInfo("npc_fastzombie", "Fast Zombie", 0.0f);
	g_MinionFastZombie->m_flExperienceScale = 1.25f;
	SetupNPCVar(g_MinionFastZombie, 0, "health",   EXPAND_LEVEL(mod_fastzombie_health));
	SetupNPCVar(g_MinionFastZombie, 1, "dmg_leap", EXPAND_LEVEL(mod_fastzombie_dmg_leap));
	SetupNPCVar(g_MinionFastZombie, 2, "dmg_claw", EXPAND_LEVEL(mod_fastzombie_dmg_claw));
	SetupNPCVar(g_MinionFastZombie, 3, "dropnocrab", 1, 0, 0);

	g_MinionZombie = new CNPCTypeInfo("npc_zombie", "Zombie", 0.0f);
	SetupNPCVar(g_MinionZombie, 0, "health",   EXPAND_LEVEL(mod_zombie_health));
	SetupNPCVar(g_MinionZombie, 1, "dmg_one",  EXPAND_LEVEL(mod_zombie_dmg_one));
	SetupNPCVar(g_MinionZombie, 2, "dmg_both", EXPAND_LEVEL(mod_zombie_dmg_both));	
	SetupNPCVar(g_MinionZombie, 3, "dropnocrab", 1, 0, 0);

	g_MinionTurret = new CNPCTypeInfo("npc_turret_floor", "Turret", 0.0f);
	SetupNPCVar(g_MinionTurret, 0, "health",    EXPAND_LEVEL(mod_turret_health));
	SetupNPCVar(g_MinionTurret, 1, "dmg_shoot", EXPAND_LEVEL(mod_turret_dmg_shoot));
	
	g_MinionVortigaunt = new CNPCTypeInfo("npc_vortigaunt", "Vortigaunt", 0.0f);
	g_MinionVortigaunt->m_flExperienceScale = 1.25f;
	g_MinionVortigaunt->m_iBitsDisabledCapacities |= bits_CAP_INNATE_MELEE_ATTACK1; // don't let vort minions use dispel ... they're imba enough
	SetupNPCVar(g_MinionVortigaunt, 0, "health",    EXPAND_LEVEL(mod_vortigaunt_health));
	SetupNPCVar(g_MinionVortigaunt, 1, "dmg_claw",  EXPAND_LEVEL(mod_vortigaunt_dmg_claw));
	SetupNPCVar(g_MinionVortigaunt, 2, "dmg_rake",  EXPAND_LEVEL(mod_vortigaunt_dmg_rake));
	SetupNPCVar(g_MinionVortigaunt, 3, "dmg_zap",   EXPAND_LEVEL(mod_vortigaunt_dmg_zap));
	SetupNPCVar(g_MinionVortigaunt, 4, "zap_spread",EXPAND_LEVEL(mod_vortigaunt_zap_spread));
	
	g_MinionManhack = new CNPCTypeInfo("npc_manhack", "Manhack", 0.0f);
	g_MinionManhack->m_flExperienceScale = 0.5f;
	SetupNPCVar(g_MinionManhack, 0, "health",    EXPAND_LEVEL(mod_manhack_health));
	SetupNPCVar(g_MinionManhack, 1, "dmg_slash",  EXPAND_LEVEL(mod_manhack_dmg_slash));
	SetupNPCVar(g_MinionManhack, 2, "dmg_held",  EXPAND_LEVEL(mod_manhack_dmg_held));
	SetupNPCVar(g_MinionManhack, 3, "engine_power",   EXPAND_LEVEL(mod_manhack_engine_power));
	g_MinionManhack->m_iVarLimitType[3] = 2; // maximum
	g_MinionManhack->m_flVarsLimit[3] = mod_manhack_engine_power_max.GetFloat();
	
	g_Crow = new CNPCTypeInfo("npc_crow", "Crow", 0.0f);
	SetupNPCVar(g_Crow, 0, "health", EXPAND_LEVEL(mod_crow_health));
#endif
}

void DeleteModules()
{
	for ( int i=0; i<NUM_MODULES; i++ )
		delete GetModule(i);

	DeleteBuffs();
}

//==========================================================================
// here follows all the nuts & bolts that shouldn't need worried about much
//==========================================================================

#ifndef CLIENT_DLL
const char *Module::GetSuicideMessage(CHL2MP_Player *pKiller)
{
	return UTIL_VarArgs("%s had an unfortunate %s accident\n",pKiller->GetPlayerName(),GetDisplayName());
}

const char *Module::GetDeathMessage(CHL2MP_Player *pKiller,CHL2MP_Player *pVictim)
{
	return UTIL_VarArgs("%s was killed by %s's %s\n",pVictim->GetPlayerName(),pKiller->GetPlayerName(),GetDisplayName());
}
#endif

bool Module::IsSuppressed(CBaseCombatCharacter *pOwner) { return pOwner->IsBuffActive(BUFF_DISABLED); }

void Module::SetModule(int i, Module *a)
{
	g_Modules[i] = a;
}

Module *GetModule(int num)
{
	return g_Modules[num];
}

Module *GetModule(const char *name)
{
	for ( int i=0; i<NUM_MODULES; i++ )
		if ( FStrEq(g_Modules[i]->GetCmdName(),name) )
			return g_Modules[i];
	
	return NULL;
}

#ifndef CLIENT_DLL
CAI_BaseNPC *CreateNPC(CNPCTypeInfo *info, Vector origin, QAngle angles, CHL2MP_Player *pOwner, int level, bool fallToGround)
{
	//CNPCTypeInfo *info = GetNPCInfo(typeName);
	if ( !info )
	{
		Warning( "No NPC type!\n" );
		return NULL;
	}

	CAI_BaseNPC *pNPC = (CAI_BaseNPC*)CBaseEntity::CreateNoSpawn( info->m_szClassname, origin, angles );
	if ( !pNPC )
	{
		Warning( "Can't make %s!\n", info->m_szTypeName );
		return NULL;
	}

	pNPC->SetMasterPlayer( pOwner ); // need to attribute their damage
	pNPC->SetLikesMaster(true);
	if ( fallToGround )
		pNPC->AddSpawnFlags( SF_NPC_FALL_TO_GROUND | SF_NPC_FADE_CORPSE );
	else
		pNPC->AddSpawnFlags( SF_NPC_FADE_CORPSE );

	pNPC->SetStats(info,level); // calls DispatchSpawn

	pNPC->ApplyBuff(BUFF_SPAWN); // fade in with fancy effects

	pNPC->GetMotor()->SetIdealYaw( angles.y );
//	CBaseEntity *pTarget = pOwner->GetAimTarget();
//	if ( pTarget )
//		pNPC->SetEnemy(pTarget);

	pNPC->SetActivity( ACT_IDLE );
	pNPC->SetNextThink( gpGlobals->curtime );
	pNPC->PhysicsSimulate();

	pNPC->m_flNextAttack = gpGlobals->curtime + 0.15f;
	pNPC->Activate();

	return pNPC;
}

#else

enum op_t
{
 op_none = 0,
 op_add,
 op_subtract,
 op_multiply,
 op_divide,
};

// this is a complete rip-off of UTIL_ReplaceKeyBindings. A very handy function to rip-off, tbh.
void SubstituteConvars( wchar_t *inbuf, int inbufsizebytes, wchar_t *outbuf, int outbufsizebytes, int ownedLevel, int maxLevel )
{
	if ( !inbuf || !inbuf[0] )
		return;

	// copy to a new buf if there are vars
	outbuf[0]=0;
	int pos = 0;
	const wchar_t *inbufend = NULL;
	if ( inbufsizebytes > 0 )
	{
		inbufend = inbuf + ( inbufsizebytes / 2 );
	}

	while( inbuf != inbufend && *inbuf != 0 )
	{
		// check for variables
		if ( *inbuf == '%' )
		{
			++inbuf;

			wchar_t *end = wcschr( inbuf, '%' );
			if ( end && ( end != inbuf ) ) // make sure we handle %% in the string, which should be treated in the output as %
			{
				int expLen = end - inbuf;
				wchar_t expression[128];
				wcsncpy( expression, inbuf, expLen );
				expression[expLen] = 0;

				inbuf += expLen;

				// if there's a # in this expression, keep it and everything after it seperate. This represents the number of decimal places to be used when writing numbers out.
				int forceDecimalPlaces = -1; // -1 means don't force a particular number
				wchar_t *hash = wcschr( expression, '#' );
				if ( hash )
				{
					expression[hash-expression] = 0;
					hash = &expression[hash-expression+1]; // this may be dodgy!!
					
					char number[4];
					g_pVGuiLocalize->ConvertUnicodeToANSI( hash, number, sizeof(number) );
					forceDecimalPlaces = atoi(number); // returns 0 if it fails. I'd have preferred -1, but never mind.					
				}
				
				// rather than just a single convar / levelvar value, however, we allow + - * / to seperate multiple values. We also allow raw numbers to be included here (for instance, some_var/2).
				// CORRECT OPERATOR PRIORITY WILL NOT BE OBSERVED! Operators will be applied in the order they are written in. YOU HAVE BEEN WARNED.
				
				bool parseError = false;
				float val1 = 0, val2 = 0;
				op_t op = op_add, nextOp = op_none; // first function is ADD cos we add it onto the zero values.
				wchar_t *token_start = expression;
				wchar_t *token_end = expression + expLen;
				
				wchar_t *opPos = wcschr( token_start, '+' ); // doesn't find anything... Boo. Don't really understand why this doesn't work when previous use of wcschr does. On a pointer, no less.
				if ( opPos && opPos != expression && opPos < token_end )
				{
					token_end = opPos;
					nextOp = op_add;
				}
				opPos = wcschr( token_start, '-' );
				if ( opPos && opPos != expression && opPos < token_end )
				{
					token_end = opPos;
					nextOp = op_subtract;
				}
				opPos = wcschr( token_start, '*' );
				if ( opPos && opPos != expression && opPos < token_end )
				{
					token_end = opPos;
					nextOp = op_multiply;
				}
				opPos = wcschr( token_start, '/' );
				if ( opPos && opPos != expression && opPos < token_end )
				{
					token_end = opPos;
					nextOp = op_divide;
				}
				
				// one bit at a time ... do a loop!
				char value[32];
				while ( !parseError && token_start && token_start != token_end )
				{					
					wchar_t token[128];
					wcsncpy( token, token_start, token_end - token_start );
					token[token_end - token_start] = 0;
					char convarName[128];
					g_pVGuiLocalize->ConvertUnicodeToANSI( token, convarName, sizeof(convarName) );

					ConVarRef cv = ConVarRef(convarName, true);
					if ( !cv.IsValid() )
					{
						ConVarRef base = ConVarRef(VarArgs("%s_base", convarName), true);
						if ( !base.IsValid() ) // don't do 3 lookups if we only have to do one!
						{// it may still be a proper float value
							float f = atof(convarName);
							if ( f != 0 )
							{
								switch ( op )
								{
								case op_add:
								    val1 += f; val2 += f; break;
								case op_subtract:
								    val1 -= f; val2 -= f; break;
								case op_multiply:
								    val1 *= f; val2 *= f; break;
								case op_divide:
								    val1 /= f; val2 /= f; break;
								}
							}
							else
							{
								Q_snprintf( value, sizeof(value), "<INVALID>" );
								parseError = true;
								break;
							}
						}
						else
						{
							ConVarRef scale = ConVarRef(VarArgs("%s_scale", convarName), true);
							ConVarRef power = ConVarRef(VarArgs("%s_power", convarName), true);					
							if ( !scale.IsValid() || !power.IsValid() )
							{
								Q_snprintf( value, sizeof(value), "<BROKEN>" );
								parseError = true;
								break;
							}
							else
							{// what if we specified the levels to be used to show this?
							 // %mod_mymod_damage% would show 1...10, %mod_mymod_damage#3% would show level 3, and %mod_mymod_damage#3#7% would show 3...7.
							 // it'd also be nice if we could specify whether values should be displayed as floats or ints. Or if we just had a way of specifying # decimal places (0 for int, 1 for most stuff)
								
								int minLevel = ownedLevel == 0 ? 1 : ownedLevel, max_Level = ownedLevel == 0 ? maxLevel : ownedLevel;
								float f1 = LEVEL2(base.GetFloat(), scale.GetFloat(), power.GetFloat(), minLevel), f2 = LEVEL2(base.GetFloat(), scale.GetFloat(), power.GetFloat(), max_Level);
								switch ( op )
								{
								case op_add:
									val1 += f1; val2 += f2; break;
								case op_subtract:
									val1 -= f1; val2 -= f2; break;
								case op_multiply:
									val1 *= f1; val2 *= f2; break;
								case op_divide:
									val1 /= f1; val2 /= f2; break;
								}
							}
						}
					}
					else
					{// valid single convar
						float f = cv.GetFloat();
						switch ( op )
						{
						case op_add:
							val1 += f; val2 += f; break;
						case op_subtract:
							val1 -= f; val2 -= f; break;
						case op_multiply:
							val1 *= f; val2 *= f; break;
						case op_divide:
							val1 /= f; val2 /= f; break;
						}
					}
					
					// now advance over the next operator ... or the end
					if ( nextOp == op_none )
					{
						token_start = token_end;
					}
					else
					{
						token_start = token_end + 1;
						token_end = expression + expLen;
						op = nextOp;
						nextOp = op_none;
						
						opPos = wcschr( token_start, '+' );
						if ( opPos && opPos != expression && opPos < token_end )
						{
							token_end = opPos;
							nextOp = op_add;
						}
						opPos = wcschr( token_start, '-' );
						if ( opPos && opPos != expression && opPos < token_end )
						{
							token_end = opPos;
							nextOp = op_subtract;
						}
						opPos = wcschr( token_start, '*' );
						if ( opPos && opPos != expression && opPos < token_end )
						{
							token_end = opPos;
							nextOp = op_multiply;
						}
						opPos = wcschr( token_start, '/' );
						if ( opPos && opPos != expression && opPos < token_end )
						{
							token_end = opPos;
							nextOp = op_divide;
						}
					}
				}
				
				if ( !parseError )
				{
					if ( forceDecimalPlaces == 0 )
					{// cast to int, so that all values are rounded down.
						if ( val1 == val2 )
							Q_snprintf( value, sizeof(value), VarArgs("%i", (int)val1) );
						else
							Q_snprintf( value, sizeof(value), VarArgs("%i...%i", (int)val1, (int)val2) );
					}
					else if ( forceDecimalPlaces > 0 )
					{// use the user-specified number of decimal places for this value.
						if ( val1 == val2 )
							Q_snprintf( value, sizeof(value), VarArgs("%.*f", forceDecimalPlaces, val1) );
						else
							Q_snprintf( value, sizeof(value), VarArgs("%.*f...%.*f", forceDecimalPlaces, val1, forceDecimalPlaces, val2) );
					}
					else
					{// automatic decimal places. For each value (if saving two), remove any trailing zeros or dots in any number that contains a dot. Then (if saving two) combine them again.
						if ( val1 == val2 )
						{
							Q_snprintf( value, sizeof(value), VarArgs("%f", val1) );
							
							// remove any trailing 0s or .s if this value contains a .
							char *dotPos = strrchr(value,'.');
							if ( dotPos )
							{
								char *pos = strrchr(value, '0');
								while ( pos && pos > dotPos && pos-value == (int)strlen(value)-1)
								{
									value[pos - &value[0]] = 0;
									pos = strrchr(value, '0');
								}
							
								// remove trailing dot
								if ( (int)strlen(value)-1 == dotPos - value )
									value[dotPos - &value[0]] = 0;
							}
						}
						else
						{
							char formatted1[16], formatted2[16];
							Q_snprintf( formatted1, sizeof(formatted1), VarArgs("%.3f", val1) );
							Q_snprintf( formatted2, sizeof(formatted2), VarArgs("%.3f", val2) );
							
							
							// for both formatted1 & formatted2, remove any trailing 0s or .s if this value contains a .
							char *dotPos = strrchr(formatted1,'.');
							if ( dotPos )
							{
								char *pos = strrchr(formatted1, '0');
								while ( pos && pos > dotPos && pos-formatted1 == (int)strlen(formatted1)-1)
								{
									formatted1[pos - &formatted1[0]] = 0;
									pos = strrchr(formatted1, '0');
								}
							
								// remove trailing dot
								if ( (int)strlen(formatted1)-1 == dotPos - formatted1 )
									formatted1[dotPos - formatted1] = 0;
							}
							dotPos = strrchr(formatted2,'.');
							if ( dotPos )
							{
								char *pos = strrchr(formatted2, '0');
								while ( pos && pos > dotPos && pos-formatted2 == (int)strlen(formatted2)-1)
								{
									formatted2[pos - &formatted2[0]] = 0;
									pos = strrchr(formatted2, '0');
								}
							
								// remove trailing dot
								if ( (int)strlen(formatted2)-1 == dotPos - formatted2 )
									formatted2[dotPos - &formatted2[0]] = 0;
							}
							
							Q_snprintf( value, sizeof(value), VarArgs("%s...%s", formatted1, formatted2) );
						}
					}
				}
				g_pVGuiLocalize->ConvertANSIToUnicode( value, expression, sizeof(expression) );
				outbuf[pos] = '\0';
				wcscat( outbuf, expression );
				pos += wcslen(expression);
			}
			else
			{
				outbuf[pos] = *inbuf;
				++pos;
			}
		}
		else
		{
			outbuf[pos] = *inbuf;
			++pos;
		}

		++inbuf;
	}

	outbuf[pos] = '\0';
}

#define MAX_DESCRIPTION 2048
wchar_t moduleDesc[MAX_DESCRIPTION];
wchar_t *Module::GetDescription(int level)
{
	wchar_t raw[MAX_DESCRIPTION];
    g_pVGuiLocalize->ConstructString( raw, sizeof(raw), g_pVGuiLocalize->Find( VarArgs("#%s_desc", GetCmdName() ) ), 0);
	SubstituteConvars( raw, MAX_DESCRIPTION, moduleDesc, MAX_DESCRIPTION, level, GetMaxLevel() );
	
    return moduleDesc;
}

// turn a floating fraction into a rounded percentage (ie 0.0499 -> "5%")
// the + 0.5f isn't a mistake, adding half then casting to int is the quickest way to round a number in C++
#define numLen	16
wchar_t numval[numLen];
wchar_t *percent(float f)
{
	//_snwprintf(numval,numLen,L"%i%%", (int)(f*100.0f + 0.5f) );
	_snwprintf(numval,numLen,L"%i", (int)(f*100.0f + 0.5f) ); // adding the % symbol in the resource file instead, now
	return numval;
}

wchar_t *toStr(int i)
{
	_snwprintf(numval,numLen,L"%i",i); // possibly try numLen*sizeof(wchar_t) for these?
	return numval;
}

wchar_t *toStr(float f)
{
	_snwprintf(numval,numLen,L"%.1f",f);
	return numval;
}

#endif


void Module::Precache()
{
#ifndef CLIENT_DLL
	if ( m_bHasUseSound )
		CBaseEntity::PrecacheScriptSound( GetUseSound() );

	if ( m_bHasTickSound )
		CBaseEntity::PrecacheScriptSound( GetTickSound() );

	if ( m_bHasCastSound )
		CBaseEntity::PrecacheScriptSound( GetCastSound() );
#endif
	if ( m_bHasParticleEffect )
		PrecacheParticleSystem(m_szParticleEffect);
	if ( m_bHasLocalParticleEffect )
		PrecacheParticleSystem(m_szLocalParticleEffect);
}

void Module::Init(const char *cmdName)
{
	Q_snprintf(m_szCommandName,sizeof(m_szCommandName), cmdName);

	const char *temp = DisplayName();
	Q_snprintf(m_szDisplayName,sizeof(m_szDisplayName), temp);
//	delete temp;
	
	temp = DisplayType();
	Q_snprintf(m_szDisplayType,sizeof(m_szDisplayType), temp);
//	delete temp;

#ifdef CLIENT_DLL
	temp = GetDisplayName();
	g_pVGuiLocalize->ConvertANSIToUnicode(temp,m_wszNameUnicode,sizeof(m_wszNameUnicode));
//	delete temp;
#endif

	temp = ParticleEffect();
	m_bHasParticleEffect = temp != NULL;	
	if ( m_bHasParticleEffect )
		Q_snprintf(m_szParticleEffect,sizeof(m_szParticleEffect), temp);
//	delete temp;
	
	temp = LocalParticleEffect();
	m_bHasLocalParticleEffect = temp != NULL;
	if ( m_bHasLocalParticleEffect )
		Q_snprintf(m_szLocalParticleEffect,sizeof(m_szLocalParticleEffect), temp);
//	delete temp;
	
	temp = CastSound();
	m_bHasCastSound = temp != NULL;
	if ( m_bHasCastSound )
		Q_snprintf(m_szCastSound,sizeof(m_szCastSound), temp);
//	delete temp;
	
	temp = UseSound();
	m_bHasUseSound = temp != NULL;
	if ( m_bHasUseSound )
		Q_snprintf(m_szUseSound,sizeof(m_szUseSound), temp);
//	delete temp;
		
	temp = TickSound();
	m_bHasTickSound = temp != NULL;
	if ( m_bHasTickSound )
		Q_snprintf(m_szTickSound,sizeof(m_szTickSound), temp);
//	delete temp;
	
#ifdef CLIENT_DLL
	Q_snprintf(m_szIconName, sizeof(m_szIconName), VarArgs("gfx/abilities/%s",GetCmdName()));
	Q_snprintf(m_szFullIconName, sizeof(m_szFullIconName), VarArgs("vgui/gfx/abilities/%s",GetCmdName()));
#endif
}

#ifdef CLIENT_DLL

void CC_ListModules( void )
{
	for ( int i=0; i<NUM_MODULES; i++ )
    {
        Module *m = GetModule(i);
        Msg("%s - %s\n",m->GetDisplayName(), m->GetCmdName());
    }

}
static ConCommand cc_listmodules("listmodules", CC_ListModules, "List the name & command name of each module. Any active module can be bound to a key by typing 'bind <key> \"+do <module command name>\"' in the console.", 0);

void CC_DescribeModules( void )
{
	for ( int i=0; i<NUM_MODULES; i++ )
    {
        Module *m = GetModule(i);
        Msg("%s - %S\n",m->GetDisplayName(), m->GetDescription(0));
    }

}
static ConCommand cc_describemodules("describemodules", CC_DescribeModules, "List the name & description of each module.", 0);


#endif
