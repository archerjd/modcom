#include "cbase.h"
#include "msg_panel.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CMsgPanel::CMsgPanel(IViewPort *pViewPort) : Frame(NULL, PANEL_MSG )
{
	m_pViewPort = pViewPort;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	m_pTitleLabel = new vgui::Label(this,"titleLabel","");
	m_pBodyLabel = new vgui::Label(this,"bodyLabel","");

	LoadControlSettings("resource/ui/MsgPanel.res");

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

	bNextPanel = false; // no next panel to show, unless one is set
}

CMsgPanel::~CMsgPanel()
{

}

void CMsgPanel::Update()
{

}

void CMsgPanel::ShowPanel( bool bShow )
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
		
		if ( bNextPanel )
		{// show the panel we've been set to when we're closed
			gViewPortInterface->ShowPanel( m_szNextPanel, true );
			bNextPanel = false;
		}
	}
}

void CMsgPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CMsgPanel::OnKeyCodePressed(vgui::KeyCode code)
{
	BaseClass::OnKeyCodePressed( code );
}


void CMsgPanel::OnCommand( const char *command )
{
	if ( FStrEq( command, "ok" ) )
	{
		ShowPanel(false);
		return;
	}
	BaseClass::OnCommand(command);
}

void CMsgPanel::SetDisplayText(const char *title, const char *body)
{
	m_pTitleLabel->SetText(title);
	m_pBodyLabel->SetText(body);
}


void CMsgPanel::ShowMessage(const char *title, const char *body)
{
	CMsgPanel *pPanel = (CMsgPanel*)gViewPortInterface->FindPanelByName( PANEL_MSG );
	pPanel->SetDisplayText(title,body);
	gViewPortInterface->ShowPanel( PANEL_MSG, true );
}

void CMsgPanel::SetNextPanel(const char *panelname)
{
	CMsgPanel *pPanel = (CMsgPanel*)gViewPortInterface->FindPanelByName( PANEL_MSG );
	pPanel->SetPanelToShowOnClose(panelname);
}