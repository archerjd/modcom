/*
a class representing a "module" of arbitary type.
modules should not inherit directly from the Module BaseClass, but instead
from one of its child classes:
ActiveModule, PassiveModule, MaintainedModule, RepeatingModule, or PassiveRepeatingModule

these classes are Static, because although we need to access from the client, and so would expect to
network them in the player class, I don't think its possible to network classes except by EHANDLE,
and as these abilities aren't (and shouldn't be) game entities, they don't have EHANDLES!

so instead, we'll assign each module an index, and network the indexes of abilities (and their levels)
that players have then we just get that index in the module list, and work with that.

Description stuff, death / suicide message stuff, and GetModule(const char *name) are still unsafe.

*/

#ifndef MODULE_H
#define MODULE_H
#pragma once

enum // list of all abilities
{
	NO_MODULE = -1,

	// keep recharge at the top
	RECHARGE,

	VITALITY,
	AUX_CAPACITY,
	ARMOR_CAPACITY,

	STRENGTH,
	BRUTEFORCE,
	CRITS,
	CLIP_SIZE,
	AMMOREGEN,
	
/*	
	PIERCING_RESIST,
	BULLET_RESIST,
	CRUSH_RESIST,
	POISON_RESIST,
	BLAST_RESIST,
	ENERGY_RESIST,
	FIRE_RESIST,
*/
	CLOAK,
	JETPACK,
	TELEPORT,
	LONG_JUMP,
	ADRENALINE,
	
	ENERGYBALL,
	FLECHETTE,
	POISON_GRENADE,
	FREEZE_GRENADE,
	INCENDIARY_GRENADE,
	MIRV,

	HEALD,
	ARMORREGEN,
	DAMAGE_AMP,
//	ATTRITION,
	WEAKEN,
	PLAGUE,
	SHOCKWAVE,

	LASERS,
	TURRET,
	MAGMINE,
//	REPMINE,
	CROW,

	MINION_ZOMBIE,
	MINION_FASTZOMBIE,
	MINION_ANTLION,
	MINION_ANTLION_WORKER,
//	MINION_FASTHEADCRAB,
	MINION_VORTIGAUNT,
	MINION_MANHACK,

	NUM_MODULES, // this is used for the array size
};

enum
{
	MODULE_CATEGORY_CORE,
	MODULE_CATEGORY_WEAPON,
	MODULE_CATEGORY_MOBILITY,
	MODULE_CATEGORY_PROJECTILES,
	MODULE_CATEGORY_TARGET_EFFECTS,
	MODULE_CATEGORY_DEPLOYABLES,
	MODULE_CATEGORY_MINIONS,
	NUM_MODULE_CATEGORIES,
};

#define MAX_DESC_LENGTH		128
#define MAX_LVL_DESC_LENGTH	64
class Module;
class Buff;
class CHL2MP_Player;

static int GetNumModules() { return NUM_MODULES; }
extern void CreateModules();
extern void DeleteModules();
extern Module *GetModule(int num);
extern Module *GetModule(const char *name);

#ifndef CLIENT_DLL
// AI usage flags
#define AI_DOESNT_USE (1<<0)
#define PASSIVE (1<<1)
#define TRIGGERED_BUFF (1<<2)
#define TRIGGERED_MOVEMENT (1<<3)
#define PROJECTILE_BALLISTIC (1<<4)
#define PROJECTILE_DIRECT (1<<5)    // do we need this, or can we hack the ballistic calculation somehow... ?
#define INSTANT_TARGET_EFFECT (1<<6) // include vort-beams in this category
#define DEPLOYABLE (1<<7)
#define MINION (1<<8)
#define MAINTAINED_EFFECT (1<<9)
#endif

// this is required for the multiple inheritance of TenLevelModule and SingleLevelModule, see end of file
class IPurchasable
{
public:
	virtual int GetUpgradeCost() = 0;
	virtual int GetMaxLevel() = 0;
};

#define MODULE_STR_LEN	32

// this is the proper "base class" 
class Module : virtual public IPurchasable
{
public:
	void Init(const char *cmdName);
	
	const char *GetDisplayName() { return m_szDisplayName; }
	const char *GetCmdName() { return m_szCommandName; }
	const char *GetDisplayType() { return m_szDisplayType; }
	
	virtual const char *DisplayName() = 0;
	virtual const char *DisplayType() = 0;
	
	virtual bool IsPurchasable() { return true; }
	virtual bool IsSuppressed(CBaseCombatCharacter *pOwner);
	
	virtual void Precache();
#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level = 1) {};
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level = 1) { return false; }
	virtual void FailEffect(CBaseCombatCharacter *pOwner, int level = 1) {};
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level = 1) {};
	virtual Buff *GetAssociatedBuff() { return NULL; }

	virtual const char *GetDeathMessage(CHL2MP_Player *pKiller,CHL2MP_Player *pVictim);
	virtual const char *GetSuicideMessage(CHL2MP_Player *pKiller);
	virtual int GetAIDescription() { return AI_DOESNT_USE; }
