#include "cbase.h"
#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include <vgui/ILocalize.h>
	#include <vgui_controls/Label.h>	// seemingly needed for g_pVGuiLocalize
#else
	#include "hl2mp_player.h"
	#include "ai_basenpc.h" // for any monster spawning afflictions
	#include "hl2_player.h"
#endif
#include "buffs.h"
#include "modules.h"
#include "particle_effects.h"
#include "mcconvar.h"
#include "mc_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef CLIENT_DLL
#define VarArgs UTIL_VarArgs
#endif

extern const char *g_szEntityEffectNames[LAST_ENTITY_EFFECT];

int g_nBuffs = NUM_BUFFS;
Buff *g_Buffs[NUM_BUFFS];

const char *GetNameOf(CBaseCombatCharacter *pBCC)
{
	if ( pBCC->IsPlayer() )
		return ToHL2MPPlayer(pBCC)->GetPlayerName();
	else if ( pBCC->IsNPC() )
		return pBCC->GetClassname()+4;
	else
		return pBCC->GetClassname();
}

//-----------------------------------------------------------------------------
// Purpose: Color player on a spree.
//-----------------------------------------------------------------------------
class BuffSpree : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Spree"; }
	virtual const char *MaterialEffect() { return "spree"; }
	virtual bool PulsatingGlow() { return true; }

	virtual int GetBuffType() { return BUFF_TYPE_NEUTRAL; }
#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
//		Msg("Applying light effect to spreer\n");
//		pVictim->AddEffects( EF_BRIGHTLIGHT ); // this doesn't even seem to show up
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
//		Msg("Stopping light effect on spreer\n");
//		pVictim->RemoveEffects( EF_BRIGHTLIGHT );
	}
#endif
	virtual bool IsPermenant() { return true; };

};

//-----------------------------------------------------------------------------
// Purpose: Color all players who are against the spree war
//-----------------------------------------------------------------------------
class BuffSpreeWar : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Spree War"; }
	virtual const char *MaterialEffect() { return "spreewar"; }

	virtual int GetBuffType() { return BUFF_TYPE_HIDDEN; }
#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level) { }
	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level) { }
#else
	virtual bool ShowLevelOnHud() { return false; };
#endif
	virtual bool IsPermenant() { return true; };
};

// minions slowly regenerate health
class BuffMinion : public RepeatingBuff
{
public:
	virtual const char *Name() { return "Minion"; }
	virtual const char *MaterialEffect() { return "minion"; }

	virtual int GetBuffType() { return BUFF_TYPE_HIDDEN; }

#ifndef CLIENT_DLL
	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->TakeHealth(1,DMG_GENERIC);
	}
	virtual float GetTickInterval(int level) { return 0.333333333f; } // 3 health / second
#else
	virtual bool ShowLevelOnHud() { return false; };
#endif
	virtual bool IsPermenant() { return true; }
};


// newly created monster. immobile, fades in with fancy particles
class BuffSpawn : public RepeatingBuff
{
public:
	virtual const char *Name() { return "Spawning"; }

	virtual int GetBuffType() { return BUFF_TYPE_HIDDEN; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->SetRenderColorA(1); //(144);
		if ( pVictim->IsNPC() )
		{
			CAI_BaseNPC *pNPC = pVictim->MyNPCPointer();
			pNPC->ClearCondition(COND_NPC_UNFREEZE);
			pNPC->SetCondition(COND_NPC_FREEZE);
			pNPC->AddFlag( FL_FROZEN );
			pNPC->m_takedamage = DAMAGE_NO;
		}

		pVictim->m_iParticleEffect = pVictim->GetSpawnEffect();
	}

	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		float fadeStart = pVictim->GetBuffEndTime(BUFF_SPAWN) - FadeDuration();
		int a;
		if ( gpGlobals->curtime < fadeStart )
			a = 1;
		else
			a = max(1,min(255,(gpGlobals->curtime - fadeStart)/FadeDuration() * 255));

