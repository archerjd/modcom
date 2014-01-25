//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client's C_BaseCombatCharacter entity
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basecombatcharacter.h"
#include "model_types.h"
#include "modcom/particle_effects.h"
#include "r_efx.h"
#include "dlight.h"
#include "networkstringtable_clientdll.h"
#include "c_ai_basenpc.h"
#include "hl2mp/c_hl2mp_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if defined( CBaseCombatCharacter )
#undef CBaseCombatCharacter	
#endif

extern const char *g_szEntityEffectNames[LAST_ENTITY_EFFECT];
extern float g_EntityEffectDurations[LAST_ENTITY_EFFECT];
extern INetworkStringTable *g_pSkinOverrideStringTable;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_BaseCombatCharacter::C_BaseCombatCharacter()
{
	for ( int i=0; i < m_iAmmo.Count(); i++ )
		m_iAmmo.Set( i, 0 );

	m_iRememberedParticleEffect = m_iParticleEffect = 0;
	m_flNextParticleEffect = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_BaseCombatCharacter::~C_BaseCombatCharacter()
{
}

/*
//-----------------------------------------------------------------------------
// Purpose: Returns the amount of ammunition of the specified type the character's carrying
//-----------------------------------------------------------------------------
int	C_BaseCombatCharacter::GetAmmoCount( char *szName ) const
{
	return GetAmmoCount( g_pGameRules->GetAmmoDef()->Index(szName) );
}
*/

//-----------------------------------------------------------------------------
// Purpose: Overload our muzzle flash and send it to any actively held weapon
//-----------------------------------------------------------------------------
void C_BaseCombatCharacter::DoMuzzleFlash()
{
	// Our weapon takes our muzzle flash command
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		pWeapon->DoMuzzleFlash();
		//NOTENOTE: We do not chain to the base here
	}
	else
	{
		BaseClass::DoMuzzleFlash();
	}
}

bool C_BaseCombatCharacter::IsBuffActive(Buff *a)
{
	return m_bBuffs.Get(a->GetBuffIndex());
}

bool C_BaseCombatCharacter::IsBuffActive(int i)
{
	return m_bBuffs.Get(i);
}

// called by c_ai_basenpc & c_hl2mp_player during ClientThink
void C_BaseCombatCharacter::ManageBuffs()
{
	for ( int i=0; i<NUM_BUFFS; i++ )
	{
		bool bOn = IsBuffActive(i);
		if ( bOn != m_bRememberedBuffStates[i] )
		{// changed this tick!
			if ( bOn )
				StartBuffEffects(i);
			else
				StopBuffEffects(i);

			m_bRememberedBuffStates[i] = bOn;
		}
		if ( bOn )
			MaintainBuffEffects(i);
	}

	//now handle MODULE effects
	if ( IsAlive() && (!IsPlayer() || ToHL2MPPlayer(this)->IsInCharacter()) )
		for ( int i=0; i<NUM_MODULES; i++ )
		{
			Module *a = GetModule(i);
			if ( (IsModuleActive(a) || !a->IsToggled()) && m_flModuleCooldownTime[i] != m_flLastModuleCooldownTime[i] )
			{// cooldown changed - checking its active stops spawning from affecting it, but do a clever check for instant
				float timeSinceAssumedStart = gpGlobals->curtime - (m_flModuleCooldownTime[i] - a->GetCooldown(GetModuleLevel(a)));
				if ( timeSinceAssumedStart > 0 && timeSinceAssumedStart < 0.1f )
				{
					m_flLastModuleCooldownTime[i] = m_flModuleCooldownTime[i];
					
					const char *particleName;
					if ( IsPlayer() && ToHL2MPPlayer(this)->IsLocalPlayer() )
						particleName = a->GetLocalParticleEffect();
					else
						particleName = a->GetParticleEffect();

					if ( particleName != NULL )
					{
						//DispatchParticleEffect(particleName, PATTACH_ABSORIGIN_FOLLOW, this );
						//int attachments = a->GetParticleAttachments();
						CParticleProperty *pProp = ParticleProp();
						CNewParticleEffect *pFX = pProp->Create( particleName, PATTACH_ABSORIGIN_FOLLOW );
						if ( pFX && pProp )
						{
							pProp->AddControlPoint( pFX, 0, this, PATTACH_ABSORIGIN_FOLLOW );
							/*int pointNum = 1;

							// if we do attachments, they'll go in here

							Vector cp1 = a->ParticleEffectControlPointOffset(***LEVEL***);
							if ( cp1 != vec3_origin )
							{
								pProp->AddControlPoint( pFX, pointNum, this, PATTACH_ABSORIGIN_FOLLOW, 0, cp1 );
								pointNum ++;
							}*/
						}
					}
				}
			}
		}
}

