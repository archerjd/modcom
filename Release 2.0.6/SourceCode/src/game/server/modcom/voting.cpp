#include "cbase.h"
#include "hl2mp/hl2mp_gamerules.h"
#include "modcom/mc_shareddefs.h"
#include "tier0/memdbgon.h"

extern ConVar mc_vote_duration, mc_vote_gamemode_enabled, mc_vote_map_enabled, mc_vote_gamemode_type, mc_gamemode, nextlevel;
ConVar mc_vote_seperation("mc_vote_seperation", "60", FCVAR_NOTIFY | FCVAR_REPLICATED, "After a vote is complete, another vote may not be started for this duration", true, 1, false, 0 );

float g_flNextVoteAllowed;
int g_iNextGameMode = -1, g_iGameModeVotedOn = PREGAME;
int gamemode_vote_stage2_a = DEATHMATCH, gamemode_vote_stage2_b = DEATHMATCH; // used for the second part of the two-stage gamemode vote only

int g_iNumMapNominations = 0;
char g_szMapNominations[MAX_MAP_NOMINATIONS][64];
char g_szVoteCategory[32];
char g_szStartingPlayerName[MAX_PLAYER_NAME_LENGTH];


const char *GetGameModeName(int mode)
{
	switch ( mode )
	{
	case DEATHMATCH:
		return "Deathmatch";
	case FFA:
		return "Free-for-all";
	case RANDOM_PVM:
		return "Players vs monsters";
	case TEAM_DEATHMATCH:
		return "Team Deathmatch";
	case HOARDER:
		return "'Hoarder' mode";
	default:
		return "Unknown";
	}
}

int CHL2MPRules::NumQueuedVotes()
{
	int num = 0;
	for ( int i=0; i<MAX_VOTE_STEPS; i++ )
		if ( m_VoteSequence[i] == VOTE_NONE )
			break;
		else if ( m_VoteSequence[i] != FORCE_EARLY_MAP_CHANGE )
			num++;
	return num;
}

bool IsMapAlreadyInList(const char *szMapName)
{
	for ( int i=0; i<g_iNumMapNominations; i++ )
		if ( FStrEq(g_szMapNominations[i], szMapName) )
			return true;
	return false;
}

void CHL2MPRules::PrepareVoteSequence(bool isSpecificModeRequest, CHL2MP_Player *pStartingPlayer)
{
	for ( int i=0; i<MAX_VOTE_STEPS; i++ )
		m_VoteSequence[i] = VOTE_NONE;

	bool triggeredManually = pStartingPlayer != NULL;
	if ( triggeredManually )
		Q_snprintf(g_szStartingPlayerName, sizeof(g_szStartingPlayerName), pStartingPlayer->GetName());
	else
		Q_snprintf(g_szStartingPlayerName, sizeof(g_szStartingPlayerName), "BoSS");
	int step = 0;
	
	if ( isSpecificModeRequest )
	{
		if ( mc_vote_gamemode_enabled.GetInt() == 0 )
			return;
		
		m_VoteSequence[step] = VOTE_GAMEMODE_YESNO;
		return;
	}
	else
	{
		if ( triggeredManually )
		{
			m_VoteSequence[step] = VOTE_ALLOW_EARLY_SELECTION;
			step++;
		}
		
		if ( mc_vote_gamemode_enabled.GetInt() > 0 && ( !triggeredManually || mc_vote_gamemode_enabled.GetInt() == 2 ) )
		{
			if ( mc_vote_gamemode_type.GetInt() == 1 )
			{
				m_VoteSequence[step] = VOTE_GAMEMODE_SINGLE_STEP;
				step++;
			}
			else if ( mc_vote_gamemode_type.GetInt() == 2 )
			{
				m_VoteSequence[step] = VOTE_GAMEMODE_PART1;
				step++;
				m_VoteSequence[step] = VOTE_GAMEMODE_PART2;
				step++;
			}
		}
	}
		
	if ( mc_vote_map_enabled.GetInt() > 0 )
		m_VoteSequence[step] = VOTE_MAP;
	step++;

	if ( triggeredManually )
		m_VoteSequence[step] = FORCE_EARLY_MAP_CHANGE;
	step++;
}