		if ( pVictim->IsPlayer() )
			a = max(a,127); // spawning players don't go too invisible

		pVictim->SetRenderColorA(a);
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->SetRenderColorA(255);
		if ( pVictim->IsNPC() )
		{
			CAI_BaseNPC *pNPC = pVictim->MyNPCPointer();
			pNPC->ClearCondition(COND_NPC_FREEZE);
			pNPC->SetCondition(COND_NPC_UNFREEZE);
			pNPC->RemoveFlag( FL_FROZEN );
			pNPC->m_takedamage = DAMAGE_YES;
		}
		
		pVictim->m_iParticleEffect = NO_ENTITY_EFFECT;
	}
	
	virtual float GetTickInterval(int level) { return 0.125f; }
	virtual float FadeDuration() { return 0.75f; }
#else
	virtual bool ShowLevelOnHud() { return false; };
#endif
	virtual float GetDuration(int level) { return 3.0f; }
};

extern McConVar minion_drain_scale;

class BuffMinionDrain : public RepeatingBuff
{
public:
	virtual const char *Name() { return "Active Minions"; }
	virtual int GetBuffType() { return BUFF_TYPE_NEUTRAL; }

#ifndef CLIENT_DLL
	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		if ( !pVictim->IsPlayer() )
			return;

		CHL2MP_Player *pPlayer = ToHL2MPPlayer(pVictim);
		if ( !pPlayer )
			return;
		
		int num = pPlayer->NumMinionsInFormation();
		if ( num == 0 )
			pPlayer->RemoveBuff(BUFF_MINION_DRAIN);
		else
		{
			float drain = num * minion_drain_scale.GetFloat();
			if ( pPlayer->GetAuxPower() < drain )
			{
				pPlayer->GetLimitedQuantities()->Reset(LQ_MINION);
				pPlayer->ForceMinionCount(0); // recalculate
				pPlayer->RemoveBuff(BUFF_MINION_DRAIN);
			}
			pPlayer->DrainAuxPower(drain);
		}
	}
	virtual float GetTickInterval(int level) { return 1; }
#endif
	virtual bool IsPermenant() { return true; }
};

class BuffDisabled : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Modules Disabled"; }
	virtual int GetBuffType() { return BUFF_TYPE_HIDDEN; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		for ( int i=0; i<NUM_MODULES; i++ )
		{
			Module *a = GetModule(i);

			if ( !pVictim->IsModuleActive(a) )
				continue;
			else
				pVictim->StopModule(a);
		}
	}
	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{		
		for ( int i=0; i<NUM_MODULES; i++ )
		{
			Module *a = GetModule(i);
			if(a->AutoStarts() && pVictim->HasModule(a))
				pVictim->StartModule(a);
		}
	}
#else
	virtual bool ShowLevelOnHud() { return false; };
#endif
	virtual bool IsPermenant() { return true; }
};

// all damage received is increased by 10% per level, for half a second per level
extern LEVEL_EXTERN(mod_damageamp);
extern LEVEL_EXTERN(mod_damageamp_duration);
class BuffDamageAmp : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Damage Amp"; }
	virtual int GetBuffType() { return BUFF_TYPE_BAD; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->ScaleDamageReceived( ScaleFactor(level) );
//		if ( pAttacker )
//			ClientPrint(pAttacker,HUD_PRINTNOTIFY,UTIL_VarArgs("Doing %s effect on %s\n",Name(),GetNameOf(pVictim)));
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->ScaleDamageReceived( 1.0f / ScaleFactor(level) ); // divide by the above amount
	}

	// add 60% + 10% per level
	float ScaleFactor(int level)
	{
		if ( level == 0 )
			return 1.0f;
		return LEVEL(mod_damageamp, level);
	}
#endif
	virtual float GetDuration(int level)
	{
		return LEVEL(mod_damageamp_duration, level);
	}

	virtual const char *MaterialEffect() { return "damageamp"; }
	virtual const char *ParticleEffect() { return "damageamp"; }
	virtual int GetParticleAttachments() { return BUFF_ATTACHMENT_EYES; }
	virtual bool PulsatingGlow() { return true; }
};

