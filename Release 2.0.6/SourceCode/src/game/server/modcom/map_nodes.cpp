#include "cbase.h"
#include "hl2mp/hl2mp_gamerules.h"
#include "ai_basenpc.h"
#include "utlbuffer.h"
#include "filesystem.h"
#include "Sprite.h"
#include "modcom/monsters.h"
#include "modcom/teleport_blocker.h"

#include "tier0/memdbgon.h"

enum
{
	NODE_OFF = 0,
	NODE_NODES,
	NODE_AIRNODES,
	NODE_NPC_SPAWNS,
	NODE_LARGE_NPC_SPAWNS,
	NODE_TELEPORT_BLOCKER,
};

static int nodePlacement = NODE_OFF;
CUtlVector<CBaseEntity*> g_Nodes;

CBaseEntity *CreateNode(Vector origin, float radius=NEW_TELEPORT_BLOCKER_RADIUS)
{
	if ( nodePlacement == NODE_TELEPORT_BLOCKER )
	{
		CTeleportBlocker *pEnt = static_cast<CTeleportBlocker*>( CBaseEntity::Create("info_teleport_node", origin, vec3_angle, NULL) );
		pEnt->SetRadius(radius);
		pEnt->SetVisible(true);
		HL2MPRules()->GetTeleportBlockers()->AddToTail(pEnt);
		return pEnt;
	}
	else
	{
		CBaseEntity *pEnt = CSprite::SpriteCreate( "sprites/glow01.vmt", origin, false );//CBaseEntity::Create("node", origin, vec3_angle, NULL );
		g_Nodes.AddToTail(pEnt);
		return pEnt;
	}
}

void CC_StartNodes( void )
{
	if ( nodePlacement == NODE_NODES )
		return;
	else if ( nodePlacement != NODE_OFF )
	{
		UTIL_ClientPrintAll(HUD_PRINTNOTIFY,"Cannot place nodegraph while placing other nodes!\n");
		return;
	}

	// if the text file already exists, load its current nodes
	char szNodeTextFilename[MAX_PATH];
	Q_snprintf( szNodeTextFilename, sizeof( szNodeTextFilename ), "maps/graphs/%s_graph.txt", STRING( gpGlobals->mapname ) );
	CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
	if ( filesystem->ReadFile( szNodeTextFilename, "MOD", buf ) )
	{	
		if (!buf.Size())
			return;

		const int maxLen = 64;
		char line[ maxLen ];
		CUtlVector<char*> floats;
		int num = 0;

		// loop through every line of the file, read it in
		while( true )
		{
			buf.GetLine( line, maxLen );
			if ( Q_strlen(line) <= 0 )
				break; // reached the end of the file

			// we've read in a string containing 3 tab separated floats
			// we need to split this into 3 floats, which we put in a vector
			V_SplitString( line, "	", floats );
			Vector origin( atof( floats[0] ), atof( floats[1] ), atof( floats[2] ) );

			floats.PurgeAndDeleteElements();

			CreateNode(origin);
			num ++;

			if ( !buf.IsValid() )
				break;
		}
	}

	nodePlacement = NODE_NODES;
	UTIL_ClientPrintAll(HUD_PRINTTALK,"Entered node placement mode. Use addnode, undonode, & delnearestnode commands to place nodes. Use savenodes command to finish.\n");
}
static ConCommand cc_startnodes("startnodes", CC_StartNodes, "Start manually placing nodegraph elements, with addnode, undonode, & delnearestnode commands. Finish with savenodes.", FCVAR_CHEAT);


