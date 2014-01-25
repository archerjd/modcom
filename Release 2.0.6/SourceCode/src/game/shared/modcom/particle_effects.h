#ifndef ENTITY_EFFECTS_H
#define ENTITY_EFFECTS_H
#pragma once

// for attaching particle effects to entities, without using afflictions (which not all will work with)
// these will work for all base combat characters

enum entity_effect_t
{
	NO_ENTITY_EFFECT = 0,
	MAGD_EFFECT,
	FREEZE_EFFECT,
	EMBER_EFFECT,
	SPAWN_HUMAN,
	SPAWN_SMALL,
	SPAWN_MEDIUM,
	SPAWN_LARGE,
	HUNTER_FLECHETTE,
	HUNTER_FLECHETTE_STRIDERBUSTER,

	LAST_ENTITY_EFFECT,
};

void SetupParticleEffects();

#endif