#include "cbase.h"
#include "hl2mp/hl2mp_gamerules.h"
#include "hl2mp/hl2mp_player.h"

#include "util/md5/md5wrapper.h"
#include <string>

#include "tier0/memdbgon.h"

extern std::string Escape(const char *instr);
extern McConVar mc_perlevel_modulepoints;

bool CHL2MPRules::IsCorrectPassword(int accountID, const char *testPassword)
{
	md5wrapper md5;
	const char *pass = g_pDB->ReadString("SELECT Pass FROM Accounts WHERE ID=%i",accountID);
	std::string doublehash = md5.getHashFromString(DoubleHashPassword(pass));
	ClearStoredPasswordHash();
	return FStrEq(doublehash.c_str(), testPassword);
}

int CHL2MPRules::GetAccountID(const char *accountName)
{
	std::string escaped = Escape(accountName);
	int retVal = g_pDB->ReadInt("SELECT ID FROM Accounts WHERE Name='%s'",escaped.c_str());
	return retVal;
}

int CHL2MPRules::AddUser(const char *logon, const char *pass)
{
	std::string escaped1 = Escape(logon);
	std::string escaped2 = Escape(pass);

	int retVal;
	if ( g_pDB->Command("INSERT INTO Accounts (Name, Pass, Created, LastActive) VALUES ('%s', '%s', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",escaped1.c_str(),escaped2.c_str()) )
		retVal = g_pDB->LastInsertID();
	else
		retVal = -1;

	return retVal;
}

CHL2MP_Player *CHL2MPRules::GetPlayerByAccountID(int accountID)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;
		if ( pPlayer->GetAccountID() == accountID )
			return pPlayer; // thats this player's account, return them
	}
	return NULL;
}

void CHL2MPRules::DeleteAllPlayerData()
{
	g_pDB->BeginTransaction();
	g_pDB->Command("DELETE FROM Characters");
	g_pDB->Command("DELETE FROM Accounts");
	g_pDB->CommitTransaction();

}

void CC_DeleteAllUsers( void )
{
	HL2MPRules()->DeleteAllPlayerData();
}
static ConCommand cc_deleteallusers("delete_all_player_data",CC_DeleteAllUsers, "Deletes all player data, for everyone on the server. Not recommended!", FCVAR_CHEAT);

int CHL2MPRules::GetNumCharacters(CHL2MP_Player *pPlayer)
{
	return g_pDB->ReadInt("select count(ID) from characters where AccountID = %i", pPlayer->GetAccountID());
}

bool CHL2MPRules::PlayerOwnsCharacter(CHL2MP_Player *pPlayer, int characterID)
{
	return g_pDB->ReadInt("select AccountID from characters where ID = %i", characterID) == pPlayer->GetAccountID();
}

const char *GetModelFieldForFaction(int faction)
{
	switch ( faction )
	{
		case FACTION_RESISTANCE:
			return "ResistanceModel";
		case FACTION_COMBINE:
			return "CombineModel";
		case FACTION_APERTURE:
			return "ApertureModel";
		default:
			return "DefaultModel";
	}
}

