#include "cbase.h"
#include "confirm_panel.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static bool bLastConfirm;

CConfPanel::CConfPanel(IViewPort *pViewPort) : Frame(NULL, PANEL_CONFIRM )
{
	m_pViewPort = pViewPort;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	m_pTitleLabel = new vgui::Label(this,"titleLabel","");
	m_pBodyLabel = new vgui::Label(this,"bodyLabel","");

	LoadControlSettings("resource/ui/ConfirmPanel.res");

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

	bYesPanel = bNoPanel = bYesCmd = bNoCmd = false;
	bLastConfirm = false;
}

CConfPanel::~CConfPanel()
{

}

void CConfPanel::Update()
{

}

void CConfPanel::ShowPanel( bool bShow )
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

void CConfPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CConfPanel::OnKeyCodePressed(vgui::KeyCode code)
{
	BaseClass::OnKeyCodePressed( code );
}


void CConfPanel::OnCommand( const char *command )
{
	if ( FStrEq( command, "yes" ) )
	{
		ShowPanel(false);

		bLastConfirm = true; // must happen BEFORE we show the next panel (oops!)

		if ( bYesPanel )
			gViewPortInterface->ShowPanel( m_szYesPanel, true );

		if ( bYesCmd )
			engine->ClientCmd( m_szYesCmd );

		bYesPanel = bNoPanel = bYesCmd = bNoCmd = false;
		return;
	}
	if ( FStrEq( command, "no" ) )
	{
		ShowPanel(false);

		bLastConfirm = false; // must happen BEFORE we show the next panel (oops!)

		if ( bNoPanel )
			gViewPortInterface->ShowPanel( m_szNoPanel, true );

		if ( bNoCmd )
			engine->ClientCmd( m_szNoCmd );

		bYesPanel = bNoPanel = bYesCmd = bNoCmd = false;
		
		return;
	}
	BaseClass::OnCommand(command);
}

void CConfPanel::SetDisplayText(const char *title, const char *body)
{
	m_pTitleLabel->SetText(title);
	m_pBodyLabel->SetText(body);
}


void CConfPanel::ShowMessage(const char *title, const char *body)
{
	CConfPanel *pPanel = (CConfPanel*)gViewPortInterface->FindPanelByName( PANEL_CONFIRM );
	pPanel->SetDisplayText(title,body);
	gViewPortInterface->ShowPanel( PANEL_CONFIRM, true );
}

void CConfPanel::SetYesAction(const char *cmd)
{
	CConfPanel *pPanel = (CConfPanel*)gViewPortInterface->FindPanelByName( PANEL_CONFIRM );
	pPanel->SetYesCommand(cmd);
}

void CConfPanel::SetYesPanel(const char *panelname)
{
	CConfPanel *pPanel = (CConfPanel*)gViewPortInterface->FindPanelByName( PANEL_CONFIRM );
	pPanel->SetPanelToShowOnYes(panelname);
}

void CConfPanel::SetNoPanel(const char *panelname)
{
	CConfPanel *pPanel = (CConfPanel*)gViewPortInterface->FindPanelByName( PANEL_CONFIRM );
	pPanel->SetPanelToShowOnNo(panelname);
}

bool CConfPanel::GetLastConfirm()
{
	return bLastConfirm;
}


void CConfPanel::SetPanelToShowOnYes(const char *panelname)
{
	Q_snprintf(m_szYesPanel,sizeof(m_szYesPanel),panelname);
	bYesPanel = true;
}

void CConfPanel::SetPanelToShowOnNo(const char *panelname)
{
	Q_snprintf(m_szNoPanel,sizeof(m_szNoPanel),panelname);
	bNoPanel = true;
}

void CConfPanel::SetYesCommand(const char *cmd)
{
	Q_snprintf(m_szYesCmd,sizeof(m_szYesCmd),cmd);
	bYesCmd = true;
}

void CConfPanel::SetNoCommand(const char *cmd)
{
	Q_snprintf(m_szNoCmd,sizeof(m_szNoCmd),cmd);
	bNoCmd = true;
}