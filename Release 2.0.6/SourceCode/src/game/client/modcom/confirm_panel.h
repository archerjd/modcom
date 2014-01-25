#ifndef CONFPANEL_H
#define CONFPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>

#include <vgui_controls/Label.h>

#include "modcom/mc_shareddefs.h"

class CConfPanel : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CConfPanel, vgui::Frame );

public:
	CConfPanel(IViewPort *pViewPort);
	virtual ~CConfPanel();

	virtual void OnThink() {}
	virtual const char *GetName( void ) { return PANEL_CONFIRM; }
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
	static void SetYesPanel(const char *panelname);
	static void SetNoPanel(const char *panelname);
	static void SetYesAction(const char *cmd);
	static bool GetLastConfirm();

	void SetDisplayText(const char *title, const char *body);
	void SetPanelToShowOnYes(const char *panelname);
	void SetPanelToShowOnNo(const char *panelname);
	void SetYesCommand(const char *cmd);
	void SetNoCommand(const char *cmd);
protected:

	// VGUI2 overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	
	// command callbacks
	virtual void OnCommand( const char *command );

private:
	IViewPort	*m_pViewPort;
	vgui::Label *m_pTitleLabel, *m_pBodyLabel;

	char m_szYesCmd[64], m_szNoCmd[64];
	char m_szYesPanel[16], m_szNoPanel[16];
	bool bYesPanel, bNoPanel, bYesCmd, bNoCmd;
};
#endif