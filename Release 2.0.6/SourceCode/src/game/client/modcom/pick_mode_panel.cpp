#include "cbase.h"
#include "modcom/mc_shareddefs.h"
#include "modcom/pick_mode_panel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar mc_gamemode;

CPickModePanel::CPickModePanel(IViewPort *pViewPort) : Frame(NULL, PANEL_PICKMODE)
{
	m_pViewPort = pViewPort;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

//	for ( int i=0; i<NUM_GAME_MODES-1; i++ )
//		m_pModeButtons[i] = new vgui::Button(this,VarArgs("btnMode%i",i),"");

	LoadControlSettings("resource/ui/PickModePanel.res");

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);
}

CPickModePanel::~CPickModePanel()
{

}

void CPickModePanel::Update()
{

}

void CPickModePanel::ShowPanel( bool bShow )
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();
//		Update();  // this is already called when showing
	}

//	SetPaintBackgroundEnabled(bShow);
//	SetPaintBorderEnabled(bShow);
	SetMouseInputEnabled(bShow);
	SetVisible(bShow);
	m_pViewPort->ShowBackGround( bShow );

	if ( !bShow )
	{
		Close();
	}
}

void CPickModePanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CPickModePanel::OnKeyCodePressed(vgui::KeyCode code)
{
	if ( code == KEY_ESCAPE )
	{
		ShowPanel(false);
		return;
	}

	BaseClass::OnKeyCodePressed( code );
}

extern bool IsValidInputString(const char *test);


void CPickModePanel::OnCommand( const char *command )
{
	if ( FStrEq( command, "cancel" ) )
	{
		ShowPanel(false);
		//gViewPortInterface->ShowPanel( PANEL_PICKCHAR, true );
		return;
	}	
	
	if ( FStrEq( command, "mode") )
	{
		ShowPanel(false);
		engine->ClientCmd("startvote");
	}

	for ( int i=1; i<=NUM_GAME_MODES; i++ )
		if ( FStrEq( command, VarArgs("mode%i",i) ) )
		{
			ShowPanel(false);
			engine->ClientCmd(VarArgs("startvote %i",i));
			break;
		}

	BaseClass::OnCommand(command);
}