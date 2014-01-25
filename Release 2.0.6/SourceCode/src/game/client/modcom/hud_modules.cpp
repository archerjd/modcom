//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "modcom/hud_modules.h"
#include "hud_macros.h"
#include "hl2mp/c_hl2mp_player.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CHudModule );

// instance ... we need to be able to get this panel from anywhere to tell it to reset abilities 
CHudModule *CHudModule::instance = NULL;
CHudModule *GetModuleHud()
{
	return CHudModule::instance;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudModule::CHudModule( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudModule" )
{
	instance = this;

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

void CHudModule::VidInit( void )
{
	for ( int i=0; i<NUM_MODULES; i++ )
	{
		icons[i] = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile( icons[i], GetModule(i)->GetFullIconName(), true, false );
	}

	//m_iWipeMaterial = surface()->CreateNewTextureID();
	//surface()->DrawSetTextureFile( m_iWipeMaterial, "vgui/gfx/buff_wipe", true, false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudModule::Init( void )
{
	m_iSelectedModule = NO_MODULE;
	//m_iWipeMaterial = -1;

	m_bLowPower = m_bToggledOn = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudModule::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudModule::ShouldDraw()
{
	bool bNeedsDraw = false;

	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return false;

	if ( m_iSelectedModule == NO_MODULE )
	{// we dont have anything selected, so see if the player has any relevant abilities
		for ( int i=0; i<NUM_MODULES; i++ )
		{
			Module *a = GetModule(i);
			if ( !a->AutoStarts() && pPlayer->HasModule(a) )
			{
				m_iSelectedModule = i; // select this, then
				UpdateIconColor();
				bNeedsDraw = true;
				break;
			}
		}
	}
	else
		bNeedsDraw = true; // we have a module selected, so we must draw
	
	return ( bNeedsDraw && CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudModule::OnThink( void )
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer || m_iSelectedModule == NO_MODULE )
		return;
	Module *a = GetModule(m_iSelectedModule);

	bool bActuallyLowPower = ( pPlayer->GetAuxPower() < a->GetAuxDrain(pPlayer,pPlayer->GetModuleLevel(a)) );
	if ( bActuallyLowPower != m_bLowPower || pPlayer->IsModuleActive(a) != m_bToggledOn )
	{
		m_bLowPower = bActuallyLowPower;
		UpdateIconColor();
	}
}

float FitVarToRange(float minRange, float maxRange, float minVar, float maxVar, float position)
{
	float fraction = ( position - minRange ) / ( maxRange - minRange );
	return minVar + ( maxVar - minVar ) * fraction;
}

//-----------------------------------------------------------------------------
// Purpose: draws the power bar
//-----------------------------------------------------------------------------
void CHudModule::Paint()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer || m_iSelectedModule == NO_MODULE )
		return;

	Module *a = GetModule(m_iSelectedModule);

	Color drawClr = m_IconColor;//pPlayer->GetPower() >= a->GetAuxDrain() ? gHUD.m_clrNormal : gHUD.m_clrCaution;

/*	surface()->DrawSetTextFont( m_hFont );
	surface()->DrawSetTextColor( drawClr );
	surface()->DrawSetTextPos( text_xpos, text_ypos );
	surface()->DrawUnicodeString( a->GetDisplayNameUnicode() );
*/
	surface()->DrawSetTexture(icons[m_iSelectedModule]);
	surface()->DrawSetColor(drawClr);
	surface()->DrawTexturedRect(icon_xpos,icon_ypos,icon_xpos+icon_size,icon_ypos+icon_size);

	if ( a->IsToggled() && (pPlayer->IsModuleActive(a) || (a->Ticks() && a->GetCooldown(pPlayer->GetModuleLevel(a)) == 0)) )
		return; // don't show cooldown for toggles (eg cloak)

	float fraction = CalcCooldownFraction(a);
	if ( fraction < 1.0f )
	{
		surface()->DrawSetColor(255,255,255,160);
		float line1x = FitVarToRange(0.0f,1.0f,icon_xpos,icon_xpos+icon_size/2,fraction);
		float line2x = FitVarToRange(0.0f,1.0f,icon_xpos+icon_size,icon_xpos+icon_size/2,fraction);
		surface()->DrawLine(line1x,icon_ypos,line1x,icon_ypos+icon_size);
		surface()->DrawLine(line2x,icon_ypos,line2x,icon_ypos+icon_size);
		//DrawRadialWipe(icon_xpos, icon_ypos, icon_xpos+icon_size, icon_ypos+icon_size, fraction, m_iWipeMaterial);
	}
}

#define WIPE_FILL_ALPHA 172
#define WIPE_LINE_ALPHA 192

#define PI 3.14159265f
#define TWO_PI PI*2.0f
#define THREE_PI_BY_TWO 3.0f*PI/2.0f
#define PI_BY_TWO PI/2.0f

#define ANGLE_FIRST_CORNER PI/4.0f
#define ANGLE_SECOND_CORNER 3.0f*PI/4.0f
#define ANGLE_THIRD_CORNER 5.0f*PI/4.0f
#define ANGLE_FOURTH_CORNER 7.0f*PI/4.0f

void CHudModule::DrawRadialWipe(int x0, int y0, int x1, int y1, float fraction, int iMaterialRef)
{
	int cx = (x0 + x1)/2, cy = (y0 + y1)/2;
	float filledAngle = TWO_PI * (1 - fraction); // the "unfilled" angle goes from 0 to 2 pi, so the filled angle goes from 2 pi to 0
	float smallAngle;
	int quartile;
	
	if ( filledAngle <= PI_BY_TWO ) // all
	{
		smallAngle = filledAngle; quartile = 1;
	}
	else if ( filledAngle <= PI ) // sanderson
	{
		smallAngle = filledAngle - PI_BY_TWO; quartile = 2;
	}
	else if ( filledAngle <= THREE_PI_BY_TWO ) // talks
	{
		smallAngle = filledAngle - PI; quartile = 3;
	}
	else // crap
	{
		smallAngle = filledAngle - THREE_PI_BY_TWO; quartile = 4;
	}
	
	float longEnough = max(x1-x0, y1-y0); // this value just has to be long enough to reach out from the centre to beyond the edge, in *ANY* direction
	float o = longEnough * sin(smallAngle);
	float a = longEnough * cos(smallAngle);
	
	int numPoints;
	vgui::Vertex_t verts[7]; // count the points anticlockwise, going TOP, CENTER then all the rest in order
	verts[0].Init( Vector2D( cx, cy ) );
	verts[1].Init( Vector2D( cx, y0) );
	float wx, wy;
	
	// y = y1 + [(y2 - y1) / (x2 - x1)] * (x - x1)
	// x = x1 + (y - y1) / [(y2 - y1) / (x2 - x1)]
	// so 
	// y = cy + ((ry - cy) / (rx - cx)) * (x - cx)
	// x = cx + (y - cy) / ((ry - cy) / (rx - cx))
		
	if ( filledAngle <= ANGLE_FIRST_CORNER ) // top - top left
	{
		numPoints = 3;
		wy = y0;
		float rx = cx - longEnough * o;
		float ry = cy - longEnough * a;
		if ( rx == cx )
			wx = cx;
		else
			wx = cx + (rx - cx) * (y0 - cy) / (ry - cy);
	}
	else if ( filledAngle <= ANGLE_SECOND_CORNER ) // top left - bottom left
	{
		numPoints = 4;
		wx = x0;
		float rx, ry;
		if ( quartile == 1 ) // top left - middle left
		{
			rx = cx - longEnough * o;
			ry = cy - longEnough * a;
		}
		else // middle left - bottom left
		{
			rx = cx - longEnough * a;
			ry = cy + longEnough * o;
		}
		if ( ry == cy )
			wy = cy;
		else
			wy = cy + (x0 - cx) * (ry - cy) / (rx - cx);
			
		verts[2].Init( Vector2D(x0, y0) );
	}
	else if ( filledAngle <= ANGLE_THIRD_CORNER ) // bottom left - bottom right
	{
		numPoints = 5;
		wy = y1;
		float rx, ry;
		if ( quartile == 2 ) // bottom left - bottom middle
		{
			rx = cx - longEnough * a;
			ry = cy + longEnough * o;
		}
		else // bottom middle - bottom right
		{
			rx = cx + longEnough * o;
			ry = cy + longEnough * a;
		}
		if ( rx == cx )
			wx = cx;
		else
			wx = cx + (y1 - cy) * (rx - cx) / (ry - cy);
			
		verts[2].Init( Vector2D(x0, y0) );
		verts[3].Init( Vector2D(x0, y1) );
	}
	else if ( filledAngle <= ANGLE_FOURTH_CORNER ) // bottom right - top right
	{
		numPoints = 6;
		wx = x1;
		float rx, ry;
		if ( quartile == 3 ) // bottom right - middle right
		{
			rx = cx + longEnough * o;
			ry = cy + longEnough * a;
		}
		else // middle right - top right
		{
			rx = cx + longEnough * a;
			ry = cy - longEnough * o;
		}
		if ( ry == cy )
			wy = cy;
		else
			wy = cy + (x1 - cx) * (ry - cy) / (rx - cx);

		verts[2].Init( Vector2D(x0, y0) );
		verts[3].Init( Vector2D(x0, y1) );
		verts[4].Init( Vector2D(x1, y1) );
	}
	else // top - top right
	{
		numPoints = 7;
		wy = y0;
		float rx = cx + longEnough * a;
		float ry = cy - longEnough * o;
		if ( rx == cx )
			wx = cx;
		else
			wx = cx + (rx - cx) * (y0 - cy) / (ry - cy);

		verts[2].Init( Vector2D(x0, y0) );
		verts[3].Init( Vector2D(x0, y1) );
		verts[4].Init( Vector2D(x1, y1) );
		verts[5].Init( Vector2D(x1, y0) );
	}

	verts[numPoints-1].Init( Vector2D(wx, wy) );
	
	vgui::surface()->DrawSetTexture( iMaterialRef );
	vgui::surface()->DrawSetColor( Color( 0, 0, 0, WIPE_FILL_ALPHA ) );
	vgui::surface()->DrawTexturedPolygon( numPoints, verts );
	
	vgui::surface()->DrawSetColor( Color( 255, 255, 255, WIPE_LINE_ALPHA ) );
	vgui::surface()->DrawLine(cx, cy, wx, wy);
}

void CHudModule::CycleForward()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer || m_iSelectedModule == NO_MODULE )
		return;

	if ( m_bToggledOn && GetModule(m_iSelectedModule)->IsToggledByHoldingKey() )
		ReleasedSelected();

	// first cycle from here to the end
	for ( int i=m_iSelectedModule+1; i<NUM_MODULES; i++ )
	{
		Module *a = GetModule(i);
		if ( !a->AutoStarts() && pPlayer->HasModule(a) )
		{
			m_iSelectedModule = i; // select this, then
			UpdateIconColor();
			return;
		}
	}

	// as we haven't found anything, cycle from the start to here
	for ( int i=0; i<m_iSelectedModule; i++ )
	{
		Module *a = GetModule(i);
		if ( !a->AutoStarts() && pPlayer->HasModule(a) )
		{
			m_iSelectedModule = i; // select this, then
			UpdateIconColor();
			return;
		}
	}

	// failed, no change
}

void CHudModule::CycleBackward()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer || m_iSelectedModule == NO_MODULE )
		return;

	if ( m_bToggledOn && GetModule(m_iSelectedModule)->IsToggledByHoldingKey() )
		ReleasedSelected();

	// first, try from here back to the start
	for ( int i=m_iSelectedModule-1; i>=0; i-- )
	{
		Module *a = GetModule(i);
		if ( !a->AutoStarts() && pPlayer->HasModule(a) )
		{
			m_iSelectedModule = i; // select this, then
			UpdateIconColor();
			return;
		}
	}

	// then try from the end back to here
	for ( int i=NUM_MODULES-1; i>m_iSelectedModule; i-- )
	{
		Module *a = GetModule(i);

		if ( !a->AutoStarts() && pPlayer->HasModule(a) )
		{
			m_iSelectedModule = i; // select this, then
			UpdateIconColor();
			return;
		}
	}

	// failed, no change
}