extern LEVEL_EXTERN(mod_cloak_suppression_time);
class DebuffSuppressCloak : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Cloak Suppressed"; }
	virtual int GetBuffType() { return BUFF_TYPE_NEUTRAL; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->EnableCloaking(false,true);
		pVictim->RemoveBuff(BUFF_SPAWN); // if you do something to suppress cloak (fire a module, etc) ... you shouldn't be invulnerable
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		Module *cloak = GetModule(CLOAK);
		if ( pVictim->IsModuleActive(cloak) )
			pVictim->EnableCloaking(true);
	}
#else
	virtual bool ShouldShowOnHud(CBaseCombatCharacter *pVictim) { return pVictim->IsModuleActive(GetModule(CLOAK)); }
	virtual bool ShowLevelOnHud() { return false; }
#endif
	virtual float GetDuration(int level) { return LEVEL(mod_cloak_suppression_time, level); }
};

extern LEVEL_EXTERN(mod_armorregen_cooldown);
class DebuffSuppressArmorRegen : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Regen Suppressed"; }
	virtual int GetBuffType() { return BUFF_TYPE_NEUTRAL; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)	{ }
	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level) { }
#else
	virtual bool ShouldShowOnHud() { return true; }
	virtual bool ShowLevelOnHud() { return false; }
#endif
	virtual float GetDuration(int level) { return LEVEL(mod_armorregen_cooldown, level); }
};

extern LEVEL_EXTERN(mod_bruteforce_buff);
extern LEVEL_EXTERN(mod_bruteforce_buff_duration);
class BuffBruteForceBuff : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Brute Force"; }
	virtual int GetBuffType() { return BUFF_TYPE_GOOD; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->ScaleDamageDealt( 1 + LEVEL(mod_bruteforce_buff, level) );
		if ( pVictim->IsPlayer() )
		{
			color32 reddish = {255,32,32,12};
			UTIL_ScreenFade( ToBasePlayer(pVictim), reddish, 0.1f, GetDuration(level)-0.05f, FFADE_OUT );
		}
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->ScaleDamageDealt( 1.0f / (1 + LEVEL(mod_bruteforce_buff, level)) ); // divide by the above amount
	}
#endif
	virtual float GetDuration(int level)
	{
		return LEVEL(mod_bruteforce_buff_duration, level);
	}
};

extern LEVEL_EXTERN(mod_weaken_damagescale);
extern LEVEL_EXTERN(mod_weaken_sprintscale);
extern LEVEL_EXTERN(mod_weaken_duration);

extern McConVar mc_player_base_walk_speed, mc_player_base_sprint_speed;
class BuffWeaken : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Weaken"; }
	virtual int GetBuffType() { return BUFF_TYPE_BAD; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->ScaleDamageDealt( LEVEL(mod_weaken_damagescale, level) );
		if ( pVictim->IsPlayer() )
		{
			CHL2MP_Player *pVictimPlayer = ToHL2MPPlayer(pVictim);

			float fullSprintSpeed = mc_player_base_sprint_speed.GetFloat();
			pVictimPlayer->SetSprintSpeed( mc_player_base_walk_speed.GetFloat() + (fullSprintSpeed-mc_player_base_walk_speed.GetFloat())*LEVEL(mod_weaken_sprintscale, level) );
		}
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->ScaleDamageDealt( 1.0f / LEVEL(mod_weaken_damagescale, level) ); // divide by the above amount
		if ( pVictim->IsPlayer() )
		{// properly recalculate sprint speed
			CHL2MP_Player *pVictimPlayer = ToHL2MPPlayer(pVictim);
			float fullSprintSpeed = mc_player_base_sprint_speed.GetFloat();
			pVictimPlayer->SetSprintSpeed( fullSprintSpeed );
		}
	}
