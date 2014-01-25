#include "cbase.h"
#include "mcconvar.h"
#include "modcom/mc_shareddefs.h"

#ifndef CLIENT_DLL
#include "hl2mp/hl2mp_gamerules.h"
extern void gamemode_hoarder_min_players_changed(IConVar *var, const char *pOldValue, float flOldValue);
#endif

// having all these definitions in this file means that when the web convar list is checked when doing a build, only one code file has to be searched, rather than the whole solution

// shared convars
// E.g.
// LEVEL_VARIABLE(name, flags, base, scale, power, description)
// this macro to be used for defining a multi-level variable by means of 3 convars, to be accessed through the LEVEL macro. base + scale * level ^ power is the formula used for all of these.

LEVEL_VARIABLE(mc_levelexp, FCVAR_NOTIFY | FCVAR_REPLICATED, "1000", "1140", "1", "experience required to progress through level");

LEVEL_VARIABLE(mod_adrenaline, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "40", "1", "Speed per level");
LEVEL_VARIABLE(mod_adrenaline_buff_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0", "0", "duration of the Adrenaline buff");
LEVEL_VARIABLE(mod_adrenaline_buff_stacks, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.5", "1", "max number of times the Adrenaline buff can stack");

LEVEL_VARIABLE(mod_ammoregen_ar2, FCVAR_NOTIFY | FCVAR_REPLICATED, "8", "0", "0", "AR2 ammo given every tick of Ammo Regeneration");
LEVEL_VARIABLE(mod_ammoregen_buckshot, FCVAR_NOTIFY | FCVAR_REPLICATED, "2", "0", "0", "shotgun ammo given every tick of Ammo Regeneration");
LEVEL_VARIABLE(mod_ammoregen_interval, FCVAR_NOTIFY | FCVAR_REPLICATED, "27.5", "-2.5", "1", "interval between ticks of Ammo Regeneration");
LEVEL_VARIABLE(mod_ammoregen_pistol, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0", "0", "pistol ammo given every tick of Ammo Regeneration");
LEVEL_VARIABLE(mod_ammoregen_smg1, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0", "0", "SMG ammo given every tick of Ammo Regeneration");

LEVEL_VARIABLE(mod_antlion_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "cooldown of Antlion minion module");
LEVEL_VARIABLE(mod_antlion_dmg_jump, FCVAR_NOTIFY | FCVAR_REPLICATED, "15", "1", "1", "antlion minion jump-on damage");
LEVEL_VARIABLE(mod_antlion_dmg_swipe, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "1", "1", "antlion minion swipe damage");
LEVEL_VARIABLE(mod_antlion_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "45", "0", "0", "AUX drain from using Antlion minion module");
LEVEL_VARIABLE(mod_antlion_health, FCVAR_NOTIFY | FCVAR_REPLICATED, "50", "10", "1", "antlion minion health");

LEVEL_VARIABLE(mod_antlionworker_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "cooldown of Antlion Worker minion module");
LEVEL_VARIABLE(mod_antlionworker_dmg_burst, FCVAR_NOTIFY | FCVAR_REPLICATED, "15", "1.3", "1", "antlion worker minion death explosion/burst damage");
LEVEL_VARIABLE(mod_antlionworker_dmg_jump, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0.5", "1", "antlion worker minion jump-on damage");
LEVEL_VARIABLE(mod_antlionworker_dmg_spit, FCVAR_NOTIFY | FCVAR_REPLICATED, "4", "0.65", "1", "antlion worker minion spit damage");
LEVEL_VARIABLE(mod_antlionworker_dmg_swipe, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "1", "1", "antlion worker minion swipe damage");
LEVEL_VARIABLE(mod_antlionworker_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "45", "0", "0", "AUX drain from using Antlion Worker minion module");
LEVEL_VARIABLE(mod_antlionworker_health, FCVAR_NOTIFY | FCVAR_REPLICATED, "25", "5", "1", "antlion worker minion health");

LEVEL_VARIABLE(mod_armorcap, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "10", "1", "armor capacity increase with each level of armor capacity");

LEVEL_VARIABLE(mod_armorregen_interval, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "interval of armor regeneration");
LEVEL_VARIABLE(mod_armorregen_amount, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "amount of armor restored per interval");
LEVEL_VARIABLE(mod_armorregen_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "2", "0", "1", "time it takes before regeneration can start again");
LEVEL_VARIABLE(mod_armorregen_maxfraction, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "");
LEVEL_VARIABLE(mod_armorregen_minaux, FCVAR_NOTIFY | FCVAR_REPLICATED, "35", "0", "1", "minimum amount of aux in reserve before regeneration will start");

LEVEL_VARIABLE(mod_attrition_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1.5", "1", "duration of attrition effect");

LEVEL_VARIABLE(mod_auxpower, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "5", "1", "aux power capacity increase with each level of aux power tank");

/*LEVEL_VARIABLE(mod_bioarmor_cast_time, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.05", "0", "1", "casting time of BioArmor module");
LEVEL_VARIABLE(mod_bioarmor_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "1.5", "1", "cooldown of BioArmor module");
LEVEL_VARIABLE(mod_bioarmor_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "5", "1", "AUX drain from using BioArmor module");
LEVEL_VARIABLE(mod_bioarmor_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "1", "duration of BioArmor effect");
LEVEL_VARIABLE(mod_bioarmor_aps, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.5", "0.55", "1", "armor per second that BioArmor will charge");*/

LEVEL_VARIABLE(mod_bruteforce, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.1", "1", "melee damage boost from brute force");
LEVEL_VARIABLE(mod_bruteforce_buff, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.05", "1", "bonus damage scaling of brute force buff");
LEVEL_VARIABLE(mod_bruteforce_buff_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "7.5", "0", "0", "duration of brute force buff");
LEVEL_VARIABLE(mod_bruteforce_buff_stacks, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "max number of times the brute force buff can stack");

LEVEL_VARIABLE(mod_clipsize_357, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.6", "1", "extra ammo for 357 with each level of Clip Size");
LEVEL_VARIABLE(mod_clipsize_ar2, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "3", "1", "extra ammo for ar2 with each level of Clip Size");
LEVEL_VARIABLE(mod_clipsize_crossbow, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.1", "1", "extra ammo for crossbow with each level of Clip Size");
LEVEL_VARIABLE(mod_clipsize_pistol, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1.8", "1", "extra ammo for pistol with each level of Clip Size");
LEVEL_VARIABLE(mod_clipsize_rpg, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.1", "1", "extra ammo for rpg with each level of Clip Size");
LEVEL_VARIABLE(mod_clipsize_shotgun, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.6", "1", "extra ammo for shotgun with each level of Clip Size");
LEVEL_VARIABLE(mod_clipsize_smg, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "4.5", "1", "extra ammo for smg with each level of Clip Size");

LEVEL_VARIABLE(mod_cloak_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "2", "0", "0", "cooldown of Cloak module");
LEVEL_VARIABLE(mod_cloak_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "AUX drain per second from using Cloak module");
LEVEL_VARIABLE(mod_cloak_movement_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "100", "0", "0", "This times velocity squared over 1 million is drained every second, in addition to the regular cooldown drain");
LEVEL_VARIABLE(mod_cloak_suppression_time, FCVAR_NOTIFY | FCVAR_REPLICATED, "2", "0", "0", "duration of cloak suppression after damage is dealt");

LEVEL_VARIABLE(mod_crits_chance, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "2", "1", "percentage chance for the Critical Hits module to trigger a critical hit");
LEVEL_VARIABLE(mod_crits_drain_proportion, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.5", "0", "1", "fraction of bullet damage each shot drains from aux when crits is active");
LEVEL_VARIABLE(mod_crits_scale, FCVAR_NOTIFY | FCVAR_REPLICATED, "4", "0", "0", "Damage increase factor of critical hits");

LEVEL_VARIABLE(mod_crow_airspeed, FCVAR_REPLICATED|FCVAR_NOTIFY, "220", "0", "1", "speed that crows fly at");
LEVEL_VARIABLE(mod_crow_explode_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "19", "3", "1", "damage dealt by exploding crows");
LEVEL_VARIABLE(mod_crow_health, FCVAR_REPLICATED|FCVAR_NOTIFY, "5", "0", "1", "health of a player-owned crow");
LEVEL_VARIABLE(mod_crow_limit, FCVAR_NOTIFY | FCVAR_REPLICATED, "4", "0", "0", "maximum number of crows that can be deployed simultaneously");

LEVEL_VARIABLE(mod_crowlauncher_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "4", "0", "0", "cooldown of Crow Launcher module");
LEVEL_VARIABLE(mod_crowlauncher_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "45", "0", "0", "AUX drain from using Crow Launcher module");

LEVEL_VARIABLE(mod_damageamp, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0.1", "0.9", "damage scaling of damage amp effect");
LEVEL_VARIABLE(mod_damageamp_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "cooldown of Damage Amp module");
LEVEL_VARIABLE(mod_damageamp_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "50", "0", "0", "AUX drain from using Damage Amp module");
LEVEL_VARIABLE(mod_damageamp_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "3", "0", "1", "duration of damage amp effect");

LEVEL_VARIABLE(mod_energyball_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "1.5", "0", "1", "cooldown of Energy Ball module");
LEVEL_VARIABLE(mod_energyball_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "14", "6", "1.1", "energy ball (module) damage");
LEVEL_VARIABLE(mod_energyball_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "5", "0.6", "AUX drain from using Energy Ball module");
LEVEL_VARIABLE(mod_energyball_lifetime, FCVAR_NOTIFY | FCVAR_REPLICATED, "15", "0", "0", "energy ball (module) lifetime");
LEVEL_VARIABLE(mod_energyball_speed, FCVAR_NOTIFY | FCVAR_REPLICATED, "1500", "0", "1", "energy ball (module) speed");

LEVEL_VARIABLE(mod_fastzombie_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "cooldown of Fast Zombie minion module");
LEVEL_VARIABLE(mod_fastzombie_dmg_claw, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0.8", "1", "fast zombie minion melee damage");
LEVEL_VARIABLE(mod_fastzombie_dmg_leap, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0.8", "1", "fast zombie minion pounce damage");
LEVEL_VARIABLE(mod_fastzombie_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "45", "0", "0", "AUX drain from using Fast Zombie minion module");
LEVEL_VARIABLE(mod_fastzombie_health, FCVAR_NOTIFY | FCVAR_REPLICATED, "40", "5.4", "1", "fast zombie minion health");

LEVEL_VARIABLE(mod_flechette_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "1.5", "0", "0", "cooldown of Flechette module");
LEVEL_VARIABLE(mod_flechette_dmg_blast, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "5", "1", "flechette explosion damage");
LEVEL_VARIABLE(mod_flechette_dmg_hit, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.5", "1.5", "1", "flechette impact damage");
LEVEL_VARIABLE(mod_flechette_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "4", "0", "0", "AUX drain from using Flechette module");
LEVEL_VARIABLE(mod_flechette_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.72", "0", "0", "burst fire duration of Flechette module");
LEVEL_VARIABLE(mod_flechette_refire, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.1", "0", "0", "refire rate of Flechette module");

LEVEL_VARIABLE(mod_freezebomb_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "0", "0", "Aux drain of freeze grenade");
LEVEL_VARIABLE(mod_freezebomb_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "3", "0.5", "1", "cooldown of freeze bomb");
LEVEL_VARIABLE(mod_freezebomb_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.39", "0.36", "1", "duration of freeze bomb immobilisation effect");

LEVEL_VARIABLE(mod_heald_cast_time, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.05", "0", "0", "casting time of HEALD module");
LEVEL_VARIABLE(mod_heald_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "3.95", "0", "0", "cooldown of HEALD module");
LEVEL_VARIABLE(mod_heald_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0", "0", "AUX drain from using HEALD module");
LEVEL_VARIABLE(mod_heald_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "1", "duration of HEALD effect");
LEVEL_VARIABLE(mod_heald_hps, FCVAR_NOTIFY | FCVAR_REPLICATED, "2", "1.2", "1", "health per second that HEALD will heal");

LEVEL_VARIABLE(mod_impact, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.0375", "1", "impact module damage bonus");

LEVEL_VARIABLE(mod_incendiary_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "cooldown of Incendiary Grenade module");
LEVEL_VARIABLE(mod_incendiary_dps, FCVAR_NOTIFY | FCVAR_REPLICATED, "7", "0", "1", "damage per second dealt by incendary grenades");
LEVEL_VARIABLE(mod_incendiary_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "40", "0", "0", "AUX drain from using Incendiary Grenade module");
LEVEL_VARIABLE(mod_incendiary_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "duration of incendiary grenade burning effect");
LEVEL_VARIABLE(mod_incendiary_tick, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "damage per second dealt by incendary grenades");

LEVEL_VARIABLE(mod_jetpack_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "0", "0", "AUX drain per-second from using Jetpack module");

LEVEL_VARIABLE(mod_lasers_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "0", "cooldown of Lasers module");
LEVEL_VARIABLE(mod_lasers_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "laser damage dealt per tick (with 20 ticks per second)");
LEVEL_VARIABLE(mod_lasers_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "12", "0", "0", "AUX drain from using Lasers module");
LEVEL_VARIABLE(mod_lasers_lifetime, FCVAR_NOTIFY | FCVAR_REPLICATED, "60", "0", "0", "lifetime of lasers before they shut down");
LEVEL_VARIABLE(mod_lasers_limit, FCVAR_NOTIFY | FCVAR_REPLICATED, "6", "0", "0", "maximum lasers a player can deploy simultaneously");
LEVEL_VARIABLE(mod_lasers_max_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "10", "1", "total damage dealt before a laser shuts down");

LEVEL_VARIABLE(mod_longjump_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0", "0", "AUX drain from using Long Jump module");
LEVEL_VARIABLE(mod_longjump_speed, FCVAR_NOTIFY | FCVAR_REPLICATED, "600.0", "0", "0", "launch speed when using Long Jump module");
LEVEL_VARIABLE(mod_longjump_use_interval, FCVAR_NOTIFY | FCVAR_REPLICATED, "250", "0", "0", "period (in ms) after ducking after which player must jump to active Long Jump module");
LEVEL_VARIABLE(mod_longjump_vertical_scale_factor, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.75", "0", "0", "Scaling to apply to vertical component of effect");

LEVEL_VARIABLE(mod_magmine_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "2.5", "0" ,"0", "cooldown of Magmine module");
LEVEL_VARIABLE(mod_magmine_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "3", "1", "damage dealt by exploding magmines");
LEVEL_VARIABLE(mod_magmine_damage_radius, FCVAR_NOTIFY | FCVAR_REPLICATED, "42", "0", "0", "damage radius of magmine explosion");
LEVEL_VARIABLE(mod_magmine_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "40", "0", "0", "AUX drain from using Magmine module");
LEVEL_VARIABLE(mod_magmine_fullradius, FCVAR_NOTIFY | FCVAR_REPLICATED, "450", "0", "1", "radius of full magmine effect");
LEVEL_VARIABLE(mod_magmine_health, FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "3", "1", "damage a magmine can take before exploding");
LEVEL_VARIABLE(mod_magmine_limit, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "0", "maximum number of magmines that can be deployed simultaneously");
LEVEL_VARIABLE(mod_magmine_strength, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0.6", "0.85", "suction strength of a magmine");
/*
LEVEL_VARIABLE(mod_repmine_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "2.5", "0" ,"0", "cooldown of Repmine module");
LEVEL_VARIABLE(mod_repmine_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "3", "1", "damage dealt by exploding repmines");
LEVEL_VARIABLE(mod_repmine_damage_radius, FCVAR_NOTIFY | FCVAR_REPLICATED, "42", "0", "0", "damage radius of repmine explosion");
LEVEL_VARIABLE(mod_repmine_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "40", "0", "0", "AUX drain from using Repmine module");
LEVEL_VARIABLE(mod_repmine_fullradius, FCVAR_NOTIFY | FCVAR_REPLICATED, "450", "0", "1", "radius of full repmine effect");
LEVEL_VARIABLE(mod_repmine_health, FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "3", "1", "damage a repmine can take before exploding");
LEVEL_VARIABLE(mod_repmine_limit, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "0", "maximum number of repmines that can be deployed simultaneously");
LEVEL_VARIABLE(mod_repmine_strength, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0.6", "0.85", "suction strength of a repmine");
*/

LEVEL_VARIABLE(mod_manhack_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "2.0", "0", "0", "cooldown of Manhack minion module");
LEVEL_VARIABLE(mod_manhack_dmg_held, FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "2", "1", "manhack minion damage-per-second while held in gravity gun");
LEVEL_VARIABLE(mod_manhack_dmg_slash, FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "2", "1", "manhack minion damage-per-second");
LEVEL_VARIABLE(mod_manhack_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "25", "0", "0", "AUX drain from using Manhack minion module");
LEVEL_VARIABLE(mod_manhack_engine_power, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0.75", "1", "manhack minion engine power");
LEVEL_VARIABLE(mod_manhack_health, FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "2", "1", "manhack minion health");
LEVEL_VARIABLE(mod_manhack_limit, FCVAR_NOTIFY | FCVAR_REPLICATED, "6", "0", "0", "maximum number of manhacks that can be deployed simultaneously");

LEVEL_VARIABLE(mod_mirvgrenade_bomblet_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "15", "3", "1", "damage dealt by secondary MIRV projectiles");
LEVEL_VARIABLE(mod_mirvgrenade_bomblet_radius, FCVAR_NOTIFY | FCVAR_REPLICATED, "100", "5", "1", "radius of secondary MIRV projectiles' blast");
LEVEL_VARIABLE(mod_mirvgrenade_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "2.5", "0", "0", "cooldown of MIRV module");
LEVEL_VARIABLE(mod_mirvgrenade_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "25", "5", "1", "damage dealt by main MIRV projectile");
LEVEL_VARIABLE(mod_mirvgrenade_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "45", "0", "0", "AUX drain from using MIRV module");
LEVEL_VARIABLE(mod_mirvgrenade_radius, FCVAR_NOTIFY | FCVAR_REPLICATED, "100", "5", "1", "radius of main MIRV projectile's blast");

LEVEL_VARIABLE(mod_plague_caster_interval, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.25", "0", "0", "interval at which plague will attempt to spread from the player activing it");
LEVEL_VARIABLE(mod_plague_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.5", "0.083", "1", "damage from each tick of plague effect");
LEVEL_VARIABLE(mod_plague_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", ".5", "1", "aux drained by plague each second");
LEVEL_VARIABLE(mod_plague_drain_per_victim, FCVAR_NOTIFY | FCVAR_REPLICATED, ".5", "0", "1", "additional aux drained by plague each tick for each active victim");
LEVEL_VARIABLE(mod_plague_radius, FCVAR_NOTIFY | FCVAR_REPLICATED, "82", "0", "1", "radius at which plague will spread");
LEVEL_VARIABLE(mod_plague_victim_interval, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.25", "0", "0", "interval at which plague will hurt and attempt to spread from victims");

LEVEL_VARIABLE(mod_poisonspit_amount, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "2", "1", "number of projectiles created by poison spit. One will always be 'large' size, the rest will randomly be 'medium' or 'small'");
LEVEL_VARIABLE(mod_poisonspit_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "2.5", "0", "0", "cooldown of Poison Spit module");
LEVEL_VARIABLE(mod_poisonspit_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "3", "0.7", "damage dealt by main poison spit projectile. Smaller projectiles deal 0.5 or 0.25 times this damage.");
LEVEL_VARIABLE(mod_poisonspit_damagescale_end, FCVAR_REPLICATED|FCVAR_NOTIFY, "1", "0", "0", "Stop scaling spit damage up after this time has passed");
LEVEL_VARIABLE(mod_poisonspit_damagescale_limit, FCVAR_REPLICATED|FCVAR_NOTIFY, "3.25", "0", "0", "Scale spit damage up to this value (when mod_poisonspit_damagescale_end has elapsed, less if impact occurs sooner)");
LEVEL_VARIABLE(mod_poisonspit_damagescale_start, FCVAR_REPLICATED|FCVAR_NOTIFY, "0.15", "0", "0", "Don't start scaling spit damage up until this much time has passed");
LEVEL_VARIABLE(mod_poisonspit_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "30", "0", "0", "AUX drain from using Poison Spit module");
LEVEL_VARIABLE(mod_poisonspit_speed, FCVAR_NOTIFY | FCVAR_REPLICATED, "1250", "0", "1", "Launch speed of poison spit projectiles");
LEVEL_VARIABLE(mod_poisonspit_spread, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.135", "0", "0", "Spread only applies to medium & small spitballs - the large one is always fired straight");

LEVEL_VARIABLE(mod_recharge_interval, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "recharge interval");
LEVEL_VARIABLE(mod_recharge_amount, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.5", "1", "recharge amount");

LEVEL_VARIABLE(mod_shockwave_aux_transfer_ratio, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "aux given to casterfor each point of aux drained from a victim");
LEVEL_VARIABLE(mod_shockwave_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0", "0", "cooldown of Shockwave module");
LEVEL_VARIABLE(mod_shockwave_damage_ratio, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "damage dealt for each point of aux drained from a victim");
LEVEL_VARIABLE(mod_shockwave_drain,FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0", "0", "AUX drain from using Shockwave module");
LEVEL_VARIABLE(mod_shockwave_radius, FCVAR_NOTIFY | FCVAR_REPLICATED, "480", "0", "0", "radius of effect of Shockwave module");
LEVEL_VARIABLE(mod_shockwave_victim_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "8.333", "1.667", "1", "aux drained from / damage dealt to each opponent by shockwave");

LEVEL_VARIABLE(mod_teleport_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "cooldown of Teleport module");
LEVEL_VARIABLE(mod_teleport_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "50", "0", "0", "AUX drain from using Teleport module");
LEVEL_VARIABLE(mod_teleport_range, FCVAR_NOTIFY | FCVAR_REPLICATED, "420", "0", "0", "maximum range of Teleport module");

LEVEL_VARIABLE(mod_turret_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "3", "0", "0", "cooldown of Turrets module");
LEVEL_VARIABLE(mod_turret_dmg_shoot, FCVAR_NOTIFY | FCVAR_REPLICATED, "4", "0.5", "1", "damage dealt by player-owned turrets");
LEVEL_VARIABLE(mod_turret_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "40", "0", "0", "AUX drain from using Turrets module");
LEVEL_VARIABLE(mod_turret_health, FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "5", "1", "health of player-owned turrets");
LEVEL_VARIABLE(mod_turret_limit, FCVAR_NOTIFY | FCVAR_REPLICATED, "2", "0", "0", "maximum number of Turrets that can be deployed simultaneously");

LEVEL_VARIABLE(mod_vitality, FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "5", "1", "player health capacity increase with each level of vitality")

LEVEL_VARIABLE(mod_vortigaunt_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "cooldown of Vortigaunt minion module");
LEVEL_VARIABLE(mod_vortigaunt_dmg_claw, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0.5", "1", "vortigaunt minion melee damage #1");
LEVEL_VARIABLE(mod_vortigaunt_dmg_rake, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0.9", "1", "vortigaunt minion melee damage #2");
LEVEL_VARIABLE(mod_vortigaunt_dmg_zap, FCVAR_NOTIFY | FCVAR_REPLICATED, "15", "1", "1", "vortigaunt minion zap damage");
LEVEL_VARIABLE(mod_vortigaunt_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "50", "0", "0", "AUX drain from using Vortigaunt minion module");
LEVEL_VARIABLE(mod_vortigaunt_health, FCVAR_NOTIFY | FCVAR_REPLICATED, "60", "2", "1", "vortigaunt minion health");
LEVEL_VARIABLE(mod_vortigaunt_zap_spread, FCVAR_NOTIFY | FCVAR_REPLICATED, "3.75", "0", "1", "vortigaunt minion zap spread (in degrees)");

LEVEL_VARIABLE(mod_weaken_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "cooldown of Weaken module");
LEVEL_VARIABLE(mod_weaken_damagescale, FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "-0.05", "1", "damage scaling of weaken effect");
LEVEL_VARIABLE(mod_weaken_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "50", "0", "0", "AUX drain from using Weaken module");
LEVEL_VARIABLE(mod_weaken_duration, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0", "0", "duration of weaken effect");
LEVEL_VARIABLE(mod_weaken_sprintscale, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.55555556", "-0.055555556", "1", "sprint speed scaling of weaken effect");

LEVEL_VARIABLE(mod_zombie_cooldown, FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "0", "cooldown of Zombie minion module");
LEVEL_VARIABLE(mod_zombie_dmg_both, FCVAR_NOTIFY | FCVAR_REPLICATED, "25", "1", "1", "zombie minion two-handed damage");
LEVEL_VARIABLE(mod_zombie_dmg_one, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "1", "1", "zombie minion one-handed damage");
LEVEL_VARIABLE(mod_zombie_drain, FCVAR_NOTIFY | FCVAR_REPLICATED, "50", "0", "0", "AUX drain from using Zombie minion module");
LEVEL_VARIABLE(mod_zombie_health, FCVAR_NOTIFY | FCVAR_REPLICATED, "75", "5", "1", "zombie minion health");

McConVar ally_experience_boost_fraction("ally_experience_boost_fraction", "0.05", FCVAR_NOTIFY | FCVAR_REPLICATED, "Extra experience fraction awarded for having faction allies nearby in PVM", true, 0, true, 1 );
McConVar gamemode_hoarder_min_players("gamemode_hoarder_min_players", "6", FCVAR_NOTIFY | FCVAR_REPLICATED, "The minimum number of players required in the hoarder gamemode for tokens to start spawning", true, 0, false, 0
#ifndef CLIENT_DLL
	, gamemode_hoarder_min_players_changed
#endif
	);
McConVar gamemode_hoarder_respawn_delay_no_tokens("gamemode_hoarder_respawn_delay_no_tokens", "13", FCVAR_NOTIFY | FCVAR_REPLICATED, "Respawn delay when a player dies with zero score tokens in the Hoarder game mode", true, 5, false, 0);
McConVar mc_headcrab_bounce("mc_headcrab_bounce", "300", FCVAR_NOTIFY | FCVAR_REPLICATED, "How hard the player is bounced upwards after jumping on a headcrab. 0 to disable", true, 0, false, 0);
McConVar mc_max_level("mc_max_level", "25", FCVAR_NOTIFY | FCVAR_REPLICATED, "Maximum player level allowed", true, 0, true, 100);
McConVar mc_player_base_armor_capacity("mc_player_base_armor_capacity", "100", FCVAR_NOTIFY | FCVAR_REPLICATED, "Default player starting armor capacity", true, 0, true, 500);
McConVar mc_player_base_aux_capacity("mc_player_base_aux_capacity", "100", FCVAR_NOTIFY | FCVAR_REPLICATED, "Default player starting aux power capacity", true, 0, true, 500);
McConVar mc_player_base_aux_recharge("mc_player_base_aux_recharge", "2", FCVAR_NOTIFY | FCVAR_REPLICATED, "Default player aux recharge per second");
McConVar mc_player_base_health("mc_player_base_health", "100", FCVAR_NOTIFY | FCVAR_REPLICATED, "Default player starting health", true, 1, true, 500);
McConVar mc_player_base_sprint_speed("mc_player_base_sprint_speed", "290", FCVAR_NOTIFY | FCVAR_REPLICATED, "Default player starting sprint speed", true, 0, true, 500);
McConVar mc_player_base_walk_speed("mc_player_base_walk_speed", "190", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar mc_player_max_crows("mc_player_max_crows", "4", FCVAR_NOTIFY | FCVAR_REPLICATED );
McConVar mc_player_max_minions("mc_player_max_minions", "4", FCVAR_NOTIFY | FCVAR_REPLICATED );
McConVar mc_respawn_delay("mc_respawn_delay", "5", FCVAR_NOTIFY, "Delay after being killed before a player can respawn", true, 0, false, 0);
McConVar minion_drain_scale("minion_drain_scale", "1.0", FCVAR_NOTIFY | FCVAR_REPLICATED );
McConVar mod_magmine_allow_upward_force("mod_magmine_allow_upward_force", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Can magmines pull upwards, at all?", true, 0, true, 1 );
McConVar mod_magmine_npc_scale("mod_magmine_npc_scale", "0.6", FCVAR_NOTIFY | FCVAR_REPLICATED );
/*
McConVar mod_repmine_allow_upward_force("mod_repmine_allow_upward_force", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Can repmines pull upwards, at all?", true, 0, true, 1 );
McConVar mod_repmine_npc_scale("mod_repmine_npc_scale", "0.6", FCVAR_NOTIFY | FCVAR_REPLICATED );
*/
McConVar pvm_buff_combine_scale("pvm_buff_combine_scale", "0.05", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar pvm_buff_interval_aperture("pvm_buff_interval_aperture", "1", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar pvm_buff_interval_resistance("pvm_buff_interval_resistance", "2.5", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar pvm_max_ally_bonus_distance("pvm_max_ally_bonus_distance", "1000", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_357_clip_size("weapon_357_clip_size", "6", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_357_primary_damage("weapon_357_primary_damage", "75", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_357_primary_refire("weapon_357_primary_refire", "0.75", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_357_reload_time("weapon_357_reload_time", "3.6667", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_ar2_clip_size("weapon_ar2_clip_size", "30", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_ar2_primary_damage("weapon_ar2_primary_damage", "11", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_ar2_primary_refire("weapon_ar2_primary_refire", "0.1", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_ar2_reload_time("weapon_ar2_reload_time", "1.5667", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_ar2_secondary_damage("weapon_ar2_secondary_damage", "0.75", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_ar2_secondary_delay("weapon_ar2_secondary_delay", "0.5", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_ar2_secondary_refire("weapon_ar2_secondary_refire", "1", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_crossbow_bolt_speed("weapon_crossbow_bolt_speed", "3500", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_crossbow_clip_size("weapon_crossbow_clip_size", "1", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_crossbow_primary_damage("weapon_crossbow_primary_damage", "75", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_crossbow_primary_refire("weapon_crossbow_primary_refire", "0.75", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_crossbow_reload_time("weapon_crossbow_reload_time", "1.8333", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_frag_damage("weapon_frag_damage", "150", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_frag_radius("weapon_frag_radius", "250", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_frag_refire("weapon_frag_refire", "1", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_gravitygun_refire("weapon_gravitygun_refire", "0.5", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_melee_primary_damage("weapon_melee_primary_damage", "32", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_melee_primary_range("weapon_melee_primary_range", "75", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_melee_primary_refire("weapon_melee_primary_refire", "0.5", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_pistol_clip_size("weapon_pistol_clip_size", "18", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_pistol_primary_damage("weapon_pistol_primary_damage", "8", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_pistol_primary_refire("weapon_pistol_primary_refire", "0.15", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_pistol_primary_refire_fastest("weapon_pistol_primary_refire_fastest", "0.15", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_pistol_reload_time("weapon_pistol_reload_time", "1.4333", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_rpg_clip_size("weapon_rpg_clip_size", "1", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_rpg_primary_damage("weapon_rpg_primary_damage", "150", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_rpg_primary_refire("weapon_rpg_primary_refire", "0.8", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_rpg_reload_time("weapon_rpg_reload_time", "1.6667", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_rpg_rocket_speed("weapon_rpg_rocket_speed", "1500", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_shotgun_clip_size("weapon_shotgun_clip_size", "6", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_shotgun_pellet_damage("weapon_shotgun_pellet_damage", "9", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_shotgun_primary_refire("weapon_shotgun_primary_refire", "0.3333", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_shotgun_pump_time("weapon_shotgun_pump_time", "0.5333", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_shotgun_secondary_refire("weapon_shotgun_secondary_refire", "0.5", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_slam_satchel_damage("weapon_slam_satchel_damage", "150", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_slam_satchel_refire("weapon_slam_satchel_refire", "0.75", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_slam_tripmine_attach_range("weapon_slam_tripmine_attach_range", "42", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_slam_tripmine_damage("weapon_slam_tripmine_damage", "150", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_slam_tripmine_refire("weapon_slam_tripmine_refire", "0.75", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_smg_clip_size("weapon_smg_clip_size", "45", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_smg_primary_damage("weapon_smg_primary_damage", "8", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_smg_primary_refire("weapon_smg_primary_refire", "0.075", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_smg_reload_time("weapon_smg_reload_time", "1.5", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_smg_secondary_damage("weapon_smg_secondary_damage", "100", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_smg_secondary_radius("weapon_smg_secondary_radius", "250", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar weapon_smg_secondary_refire("weapon_smg_secondary_refire", "1", FCVAR_NOTIFY | FCVAR_REPLICATED);

#ifndef CLIENT_DLL
// server-only convars
LEVEL_VARIABLE(mod_crow_explode_radius, FCVAR_GAMEDLL|FCVAR_NOTIFY, "80", "3", "1", "crow explosion radius");
LEVEL_VARIABLE(mod_crow_takeoff_speed, FCVAR_GAMEDLL|FCVAR_NOTIFY, "140", "5", "1", "speed that crows lift off from the ground at");
LEVEL_VARIABLE(mod_freeze_grenade_radius, FCVAR_NOTIFY | FCVAR_REPLICATED, "75", "23", "1", "freeze grenade radius");
LEVEL_VARIABLE(mod_incendiary_grenade_damage, FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "5", "1", "incendiary grenade damage");
LEVEL_VARIABLE(mod_incendiary_grenade_radius, FCVAR_NOTIFY | FCVAR_REPLICATED, "140", "0", "0", "incendiary grenade radius");
LEVEL_VARIABLE(npc_aux_max, FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "0", "0", "AUX power max of all NPCs");
LEVEL_VARIABLE(npc_aux_recharge, FCVAR_NOTIFY | FCVAR_REPLICATED, "0.5", "0", "0", "AUX power recharged everys second by each NPC");
McConVar gamemode_hoarder_exp_scale("gamemode_hoarder_exp_scale", "0.5", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Kill experience in the Hoarder gamemode is scaled down by this amount");
McConVar gamemode_hoarder_monster_score_token_expiry_time("gamemode_hoarder_monster_score_token_expiry_time", "60", FCVAR_GAMEDLL | FCVAR_NOTIFY, "How long after spawning a monster with a score token will have that token removed - only affects the Hoarder game mode");
McConVar gamemode_hoarder_token_add_interval("gamemode_hoarder_token_add_interval", "30", FCVAR_GAMEDLL | FCVAR_NOTIFY, "The interval in the hoarder gamemode at which monsters with score tokens will spawn");
McConVar gamemode_hoarder_victory_exp("gamemode_hoarder_victory_exp", "1000", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded to all players on the winning faction in the Hoarder game mode");
McConVar mc_aux_batteries_min("mc_aux_batteries_min", "10", FCVAR_NOTIFY, "Minimum number of aux batteries that will be created at the start of a game.");
McConVar mc_aux_batteries_per_armor_battery("mc_aux_batteries_per_armor_battery", "1", FCVAR_NOTIFY, "Number of aux batteries that will be created at the start of a game, for each armor battery in the game.");
McConVar mc_aux_batteries_per_armor_charger("mc_aux_batteries_per_armor_charger", "0", FCVAR_NOTIFY, "Number of aux batteries that will be created at the start of a game, for each armor charger in the game.");
McConVar mc_aux_batteries_per_health_charger("mc_aux_batteries_per_health_charger", "0", FCVAR_NOTIFY, "Number of aux batteries that will be created at the start of a game, for each health charger in the game.");
McConVar mc_aux_batteries_per_health_kit("mc_aux_batteries_per_health_kit", "1", FCVAR_NOTIFY, "Number of aux batteries that will be created at the start of a game, for each health kit in the game.");
McConVar mc_aux_batteries_per_health_vial("mc_aux_batteries_per_health_vial", "0.5", FCVAR_NOTIFY, "Number of aux batteries that will be created at the start of a game, for each health vial in the game.");
McConVar mc_auxpower_pickup_energy("mc_auxpower_pickup_energy", "30", FCVAR_NOTIFY | FCVAR_REPLICATED );
McConVar mc_combo_kill_experience("mc_combo_kill_experience", "50", FCVAR_GAMEDLL | FCVAR_NOTIFY );
McConVar mc_death_shared_experience_scaling("mc_death_shared_experience_scaling", "0.5", FCVAR_NOTIFY, "When a player dies, all records of damage dealt by them (for the purposes of awarding shared experience) are reduced by this factor.", true, 0, true, 1);
McConVar mc_experience_minion("mc_experience_minion", "0.5", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Scale factor applied to experience awarded when killing another player's minion", true, 0, true, 10 );
McConVar mc_experience_monster("mc_experience_monster", "0.24", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Scale factor applied to experience awarded when killing a monster", true, 0, true, 10 );
McConVar mc_experience_spree_bonus("mc_experience_spree_bonus", "0.05", FCVAR_NOTIFY, "Fractional bonus experience awarded for each kill of a spree. Applied cumulatively.");
McConVar mc_faction_win_experience("mc_faction_win_experience", "500", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded to winning faction in PVM or Team Deathmatch", true, 0, false, 0 );
McConVar mc_killexp_leveldiff_00("mc_killexp_leveldiff_00", "100", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is of the same level as the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n01("mc_killexp_leveldiff_n01", "96", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 1 level lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n02("mc_killexp_leveldiff_n02", "93", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 2 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n03("mc_killexp_leveldiff_n03", "89", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 3 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n04("mc_killexp_leveldiff_n04", "86", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 4 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n05("mc_killexp_leveldiff_n05", "82", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 5 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n06("mc_killexp_leveldiff_n06", "78", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 6 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n07("mc_killexp_leveldiff_n07", "75", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 7 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n08("mc_killexp_leveldiff_n08", "71", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 8 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n09("mc_killexp_leveldiff_n09", "68", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 9 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n10("mc_killexp_leveldiff_n10", "64", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 10 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n11("mc_killexp_leveldiff_n11", "60", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 11 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n12("mc_killexp_leveldiff_n12", "57", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 12 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n13("mc_killexp_leveldiff_n13", "53", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 13 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n14("mc_killexp_leveldiff_n14", "49", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 14 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n15("mc_killexp_leveldiff_n15", "46", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 15 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n16("mc_killexp_leveldiff_n16", "42", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 16 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n17("mc_killexp_leveldiff_n17", "39", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 17 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n18("mc_killexp_leveldiff_n18", "35", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 18 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n19("mc_killexp_leveldiff_n19", "31", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 19 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n20("mc_killexp_leveldiff_n20", "28", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 20 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n21("mc_killexp_leveldiff_n21", "24", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 21 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n22("mc_killexp_leveldiff_n22", "21", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 22 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n23("mc_killexp_leveldiff_n23", "17", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 23 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_n24("mc_killexp_leveldiff_n24", "13", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 24 levels lower than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p01("mc_killexp_leveldiff_p01", "127", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 1 level higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p02("mc_killexp_leveldiff_p02", "154", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 2 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p03("mc_killexp_leveldiff_p03", "181", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 3 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p04("mc_killexp_leveldiff_p04", "208", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 4 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p05("mc_killexp_leveldiff_p05", "235", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 5 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p06("mc_killexp_leveldiff_p06", "263", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 6 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p07("mc_killexp_leveldiff_p07", "290", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 7 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p08("mc_killexp_leveldiff_p08", "317", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 8 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p09("mc_killexp_leveldiff_p09", "344", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 9 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p10("mc_killexp_leveldiff_p10", "370", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 10 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p11("mc_killexp_leveldiff_p11", "398", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 11 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p12("mc_killexp_leveldiff_p12", "425", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 12 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p13("mc_killexp_leveldiff_p13", "452", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 13 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p14("mc_killexp_leveldiff_p14", "479", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 14 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p15("mc_killexp_leveldiff_p15", "506", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 15 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p16("mc_killexp_leveldiff_p16", "533", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 16 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p17("mc_killexp_leveldiff_p17", "560", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 17 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p18("mc_killexp_leveldiff_p18", "588", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 18 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p19("mc_killexp_leveldiff_p19", "615", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 19 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p20("mc_killexp_leveldiff_p20", "642", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 20 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p21("mc_killexp_leveldiff_p21", "669", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 21 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p22("mc_killexp_leveldiff_p22", "696", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 22 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p23("mc_killexp_leveldiff_p23", "723", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 23 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_killexp_leveldiff_p24("mc_killexp_leveldiff_p24", "750", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Experience awarded for a kill where the victim is 24 levels higher than the killer", true, 0, true, 1000 );
McConVar mc_mechanical_damage_scale_energy("mc_mechanical_damage_scale_energy", "1.5", FCVAR_NOTIFY);
McConVar mc_perlevel_modulepoints("mc_perlevel_modulepoints", "2", FCVAR_GAMEDLL | FCVAR_NOTIFY );McConVar mc_combo_kill_max_interval("mc_combo_kill_max_interval", "0.75", FCVAR_GAMEDLL | FCVAR_NOTIFY );
McConVar mc_player_base_sprint_drain("mc_player_base_sprint_drain", "10", FCVAR_NOTIFY | FCVAR_REPLICATED ); // 3.3333333 originally
McConVar mc_player_startlevel("mc_player_startlevel", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "The level new players start the game at", true, 0, true, 100 );
McConVar mc_spawn_protect_time("mc_spawn_protect_time", "2", FCVAR_NOTIFY, "Time after spawning when players cannot be harmed", true, 0, true, 5);
McConVar mc_spree_start("mc_spree_start", "5", FCVAR_NOTIFY, "Number of consecutive kills needed to start a spree");
McConVar mc_spreewar_start("mc_spreewar_start", "25", FCVAR_NOTIFY, "Number of consecutive kills needed to start a spree war");
McConVar mc_vortigaunt_armor_charge_per_level( "mc_vortigaunt_armor_charge_per_level","10", FCVAR_CHEAT);
McConVar mc_vortigaunt_armor_charge_per_token( "mc_vortigaunt_armor_charge_per_token","2", FCVAR_CHEAT);
McConVar mod_magmine_downward_force_to_count_as_kill("mod_magmine_downward_force_to_count_as_kill", "100", FCVAR_NOTIFY, "How hard a player has to be being pulled downward by a MAGD for fall deaths to count as a kill by the MAGD owner", true, 0, false, 0);
McConVar mod_magmine_gravitykills("mod_magmine_gravitykills", "1", FCVAR_NOTIFY, "Do players falling near MAGDs count as kills?", true, 0, true, 1);
/*
McConVar mod_repmine_downward_force_to_count_as_kill("mod_repmine_downward_force_to_count_as_kill", "100", FCVAR_NOTIFY, "How hard a player has to be being pulled downward by a MAGD for fall deaths to count as a kill by the MAGD owner", true, 0, false, 0);
McConVar mod_repmine_gravitykills("mod_repmine_gravitykills", "1", FCVAR_NOTIFY, "Do players falling near MAGDs count as kills?", true, 0, true, 1);
*/
McConVar mod_manhack_engine_power_max("mod_manhack_engine_power_max", "25", FCVAR_NOTIFY | FCVAR_REPLICATED);
McConVar mod_turret_ammo_regen_interval("mod_turret_ammo_regen_interval", "0.25", FCVAR_NOTIFY);
McConVar mod_turret_clip("mod_turret_clip", "75", FCVAR_NOTIFY);
McConVar mod_turret_health_regen_interval("mod_turret_health_regen_interval", "0.667", FCVAR_NOTIFY);
McConVar mod_turret_min_ammo("mod_turret_min_ammo", "17", FCVAR_NOTIFY, "Turret must have this much ammo to open fire", true, 1, false, 0);
McConVar npc_turret_angular_limit("npc_turret_angular_limit", "7500", FCVAR_NOTIFY);
McConVar npc_turret_linear_limit("npc_turret_linear_limit", "2500", FCVAR_NOTIFY);
McConVar pvm_max_bosses("pvm_max_bosses", "2", FCVAR_GAMEDLL | FCVAR_NOTIFY, "The limit to how many boss monsters are able to spawn in a PVM match", true, 1, true, 64 );
McConVar pvm_max_monster_level_offset_below_player_average("pvm_max_monster_level_offset_below_player_average", "3", FCVAR_GAMEDLL | FCVAR_NOTIFY, "The absolute limit for how many PvM monsters can be present in the game", true, 0, false, 100 );
McConVar pvm_max_monsters("pvm_max_monsters", "20", FCVAR_GAMEDLL | FCVAR_NOTIFY, "The absolute limit for how many PvM monsters can be present in the game", true, 1, true, 128 );
McConVar pvm_monsters_base("pvm_monsters_base", "8", FCVAR_GAMEDLL | FCVAR_NOTIFY, "The number of monsters that will be present in game, regardless of the number of players, when PvM is active", true, 0, true, 128 );
McConVar pvm_monsters_per_player("pvm_monsters_per_player", "4", FCVAR_GAMEDLL | FCVAR_NOTIFY, "The number of additional monsters that will be present in the game, for each player", true, 0, true, 128 );
McConVar pvm_wave_interval("pvm_wave_interval", "5.0", FCVAR_GAMEDLL | FCVAR_NOTIFY, "The interval between waves of hostile monsters spawning in PvM gameplay", true, 0.0, false, 0 );
McConVar sk_antlion_worker_spit_grenade_poison_ratio ( "sk_antlion_worker_spit_grenade_poison_ratio","0.3", FCVAR_NONE, "Percentage of an antlion worker's spit damage done as poison (which regenerates)");
McConVar sk_dmg_combineball("sk_dmg_combineball", "225", FCVAR_NOTIFY);
McConVar sk_vortigaunt_zap_range( "sk_vortigaunt_zap_range", "900", FCVAR_NONE, "Range of vortigaunt's ranged attack" );
McConVar sv_hl2mp_item_respawn_time( "sv_hl2mp_item_respawn_time", "30", FCVAR_GAMEDLL | FCVAR_NOTIFY );
McConVar sv_hl2mp_weapon_respawn_time( "sv_hl2mp_weapon_respawn_time", "20", FCVAR_GAMEDLL | FCVAR_NOTIFY );
#endif