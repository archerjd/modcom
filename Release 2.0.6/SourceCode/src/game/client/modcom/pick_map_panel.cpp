#include "cbase.h"
#include "pick_map_panel.h"
#include "networkstringtable_clientdll.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar mc_can_vote_map;

CPickMapPanel::CPickMapPanel(IViewPort *pViewPort) : Frame(NULL, PANEL_PICKMAP)
{
	m_pViewPort = pViewPort;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	m_pMapname = new vgui::ComboBox(this, "mapname", 7, false);

	LoadControlSettings("resource/ui/PickMapPanel.res");

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);
}

CPickMapPanel::~CPickMapPanel()
{

}

void CPickMapPanel::Update()
{

}

extern INetworkStringTable *g_pMapListStringTable;
void CPickMapPanel::ShowPanel( bool bShow )
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		m_pMapname->RemoveAll();
		for ( int i=0; i<g_pMapListStringTable->GetNumStrings(); i++ )
		{
			KeyValues *kv = new KeyValues("UserData", "command", "");
			m_pMapname->AddItem( g_pMapListStringTable->GetString(i), kv );	
			kv->deleteThis();
		}
		m_pMapname->SetEditable(false);

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

void CPickMapPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	m_pMapname->GetMenu()->MakeReadyForUse();
	BaseClass::ApplySchemeSettings(pScheme);
}

void CPickMapPanel::OnKeyCodePressed(vgui::KeyCode code)
{
	if ( code == KEY_ESCAPE )
	{
		ShowPanel(false);
		return;
	}

	BaseClass::OnKeyCodePressed( code );
}

extern bool IsValidInputString(const char *test);


void CPickMapPanel::OnCommand( const char *command )
{
	if ( FStrEq( command, "cancel" ) )
	{
		ShowPanel(false);
		return;
	}
	else if ( FStrEq( command, "ok" ) )
	{
		engine->ClientCmd(VarArgs("nominate %s",g_pMapListStringTable->GetString(m_pMapname->GetActiveItem())));
		ShowPanel(false);
		return;
	}

	BaseClass::OnCommand(command);
}