#endif
	virtual float GetDuration(int level)
	{
		return LEVEL(mod_weaken_duration, level);
	}
};

extern LEVEL_EXTERN(mod_attrition_duration);
class BuffAttrition : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Attrition"; }
	virtual int GetBuffType() { return BUFF_TYPE_BAD; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
	}
#endif
	virtual float GetDuration(int level)
	{
		return LEVEL(mod_attrition_duration, level);
	}

	virtual const char *MaterialEffect() { return "attrition"; }
	//virtual const char *ParticleEffect() { return "attrition"; }
	virtual int GetParticleAttachments() {return BUFF_ATTACHMENT_CHEST;}
};

// restore health over time
extern LEVEL_EXTERN(mod_heald_duration);
extern LEVEL_EXTERN(mod_heald_hps);
class BuffHeald : public RepeatingBuff
{
public:
	virtual const char *Name() { return "HEALD"; }
	virtual int GetBuffType() { return BUFF_TYPE_GOOD; }

#ifndef CLIENT_DLL
	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		pVictim->TakeHealth(1,DMG_GENERIC);
		pVictim->RemoveBuff(DEBUFF_PLAGUE);
		//pVictim->RemoveBuff(BUFF_BLEED);
		//pVictim->RemoveBuff(BUFF_CROSSBOW_POISON);
	}
	virtual float GetTickInterval(int level) { return 1.0f / LEVEL(mod_heald_hps, level); }
#endif
	virtual float GetDuration(int level) { return LEVEL(mod_heald_duration, level); }

	virtual const char *ParticleEffect() { return "heald"; }
};

// restore armor over time
/*
extern LEVEL_EXTERN(mod_bioarmor_duration);
extern LEVEL_EXTERN(mod_bioarmor_aps);
class BuffBioArmor : public RepeatingBuff
{
public:
	virtual const char *Name() { return "BioArmor"; }
	virtual int GetBuffType() { return BUFF_TYPE_GOOD; }

#ifndef CLIENT_DLL
	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pVictim);
		int limit =  pOwnerPlayer->GetMaxArmor();
		pOwnerPlayer->IncrementArmorValue( LEVEL(mod_bioarmor_aps, level), limit );
	}
	virtual float GetTickInterval(int level) { return 1.0f / 10; }
#endif
	virtual float GetDuration(int level) { return LEVEL(mod_bioarmor_duration, level); }

	virtual const char *ParticleEffect() { return "bioarmor"; }
};
*/
// victim is immobilised for several seconds, and cannot act
extern LEVEL_EXTERN(mod_freezebomb_duration);
class BuffFreeze : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Frozen"; }
	virtual int GetBuffType() { return BUFF_TYPE_BAD; }
	
#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		if ( pVictim->IsOnFire() )
		{
			pVictim->IgniteLifetime(.01f);
			pVictim->Extinguish();
		}

		if ( pVictim->IsPlayer() )
		{
			CHL2MP_Player *pPlayer = ToHL2MPPlayer(pVictim);
			pPlayer->EnableControl(false);
			color32 whitish = { 112, 192, 255, 128 };
			UTIL_ScreenFade( pPlayer, whitish, GetDuration(level), 0.0f, FFADE_IN );
		}
		else if ( pVictim->IsNPC() )
		{
			CAI_BaseNPC *pNPC = pVictim->MyNPCPointer();
			pNPC->ClearCondition(COND_NPC_UNFREEZE);
			pNPC->SetCondition(COND_NPC_FREEZE);
			pNPC->AddFlag( FL_FROZEN );
			pNPC->SetLocalAngularVelocity(vec3_angle);
			pNPC->SetAbsVelocity( vec3_origin );
		}
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		if ( pVictim->IsPlayer() )
		{
			CHL2MP_Player *pPlayer = ToHL2MPPlayer(pVictim);
			pPlayer->EnableControl(true);
		}
		else if ( pVictim->IsNPC() )
		{
			CAI_BaseNPC *pNPC = pVictim->MyNPCPointer();
			pNPC->ClearCondition(COND_NPC_FREEZE);
			pNPC->SetCondition(COND_NPC_UNFREEZE);
			pNPC->RemoveFlag( FL_FROZEN );
		}
	}
