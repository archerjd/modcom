#ifndef FACTIONMODELPANEL_H
#define FACTIONMODELPANEL_H
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
#include "vgui_controls/ScrollBar.h"
#include "vgui_controls/ScrollBarSlider.h"
#include "vgui_controls/ComboBox.h"

#include "modcom/mc_shareddefs.h"

#define MAX_MODELS	35 // using fixed strings is much easier than going all pointer-y

class CFactionModelPanel : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CFactionModelPanel, vgui::Frame );

public:
	CFactionModelPanel(IViewPort *pViewPort);
	virtual ~CFactionModelPanel();

	virtual void OnThink() {}
	virtual const char *GetName( void ) { return PANEL_FACTIONMODEL; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};
	virtual void Update() {}
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow ) {}

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

protected:
	
	// load in all player material files
	//void LoadAllFilesRecursive(const char *rootdir, const char *pathID, const char *extension,const char *reldir = "");
/*	void AddMaterialFile(const char *filename, int faction);

	const char *GetTextureName();
	const char *GetModelName();

	// VGUI2 overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	
	// command callbacks
	virtual void OnCommand( const char *command ); */

private:
//	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", data );
	IViewPort	*m_pViewPort;

	int m_iNumModelsCombine, m_iNumModelsResistance, m_iNumModelsAperture, m_iModel;
	char m_szCombineModels[MAX_MODELS][128];
	char m_szResistanceModels[MAX_MODELS][128];
	char m_szApertureModels[MAX_MODELS][128];

	vgui::TextEntry *m_pCharname;
	vgui::CBitmapImagePanel *m_pCharImage;
	vgui::Label *m_pFactionDesc;
	vgui::ComboBox *m_pFactionSelect, *m_pProgressionSelect;
	int m_iFaction;
};


#endif