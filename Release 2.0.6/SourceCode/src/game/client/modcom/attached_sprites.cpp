#include "cbase.h"
#include "attached_sprites.h"
#include "voice_status.h"
#include "r_efx.h"
#include "cliententitylist.h"
#include "c_baseplayer.h"
#include "materialsystem/imesh.h"
#include "view.h"
#include "convar.h"
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include "vgui_bitmapimage.h"
#include "materialsystem/imaterial.h"
#include "cdll_int.h"
#include "hl2mp/c_hl2mp_player.h"
#include "hl2mp/hl2mp_gamerules.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CAttachedSpriteManager *g_AttachedSprites = NULL;

CAttachedSpriteManager* GetAttachedSpriteManager()
{
	if ( !g_AttachedSprites )
	{
		AttachedSpriteManager_Init();
	}

	return g_AttachedSprites;
}

void AttachedSpriteManager_Init()
{
	if ( g_AttachedSprites )
		return;

	g_AttachedSprites = new CAttachedSpriteManager();
}

void AttachedSpriteManager_Shutdown()
{
	delete g_AttachedSprites;
	g_AttachedSprites = NULL;
}

#define ICON_VISIBILITY_THRESHOLD 50.0f

ConVar cl_icon_voice_size("cl_icon_voice_size", "16", 0, "Size of the icon shown above speaking players");
ConVar cl_icon_voice_height("cl_icon_voice_height", "22", 0, "Height above the head of the icon shown above speaking players"); // formerly 35

ConVar cl_icon_tokens_size("cl_icon_tokens_size", "22", 0, "Size of the icon shown above players with score tokens in the Hoarder game mode");
ConVar cl_icon_tokens_height("cl_icon_tokens_height", "46", 0, "Height above the head of the icon shown above players with score tokens in the Hoarder game mode");

CAttachedSpriteManager::CAttachedSpriteManager()
{
	m_pVoiceLabelMaterial = NULL;
	m_bHeadLabelsDisabled = false;
	
	for ( int i=0; i<MAX_SCORE_TOKEN_INDICATOR; i++ )
		m_pScoreTokenLabelMaterials[i] = NULL;
}

CAttachedSpriteManager::~CAttachedSpriteManager()
{
	if ( m_pVoiceLabelMaterial )
		m_pVoiceLabelMaterial->DecrementReferenceCount();

	for ( int i=0; i<MAX_SCORE_TOKEN_INDICATOR; i++ )
		if ( m_pScoreTokenLabelMaterials[i] )
			m_pScoreTokenLabelMaterials[i]->DecrementReferenceCount();
}

int CAttachedSpriteManager::Init()
{
	m_pVoiceLabelMaterial = materials->FindMaterial( "voice/icntlk_pl", TEXTURE_GROUP_VGUI );
	m_pVoiceLabelMaterial->IncrementReferenceCount();

	for ( int i=0; i<MAX_SCORE_TOKEN_INDICATOR; i++ )
	{
		m_pScoreTokenLabelMaterials[i] = materials->FindMaterial( VarArgs("vgui/gfx/scoretokens%i", i+1), TEXTURE_GROUP_VGUI );
		m_pScoreTokenLabelMaterials[i]->IncrementReferenceCount();
	}
		
	return 1;
}

void CAttachedSpriteManager::VidInit()
{
}