#else
	virtual wchar_t *GetDataTableValue1(int level) { return L""; }
	virtual wchar_t *GetDataTableValue2(int level) { return L""; }
	virtual wchar_t *GetDataTableValue3(int level) { return L""; }
	virtual wchar_t *GetDataTableValue4(int level) { return L""; }
	virtual wchar_t *GetDataTableValue5(int level) { return L""; }
	virtual wchar_t *GetDataTableValue6(int level) { return L""; }
	virtual wchar_t *GetDataTableValue7(int level) { return L""; }
	virtual wchar_t *GetDataTableValue8(int level) { return L""; }
	virtual wchar_t *GetDataTableValue9(int level) { return L""; }
	virtual wchar_t *GetDataTableValue10(int level) { return L""; }
	virtual wchar_t *GetDataTableRowName1() { return L""; }
	virtual wchar_t *GetDataTableRowName2() { return L""; }
	virtual wchar_t *GetDataTableRowName3() { return L""; }
	virtual wchar_t *GetDataTableRowName4() { return L""; }
	virtual wchar_t *GetDataTableRowName5() { return L""; }
	virtual wchar_t *GetDataTableRowName6() { return L""; }
	virtual wchar_t *GetDataTableRowName7() { return L""; }
	virtual wchar_t *GetDataTableRowName8() { return L""; }
	virtual wchar_t *GetDataTableRowName9() { return L""; }
	virtual wchar_t *GetDataTableRowName10() { return L""; }
	
	wchar_t *GetDataTableValue(int param, int level)
	{
		switch ( param )
		{
			case 1: return GetDataTableValue1(level);
			case 2: return GetDataTableValue2(level);	
			case 3: return GetDataTableValue3(level);
			case 4: return GetDataTableValue4(level);
			case 5: return GetDataTableValue5(level);
			case 6: return GetDataTableValue6(level);
			case 7: return GetDataTableValue7(level);
			case 8: return GetDataTableValue8(level);
			case 9: return GetDataTableValue9(level);
			case 10: return GetDataTableValue10(level);
			default: return L"";
		}
	}
	
	wchar_t *GetDataTableRowName(int param)
	{
		switch ( param )
		{
			case 1: return GetDataTableRowName1();
			case 2: return GetDataTableRowName2();
			case 3: return GetDataTableRowName3();
			case 4: return GetDataTableRowName4();
			case 5: return GetDataTableRowName5();
			case 6: return GetDataTableRowName6();
			case 7: return GetDataTableRowName7();
			case 8: return GetDataTableRowName8();
			case 9: return GetDataTableRowName9();
			case 10: return GetDataTableRowName10();
			default: return L"";
		}
	}
	
	wchar_t *GetDescription(int level);
	wchar_t *GetDisplayNameUnicode() { return m_wszNameUnicode; }
	
	const char *GetIconName() { return m_szIconName; } // get icon name within the vgui folder
	const char *GetFullIconName() { return m_szFullIconName; } // get icon relative to materials directory
	
	virtual bool ShouldShowDrainOnHud() { return false; } 
	virtual bool ShouldShowLevel0OnDataTables() { return false; }
#endif
	
	void SetModuleIndex(int index, int category) { m_iModuleIndex = index; m_iCategory = category, SetModule(index,this); }
	int GetModuleIndex() { return m_iModuleIndex; }
	int GetCategory() { return m_iCategory; }

	virtual bool IsToggled() { return false; }
	virtual bool IsToggledByHoldingKey() { return false; }
	virtual bool AllowManualStop() { return true; }
	virtual bool AutoStarts() { return false; }
	virtual bool Ticks() { return false; }
	virtual bool ShouldTurnOffBeforeUpgrading() { return true; }
	virtual bool ShouldShowCooldown() = 0;
	virtual bool ShouldShowCastTime() = 0;

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) = 0; // how much power does this module drain to "use"
	virtual bool UsesSmoothAuxDrain() { return false; }
	virtual float GetCooldown(int level) = 0; // how long after its' stopped before it can be used again?
	virtual float GetTickInterval(int level) { return 0; }// for maintained abilities only - how long between ticks?
	virtual float GetInitialCooldown(int level) { return 0; } // how long after buying/spawning before it can be first used?
	virtual float GetCastTime(int level) { return 0.0f; }
	virtual float GetMaxDuration(int level) { return 0.0f; }

//	these now return an empty string rather than null ... 
	const char *GetParticleEffect() { if ( m_bHasParticleEffect ) return m_szParticleEffect; return NULL; }
	const char *GetLocalParticleEffect() { if ( m_bHasLocalParticleEffect ) return m_szLocalParticleEffect; return NULL; }
	virtual Vector ParticleEffectControlPointOffset(int level) { return vec3_origin; }
	const char *GetCastSound() { if ( m_bHasCastSound ) return m_szCastSound; return NULL; }
	const char *GetUseSound() { if ( m_bHasUseSound ) return m_szUseSound; return NULL; }
	const char *GetTickSound() { if ( m_bHasTickSound ) return m_szTickSound; return NULL; }
	
	virtual const char *ParticleEffect() { return NULL; }
	virtual const char *LocalParticleEffect() { return ParticleEffect(); }
	virtual const char *CastSound() { return NULL; }
	virtual const char *UseSound() { return NULL; }
	virtual const char *TickSound() { return NULL; }