void CHL2MPRules::ApplyDataToPlayer(CHL2MP_Player *pPlayer, int characterID)
{
	//read the data
	dbReadResult *character = g_pDB->ReadMultiple("select Name, DefaultModel, Modules, Faction, Level, Exp, AP, PlayerKills, PlayerDeaths, MonsterKills, MonsterDeaths, Sprees, SpreeWars, BestSpree, TimePlayed from Characters where ID = %i", characterID);
		
	if ( character->Count() > 0 ) // if its 0, no data for some reason
	{// apply this data to the player
		//engine->ClientCommand(pPlayer->edict(),UTIL_VarArgs("setinfo name \"%s\"",character->Element(0).text));
		pPlayer->SetCharacterName(character->Element(0).text);
		pPlayer->SetModel(character->Element(1).text);
		int faction = character->Element(3).integer;
		if ( faction > FACTION_NONE && faction <= NUM_FACTIONS )
			pPlayer->SetFaction(faction);
		else
			pPlayer->SetFaction(FACTION_COMBINE);
		pPlayer->SetLevelAndExp(character->Element(4).integer, character->Element(5).integer);
		int ap = character->Element(6).integer;

		// setup their modules
		CUtlVector<char*> modules;
		V_SplitString( character->Element(2).text, ";", modules);
		int modulesLength = modules.Count();

		bool anyRemoved = false;
		for ( int i=0; i<modulesLength; i+=2 )
		{
			Module *m = GetModule(modules[i]);
			int level = atoi(modules[i+1]);
			
			if ( m == NULL || !m->IsPurchasable() )
			{// award module points, remove this module from the character data
				int upgradeCost = m == NULL ? 1 : m->GetUpgradeCost();
				ap += upgradeCost * level;

				if ( m == NULL )
					ClientPrint(pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs("Module '%s' not recognised, awarding %i module points to compensate...\n", modules[i], upgradeCost * level));
				else
					ClientPrint(pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs("The '%s' module has been disabled, awarding %i module points to compensate...\n", m->GetDisplayName(), upgradeCost * level));
				anyRemoved = true;
			}
			else
				pPlayer->SetModuleLevel(m,level);
		}
		modules.PurgeAndDeleteElements();
		if ( anyRemoved )
			pPlayer->SerializeModuleData(); // must be recalculated
		else
			pPlayer->SetSerializedModuleData(character->Element(2).text);
	
		pPlayer->SetModulePoints(ap);

		// 7			8			  9				10			   11	   12		  13		 14			 15
		// PlayerKills, PlayerDeaths, MonsterKills, MonsterDeaths, Sprees, SpreeWars, BestSpree, TimePlayed, Progression
		pPlayer->SetStats(character->Element(7).integer, character->Element(8).integer, character->Element(9).integer, character->Element(10).integer, character->Element(11).integer,
						  character->Element(12).integer, character->Element(13).integer, character->Element(14).integer, character->Element(15).floating);
		pPlayer->EnterCharacter(characterID);
	}
	
	// clean up
	character->Purge();
	delete character;
}

void CHL2MPRules::SavePlayerData(CHL2MP_Player *pPlayer)
{
	int changedBits = pPlayer->GetChangedBits();
	if ( !changedBits )
		return;
	
	int characterID = pPlayer->GetCharacterID();
	bool updateCharacterTable = (changedBits & (BITS_CHANGED_CHARACTER|BITS_CHANGED_MODULES)) != 0;
	if ( changedBits & BITS_CHANGED_MODULES )
		pPlayer->SerializeModuleData(); // recalculate this

	g_pDB->BeginTransaction();

	if ( updateCharacterTable )
		g_pDB->Command("update Characters set Exp = %i, Level = %i, AP = %i, Modules = '%s', LastActive = CURRENT_TIMESTAMP, PlayerKills = %i, PlayerDeaths = %i, MonsterKills = %i, MonsterDeaths = %i, Sprees = %i, SpreeWars = %i, BestSpree = %i, TimePlayed = %f where ID = %i", pPlayer->GetTotalExp(), pPlayer->GetLevel(), pPlayer->GetModulePoints(), pPlayer->GetSerializedModules(), pPlayer->GetStatPlayerKills(), pPlayer->GetStatPlayerDeaths(), pPlayer->GetStatMonsterKills(), pPlayer->GetStatMonsterDeaths(), pPlayer->GetStatNumSprees(), pPlayer->GetStatNumSpreeWars(), pPlayer->GetStatBestSpree(), pPlayer->TimePlayed(), characterID);
	
	// if leaving character without having changed anything else, be sure to update the time values
	if ( (changedBits & BITS_CHANGED_LEAVING_CHAR) && !(changedBits & BITS_CHANGED_CHARACTER) )
		g_pDB->Command("update Characters set LastActive = CURRENT_TIMESTAMP, TimePlayed = %f where ID = %i", pPlayer->TimePlayed(), characterID);

	if ( changedBits & BITS_CHANGED_LOGGING_OFF )
		g_pDB->Command("update Accounts set LastActive = CURRENT_TIMESTAMP where ID = %i", pPlayer->GetAccountID());

	g_pDB->CommitTransaction();

	if ( !g_fGameOver && (changedBits & (BITS_CHANGED_LEAVING_CHAR|BITS_CHANGED_LOGGING_OFF)) && HL2MPRules() )
	{// if its not the end of the game (when all player stats are saved automatically), save my stats when I leave a character
		g_pDB2->Command("insert into gameplayer (gameid, characterid, faction, plevel, modules, gameexp, duration, playerkills, playerdeaths, monsterkills, monsterdeaths, rank) values ('%s', %i, %i, %i, '%s', %i, %.2f, %i, %i, %i, %i, 0)", m_szGameID, characterID, pPlayer->GetFaction(), pPlayer->GetLevel(), pPlayer->GetSerializedModules(),pPlayer->GetGameExp(), pPlayer->GameTimePlayed(), pPlayer->GetPlayerKills(), pPlayer->GetPlayerDeaths(), pPlayer->GetMonsterKills(), pPlayer->GetMonsterDeaths());
	}

	pPlayer->ClearChangedBits();
}

