//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "text_message.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"
#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialvar.h"
#include "IEffects.h"
#include "hudelement.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_playerresource.h"
#include "hl2mp/c_hl2mp_player.h"
#include "hl2mp/hl2mp_gamerules.h"
#include "modcom/mc_shareddefs.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MIN_OPACITY 50
#define MAX_OPACITY 255
#define ICON_SIZE 32

ConVar cl_allyicons_max_dist("cl_allyicons_max_dist", "2000", FCVAR_ARCHIVE, "The distance at which hud ally indicators reach maximum transparency");
ConVar cl_allyicons_show("cl_allyicons_show", "1",FCVAR_ARCHIVE, "Controls whether icons will be drawn to show where allies are located in team games.\n0 will never draw, 1 will draw always, 2 will draw only when they are occluded.", true, 0, true, 2);
ConVar cl_allyicons_offset("cl_allyicons_offset", "16", FCVAR_ARCHIVE, "The vertical offset (from the centre) used when drawing ally indicator icons", true, -48, true, 48);

//-----------------------------------------------------------------------------
// Purpose: HDU Damage indication
//-----------------------------------------------------------------------------
class CHudDamageIndicator : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudDamageIndicator, vgui::Panel );

public:
	CHudDamageIndicator( const char *pElementName );
	void Init( void );
	void Reset( void );
	virtual bool ShouldDraw( void );

	// Handler for our message
	void MsgFunc_Damage( bf_read &msg );

private:
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	CPanelAnimationVarAliasType( float, m_flDmgX, "dmg_xpos", "10", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flDmgY, "dmg_ypos", "80", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flDmgWide, "dmg_wide", "30", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flDmgTall1, "dmg_tall1", "300", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flDmgTall2, "dmg_tall2", "240", "proportional_float" );

	CPanelAnimationVar( Color, m_DmgColorLeft, "DmgColorLeft", "255 0 0 0" );
	CPanelAnimationVar( Color, m_DmgColorRight, "DmgColorRight", "255 0 0 0" );

	CPanelAnimationVar( Color, m_DmgHighColorLeft, "DmgHighColorLeft", "255 0 0 0" );
	CPanelAnimationVar( Color, m_DmgHighColorRight, "DmgHighColorRight", "255 0 0 0" );

	CPanelAnimationVar( Color, m_DmgFullscreenColor, "DmgFullscreenColor", "255 0 0 0" );

	void DrawDamageIndicator(int side);
	void DrawFullscreenDamageIndicator();
	void GetDamagePosition( const Vector &vecDelta, float *flRotation );

	CMaterialReference m_WhiteAdditiveMaterial;

	int m_iCombineIcon, m_iResistanceIcon, m_iApertureIcon;
};

DECLARE_HUDELEMENT( CHudDamageIndicator );
DECLARE_HUD_MESSAGE( CHudDamageIndicator, Damage );

enum
{
	DAMAGE_ANY,
	DAMAGE_LOW,
	DAMAGE_HIGH,
};

#define ANGLE_ANY	0.0f
#define DMG_ANY		0

struct DamageAnimation_t
{
	const char *name;
	int bitsDamage;
	float angleMinimum;
	float angleMaximum;
	int damage; 
};

