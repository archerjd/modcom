//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include <KeyValues.h>
#include "FunctionProxy.h"
#include "toolframework_client.h"
#include "hl2mp/c_hl2mp_player.h"
#include "c_ai_basenpc.h"
#include "c_basecombatweapon.h"
#include "hl2mp_gamerules.h"
#include "modcom/mc_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// forward declarations
void ToolFramework_RecordMaterialParams( IMaterial *pMaterial );


#include "materialsystem/imaterialproxy.h"

class IMaterial;
class IMaterialVar;

#pragma warning (disable : 4100)

class CCustomProxy : public IMaterialProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity ) = 0;
	virtual void Release( void ) { delete this; }
	virtual IMaterial *GetMaterial();

protected:
	C_BaseEntity *BindArgToEntity( void *pArg );
	IMaterialVar *m_pBaseTextureVar;
};

//EXPOSE_INTERFACE( CCustomProxy, IMaterialProxy, "WeaponSkin" IMATERIAL_PROXY_INTERFACE_VERSION );

C_BaseEntity *CCustomProxy::BindArgToEntity( void *pArg )
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return pRend->GetIClientUnknown()->GetBaseEntity();
}

IMaterial *CCustomProxy::GetMaterial()
{
	return m_pBaseTextureVar->GetOwningMaterial();
}

bool CCustomProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool wasFound;
	m_pBaseTextureVar = pMaterial->FindVar( "$baseTexture", &wasFound, true );
	return wasFound;
}



// ====================================================
// Proxy used for changing the skin of a weapon based on its owner's faction
// ====================================================

class CWeaponSkinProxy : public CCustomProxy
{
	//DECLARE_CLASS( CWeaponSkinProxy, CCustomProxy );
public:
	CWeaponSkinProxy() {}
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );

private:
	IMaterialVar *m_pDefaultTextureVar, *m_pTexResistance, *m_pTexCombine, *m_pTexAperture;
	bool hasResistanceTexture, hasCombineTexture, hasApertureTexture;
};

EXPOSE_INTERFACE( CWeaponSkinProxy, IMaterialProxy, "WeaponSkin" IMATERIAL_PROXY_INTERFACE_VERSION );

bool CWeaponSkinProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool wasFound = CCustomProxy::Init(pMaterial, pKeyValues);
	m_pDefaultTextureVar = pMaterial->FindVar( "DefaultTexture", &wasFound, true );
	if ( !m_pDefaultTextureVar )
		return false;
	m_pDefaultTextureVar->SetTextureValue(m_pBaseTextureVar->GetTextureValue());
	
	m_pTexResistance = pMaterial->FindVar( "ResistanceTexture", &wasFound, false );
	if ( wasFound )
	{
		const char *szVal = m_pTexResistance->GetStringValue();
		ITexture *pTexResistance = materials->FindTexture( szVal, TEXTURE_GROUP_MODEL );
		m_pTexResistance->SetTextureValue(pTexResistance);
		hasResistanceTexture = /*m_pTexResistance &&*/ !pTexResistance->IsError();
	}
	else
		hasResistanceTexture = false;

	m_pTexCombine = pMaterial->FindVar( "CombineTexture", &wasFound, false );
	if ( wasFound )
	{
		const char *szVal = m_pTexCombine->GetStringValue();
		ITexture *pTexCombine = materials->FindTexture( szVal, TEXTURE_GROUP_MODEL );
		m_pTexCombine->SetTextureValue(pTexCombine);
		hasCombineTexture = /*m_pTexCombine &&*/ !pTexCombine->IsError();
	}
	else
		hasCombineTexture = false;

	m_pTexAperture = pMaterial->FindVar( "ApertureTexture", &wasFound, false );
	if ( wasFound )
	{
		const char *szVal = m_pTexAperture->GetStringValue();
		ITexture *pTexAperture = materials->FindTexture( szVal, TEXTURE_GROUP_MODEL );
		m_pTexAperture->SetTextureValue(pTexAperture);
		hasApertureTexture = /*m_pTexAperture &&*/ !pTexAperture->IsError();
	}
	else
		hasApertureTexture = false;

	return true;
}

void CWeaponSkinProxy::OnBind( void *pC_BaseEntity )
{
	if (!pC_BaseEntity)
		return;
	C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );

	if ( !pEntity )
	{
		//Msg("entity is null\n");
		return;
	}

	C_BaseViewModel *pVM = dynamic_cast<C_BaseViewModel*>( pEntity );
	C_BaseCombatWeapon *pWeapon = pWeapon = pVM ? pVM->GetOwningWeapon() : dynamic_cast<C_BaseCombatWeapon*>( pEntity );

	if ( !pWeapon )
	{
		//Msg("Can't find weapon\n");
		return;
	}

	//Msg(VarArgs("My entity: %s\n",pWeapon->GetClassname()));

	C_HL2MP_Player *pOwner = ToHL2MPPlayer( pWeapon->GetOwner() ); // this is null!

	IMaterialVar *pDesiredTexture = m_pDefaultTextureVar;
	if ( pOwner != NULL )
	{
		switch ( pOwner->GetFaction() )
		{
		case FACTION_RESISTANCE:
			if ( hasResistanceTexture )
				pDesiredTexture = m_pTexResistance;
			break;
		case FACTION_COMBINE:
			if ( hasCombineTexture )
				pDesiredTexture = m_pTexCombine;
			break;
		case FACTION_APERTURE:
			if ( hasApertureTexture )
				pDesiredTexture = m_pTexAperture;
			break;
		}
	}

	ITexture *pTexture = pDesiredTexture->GetTextureValue();
	if ( /*pDesiredTexture &&*/ m_pBaseTextureVar->GetTextureValue() != pTexture )
	{
		//Msg(VarArgs("Texture changed to %s\n",pTexture->GetName()));
		m_pBaseTextureVar->SetTextureValue(pTexture);
		GetMaterial()->RefreshPreservingMaterialVars();
	}