void CC_SaveNodes( void )
{
	if ( nodePlacement == NODE_NODES )
	{
		// save the nodes
		char szNodeTextFilename[MAX_PATH];
		Q_snprintf( szNodeTextFilename, sizeof( szNodeTextFilename ),
					"maps/graphs/%s_graph.txt", STRING( gpGlobals->mapname ), GetPlatformExt() );

		CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
		for ( int i=0; i<g_Nodes.Size(); i++ )
		{
			buf.PutString( UTIL_VarArgs("%f	%f	%f\n",g_Nodes[i]->GetAbsOrigin().x,
													g_Nodes[i]->GetAbsOrigin().y,
													g_Nodes[i]->GetAbsOrigin().z) );
		}
		filesystem->WriteFile(szNodeTextFilename,"game",buf);

		// clean up & exit node mode
		g_Nodes.PurgeAndDeleteElements();
		nodePlacement = NODE_OFF;
		UTIL_ClientPrintAll(HUD_PRINTTALK,"Saved nodes & exited node placement mode. Reload map to build nodegraph.\n");
	}
	else if ( nodePlacement == NODE_AIRNODES )
	{
		// save the nodes
		char szNodeTextFilename[MAX_PATH];
		Q_snprintf( szNodeTextFilename, sizeof( szNodeTextFilename ),
					"maps/graphs/%s_airgraph.txt", STRING( gpGlobals->mapname ), GetPlatformExt() );

		CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
		for ( int i=0; i<g_Nodes.Size(); i++ )
		{
			buf.PutString( UTIL_VarArgs("%f	%f	%f\n",g_Nodes[i]->GetAbsOrigin().x,
													g_Nodes[i]->GetAbsOrigin().y,
													g_Nodes[i]->GetAbsOrigin().z) );
		}
		filesystem->WriteFile(szNodeTextFilename,"game",buf);

		// clean up & exit node mode
		g_Nodes.PurgeAndDeleteElements();
		nodePlacement = NODE_OFF;
		UTIL_ClientPrintAll(HUD_PRINTTALK,"Saved nodes & exited air node placement mode. Reload map to build nodegraph.\n");
	}
	else if ( nodePlacement == NODE_NPC_SPAWNS )
	{
		// take the map data, delete all monster spawns in it, add all our new ones, and then save it
		KeyValues *npcSpawns = HL2MPRules()->GetMapData()->FindKey("NpcSpawns");
		if ( npcSpawns != NULL )
		{
			HL2MPRules()->GetMapData()->RemoveSubKey(npcSpawns);
			npcSpawns->deleteThis();
		}
		npcSpawns = new KeyValues("NpcSpawns");

		for ( int i=0; i<g_Nodes.Size(); i++ )
			npcSpawns->SetString(UTIL_VarArgs("%f %f %f",g_Nodes[i]->GetAbsOrigin().x,
														   g_Nodes[i]->GetAbsOrigin().y,
														   g_Nodes[i]->GetAbsOrigin().z), "x");
		
		HL2MPRules()->GetMapData()->AddSubKey(npcSpawns);

		// save the spawn point
		HL2MPRules()->GetMapData()->SaveToFile(filesystem, UTIL_VarArgs("maps/graphs/%s.txt", STRING( gpGlobals->mapname )), "MOD");

		// clean up & exit node mode
		g_Nodes.PurgeAndDeleteElements();
		nodePlacement = NODE_OFF;
		UTIL_ClientPrintAll(HUD_PRINTTALK,"Saved npc spawn points & exited spawn placement mode. Reload map to load new spawn points.\n");
	}
	else if ( nodePlacement == NODE_LARGE_NPC_SPAWNS )
	{
		// take the map data, delete all monster spawns in it, add all our new ones, and then save it
		KeyValues *npcSpawns = HL2MPRules()->GetMapData()->FindKey("LargeNpcSpawns");
		if ( npcSpawns != NULL )
		{
			HL2MPRules()->GetMapData()->RemoveSubKey(npcSpawns);
			npcSpawns->deleteThis();
		}
		npcSpawns = new KeyValues("LargeNpcSpawns");

		for ( int i=0; i<g_Nodes.Size(); i++ )
			npcSpawns->SetString(UTIL_VarArgs("%f %f %f",g_Nodes[i]->GetAbsOrigin().x,
														   g_Nodes[i]->GetAbsOrigin().y,
														   g_Nodes[i]->GetAbsOrigin().z), "x");
		
		HL2MPRules()->GetMapData()->AddSubKey(npcSpawns);

		// save the spawn point
		HL2MPRules()->GetMapData()->SaveToFile(filesystem, UTIL_VarArgs("maps/graphs/%s.txt", STRING( gpGlobals->mapname )), "MOD");

		// clean up & exit node mode
		g_Nodes.PurgeAndDeleteElements();
		nodePlacement = NODE_OFF;
		UTIL_ClientPrintAll(HUD_PRINTTALK,"Saved large npc spawn points & exited spawn placement mode. Reload map to load new spawn points.\n");
	}
	else if ( nodePlacement == NODE_TELEPORT_BLOCKER )
	{
		// take the map data, delete all blockers in it, add all our new ones, and then save it
		KeyValues *mapData = new KeyValues("MapData");
		mapData->LoadFromFile(filesystem, UTIL_VarArgs("maps/graphs/%s.txt", STRING( gpGlobals->mapname )), "MOD");

		KeyValues *teleportBlockers = mapData->FindKey("TeleportBlockers");
		if ( teleportBlockers != NULL )
		{
			mapData->RemoveSubKey(teleportBlockers);
			teleportBlockers->deleteThis();
		}
		teleportBlockers = new KeyValues("TeleportBlockers");

		for ( int i=0; i<HL2MPRules()->GetTeleportBlockers()->Size(); i++ )
			teleportBlockers->SetString(UTIL_VarArgs("%f %f %f %.1f",
															HL2MPRules()->GetTeleportBlockers()->Element(i)->GetAbsOrigin().x,
															HL2MPRules()->GetTeleportBlockers()->Element(i)->GetAbsOrigin().y,
															HL2MPRules()->GetTeleportBlockers()->Element(i)->GetAbsOrigin().z,
															HL2MPRules()->GetTeleportBlockers()->Element(i)->GetRadius()), "x");
		mapData->AddSubKey(teleportBlockers);

		// save the new data
		mapData->SaveToFile(filesystem, UTIL_VarArgs("maps/graphs/%s.txt", STRING( gpGlobals->mapname )), "MOD");
		mapData->deleteThis();

		// clean up & exit node mode
		nodePlacement = NODE_OFF;
		int num = HL2MPRules()->GetTeleportBlockers()->Count();
		for ( int i=0; i<num; i++ )
			HL2MPRules()->GetTeleportBlockers()->Element(i)->SetVisible(false);
		UTIL_ClientPrintAll(HUD_PRINTTALK,"Saved teleport blockers & exited spawn placement mode. New blockers should be effective immediately.\n");
	}
}
static ConCommand cc_savenodes("savenodes", CC_SaveNodes, "Finish manually placing node elements, and save the .txt", FCVAR_CHEAT);


