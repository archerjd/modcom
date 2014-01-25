#ifndef PICKCHARPANEL_H
#define PICKCHARPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui/MouseCode.h>
#include <game/client/iviewport.h>
#include <igameevents.h>
#include "GameEventListener.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/ToggleButton.h>
#include "vgui_controls/BitmapImagePanel.h"
#include "modcom/mc_shareddefs.h"

class ButtonImagePanel;

class CPickCharPanel : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CPickCharPanel, vgui::Frame );

public:
	CPickCharPanel(IViewPort *pViewPort);
	virtual ~CPickCharPanel();

	virtual void OnThink() {}
	virtual const char *GetName( void ) { return PANEL_PICKCHAR; }
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

	void ResetCharInfo(); // clear ALL character info, reset iterator
	void Setup(int page, int totNumChars);
	void AddCharacterInfo(int id,const char *name,const char *model,int iLevel, const char *created, const char *lastAccess);

protected:

	// VGUI2 overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	
	// command callbacks
	virtual void OnCommand( const char *command );

private:
	IViewPort	*m_pViewPort;

	vgui::Label *m_pBodyLabel1, *m_pBodyLabel2;
	int m_iCharacterIDs[CHARS_PER_PAGE];
	vgui::Button *m_pCharButtons[CHARS_PER_PAGE], *m_pNewCharButton, *m_pButtonPrev, *m_pButtonNext;
	vgui::CBitmapImagePanel *m_pCharImages[CHARS_PER_PAGE];

	vgui::ToggleButton *m_pDeleteBtn;
	char m_szMaterials[CHARS_PER_PAGE][MODEL_LENGTH];

	int m_iTotNumChars, m_iPageNumber;
	//int m_iAwaitDelete;
	int m_iIterator;
};

class ButtonImagePanel : public vgui::CBitmapImagePanel
{
public:
	ButtonImagePanel(vgui::Panel *pParent, const char *name, vgui::Panel *pTarget)
		: CBitmapImagePanel(pParent, name)
	{
		m_pParent = pTarget;
		SetMouseInputEnabled(false);
	}
	virtual void OnCursorEntered() { m_pParent->OnCursorEntered(); }
	virtual void OnCursorExited() { m_pParent->OnCursorExited(); }
	virtual void OnMousePressed(vgui::MouseCode code) { m_pParent->OnMousePressed(code); }
	virtual void OnMouseDoublePressed(vgui::MouseCode code) { m_pParent->OnMouseDoublePressed(code); }
	virtual void OnMouseReleased(vgui::MouseCode code) { m_pParent->OnMouseReleased(code); }
private:
	vgui::Panel *m_pParent;
};

#endif