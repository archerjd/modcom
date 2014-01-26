using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace ConvarController
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            //int FCVAR_NOTIFY = 0, FCVAR_REPLICATED = 0, FCVAR_GAMEDLL = 0;
            conVarControl1.Changed += new EventHandler(conVarControl1_Changed);
            levelVarControl1.Changed += new EventHandler(levelVarControl1_Changed);
            conVarControl1.AllowCompress = levelVarControl1.AllowCompress = false;

            /*
            new ConVar("sk_dmg_combineball", "225");
            new ConVar("sk_vortigaunt_zap_range", "900", "Range of vortigaunt's ranged attack");
            new ConVar("ally_experience_boost_fraction", "0.05", "Extra experience fraction awarded for having faction allies nearby in PVM");
            new ConVar("mc_armor_resistances", "0", "When set to 1, the 10-level resistance modules affect armor only, and the 1-level resistances affect health only. When set to 0, all resistance modules scale damage to both health and armor");
            new ConVar("mc_aux_recharge_innate", "5");
            new ConVar("mc_combo_kill_experience", "50");
            new ConVar("mc_combo_kill_max_interval", "0.75");
            new ConVar("mc_crowbar_damage", "32", "");
            new ConVar("mc_crowbar_refire", "0.5", "");
            new ConVar("mc_dev_debug_factions", "0", "Disables limits on faction experience, so it can be applied with any number of players");
            new ConVar("mc_experience_base", "100", "Experience for killing another player of the same level");
            new ConVar("mc_experience_level_falloff", "1", "Rescales how the base experience difference changes when the killer and victim are of different levels");
            new ConVar("mc_experience_minion", "0.5", "Scale factor applied to experience awarded when killing another player's minion");
            new ConVar("mc_experience_monster", "0.3", "Scale factor applied to experience awarded when killing a monster");
            new ConVar("mc_experience_spree_bonus", "0.05", "Fractional bonus experience awarded for each kill of a spree. Applied cumulatively.");
            new ConVar("mc_faction_win_experience", "500", "Experience awarded to winning faction in PVM or Team Deathmatch");
            new ConVar("mc_headcrab_bounce", "300", "How hard the player is bounced upwards after jumping on a headcrab. 0 to disable");
            new ConVar("mc_huladoll_energygain", "15");
            new ConVar("mc_levelexp_base", "1000", "Experience formula control. Changes have no effect until next map!");
            new ConVar("mc_levelexp_curve", "1200", "Experience formula control. Changes have no effect until next map!");
            new ConVar("mc_levelexp_scale", "900", "Experience formula control. Changes have no effect until next map!");
            new ConVar("mc_max_level", "25", "Maximum player level allowed");
            new ConVar("mc_mechanical_damage_scale_energy", "1.5");
            new ConVar("mc_perlevel_modulepoints_1", "3", "Module points given to players who select progression option 1");
            new ConVar("mc_perlevel_modulepoints_2", "2", "Module points given to players who select progression option 2");
            new ConVar("mc_perlevel_modulepoints_3", "1", "Module points given to players who select progression option 3");
            new ConVar("mc_perlevel_weaponpoints_1", "2", "Weapon upgrade points given to players who select progression option 1");
            new ConVar("mc_perlevel_weaponpoints_2", "4", "Weapon upgrade points given to players who select progression option 2");
            new ConVar("mc_perlevel_weaponpoints_3", "6", "Weapon upgrade points given to players who select progression option 3");
            new ConVar("mc_player_max_crows", "4");
            new ConVar("mc_player_max_lasers", "6");
            new ConVar("mc_player_max_magmines", "1");
            new ConVar("mc_player_max_manhacks", "6");
            new ConVar("mc_player_max_minions", "4");
            new ConVar("mc_player_max_turrets", "2");
            new ConVar("mc_player_startlevel", "1", "The level new players start the game at");
            new ConVar("mc_securitycamera_damage", "32", "");
            new ConVar("mc_securitycamera_refire", "0.5", "");
            new ConVar("mc_spawn_protect_time", "2", "Time after spawning when players cannot be harmed");
            new ConVar("mc_spree_start", "5", "Number of consecutive kills needed to start a spree");
            new ConVar("mc_spreewar_start", "25", "Number of consecutive kills needed to start a spree war");
            new ConVar("mc_stunstick_damage", "32", "");
            new ConVar("mc_stunstick_refire", "0.5", "");
            new ConVar("mc_vortigaunt_armor_charge_per_level", "10");
            new ConVar("mc_vortigaunt_armor_charge_per_token", "2");
            new ConVar("minion_drain_scale", "1.0");
            new ConVar("mod_ammoregen_ar2", "8");
            new ConVar("mod_ammoregen_buckshot", "2");
            new ConVar("mod_ammoregen_pistol", "10");
            new ConVar("mod_ammoregen_smg1", "10");
            new ConVar("mod_antitoxin_scale", "60", "Reduces all poison & acid damage taken by this percentage");
            new ConVar("mod_antlion_cooldown", "5");
            new ConVar("mod_antlion_drain", "45");
            new ConVar("mod_antlionworker_cooldown", "5");
            new ConVar("mod_antlionworker_drain", "45");
            new ConVar("mod_armorregen_maxfraction_max", "1.0");
            new ConVar("mod_armorregen_maxfraction_min", "0.75");
            new ConVar("mod_attrition_cooldown", "5");
            new ConVar("mod_attrition_drain", "25");
            new ConVar("mod_barnacle_cooldown", "5");
            new ConVar("mod_barnacle_drain", "50");
            new ConVar("mod_bulletresist_scale", "6", "Reduces all bullet damage taken by this percentage");
            new ConVar("mod_cloak_cooldown", "0.2");
            new ConVar("mod_cloak_drain", "1.0");
            new ConVar("mod_crits_scale", "4", "Damage increase factor of critical hits");
            new ConVar("mod_crowlauncher_cooldown", "4");
            new ConVar("mod_crowlauncher_drain", "45");
            new ConVar("mod_damageamp_cooldown", "1");
            new ConVar("mod_damageamp_cost", "50");
            new ConVar("mod_energyball_cooldown", "0.85");
            new ConVar("mod_energyball_drain", "18");
            new ConVar("mod_energyresist_scale", "7", "Reduces all damage taken from lasers & other high-energy attacks by this percentage");
            new ConVar("mod_fastheadcrab_cooldown", "5");
            new ConVar("mod_fastheadcrab_drain", "30");
            new ConVar("mod_fastzombie_cooldown", "5");
            new ConVar("mod_fastzombie_drain", "45");
            new ConVar("mod_flechette_cooldown", "1.5");
            new ConVar("mod_flechette_drain", "4");
            new ConVar("mod_flechette_duration", "0.72");
            new ConVar("mod_flechette_refire", "0.1");
            new ConVar("mod_freezebomb_cooldown", "10");
            new ConVar("mod_freezebomb_drain", "45");
            new ConVar("mod_heald_cast_time", "0.05");
            new ConVar("mod_heald_cooldown", "3.95");
            new ConVar("mod_heald_drain", "25");
            new ConVar("mod_impactresist_scale", "6", "Reduces all blast, crush & blunt impact damage taken by this percentage");
            new ConVar("mod_incendiary_cooldown", "5");
            new ConVar("mod_incendiary_drain", "40");
            new ConVar("mod_jetpack_cooldown", "0.1");
            new ConVar("mod_jetpack_drain", "2");
            new ConVar("mod_lasers_cooldown", "1");
            new ConVar("mod_lasers_drain", "12");
            new ConVar("mod_lasers_lifetime", "60");
            new ConVar("mod_longjump_drain", "10");
            new ConVar("mod_longjump_speed", "600.0");
            new ConVar("mod_longjump_use_interval", "250");
            new ConVar("mod_longjump_vertical_scale_factor", "0.75", "Scaling to apply to vertical effects");
            new ConVar("mod_magmine_allow_upward_force", "0", "Can magmines pull upwards, at all?");
            new ConVar("mod_magmine_cooldown", "2.5");
            new ConVar("mod_magmine_damage_radius", "42");
            new ConVar("mod_magmine_downward_force_to_count_as_kill", "100", "How hard a player has to be being pulled downward by a MAGD for fall deaths to count as a kill by the MAGD owner");
            new ConVar("mod_magmine_drain", "40");
            new ConVar("mod_magmine_gravitykills", "1", "Do players falling near MAGDs count as kills?");
            new ConVar("mod_magmine_max_speed", "500");
            new ConVar("mod_magmine_npc_scale", "0.6");
            new ConVar("mod_manhack_cooldown", "2.0");
            new ConVar("mod_manhack_drain", "25");
            new ConVar("mod_mindabsorb_cooldown", "10");
            new ConVar("mod_mindabsorb_cooldown_variation", "4");
            new ConVar("mod_mindabsorb_damage_per_level", "6");
            new ConVar("mod_mindabsorb_radius", "480");
            new ConVar("mod_mirvgrenade_cooldown", "2.5");
            new ConVar("mod_mirvgrenade_drain", "45");
            new ConVar("mod_phaseshift_cooldown", "30");
            new ConVar("mod_phaseshift_drain", "9");
            new ConVar("mod_phaseshift_initial_cooldown", "10");
            new ConVar("mod_phaseshift_tick", "0.5");
            new ConVar("mod_piercingresist_scale", "7", "Reduces all piercing & slashing damage taken by this percentage");
            new ConVar("mod_poisonspit_cooldown", "2.5");
            new ConVar("mod_poisonspit_damagescale_end", "1.00", "Stop scaling spit damage up after this time has passed");
            new ConVar("mod_poisonspit_damagescale_limit", "3.25", "Scale spit damage up to this value (when mod_poisonspit_damagescale_end has elapsed, less if impact occurs sooner)");
            new ConVar("mod_poisonspit_damagescale_start", "0.15", "Don't start scaling spit damage up until this much time has passed");
            new ConVar("mod_poisonspit_drain", "30");
            new ConVar("mod_poisonspit_sound", "0", "Play antlion 'spitting' sound when launched");
            new ConVar("mod_poisonspit_spread", "0.135", "Spread only applies to medium & small spitballs - the large one is always fired straight");
            new ConVar("mod_recharge_tick", "2");
            new ConVar("mod_regeneration_maxhealingfraction_max", "1.0");
            new ConVar("mod_regeneration_maxhealingfraction_min", "0.75");
            new ConVar("mod_shockabsorbers_ratio", "1");
            new ConVar("mod_shockwave_cooldown", "6");
            new ConVar("mod_shockwave_drain", "45");
            new ConVar("mod_sprint_drain_base", "0.333333");
            new ConVar("mod_sprint_drain_per_level", "1.0");
            new ConVar("mod_teleport_cooldown", "1.5");
            new ConVar("mod_teleport_drain", "30");
            new ConVar("mod_teleport_range", "420");
            new ConVar("mod_thermalinsulation_scale", "40", "Reduces all burn & freeze damage by this percentage");
            new ConVar("mod_turret_ammo_regen_interval", "0.25");
            new ConVar("mod_turret_clip", "75");
            new ConVar("mod_turret_cooldown", "3");
            new ConVar("mod_turret_drain", "40");
            new ConVar("mod_turret_health_regen_interval", "0.667");
            new ConVar("mod_turret_min_ammo", "17", "Turret must have this much ammo to open fire");
            new ConVar("mod_vortigaunt_cooldown", "5");
            new ConVar("mod_vortigaunt_drain", "50");
            new ConVar("mod_weaken_cooldown", "10");
            new ConVar("mod_weaken_cost", "50");
            new ConVar("mod_weaken_duration", "10");
            new ConVar("mod_zombie_cooldown", "5");
            new ConVar("mod_zombie_drain", "50");
            new ConVar("npc_turret_angular_limit", "7500");
            new ConVar("npc_turret_linear_limit", "2500");
            new ConVar("pvm_buff_combine_scale", "0.05");
            new ConVar("pvm_buff_interval_aperture", "1");
            new ConVar("pvm_buff_interval_resistance", "2.5");
            new ConVar("pvm_max_ally_bonus_distance", "1000");
            new ConVar("pvm_max_bosses", "2", "The limit to how many Antlion Guards are able to spawn in a PVM match");
            new ConVar("pvm_max_monster_level_offset_below_player_average", "3", "The absolute limit for how many PvM monsters can be present in the game");
            new ConVar("pvm_max_monsters", "20", "The absolute limit for how many PvM monsters can be present in the game");
            new ConVar("pvm_monsters_base", "8", "The number of monsters that will be present in game, regardless of the number of players, when PvM is active");
            new ConVar("pvm_monsters_per_player", "4", "The number of additional monsters that will be present in the game, for each player");
            new ConVar("pvm_wave_interval", "5.0", "The interval between waves of hostile monsters spawning in PvM gameplay");
            new ConVar("upgrade_357_knockback", "48", "Knockback force with each upgrade level");
            new ConVar("upgrade_357_penetration_depth", "10", "Penetration depth with each upgrade level");
            new ConVar("upgrade_357_power", "0.075", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_ar2_alt_ammo", "45", "The ammo given by the Ammo Converter upgrade");
            new ConVar("upgrade_ar2_em_armor_damage", "0.3", "Fractional damage increase vs armor with each upgrade level");
            new ConVar("upgrade_ar2_em_mech_damage", "0.2", "Fractional damage increase vs mechanicals with each upgrade level");
            new ConVar("upgrade_ar2_em_organic_damage", "0.015", "Fractional damage decrease vs organics with each upgrade level");
            new ConVar("upgrade_ar2_gauss_charge_damage", "175", "");
            new ConVar("upgrade_ar2_gauss_charge_time", "2.5", "");
            new ConVar("upgrade_ar2_penetration_depth", "6", "Increase with each level in the maximum depth a shot can penetrate before it has expended all of its energy");
            new ConVar("upgrade_ar2_slowing_decrease", "0.065", "");
            new ConVar("upgrade_ar2_slowing_duration", "0.25", "");
            new ConVar("upgrade_crossbow_bounce_base", "0.1", "Limit for dot product between bolt's angle and normal to surface it hits for a bounce to occur");
            new ConVar("upgrade_crossbow_bounce_minspeed", "900", "Speed below which a bolt will not bounce");
            new ConVar("upgrade_crossbow_bounce_scale", "0.9", "Bounce angle dot product increase each level");
            new ConVar("upgrade_crossbow_piercing", "0.05", "Chance to ignore armor with each upgrade level");
            new ConVar("upgrade_crossbow_tension_charge_time", "1.0", "Must be held this long to apply the full effect");
            new ConVar("upgrade_crossbow_tension_damage", "0.1", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_crossbow_tension_speed", "0.1", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_gravgun_pickupmass", "0.5", "Fractional max pickup mass increase with each upgrade level");
            new ConVar("upgrade_gravgun_punting", "0.175", "Fractional punting strength increase with each upgrade level");
            new ConVar("upgrade_gravgun_suction", "0.3", "Fractional suction strength increase with each upgrade level");
            new ConVar("upgrade_grenade_damage", "0.1", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_grenade_radius", "0.167", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_grenade_range", "0.15", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_grenade_refire", "0.08", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_melee_damage", "0.1", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_melee_range", "3", "Range increase with each upgrade level");
            new ConVar("upgrade_melee_swing", "0.05", "Fractional refire decrease with each upgrade level");
            new ConVar("upgrade_pistol_damage", "0.1", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_pistol_recoil", "0.07", "Fractional spread decrease with each upgrade level");
            new ConVar("upgrade_rpg_damage", "0.1", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_rpg_damage_base", "200", "Default RPG health");
            new ConVar("upgrade_rpg_damage_scale", "9", "RPG health increment");
            new ConVar("upgrade_rpg_health_base", "30", "Default RPG health");
            new ConVar("upgrade_rpg_health_scale", "10", "RPG health increment");
            new ConVar("upgrade_rpg_radius", "0.167", "Fractional explosion radius increase with each upgrade level");
            new ConVar("upgrade_rpg_speed", "0.2", "Fractional rocket speed increase with each upgrade level");
            new ConVar("upgrade_shotgun_blastcaps_damage", "2", "AOE damage with each level");
            new ConVar("upgrade_shotgun_blastcaps_radius", "72", "AOE radius");
            new ConVar("upgrade_shotgun_knockback", "18", "Knockback increase with each upgrade level");
            new ConVar("upgrade_shotgun_pellets", "1", "Extra pellets per shot with each upgrade level");
            new ConVar("upgrade_shotgun_slugs_damage_base", "25", "Base slug damage");
            new ConVar("upgrade_shotgun_slugs_damage_extramax", "80", "Max additional damage a slug can deal");
            new ConVar("upgrade_shotgun_slugs_falloff_range", "1280", "Range over which slug damage falls off from extramax + base to just base");
            new ConVar("upgrade_shotgun_spread", "0.07", "Fractional spread decrease with each upgrade level");
            new ConVar("upgrade_slam_damage", "0.1", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_slam_radius", "0.167", "Fractional explosion radius increase with each upgrade level");
            new ConVar("upgrade_slam_transparency", "0.09", "Fractional laser transparency reduction with each upgrade level");
            new ConVar("upgrade_smg_accuracy", "0.08", "Fractional bullet spread decrease increase with each upgrade level");
            new ConVar("upgrade_smg_clip", "5", "Clip size increase with each upgrade level");
            new ConVar("upgrade_smg_damage", "0.1", "Fractional damage increase with each upgrade level");
            new ConVar("upgrade_smg_radius", "0.18", "Fractional grenade blast radius increase with each upgrade level");
            new ConVar("upgrade_smg_range", "0.3", "Fractional grenade speed increase with each upgrade level");

            new ConVar("mc_player_health_bonus_per_level", "5", FCVAR_NOTIFY | FCVAR_REPLICATED );
            new ConVar("mod_cloak_movement_drain", "25", FCVAR_NOTIFY | FCVAR_REPLICATED, "This times velocity squared over 1 million is drained every tick, in addition to the regular cooldown drain");
            new ConVar("mod_energyball_lifetime", "8", FCVAR_NOTIFY | FCVAR_REPLICATED );
            new ConVar("mod_plague_caster_interval", "0.66", FCVAR_NOTIFY|FCVAR_REPLICATED, "interval at which plague will attempt to spread from the player activing it");
            new ConVar("mod_plague_victim_interval","2", FCVAR_NOTIFY | FCVAR_REPLICATED );

            new Level_Variable("mc_immobilize_duration", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.025", "1", "duration of brute force immobilize effect");
            new Level_Variable("mod_ammoregen_cooldown", FCVAR_NOTIFY | FCVAR_REPLICATED, "27.5", "-2.5", "1", "interval between ticks of Ammo Regeneration");
            new Level_Variable("mod_armorcap", FCVAR_NOTIFY | FCVAR_REPLICATED, "25", "25", "1", "armor capacity with each level of armor capacity");
            new Level_Variable("mod_armorregen_amount", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "amount of armor restored each tick");
            new Level_Variable("mod_armorregen_tick", FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "tick interval of armor regeneration");
            new Level_Variable("mod_attrition_duration", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1.5", "1", "duration of attrition effect");
            new Level_Variable("mod_auxpower", FCVAR_NOTIFY | FCVAR_REPLICATED, "70", "10", "1", "aux power capacity with each level of aux power tank");
            new Level_Variable("mod_crits_chance", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "percentage chance for the Critical Hits module to trigger a critical hit");
            new Level_Variable("mod_crow_airspeed", FCVAR_REPLICATED | FCVAR_NOTIFY, "240", "20", "1", "speed that crows fly at");
            new Level_Variable("mod_crow_explode_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "19", "3", "1", "damage dealt by exploding crows");
            new Level_Variable("mod_crow_explode_radius", FCVAR_GAMEDLL | FCVAR_NOTIFY, "80", "3", "1", "crow explosion radius");
            new Level_Variable("mod_crow_takeoff_speed", FCVAR_GAMEDLL | FCVAR_NOTIFY, "140", "5", "1", "speed that crows lift off from the ground at");
            new Level_Variable("mod_damageamp_duration", FCVAR_GAMEDLL | FCVAR_NOTIFY, "0", "0.5", "1", "duration of damage amp effect");
            new Level_Variable("mod_damageamp_extra", FCVAR_NOTIFY | FCVAR_REPLICATED, "1.6", "0.1", "1", "damage scaling of damage amp effect");
            new Level_Variable("mod_energyball_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "15", "5", "1", "energy ball (module) damage");
            new Level_Variable("mod_energyball_speed", FCVAR_NOTIFY | FCVAR_REPLICATED, "1500", "0", "1", "energy ball (module) speed");
            new Level_Variable("mod_flechette_dmg_blast", FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "5", "1", "flechette explosion damage");
            new Level_Variable("mod_flechette_dmg_hit", FCVAR_NOTIFY | FCVAR_REPLICATED, "0.5", "1.5", "1", "flechette impact damage");
            new Level_Variable("mod_freezebomb_duration", FCVAR_NOTIFY | FCVAR_REPLICATED, "0.39", "0.36", "1", "duration of freeze bomb immobilisation effect");
            new Level_Variable("mod_freezebomb_radius", FCVAR_NOTIFY | FCVAR_REPLICATED, "75", "23", "1", "radius of freeze bomb effect");
            new Level_Variable("mod_ghost_evade", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "0.006", "1", "chance of evading an attack completely");
            new Level_Variable("mod_ghost_opacity", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "15", "1", "opacity reduction of ghost");
            new Level_Variable("mod_heald_duration", FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "1", "duration of HEALD effect");
            new Level_Variable("mod_heald_hps", FCVAR_NOTIFY | FCVAR_REPLICATED, "2", "1.2", "1", "health per second that HEALD will heal");
            new Level_Variable("mod_incendiary_dps", FCVAR_NOTIFY | FCVAR_REPLICATED, "7", "0", "1", "damage per second dealt by incendary grenades");
            new Level_Variable("mod_incendiary_duration", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "duration of incendiary grenade burning effect");
            new Level_Variable("mod_incendiary_radius", FCVAR_NOTIFY | FCVAR_REPLICATED, "140", "0", "1", "radius of incendary grenade effect");
            new Level_Variable("mod_incendiary_tick", FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "damage per second dealt by incendary grenades");
            new Level_Variable("mod_lasers_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "laser damage dealt per tick (with 20 ticks per second)");
            new Level_Variable("mod_lasers_max_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "30", "20", "1", "total damage dealt before a laser shuts down");
            new Level_Variable("mod_magmine_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "10", "3", "1", "damage dealt by exploding magmines");
            new Level_Variable("mod_magmine_fullradius", FCVAR_NOTIFY | FCVAR_REPLICATED, "100", "50", "1", "radius of full magmine effect");
            new Level_Variable("mod_magmine_health", FCVAR_NOTIFY | FCVAR_REPLICATED, "20", "3", "1", "damage a magmine can take before exploding");
            new Level_Variable("mod_magmine_strength", FCVAR_NOTIFY | FCVAR_REPLICATED, "0.85", "0.6", "1", "suction strength of a magmine");
            new Level_Variable("mod_mindabsorb_minpower", FCVAR_NOTIFY | FCVAR_REPLICATED, "52.7778", "-2.7778", "1", "minimum free aux power to trigger mind absorb");
            new Level_Variable("mod_mirvgrenade_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "25", "5", "1", "damage dealt by main MIRV projectile");
            new Level_Variable("mod_mirvgrenade_radius", FCVAR_NOTIFY | FCVAR_REPLICATED, "100", "5", "1", "radius of main MIRV projectile's blast");
            new Level_Variable("mod_mirvlet_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "15", "3", "1", "damage dealt by secondary MIRV projectiles");
            new Level_Variable("mod_mirvlet_radius", FCVAR_NOTIFY | FCVAR_REPLICATED, "100", "5", "1", "radius of secondary MIRV projectiles' blast");
            new Level_Variable("mod_plague_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "0.4938", "0.6173", "1", "damage from each tick of plague effect");
            new Level_Variable("mod_plague_radius", FCVAR_NOTIFY | FCVAR_REPLICATED, "42", "10", "1", "radius at which plague will spread");
            new Level_Variable("mod_poisonspit_amount", FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "2", "1", "number of projectiles created by poison spit. One will always be 'large' size, the rest will randomly be 'medium' or 'small'");
            new Level_Variable("mod_poisonspit_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "2", "2", "1", "damage dealt by main poison spit projectile. Smaller projectiles deal 0.5 or 0.25 times this damage.");
            new Level_Variable("mod_poisonspit_speed", FCVAR_NOTIFY | FCVAR_REPLICATED, "1250", "0", "1", "Launch speed of poison spit projectiles");
            new Level_Variable("mod_recharge_amount", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "armor recharged each tick");
            new Level_Variable("mod_regen_amount", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "damage healed each tick of regeneration");
            new Level_Variable("mod_regen_tick", FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "interval between ticks of regeneration");
            new Level_Variable("mod_runningman", FCVAR_NOTIFY | FCVAR_REPLICATED, "245", "25", "1", "sprint speed with each level of running man");
            new Level_Variable("mod_shockwave_damage", FCVAR_NOTIFY | FCVAR_REPLICATED, "15", "4", "1", "damage dealt by shockwave module"); ;
            new Level_Variable("mod_shockwave_radius", FCVAR_NOTIFY | FCVAR_REPLICATED, "375", "25", "1", "radius of effect of shockwave module");
            new Level_Variable("mod_startarmor", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "10", "1", "armor given by each level of start armor");
            new Level_Variable("mod_vitality", FCVAR_NOTIFY | FCVAR_REPLICATED, "70", "10", "1", "player health capacity with each level of vitality");
            new Level_Variable("mod_weaken_damagescale", FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "-0.05", "1", "damage scaling of weaken effect");
            new Level_Variable("mod_weaken_sprintscale", FCVAR_NOTIFY | FCVAR_REPLICATED, "0.55555556", "-0.055555556", "1", "sprint speed scaling of weaken effect");
            new Level_Variable("upgrade_crossbow_poison_dps", FCVAR_NOTIFY | FCVAR_REPLICATED, "4", "0", "1", "damage per second of melee bleed upgrade effect");
            new Level_Variable("upgrade_crossbow_poison_duration", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "2", "1", "duration of melee bleed upgrade effect");
            new Level_Variable("upgrade_crossbow_poison_tick", FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "interval between ticks of melee bleed upgrade effect");
            new Level_Variable("upgrade_melee_bleed_dps", FCVAR_NOTIFY | FCVAR_REPLICATED, "5", "0", "1", "damage per second of melee bleed upgrade effect");
            new Level_Variable("upgrade_melee_bleed_duration", FCVAR_NOTIFY | FCVAR_REPLICATED, "0", "1", "1", "duration of melee bleed upgrade effect");
            new Level_Variable("upgrade_melee_bleed_tick", FCVAR_NOTIFY | FCVAR_REPLICATED, "1", "0", "1", "interval between ticks of melee bleed upgrade effect");
            */
        }

        // start assembling a level variable for any convar ending in _base, _scale or _power - and if we don't get all 3 by the end, leave them as single
        private void btnLoadConvars_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "All files (*.*)|*.*";
            ofd.Title = "Select convar list file to open";
            if (ofd.ShowDialog() != DialogResult.OK)
                return;

            string[] data = File.ReadAllLines(ofd.FileName);
            ConVar.List.Clear();
            ConVar.MiscList.Clear();
            Level_Variable.MiscList.Clear();
            Module.List.Clear();
            RefreshListView();

            List<string> invalid = new List<string>();
            char[] splitBy = new char[] { ' ', '	' };
            foreach (string line in data)
            {
                if (line.Length == 0)
                    continue;

                string[] split = line.Split(splitBy, StringSplitOptions.RemoveEmptyEntries);
                if (split.Length < 2)
                {
                    invalid.Add(split[0]);
                    continue;
                }
                new ConVar(split[0], split[1]);
            }

            // now any "partial" level variables remaining can't be full proper ones, so break them up
            foreach (Level_Variable.Potential p in Level_Variable.potentialLevelVars.Values)
                p.BreakUp();

            RefreshListView();

            if (invalid.Count > 0)
            {
                StringBuilder sb = new StringBuilder();
                foreach (string s in invalid)
                    sb.AppendFormat(", {0}", s);
                MessageBox.Show(sb.ToString().Substring(2), "Invalid ConVars", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void btnLoadValues_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "Config files (*.cfg)|*.cfg|All files (*.*)|*.*";
            ofd.Title = "Select convar value file to open";
            if (ofd.ShowDialog() != DialogResult.OK)
                return;

            string[] data = File.ReadAllLines(ofd.FileName);

            // reset all convars to default
            foreach (KeyValuePair<string, ConVar> kvp in ConVar.List)
                kvp.Value.Value = kvp.Value.Default;

            // now for each line in the file, find the ConVar with that name and update it
            List<string> unrecognised = new List<string>();
            foreach (string line in data)
            {
                int firstSpace = line.IndexOf(' ');
                if ( firstSpace == -1 )
                    firstSpace = line.IndexOf('	');
                string name = line.Substring(0, firstSpace).Trim();
                string value = line.Substring(firstSpace + 1).Trim();
                if (!ConVar.List.ContainsKey(name))
                {
                    if ( name != "mc_use_defaults" )
                        unrecognised.Add(name);
                    continue;
                }

                ConVar.List[name].Value = value;
            }

            if (unrecognised.Count > 0)
            {
                StringBuilder sb = new StringBuilder();
                foreach (string s in unrecognised)
                    sb.AppendFormat(", {0}", s);
                MessageBox.Show(sb.ToString().Substring(2), "Unrecognised ConVars", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            RefreshListView();
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "Config files (*.cfg)|*.cfg|All files (*.*)|*.*";
            sfd.Title = "Enter filename to save";
            //sfd.InitialDirectory = ; // look up steam directory?
            sfd.CheckFileExists = false;
            if (sfd.ShowDialog() != DialogResult.OK)
                return;

            StringBuilder sb = new StringBuilder();
            sb.AppendLine("mc_use_defaults 1"); // when this script is loaded, any non-standard values will be cleared
            foreach (KeyValuePair<string, ConVar> kvp in ConVar.List)
                if (!kvp.Value.IsDefault)
                    sb.AppendLine(kvp.Value.Name + " " + kvp.Value.Value);
            File.WriteAllText(sfd.FileName, sb.ToString());
        }

        private void txtFilter_TextChanged_1(object sender, EventArgs e)
        {
            RefreshListView();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            RefreshListView();
        }

        private void RefreshListView()
        {
            allowCheckChange = true;

            lstMiscConvars.Items.Clear();
            foreach (KeyValuePair<string, ConVar> kvp in ConVar.MiscList)
                if (txtFilter.Text == string.Empty || kvp.Key.Contains(txtFilter.Text))
                    lstMiscConvars.Items.Add(kvp.Key, kvp.Value.IsDefault);
            foreach (KeyValuePair<string, Level_Variable> kvp in Level_Variable.MiscList)
                if (txtFilter.Text == string.Empty || kvp.Key.Contains(txtFilter.Text))
                    lstMiscConvars.Items.Add(kvp.Key, kvp.Value.IsDefault);

            lstAllModules.Items.Clear();
            foreach (KeyValuePair<string, Module> kvp in Module.List)
                if (txtFilter.Text == string.Empty || kvp.Key.Contains(txtFilter.Text))
                    lstAllModules.Items.Add(kvp.Key, kvp.Value.IsDefault);
            
            allowCheckChange = false;
        }

        bool allowCheckChange = false;

        private void checkedListBox1_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            if ( !allowCheckChange )
                e.NewValue = e.CurrentValue;
        }

        private void btnReset_Click(object sender, EventArgs e)
        {
            txtFilter.Text = "";
            lstMiscConvars.SelectedIndex = -1;
            RefreshListView();
        }

        private void checkedListBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lstMiscConvars.SelectedIndex == -1)
            {
                conVarControl1.Var = null;
                levelVarControl1.Var = null;
                conVarControl1.Visible = levelVarControl1.Visible = false;
            }
            else
            {
                string item = lstMiscConvars.SelectedItem as string;
                if (ConVar.MiscList.ContainsKey(item))
                {
                    conVarControl1.Var = ConVar.MiscList[item];
                    conVarControl1.Visible = true;
                    levelVarControl1.Visible = false;
                }
                else if (Level_Variable.MiscList.ContainsKey(item))
                {
                    levelVarControl1.Var = Level_Variable.MiscList[item];
                    levelVarControl1.Visible = true;
                    conVarControl1.Visible = false;
                }
            }
        }

        private void lstAllModules_SelectedIndexChanged(object sender, EventArgs e)
        {
            if ( lstAllModules.SelectedIndex != -1 )
            {
                Module m = Module.List[lstAllModules.SelectedItem as string];
                moduleVarPanel.Controls.Clear();
                bool first = true;
                foreach ( Level_Variable var in m.levelVars )
                {
                    LevelVarControl control = new LevelVarControl();
                    control.Var = var;
                    if (groupMode == ExpansionMode.One)
                        if (!first)
                            control.Compressed = true;
                        else
                            first = false;
                    moduleVarPanel.Controls.Add(control);
                    control.Expanded += new EventHandler(moduleControl_Expanded);
                    control.Changed += new EventHandler(levelVarControl1_Changed);
                }
                foreach (ConVar var in m.convars)
                {
                    ConVarControl control = new ConVarControl();
                    control.Var = var;
                    if (groupMode == ExpansionMode.One)
                        if (!first)
                            control.Compressed = true;
                        else
                            first = false;
                    moduleVarPanel.Controls.Add(control);
                    control.Expanded += new EventHandler(moduleControl_Expanded);
                    control.Changed += new EventHandler(conVarControl1_Changed);
                }
            }
        }

        void moduleControl_Expanded(object sender, EventArgs e)
        {
            if (groupMode == ExpansionMode.All)
                return;
            IVarControl control = sender as IVarControl;
            foreach (IVarControl c in moduleVarPanel.Controls)
                if (c.Compressed == false && c != control)
                    c.Compressed = true;
        }

        void conVarControl1_Changed(object sender, EventArgs e)
        {
            // update each list to show whether this var / module is default or not
            ConVarControl control = sender as ConVarControl;
            allowCheckChange = true;

            // first the "misc" list
            int index = lstMiscConvars.Items.IndexOf(control.Var.Name);
            if (index > -1)
                lstMiscConvars.SetItemChecked(index, control.Var.IsDefault);

            // and the "modules" list - find this var's module!
            if (control.Var.Module != null)
            {
                index = lstAllModules.Items.IndexOf(control.Var.Module.Name);
                if (index > -1)
                {
                    CheckState state;
                    if ( !control.Var.Module.IsEnabled )
                        state = CheckState.Unchecked;     // unchecked for disabled modules
                    else if ( control.Var.Module.IsDefault )
                        state = CheckState.Checked;       // checked for default modules
                    else
                        state = CheckState.Indeterminate; // indeterminate for non-default modules
                    lstAllModules.SetItemCheckState(index, state);
                }
            }

            allowCheckChange = false;
        }

        void levelVarControl1_Changed(object sender, EventArgs e)
        {
            // update the convar list and the level var list to show whether these values are default or not
            LevelVarControl control = sender as LevelVarControl;
            allowCheckChange = true;

            // first the "misc" list
            int index = lstMiscConvars.Items.IndexOf(control.Var.Name);
            if (index > -1)
            {
                allowCheckChange = true;
                lstMiscConvars.SetItemChecked(index, control.Var.IsDefault);
                allowCheckChange = false;
            }

            // and the "modules" list - find this var's module!
            if (control.Var.Module != null)
            {
                index = lstAllModules.Items.IndexOf(control.Var.Module.Name);
                if (index > -1)
                    lstAllModules.SetItemChecked(index, control.Var.Module.IsDefault);
            }

            allowCheckChange = false;
        }

        enum ExpansionMode
        {
            One,
            All,
        }

        ExpansionMode groupMode = ExpansionMode.One;
        private void tabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            rbOne.Visible = rbAll.Visible = lblExpand.Visible = (tabControl1.SelectedIndex != 0);
        }

        private void rbOne_CheckedChanged(object sender, EventArgs e)
        {
            if (!rbOne.Checked)
                return;
            groupMode = ExpansionMode.One;
            bool first = true;
            foreach (IVarControl control in moduleVarPanel.Controls)
            {
                control.Compressed = !first;
                first = false;
            }
        }

        private void rbAll_CheckedChanged(object sender, EventArgs e)
        {
            if (!rbAll.Checked)
                return;
            groupMode = ExpansionMode.All;
            foreach (IVarControl control in moduleVarPanel.Controls)
                control.Compressed = false;
        }
    }
}