void CC_AddNode( void )
{
	if ( nodePlacement == NODE_OFF )
		return;

	CBasePlayer *pPlayer = UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() );
	Vector vecOrigin = pPlayer->GetMoveType() == MOVETYPE_NOCLIP ? pPlayer->EyePosition() : pPlayer->GetAbsOrigin() + Vector(0,0,10);
	
	if ( nodePlacement == NODE_AIRNODES )
	{// air nodes must have at least 64 units of empty space above them to be valid
		trace_t tr;
		UTIL_TraceLine(vecOrigin, vecOrigin + Vector(0,0,64), MASK_NPCSOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_NONE, &tr );
		
		if ( tr.DidHitWorld() )
		{
			float downDist = 64.0f * (1.0f - tr.fraction);
			UTIL_TraceLine(vecOrigin, vecOrigin - Vector(0,0,downDist), MASK_NPCSOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_NONE, &tr );
			if ( tr.DidHitWorld() )
			{
				ClientPrint(pPlayer,HUD_PRINTCONSOLE,"Insufficient space: air nodes must have 64 units free above them to be valid\n");
				return;			
			}
			else
			{
				vecOrigin = tr.endpos;
				ClientPrint(pPlayer,HUD_PRINTCONSOLE,"Shifted node downwards to fit\n");
			}
		}		
	
	}
	
	CreateNode(vecOrigin);
	UTIL_ClientPrintAll(HUD_PRINTCONSOLE,UTIL_VarArgs("Added node at %.0f, %.0f, %.0f\n",vecOrigin.x,vecOrigin.y,vecOrigin.z));
}
static ConCommand cc_addnode("addnode", CC_AddNode, "Manually place a node element at your feet. First, run startnodes / startspawns / startteleport.", FCVAR_CHEAT);


