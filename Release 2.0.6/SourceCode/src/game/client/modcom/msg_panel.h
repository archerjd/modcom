#ifndef MSGPANEL_H
#define MSGPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>

#include <vgui_controls/Label.h>

#include "modcom/mc_shareddefs.h"

class CMsgPanel : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CMsgPanel, vgui::Frame );

public:
	CMsgPanel(IViewPort *pViewPort);
	virtual ~CMsgPanel();

	virtual void OnThink() {}
	virtual const char *GetName( void ) { return PANEL_MSG; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	static void ShowMessage(const char *title, const char *body);
	static void SetNextPanel(const char *panelname);

	void SetDisplayText(const char *title, const char *body);
	void SetPanelToShowOnClose(const char *panelname) { m_szNextPanel = panelname; bNextPanel = true; }

protected:

	// VGUI2 overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	
	// command callbacks
	virtual void OnCommand( const char *command );

private:
	IViewPort	*m_pViewPort;
	vgui::Label *m_pTitleLabel, *m_pBodyLabel;

	const char *m_szNextPanel;
	bool bNextPanel;
};
#endif