//-----------------------------------------------------------------------------
// Purpose: List of damage animations, finds first that matches criteria
//-----------------------------------------------------------------------------
static DamageAnimation_t g_DamageAnimations[] =
{
	{ "HudTakeDamageDrown",		DMG_DROWN,	ANGLE_ANY,	ANGLE_ANY,	DAMAGE_ANY },
	{ "HudTakeDamagePoison",	DMG_POISON,	ANGLE_ANY,	ANGLE_ANY,	DAMAGE_ANY },
	{ "HudTakeDamageBurn",		DMG_BURN,	ANGLE_ANY,	ANGLE_ANY,	DAMAGE_ANY },
	{ "HudTakeDamageRadiation",	DMG_RADIATION,	ANGLE_ANY,	ANGLE_ANY,	DAMAGE_ANY },
	{ "HudTakeDamageRadiation",	DMG_ACID,	ANGLE_ANY,	ANGLE_ANY,	DAMAGE_ANY },

	{ "HudTakeDamageHighLeft",	DMG_ANY,	45.0f,		135.0f,		DAMAGE_HIGH },
	{ "HudTakeDamageHighRight",	DMG_ANY,	225.0f,		315.0f,		DAMAGE_HIGH },
	{ "HudTakeDamageHigh",		DMG_ANY,	ANGLE_ANY,	ANGLE_ANY,	DAMAGE_HIGH },
	
	{ "HudTakeDamageLeft",		DMG_ANY,	45.0f,		135.0f,		DAMAGE_ANY },
	{ "HudTakeDamageRight",		DMG_ANY,	225.0f,		315.0f,		DAMAGE_ANY },
	{ "HudTakeDamageBehind",	DMG_ANY,	135.0f,		225.0f,		DAMAGE_ANY },

	// fall through to front damage
	{ "HudTakeDamageFront",		DMG_ANY,	ANGLE_ANY,	ANGLE_ANY,	DAMAGE_ANY },
	{ NULL },
};


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudDamageIndicator::CHudDamageIndicator( const char *pElementName ) : CHudElement( pElementName ), BaseClass(NULL, "HudDamageIndicator")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_WhiteAdditiveMaterial.Init( "vgui/white_additive", TEXTURE_GROUP_VGUI ); 
	
	SetHiddenBits( HIDEHUD_HEALTH );

	m_iCombineIcon = surface()->DrawGetTextureId( "vgui/gfx/ally_combine" );
	if ( m_iCombineIcon == -1 ) // we didn't find it, so create a new one
		m_iCombineIcon = surface()->CreateNewTextureID(); 
	surface()->DrawSetTextureFile( m_iCombineIcon, "vgui/gfx/ally_combine", true, false );

	m_iResistanceIcon = surface()->DrawGetTextureId( "vgui/gfx/ally_resistance" );
	if ( m_iResistanceIcon == -1 ) // we didn't find it, so create a new one
		m_iResistanceIcon = surface()->CreateNewTextureID(); 
	surface()->DrawSetTextureFile( m_iResistanceIcon, "vgui/gfx/ally_resistance", true, false );

	m_iApertureIcon = surface()->DrawGetTextureId( "vgui/gfx/ally_aperture" );
	if ( m_iApertureIcon == -1 ) // we didn't find it, so create a new one
		m_iApertureIcon = surface()->CreateNewTextureID(); 
	surface()->DrawSetTextureFile( m_iApertureIcon, "vgui/gfx/ally_aperture", true, false );

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDamageIndicator::Reset( void )
{
	m_DmgColorLeft[3] = 0;
	m_DmgColorRight[3] = 0;
	m_DmgHighColorLeft[3] = 0;
	m_DmgHighColorRight[3] = 0;
	m_DmgFullscreenColor[3] = 0;
}

void CHudDamageIndicator::Init( void )
{
	HOOK_HUD_MESSAGE( CHudDamageIndicator, Damage );
}

extern int ScreenTransform( const Vector& point, Vector& screen );
bool GetScreenPos(Vector testPos,int &x, int &y)
{
	Vector screenPos;
	bool retVal = ScreenTransform( testPos, screenPos ) == 0;
	int sw = ScreenWidth(), sh = ScreenHeight();

	x = 0.5 * screenPos[0] * sw + 0.5 * sw;
	y = -0.5 * screenPos[1] * sh + 0.5 * sh;

	return retVal;
}

