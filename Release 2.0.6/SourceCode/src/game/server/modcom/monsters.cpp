#include "cbase.h"
#include "modcom/monsters.h"
#include "filesystem.h"
#include "utlbuffer.h"
#include <string>
#include "materialsystem/imaterial.h"
#include "hl2mp/hl2mp_gamerules.h"
#include "ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CUtlVector<CNPCTypeInfo*> g_pNPCInfo;
float g_flRandomSpawnFrequencySum = 0.0f;
extern INetworkStringTable *g_pNPCNamesStringTable;
extern INetworkStringTable *g_pSkinOverrideStringTable;
extern bool AddDownload(const char *file);
extern void KillAllNPCsWithMaster(CHL2MP_Player *master, bool minionsOnly);

extern McConVar pvm_wave_interval, pvm_max_monsters, pvm_monsters_base, pvm_monsters_per_player, pvm_max_bosses, pvm_max_monster_level_offset_below_player_average,
				gamemode_hoarder_token_add_interval;

ConVar mc_dev_spawnMonsterlevel("mc_dev_spawnMonsterlevel", "6", FCVAR_CHEAT, "The default level for mc_dev_createnpc command");

void AddMaterialDownload(const char* matFile)
{
	AddDownload(matFile);
			
	// if we have a custom material, then we must add its textures to the download table also.
	// Open it up and read through each key - we'll count everything with a '/' in as a texture.
	KeyValues *mat = new KeyValues("mat");
	if ( mat->LoadFromFile( g_pFullFileSystem, matFile, "MOD") )
	{
		for ( KeyValues *sub = mat->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
			if ( Q_strrchr( sub->GetString(), '/' ) )
			{
				const char *textureFile = UTIL_VarArgs("materials/%s.vtf",sub->GetString());
				AddDownload(textureFile);
			}
	}
	mat->deleteThis();
}

void AddModelDownload(const char* modelName)
{
	//AddDownload(modelName);

	// get a copy of the model name with "mdl" chopped off the end, and a * added on
	char filePattern[MAX_FILE_PATH];
	Q_snprintf(filePattern, sizeof(filePattern), modelName);
	filePattern[strlen(filePattern)-3] = '*';
	filePattern[strlen(filePattern)-2] = '\0';

	// now find all files that match the given one, with any given extensions
	FileFindHandle_t fh;
	for (const char *pszName=filesystem->FindFirstEx(filePattern,"MOD",&fh); pszName; pszName=filesystem->FindNext(fh)) // finds files in the mod dir only
	//for (const char *pszName=filesystem->FindFirst(filePattern,&fh); pszName; pszName=filesystem->FindNext(fh)) // also finds files in gcfs and other game dirs
		AddDownload(UTIL_VarArgs("models/%s", pszName)); // this catches the .mdl as well as any other files with similar extensions
	g_pFullFileSystem->FindClose( fh );

	// now we want to get at the materials loaded by that model
/*	int modelIndex = CBaseEntity::PrecacheModel(modelName); // when this line is run, it messes up the filename of the "loaded X from Y" message for the vort scientist
	const model_t *model = const_cast<model_t *>(modelinfo->GetModel(modelIndex));
	int materialCount = modelinfo->GetModelMaterialCount(model);
	Msg("Got %i materials for model: %s (%i)\n", materialCount, modelName, modelIndex);
	IMaterial *pMaterial;
	for ( int i=1; i<=materialCount; i++ )
	{
		modelinfo->GetModelMaterials( model, i, &pMaterial );
		AddMaterialDownload(UTIL_VarArgs("materials/%s.vmt",pMaterial->GetName()));
	}*/
}

void pvmdir_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue );
ConVar pvmdir( "pvmdir", "pvm", FCVAR_ARCHIVE, "Directory used to load PVM monster scripts from. Don't begin or end the value with a slash!", pvmdir_ChangeCallback );
void pvmdir_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue )
{
	// check directory exists ... if it does not, set value back to previous
	if ( !g_pFullFileSystem->FileExists(pvmdir.GetString(), "mod") )
	{
		Msg("Directory doesn't exist, reverting\n");
		pvmdir.SetValue(pOldString);
	}
}

