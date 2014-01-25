#ifndef VOTEPANEL_H
#define VOTEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

#include <game/client/iviewport.h>
#include <igameevents.h>
#include "GameEventListener.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/TextEntry.h>
#include "vgui_controls/BitmapImagePanel.h"

class CVotePanel : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CVotePanel, vgui::Frame );

public:
	CVotePanel(IViewPort *pViewPort);
	virtual ~CVotePanel();

	virtual void OnThink() {}
	virtual const char *GetName( void ) { return PANEL_VOTE; }
	virtual void Update();
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return false; }
	virtual void ShowPanel( bool bShow );

	void SetMessage(const char *pMsg);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

protected:
	
	// VGUI2 overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);

	// command callbacks
	virtual void OnCommand( const char *command );

private:
	vgui::Label *m_pMessageLabel;
	IViewPort	*m_pViewPort;
};

#endif //VOTEPANEL_H