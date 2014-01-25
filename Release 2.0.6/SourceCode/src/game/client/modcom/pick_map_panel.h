#ifndef PICKMAPPANEL_H
#define PICKMAPPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

#include <game/client/iviewport.h>
#include <igameevents.h>
#include "GameEventListener.h"

#include <vgui_controls/Button.h>
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/BitmapImagePanel.h"

class CPickMapPanel : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CPickMapPanel, vgui::Frame );

public:
	CPickMapPanel(IViewPort *pViewPort);
	virtual ~CPickMapPanel();

	virtual void OnThink() {}
	virtual const char *GetName( void ) { return PANEL_PICKMAP; }
	virtual void Update();
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

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
	IViewPort	*m_pViewPort;

	vgui::ComboBox *m_pMapname;
};

#endif