void CC_UndoNode( void )
{
	if ( nodePlacement == NODE_OFF )
		return;

	CBasePlayer *pPlayer = UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() );
	if ( nodePlacement == NODE_TELEPORT_BLOCKER )
	{
		CBaseEntity *pNode = HL2MPRules()->GetTeleportBlockers()->Tail();
		
		NDebugOverlay::Line( pPlayer->GetAbsOrigin(), pNode->GetAbsOrigin(), 0, 255, 255, true, 2.0f );		
		UTIL_Remove( pNode );
		HL2MPRules()->GetTeleportBlockers()->Remove( HL2MPRules()->GetTeleportBlockers()->Size()-1 );

		UTIL_ClientPrintAll(HUD_PRINTCONSOLE,"Last-placed node removed\n");
	}
	else if ( g_Nodes.Size() > 0 )
	{
		CBaseEntity *pNode = g_Nodes.Tail();
		
		NDebugOverlay::Line( pPlayer->GetAbsOrigin(), pNode->GetAbsOrigin(), 0, 255, 255, true, 2.0f );		
		UTIL_Remove( pNode );
		g_Nodes.Remove( g_Nodes.Size()-1 );

		UTIL_ClientPrintAll(HUD_PRINTCONSOLE,"Last-placed node removed\n");
	}
	else
		UTIL_ClientPrintAll(HUD_PRINTTALK,"Node list is empty\n");
}
static ConCommand cc_undonode("undonode", CC_UndoNode, "Delete the last placed node. First, run startnodes / startnpcspawns / startteleport.", FCVAR_CHEAT);


void CC_RemoveNearestNode( void )
{
	if ( nodePlacement == NODE_OFF )
		return;

	CBaseEntity *pNearestNode;
	float nearestDistSq = FLT_MAX;
	CBasePlayer *pPlayer = UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() );

	if ( nodePlacement == NODE_TELEPORT_BLOCKER && HL2MPRules()->GetTeleportBlockers()->Count() > 0 )
	{
		int nearestIndex = -1;
		for ( int i=0; i<HL2MPRules()->GetTeleportBlockers()->Count(); i++ )
		{
			float distSq = (HL2MPRules()->GetTeleportBlockers()->Element(i)->GetAbsOrigin() - pPlayer->GetAbsOrigin() ).LengthSqr();
			if ( distSq < nearestDistSq )
			{
				nearestDistSq = distSq;
				nearestIndex = i;
			}
		}
		
		pNearestNode = HL2MPRules()->GetTeleportBlockers()->Element(nearestIndex);
		HL2MPRules()->GetTeleportBlockers()->Remove( nearestIndex );
	}
	else if ( g_Nodes.Size() > 0 )
	{
		int nearestIndex = -1;
		for ( int i=0; i<g_Nodes.Count(); i++ )
		{
			float distSq = (g_Nodes[i]->GetAbsOrigin() - pPlayer->WorldSpaceCenter() ).LengthSqr();
			if ( distSq < nearestDistSq )
			{
				nearestDistSq = distSq;
				nearestIndex = i;
			}
		}
		
		pNearestNode = g_Nodes[nearestIndex];
		g_Nodes.Remove( nearestIndex );
	}
	else
		pNearestNode = NULL;

	if ( pNearestNode != NULL )
	{
		NDebugOverlay::Line( pPlayer->WorldSpaceCenter(), pNearestNode->GetAbsOrigin(), 0, 255, 255, true, 2.0f );
		UTIL_Remove( pNearestNode );
		UTIL_ClientPrintAll(HUD_PRINTCONSOLE,"Nearest node removed\n");
	}
	else
		UTIL_ClientPrintAll(HUD_PRINTTALK,"Node list is empty\n");
}
static ConCommand cc_removenearestnode("delnearestnode", CC_RemoveNearestNode, "Delete the nearest node to the player. First, run startnodes / startspawns / startteleport.", FCVAR_CHEAT);


