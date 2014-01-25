#include "cbase.h"
#include "particle_effects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *g_szEntityEffectNames[LAST_ENTITY_EFFECT];
float g_EntityEffectDurations[LAST_ENTITY_EFFECT];

void SetupParticleEffects()
{
	g_szEntityEffectNames[0] = "NONE";
	g_szEntityEffectNames[MAGD_EFFECT] = "magd";
	g_szEntityEffectNames[FREEZE_EFFECT] = "freeze";
	g_szEntityEffectNames[EMBER_EFFECT] = "ember";
	g_szEntityEffectNames[SPAWN_HUMAN] = "spawn_human";
	g_szEntityEffectNames[SPAWN_SMALL] = "spawn_small";
	g_szEntityEffectNames[SPAWN_MEDIUM] = "spawn_medium";
	g_szEntityEffectNames[SPAWN_LARGE] = "spawn_large";
	g_szEntityEffectNames[HUNTER_FLECHETTE] = "hunter_flechette_trail";
	g_szEntityEffectNames[HUNTER_FLECHETTE_STRIDERBUSTER] = "hunter_flechette_trail_striderbuster";
	
	g_EntityEffectDurations[0] = -1;
	g_EntityEffectDurations[MAGD_EFFECT] = -1;
	g_EntityEffectDurations[FREEZE_EFFECT] = -1;
	g_EntityEffectDurations[EMBER_EFFECT] = -1;
	g_EntityEffectDurations[SPAWN_HUMAN] = -1;
	g_EntityEffectDurations[SPAWN_SMALL] = -1;
	g_EntityEffectDurations[SPAWN_MEDIUM] = -1;
	g_EntityEffectDurations[SPAWN_LARGE] = -1;
	g_EntityEffectDurations[HUNTER_FLECHETTE] = -1;
	g_EntityEffectDurations[HUNTER_FLECHETTE_STRIDERBUSTER] = -1;
}