#endif
	virtual float GetDuration(int level)
	{
		return LEVEL(mod_freezebomb_duration, level);
	}

	virtual const char *MaterialEffect() { return "freeze"; }
	virtual const char *ParticleEffect() { return "freeze"; }
	virtual bool PulsatingGlow() { return true; }
	virtual const char *StartSound() { return "Buff.Freeze"; }
};

/*
// player's controls are randomized for several seconds
class BuffDisorientation : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Disorientation"; }
	virtual int GetBuffType() { return BUFF_TYPE_BAD; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		if ( pVictim->IsPlayer() )
		{
			CHL2MP_Player *pPlayer = ToHL2MPPlayer(pVictim);
			pPlayer->SetDisorientated(true);
		}
		else if ( pVictim->IsNPC() )
		{
			CAI_BaseNPC *pNPC = pVictim->MyNPCPointer();
			if ( !pNPC->IsCurSchedule(SCHED_NPC_FREEZE) ) // only toggle if not frozen
				pNPC->ToggleFreeze();
		}
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		if ( pVictim->IsPlayer() )
		{
			CHL2MP_Player *pPlayer = ToHL2MPPlayer(pVictim);
			pPlayer->SetDisorientated(false);
		}
		else if ( pVictim->IsNPC() )
		{
			CAI_BaseNPC *pNPC = pVictim->MyNPCPointer();
			if ( pNPC->IsCurSchedule(SCHED_NPC_FREEZE) ) // only toggle if frozen
				pNPC->ToggleFreeze();
		}
	}
#endif

	// half a second per level
	virtual float GetDuration(int level)
	{
		return 7.0f;
	}
};
*/

int RandomlyRoundFloat(float f)
{// reversed when this function returns high / low - cos 0.25 has a 3/4 chance of generating a number higher than it, and that 3/4 chance SHOULD relate to rounding it down.
	float low = floor(f);
	float high = ceil(f);
	if ( random->RandomFloat(low,high) >= f )
		return (int)low;
	else
		return (int)high;
}

// player slowly loses health
extern LEVEL_EXTERN(mod_plague_damage);
extern LEVEL_EXTERN(mod_plague_victim_interval);
#ifndef CLIENT_DLL
extern void EmitPlague(CBaseCombatCharacter *pEmitter, CBaseCombatCharacter *pAttacker, int level);
#endif
class BuffPlague : public RepeatingBuff
{
public:
	virtual const char *Name() { return "Plague"; }
	virtual int GetBuffType() { return BUFF_TYPE_BAD; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker = NULL, int level = 1)
	{
		if ( pAttacker && pAttacker != pVictim )
		{// increase attacker's plague status buff level by 1
			pAttacker->ApplyBuff(BUFF_PLAGUE_STATUS, NULL, pAttacker->GetBuffLevel(BUFF_PLAGUE_STATUS)+1);
		}
	
		if ( !pVictim->IsPlayer() )
			return;
		ClientPrint( ToHL2MPPlayer(pVictim), HUD_PRINTTALK, pAttacker ? UTIL_VarArgs("You have been infected with %s's plague!\n", pAttacker->GetName()) : "You have been infected with the plague!\n");
	}

	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		int damage = RandomlyRoundFloat( AverageDamagePerTick(level) );

		CTakeDamageInfo info( pAttacker, pAttacker, damage, DMG_GENERIC );
		info.SetModule(PLAGUE);
		pVictim->TakeDamage( info );

