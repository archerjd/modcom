//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Handling for the suit batteries.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2_player.h"
#include "hl2mp_player.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "modcom/mcconvar.h"
#include "engine/IEngineSound.h"

#ifdef USE_OMNIBOT
#include "../omnibot/omnibot_interface.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CItemBattery : public CItem
{
public:
	DECLARE_CLASS( CItemBattery, CItem );
#ifdef USE_OMNIBOT
	int GetOmnibotClass() const {
		return Omnibot::MC_CLASSEX_BATTERY;
	}
#endif
	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/battery.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/battery.mdl");

		PrecacheScriptSound( "ItemBattery.Touch" );

	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->IsBuffActive(DEBUFF_ATTRITION) )
			return false;
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>( pPlayer );
		return ( pHL2Player && pHL2Player->ApplyBattery() );
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);
PRECACHE_REGISTER(item_battery);

//-----------------------------------------------------------------------------
// Purpose: Aux pickup. Currently randomly placed on the map.
//-----------------------------------------------------------------------------
extern McConVar mc_auxpower_pickup_energy;
#define AUX_POWER_PICKUP_MODEL "models/items/batter2.mdl"
 
class CItemAuxBattery : public CItem
{
public:
	DECLARE_CLASS( CItemAuxBattery, CItem );
#ifdef USE_OMNIBOT
	int GetOmnibotClass() const {
		return Omnibot::MC_CLASSEX_BATTERY;
	}
#endif
	void Spawn( void )
	{ 
		Precache( );
		SetModel( AUX_POWER_PICKUP_MODEL );
		Randomize();
		BaseClass::Spawn( );

//		SetThink( &CItemAuxPickup::Randomize );
//		SetNextThink(gpGlobals->curtime + 50.0f);
	}
	void Precache( void )
	{
		PrecacheModel (AUX_POWER_PICKUP_MODEL);
		PrecacheScriptSound( "ItemBattery.Touch" );
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->IsBuffActive(DEBUFF_ATTRITION) )
			return false;

		CHL2MP_Player *pHl2mpPlayer = dynamic_cast<CHL2MP_Player *>( pPlayer );
		if ( !pHl2mpPlayer || pHl2mpPlayer->GetAuxPower() >= pHl2mpPlayer->GetMaxAuxPower() )
			return false;
		
		pHl2mpPlayer->AddAuxPower( mc_auxpower_pickup_energy.GetFloat() );
		CPASAttenuationFilter filter( pHl2mpPlayer, "ItemBattery.Touch" );
		EmitSound( filter, pHl2mpPlayer->entindex(), "ItemBattery.Touch" );

		// make a new one here, as just respawning won't make it change position
		CBaseEntity *pPower = CreateEntityByName("item_aux_battery");
		pPower->Spawn();
		pPower->Respawn(); // don't appear til respawn timer is done
		pPower->AddFlag(SF_NORESPAWN);
		
		UTIL_Remove(this);
		return true;
	}
	virtual CBaseEntity* Respawn( void )
	{
		BaseClass::Respawn();
		Randomize(); // next time, don't respawn in the same place
		UTIL_DropToFloor( this, MASK_SOLID );
		return this;
	}
	void Randomize( void )
	{
		SetAbsOrigin( HL2MPRules()->GetRandomSpawnPoint( Vector(-1,-1,-1), Vector(1,1,1) ) );
		SetAbsAngles( HL2MPRules()->GetRandomAngle() );
	}
};

/*
BEGIN_DATADESC( CItemAuxPickup )
	DEFINE_FUNCTION( RandomizeThink ),
END_DATADESC()
*/
LINK_ENTITY_TO_CLASS(item_aux_battery, CItemAuxBattery);
PRECACHE_REGISTER(item_aux_battery);



#define SCORE_TOKEN_MODEL "models/strider_parts/strider_brain.mdl"
class CItemScoreToken : public CItem
{
public:
	DECLARE_CLASS( CItemScoreToken, CItem );
#ifdef USE_OMNIBOT
	int GetOmnibotClass() const {
		return Omnibot::MC_CLASSEX_POWERCUBE;
	}
#endif
	void Spawn( void )
	{ 
		Precache( );
		SetModel( SCORE_TOKEN_MODEL );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel (SCORE_TOKEN_MODEL);

		PrecacheScriptSound( "ScoreToken.Pickup" );
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		pPlayer->AddScoreToken();
		CPASAttenuationFilter filter( pPlayer, "ScoreToken.Pickup" );
		EmitSound( filter, pPlayer->entindex(), "ScoreToken.Pickup" );
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_score_token, CItemScoreToken);
PRECACHE_REGISTER(item_score_token);