CNPCTypeInfo::CNPCTypeInfo(const char *szClassname, const char *szTypeName, float spawnFrequency)
{
	Q_snprintf(m_szClassname, sizeof(m_szClassname), szClassname);
	Q_snprintf(m_szTypeName, sizeof(m_szClassname), szTypeName);
	flRandomSpawnFrequency = spawnFrequency;
	m_iNameIndex = g_pNPCNamesStringTable->AddString( true, szTypeName );
	m_iSkinOverride = 0;
	m_iSkinNumber = -1;
	m_szModelName[0] = '\0';
	m_iNumVars = 0;
	m_bIsBoss = false;
	m_bHasEffectColor = false;
	
	m_iBitsDisabledCapacities = 0;
	
	//numModules = 0;
	Undefined = false;
}

const char *CNPCTypeInfo::GetSoundOverride(const char *soundName)
{
	// returns a randomly selected value from the options available, or NULL if there are no overrides for this sound.
	int num = (int)m_SoundList.count(soundName);
	if ( num == 0 )
		return NULL;

	std::multimap<std::string,std::string>::iterator it;
	int index = random->RandomInt(0,num-1);
	int pos = 0;
    for (it=m_SoundList.equal_range(soundName).first; it!=m_SoundList.equal_range(soundName).second; ++it)
		if ( pos == index )
			return (*it).second.c_str();
		else
			pos++;
	
	// should never get this far!
	Warning("GetSoundOverride stepped past all sounds for '%s' ... this is wrong!\n", soundName);
	return NULL;
}