		// spread the plague to anyone around me, too
		EmitPlague(pVictim, pAttacker, level);
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		Msg("Stopping plague\n");
		if ( pAttacker && pAttacker != pVictim )
		{// decrease attacker's plague status buff level by 1 - if the buff is active at all
			int level = pAttacker->GetBuffLevel(BUFF_PLAGUE_STATUS);
		
			Msg("Got attacker\n");
			if ( level > 0 )
			{
				Msg("Attacker has buff already... removing it & reapplying lower level\n");
				pAttacker->RemoveBuff(BUFF_PLAGUE_STATUS);
				pAttacker->ApplyBuff(BUFF_PLAGUE_STATUS, NULL, level-1);
			}
		}
	}
	
	float AverageDamagePerTick(int level)
	{
		return LEVEL(mod_plague_damage, level);
	}
#endif
	virtual const char *MaterialEffect() { return "plague"; }
	virtual const char *ParticleEffect() { return "plague"; }

	virtual bool IsPermenant() { return true; }
	virtual float GetTickInterval(int level) { return LEVEL(mod_plague_victim_interval, level); }
};

class BuffPlagueStatus : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Plague victims"; }

	virtual int GetBuffType() { return BUFF_TYPE_GOOD; }
#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level) {}
	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level) { }
#endif
	virtual bool IsPermenant() { return true; };
};


extern LEVEL_EXTERN(mod_adrenaline);
extern LEVEL_EXTERN(mod_adrenaline_buff);
extern LEVEL_EXTERN(mod_adrenaline_buff_duration);
extern McConVar mc_player_base_walk_speed;
class BuffAdrenaline : public MaintainedBuff
{
public:
	virtual const char *Name() { return "Adrenaline"; }
	virtual int GetBuffType() { return BUFF_TYPE_GOOD; }

//#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pVictim);
		if ( pOwnerPlayer )
			pOwnerPlayer->SetMaxSpeed( mc_player_base_walk_speed.GetInt() + (int)LEVEL(mod_adrenaline, level) );
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		CHL2MP_Player *pOwnerPlayer = ToHL2MPPlayer(pVictim);
		if ( pOwnerPlayer )
			pOwnerPlayer->SetMaxSpeed( mc_player_base_walk_speed.GetInt() );
	}
//#endif
	virtual float GetDuration(int level)
	{
		return LEVEL(mod_adrenaline_buff_duration, level);
	}
};




class BuffJetpackFlame : public RepeatingBuff
{
public:
	virtual const char *Name() { return "Jetpack"; }
	virtual const char *ParticleEffect() { return "jetpack"; }
	virtual const char *LocalParticleEffect() { return "jetpack local"; }
	virtual const char *TickSound() { return "Buff.Jetpack"; }

	virtual int GetBuffType() { return BUFF_TYPE_HIDDEN; }

#ifndef CLIENT_DLL
//#define IMPULSE_PER_SECOND	775
	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
/*		// this keeps thrust constant (rather than a pulse every 1/10 sec),
		// and also keeps it constant no matter the framerate
		float speed = IMPULSE_PER_SECOND * gpGlobals->frametime;
		pVictim->SetGroundEntity(NULL);
		pVictim->ApplyAbsVelocityImpulse(Vector(0,0,speed));

		// for the jetpack, it would be nice to have this predicted client-side*/
	}
	
	virtual float GetTickInterval(int level) { return 0.20f; } // tick is now for the sound only
#else
	virtual bool ShowLevelOnHud() { return false; };
#endif

	virtual bool IsPermenant() { return true; }
};


extern LEVEL_EXTERN(mod_incendiary_dps);
extern LEVEL_EXTERN(mod_incendiary_duration);
extern LEVEL_EXTERN(mod_incendiary_tick);
class BuffBurn : public RepeatingBuff
{
public:
	virtual const char *Name() { return "Burning"; }
	virtual int GetBuffType() { return BUFF_TYPE_BAD; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		if ( pVictim->IsBuffActive(DEBUFF_FREEZE) )
			pVictim->RemoveBuff(DEBUFF_FREEZE);

		pVictim->Ignite(GetDuration(level),false);
	}

	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		CTakeDamageInfo info( pAttacker, pAttacker, DamagePerTick(level), DMG_BURN | DMG_DIRECT );
		info.SetModule(INCENDIARY_GRENADE);
		pVictim->TakeDamage( info );
	}

	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker, int level)
	{
		if ( pVictim->IsOnFire() )
		{
			pVictim->IgniteLifetime(.01f);
			pVictim->Extinguish();
		}
	}

	float DamagePerTick(int level)
	{
		return LEVEL(mod_incendiary_dps, level) * LEVEL(mod_incendiary_tick, level);
	}

	virtual float GetTickInterval(int level) { return LEVEL(mod_incendiary_tick, level); }