// we've changed selected module. update the color of the icon to show
// if we have enough power to use the new one or not
void CHudModule::UpdateIconColor()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer || m_iSelectedModule == NO_MODULE )
		return;
	Module *a = GetModule(m_iSelectedModule);

	if ( pPlayer->IsModuleActive(a) != m_bToggledOn )
	{
		if ( pPlayer->IsModuleActive(a) )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ModuleToggleOn");
			m_bToggledOn = true;
		}
		else
		{// will play ModulePowerOK thanks to the else if below
			m_bToggledOn = false;
		}
	}

	if ( pPlayer->GetAuxPower() < a->GetAuxDrain(pPlayer,pPlayer->GetModuleLevel(a)) )
	{// fade to red
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ModulePowerLow");
		m_bLowPower = true;
	}
	else if ( !m_bToggledOn )
	{// fade from red
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ModulePowerOk");
		m_bLowPower = false;
	}
}

void CHudModule::UseSelected()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer || m_iSelectedModule == NO_MODULE )
		return;

	Module *a = GetModule(m_iSelectedModule);
//	if ( !pPlayer->HasModule(a) )	// they HAVE to have it or it wouldnt be selected
//		return;

	// should this panel decide they dont have enough power? not for now
	engine->ClientCmd(VarArgs("+do %s",a->GetCmdName()));

	int remainingPower = pPlayer->GetAuxPower() - a->GetAuxDrain(pPlayer,pPlayer->GetModuleLevel(a));
	if ( remainingPower < a->GetAuxDrain(pPlayer,pPlayer->GetModuleLevel(a)) )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("UseModuleLow");
		m_bLowPower = true;
		m_bToggledOn = a->IsToggled();
	}
	else
	{
		if ( a->IsToggled() )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ModuleToggleOn");
			m_bToggledOn = true;
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("UseModule");
			m_bToggledOn = false;
		}
		m_bLowPower = false;
	}
}

// turn off the jetpack when they release the key
void CHudModule::ReleasedSelected()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer || m_iSelectedModule == NO_MODULE )
		return;

	Module *a = GetModule(m_iSelectedModule);
	if ( a->IsToggledByHoldingKey() )
	{
		engine->ClientCmd(VarArgs("-do %s",a->GetCmdName())); // stop it again
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ModulePowerOk");
		m_bToggledOn = false;
	}
}

float CHudModule::CalcCooldownFraction(Module *a)
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer || m_iSelectedModule == NO_MODULE )
		return 1.0f;

	float end = pPlayer->GetCooldownEnd(a);
	if ( end <= gpGlobals->curtime )
		return 1.0f;

	int level = pPlayer->GetModuleLevel(a);
	float duration = a->GetCooldown(level) + a->GetCastTime(level);
	float start = end - duration;
	return ( gpGlobals->curtime - start ) / ( end - start );
}