void CHL2MPRules::RunNextVote()
{
	for ( int i=0; i<MAX_VOTE_OPTIONS; i++ )
		m_iVotes[i] = 0;

	const char *message = NULL;
	switch ( m_VoteSequence[0] )
	{
	case FORCE_EARLY_MAP_CHANGE:
		GoToIntermission();
		return;
	case VOTE_MAP:
		// fill out as much of the map list as isn't filled by nominations
		if ( g_iNumMapNominations < MAX_MAP_NOMINATIONS )
		{// try getting one map sequentially, fill the rest randomly
			char temp[64];
			GetNextLevelName(temp, sizeof(temp), false);
			if ( !IsMapAlreadyInList(temp) )
			{
				Q_snprintf(g_szMapNominations[g_iNumMapNominations], sizeof(g_szMapNominations[g_iNumMapNominations]), temp);
				g_iNumMapNominations++;
			}

			int numSkipped = 0; // sanity check - prevent infinate loop when server only has < 4 maps for whatever reason
			while ( g_iNumMapNominations < MAX_MAP_NOMINATIONS )
			{
				GetNextLevelName(temp, sizeof(temp), true);
				if ( numSkipped < 5 && IsMapAlreadyInList(temp) )
				{
					numSkipped ++;
					continue;
				}
				Q_snprintf(g_szMapNominations[g_iNumMapNominations], sizeof(g_szMapNominations[g_iNumMapNominations]), temp);
				g_iNumMapNominations++;
			}
		}

		message = UTIL_VarArgs("Vote for the next map:\n %s : %s\n %s : %s\n %s : %s\n %s : %s", "%vote 1%", g_szMapNominations[0], "%vote 2%", g_szMapNominations[1], "%vote 3%", g_szMapNominations[2], "%vote 4%", g_szMapNominations[3] );
		// Vote for the next map:
		//	F1 - g_szMapNominations[0]
		//	F2 - g_szMapNominations[1]
		//	F3 - g_szMapNominations[2]
		//	F4 - g_szMapNominations[3]
		break;
	case VOTE_ALLOW_EARLY_SELECTION:
		message = UTIL_VarArgs("%s wants to start an early vote for the next map. Do you?\n %s : Yes\n %s : No", g_szStartingPlayerName, "%vote 1%", "%vote 2%");
		// XXX wants to start an early vote on the next map. Do you?
		//	F1 - Yes
		//	F2 - No
		break;
	case VOTE_GAMEMODE_PART1:
		message = UTIL_VarArgs("Vote for the next game mode, by category:\n %s : Competing teams\n %s : No teams\n %s : Single team co-op", "%vote 1%", "%vote 2%", "%vote 3%");
		// Vote for the next game mode, by category:
		//	F1 - Competing teams
		//	F2 - No teams
		//	F3 - Single team co-op
		break;
	case VOTE_GAMEMODE_PART2:
		message = UTIL_VarArgs("Vote for the next game mode, within '%s' category:\n %s : %s\n %s : %s", g_szVoteCategory, "%vote 1%", GetGameModeName(gamemode_vote_stage2_a), "%vote 2%", GetGameModeName(gamemode_vote_stage2_b));
		// Vote for the next game mode, within "SELECTED" category:
		//	F1 - gamemode_vote_stage2_a
		//	F2 - gamemode_vote_stage2_b
		//optional F3 - None of these! Anything else!
		break;
	case VOTE_GAMEMODE_SINGLE_STEP:
		message = UTIL_VarArgs("Vote for the next game mode:\n %s : %s\n %s : %s\n %s : %s\n %s : %s\n %s : %s", "%vote 1%", GetGameModeName(DEATHMATCH), "%vote 2%", GetGameModeName(FFA), "%vote 3%", GetGameModeName(RANDOM_PVM), "%vote 4%", GetGameModeName(TEAM_DEATHMATCH), "%vote 5%", GetGameModeName(HOARDER));
		// Vote for the next game mode:
		//	F1 - Deathmatch
		//	F2 - Free-for-all
		//	F3 - PVM Coop
		//	F4 - Team Deathmatch
		//	F5 - "Hoarder" mode
		break;
	case VOTE_GAMEMODE_YESNO:
		message = UTIL_VarArgs("%s wants to change the game mode to %s. Do you?\n %s : Yes\n %s : No", g_szStartingPlayerName, GetGameModeName(g_iGameModeVotedOn), "%vote 1%", "%vote 2%");
		// XXX wants to change the game mode to game_mode_voted_on. Do you?
		//	F1 - Yes
		//	F2 - No
		break;
	default:
		return;
	}

	m_bInVote = true;
	m_flVoteEndTime = gpGlobals->curtime + mc_vote_duration.GetFloat();
	g_flNextVoteAllowed = m_flVoteEndTime + mc_vote_seperation.GetFloat();

	
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer ) // do we want the IsInCharacter bit?
			continue;
		pPlayer->AllowVote(true);
		m_iTotalVoters ++;
	}

	CRecipientFilter filter;
	filter.AddAllPlayers();
	filter.MakeReliable();
	UserMessageBegin( filter, "Vote" );
		WRITE_BYTE( 1 );				// open and display
		WRITE_STRING( message );
	MessageEnd();
}