void CC_StartAirNodes( void )
{
	if ( nodePlacement == NODE_AIRNODES )
		return;
	else if ( nodePlacement != NODE_OFF )
	{
		UTIL_ClientPrintAll(HUD_PRINTNOTIFY,"Cannot place air nodes while placing other nodes!\n");
		return;
	}

	// if the text file already exists, load its current nodes
	char szNodeTextFilename[MAX_PATH];
	Q_snprintf( szNodeTextFilename, sizeof( szNodeTextFilename ), "maps/graphs/%s_airgraph.txt", STRING( gpGlobals->mapname ) );
	CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
	if ( filesystem->ReadFile( szNodeTextFilename, "MOD", buf ) )
	{	
		if (!buf.Size())
			return;

		const int maxLen = 64;
		char line[ maxLen ];
		CUtlVector<char*> floats;
		int num = 0;

		// loop through every line of the file, read it in
		while( true )
		{
			buf.GetLine( line, maxLen );
			if ( Q_strlen(line) <= 0 )
				break; // reached the end of the file

			// we've read in a string containing 3 tab separated floats
			// we need to split this into 3 floats, which we put in a vector
			V_SplitString( line, "	", floats );
			Vector origin( atof( floats[0] ), atof( floats[1] ), atof( floats[2] ) );

			floats.PurgeAndDeleteElements();

			CreateNode(origin);
			num ++;

			if ( !buf.IsValid() )
				break;
		}
	}

	nodePlacement = NODE_AIRNODES;
	UTIL_ClientPrintAll(HUD_PRINTTALK,"Entered air node placement mode. Use noclip to fly around, and addnode, undonode, & delnearestnode commands to place air nodes. Use savenodes command to finish.\n");
}
static ConCommand cc_startairnodes("startairnodes", CC_StartAirNodes, "Start manually placing air nodegraph elements, with addnode, undonode, & delnearestnode commands. Finish with savenodes.", FCVAR_CHEAT);