const char *g_ppszCombineModels[] =
{
	"models/combine_soldier.mdl",
	"models/combine_soldier_prisonguard.mdl",
	"models/combine_super_soldier.mdl",
	"models/police.mdl",
};

// unfortunately different player models and npcs often have different names for their attachments... this function gets around that as best as we can
const char *ResolveAttachmentName(int modelIndex, int attachment)
{
	const char *modelname = modelinfo->GetModelName(modelinfo->GetModel(modelIndex));
	
	switch ( attachment )
	{
	case BUFF_ATTACHMENT_EYES:
	{
		if ( FStrEq("models/Hunter.mdl", modelname) )
			return "top_eye";
		else if ( FStrEq("models/antlion_worker.mdl", modelname) )
			return "mouth";
		return "eyes";
	}
	case BUFF_ATTACHMENT_CHEST:
	{
		if ( FStrEq("models/vortigaunt.mdl", modelname) )
			return "attach_lShoulerBladeAim";
		else if ( FStrEq("models/Hunter.mdl", modelname) )
			return "head_radius_measure";
		int num = ARRAYSIZE( g_ppszCombineModels );
		for ( int i=0; i<num; i++ )
			if ( FStrEq(g_ppszCombineModels[i], modelname) )
				return "beam_damagechest";
		return "chest";
	}
	case BUFF_ATTACHMENT_LEFT_HAND:
		if ( FStrEq("models/vortigaunt.mdl", modelname) )
			return "leftclaw";
		else if ( FStrEq("models/Hunter.mdl", modelname) )
			return "left_toe";
		else if ( FStrEq("models/AntLion.mdl", modelname) || FStrEq("models/antlion_worker.mdl", modelname) || FStrEq("models/antlion_guard.mdl", modelname))
			return "leftfront";
		return "anim_attachment_LH";

	case BUFF_ATTACHMENT_RIGHT_HAND:
		if ( FStrEq("models/vortigaunt.mdl", modelname) )
			return "rightclaw";
		else if ( FStrEq("models/Hunter.mdl", modelname) )
			return "right_toe";
		else if ( FStrEq("models/AntLion.mdl", modelname) || FStrEq("models/antlion_worker.mdl", modelname) || FStrEq("models/antlion_guard.mdl", modelname))
			return "rightfront";
		return "anim_attachment_RH";

	default:
		return 0;
	}
}

