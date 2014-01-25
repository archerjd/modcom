#ifndef VORTEX_MSG_H
#define VORTEX_MSG_H
#pragma once

// this macro to be used for defining a multi-level variable by means of 3 convars, to be accessed through the LEVEL macro. base + scale * level ^ power is the formula used for all of these.
#define LEVEL_VARIABLE(name, flags, base, scale, power, description) \
	McConVar name##_base( #name "_base", base , flags, "Base value for " description ); \
	McConVar name##_scale( #name "_scale", scale , flags, "Scale factor for " description ); \
	McConVar name##_power( #name "_power", power , flags, "Scaling power for " description );

// use these to get the value for a given level of a base/scale/power variable
#define LEVEL2(base, scale, power, level) (level == 0 ? 0 : (power == 1.0f ? (base + scale * level) : (base + scale * pow((float)level, power))))
#define LEVEL(name, level) LEVEL2(name##_base.GetFloat(), name##_scale.GetFloat(), name##_power.GetFloat(), level)

#define LEVEL_POINTER(name, level) (level == 0 ? 0 : (name##_power->GetFloat() == 1.0f ? (name##_base->GetFloat() + name##_scale->GetFloat() * level) : (name##_base->GetFloat() + name##_scale->GetFloat() * pow((float)level, name##_power->GetFloat()))))

#define EXPAND_LEVEL(name) name##_base.GetFloat(), name##_scale.GetFloat(), name##_power.GetFloat()

// use this for easily externing all convars of a variable
#define LEVEL_EXTERN(name) McConVar name##_base, name##_scale, name##_power

const char *DoubleHashPassword(const char *passHash);
void ClearStoredPasswordHash();

#define ALLY_ICON_VERTICAL_OFFSET	36

// reasons for logon to fail ... password confirmation mismatch is handled on client, so isn't here
enum
{
	NO_MESSAGE = 0,
	ACCOUNT_DOESNT_EXIST,
	ACCOUNT_ALREADY_EXISTS,
	PASSWORD_INCORRECT,
	ACCOUNT_ALREADY_LOGGED_IN,
	MANUAL_LOG_OUT,
};

enum
{
	PREGAME = 0,
	DEATHMATCH = 1,
	RANDOM_PVM = 2,
	FFA = 3,
	TEAM_DEATHMATCH = 4,
	HOARDER = 5,
	NUM_GAME_MODES = HOARDER,
};

enum
{
	FACTION_NONE = 0,
	FACTION_COMBINE,
	FACTION_RESISTANCE,
	FACTION_APERTURE,
	NUM_FACTIONS = FACTION_APERTURE,
};

// reasons for character creation to fail
enum
{
	NO_ERROR = 0,
	DUPLICATE_NAME,
	INVALID_CLASS,
	INVALID_MODEL,
};

#define NAME_LENGTH	28 // for consistency
#define MODEL_LENGTH 256
#define MAX_CHARS	12 // max characters per account
#define CHARS_PER_PAGE 3 // character selection currently uses pages, rather than showing all at once

// tabs on the main menu - here so we can request specific pages to be default
enum
{
	TAB_GAME = 0,
	TAB_CHARACTER,
	TAB_MODULES,
	TAB_WEAPONS,
	TAB_TALENTS,

	NUM_TABS,
};

// this needs a global define, alas
#define MAX_MAGMINES_PER_PLAYER 1
#define MAX_MAGMINES MAX_PLAYERS * MAX_MAGMINES_PER_PLAYER

// vote flags
#define VOTE_FLAG_ANYTIME		(1 << 0)
#define VOTE_FLAG_END_OF_ROUND	(1 << 1)
#define	VOTE_FLAG_SINGLE		(1 << 2) // game mode only, and only if flag 1 is enabled
#define VOTE_FLAG_NOMINATIONS	(1 << 2) // map only
#define VOTE_FLAG_PRELIMINARY	(1 << 3)


#ifndef _WIN32
	#define max(a,b) (a) > (b) ? (a) : (b)
	#define min(a,b) (a) < (b) ? (a) : (b)
#endif

#endif