void Load(const char* filename)
{
	KeyValues *stats = new KeyValues("stats");
    if ( !stats->LoadFromFile( g_pFullFileSystem, UTIL_VarArgs("%s/%s",pvmdir.GetString(), filename), "MOD" ) )
    {
        Warning( "Unable to load %s\n", filename );
		return;
	}

	float frequency = stats->GetFloat("random_spawn_frequency", 0.0f);
	const char *className = stats->GetString("class", "npc_antlion");
	const char *typeName = stats->GetString("name", "Unknown NPC");
	
	CNPCTypeInfo *info = new CNPCTypeInfo(className, typeName, frequency);	
	info->m_bIsBoss = stats->GetInt("boss", 0) != 0;
	info->m_flExperienceScale = stats->GetFloat("experience_scale", 1.0f);
		
	const char *modelName = stats->GetString("model", NULL);
	if ( modelName )
	{
		Q_snprintf(info->m_szModelName, sizeof(info->m_szModelName), modelName);
		AddModelDownload(modelName);
	}
	
	// custom models will require their materials to be downloaded also. Can't detect them automatically on the server, it would seem, so list them manually
	KeyValues *materials = stats->FindKey("materials", false);
	if ( materials != NULL )
		for ( KeyValues *sub = materials->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
			AddMaterialDownload(UTIL_VarArgs("materials/%s",sub->GetString())); // also handles downloading textures contained within the material

	info->m_iSkinNumber = stats->GetInt("skin_number", -1);

	KeyValues *skin = stats->FindKey("skin", false);
	if ( skin != NULL )
		if ( info->m_iSkinNumber != -1 ) // can't have skin number AND skin override set!
			Warning("Monster '%s' has a skin number set, but also specifies skin overrides! Can't use both these features!", filename);
		else
			{
				const char *skinName = skin->GetString();
				info->m_iSkinOverride = g_pSkinOverrideStringTable->AddString( true, skinName);
				AddMaterialDownload(UTIL_VarArgs("materials/%s",skinName)); // also handles downloading textures contained within the material
			}
			/*for ( KeyValues *sub = skin->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
			{
				int index = atoi(sub->GetName());
				if ( index <= 0 || index > MAX_SKIN_OVERRIDES )
					continue;
				
				const char *skinName = sub->GetString();
				info->m_iSkinOverrides[index-1] = g_pSkinOverrideStringTable->AddString( true, skinName);
				AddMaterialDownload(UTIL_VarArgs("materials/%s",skinName)); // also handles downloading textures contained within the material
			}*/
			
	info->m_bHasEffectColor = stats->FindKey("effect_color", false) != NULL;
	if ( info->m_bHasEffectColor )
		info->m_effectColor = stats->GetColor("effect_color");
	
	if ( stats->GetInt("disable_ranged_attack1", 0) == 1 )
		info->m_iBitsDisabledCapacities |= bits_CAP_INNATE_RANGE_ATTACK1;
	if ( stats->GetInt("disable_ranged_attack2", 0) == 1 )
		info->m_iBitsDisabledCapacities |= bits_CAP_INNATE_RANGE_ATTACK2;
	if ( stats->GetInt("disable_melee_attack1", 0) == 1 )
		info->m_iBitsDisabledCapacities |= bits_CAP_INNATE_MELEE_ATTACK1;
	if ( stats->GetInt("disable_melee_attack2", 0) == 1 )
		info->m_iBitsDisabledCapacities |= bits_CAP_INNATE_MELEE_ATTACK2;

	if ( stats->GetInt("disable_jumping", 0) == 1 )
		info->m_iBitsDisabledCapacities |= bits_CAP_MOVE_JUMP;
	if ( stats->GetInt("disable_flying", 0) == 1 )
		info->m_iBitsDisabledCapacities |= bits_CAP_MOVE_FLY;
	if ( stats->GetInt("disable_walking", 0) == 1 )
		info->m_iBitsDisabledCapacities |= bits_CAP_MOVE_GROUND;

	// now read all npc vars
	KeyValues *vars = stats->FindKey("vars", false);
	if ( vars == NULL )
		Warning(UTIL_VarArgs("Can't find vars section of %s!\n", filename));
	else
		for ( KeyValues *sub = vars->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
		{
			if ( info->m_iNumVars >= MAX_NPC_VARS )
			{
				Warning(UTIL_VarArgs("%s contains too many vars - the maximum is %i!\n", filename, MAX_NPC_VARS));
				break;
			}
			Q_snprintf(info->m_szVarNames[info->m_iNumVars], MAX_NPC_VAR_NAME, sub->GetName());
			
			// now gotta split the value up into parts and store it in the member floats
			CUtlVector<char*> bits;
			V_SplitString( sub->GetString(), " ", bits );
			
			// either 1, 3 or 5 bits (base), (base, scale, power) or (base, scale, power, limitType, limit)
			if ( bits.Count() == 1 )
			{
				info->m_flVarsBase[info->m_iNumVars] = atof(bits[0]);
				info->m_flVarsScale[info->m_iNumVars] = 0;
				info->m_flVarsPower[info->m_iNumVars] = 0;
				info->m_iVarLimitType[info->m_iNumVars] = 0;
				info->m_flVarsLimit[info->m_iNumVars] = 0;
			}
			else if ( bits.Count() > 2 )
			{
				info->m_flVarsBase[info->m_iNumVars] = atof(bits[0]);
				info->m_flVarsScale[info->m_iNumVars] = atof(bits[1]);
				info->m_flVarsPower[info->m_iNumVars] = atof(bits[2]);
				
				if ( bits.Count () > 4 ) // has a limit )
				{
					info->m_flVarsLimit[info->m_iNumVars] = atof(bits[4]);
					if ( FStrEq("min", bits[3]) )
						info->m_iVarLimitType[info->m_iNumVars] = 1;
					else if ( FStrEq("max", bits[3]) )
						info->m_iVarLimitType[info->m_iNumVars] = 2;
					else
					{
						Warning( "Invalid limit type (%s) on var %s of %s\n", bits[3], sub->GetName(), filename );
						info->m_iVarLimitType[info->m_iNumVars] = 0;
					}
				}
				else
				{// no limit
					info->m_iVarLimitType[info->m_iNumVars] = 0;
					info->m_flVarsLimit[info->m_iNumVars] = 0;
				}
			}
			else
				Warning(UTIL_VarArgs("Var '%s' in %s should contain 1, 3 or 5 parts (base ... base/scale/power or base/scale/power/limittype/limit)...: %s\n", sub->GetName(), filename, sub->GetString()));
			bits.PurgeAndDeleteElements();
			info->m_iNumVars ++;
		}
	
	KeyValues *sounds = stats->FindKey("sounds", false);
	if ( sounds != NULL )
		for ( KeyValues *sub = sounds->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
		{
			const char *soundFile = sub->GetString();
			CBaseEntity::PrecacheScriptSound(soundFile);
			AddDownload(UTIL_VarArgs("sound/%s",soundFile)); // this sound will need to be sent to clients
			info->m_SoundList.insert(std::pair<std::string,std::string>(sub->GetName(),soundFile)); // and stored so that the npc uses it instead of the default sound
		}
	
	g_pNPCInfo.AddToTail(info);

	g_flRandomSpawnFrequencySum += frequency;
	Msg("Loaded %s stats from %s\n", typeName, filename);
}

int CNPCTypeInfo::GetVarIndex(const char *varName)
{
	for ( int i=0; i<m_iNumVars && i<MAX_NPC_VARS; i++ )
		if ( FStrEq(m_szVarNames[i], varName) )
			return i;
			
	return -1; // its not an error if the var isn't found ... this function is used to make vars optional
}

float CNPCTypeInfo::GetVarValue(const char *varName, int level)
{
	for ( int i=0; i<m_iNumVars && i<MAX_NPC_VARS; i++ )
		if ( FStrEq(m_szVarNames[i], varName) )
		{
			return GetVarValue(i, level);
		}
	
	Warning("NPC var not found: %s\n", varName);
	return 1;
}

float CNPCTypeInfo::GetVarValue(int varIndex, int level)
{
	if ( varIndex < 0 || varIndex >= m_iNumVars || varIndex >= MAX_NPC_VARS )
	{
		Warning("Invalid NPC var index: %i\n", varIndex);
		return 1;
	}

	float val = m_flVarsBase[varIndex] + m_flVarsScale[varIndex] * pow((float)level, m_flVarsPower[varIndex]);
	switch ( m_iVarLimitType[varIndex] )
	{
		case 1: // min
			return max(m_flVarsLimit[varIndex], val);
		case 2: // max
			return min(m_flVarsLimit[varIndex], val);
		default:
			return val;
	}
}

CNPCTypeInfo *g_pNPCErrorType = NULL;
void LoadMonsterStats()
{
	g_flRandomSpawnFrequencySum = 0.0f;
	g_pNPCInfo.RemoveAll(); // PurgeAndDeleteAll()
	//g_pNPCNamesStringTable->RemoveAll(); // this isn't an option... so pvm_reloadstats may not be an option!

	int numTypes = 1;
	g_pNPCNamesStringTable->AddString( true, "Unnamed NPC" );
	g_pSkinOverrideStringTable->AddString( true, " " );
	FileFindHandle_t findHandle;
	for (const char *pszName=filesystem->FindFirst("pvm/*.txt",&findHandle); pszName; pszName=filesystem->FindNext(findHandle))
		if ( numTypes < MAX_NPC_TYPES )
		{
			Load(pszName);
			numTypes++;
		}
		else
			Warning(UTIL_VarArgs("Unable to load %s, as limit of %i npc types has been reached!\n", pszName, (MAX_NPC_TYPES-1)));

	if ( g_pNPCErrorType == NULL )
		 g_pNPCErrorType = new CNPCTypeInfo("npc_antlion", "Error loading NPCs!", 0.0f);
		 
#ifdef DEBUG
	Msg("g_pSkinOverrideStringTable contains %i values\n", g_pSkinOverrideStringTable->GetNumStrings());
#endif
}

CNPCTypeInfo *randomType = new CNPCTypeInfo();
CNPCTypeInfo *GetNPCInfo(const char *name)
{
	int numNpcTypes = g_pNPCInfo.Count();
	for ( int i=0; i<numNpcTypes; i++ )
		if ( FStrEq(g_pNPCInfo[i]->m_szTypeName, name) )
			return g_pNPCInfo[i];
	return randomType;
}




// from here on, gamerules methods related to spawning monsters... as the gamerules file was getting unmanagably big

// how many pvm monsters are there right now
// if a player parameter is given, returns how many are targetting that player,
// otherwise returns how many there are in general
int CHL2MPRules::NumMonsters(CHL2MP_Player* pPlayer)
{
	int num = 0;
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	bool bSpecificPlayer = (pPlayer != NULL);
	for ( int i = 0; i < nAIs; i++ )// look through all the AIs and check each
	{
		CAI_BaseNPC *pNPC = ppAIs[i];
		if ( !pNPC )
			continue;

		if ( !pNPC->LikesMaster() )
		{
			if ( bSpecificPlayer )
			{
				if ( pNPC->GetMasterPlayer() == pPlayer )
					num ++;
			}
			else
				num ++;
		}
	}

	return num;
}

CBaseEntity *CHL2MPRules::GetNpcSpawnPoint(bool largeSpawnsOnly)
{
	CUtlVector<CBaseEntity*> *list = largeSpawnsOnly ? &m_pLargeNpcSpawns : &m_pNpcSpawns;
	
	// pick a random index in m_pNpcSpawns, try spawning.
	int max = list->Count() - 1;
	if ( max < 0 )
		return NULL;
	int startIndex = random->RandomInt(0,max);

	if ( IsSpawnPointValid(list->Element(startIndex),NULL) )
		return list->Element(startIndex);
	
	// if that doesn't work, cycle through all the spawns (looping back to the start), and if that doesnt work, return null (fail)
	for ( int i=startIndex+1; i<=max; i++ )
		if ( IsSpawnPointValid(list->Element(i),NULL) )
			return list->Element(i);

	for ( int i=0; i<startIndex; i++ )
		if ( IsSpawnPointValid(list->Element(i),NULL) )
			return list->Element(i);

	return NULL;
}

void CHL2MPRules::SpawnMonsterWave()
{
	m_flNextWave = gpGlobals->curtime + pvm_wave_interval.GetFloat();

	int numPlayers = NumPlayers();
	if ( numPlayers == 0 )
		return;
	
	int numMonsters = NumMonsters();
	int desiredNumMonsters = min( pvm_max_monsters.GetInt(), pvm_monsters_base.GetInt() + pvm_monsters_per_player.GetInt() * numPlayers );
	int numToSpawn = max(0, desiredNumMonsters - numMonsters);

	if ( numToSpawn > 0 )
		SpawnMonster(randomType,NULL,numToSpawn);
}

// classname parameter can be null (will be a random monster type for each player)
void CHL2MPRules::SpawnMonsterForEachPlayer(CNPCTypeInfo *typeInfo, int num)
{
	int numMonsters = NumMonsters();
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
		if ( !pPlayer || !pPlayer->IsInCharacter() )
			continue;

		for ( int j=0; j<num; j++ )
		{
			if ( typeInfo->Undefined )
				typeInfo = GetRandomMonsterType();

			if ( SpawnMonster(typeInfo, pPlayer) )
				numMonsters ++;
			else
				Warning("Error spawning monster %s!\n",typeInfo->m_szClassname);
		}
	}
}

// all parameters can be null (random class & player will be selected for each, and will spawn 1)
bool CHL2MPRules::SpawnMonster(CNPCTypeInfo *typeInfo, CHL2MP_Player *pPlayer, int num)
{
	bool bRandomPlayer = pPlayer == NULL;
	bool bRandomType = typeInfo->Undefined;

	bool retVal = true;
	
	for ( int i=0; i<num; i++ )
	{
		bool mustChangeType = false;
		CBaseEntity *pSpawn;
		do
		{
			if ( bRandomType )
				typeInfo = GetRandomMonsterType();
			if ( bRandomPlayer )
				pPlayer = PickPlayerWithLeastMonstersTargetting();

			bool bNeedsLargeSpawnPoint = FStrEq(typeInfo->m_szClassname, "npc_antlionguard");
			pSpawn = GetNpcSpawnPoint(bNeedsLargeSpawnPoint);

			mustChangeType = pSpawn == NULL && bNeedsLargeSpawnPoint && bRandomType;
			if ( mustChangeType )
				Msg("Can't spawn antlion guard, got no place for it\n");

			if (typeInfo->m_bIsBoss)
			{
				if(HL2MPRules()->GetNumBossNpcs() >= pvm_max_bosses.GetInt())
				{
					Msg(UTIL_VarArgs("Can't spawn %s, boss limit reached\n",typeInfo->m_szTypeName));
					mustChangeType = true;
				}
				else
					BossNpcAdded();
			}
		}	
		while ( mustChangeType ); // if there's no large spawn points, don't spawn guards. If there are too many bosses, don't spawn bosses.

		if ( pSpawn == NULL )
		{
			retVal = false; // can't find another spawn point, fail
			break;
		}
		if ( pPlayer == NULL )
		{
			retVal = false; // can't find a player to target, fail
			break;
		}

		retVal = retVal && SpawnMonster(typeInfo,pSpawn->GetAbsOrigin(),GetRandomAngle(), pPlayer);
	}
	
	return retVal;
}

bool CHL2MPRules::SpawnMonster(CNPCTypeInfo *typeInfo, Vector origin, QAngle angles, CHL2MP_Player *pPlayer)
{
	CAI_BaseNPC *pNPC = static_cast<CAI_BaseNPC*>(CBaseEntity::CreateNoSpawn( typeInfo->m_szClassname, origin, angles ));
	if ( pNPC )
	{
		int level;
		if ( pPlayer )
		{
			level = CalculateMonsterLevelForPlayer(pPlayer);
		}
		else
		{
			int min, max, average;
			GetPlayerLevelDistribution(min,max,average);
			pPlayer = PickPlayerWithLeastMonstersTargetting();
			level = average; // just average for now
		}
		
		if ( ShouldUseScoreTokens() && m_flNextScoreTokenSpawnTime <= gpGlobals->curtime )
		{
			if ( GetTotalNumScoreTokens() < GetTargetScoreTokenLimit() )
				pNPC->AddScoreToken();
			m_flNextScoreTokenSpawnTime += gamemode_hoarder_token_add_interval.GetFloat(); // whether we're at the limit or not, update the counter
		}

		pNPC->AddSpawnFlags( SF_NPC_FALL_TO_GROUND | SF_NPC_FADE_CORPSE );
		pNPC->SetStats( typeInfo, level ); // calls DispatchSpawn
		
		pNPC->ApplyBuff(BUFF_SPAWN); // be briefly invisible

		pNPC->SetActivity( ACT_IDLE );
		pNPC->PhysicsSimulate();

		//pNPC->SetNextThink(gpGlobals->curtime + GetBuff(BUFF_SPAWN_MONSTER)->GetDuration(1));
		pNPC->Activate();
		
		pNPC->SetMasterPlayer(pPlayer);
		pNPC->SetLikesMaster(false);

		return true;
	}
	
	return false;
}

// Purpose: Create an NPC of the given type
void CC_Dev_CreateNPC( const CCommand& args )
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	int num = args.ArgC();
	if ( pPlayer == NULL || num < 2 )
		return;

	CNPCTypeInfo *pStats;
	
	switch ( num )
	{
	case 2:
		pStats = GetNPCInfo(args[1]); break;
	case 3:
		pStats = GetNPCInfo(UTIL_VarArgs("%s %s",args[1],args[2])); break;
	//case 4:
	default:
		pStats = GetNPCInfo(UTIL_VarArgs("%s %s %s",args[1],args[2],args[3])); break;
	}

	if ( pStats == NULL )
	{
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "NPC type not recognised, unable to spawn NPC\n");
		return;
	}

	// Now find position & angle
	trace_t tr;
	Vector forward;
	pPlayer->EyeVectors( &forward );

	UTIL_TraceLine(pPlayer->EyePosition(),
		pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH,MASK_SOLID, 
		pPlayer, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 1.0 )
	{
		// Raise the end position a little up off the floor, place the npc and drop him down
		tr.endpos.z += 12;
	}

	CAI_BaseNPC *pNPC = static_cast<CAI_BaseNPC*>(CBaseEntity::CreateNoSpawn( pStats->m_szClassname, tr.endpos, QAngle(0,0,0) ));

	//If the NPC is a boss, we want to restrict its spawn amount
	if ( pStats->m_bIsBoss )
	{
		if( HL2MPRules()->GetNumBossNpcs() >= pvm_max_bosses.GetInt())
		{
			Msg(UTIL_VarArgs("Can't spawn %s, boss limit reached\n",pStats->m_szTypeName));
			return;
		}
		else
			HL2MPRules()->BossNpcAdded();
	}

	if (pNPC)
	{
		int level = mc_dev_spawnMonsterlevel.GetInt(); //using the convar to determine level

		pNPC->AddSpawnFlags( SF_NPC_FADE_CORPSE );
		pNPC->SetStats( pStats, level ); // calls DispatchSpawn

		pNPC->ApplyBuff( BUFF_SPAWN);

		pNPC->SetActivity( ACT_IDLE );
		pNPC->PhysicsSimulate();

		//pNPC->SetNextThink(gpGlobals->curtime + GetBuff(BUFF_SPAWN_MONSTER)->GetDuration(1));
		pNPC->Activate();
		
		pNPC->SetMasterPlayer(pPlayer);
		pNPC->SetLikesMaster(false);
	}
}
static ConCommand mc_dev_createnpc("mc_dev_createnpc", CC_Dev_CreateNPC, "Spawns a monster with a specified level where the player is looking", FCVAR_CHEAT);

