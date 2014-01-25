
/*
a class representing an "affliction" of an arbitrary type.
These are applied to other players by abilities such as
DamageAmp and Plague, and come in two types:
TickingBuff and ConstantBuff
They are time-limited!

They behave very similarly to abilities, and remember what
player started them off, and with what ability
*/

#ifndef BUFF_H
#define BUFF_H
#pragma once

#include "Color.h"
#ifndef CLIENT_DLL
#include "cbase.h"
#endif

class CHL2MP_Player;
class CBaseCombatCharacter;

enum // list of all afflictions
{
	BUFF_NONE = -1,
	BUFF_SPREE = 0, // I'm on a spree, so I'm colored funny
	BUFF_SPREE_WAR, // we're all against someone on a spree
	
	BUFF_SPAWN, // fade in, particles, immobile
	BUFF_MINION, // I'm a pet, so should heal slowly over time... and be blue
	BUFF_MINION_DRAIN, // the aux drain minions cause on their master
	DEBUFF_BURN, // affliction that basically calls Ignite to set target on fire
		
	BUFF_DISABLED, //all modules turned off
	DEBUFF_SUPPRESS_CLOAK, // cloak is suppressed (no aux cost / effect) but not disabled, for the duration 
	DEBUFF_SUPPRESS_ARMORREGEN, // armor regen is suppressed for a pirod of time

	// proper 'afflictions' start here
	DEBUFF_DAMAGE_AMP,
	BUFF_BRUTEFORCE, // ok this is good rather than bad
	DEBUFF_WEAKEN,
	DEBUFF_PLAGUE,
	BUFF_PLAGUE_STATUS, // buff used to show number of plague victims
	
	BUFF_HEALD,
	// BUFF_BIOARMOR,
	BUFF_ADRENALINE,
	DEBUFF_ATTRITION,

	DEBUFF_FREEZE,
	BUFF_JETPACK_FLAME,

	NUM_BUFFS, // this is used for the array size
};

#define MAX_BUFF_LEVEL 63 // don't let any buff get to a higher level than this
#define BITS_FOR_MAX_BUFF_LEVEL 6 // 2^(MAX_BUFF_LEVEL+1)

enum
{
	BUFF_TYPE_NEUTRAL = 0, // neutral effects cannot be 
	BUFF_TYPE_GOOD, // will be displayed in a positive way. Buff stealing would steal only these.
	BUFF_TYPE_BAD, // will be displayed in a bad way. Cleansing would remove only these.
	BUFF_TYPE_HIDDEN, // like neutral, but won't even be displayed

	NUM_BUFF_TYPES,
};

// affliction particle attachments
#define BUFF_ATTACHMENT_EYES (1<<0)
#define BUFF_ATTACHMENT_CHEST (1<<1)
#define BUFF_ATTACHMENT_LEFT_HAND (1<<2)
#define BUFF_ATTACHMENT_RIGHT_HAND (1<<3)

class Buff;
#define AFF_STR_LEN	32

static int GetNumBuffs() { return NUM_BUFFS; }
extern void CreateBuffs();
extern void DeleteBuffs();
extern Buff *GetBuff(int num);
extern Buff *GetBuff(const char *name);

class Buff
{
public:
	void Init();

	const char *GetName() { return m_szName; }
	virtual const char *Name() = 0;
	

	virtual void Precache();

	virtual int GetBuffType() { return BUFF_TYPE_NEUTRAL; }
	
#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker = NULL, int level = 1 ) {};
	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker = NULL, int level = 1 ) {};
	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker = NULL, int level = 1 ) {};

	virtual bool Ticks() = 0;
	virtual bool SoundTicks() { return false; }
	virtual float GetTickInterval(int level) { return 1.0f; }
#else
	wchar_t *GetDisplayNameUnicode() { return m_wszNameUnicode; }
	virtual bool ShouldShowOnHud(CBaseCombatCharacter *pVictim) { return GetBuffType() != BUFF_TYPE_HIDDEN; }
	virtual bool ShowLevelOnHud() { return true; };
#endif
	virtual float GetDuration(int level) { return 10.0f; }
	virtual bool IsPermenant() { return false; };

	void SetBuffIndex(int i) { m_iBuffIndex = i; SetBuff(i,this); }
	int GetBuffIndex() { return m_iBuffIndex; }

	const char *GetParticleEffect() { if ( m_bHasParticleEffect ) return m_szParticleEffect; return NULL; }
	const char *GetMaterialEffect() { if ( m_bHasMaterialEffect ) return m_szMaterialEffect; return NULL; }
	const char *GetLocalParticleEffect() { if ( m_bHasLocalParticleEffect ) return m_szLocalParticleEffect; return NULL; } // play this effect instead when active on the local player
	const char *GetStartSound() { if ( m_bHasStartSound ) return m_szStartSound; return NULL; }
	const char *GetTickSound() { if ( m_bHasTickSound ) return m_szTickSound; return NULL; }
	
	virtual const char *ParticleEffect() { return NULL; }
	virtual const char *MaterialEffect() { return NULL; }
	virtual int GetParticleAttachments() { return 0; } // no attachments used by default
	virtual bool PulsatingGlow() { return false; }
	virtual const char *LocalParticleEffect() { return ParticleEffect(); }
	virtual const char *StartSound() { return NULL; }
	virtual const char *TickSound() { return NULL; }

private:
	static void SetBuff(int i, Buff *a);
	int m_iBuffIndex;

	bool m_bHasParticleEffect, m_bHasLocalParticleEffect, m_bHasMaterialEffect, m_bHasStartSound, m_bHasTickSound;
	char m_szName[AFF_STR_LEN];
	char m_szParticleEffect[AFF_STR_LEN];
	char m_szMaterialEffect[AFF_STR_LEN];
	char m_szLocalParticleEffect[AFF_STR_LEN];
	char m_szStartSound[AFF_STR_LEN];
	char m_szTickSound[AFF_STR_LEN];

#ifdef CLIENT_DLL
	wchar_t m_wszNameUnicode[AFF_STR_LEN];
#endif
};

// eg plague, or some Damage over Time affliction. Possibly Curse too.
class RepeatingBuff : public Buff
{
public:
#ifndef CLIENT_DLL
	virtual void DoEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker = NULL, int level = 1 ) = 0;
	virtual bool Ticks() { return true; }
	virtual float GetTickInterval(int level) = 0;
#endif
};

// eg Damage Amplifier - it starts and stops (by setting a value, really), and does no tick effect
class MaintainedBuff : public Buff
{
public:
#ifndef CLIENT_DLL
	virtual void StartEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker = NULL, int level = 1 ) = 0;
	virtual void StopEffect(CBaseCombatCharacter *pVictim, CBaseCombatCharacter *pAttacker = NULL, int level = 1 ) = 0;	
	virtual bool Ticks() { return true; }
	virtual float GetTickInterval(int level) { return 1.0f; }
#endif
};

#endif
