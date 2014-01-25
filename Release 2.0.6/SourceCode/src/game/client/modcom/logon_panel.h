#ifndef LOGONPANEL_H
#define LOGONPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

#include <game/client/iviewport.h>
#include <igameevents.h>
#include "GameEventListener.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/CheckButton.h>

class TogglePairButton;
class NotifyingTextEntry;
struct sqlite3;

class CLogonPanel : public vgui::Frame, public IViewPortPanel, public CGameEventListener
{
private:
	DECLARE_CLASS_SIMPLE( CLogonPanel, vgui::Frame );

public:
	CLogonPanel(IViewPort *pViewPort);
	virtual ~CLogonPanel();

	virtual void OnThink() {}
	virtual const char *GetName( void ) { return PANEL_LOGON; }
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

	virtual void FireGameEvent( IGameEvent *event );

	void SetConfirmVisible(bool b); // should password confirm box be shown?
	void ForgetPassword();
protected:

	// VGUI2 overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	
	// command callbacks
	virtual void OnCommand( const char *command );

private:
	IViewPort	*m_pViewPort;

	vgui::Label *m_pTitleLabel, *m_pConfirmPasswordLabel;
	TogglePairButton *m_pExistingButton, *m_pNewButton;
	vgui::TextEntry *m_pUsername, *m_pConfirmPassword;
	NotifyingTextEntry *m_pPassword;
	vgui::CheckButton *m_pRememberMe;

	char m_szGameAddress[32];
	char m_szStoredPassword[38];
	bool m_bHasStoredPassword;
};



class TogglePairButton : public vgui::ToggleButton
{
public:
	TogglePairButton( CLogonPanel *pParent, const char *pName, const char *pText, bool bShow )
		: ToggleButton( (vgui::Panel*)pParent, pName, pText )
	{
		m_pParent = pParent;
		m_bShowExtra = bShow;
	}
	void SetPartner(TogglePairButton *p) { m_pPartner = p; }
	virtual void DoClick();
	void Enable();
	void Disable();

private:
	TogglePairButton *m_pPartner;
	CLogonPanel *m_pParent;
	bool m_bShowExtra;
};

class NotifyingTextEntry : public vgui::TextEntry
{
public:
	NotifyingTextEntry( CLogonPanel *pParent, const char *pName)
		: vgui::TextEntry( (vgui::Panel*)pParent, pName)
	{
		m_pParent = pParent;
	}

	virtual void OnKeyCodePressed( vgui::KeyCode code );

private:
	CLogonPanel *m_pParent;
};
#endif