// would be nice if this didn't just work in consecutive order
CHL2MP_Player *CHL2MPRules::PickPlayerWithLeastMonstersTargetting()
{
	int leastNum = pvm_max_monsters.GetInt();
	CHL2MP_Player *pLeastPlayer = NULL;
	
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
		if ( !pPlayer || !pPlayer->IsInCharacter() )
			continue;
			
		int num = NumMonsters(pPlayer);
		if ( num < leastNum || ( num == leastNum && random->RandomInt(1,100) < 40 ) )
		{
			leastNum = num;
			pLeastPlayer = pPlayer;
		}
	}
	return pLeastPlayer;
}

void CHL2MPRules::GetPlayerLevelDistribution(int &min, int &max, int &mean)
{
	int numPlayers = NumPlayers();
	if ( numPlayers == 0 )
	{
		min = max = mean = 1;
		return;
	}
	
	int levelTotal = 0;
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );
		if ( !pPlayer || !pPlayer->IsInCharacter() )
			continue;
	
		if ( pPlayer->GetLevel() < min )
			min = pPlayer->GetLevel();
		else if ( pPlayer->GetLevel() > max )
			max = pPlayer->GetLevel();
		
		levelTotal += pPlayer->GetLevel();
	}

	mean = RoundFloatToInt((float)levelTotal / (float)numPlayers);
}