void CC_StartNpcSpawns( void )
{
	if ( nodePlacement == NODE_NPC_SPAWNS )
		return;
	else if ( nodePlacement != NODE_OFF )
	{
		UTIL_ClientPrintAll(HUD_PRINTNOTIFY,"Cannot place NPC spawn points while placing other nodes!\n");
		return;
	}

	// if map data already has NPC spawns, load and visualise them
	KeyValues *npcSpawns = HL2MPRules()->GetMapData()->FindKey("NpcSpawns");
	if ( npcSpawns != NULL )
	{
		int num = 0;
		
		// all map info files *should* have NPC spawns... most other stuff is fairly optional
		for ( KeyValues *sub = npcSpawns->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
		{//	sub->GetName(), sub->GetString();
			CUtlVector<char*> floats;
			V_SplitString( sub->GetName(), " ", floats );
			Vector origin( atof( floats[0] ), atof( floats[1] ), atof( floats[2] ) );
			floats.PurgeAndDeleteElements();

			CreateNode(origin);
			num ++;
		}
	}
	else
		Warning( "Map data file contains no NPC spawn points\n");

	nodePlacement = NODE_NPC_SPAWNS;
	UTIL_ClientPrintAll(HUD_PRINTTALK,"Entered spawn placement mode. Use addnode, undonode, & delnearestnode commands to place npc spawn points. Use savenodes command to finish.\n");
}
static ConCommand cc_startnpcspawns("startnpcspawns", CC_StartNpcSpawns, "Start manually placing npc spawn points, with addnode, undonode, & delnearestnode commands. Finish with savenodes.", FCVAR_CHEAT);

void CC_StartLargeNpcSpawns( void )
{
	if ( nodePlacement == NODE_LARGE_NPC_SPAWNS )
		return;
	else if ( nodePlacement != NODE_OFF )
	{
		UTIL_ClientPrintAll(HUD_PRINTNOTIFY,"Cannot place large NPC spawn points while placing other nodes!\n");
		return;
	}

	// if map data already has NPC spawns, load and visualise them
	KeyValues *npcSpawns = HL2MPRules()->GetMapData()->FindKey("LargeNpcSpawns");
	if ( npcSpawns != NULL )
	{
		int num = 0;
		
		// all map info files *should* have NPC spawns... most other stuff is fairly optional
		for ( KeyValues *sub = npcSpawns->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
		{//	sub->GetName(), sub->GetString();
			CUtlVector<char*> floats;
			V_SplitString( sub->GetName(), " ", floats );
			Vector origin( atof( floats[0] ), atof( floats[1] ), atof( floats[2] ) );
			floats.PurgeAndDeleteElements();

			CreateNode(origin);
			num ++;
		}
	}
	else
		Warning( "Map data file contains no large NPC spawn points\n");

	nodePlacement = NODE_LARGE_NPC_SPAWNS;
	UTIL_ClientPrintAll(HUD_PRINTTALK,"Entered spawn placement mode. Use addnode, undonode, & delnearestnode commands to place large npc spawn points. Use savenodes command to finish.\n");
}
static ConCommand cc_startlargenpcspawns("startlargenpcspawns", CC_StartLargeNpcSpawns, "Start manually placing large npc spawn points, with addnode, undonode, & delnearestnode commands. Finish with savenodes.", FCVAR_CHEAT);


void CC_StartTeleport( void )
{
	if ( nodePlacement == NODE_TELEPORT_BLOCKER )
		return;
	else if ( nodePlacement != NODE_OFF )
	{
		UTIL_ClientPrintAll(HUD_PRINTNOTIFY,"Cannot place teleport blockers while placing other nodes!\n");
		return;
	}

	int num = HL2MPRules()->GetTeleportBlockers()->Count();
	for ( int i=0; i<num; i++ )
		HL2MPRules()->GetTeleportBlockers()->Element(i)->SetVisible(true);

	nodePlacement = NODE_TELEPORT_BLOCKER;
	UTIL_ClientPrintAll(HUD_PRINTTALK,"Entered teleport blocker placement mode. Use addnode, undonode & delnearestnode commands to place teleport blocker spheres. Use resizenode to adjust the radius of the nearest blocker. Use savenodes command to finish.\n");
}
static ConCommand cc_startteleport("startteleport", CC_StartTeleport, "Start manually placing teleport nodes, with addnode, undonode, & delnearestnode commands. Use resizenode to adjust the radius of the nearest blocker. Finish with savenodes.", FCVAR_CHEAT);

void CC_ResizeNode(const CCommand& args )
{
	if ( args.ArgC() < 2 || nodePlacement != NODE_TELEPORT_BLOCKER )
		return;

	float radius = atof(args[1]);
	int number = args.ArgC() < 3 ? 1 : atoi(args[2]);

	int numBlockers = HL2MPRules()->GetTeleportBlockers()->Count();
	if ( numBlockers > 0 )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() );
		CUtlVector<int> nearestNodes;
		CUtlVector<float> distances;
		for ( int i=0; i<numBlockers; i++ )
		{
			float distSq = (HL2MPRules()->GetTeleportBlockers()->Element(i)->GetAbsOrigin() - pPlayer->WorldSpaceCenter() ).LengthSqr();
			int numSaved = nearestNodes.Count();
			if ( numSaved < number || distSq < distances.Tail() )
			{
				for ( int j=0; j<number; j++ )
					if ( j >= numSaved )
					{
						nearestNodes.AddToTail(i);
						distances.AddToTail(distSq);
						break;
					}
					else if ( distSq < distances[j] )
					{
						nearestNodes.Remove(j);
						distances.Remove(j);
						nearestNodes.InsertBefore(j,i);
						distances.InsertBefore(j,distSq);
						break;
					}
			}
		}
		
		for ( int i=0; i<nearestNodes.Count(); i++ )
		{
			CTeleportBlocker *pNearestNode = HL2MPRules()->GetTeleportBlockers()->Element(nearestNodes[i]);
			pNearestNode->SetRadius(radius);
			NDebugOverlay::Line( pPlayer->WorldSpaceCenter(), pNearestNode->GetAbsOrigin(), 0, 255, 0, true, 2.0f );
		}
	}
	else
		UTIL_ClientPrintAll(HUD_PRINTTALK,"Node list is empty\n");
}
static ConCommand cc_resizenode("resizenode", CC_ResizeNode, "Resize the nearest teleport blocker to the player. First, run startteleport.", FCVAR_CHEAT);