void CHL2MPRules::VoteFinished()
{
	VoteType_t lastVote = m_VoteSequence[0]; // that which has just finished
	
	switch ( lastVote )
	{
	case VOTE_MAP: // the map vote options will have been stored off somewhere, right?
		int winningIndex;
		if ( m_iVotes[0] >= m_iVotes[1] && m_iVotes[0] >= m_iVotes[2] && m_iVotes[0] >= m_iVotes[3] )
			winningIndex = 0;
		else if ( m_iVotes[1] >= m_iVotes[0] && m_iVotes[1] >= m_iVotes[2] && m_iVotes[1] >= m_iVotes[3] )
			winningIndex = 1;
		else if ( m_iVotes[2] >= m_iVotes[0] && m_iVotes[2] >= m_iVotes[1] && m_iVotes[0] >= m_iVotes[3] )
			winningIndex = 2;
		else if ( m_iVotes[3] >= m_iVotes[0] && m_iVotes[3] >= m_iVotes[1] && m_iVotes[3] >= m_iVotes[2] )
			winningIndex = 3;
		else
			winningIndex = 0;

		nextlevel.SetValue(g_szMapNominations[winningIndex]);
		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s selected: %i voted %s, %i voted %s, %i voted %s, %i voted %s\n",g_szMapNominations[winningIndex], m_iVotes[0], g_szMapNominations[0], m_iVotes[1], g_szMapNominations[1], m_iVotes[2], g_szMapNominations[2], m_iVotes[3], g_szMapNominations[3]) );
		break;
	case VOTE_ALLOW_EARLY_SELECTION:
		if ( m_iVotes[0] > m_iVotes[1] )
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote succeeded: %i voted yes, %i voted no - starting early vote\n",m_iVotes[0],m_iVotes[1]) );
		else
		{// vote failed, clear out all queued votes
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote failed: %i voted yes, %i voted no\n",m_iVotes[0],m_iVotes[1]) );
			PrepareVoteSequence(false, NULL); // set up the end of map votes in case no others are called before then
		}
		break;
	case VOTE_GAMEMODE_PART1:
		if ( m_iVotes[0] >= m_iVotes[1] && m_iVotes[0] >= m_iVotes[2] ) // "competing teams" won
		{
			gamemode_vote_stage2_a = HOARDER;
			gamemode_vote_stage2_b = TEAM_DEATHMATCH;
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote result: 'competing teams' option selected\n %i voted 'competing teams', %i voted 'no teams', %i voted 'single team co-op'\n",m_iVotes[0],m_iVotes[1],m_iVotes[2]) );
			Q_snprintf(g_szVoteCategory, sizeof(g_szVoteCategory), "competing teams");
		}
		else if ( m_iVotes[1] >= m_iVotes[0] && m_iVotes[1] >= m_iVotes[2] ) // "no teams" won
		{
			gamemode_vote_stage2_a = FFA;
			gamemode_vote_stage2_b = DEATHMATCH;
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote result: 'no teams' option selected\n %i voted 'competing teams', %i voted 'no teams', %i voted 'single team co-op'\n",m_iVotes[0],m_iVotes[1],m_iVotes[2]) );
			Q_snprintf(g_szVoteCategory, sizeof(g_szVoteCategory), "no teams");
		}
		else // "single team" coop won
		{
			g_iNextGameMode = RANDOM_PVM;
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote result: 'single team' option selected, defaulting to PVM game mode\n %i voted 'competing teams', %i voted 'no teams', %i voted 'single team co-op'\n",m_iVotes[0],m_iVotes[1],m_iVotes[2]) );
			Q_snprintf(g_szVoteCategory, sizeof(g_szVoteCategory), "single team");

			// shunt up all vote options to skip the part 2 vote, as PVM coop has been chosen in 1 step
			for ( int i=0; i<MAX_VOTE_STEPS-1; i++ )
				m_VoteSequence[i] = m_VoteSequence[i+1];
			m_VoteSequence[MAX_VOTE_STEPS-1] = VOTE_NONE;
		}
		break;
	case VOTE_GAMEMODE_PART2:
		if ( m_iVotes[0] >= m_iVotes[1] )
		{
			g_iNextGameMode = gamemode_vote_stage2_a;
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote result: %s game mode selected: %i voted %s, %i voted %s\n",GetGameModeName(gamemode_vote_stage2_a), m_iVotes[0], GetGameModeName(gamemode_vote_stage2_a), m_iVotes[1], GetGameModeName(gamemode_vote_stage2_b)) );
		}
		else
		{
			g_iNextGameMode = gamemode_vote_stage2_b;
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote result: %s game mode selected: %i voted %s, %i voted %s\n",GetGameModeName(gamemode_vote_stage2_b), m_iVotes[0], GetGameModeName(gamemode_vote_stage2_a), m_iVotes[1], GetGameModeName(gamemode_vote_stage2_b)) );
		}
		break;
	case VOTE_GAMEMODE_SINGLE_STEP:
	{
		int biggestIndex = 0, biggestNum = m_iVotes[0];
		for ( int i=1; i<NUM_GAME_MODES; i++ )
			if ( m_iVotes[i] > biggestNum )
			{
				biggestIndex = i;
				biggestNum = m_iVotes[i];
			}

		switch ( biggestIndex )
		{// FFA and PVM are the other way round in the vote list
		case 1:  g_iNextGameMode = 3; break;
		case 2:  g_iNextGameMode = 2; break;
		default: g_iNextGameMode = biggestIndex + 1; break;
		}

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote result: %s game mode selected:\n%i voted %s, %i voted %s, %i voted %s, %i voted %s, %i voted %s\n",GetGameModeName(g_iNextGameMode), m_iVotes[0], GetGameModeName(1), m_iVotes[1], GetGameModeName(2), m_iVotes[2], GetGameModeName(3), m_iVotes[3], GetGameModeName(4), m_iVotes[4], GetGameModeName(5)) );
		break;
	}
	case VOTE_GAMEMODE_YESNO:
		if ( m_iVotes[0] > m_iVotes[1] )
		{
			
			mc_gamemode.SetValue(g_iGameModeVotedOn);
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote succeeded: %i voted yes, %i voted no - %s game mode selected\n", m_iVotes[0], m_iVotes[1], GetGameModeName(g_iGameModeVotedOn)) );
		}
		else
		{// vote failed, clear out all queued votes
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("Vote failed: %i voted yes, %i voted no - game mode not changed\n",m_iVotes[0],m_iVotes[1]) );
			PrepareVoteSequence(false, NULL); // set up the end of map votes in case no others are called before then
		}
		break;
	}
	
	// shunt up all vote options by 1
	for ( int i=0; i<MAX_VOTE_STEPS-1; i++ )
		m_VoteSequence[i] = m_VoteSequence[i+1];
	m_VoteSequence[MAX_VOTE_STEPS-1] = VOTE_NONE;
	
	// if we have any votes left to run, run them ... otherwise quit, changing map soon if we got through the whole sequence
	if ( m_VoteSequence[0] != VOTE_NONE )
		RunNextVote();
	else
	{
		m_bInVote = false;
		g_flNextVoteAllowed = gpGlobals->curtime + mc_vote_seperation.GetFloat();
	}
}