/*	if ( ToolsEnabled() )
	{
		ToolFramework_RecordMaterialParams( GetMaterial() );
	}*/
}



// ====================================================
// Proxy used for changing a material based on whether its owner is 'friendly' to the local player or not
// ====================================================

class CIFFProxy : public CCustomProxy
{
	//DECLARE_CLASS( CIFFProxy, CCustomProxy );
public:
	CIFFProxy() {}
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );

private:
	IMaterialVar *m_pTexFriendly, *m_pTexHostile;
};

EXPOSE_INTERFACE( CIFFProxy, IMaterialProxy, "IFF" IMATERIAL_PROXY_INTERFACE_VERSION );

bool CIFFProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool wasFound = CCustomProxy::Init(pMaterial, pKeyValues);
	m_pTexFriendly = pMaterial->FindVar( "FriendlyTexture", &wasFound, false );
	if ( wasFound )
	{
		const char *szVal = m_pTexFriendly->GetStringValue();
		ITexture *pTexFriendly = materials->FindTexture( szVal, TEXTURE_GROUP_MODEL );
		m_pTexFriendly->SetTextureValue(pTexFriendly);
	}

	m_pTexHostile = pMaterial->FindVar( "HostileTexture", &wasFound, false );
	if ( wasFound )
	{
		const char *szVal = m_pTexHostile->GetStringValue();
		ITexture *pTexHostile = materials->FindTexture( szVal, TEXTURE_GROUP_MODEL );
		m_pTexHostile->SetTextureValue(pTexHostile);
	}

	return true;
}

void CIFFProxy::OnBind( void *pC_BaseEntity )
{
	if (!pC_BaseEntity)
		return;
	C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );

	if ( !pEntity )
	{
		//Msg("entity is null\n");
		return;
	}

	C_BaseCombatCharacter *pBCC = dynamic_cast<C_BaseCombatCharacter*>( pEntity );
	
	if ( !pBCC )
		return;

	//Msg(VarArgs("My entity: %s\n",pBCC->GetClassname()));
	IMaterialVar *pDesiredTexture;
	if ( !HL2MPRules()->IsFriendly(C_BasePlayer::GetLocalPlayer(), pBCC) )
		pDesiredTexture = m_pTexHostile;
	else
		pDesiredTexture = m_pTexFriendly;

	ITexture *pTexture = pDesiredTexture->GetTextureValue();
	if ( m_pBaseTextureVar->GetTextureValue() != pTexture )
	{
		m_pBaseTextureVar->SetTextureValue(pTexture);
		GetMaterial()->RefreshPreservingMaterialVars();
	}

/*	if ( ToolsEnabled() )
	{
		ToolFramework_RecordMaterialParams( GetMaterial() );
	}*/
}

/*


// ====================================================
// Proxy used for changing the skin skin of an NPC or ragdoll if it has an "override" set for this material
// ====================================================

class CSkinOverrideProxy : public CCustomProxy
{
	//DECLARE_CLASS( CSkinOverrideProxy, CCustomProxy );
public:
	CSkinOverrideProxy() { overrideMaterialNumber = 0; }
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );

private:
	IMaterialVar *m_pDefaultTextureVar;
	int overrideMaterialNumber;
};

EXPOSE_INTERFACE( CSkinOverrideProxy, IMaterialProxy, "SkinOverride" IMATERIAL_PROXY_INTERFACE_VERSION );

bool CSkinOverrideProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool wasFound = CCustomProxy::Init(pMaterial, pKeyValues);
	
	IMaterialVar *m_pDefaultTextureVar = pMaterial->FindVar( "SkinMatNum", &wasFound, false );
	if ( wasFound )
	{
		overrideMaterialNumber = m_pDefaultTextureVar->GetIntValue();
		m_pDefaultTextureVar->SetTextureValue(m_pBaseTextureVar->GetTextureValue());
	}
	else
		overrideMaterialNumber = 0;

	return true;
}

void CSkinOverrideProxy::OnBind( void *pC_BaseEntity )
{
	if (!pC_BaseEntity)
		return;
	C_BaseEntity *pEntity = BindArgToEntity( pC_BaseEntity );

	ITexture *pTexture;
	if ( !pEntity )
		pTexture = m_pDefaultTextureVar->GetTextureValue();
	else if ( pEntity->IsNPC() )
	{
		C_AI_BaseNPC *pNPC = pEntity->MyNPCPointer();
		if ( pNPC->ShouldOverrideSkin(overrideMaterialNumber) )
			pTexture = pNPC->GetOverrideSkinTexture(overrideMaterialNumber);
		else
			pTexture = m_pDefaultTextureVar->GetTextureValue();
	}
	else if ( pEntity->IsRagdoll() )
	{
		C_ClientRagdoll *pRagdoll = pEntity->MyRagdollPointer();
		if ( pRagdoll->ShouldOverrideSkin(overrideMaterialNumber) )
			pTexture = pRagdoll->GetOverrideSkinTexture(overrideMaterialNumber);
		else
			pTexture = m_pDefaultTextureVar->GetTextureValue();
	}
	else
		pTexture = m_pDefaultTextureVar->GetTextureValue();

	if ( m_pBaseTextureVar->GetTextureValue() != pTexture )
	{
		m_pBaseTextureVar->SetTextureValue(pTexture);
		GetMaterial()->RefreshPreservingMaterialVars();
	}

//	if ( ToolsEnabled() )
//	{
//		ToolFramework_RecordMaterialParams( GetMaterial() );
//	}
}
*/