private:
	static void SetModule(int i, Module *a);
	int m_iModuleIndex, m_iCategory;
	
	char m_szDisplayName[MODULE_STR_LEN];
	char m_szCommandName[MODULE_STR_LEN];
	char m_szDisplayType[MODULE_STR_LEN];
	
	bool m_bHasParticleEffect, m_bHasLocalParticleEffect, m_bHasCastSound, m_bHasUseSound, m_bHasTickSound;
	char m_szParticleEffect[MODULE_STR_LEN];
	char m_szLocalParticleEffect[MODULE_STR_LEN];
	char m_szCastSound[MODULE_STR_LEN];
	char m_szUseSound[MODULE_STR_LEN];
	char m_szTickSound[MODULE_STR_LEN];
	
#ifdef CLIENT_DLL
	char m_szIconName[64];
	char m_szFullIconName[64];

	wchar_t m_wszNameUnicode[MODULE_STR_LEN];
#endif
};


// these are used manually, and have an "instant" effect (ie that is not maintained)
// example: magmine
class InstantModule : public Module
{
public:
	virtual const char *DisplayType() { return "Instant"; }

	virtual bool ShouldShowCooldown() { return true; }
	virtual bool ShouldShowCastTime() { return true; }

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level = 1 ) = 0;
#endif
};


// these have a constant effect on the player, and drain no power
// example: strength
class PassiveModule : public Module
{
public:
	virtual const char *DisplayType() { return "Passive"; }

	virtual bool IsToggled() { return true; }
	virtual bool AutoStarts() { return true; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return 0; }
	virtual float GetCooldown(int level) { return 1.0f; }
	
	virtual bool ShouldShowCooldown() { return false; }
	virtual bool ShouldShowCastTime() { return false; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level = 1 ) = 0;
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level = 1 ) = 0;
#endif
};


// similar to passive, these however do drain power, so require manual activation / deactivation
// example: critical hits
class MaintainedModule : public Module
{
public:
	virtual const char *DisplayType() { return "Toggled"; }

	virtual bool IsToggled() { return true; }

	virtual bool ShouldShowCooldown() { return true; }
	virtual bool ShouldShowCastTime() { return true; }

#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pOwner, int level = 1 ) = 0;
	virtual void StopEffect(CBaseCombatCharacter *pOwner, int level = 1 ) = 0;
#else
	virtual bool ShouldShowDrainOnHud() { return true; }
#endif
};



// these are used manually, and continue to "tick" until out of power, or turned off
// example: cloak, plague
class ToggledRepeatingModule : public Module
{
public:
	virtual const char *DisplayType() { return "Toggled"; }

	virtual bool Ticks() { return true; }
	virtual bool IsToggled() { return true; }

	virtual bool ShouldShowCooldown() { return true; }
	virtual bool ShouldShowCastTime() { return false; }
	
	virtual float GetTickInterval(int level) = 0;

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level = 1 ) = 0;
#else
	virtual bool ShouldShowDrainOnHud() { return true; }
#endif
};

// these tick, and are manually turned on by holding down the key, and off by releasing it
// example: jetpack
class MaintainedRepeatingModule : public ToggledRepeatingModule
{
public:
	virtual bool IsToggledByHoldingKey() { return true; }
	virtual float GetTickInterval(int level) = 0;
};

// these are on all the time, but exert their effect through a repeating "tick" - they cost no power.
// example: Regeneration, Plague
class PassiveRepeatingModule : public Module
{
public:
	virtual const char *DisplayType() { return "Passive"; }

	virtual bool Ticks() { return true; }
	virtual bool IsToggled() { return true; }
	virtual bool AutoStarts() { return true; }

	virtual float GetAuxDrain(CBaseCombatCharacter *pOwner, int level) { return 0; }
	
	virtual float GetCooldown(int level) { return 0; }
	virtual float GetTickInterval(int level) = 0;

	virtual bool ShouldShowCooldown() { return true; }
	virtual bool ShouldShowCastTime() { return false; }

#ifndef CLIENT_DLL
	virtual bool DoEffect(CBaseCombatCharacter *pOwner, int level = 1 ) = 0;
#endif
};


// the following classes manage levelling up, in order to have things standardised, but also
// to not force particular classes of module (eg special) into the same level up model

class TenLevelModule : virtual public IPurchasable
{
public:
	virtual int GetUpgradeCost() { return 1; }
	virtual int GetMaxLevel() { return 10; }
};

class SingleLevelModule : virtual public IPurchasable
{
public:
	virtual int GetUpgradeCost() { return 4; }
	virtual int GetMaxLevel() { return 1; }
};

class CheapSingleLevelModule : virtual public IPurchasable
{
public:
	virtual int GetUpgradeCost() { return 3; }
	virtual int GetMaxLevel() { return 1; }
};

#endif MODULE_H
