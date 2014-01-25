#ifndef MENUPOPUP_H
#define MENUPOPUP_H
#ifdef _WIN32
#pragma once
#endif
 
#include "vgui_helpers.h"
#include <vgui_controls/Frame.h>
 
 
class CMenuPopupPanel : public vgui::Frame
{
 	DECLARE_CLASS_SIMPLE(CMenuPopupPanel, vgui::Frame);
 public:
 	CMenuPopupPanel( vgui::VPANEL parent );
	void SetMessage(const char* title, const char* text);
	void ShowMissingContentLabels(bool show);
	void OnCommand( const char *command );
private:
	vgui::Label *m_pLabel, *m_pExtraLabel1, *m_pExtraLabel2;
};
 
IGameUI* GetMenuPopupPanel();
 
#endif // MENUPOPUP_H