bool CHL2MPRules::Nominate(CHL2MP_Player *pPlayer, const char *szValue)
{
	if ( m_bInVote )
		return false;

	if ( g_iNumMapNominations >= MAX_MAP_NOMINATIONS )
	{
		ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to nominate map - too many nominations have already been made\n");
		return false;
	}

	if( !engine->IsMapValid(szValue) )
	{
		ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to nominate map - map name not recognised\n");
		return false;
	}
	
	// check it's not already in the list
	if ( IsMapAlreadyInList(szValue) )
	{
		ClientPrint(pPlayer, HUD_PRINTTALK, UTIL_VarArgs("Map %s has already been nominated by another player\n", szValue));
		return false;
	}

	Q_snprintf(g_szMapNominations[g_iNumMapNominations], sizeof(g_szMapNominations[g_iNumMapNominations]), szValue);
	g_iNumMapNominations ++;

	ClientPrint(pPlayer, HUD_PRINTTALK, UTIL_VarArgs("Nominated map: %s\n", szValue));
	return true;
}

void CHL2MPRules::Vote(CHL2MP_Player *pPlayer, int vote)
{
	vote--; // need to make this 0-based, not 1-based
	if ( !IsInVote() || vote < 0 || vote >= MAX_VOTE_OPTIONS)
		return;

	if ( m_iVotes[vote] == -1 )
	{
		ClientPrint(pPlayer, HUD_PRINTTALK, UTIL_VarArgs("You voted for option %i, which is not valid.\n", (vote+1)));
		return;
	}
	m_iVotes[vote] ++;
	ClientPrint(pPlayer, HUD_PRINTTALK, UTIL_VarArgs("You voted for option %i.\n", (vote+1)));

	//Msg("%s voted '%s'\n", pPlayer->GetPlayerName(), vote ? "yes" : "no" ); // temp

	CRecipientFilter filter;
	filter.AddRecipient(pPlayer);
	filter.MakeReliable();
	UserMessageBegin( filter, "Vote" );
		WRITE_BYTE( 0 );				// close
	MessageEnd();

	pPlayer->AllowVote(false);


	// if there's no one left who can still vote, end the vote early
	bool anyLeft = false;
	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( i ) );
		if ( !pPlayer || !pPlayer->CanVote() )
			continue;
		anyLeft = true;
		break;
	}

	if ( !anyLeft )
		VoteFinished();
}