extern ConVar mc_gamemode;

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudDamageIndicator::ShouldDraw( void )
{
/*
	bool bNeedsDraw = m_DmgColorLeft[3] || 
						m_DmgColorRight[3] || 
						m_DmgHighColorLeft[3] || 
						m_DmgHighColorRight[3] ||
						m_DmgFullscreenColor[3];

	if ( HL2MPRules()->IsTeamplay() && cl_allyicons_show.GetInt() > 0 )
	{
		// update the screen positions of all allied players, mark everyone else that we shouldn't draw them
		C_HL2MP_Player *pLocal = C_HL2MP_Player::GetLocalHL2MPPlayer();
		if ( pLocal )
			for ( int i=0; i<MAX_PLAYERS; i++ )
			{
				if ( (i+1) != pLocal->entindex() && g_PR->IsAlive(i+1) && g_PR->IsInCharacter(i+1) && g_PR->GetFaction(i+1) == pLocal->GetFaction() )
				{
					int x, y;
					Vector pos = g_PR->GetPos(i+1) + Vector(0,0,cl_allyicons_offset.GetFloat());
					GetScreenPos(pos, x, y);
					if ( x > -30 && x < ScreenWidth()+30 && y > -30 && y < ScreenHeight()+30 ) // is roughly on-screen, so draw them
					{
						bool okToDraw = true;
						if ( cl_allyicons_show.GetInt() == 2 ) // only if occluded
						{// they're in the screen. do a traceline to see if we can see them already or not
							trace_t tr;
							UTIL_TraceLine(pLocal->EyePosition(),pos,MASK_VISIBLE,pLocal,COLLISION_GROUP_NONE,&tr);
							okToDraw = tr.fraction < 1.0f && (tr.m_pEnt == NULL || tr.m_pEnt != pPlayer);
						}
						if ( okToDraw )
						{
							m_AllyInfo[i].iDrawX = x; m_AllyInfo[i].iDrawY = y;
							m_AllyInfo[i].bShouldDraw = true;
							// also work out opacity based on distance...
							float dist = (pos - pLocal->EyePosition()).Length();
							m_AllyInfo[i].iOpacity = (1.0f - dist / cl_allyicons_max_dist.GetFloat()) * (MAX_OPACITY - MIN_OPACITY) + MIN_OPACITY;
							bNeedsDraw = true;
						}
						else
							m_AllyInfo[i].bShouldDraw = false; // already visible, don't draw icon
					}
					else
						m_AllyInfo[i].bShouldDraw = false; // not facing them, so don't try and draw icon
				}
				else
					m_AllyInfo[i].bShouldDraw = false; // don't draw this player, they're not an ally or don't exist
			}
	}
	else
		for ( int i=0; i<MAX_PLAYERS; i++ )
			m_AllyInfo[i].bShouldDraw = false;
*/
	return /*( bNeedsDraw &&*/( CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: Draws a damage quad
//-----------------------------------------------------------------------------
void CHudDamageIndicator::DrawDamageIndicator(int side)
{
	CMatRenderContextPtr pRenderContext( materials );
	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_WhiteAdditiveMaterial );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	int insetY = (m_flDmgTall1 - m_flDmgTall2) / 2;

	int x1 = m_flDmgX;
	int x2 = m_flDmgX + m_flDmgWide;
	int y[4] = { m_flDmgY, m_flDmgY + insetY, m_flDmgY + m_flDmgTall1 - insetY, m_flDmgY + m_flDmgTall1 };
	int alpha[4] = { 0.0f, 1.0f, 1.0f, 0.0f };

	// see if we're high damage
	bool bHighDamage = false;
	if ( m_DmgHighColorRight[3] > m_DmgColorRight[3] || m_DmgHighColorLeft[3] > m_DmgColorLeft[3] )
	{
		// make more of the screen be covered by damage
		x1 = GetWide() * 0.0f;
		x2 = GetWide() * 0.5f;
		y[0] = 0.0f;
		y[1] = 0.0f;
		y[2] = GetTall();
		y[3] = GetTall();
		alpha[0] = 1.0f;
		alpha[1] = 0.0f;
		alpha[2] = 0.0f;
		alpha[3] = 1.0f;
		bHighDamage = true;
	}

	int r, g, b, a;
	if (side == 1)
	{
		if ( bHighDamage )
		{
			r = m_DmgHighColorRight[0], g = m_DmgHighColorRight[1], b = m_DmgHighColorRight[2], a = m_DmgHighColorRight[3];
		}
		else
		{
			r = m_DmgColorRight[0], g = m_DmgColorRight[1], b = m_DmgColorRight[2], a = m_DmgColorRight[3];
		}

		// realign x coords
		x1 = GetWide() - x1;
		x2 = GetWide() - x2;

		meshBuilder.Color4ub( r, g, b, a * alpha[0]);
		meshBuilder.TexCoord2f( 0,0,0 );
		meshBuilder.Position3f( x1, y[0], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[3] );
		meshBuilder.TexCoord2f( 0,0,1 );
		meshBuilder.Position3f( x1, y[3], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[2] );
		meshBuilder.TexCoord2f( 0,1,1 );
		meshBuilder.Position3f( x2, y[2], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[1] );
		meshBuilder.TexCoord2f( 0,1,0 );
		meshBuilder.Position3f( x2, y[1], 0 );
		meshBuilder.AdvanceVertex();
	}
	else
	{
		if ( bHighDamage )
		{
			r = m_DmgHighColorLeft[0], g = m_DmgHighColorLeft[1], b = m_DmgHighColorLeft[2], a = m_DmgHighColorLeft[3];
		}
		else
		{
			r = m_DmgColorLeft[0], g = m_DmgColorLeft[1], b = m_DmgColorLeft[2], a = m_DmgColorLeft[3];
		}

		meshBuilder.Color4ub( r, g, b, a * alpha[0] );
		meshBuilder.TexCoord2f( 0,0,0 );
		meshBuilder.Position3f( x1, y[0], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[1] );
		meshBuilder.TexCoord2f( 0,1,0 );
		meshBuilder.Position3f( x2, y[1], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[2] );
		meshBuilder.TexCoord2f( 0,1,1 );
		meshBuilder.Position3f( x2, y[2], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[3] );
		meshBuilder.TexCoord2f( 0,0,1 );
		meshBuilder.Position3f( x1, y[3], 0 );
		meshBuilder.AdvanceVertex();
	}

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose: Draws full screen damage fade
//-----------------------------------------------------------------------------
void CHudDamageIndicator::DrawFullscreenDamageIndicator()
{
	CMatRenderContextPtr pRenderContext( materials );
	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_WhiteAdditiveMaterial );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );
	int r = m_DmgFullscreenColor[0], g = m_DmgFullscreenColor[1], b = m_DmgFullscreenColor[2], a = m_DmgFullscreenColor[3];

	float wide = GetWide(), tall = GetTall();

	meshBuilder.Color4ub( r, g, b, a );
	meshBuilder.TexCoord2f( 0,0,0 );
	meshBuilder.Position3f( 0.0f, 0.0f, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub( r, g, b, a );
	meshBuilder.TexCoord2f( 0,1,0 );
	meshBuilder.Position3f( wide, 0.0f, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub( r, g, b, a );
	meshBuilder.TexCoord2f( 0,1,1 );
	meshBuilder.Position3f( wide, tall, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub( r, g, b, a );
	meshBuilder.TexCoord2f( 0,0,1 );
	meshBuilder.Position3f( 0.0f, tall, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose: Paints the damage display
//-----------------------------------------------------------------------------
void CHudDamageIndicator::Paint()
{
	// draw fullscreen damage indicators
	DrawFullscreenDamageIndicator();

	// draw side damage indicators
	DrawDamageIndicator(0);
	DrawDamageIndicator(1);

	// now handle ally tracking hud elements
	if ( HL2MPRules()->IsTeamplay() && cl_allyicons_show.GetInt() > 0 )
	{
		// update the screen positions of all allied players, mark everyone else that we shouldn't draw them
		C_HL2MP_Player *pLocal = C_HL2MP_Player::GetLocalHL2MPPlayer();
		if ( pLocal )
			for ( int i=0; i<MAX_PLAYERS; i++ )
			{
				if ( (i+1) != pLocal->entindex() && g_PR->IsAlive(i+1) && g_PR->IsInCharacter(i+1) && HL2MPRules()->IsFriendly(UTIL_PlayerByIndex( i+1 ), pLocal) )
				{
					int x, y;
					Vector pos = g_PR->GetPos(i+1) + Vector(0,0,cl_allyicons_offset.GetFloat());
					GetScreenPos(pos, x, y);
					if ( x > -30 && x < ScreenWidth()+30 && y > -30 && y < ScreenHeight()+30 ) // is roughly on-screen, so draw them
					{
						bool okToDraw = true;
						if ( cl_allyicons_show.GetInt() == 2 ) // only if occluded
						{// they're in the screen. do a traceline to see if we can see them already or not
							trace_t tr;
							UTIL_TraceLine(pLocal->EyePosition(),pos,MASK_VISIBLE,pLocal,COLLISION_GROUP_NONE,&tr);
							okToDraw = tr.fraction < 1.0f && (tr.m_pEnt == NULL || tr.m_pEnt != UTIL_PlayerByIndex(i+1));
						}
						if ( okToDraw )
						{
							int iDrawX = x; int iDrawY = y;
							// also work out opacity based on distance...
							float dist = (pos - pLocal->EyePosition()).Length();
							int iOpacity = (1.0f - dist / cl_allyicons_max_dist.GetFloat()) * (MAX_OPACITY - MIN_OPACITY) + MIN_OPACITY;
							
							switch ( g_PR->GetFaction(i+1) )
							{
							case FACTION_COMBINE:
								 surface()->DrawSetTexture(m_iCombineIcon); break;
							case FACTION_RESISTANCE:
								 surface()->DrawSetTexture(m_iResistanceIcon); break;
							case FACTION_APERTURE:
								 surface()->DrawSetTexture(m_iApertureIcon); break;
							}

							surface()->DrawSetColor(255,255,255,iOpacity);
							surface()->DrawTexturedRect(iDrawX-ICON_SIZE/2,iDrawY-ICON_SIZE/2,iDrawX+ICON_SIZE/2,iDrawY+ICON_SIZE/2);
						}
					}
				}
			}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Message handler for Damage message
//-----------------------------------------------------------------------------
void CHudDamageIndicator::MsgFunc_Damage( bf_read &msg )
{
	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits

	Vector vecFrom;

	vecFrom.x = msg.ReadFloat();
	vecFrom.y = msg.ReadFloat();
	vecFrom.z = msg.ReadFloat();

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	// player has just died, just run the dead damage animation
	if ( pPlayer->GetHealth() <= 0 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HudPlayerDeath" );
		return;
	}

	// ignore damage without direction
	// this should never happen, unless it's drowning damage, 
	// or the player is forcibly killed, handled above
	if ( vecFrom == vec3_origin && !(bitsDamage & DMG_DROWN))
		return;

	Vector vecDelta = (vecFrom - MainViewOrigin());
	VectorNormalize( vecDelta );

	int highDamage = DAMAGE_LOW;
	if ( damageTaken > 25 )
	{
		highDamage = DAMAGE_HIGH;
	}

	// if we have no suit, all damage is high
	if ( !pPlayer->IsSuitEquipped() )
	{
		highDamage = DAMAGE_HIGH;
	}

	if ( damageTaken > 0 || armor > 0 )
	{
		// see which quandrant the effect is in
		float angle;
		GetDamagePosition( vecDelta, &angle );

		// see which effect to play
		DamageAnimation_t *dmgAnim = g_DamageAnimations;
		for ( ; dmgAnim->name != NULL; ++dmgAnim )
		{
			if ( dmgAnim->bitsDamage && !(bitsDamage & dmgAnim->bitsDamage) )
				continue;

			if ( dmgAnim->angleMinimum && angle < dmgAnim->angleMinimum )
				continue;

			if ( dmgAnim->angleMaximum && angle > dmgAnim->angleMaximum )
				continue;

			if ( dmgAnim->damage && dmgAnim->damage != highDamage )
				continue;

			// we have a match, break
			break;
		}

		if ( dmgAnim->name )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( dmgAnim->name );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Convert a damage position in world units to the screen's units
//-----------------------------------------------------------------------------
void CHudDamageIndicator::GetDamagePosition( const Vector &vecDelta, float *flRotation )
{
	float flRadius = 360.0f;

	// Player Data
	Vector playerPosition = MainViewOrigin();
	QAngle playerAngles = MainViewAngles();

	Vector forward, right, up(0,0,1);
	AngleVectors (playerAngles, &forward, NULL, NULL );
	forward.z = 0;
	VectorNormalize(forward);
	CrossProduct( up, forward, right );
	float front = DotProduct(vecDelta, forward);
	float side = DotProduct(vecDelta, right);
	float xpos = flRadius * -side;
	float ypos = flRadius * -front;

	// Get the rotation (yaw)
	*flRotation = atan2(xpos, ypos) + M_PI;
	*flRotation *= 180 / M_PI;

	float yawRadians = -(*flRotation) * M_PI / 180.0f;
	float ca = cos( yawRadians );
	float sa = sin( yawRadians );
				 
	// Rotate it around the circle
	xpos = (int)((ScreenWidth() / 2) + (flRadius * sa));
	ypos = (int)((ScreenHeight() / 2) - (flRadius * ca));
}

//-----------------------------------------------------------------------------
// Purpose: hud scheme settings
//-----------------------------------------------------------------------------
void CHudDamageIndicator::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);

	int wide, tall;
	GetHudSize(wide, tall);
	SetSize(wide, tall);
}