void CAttachedSpriteManager::DrawHeadLabels()
{
	if ( m_bHeadLabelsDisabled )
		return;

	C_HL2MP_Player *pLocal = C_HL2MP_Player::GetLocalHL2MPPlayer();
	CVoiceStatus *voiceManager = GetClientVoiceMgr();

	bool indicateScoreTokens = HL2MPRules() && HL2MPRules()->ShouldUseScoreTokens();	
	for(int i=0; i<MAX_PLAYERS; i++)
	{
		IClientNetworkable *pClient = cl_entitylist->GetClientEntity( i+1 );
		
		// Don't show an icon if the player is not in our PVS.
		if ( !pClient || pClient->IsDormant() )
			continue;

		C_BasePlayer *pPlayer = dynamic_cast<C_BasePlayer*>(pClient);
		if( !pPlayer || pPlayer == pLocal ) // don't draw icons on local player, ever... ?
			continue;

		C_HL2MP_Player *pHL2MP = ToHL2MPPlayer(pPlayer);
		if ( pPlayer->IsPlayerDead() || !pHL2MP->IsInCharacter() )
			continue; // draw no icons for players who aren't alive and in a character
		
		bool visible = pPlayer->GetRenderColor().a >= ICON_VISIBILITY_THRESHOLD;
		bool isAlly =  HL2MPRules() && HL2MPRules()->IsTeamplay() && pHL2MP->GetFaction() == pLocal->GetFaction();
		
		// handle voice indicator icons
		if ( voiceManager->IsPlayerSpeaking(i) && (visible || isAlly) && m_pVoiceLabelMaterial != NULL )
			DrawIcon(pPlayer, m_pVoiceLabelMaterial, cl_icon_voice_height.GetFloat(), cl_icon_voice_size.GetFloat());

		// handle score token indicator icons
		if (indicateScoreTokens && (visible || isAlly))
			ShowScoreTokens(pHL2MP, isAlly);
	}

	// now loop over anything that might be an NPC to show their score tokens also
	int highest = cl_entitylist->GetHighestEntityIndex();
	for ( int i=MAX_PLAYERS; i<highest; i++ )
	{
		IClientNetworkable *pClient = cl_entitylist->GetClientEntity( i+1 );
		
		// Don't show an icon if the entity is not in our PVS.
		if ( !pClient || pClient->IsDormant() )
			continue;

		C_AI_BaseNPC *pNPC = dynamic_cast<C_AI_BaseNPC*>(pClient);
		if( !pNPC || !pNPC->IsAlive() )
			continue;

		bool visible = pNPC->GetRenderColor().a >= ICON_VISIBILITY_THRESHOLD;
		bool isAlly =  HL2MPRules() && HL2MPRules()->IsTeamplay() && HL2MPRules()->IsFriendly(pLocal, pNPC);
		if (indicateScoreTokens && (visible || isAlly))
			ShowScoreTokens(pNPC, isAlly);
	}
}

void CAttachedSpriteManager::DrawIcon(C_BaseCombatCharacter *pTarget, IMaterial *pMaterial, float verticalOffsetFromEyes, float iconSize, double r, double g, double b)
{
	// Place it 20 units above his head.
	Vector vOrigin = pTarget->WorldSpaceCenter();
	vOrigin.z += verticalOffsetFromEyes;

	// Align it so it never points up or down.
	Vector vUp( 0, 0, 1 );
	Vector vRight = CurrentViewRight();
	if ( fabs( vRight.z ) > 0.95 )	// don't draw it edge-on
		return;

	vRight.z = 0;
	VectorNormalize( vRight );
	
	iconSize *= 0.5f;

	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( pMaterial );
	IMesh *pMesh = pRenderContext->GetDynamicMesh();
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	meshBuilder.Color3f( r, g, b );
	meshBuilder.TexCoord2f( 0,0,0 );
	meshBuilder.Position3fv( (vOrigin + (vRight * -iconSize) + (vUp * iconSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3f( r, g, b );
	meshBuilder.TexCoord2f( 0,1,0 );
	meshBuilder.Position3fv( (vOrigin + (vRight * iconSize) + (vUp * iconSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3f( r, g, b );
	meshBuilder.TexCoord2f( 0,1,1 );
	meshBuilder.Position3fv( (vOrigin + (vRight * iconSize) + (vUp * -iconSize)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3f( r, g, b );
	meshBuilder.TexCoord2f( 0,0,1 );
	meshBuilder.Position3fv( (vOrigin + (vRight * -iconSize) + (vUp * -iconSize)).Base() );
	meshBuilder.AdvanceVertex();
	meshBuilder.End();
	pMesh->Draw();
}

void CAttachedSpriteManager::ShowScoreTokens(C_BaseCombatCharacter *pBCC, bool isAlly)
{
	int numTokens = pBCC->GetNumScoreTokens();
	if ( numTokens > 0 )
	{
		numTokens = min(numTokens, MAX_SCORE_TOKEN_INDICATOR) - 1;
		if ( !m_pScoreTokenLabelMaterials[numTokens] )
			return;
		
		double r, g, b;
		if ( isAlly )
			r = g = b = 0.5;
		else
		{
			r = 1.0; g = 0.6275; b = 0.0; // same as 255, 160, 0
		}
		DrawIcon(pBCC, m_pScoreTokenLabelMaterials[numTokens], cl_icon_tokens_height.GetFloat(), cl_icon_tokens_size.GetFloat(), r, g, b);
	}
}