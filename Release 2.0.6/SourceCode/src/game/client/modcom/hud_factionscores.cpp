#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <igameresources.h>

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
// Purpose: Display faction scores (for Hoarder game mode) on hud
//-----------------------------------------------------------------------------
class CHudFactionScores : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudFactionScores, vgui::Panel );

public:
	CHudFactionScores( const char *pElementName );
	void Init( void );
	void Reset( void );
	void VidInit( void );
	bool ShouldDraw( void );
	void OnThink( void );
	void Paint( void );

private:
	int m_iScores[NUM_FACTIONS];
	int m_iTargets[NUM_FACTIONS];

	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "4", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "72", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarSeperator, "BarSeperator", "2", "proportional_float" );

};

DECLARE_HUDELEMENT( CHudFactionScores );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudFactionScores::CHudFactionScores( const char *pElementName ) : CHudElement( pElementName ), BaseClass(NULL, "HudFactionScores")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT );
	for ( int i=0; i<NUM_FACTIONS; i++ )
	{
		m_iScores[i] = 0;
		m_iTargets[i] = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudFactionScores::Init( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudFactionScores::Reset( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudFactionScores::VidInit( void )
{
	Reset();
}

bool CHudFactionScores::ShouldDraw()
{
	return HL2MPRules() && HL2MPRules()->ShouldUseScoreTokens();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudFactionScores::OnThink()
{
	if ( !HL2MPRules() )
		return;

	bool changed = false;
	for ( int i=0; i<NUM_FACTIONS; i++ )
	{
		int val = HL2MPRules()->GetFactionScoreTokenCount(i+1);
		if ( m_iScores[i] != val )
		{
			changed = true;
			m_iScores[i] = val;
		}

		val = HL2MPRules()->GetFactionScoreTokenTarget(i+1);
		if ( m_iTargets[i] != val )
		{
			changed = true;
			m_iTargets[i] = val;
		}
	}
/*
	// don't bother updating if nothing's changed
	if ( !changed )
		return;

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

	SetDisplayValue( m_iExp );*/
}

void CHudFactionScores::Paint()
{
	int ypos = m_flBarInsetY;
	for ( int i=0; i<NUM_FACTIONS; i++ )
	{
		float fraction = 0;
		if ( m_iTargets[i] > 0 )
			fraction = (float)m_iScores[i] / (float)m_iTargets[i];

		int fillWidth = max(m_flBarWidth * fraction, 1);

		// draw the full portion of the bar
		Color c = GameResources()->GetTeamColor(i+1);
		vgui::surface()->DrawSetColor( c );
		vgui::surface()->DrawFilledRect( m_flBarInsetX, ypos, m_flBarInsetX + fillWidth, ypos + m_flBarHeight );

		// draw the empty portion of the bar.
		c.SetColor(c.r() * 0.2, c.g() * 0.2, c.b() * 0.2, c.a() );
		vgui::surface()->DrawSetColor( c );
		vgui::surface()->DrawFilledRect( m_flBarInsetX + fillWidth, ypos, m_flBarInsetX + m_flBarWidth, ypos + m_flBarHeight );

		ypos += m_flBarHeight + m_flBarSeperator;
	}
}