#ifndef BUYMENU_H
#define BUYMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/BitmapImagePanel.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/PanelListPanel.h>

#include <game/client/iviewport.h>
#include <vgui/KeyCode.h>

#include <utlvector.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ToggleButton.h>
#include "scrollpanel.h"

#include "modcom/datatable.h"
#include "modcom/modules.h"
#include "modcom/mc_shareddefs.h"

namespace vgui
{
	class RichText;
}

class ModulePanel;
class ModuleTree;
class ModuleButton;
class TabButton;
class CategoryButton;
class MenuTab;
class ModuleCategoryButton;

//-----------------------------------------------------------------------------
// Purpose: Displays the buy menu
//-----------------------------------------------------------------------------
class CVortexMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CVortexMenu, vgui::Frame );

public:
	CVortexMenu(IViewPort *pViewPort);
	virtual ~CVortexMenu();

	virtual void OnThink();
	virtual const char *GetName( void ) { return PANEL_BUY; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return false; } // it will need an update after a map change
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	int GetActiveTab() { return m_iActiveTab; }
	void SwitchToTab(int tab);
	
	void SwitchToModule(int module);

	void UpdateTipLabel();
	int GetPlayerAP();
	void BoughtModule(Module *a, bool isNew);
	void PredictAPChange(int i);

	void UpdateCharacterInfo(char *model, char *created, int playerKills, int playerDeaths, int monsterKills, int monsterDeaths, int numSprees, int numSpreeWars, int biggestSpree, float playTime);

protected:

	// VGUI2 overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	
	// command callbacks
	virtual void OnCommand( const char *command );

private:
	IViewPort	*m_pViewPort;

	MenuTab *m_pTabs[NUM_TABS];
	TabButton *m_pTabButtons[NUM_TABS];

	ModuleTree *m_pModuleTree;
	ModulePanel *m_pModulePanel;
	vgui::Label *m_pInfoLabel, *m_pTipLabel;
	ButtonCode_t m_iBuyModulesKey;
	
	vgui::Label *m_pCharNameLabel, *m_pCharLevelLabel, *m_pCharExpLabel, *m_pCharGameExpLabel, *m_pCharCreatedLabel;
	vgui::Label *m_pPlayerKillsLabel, *m_pPlayerDeathsLabel, *m_pMonsterKillsLabel, *m_pMonsterDeathsLabel;
	vgui::Label *m_pSpreesLabel, *m_pSpreeWarsLabel, *m_pBiggestSpreeLabel, *m_pPlayTimeLabel;
	vgui::CBitmapImagePanel *m_pCharImage;
	vgui::Button *m_pBtnChangeFaction1, *m_pBtnChangeFaction2;

	vgui::Button *m_pBtnVoteMode, *m_pBtnNomMap;
	int m_iActiveTab, m_iModuleCategory;

	vgui::Label *m_pGameModeLabel, *m_pMapLabel, *m_pModeDescLabel;

	float m_flPredictionExpiryTime, m_flPrediction2ExpiryTime;
	int m_iPredictedAP;

	bool m_bBoughtActiveModule, m_bBoughtToggledModule, m_bBoughtPassiveModule;
};

class ModuleTree : public ScrollPanel
{
public:
	ModuleTree( vgui::Panel *pParent, CVortexMenu *pMenu, const char *pName );
	void Update();
	void ToggleCategory(int category);
	void SetSelection(vgui::Button *b);
protected:
	//virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnCommand( char const *cmd );

private:
	vgui::Button *m_pButtons[NUM_MODULES + NUM_MODULE_CATEGORIES];
	vgui::Button *m_pCategoryButtons[NUM_MODULE_CATEGORIES]; // pointers to the above
	ModuleButton *m_pItemButtons[NUM_MODULES]; // pointers to the above
	int m_iSelectedCategory;

	CVortexMenu *m_pMenu;
};

class ModuleButton : public vgui::ToggleButton
{
public:
	ModuleButton(ModuleTree *pParent, const char *name, const char *text, int module)
		: vgui::ToggleButton(pParent, name, text)
	{
		m_iModule = module;
		pMyParent = pParent;
	}

	int GetModuleIndex() { return m_iModule; }
	virtual void Enable();
	virtual void Disable();
	virtual void DoClick();
private:
	int m_iModule;
	ModuleTree *pMyParent;
};

class ModulePanel : public ScrollPanel
{
public:
	ModulePanel( vgui::Panel *pParent, CVortexMenu *pMenu, const char *pName );
	~ModulePanel() { if ( m_pDataTable != NULL ) delete m_pDataTable; }
	void Init(int module);
	void RecalculateLevel();
	void Update();
	int GetModuleIndex() { return m_iModule; }
protected:
	//virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnCommand( char const *cmd );

private:
	CVortexMenu *m_pMenu; // the menu that owns me
	vgui::Label *m_pNameLabel, *m_pLevelLabel, *m_pTypeLabel;
	vgui::Label *m_pPowerLabel, *m_pCastTimeLabel, *m_pCooldownLabel;
	vgui::Label *m_pDescriptionLabel;
	vgui::ImagePanel *m_pModuleIcon;
	DataTable *m_pDataTable;
	vgui::Button *m_pBuyButton, *m_pImageBackground;
	int m_iModule;
	int level;

	//Color _dullbg, _brightbg;
};

class TabButton : public vgui::ToggleButton
{
public:
	TabButton( vgui::Panel *pParent, CVortexMenu *pMenu, const char *pName, const char *pText, int myNum )
		: vgui::ToggleButton( pParent, pName, pText )
	{
		m_iNum = myNum;
		pMyParent = pMenu;
//		SetDrawStrongBackground(true);
	}

	virtual void Enable();
	virtual void Disable();
	virtual void DoClick();

protected:
	CVortexMenu *pMyParent;
	int m_iNum;
};

class CategoryButton : public TabButton
{
public:
	CategoryButton( vgui::Panel *pParent, CVortexMenu *pMenu, const char *pName, const char *pText, int myNum )
		: TabButton( pParent, pMenu, pName, pText, myNum )
	{
	}

	virtual void Enable();
};

class MenuTab : public vgui::EditablePanel
{
public:
	MenuTab( CVortexMenu *pParent, const char *pName )
		: EditablePanel( (vgui::Panel*)pParent, pName )
	{
		m_pParent = pParent;
	}

protected:
	virtual void OnCommand( char const *cmd );

private:
	CVortexMenu *m_pParent;
};

#endif