#include "cbase.h"
#include "menupopup.h"
// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"
 
GameUI<CMenuPopupPanel> g_MenuPopupPanel;
 
IGameUI* GetMenuPopupPanel()
{
	return &g_MenuPopupPanel;
}
 
CMenuPopupPanel::CMenuPopupPanel( vgui::VPANEL parent ) : BaseClass( NULL, "menupopup" )
{
 	SetParent(parent);
 	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
 	SetScheme( scheme );

	m_pLabel = new vgui::Label(this,"message","");
	m_pExtraLabel1 = new vgui::Label(this,"extraLabel1","");
	m_pExtraLabel2 = new vgui::Label(this,"extraLabel2","");

 	LoadControlSettings("resource/ui/MenuPopup.res");
 	CenterThisPanelOnScreen();//keep in mind, hl2 supports widescreen 
 	SetVisible(false);//made visible on command later 
 
 	//Other useful options
 	SetSizeable(false);
 	//SetMoveable(false);
}

void CMenuPopupPanel::SetMessage(const char *title, const char* text)
{
	SetTitle(title, true);
	m_pLabel->SetText(text);
	SetVisible(true);
}

void CMenuPopupPanel::ShowMissingContentLabels(bool show)
{
	m_pExtraLabel1->SetVisible(show);
	m_pExtraLabel2->SetVisible(show);

	if ( !show )
	{
		int w, h;
		GetSize(w, h);

		h = vgui::scheme()->GetProportionalScaledValue(48);
		SetSize(w, h);


		m_pLabel->GetPos(w, h);
		h = vgui::scheme()->GetProportionalScaledValue(20);
		m_pLabel->SetPos(w,h);
	}
}

void CMenuPopupPanel::OnCommand( const char *command )
{
	if ( FStrEq( command, "Exit" ) )
	{
		SetVisible(false);
		engine->ClientCmd("quit");
	}

	BaseClass::OnCommand(command);
}