#endif
	virtual float GetDuration(int level) { return LEVEL(mod_incendiary_duration, level); }
};

// all abilities must be added to this list, and also to the enum in the header
void CreateBuffs()
{
	(new BuffSpree())->SetBuffIndex(BUFF_SPREE);
	(new BuffSpreeWar())->SetBuffIndex(BUFF_SPREE_WAR);
	(new BuffMinion())->SetBuffIndex(BUFF_MINION);
	(new BuffSpawn())->SetBuffIndex(BUFF_SPAWN);
	(new BuffDisabled())->SetBuffIndex(BUFF_DISABLED);
	(new BuffDamageAmp())->SetBuffIndex(DEBUFF_DAMAGE_AMP);
	(new BuffBruteForceBuff())->SetBuffIndex(BUFF_BRUTEFORCE);
	(new BuffWeaken())->SetBuffIndex(DEBUFF_WEAKEN);
	(new BuffHeald())->SetBuffIndex(BUFF_HEALD);
	(new BuffAttrition())->SetBuffIndex(DEBUFF_ATTRITION);
	(new BuffFreeze())->SetBuffIndex(DEBUFF_FREEZE);
	(new BuffPlague())->SetBuffIndex(DEBUFF_PLAGUE);
	(new BuffPlagueStatus())->SetBuffIndex(BUFF_PLAGUE_STATUS);
	(new BuffJetpackFlame())->SetBuffIndex(BUFF_JETPACK_FLAME);
	(new BuffBurn())->SetBuffIndex(DEBUFF_BURN);
	(new BuffMinionDrain())->SetBuffIndex(BUFF_MINION_DRAIN);
	(new BuffAdrenaline())->SetBuffIndex(BUFF_ADRENALINE);
	(new DebuffSuppressCloak())->SetBuffIndex(DEBUFF_SUPPRESS_CLOAK);
	// (new BuffBioArmor())->SetBuffIndex(BUFF_BIOARMOR);
	(new DebuffSuppressArmorRegen())->SetBuffIndex(DEBUFF_SUPPRESS_ARMORREGEN);

	for ( int i=0; i<NUM_BUFFS; i++ )
		GetBuff(i)->Init();
}

void DeleteBuffs()
{
	for ( int i=0; i<NUM_BUFFS; i++ )
		delete GetBuff(i);
}

void Buff::SetBuff(int i, Buff *a)
{
	g_Buffs[i] = a;
}

Buff *GetBuff(int num)
{
	return g_Buffs[num];
}

Buff *GetBuff(const char *name)
{
	for ( int i=0; i<NUM_BUFFS; i++ )
		if ( FStrEq(g_Buffs[i]->GetName(),name) )
			return g_Buffs[i];
	
	return NULL;
}

void Buff::Precache()
{
#ifndef CLIENT_DLL
	if ( GetParticleEffect() != NULL )
		PrecacheParticleSystem( GetParticleEffect() );

	if ( GetLocalParticleEffect() != NULL )
		PrecacheParticleSystem( GetLocalParticleEffect() );

	if ( GetStartSound() != NULL )
		CBaseEntity::PrecacheScriptSound( GetStartSound() );
	
	if ( GetTickSound() != NULL )
		CBaseEntity::PrecacheScriptSound( GetTickSound() );
#endif
};

