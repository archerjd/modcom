//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#if !defined( HUD_BUFFS_H )
#define HUD_BUFFS_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>
#include "modcom/buffs.h"

class C_HL2MP_Player;

//-----------------------------------------------------------------------------
// Purpose: Shows the sprint power bar
//-----------------------------------------------------------------------------
class CHudBuffs : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudBuffs, vgui::Panel );

public:
	CHudBuffs( const char *pElementName );
	virtual void	Init( void );
	virtual void	VidInit( void );
	virtual void	Reset( void );
	virtual void	OnThink( void );
	bool			ShouldDraw( void );

	static CHudBuffs *instance;

	//void UseSelected();
	//void ReleasedSelected(); // on key up, for turning off jetpack
	//void CycleForward();
	//void CycleBackward();

protected:
	virtual void	Paint();
	//void UpdateIconColor();

	void AnimateItemRemoval(int i);
	float GetProgressFraction(C_HL2MP_Player *pPlayer, Buff *b);

private:	
	CPanelAnimationVar( vgui::HFont, m_hFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, item_width, "item_width", "46", "proportional_float" );
	CPanelAnimationVarAliasType( float, item_height, "item_height", "16", "proportional_float" );
	CPanelAnimationVarAliasType( float, item_seperator, "item_seperator", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_xoffset, "text_xoffset", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_yoffset, "text_yoffset", "2", "proportional_float" );
	
	CPanelAnimationVar( Color, m_TextColor, "TextColor", "255 255 255 255" );
	CPanelAnimationVar( Color, m_NeutralColor, "NeutralColor", "128 128 128 255" );
	CPanelAnimationVar( Color, m_BackColor, "BackColor", "32 32 32 96" );
	
	CPanelAnimationVarAliasType( float, item1_extra, "item1_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item2_extra, "item2_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item3_extra, "item3_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item4_extra, "item4_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item5_extra, "item5_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item6_extra, "item6_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item7_extra, "item7_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item8_extra, "item8_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item9_extra, "item9_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item10_extra, "item10_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item11_extra, "item11_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item12_extra, "item12_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item13_extra, "item13_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item14_extra, "item14_extra", "0", "float" );
	CPanelAnimationVarAliasType( float, item15_extra, "item15_extra", "0", "float" );
	
	Color m_GoodColor, m_BadColor;
	int m_iBuffsBeingShown[NUM_BUFFS];
	bool m_bScheduledRemovals[NUM_BUFFS];

	int m_iNumBuffsBeingShown;
	
	int m_WipeTexture, m_LevelTexture;
};	

// accessor
CHudBuffs *GetBuffHud();

#endif // HUD_BUFFS_H