const char *CHL2MPRules::GetCharacterName(int characterID)
{
	return g_pDB->ReadString("select Name from Characters where ID = %i", characterID);
}

const char *CHL2MPRules::GetCharacterModel(int characterID, int faction)
{
	const char *modelField = GetModelFieldForFaction(faction);
	return g_pDB->ReadString("select %s from Characters where ID = %i", modelField, characterID);
}

int CHL2MPRules::GetCharacterLevel(int characterID)
{
	return g_pDB->ReadInt("select Level from Characters where ID = %i", characterID);
}

extern McConVar mc_player_startlevel;
extern int TotExpForLevelUp( int level );

int CHL2MPRules::AddNewCharacter(CHL2MP_Player *pPlayer, const char* name, const char *model, int faction, bool bLoadThisCharacter)
{
	if ( GetNumCharacters(pPlayer) >= MAX_CHARS )
		return -1;

	std::string escapedName = Escape(name);
	int initialMP = mc_perlevel_modulepoints.GetInt() * mc_player_startlevel.GetInt();
	
	const char *combineModel = faction == FACTION_COMBINE ? model : CHL2MP_Player::GetRandomModelForFaction(FACTION_COMBINE),
			*resistanceModel = faction == FACTION_RESISTANCE ? model : CHL2MP_Player::GetRandomModelForFaction(FACTION_RESISTANCE),
			  *apertureModel = faction == FACTION_APERTURE ? model : CHL2MP_Player::GetRandomModelForFaction(FACTION_APERTURE);
	g_pDB->Command("insert into Characters (AccountID, Name, Faction, Modules, Level, Exp, AP, DefaultModel, CombineModel, ResistanceModel, ApertureModel, Created, LastActive, PlayerKills, PlayerDeaths, MonsterKills, MonsterDeaths, Sprees, SpreeWars, BestSpree, TimePlayed) values (%i, '%s', %i, '', %i, %i, %i, '%s', '%s', '%s', '%s', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, 0, 0, 0, 0, 0, 0, 0, 0.0)", pPlayer->GetAccountID(), name, faction, mc_player_startlevel.GetInt(), TotExpForLevelUp(mc_player_startlevel.GetInt()-1), initialMP, model, combineModel, resistanceModel, apertureModel);
	
	int newCharacterID = g_pDB->LastInsertID();
	if ( bLoadThisCharacter )
		ApplyDataToPlayer(pPlayer, newCharacterID);
	return newCharacterID;
}

void CHL2MPRules::DeleteCharacter(CHL2MP_Player *pPlayer, int characterID)
{
	if ( !PlayerOwnsCharacter(pPlayer,characterID) )
		return; // can only delete our own characters
	
	g_pDB->Command("delete from Characters where ID = %i", characterID);
}