#include "cbase.h"
#include "player_limited_quantities.h"
#include "hl2mp_player.h"
#include "utlvector.h"
#include "ai_basenpc.h"
#include "hl2mp/grenade_tripmine.h"
#include "hl2/npc_turret_floor.h"
#include "hl2/npc_manhack.h"
#include "modcom/mcconvar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int LimitedQuantity::GetLimit(CHL2MP_Player *pPlayer)
{
	if ( m_ModuleIndex == NO_MODULE || m_limit_scale == NULL || m_limit_power == NULL)
		return m_limit_base->GetInt();
	else
		return LEVEL_POINTER(m_limit, pPlayer->GetModuleLevel(m_ModuleIndex));
}

// before this class itself, we need our "clear" functions for each type
extern void KillAllNPCsWithMaster(CHL2MP_Player *master, bool minionsOnly);
void ClearMinions(CHL2MP_Player *pPlayer)
{
	KillAllNPCsWithMaster(pPlayer,true);
}

void ClearLasers(CHL2MP_Player *pPlayer)
{
	CBaseEntity *pEntity = NULL;
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_tripmine" )) != NULL)
	{
		CTripmineGrenade *pMine = dynamic_cast<CTripmineGrenade *>(pEntity);
		if (pMine->m_hOwner == pPlayer )
		{
			pMine->m_takedamage	= DAMAGE_NO;

			pMine->SetThink( &CTripmineGrenade::DelayDeathThink );
			pMine->SetNextThink( gpGlobals->curtime + 0.25 );

			pMine->EmitSound( "TripmineGrenade.StopSound" );
		}
	}
}

void ClearCrows(CHL2MP_Player *pPlayer)
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_crow" )) != NULL)
	{
		CAI_BaseNPC *pCrow = dynamic_cast<CAI_BaseNPC *>(pEntity);
		if (pCrow && pCrow->LikesMaster() && pCrow->GetMasterPlayer() == pPlayer )
		{
			pCrow->SetMasterPlayer(NULL); // stop this counting as a kill for anyone
			CTakeDamageInfo info( pCrow, pCrow, pCrow->GetMaxHealth()*2, DMG_GENERIC ); // enough to splat it
			pCrow->TakeDamage(info); // kill
		}
	}
}
/*
void ClearBarnacles(CHL2MP_Player *pPlayer)
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_barnacle" )) != NULL)
	{
		CAI_BaseNPC *pBarnacle = dynamic_cast<CAI_BaseNPC *>(pEntity);
		if (pBarnacle && pBarnacle->LikesMaster() && pBarnacle->GetMasterPlayer() == pPlayer )
		{
			pBarnacle->SetMasterPlayer(NULL); // stop this counting as a kill for anyone
			CTakeDamageInfo info( pBarnacle, pBarnacle, pBarnacle->GetMaxHealth()*2, DMG_GENERIC ); // enough to splat it
			pBarnacle->TakeDamage(info); // kill
		}
	}
}
*/
void ClearMagmines(CHL2MP_Player *pPlayer)
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "grenade_magmine" )) != NULL)
	{
		CMagMine *pMine = dynamic_cast<CMagMine*>(pEntity);
		if ( pPlayer == NULL || pMine->GetThrower() == pPlayer )
		{
			CTakeDamageInfo info( pMine, pMine, 100, DMG_GENERIC );
			pMine->Event_Killed(info);
		}
	}
}
extern McConVar mc_player_max_minions;
extern LEVEL_EXTERN(mod_lasers_limit);
extern LEVEL_EXTERN(mod_turret_limit);
extern LEVEL_EXTERN(mod_crow_limit);
extern LEVEL_EXTERN(mod_magmine_limit);
extern LEVEL_EXTERN(mod_manhack_limit);

LimitedQuantities::LimitedQuantities(CHL2MP_Player *pPlayer)
{
	m_pMyPlayer = pPlayer;
	AddNewType(LQ_MINION, &mc_player_max_minions, NULL, NULL, NO_MODULE, ClearMinions);
	AddNewType(LQ_LASER, &mod_lasers_limit_base, &mod_lasers_limit_scale, &mod_lasers_limit_power, LASERS, ClearLasers);
	AddNewType(LQ_TURRET, &mod_turret_limit_base, &mod_turret_limit_scale, &mod_turret_limit_power, TURRET, ClearMinions);
	AddNewType(LQ_CROW, &mod_crow_limit_base, &mod_crow_limit_scale, &mod_crow_limit_power, CROW, ClearCrows);
	AddNewType(LQ_MAGMINE, &mod_magmine_limit_base, &mod_magmine_limit_scale, &mod_magmine_limit_power, MAGMINE, ClearMagmines);
	AddNewType(LQ_MANHACK, &mod_manhack_limit_base, &mod_manhack_limit_scale, &mod_manhack_limit_power, MINION_MANHACK, ClearMinions);
}

LimitedQuantities::~LimitedQuantities()
{
	m_quantities.PurgeAndDeleteElements();
}

int LimitedQuantities::GetLimit(limited_quantity_t name)
{
	return GetByName(name)->GetLimit(m_pMyPlayer);
}

int LimitedQuantities::GetCount(limited_quantity_t name)
{
	return GetByName(name)->GetCount();
}

bool LimitedQuantities::IsFull(limited_quantity_t name, int desiredNumToAdd)
{
	return GetByName(name)->IsFull(m_pMyPlayer, desiredNumToAdd);
}

void LimitedQuantities::Add(limited_quantity_t name, int num)
{
	GetByName(name)->Add(num);
}

void LimitedQuantities::Remove(limited_quantity_t name, int num)
{
	GetByName(name)->Remove(num);
}

void LimitedQuantities::AddNewType(limited_quantity_t name, McConVar *limit_base, McConVar *limit_scale, McConVar *limit_power, int moduleIndex, ClearFunc c)
{
	LimitedQuantity *q = new LimitedQuantity(name,limit_base,limit_scale,limit_power,moduleIndex,c);
	m_quantities.AddToTail( q );
}
void LimitedQuantities::ResetAll()
{
	int num = m_quantities.Count();
	for ( int i=0; i<num; i++ )
	{
		m_quantities.Element(i)->GetClearFunction()(m_pMyPlayer);
		m_quantities.Element(i)->SetCountToZero();
	}
}

void LimitedQuantities::Reset(limited_quantity_t name)
{
	LimitedQuantity *l = GetByName(name);
	l->SetCountToZero();
	l->GetClearFunction()(m_pMyPlayer);
}

// gah, this is where it all falls down, probably
LimitedQuantity *LimitedQuantities::GetByName(limited_quantity_t name)
{
/*
	LimitedQuantity *l = new LimitedQuantity(name,0,NULL); // sneaky use of Find, as == only compares names
	l = m_quantities.Element(m_quantities.Find(l));
*/
	
	int num = m_quantities.Count();
	for ( int i=0; i<num; i++ )
	{
		LimitedQuantity *l = m_quantities.Element(i);
		if ( name == l->GetName() )
			return l;
	}

	Error("Attempting to access player limited quantity of unknown type '%i'\n",name);
	return NULL;
}