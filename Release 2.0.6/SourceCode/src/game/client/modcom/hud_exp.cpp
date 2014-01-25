#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"

#include "vgui_controls/AnimationController.h"
#include "vgui/ILocalize.h"
#include "c_hl2mp_player.h"
#include "hl2mp_gamerules.h"

#include <vgui/ISurface.h>
#include "modcom/mc_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_BAT	-1

//-----------------------------------------------------------------------------
// Purpose: Display Experience on hud
//-----------------------------------------------------------------------------
class CHudExp : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudExp, CHudNumericDisplay );

public:
	CHudExp( const char *pElementName );
	void Init( void );
	void Reset( void );
	void VidInit( void );
	void OnThink( void );
	void Paint( void );

private:
	int m_iExp;
	int m_iNextLevel;

	CPanelAnimationVar( Color, m_brightBarColor, "BrightBarColor", "255 255 255 255" );
	CPanelAnimationVar( Color, m_dullBarColor, "DullBarColor", "255 255 255 96" );

	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "33", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "72", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "10", "proportional_float" );

};

DECLARE_HUDELEMENT( CHudExp );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudExp::CHudExp( const char *pElementName ) : BaseClass(NULL, "HudExp"), CHudElement( pElementName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudExp::Init( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudExp::Reset( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudExp::VidInit( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudExp::OnThink()
{
	int newExp = 0;
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();


	if ( pPlayer )
		newExp = max( pPlayer->GetGameExp(), 0 );

	// Only update if we've gained experience.
	if ( newExp == m_iExp )
		return;

	m_iExp = newExp;
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ExpGain");
 
	// Roll effect.
	if ( m_iExp >= newExp - 8)
		m_iExp = newExp;
	else
		m_iExp = m_iExp + 8;

	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ExpGainLoop");


	if ( m_iExp >= newExp )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ExpGain");
	}

	SetDisplayValue( m_iExp );
}

void CHudExp::Paint()
{
	BaseClass::Paint();

	C_HL2MP_Player *pPlayer = (C_HL2MP_Player*)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;
	
	float fraction = 1.0f;
	if ( pPlayer->GetLevel() < HL2MPRules()->GetMaxLevel() )
	{
		float current = pPlayer->GetTotalExp();
		float start = TotExpForLevelUp(pPlayer->GetLevel()-1);
		float end = TotExpForLevelUp(pPlayer->GetLevel());
		fraction = ( current - start ) / ( end - start );
//		Msg("current, start, end, fraction: %.0f, %.0f, %.0f, %.2f\n",current,start,end,fraction);
	}

	// draw the full portion of the bar
	vgui::surface()->DrawSetColor( m_brightBarColor );
	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;
	vgui::surface()->DrawFilledRect( xpos, ypos, xpos + m_flBarWidth * fraction, ypos + m_flBarHeight );

	// draw the empty portion of the bar.
	vgui::surface()->DrawSetColor( m_dullBarColor );
	vgui::surface()->DrawFilledRect( xpos + m_flBarWidth * fraction, ypos, xpos + m_flBarWidth, ypos + m_flBarHeight );

//	Msg("fraction %.2f\n",fraction);
}