void CC_NodeSize()
{
	if ( nodePlacement != NODE_TELEPORT_BLOCKER )
		return;

	float nearestDistSq = FLT_MAX;
	CBasePlayer *pPlayer = UTIL_PlayerByIndex( UTIL_GetCommandClientIndex() );

	int num = HL2MPRules()->GetTeleportBlockers()->Count();
	if ( num > 0 )
	{
		int nearestIndex = -1;
		for ( int i=0; i<num; i++ )
		{
			float distSq = (HL2MPRules()->GetTeleportBlockers()->Element(i)->GetAbsOrigin() - pPlayer->WorldSpaceCenter() ).LengthSqr();
			if ( distSq < nearestDistSq )
			{
				nearestDistSq = distSq;
				nearestIndex = i;
			}
		}
		
		CTeleportBlocker *pNearestNode = HL2MPRules()->GetTeleportBlockers()->Element(nearestIndex);
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs("Node radius is %.1f\n", pNearestNode->GetRadius()));
		NDebugOverlay::Line( pPlayer->WorldSpaceCenter(), pNearestNode->GetAbsOrigin(), 0, 255, 0, true, 2.0f );
	}
}
static ConCommand cc_nodesize("nodesize", CC_NodeSize, "Returns the radius of the nearest teleport blocker to the player. First, run startteleport.", FCVAR_CHEAT);

extern void LoadMonsterStats();
extern CNPCTypeInfo *GetNPCInfo(const char *name);
void CC_ReloadStats( void )
{
	LoadMonsterStats();
	
	// now we need to correct every monster's pointer to its stats file
	// SetLevel does this, so call it with an unchanged level
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	for ( int i = 0; i < nAIs; i++ )
	{
		CAI_BaseNPC *pNPC = ppAIs[i];
		if ( !pNPC )
			continue;

		pNPC->SetStats( GetNPCInfo(pNPC->GetTypeName()), pNPC->GetLevel(), false );
	}

	UTIL_ClientPrintAll(HUD_PRINTNOTIFY,"All monster stats reloaded\n");
}
static ConCommand cc_reloadstats("pvm_reloadstats", CC_ReloadStats, "Reload all monster stats keyvalues files immediately", FCVAR_GAMEDLL);