// that is, particle effects
void C_BaseCombatCharacter::StartBuffEffects(int i)
{
	Buff *a = GetBuff(i);
	const char *particleName;
	if ( C_BasePlayer::GetLocalPlayer() == this )
		particleName = a->GetLocalParticleEffect();
	else
		particleName = a->GetParticleEffect();
	if ( particleName != NULL )
	{
		int attachments = a->GetParticleAttachments();
		//DispatchParticleEffect(particleName, PATTACH_ABSORIGIN_FOLLOW, this );

		CParticleProperty *pProp = ParticleProp();
		CNewParticleEffect *pFX = pProp->Create( particleName, PATTACH_ABSORIGIN_FOLLOW );
		if ( pFX )
		{
			pProp->AddControlPoint( pFX, 0, this, PATTACH_ABSORIGIN_FOLLOW );
			int pointNum = 1;
			if ( attachments & BUFF_ATTACHMENT_EYES )
			{
				const char *pszAttachment = ResolveAttachmentName(GetModelIndex(),BUFF_ATTACHMENT_EYES);
				int iAttachment = LookupAttachment( pszAttachment );
				//Msg(VarArgs("iAttachment is %i\n", iAttachment));
				if ( iAttachment <= 0 )
					pProp->AddControlPoint( pFX, pointNum, this, PATTACH_ABSORIGIN_FOLLOW, 0, Vector(0,0,BoundingRadius()) );
				else
					pProp->AddControlPoint( pFX, pointNum, this, PATTACH_POINT_FOLLOW, pszAttachment);
				pointNum++;
			}
			else if ( attachments & BUFF_ATTACHMENT_CHEST )
			{
				const char *pszAttachment = ResolveAttachmentName(GetModelIndex(),BUFF_ATTACHMENT_CHEST);
				int iAttachment = LookupAttachment( pszAttachment );
				if ( iAttachment <= 0 )
					pProp->AddControlPoint( pFX, pointNum, this, PATTACH_ABSORIGIN_FOLLOW, 0, Vector(0,0,BoundingRadius()) );
				else
					pProp->AddControlPoint( pFX, pointNum, this, PATTACH_POINT_FOLLOW, pszAttachment);
				pointNum++;
			}
			else if ( attachments & BUFF_ATTACHMENT_LEFT_HAND )
			{
				const char *pszAttachment = ResolveAttachmentName(GetModelIndex(),BUFF_ATTACHMENT_LEFT_HAND);
				int iAttachment = LookupAttachment( pszAttachment );
				if ( iAttachment <= 0 )
					pProp->AddControlPoint( pFX, pointNum, this, PATTACH_ABSORIGIN_FOLLOW, 0, Vector(0,0,BoundingRadius()) );
				else
					pProp->AddControlPoint( pFX, pointNum, this, PATTACH_POINT_FOLLOW, pszAttachment);
				pointNum++;
			}
			else if ( attachments & BUFF_ATTACHMENT_RIGHT_HAND )
			{
				const char *pszAttachment = ResolveAttachmentName(GetModelIndex(),BUFF_ATTACHMENT_RIGHT_HAND);
				int iAttachment = LookupAttachment( pszAttachment );
				if ( iAttachment <= 0 )
					pProp->AddControlPoint( pFX, pointNum, this, PATTACH_ABSORIGIN_FOLLOW, 0, Vector(0,0,BoundingRadius()) );
				else
					pProp->AddControlPoint( pFX, pointNum, this, PATTACH_POINT_FOLLOW, pszAttachment);
				pointNum++;
			}
		}
	}
}

void C_BaseCombatCharacter::MaintainBuffEffects(int i)
{

}

void C_BaseCombatCharacter::StopBuffEffects(int i)
{
	const char *particleName = GetBuff(i)->GetParticleEffect();
	if ( particleName != NULL )
	{
		StopParticleEffects(this);

		// but this will stop them all, so start all the others (that are currently active) up again
		for ( int j=0; j<GetNumBuffs(); j++ )
			if ( IsBuffActive(GetBuff(j)) )
				StartBuffEffects(j);
	}
}

#define MAX_OVERLAYS 5

void RenderOverlays(C_BaseAnimating *pModel, C_BaseCombatCharacter *pBCC)
{
	int drawFlags = STUDIO_RENDER|STUDIO_TRANSPARENCY|STUDIO_NOSHADOWS; // STUDIO_TWOPASS?
	int drawFlagsGlow = drawFlags|STUDIO_ITEM_BLINK;

	// for every active affliction that uses a material proxy ... draw us again using that material
	// do we want to limit the max number of times it can draw?
	// if someone has spree + damage amp + weaken + freeze + plague on them at once, would we rather be inefficient or not draw something (eg plague)
	int numOverlays = 0;
	for ( int i=0; i<NUM_BUFFS; i++ )
	{
		Buff *a = GetBuff(i);
		if ( numOverlays < MAX_OVERLAYS && pBCC->IsBuffActive(i) && a->GetMaterialEffect() != NULL )
		{
			modelrender->ForcedMaterialOverride( materials->FindMaterial( a->GetMaterialEffect(), TEXTURE_GROUP_CLIENT_EFFECTS ) );
			pModel->C_BaseAnimating::DrawModel(a->PulsatingGlow() ? drawFlagsGlow : drawFlags); // not affected by changing render color - even when forced override is off - what is going on??
			modelrender->ForcedMaterialOverride(0);
			numOverlays ++;
		}
	}
}

