//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#if !defined( HUD_MODULE_H )
#define HUD_MODULE_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>
#include "modcom/modules.h"

//-----------------------------------------------------------------------------
// Purpose: Shows the sprint power bar
//-----------------------------------------------------------------------------
class CHudModule : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudModule, vgui::Panel );

public:
	CHudModule( const char *pElementName );
	virtual void	Init( void );
	virtual void	VidInit( void );
	virtual void	Reset( void );
	virtual void	OnThink( void );
	bool			ShouldDraw( void );

	static CHudModule *instance;

	void UseSelected();
	void ReleasedSelected(); // on key up, for turning off jetpack
	void CycleForward();
	void CycleBackward();

protected:
	virtual void	Paint();
	void DrawRadialWipe(int x0, int y0, int x1, int y1, float fraction, int iMaterialRef);
	void UpdateIconColor();
	float CalcCooldownFraction(Module *a);

private:	
	CPanelAnimationVar( vgui::HFont, m_hFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "42", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_size, "icon_size", "40", "proportional_float" );

	CPanelAnimationVar( Color, m_IconColor, "IconColor", "0 0 0 0" );

	int m_iSelectedModule;
	int icons[NUM_MODULES];
	//int m_iWipeMaterial;

	// animation control
	bool m_bLowPower, m_bToggledOn;
};	

// accessor
CHudModule *GetModuleHud();

#endif // HUD_MODULE_H