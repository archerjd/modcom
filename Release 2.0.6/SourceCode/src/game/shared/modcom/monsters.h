#ifndef PVM_H
#define PVM_H

#if defined( _WIN32 )
#pragma once
#endif

#define MAX_NPC_CLASSNAME 28
#define MAX_NPC_TYPE_NAME 32
#define MAX_NPC_TYPES 256
#define BITS_FOR_MAX_NPC_TYPES 8 // 9 is 512, 8 is 256, 7 is 128, 6 is 64
#define MAX_NPC_VAR_NAME 32
#define MAX_NPC_VARS	12
#define MAX_FILE_PATH 64
//#define MAX_MODULES_PER_NPC 10

#ifndef CLIENT_DLL

#include <map>

/*
struct CNPCModule
{
public:
	CNPCModule()
	{
		ModuleNumber = -1;
		LevelOffset = 0;
		UseWeighting = 0;
	}

	CNPCModule(int ability, int levelOffset, int weighting)
	{
		ModuleNumber = ability;
		LevelOffset = levelOffset;
		UseWeighting = weighting;
	}

	int ModuleNumber;
	int LevelOffset;
	int UseWeighting;
};
*/
extern INetworkStringTable *g_pNPCNamesStringTable;

struct CNPCTypeInfo
{
public:
	CNPCTypeInfo()
	{
		Undefined = true;
		flRandomSpawnFrequency = 0;
		m_iNameIndex = 0;
		m_iSkinNumber = -1;
		m_iNumVars = 0;
		m_flExperienceScale = 1;
		m_iBitsDisabledCapacities = 0;
		m_iSkinOverride = 0;

		m_bHasEffectColor = false;
		m_effectColor = Color(0,0,0,0);
	}

	CNPCTypeInfo(const char *szClassname, const char *szTypeName, float spawnFrequency);
	const char *GetSoundOverride(const char *soundName);

	char m_szClassname[MAX_NPC_CLASSNAME];
	char m_szTypeName[MAX_NPC_TYPE_NAME];
	char m_szModelName[MAX_FILE_PATH];
	float flRandomSpawnFrequency;
	int m_iNameIndex;
	int m_iSkinNumber; // not skin override ... model's skin number
	
	int m_iSkinOverride;

	bool m_bHasEffectColor;
	Color m_effectColor;

	float m_flExperienceScale;
	bool m_bIsBoss;

	int GetVarIndex(const char *varName);
	float GetVarValue(const char *varName, int level);
	float GetVarValue(int varIndex, int level);

	char m_szVarNames[MAX_NPC_VARS][MAX_NPC_VAR_NAME];
	float m_flVarsBase[MAX_NPC_VARS], m_flVarsScale[MAX_NPC_VARS], m_flVarsPower[MAX_NPC_VARS], m_flVarsLimit[MAX_NPC_VARS];
	int m_iVarLimitType[MAX_NPC_VARS]; // 0 = none, 1 = min, 2 = max
	int m_iNumVars;

	std::multimap<std::string,std::string> m_SoundList;
	
	int m_iBitsDisabledCapacities;

	bool Undefined;
};

#endif

#endif // PVM_H