// don't spawn monsters much lower than the average player level
int CHL2MPRules::CalculateMonsterLevelForPlayer(CHL2MP_Player *pPlayer)
{
	int min, max, average;
	GetPlayerLevelDistribution(min,max,average);
	float offset = pPlayer->GetLevel() - average; // offset from average

	int offset_limit = pvm_max_monster_level_offset_below_player_average.GetInt();
	average += random->RandomInt(0,1); // possibly add 1 to the monster level
	if ( offset < -offset_limit )
		return average - offset_limit;
	else
		return average + offset;
}

CNPCTypeInfo* CHL2MPRules::GetRandomMonsterType()
{
//	int num = g_pNPCInfo.Count();
//	return g_pNPCInfo.Element( random->RandomInt(0,num-1) ).c_str();

	if ( g_flRandomSpawnFrequencySum == 0.0f )
		return g_pNPCErrorType; // fail, no proper monster info

	float target = random->RandomFloat(0, g_flRandomSpawnFrequencySum);
	float cumulative = 0;
	int pos = -1;
	int posmax = g_pNPCInfo.Count();

	do 
	{
		pos ++;
		cumulative += g_pNPCInfo[pos]->flRandomSpawnFrequency;
	}
	while ( cumulative <= target && pos < posmax );

	//UTIL_ClientPrintAll(HUD_PRINTCONSOLE, UTIL_VarArgs("Target value: %.3f, total: %.3f.\n Position %i selected; %s\n", target, g_flRandomSpawnFrequencySum, pos, g_ppszAllMonsters[pos]));

	return g_pNPCInfo[pos];
}

void CHL2MPRules::ResetPVM()
{
	m_flNextWave = 0;
	KillAllNPCsWithMaster(NULL,false);
}