void Buff::Init()
{
	Q_snprintf(m_szName,sizeof(m_szName), Name());

	const char *temp = ParticleEffect();
	m_bHasParticleEffect = temp != NULL;	
	if ( m_bHasParticleEffect )
		Q_snprintf(m_szParticleEffect,sizeof(m_szParticleEffect), temp);
//	delete [] temp;
	
	temp = LocalParticleEffect();
	m_bHasLocalParticleEffect = temp != NULL;
	if ( m_bHasLocalParticleEffect )
		Q_snprintf(m_szLocalParticleEffect,sizeof(m_szLocalParticleEffect), temp);
//	delete [] temp;

	temp = MaterialEffect();
	m_bHasMaterialEffect = temp != NULL;	
	if ( m_bHasMaterialEffect )
		Q_snprintf(m_szMaterialEffect,sizeof(m_szMaterialEffect), VarArgs("overlays/%s",temp));
//	delete [] temp;

	temp = StartSound();
	m_bHasStartSound = temp != NULL;
	if ( m_bHasStartSound )
		Q_snprintf(m_szStartSound,sizeof(m_szStartSound), temp);
//	delete [] temp;
		
	temp = TickSound();
	m_bHasTickSound = temp != NULL;
	if ( m_bHasTickSound )
		Q_snprintf(m_szTickSound,sizeof(m_szTickSound), temp);
//	delete [] temp;

#ifdef CLIENT_DLL
	temp = GetName();
	g_pVGuiLocalize->ConvertANSIToUnicode(temp,m_wszNameUnicode,sizeof(m_wszNameUnicode));
//	delete temp;
#endif
}

#ifndef CLIENT_DLL
void CC_Afflict( const CCommand &args )
{
	CHL2MP_Player *pCommandPlayer = ToHL2MPPlayer(UTIL_GetCommandClient());

	if ( args.ArgC() < 3 )
	{
		ClientPrint(pCommandPlayer,HUD_PRINTCONSOLE,"Syntax: afflict <player name> <affliction index> <level>\n");
		return;
	}
	
	int index = atoi(args[2]);
	if ( index < 0 || index >=  NUM_BUFFS)
	{
		ClientPrint(pCommandPlayer,HUD_PRINTCONSOLE,"Invalid amount\n");
		return;
	}
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer || !FStrEq(args[1],pPlayer->GetPlayerName()) )
			continue;

		pPlayer->ApplyBuff(index);

		ClientPrint(pCommandPlayer,HUD_PRINTCONSOLE,"Success\n");
		return;
	}
	ClientPrint(pCommandPlayer,HUD_PRINTCONSOLE,"Failed\n");
}


static ConCommand cc_afflict("afflict", CC_Afflict, "Add an affliction to named player\nafflict <player name> <affliction index> <level>", FCVAR_CHEAT);
#endif

#ifndef CLIENT_DLL
void CC_DeAfflict( const CCommand &args )
{
	CHL2MP_Player *pCommandPlayer = ToHL2MPPlayer(UTIL_GetCommandClient());

	if ( args.ArgC() < 3 )
	{
		ClientPrint(pCommandPlayer,HUD_PRINTCONSOLE,"Syntax: deafflict <player name> <affliction index>\n");
		return;
	}
	
	int index = atoi(args[2]);
	if ( index < 0 || index >=  NUM_BUFFS)
	{
		ClientPrint(pCommandPlayer,HUD_PRINTCONSOLE,"Invalid amount\n");
		return;
	}
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer || !FStrEq(args[1],pPlayer->GetPlayerName()) )
			continue;

		pPlayer->RemoveBuff(index);

		ClientPrint(pCommandPlayer,HUD_PRINTCONSOLE,"Success\n");
		return;
	}
	ClientPrint(pCommandPlayer,HUD_PRINTCONSOLE,"Failed\n");
}


static ConCommand cc_deafflict("deafflict", CC_DeAfflict, "Removes an affliction to named player\ndeafflict <player name> <affliction index>", FCVAR_CHEAT);
#endif