/*
bool CHL2MPRules::StartVote(CHL2MP_Player *pPlayer, int voteType, const char *szValue)
{
	if (!(voteType / 2 > 0)) //catch the override & internal votes
	{
		if ( m_bInVote )
			return false;

		if ( m_flNextVoteAllowed > gpGlobals->curtime )
		{
			if ( pPlayer != NULL )
				ClientPrint(pPlayer, HUD_PRINTTALK, UTIL_VarArgs("Another vote cannot be started for %.0f seconds\n", m_flNextVoteAllowed - gpGlobals->curtime));
			return false;
		}
	}

	//Vote is a go, set up and start
	m_bInVote = true;
	m_iVoteType = voteType;
	Q_snprintf(m_szVoteArgs, sizeof(m_szVoteArgs), szValue);
	m_flVoteEndTime = gpGlobals->curtime + mc_vote_duration.GetFloat();
	m_flNextVoteAllowed = m_flVoteEndTime + mc_vote_seperation.GetFloat();

	//Vote starts, branch into the correct vote
	if ( voteType == VOTE_START_GAMEMODE || voteType == VOTE_OVERRIDE_GAMEMODE )
	{
		UTIL_ClientPrintAll(HUD_PRINTTALKCONSOLE, UTIL_VarArgs("Vote started by %s", pPlayer ? pPlayer->GetPlayerName() : "BoSS"));
		if (mc_vote_type_gamemode.GetInt() & VOTE_FLAG_PRELIMINARY )
			StartVote(pPlayer, VOTE_PRE_GAMEMODE, szValue);
		else if ((mc_vote_type_gamemode.GetInt() & VOTE_FLAG_SINGLE) && !FStrEq(szValue, ""))
			StartVote(pPlayer, VOTE_GAMEMODE, szValue);
		else
			StartVote(pPlayer, VOTE_GAMEMODE_PART1, szValue);
		return true;
	}
	else if ( voteType == VOTE_START_MAP || voteType == VOTE_OVERRIDE_MAP ) //map vote code
	{
		UTIL_ClientPrintAll(HUD_PRINTTALKCONSOLE, UTIL_VarArgs("Vote started by %s", pPlayer ? pPlayer->GetPlayerName() : "BoSS"));
		if (mc_vote_type_gamemode.GetInt() & VOTE_FLAG_PRELIMINARY)
			StartVote(pPlayer, VOTE_PRE_MAP, szValue);
		else
			StartVote(pPlayer, VOTE_MAP, szValue);
		return true;
	}

	//Votes a go.. most likely, define a message, and set votes to null, as they test voteability.
	m_i1Votes = m_i2Votes = m_i3Votes = m_i4Votes = m_iTotalVoters = 0;
	const char *message = "";

	if (voteType == VOTE_PRE_GAMEMODE)
	{
		message = UTIL_VarArgs("%s wants to start a game mode vote:\n%s : Yes\n%s : No", pPlayer ? pPlayer->GetPlayerName() : "BoSS", "%vote 1%", "%vote 2%");
		m_i3Votes = m_i4Votes = -1;
	}
	else if (voteType == VOTE_PRE_MAP)
	{
		message = UTIL_VarArgs("%s wants to start a map vote:\n%s : Yes\n%s : No", pPlayer ? pPlayer->GetPlayerName() : "BoSS", "%vote 1%", "%vote 2%");
		m_i3Votes = m_i4Votes = -1;
	}
	else if (voteType == VOTE_GAMEMODE)
	{
		m_iVoteValue = atoi(szValue);
		switch ( m_iVoteValue )
		{
		case DEATHMATCH:
			szValue = "Deathmatch"; break;
		case RANDOM_PVM:
			szValue = "PVM"; break;
		case FFA:
			szValue = "FFA"; break;
		case HOARDER:
			szValue = "Hoarder"; break;
		case TEAM_DEATHMATCH:
		default:
			szValue = "Team Deathmatch"; break;
		}

		message = UTIL_VarArgs("%s wants to change the game mode to %s:\n%s : Yes\n%s : No", pPlayer ? pPlayer->GetPlayerName() : "BoSS", szValue, "%vote 1%", "%vote 2%");
		m_i3Votes = m_i4Votes = -1;
	}
	else if(voteType == VOTE_GAMEMODE_PART1)
	{
		message = UTIL_VarArgs("Game mode vote started - select type:\n%s : Teamplay\n%s : No teams\n%s : Cooperative", "%vote 1%", "%vote 2%", "%vote 3%");
		m_i4Votes = -1;
	}
	else if(voteType == VOTE_GAMEMODE_PART2)
	{
		message = UTIL_VarArgs("Should game include monsters?\n%s : Yes\n%s : No", "%vote 1%", "%vote 2%");
		m_i3Votes = m_i4Votes = -1;
	}
	else if(voteType == VOTE_MAP)
	{//Under constructuion
		while (m_szMapNoms.Count() < 4 )
		{
			if (m_szMapNoms.Count() == 3)
			{
				char temp[64];
				GetNextLevelName(temp, 64, false);
				m_szMapNoms.AddToTail(temp);
			}
			else
			{
				char temp[64];
				GetNextLevelName(temp, 64, true);
				m_szMapNoms.AddToTail(temp);
			}
		}

		char voteMapname[4][64];

		for (int i = 0; i < 4; i++)
			Q_snprintf(voteMapname[i], sizeof(voteMapname[i]), szValue);

			message = UTIL_VarArgs("%s started a map vote\n%s : %s\n%s : %s\n%s : %s\n%s : %s", pPlayer ? pPlayer->GetPlayerName() : "BoSS", "%vote 1%", voteMapname[1], "%vote 2%", voteMapname[2], "%vote 3%", voteMapname[3], "%vote 4%", voteMapname[4]);
	}

	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer ) // do we want the IsInCharacter bit?
			continue;
		pPlayer->AllowVote(true);
		m_iTotalVoters ++;
	}

	CRecipientFilter filter;
	filter.AddAllPlayers();
	filter.MakeReliable();
	UserMessageBegin( filter, "Vote" );
		WRITE_BYTE( 1 );				// open and display
		WRITE_STRING( message );
	MessageEnd();

	return true;
}

void CHL2MPRules::EndVote()
{
	m_bInVote = false;
	m_flNextVoteAllowed = gpGlobals->curtime + mc_vote_seperation.GetFloat();

	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;
		pPlayer->AllowVote(false);
	}

	CRecipientFilter filter;
	filter.AddAllPlayers();
	filter.MakeReliable();
	UserMessageBegin( filter, "Vote" );
		WRITE_BYTE( 0 );				// close
	MessageEnd();

	// !! Now check the results
	if(m_iVoteType == VOTE_PRE_GAMEMODE || m_iVoteType == VOTE_PRE_MAP || m_iVoteType == VOTE_GAMEMODE)
	{
		if ( m_i1Votes > m_i2Votes )
		{		
			UTIL_ClientPrintAll(HUD_PRINTTALKCONSOLE, UTIL_VarArgs("%i players voted yes, %i voted no - vote succeeded\n",m_i1Votes,m_i2Votes) );
			if(m_iVoteType == VOTE_PRE_GAMEMODE)
			{
				if ((mc_vote_type_gamemode.GetInt() & VOTE_FLAG_SINGLE) && !FStrEq(m_szVoteArgs,"")) // VOTEBUG && should be || ?
					StartVote(NULL, VOTE_GAMEMODE, m_szVoteArgs);
				else
					StartVote(NULL, VOTE_GAMEMODE_PART1, m_szVoteArgs);
			}
			else if(m_iVoteType == VOTE_GAMEMODE)
				mc_gamemode.SetValue(atoi(m_szVoteArgs));
			else
				StartVote(NULL, VOTE_MAP, m_szVoteArgs);
			return;
		}
		else
			UTIL_ClientPrintAll(HUD_PRINTTALKCONSOLE, UTIL_VarArgs("%i players voted yes, %i voted no - vote failed\n",m_i1Votes,m_i2Votes) );
	}
	else if(m_iVoteType == VOTE_GAMEMODE_PART1)
	{	
		const char *message = "";
		if (m_i1Votes >= m_i2Votes && m_i1Votes >= m_i3Votes)		//all ties
		{
			m_iVoteValue = 1;
			message = "Teamplay";
		}
		else if (m_i2Votes >= m_i1Votes && m_i2Votes >= m_i3Votes) //wins vs. 3 on ties
		{
			m_iVoteValue = 2;
			message = "No teams";
		}
		else if (m_i3Votes >= m_i1Votes && m_i3Votes >= m_i2Votes)
		{
			m_iVoteValue = 3;
			message = "Cooperative Players vs Monsters";
		}

		UTIL_ClientPrintAll(HUD_PRINTTALKCONSOLE, UTIL_VarArgs("%i for Teamplay, %i for No Teams, %i for Cooperative - %s won!\n",m_i1Votes,m_i2Votes,m_i3Votes, message) );

		// we don't have a monster-less cooperative gamemode, so don't ask players if they want monsters...
		if ( m_iVoteValue == 3 )
			mc_gamemode.SetValue(2);
		else
			StartVote(NULL, VOTE_GAMEMODE_PART2, m_szVoteArgs);
		return;
	}
	else if(m_iVoteType == VOTE_GAMEMODE_PART2)
	{
		const char *message = "";
		if ( m_i1Votes > m_i2Votes )
		{		
			switch (m_iVoteValue)
			{
				case 1: // teams
					mc_gamemode.SetValue(5);
					message = "\"Hoarder\" mode";
					break;
				case 2: // no teams
					mc_gamemode.SetValue(3); //
					message = "Free-for-all";
					break;
				case 3: // coop
					mc_gamemode.SetValue(2);
					message = "Cooperative Players vs. Monsters";
					break;
			}
		}
		else
		{
			switch (m_iVoteValue)
			{
				case 1:
					mc_gamemode.SetValue(1);
					message = "Deathmatch";
					break;
				case 2:
					mc_gamemode.SetValue(4);
					message = "Team Deathmatch";
					break;
				case 3: // coop
					mc_gamemode.SetValue(2);
					message = "Cooperative Players vs. Monsters";
					break;
			}
		}
		UTIL_ClientPrintAll(HUD_PRINTTALKCONSOLE, UTIL_VarArgs("%i players voted yes, %i voted no - %s mode chosen\n",m_i1Votes,m_i2Votes, message) );
		return;
	}
	else if (m_iVoteType == VOTE_MAP)
	{
		const char * VoteMapname = "";

		if (m_i1Votes >= m_i2Votes && m_i1Votes >= m_i3Votes && m_i1Votes >= m_i4Votes) //gets all ties
			VoteMapname = m_szMapNoms[0];
		else if (m_i2Votes >= m_i1Votes && m_i2Votes >= m_i3Votes && m_i2Votes >= m_i4Votes)
			VoteMapname = m_szMapNoms[1];
		else if (m_i3Votes >= m_i2Votes && m_i3Votes >= m_i1Votes && m_i3Votes >= m_i4Votes)
			VoteMapname = m_szMapNoms[2];
		else if (m_i4Votes >= m_i2Votes && m_i4Votes >= m_i3Votes && m_i4Votes >= m_i1Votes)
			VoteMapname = m_szMapNoms[3];

		g_fGameOver = true;
		Msg( "CHANGE LEVEL: %s\n", VoteMapname );
		engine->ChangeLevel( VoteMapname, NULL );

		m_szMapNoms.PurgeAndDeleteElements();
		return;
	}
}
*/