// lets us do coloring and material effects
int C_BaseCombatCharacter::DrawModel( int flags )
{
	// what a place to handle this... :/
	if ( ( m_iParticleEffect != m_iRememberedParticleEffect && m_iParticleEffect > 0 && m_iParticleEffect < LAST_ENTITY_EFFECT )
		|| ( g_EntityEffectDurations[m_iParticleEffect] > 0 && m_flNextParticleEffect <= gpGlobals->curtime + g_EntityEffectDurations[m_iParticleEffect] ) )
	{
		//DispatchParticleEffect( g_szEntityEffectNames[m_iParticleEffect], PATTACH_ABSORIGIN_FOLLOW, this );

		CParticleProperty * pProp = ParticleProp();
		CNewParticleEffect *pFX = pProp->Create( g_szEntityEffectNames[m_iParticleEffect], PATTACH_ABSORIGIN_FOLLOW );
		if ( pFX )
		{
			pProp->AddControlPoint( pFX, 0, this, PATTACH_ABSORIGIN_FOLLOW );
			int particleCPHeight = 0;
			switch ( m_iParticleEffect )
			{
			case SPAWN_HUMAN:
				particleCPHeight = 72; break;
			case SPAWN_SMALL:
				particleCPHeight = 18; break;
			case SPAWN_MEDIUM:
				particleCPHeight = 46; break;
			case SPAWN_LARGE:
				particleCPHeight = 90; break;
			}
			if ( particleCPHeight > 0 )
				pProp->AddControlPoint( pFX, 1, this, PATTACH_ABSORIGIN_FOLLOW, 0, Vector(0,0,particleCPHeight) );
		}



		m_iRememberedParticleEffect = m_iParticleEffect;

		if ( g_EntityEffectDurations[m_iParticleEffect] > 0 )
			m_flNextParticleEffect = gpGlobals->curtime + g_EntityEffectDurations[m_iParticleEffect];
	}

	int skinOverride = 0;
	int retVal;
	if ( IsNPC() )
		skinOverride = MyNPCPointer()->GetSkinOverride();
	if ( skinOverride > 0 )
	{
		modelrender->ForcedMaterialOverride( materials->FindMaterial( g_pSkinOverrideStringTable->GetString(skinOverride), TEXTURE_GROUP_CLIENT_EFFECTS ) );
		retVal = BaseClass::DrawModel(flags);
		modelrender->ForcedMaterialOverride(0);
	}
	else
		retVal = BaseClass::DrawModel(flags);

	RenderOverlays(this,this);
	return retVal;
}

IMPLEMENT_CLIENTCLASS(C_BaseCombatCharacter, DT_BaseCombatCharacter, CBaseCombatCharacter);

// Only send active weapon index to local player
BEGIN_RECV_TABLE_NOBASE( C_BaseCombatCharacter, DT_BCCLocalPlayerExclusive )
	RecvPropTime( RECVINFO( m_flNextAttack ) ),
END_RECV_TABLE();


BEGIN_RECV_TABLE(C_BaseCombatCharacter, DT_BaseCombatCharacter)
	RecvPropDataTable( "bcc_localdata", 0, 0, &REFERENCE_RECV_TABLE(DT_BCCLocalPlayerExclusive) ),
	RecvPropEHandle( RECVINFO( m_hActiveWeapon ) ),
	RecvPropArray3( RECVINFO_ARRAY(m_hMyWeapons), RecvPropEHandle( RECVINFO( m_hMyWeapons[0] ) ) ),
	RecvPropInt( RECVINFO( m_iLevel ) ),
	RecvPropInt( RECVINFO( m_iParticleEffect ) ),

#ifdef INVASION_CLIENT_DLL
	RecvPropInt( RECVINFO( m_iPowerups ) ),
#endif

END_RECV_TABLE()


BEGIN_PREDICTION_DATA( C_BaseCombatCharacter )

	DEFINE_PRED_ARRAY( m_iAmmo, FIELD_INTEGER,  MAX_AMMO_TYPES, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flNextAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hActiveWeapon, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_ARRAY( m_hMyWeapons, FIELD_EHANDLE, MAX_WEAPONS, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA()
