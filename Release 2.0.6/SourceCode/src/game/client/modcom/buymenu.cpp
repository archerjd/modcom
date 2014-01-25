// ability buying menu

#include "cbase.h"
#include "iclientmode.h"

#include "vgui_controls/AnimationController.h"
#include "vgui/ILocalize.h"
#include "c_hl2mp_player.h"
#include "buymenu.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui_controls/ComboBox.h>

#include <stdlib.h> // MAX_PATH define
#include <stdio.h>

#include "hl2mp_gamerules.h"

#include "IGameUIFuncs.h" // for key bindings
#include <igameresources.h>

#include "hudelement.h"
#include "hud_hintdisplay.h"

extern IGameUIFuncs *gameuifuncs; // for key binding details

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef _XBOX
extern IGameUIFuncs *gameuifuncs; // for key binding details
#endif

using namespace vgui;

#define BUTTONS_PER_ROW	4
#define BUTTONS_TALL	4
#define BUTTON_SIZE		72
#define BUTTON_SPACING	4

#define BORDER_SIDE		2
#define BORDER_TOP		20
#define BORDER_BOTTOM	BORDER_SIDE

#define DEFAULT_TAB		TAB_MODULES

extern ConVar mc_gamemode;
extern ConVar mc_vote_gamemode_enabled;
extern ConVar mc_vote_map_enabled;

CVortexMenu::CVortexMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_BUY )
{
	m_pViewPort = pViewPort;

	// load the new scheme early!!
	SetScheme("ClientScheme");
//	HFont btnFont = scheme()->GetIScheme( GetScheme() )->GetFont( "ButtonBig" );
//	HFont titleFont = scheme()->GetIScheme( GetScheme() )->GetFont( "Bigger" );
//	pointsFont = scheme()->GetIScheme( GetScheme() )->GetFont( "ReallyBig" );

	m_flPredictionExpiryTime = m_flPrediction2ExpiryTime = 0;
	m_iPredictedAP = 0;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	m_iBuyModulesKey = BUTTON_CODE_INVALID; // looked up in Activate()
//	m_pOkButton = new Button(this,"okButton","OK");

	m_pInfoLabel = new Label(this,"infoLabel","");
	m_pTipLabel = new Label(this,"tipLabel","");

	// set up the tabs
	m_pTabs[TAB_GAME] = new MenuTab(this,"game");
	m_pTabs[TAB_CHARACTER] = new MenuTab(this,"character");
	m_pTabs[TAB_MODULES] = new MenuTab(this,"abilities");
	m_pTabs[TAB_WEAPONS] = new MenuTab(this,"weapons");
	m_pTabs[TAB_TALENTS] = new MenuTab(this,"talents");
	//m_pTabs[TAB_MP3PLAYER] = new MenuTab(this,"mp3 player");

	m_pTabButtons[TAB_GAME] = new TabButton(this,this,"btnTab1","#tab_game",TAB_GAME);
	m_pTabButtons[TAB_CHARACTER] = new TabButton(this,this,"btnTab2","#tab_character",TAB_CHARACTER);
	m_pTabButtons[TAB_MODULES] = new TabButton(this,this,"btnTab3","#tab_abilities",TAB_MODULES);
	m_pTabButtons[TAB_WEAPONS] = new TabButton(this,this,"btnTab4","#tab_weapons",TAB_WEAPONS);
	m_pTabButtons[TAB_TALENTS] = new TabButton(this,this,"btnTab5","#tab_talents",TAB_TALENTS);
	//m_pTabButtons[TAB_TALENTS] = new TabButton(this,this,"btnTab6","#tab_mp3player",TAB_MP3PLAYER);

	for ( int i=0; i<NUM_TABS; i++ )
	{
		m_pTabs[i]->SetPaintBackgroundEnabled(false);
		m_pTabs[i]->SetPaintBorderEnabled(false);
	}

	LoadControlSettings("resource/ui/VortexMenu.res");
//	InvalidateLayout();

	m_iActiveTab = DEFAULT_TAB;
	m_iModuleCategory = MODULE_CATEGORY_CORE;
	
	//set up game tab
	MenuTab *t = m_pTabs[TAB_GAME];
	m_pMapLabel = new vgui::Label(t,"mapLabel","");
	m_pGameModeLabel = new vgui::Label(t,"gameModeLabel","");
	m_pModeDescLabel = new vgui::Label(t,"modeDescLabel","");
	m_pBtnVoteMode = new vgui::Button(t, "btnVoteMode", "");
	m_pBtnNomMap = new vgui::Button(t, "btnNomMap", "");
	t->LoadControlSettings("resource/ui/MenuGamePanel.res");

	// set up character tab
	t = m_pTabs[TAB_CHARACTER];
	m_pCharNameLabel = new vgui::Label(t,"charNameLabel","");
	m_pCharLevelLabel = new vgui::Label(t,"charLevelLabel","");
	m_pCharExpLabel = new vgui::Label(t,"charExpLabel","");
	m_pCharGameExpLabel = new vgui::Label(t,"charGameExpLabel","");
	m_pCharCreatedLabel = new vgui::Label(t,"charCreatedLabel","");
	m_pPlayerKillsLabel = new vgui::Label(t,"playerKillsLabel","");
	m_pPlayerDeathsLabel = new vgui::Label(t,"playerDeathsLabel","");
	m_pMonsterKillsLabel = new vgui::Label(t,"monsterKillsLabel","");
	m_pMonsterDeathsLabel = new vgui::Label(t,"monsterDeathsLabel","");

	m_pSpreesLabel = new vgui::Label(t,"spreesLabel","");
	m_pSpreeWarsLabel = new vgui::Label(t,"spreeWarsLabel","");
	m_pBiggestSpreeLabel = new vgui::Label(t,"biggestSpreeLabel","");
	m_pPlayTimeLabel = new vgui::Label(t,"timePlayedLabel","");

	m_pBtnChangeFaction1 = new vgui::Button(t,"btnChangeFaction1", "");
	m_pBtnChangeFaction2 = new vgui::Button(t,"btnChangeFaction2", "");
	m_pCharImage = new vgui::CBitmapImagePanel(t,"charImage");

	t->LoadControlSettings("resource/ui/MenuCharPanel.res");

	// set up modules tab
	t = m_pTabs[TAB_MODULES];
	m_pModuleTree = new ModuleTree(t,this,"moduleTree");
	m_pModulePanel = new ModulePanel(t,this,"modulePanel");
	t->LoadControlSettings("resource/ui/MenuModuleTab.res");
	
	// set up weapons tab
	m_pTabButtons[TAB_WEAPONS]->SetVisible(false);

	// set up talent tab
	m_pTabButtons[TAB_TALENTS]->SetVisible(false);

	//set up MP3 Player tab
	//m_pTabButtons[TAB_MP3PLAYER]->SetVisible(false);

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);
}

CVortexMenu::~CVortexMenu()
{

}

void SetSizeAndPos( vgui::Panel *pElement, int x, int y, int w, int h )
{
	pElement->SetPos( scheme()->GetProportionalScaledValue(x), scheme()->GetProportionalScaledValue(y) );
	pElement->SetSize( scheme()->GetProportionalScaledValue(w), scheme()->GetProportionalScaledValue(h) );
}

void CVortexMenu::Update()
{
	m_flPredictionExpiryTime = m_flPrediction2ExpiryTime = 0;

	int x = 0, y = 50;
	int w = 480, h = 360; // size of this menu
	h -= y;

	for ( int i=0; i<NUM_TABS; i++ )
	{
		m_pTabs[i]->SetVisible(false);
		SetSizeAndPos(m_pTabs[i],x,y,w,h);
	}

	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return;

	m_pInfoLabel->SetText( VarArgs( "Lvl%i %s", pPlayer->GetLevel(), pPlayer->GetPlayerName() ));
	m_pTabButtons[m_iActiveTab]->DoClick();
	SwitchToTab(m_iActiveTab);

	m_pBtnVoteMode->SetEnabled( !HL2MPRules()->IsInVote() );	
	m_pBtnNomMap->SetEnabled( !HL2MPRules()->IsInVote() );

	m_pBtnNomMap->SetVisible( mc_vote_map_enabled.GetInt() >= 2 );
	m_pBtnVoteMode->SetVisible( mc_vote_gamemode_enabled.GetInt() >= 2 );

	m_pMapLabel->SetText( engine->GetLevelName()+5 );
	m_pGameModeLabel->SetText(VarArgs("#mode%i",mc_gamemode.GetInt()));	
	m_pModeDescLabel->SetText(VarArgs("#modedesc%i",mc_gamemode.GetInt()));	

	if ( HL2MPRules()->IsTeamplay() )
	{
		int currentFaction = C_HL2MP_Player::GetLocalHL2MPPlayer()->GetFaction();
		int other1, other2;
		switch ( currentFaction )
		{
		case FACTION_COMBINE:
			other1 = FACTION_RESISTANCE; other2 = FACTION_APERTURE; break;
		case FACTION_RESISTANCE:
			other1 = FACTION_COMBINE; other2 = FACTION_APERTURE; break;
		case FACTION_APERTURE:
			other1 = FACTION_COMBINE; other2 = FACTION_RESISTANCE; break;
		default:
			return;
		}

		vgui::Button *pOtherButton;
		bool bAllowed = HL2MPRules()->IsFactionChangeAllowed(currentFaction,other1);
		if ( bAllowed )
		{
			m_pBtnChangeFaction1->SetVisible(true);
			m_pBtnChangeFaction1->SetCommand(VarArgs("faction %i", other1));
			m_pBtnChangeFaction1->SetText(VarArgs("#change_faction_%i", other1));
			pOtherButton = m_pBtnChangeFaction2;
		}
		else
		{
			m_pBtnChangeFaction1->SetVisible(false);
			pOtherButton = m_pBtnChangeFaction1;
			m_pBtnChangeFaction2->SetVisible(false);
		}

		// if the first change isn't allowed, the first button will be used to show the second option, if that makes sense
		bAllowed = HL2MPRules()->IsFactionChangeAllowed(currentFaction,other2);
		if ( bAllowed )
		{
			pOtherButton->SetVisible(bAllowed);
			pOtherButton->SetCommand(VarArgs("faction %i", other2));
			pOtherButton->SetText(VarArgs("#change_faction_%i", other2));
		}
		else
			pOtherButton->SetVisible(false);
	}
	else
	{
		m_pBtnChangeFaction1->SetVisible(false);
		m_pBtnChangeFaction2->SetVisible(false);
	}

	m_pModuleTree->Update(); // otherwise initially all buttons at 0,0, as game screen res not specified
	m_pModulePanel->RecalculateLevel();
	m_pModulePanel->Update();
}

void CVortexMenu::OnThink()
{
	
}

void CVortexMenu::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return;

	if ( bShow )
	{
		// hide crosshair & experience panel
		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
//		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("EnterMenu");

		// clear hint panel
		CLEAR_HINT();

		Activate();
//		Update();  // this is already called when showing

		// get key binding if shown
		if ( m_iBuyModulesKey == BUTTON_CODE_INVALID ) // you need to lookup the jump key AFTER the engine has loaded
		{
			m_iBuyModulesKey = gameuifuncs->GetButtonCodeForBind( "buy" );
		}

		m_bBoughtActiveModule = m_bBoughtToggledModule = m_bBoughtPassiveModule = false;
	}
	else
	{
		// show crosshair & experience panel
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
//		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ExitMenu");

		// show hints if we just bought stuff - first count how many abilities of each type the player has
		if ( m_bBoughtActiveModule || m_bBoughtToggledModule || m_bBoughtPassiveModule )
		{
			int numActiveModules = 0, numToggledModules = 0, /*numToggledHeldModules = 0,*/ numPassiveModules = 0;
			for ( int i=0; i<NUM_MODULES; i++ )
			{
				Module *a = GetModule(i);
				if ( pPlayer->HasModule(a) )
				{
					/*if ( a->IsToggledByHoldingKey() )
						numToggledHeldModules ++;
					else*/ if ( a->IsToggled() )
						numToggledModules ++;
					else if ( a->AutoStarts() )
						numPassiveModules ++;
					else
						numActiveModules ++;
				}
			}

			if ( m_bBoughtActiveModule && numActiveModules == 1 )
				SHOW_HINT("Hint_BoughtActive") // show active hint
			else if ( m_bBoughtToggledModule && numToggledModules == 1 )
				SHOW_HINT("Hint_BoughtToggled") // show toggled hint
			else if ( m_bBoughtPassiveModule && numPassiveModules == 1 )
				SHOW_HINT("Hint_BoughtPassive") // show passive hint
			else if ( (m_bBoughtActiveModule || m_bBoughtToggledModule) && numActiveModules + numToggledModules == 2 )
				SHOW_HINT("Hint_MultipleModules") // show multiple hint
		}
	}

//	SetPaintBackgroundEnabled(bShow);
//	SetPaintBorderEnabled(bShow);
	SetMouseInputEnabled(bShow);
	SetKeyBoardInputEnabled(false);
	SetVisible(bShow);
//	m_pViewPort->ShowBackGround( bShow ); // whatever this was, it kicked in when keyboard was turned off

	if ( !bShow )
		Close();
}

void CVortexMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CVortexMenu::OnKeyCodePressed(vgui::KeyCode code)
{
	if ( ( m_iBuyModulesKey != BUTTON_CODE_INVALID && m_iBuyModulesKey == code ) || code == KEY_ESCAPE )
	{
		ShowPanel(false);
		return;
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

void CVortexMenu::OnCommand( const char *command )
{
	if ( FStrEq( command, "ok" ) || FStrEq( command, "close" ) )
	{
		ShowPanel(false);
	}

	BaseClass::OnCommand(command);
}

void MenuTab::OnCommand( const char *command )
{
	if ( FStrEq( command, "votegamemode" ) )
	{
		if ( HL2MPRules()->IsInVote() || mc_vote_gamemode_enabled.GetInt() < 2 )
			return;

		m_pParent->ShowPanel(false);
		//if ( mc_vote_gamemode_enabled.GetInt() > 2 )
			gViewPortInterface->ShowPanel( PANEL_PICKMODE, true );
		//else
			//engine->ClientCmd("startvote");
		return;
	}
	else if ( FStrEq( command, "nommap" ) )
	{
		if ( !HL2MPRules()->IsInVote() && mc_vote_map_enabled.GetInt() >= 2 )
		{
			m_pParent->ShowPanel(false);
			gViewPortInterface->ShowPanel( PANEL_PICKMAP, true );
			return;
		}
	}
	else if ( FStrEq( command, "pickchar" ) || FStrEq( command, "logoff" ) || Q_strstr(command, "faction ") != NULL )
	{
		m_pParent->ShowPanel(false);
		engine->ClientCmd(command);
	}

	EditablePanel::OnCommand(command);
}

void CVortexMenu::SwitchToModule(int module)
{
	m_pModulePanel->Init(module);
}

// called by one of the tab buttons, this shows its panel,
// disables all the other buttons & hides their panels
void CVortexMenu::SwitchToTab(int i)
{
	if ( i < 0 || i >= NUM_TABS )
		return;

	for ( int j=0; j<NUM_TABS; j++ )
		if ( i != j )
		{
			m_pTabButtons[j]->Disable();
			m_pTabs[j]->SetVisible(false);
		}

	m_pTabs[i]->SetVisible(true);
	m_iActiveTab = i;
	UpdateTipLabel();
}

void CVortexMenu::UpdateTipLabel()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return;
	
	wchar_t tipText[96];

	if ( m_iActiveTab == TAB_GAME )
		g_pVGuiLocalize->ConstructString( tipText, sizeof( tipText ), g_pVGuiLocalize->Find( "#tip_game" ), 0 );
	else if ( m_iActiveTab == TAB_CHARACTER )
		g_pVGuiLocalize->ConstructString( tipText, sizeof( tipText ), g_pVGuiLocalize->Find( "#tip_character" ), 0 );
	else if ( m_iActiveTab == TAB_MODULES )
	{
		wchar_t *ap = new wchar_t[8];
		_snwprintf(ap,8,L"%i",GetPlayerAP());
		g_pVGuiLocalize->ConstructString( tipText, sizeof( tipText ), g_pVGuiLocalize->Find( "#tip_abilities" ), 1, ap );
	}
	else if ( m_iActiveTab == TAB_TALENTS )
		g_pVGuiLocalize->ConstructString( tipText, sizeof( tipText ), g_pVGuiLocalize->Find( "#tip_talents" ), 0 );
	else if ( m_iActiveTab == TAB_WEAPONS )
		g_pVGuiLocalize->ConstructString( tipText, sizeof( tipText ), g_pVGuiLocalize->Find( "#tip_weapons" ), 0 );

	m_pTipLabel->SetText(tipText);
}

/*	When the player buys an ability, the buy is sent to the server, and then the AP are
	updated and sent back to us. This takes time, and we want to redraw right away.
	So when they buy, we 'predict' what their new value is, this is considered valid only
	for a short time, as after that, the 'correct' value (almost certainly identical) will
	have been received
*/
#define PREDICTION_VALIDITY_TIME 0.75f
int CVortexMenu::GetPlayerAP()
{
	if ( m_flPredictionExpiryTime > gpGlobals->curtime )
		return m_iPredictedAP;

	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return 0;

	return pPlayer->GetAP();
}

void CVortexMenu::BoughtModule(Module *a, bool isNew)
{
	if ( isNew )
	{
		if ( a->AutoStarts() )
			m_bBoughtPassiveModule = true;
		else if ( a->IsToggled() )
			m_bBoughtToggledModule = true;
		else
			m_bBoughtActiveModule = true;
	}
	PredictAPChange(a->GetUpgradeCost());
}

// i is a subtraction, its negative
void CVortexMenu::PredictAPChange(int i)
{
	if ( m_flPredictionExpiryTime > gpGlobals->curtime ) // we're "in a prediction"
	{
		m_iPredictedAP -= i;
	}
	else
	{
		C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
		if ( !pPlayer )
			return;
		m_iPredictedAP = pPlayer->GetAP() - i;
	}

	m_flPredictionExpiryTime = gpGlobals->curtime + PREDICTION_VALIDITY_TIME;
	UpdateTipLabel();
}

// ===================================================================

void TabButton::DoClick()
{
	if ( IsSelected() )
		return; // if already selected, do nothing!

	Enable();
	FireActionSignal();

	Repaint();
}

void TabButton::Enable()
{
	ForceDepressed(true);
	SetSelected(true);
	
	pMyParent->SwitchToTab(m_iNum);
}

void TabButton::Disable()
{
	ForceDepressed(false);
	SetSelected(false);
}

void CategoryButton::Enable()
{
	ForceDepressed(true);
	SetSelected(true);
}

// ===================================================================


void ModuleButton::DoClick()
{
	if ( IsSelected() )
		return; // if already selected, do nothing!

	Enable();
	FireActionSignal();

	Repaint();
}

void ModuleButton::Enable()
{
	ForceDepressed(true);
	SetSelected(true);
	
	pMyParent->SetSelection(this);
}

void ModuleButton::Disable()
{
	ForceDepressed(false);
	SetSelected(false);
}

// ===================================================================
#define FULL_WIDTH 96
#define BUTTON_WIDTH 88
#define BUTTON_HEIGHT 28
#define MOD_BUTTON_XPOS FULL_WIDTH - BUTTON_WIDTH
ModuleTree::ModuleTree( vgui::Panel *pParent, CVortexMenu *pMenu, const char *pName )
	: ScrollPanel( pParent, pName, true )
{
	m_pMenu = pMenu;
	m_iSelectedCategory = -1;
	SetPaintBackgroundEnabled(false);

	int pos = 0;
	for ( int i=0; i<NUM_MODULE_CATEGORIES; i++ )
	{
		m_pCategoryButtons[i] = new vgui::Button(this, "catbtn", VarArgs("#modcat%i", i+1));
		m_pCategoryButtons[i]->SetSize( scheme()->GetProportionalScaledValue(BUTTON_WIDTH), scheme()->GetProportionalScaledValue(BUTTON_HEIGHT) );
		m_pCategoryButtons[i]->SetCommand(VarArgs("cat%i",i));
		m_pCategoryButtons[i]->AddActionSignalTarget(this);
		m_pButtons[pos] = m_pCategoryButtons[i];

		pos++;
		for ( int j=0; j<NUM_MODULES; j++ )
		{
			Module *m = GetModule(j);
			if ( m->GetCategory() == i && m->IsPurchasable() )
			{
				m_pItemButtons[j] = new ModuleButton(this, "modbtn", m->GetDisplayName(), j);
				m_pItemButtons[j]->SetSize( scheme()->GetProportionalScaledValue(BUTTON_WIDTH), scheme()->GetProportionalScaledValue(BUTTON_HEIGHT) );
				m_pItemButtons[j]->AddActionSignalTarget(this);
				m_pItemButtons[j]->SetCommand(VarArgs("mod%i",m->GetModuleIndex()));
				m_pButtons[pos] = m_pItemButtons[j];
				pos++;
			}
		}
	}

	Update();
}

void ModuleTree::Update()
{
	int catPos = -1, xPos = scheme()->GetProportionalScaledValue(MOD_BUTTON_XPOS), yPos = 0, interval = scheme()->GetProportionalScaledValue(BUTTON_HEIGHT);
	for ( int btnPos = 0; btnPos < NUM_MODULES + NUM_MODULE_CATEGORIES; btnPos ++ )
	{
		if ( catPos < (NUM_MODULE_CATEGORIES-1) && m_pCategoryButtons[catPos+1] == m_pButtons[btnPos] )
		{
			catPos++;
			m_pButtons[btnPos]->SetPos(0, yPos );
			yPos += interval;
			continue;
		}
		bool expanded = m_iSelectedCategory == catPos && GetModule(static_cast<ModuleButton*>(m_pButtons[btnPos])->GetModuleIndex())->IsPurchasable();
		m_pButtons[btnPos]->SetVisible(expanded);
		if ( expanded )
		{
			m_pButtons[btnPos]->SetPos(xPos, yPos );
			yPos += interval;
		}
		else
			m_pButtons[btnPos]->SetPos( 0,0 );
	}

	SetMaxScrollValue(yPos);
	ScrollPanel::Update();
}

void ModuleTree::ToggleCategory(int category)
{
	if ( m_iSelectedCategory == category )
		m_iSelectedCategory = -1;
	else
		m_iSelectedCategory = category;
	Update();
}

void ModuleTree::SetSelection(vgui::Button *b)
{
	for ( int i=0; i<NUM_MODULES; i++ )
		if ( m_pItemButtons[i] != b )
			m_pItemButtons[i]->Disable();
}

void ModuleTree::OnCommand( char const *cmd )
{
	if ( !Q_strncmp( cmd, "cat", 3 ) )
		ToggleCategory(atoi(cmd+3));
	else if ( !Q_strncmp( cmd, "mod", 3 ) )
		m_pMenu->SwitchToModule(atoi(cmd+3));
}

// ===================================================================

ModulePanel::ModulePanel( vgui::Panel *pParent, CVortexMenu *pMenu, const char *pName )
	: ScrollPanel( pParent, pName, true )
{
	m_pMenu = pMenu;
	m_iModule = -1;
	level = 0;
	m_pDataTable = 0;

	m_pNameLabel = new vgui::Label(this,"nameLabel","");
	m_pLevelLabel = new vgui::Label(this,"levelLabel","");
	m_pTypeLabel = new vgui::Label(this,"typeLabel","");

	m_pPowerLabel = new vgui::Label(this,"powerLabel","");
	m_pCastTimeLabel = new vgui::Label(this,"casttimeLabel","");
	m_pCooldownLabel = new vgui::Label(this,"cooldownLabel","");

	m_pDescriptionLabel = new vgui::Label(this,"descriptionLabel","");

	m_pModuleIcon = new vgui::ImagePanel(this,"abilityIcon");

	m_pImageBackground = new vgui::Button(this,"iconBackground","",this,"bg");
	m_pBuyButton = new vgui::Button(this,"buyButton","Buy",this,"buy");
	LoadControlSettings("resource/ui/ModulePanel.res");
	//m_pDescriptionLabel->SetAutoResize(vgui::Panel::PIN_TOPLEFT, AUTORESIZE_DOWN, 0, 0, 0, 0);
	SetPaintBackgroundEnabled(false);
}
/*
void ModulePanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	vgui::Panel::ApplySchemeSettings(pScheme);
	_dullbg  = pScheme->GetColor("Panel.DullSolidBg", Color( 32,32,32,220 ) ); 
	_brightbg = pScheme->GetColor("Panel.BrightSolidBg", Color( 96,96,96,220 ) ); 
}
*/
void ModulePanel::OnCommand( char const *cmd )
{
	if ( !Q_stricmp( cmd, "buy" ) )
	{
		C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
		Module *a = GetModule(m_iModule);
		if ( !pPlayer )
			return; // who the hell would have sent the command anyway?
		
		if ( a->GetUpgradeCost() > m_pMenu->GetPlayerAP() )
			return; // can't afford it, how the hell you clickin this this stuff?

		engine->ClientCmd(VarArgs("buy %s",a->GetCmdName()));
		
	
		// predict the upgrade, so that we can update the Level label instantly
		//level = pPlayer->GetModuleLevel(a);
		m_pMenu->BoughtModule(a, level == 1);

		if ( pPlayer && level < a->GetMaxLevel() )
			level ++;
		Update();
	}
}

void ModulePanel::Init(int module)
{
	Module *a = GetModule(module);
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !a || !pPlayer )
		return;
	m_iModule = module;
	level = pPlayer->GetModuleLevel(a);
	
	int x,y; // for calculating data table positions & scroll height
	m_pDescriptionLabel->GetPos(x,y);
	y += m_pDescriptionLabel->GetTall()+16;
	
	// set up the logo
	m_pModuleIcon->SetImage(a->GetIconName());
	m_pModuleIcon->SetShouldScaleImage(true);
	//m_pModuleIcon->SetSize( scheme()->GetProportionalScaledValue(48), scheme()->GetProportionalScaledValue(48) );

	// set up the ability name & type labels
	m_pNameLabel->SetText(a->GetDisplayName());
	m_pTypeLabel->SetText(VarArgs("%s module", a->GetDisplayType()));

	// set up the power, cast time & cooldown labels
	m_pPowerLabel->SetText(VarArgs("Drain: %.0f aux",GetModule(m_iModule)->GetAuxDrain(pPlayer, level==0?1:level)));

	if ( m_pDataTable != NULL )
	{
		delete m_pDataTable;
		m_pDataTable = NULL;
	}

	if ( a->GetMaxLevel() > 1 )
	{
		x = vgui::scheme()->GetProportionalScaledValue(8);

		int maxLevel = a->GetMaxLevel();
		int numCols = a->ShouldShowLevel0OnDataTables() ? maxLevel + 1 : maxLevel;
		int levelOffset = a->ShouldShowLevel0OnDataTables() ? 0 : 1;
		
		wchar_t temp[16];

		// filter out invariant rows ... if the value is constant, don't bother showing it
		int numRows = 0;
		for ( int i=1; i<MAX_ROWS; i++ )
		{
			_snwprintf(temp, sizeof(temp), a->GetDataTableValue(i,maxLevel));
			if ( wcscmp(a->GetDataTableValue(i,1), temp) != 0 )
				numRows++;
		}
		
		if ( numRows > 0 )
		{
			m_pDataTable = new DataTable(this, numCols, numRows+1);
			m_pDataTable->SetRowName(0,L"Level");
			for ( int i=0; i<numCols; i++ )
			{//	populate level numbers
				_snwprintf(temp, sizeof(temp), L"%i", i+levelOffset);
				m_pDataTable->SetValue(i,0,temp);
			}
			
			int row = 1;
			for ( int i=1; i<MAX_ROWS; i++ )
			{
				// filter out invariant rows ... if the value is constant, don't bother showing it
				_snwprintf(temp, sizeof(temp), a->GetDataTableValue(i,maxLevel));
				if ( wcscmp(a->GetDataTableValue(i,1), temp) != 0 )
				{// level 1 & max level values differ, so give this parameter a row in the data table
					m_pDataTable->SetRowName(row,a->GetDataTableRowName(i));
					for ( int j=0; j<numCols; j++ )
						m_pDataTable->SetValue(j,row,a->GetDataTableValue(i,j+levelOffset));
					row++;
				}
			}
			
			m_pDataTable->SetPos(x,y);
			y += m_pDataTable->GetTall() + 4;
		}
	}
	
	// now need to adjust the max scroll value based on the length of the description label (or the data tables, if there are any)
	SetMaxScrollValue(y);
	ScrollPanel::Update();

	Update();
}

void ModulePanel::RecalculateLevel()
{
	Module *a = GetModule(m_iModule);
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !a || !pPlayer )
		level = 0;
	else
		level = pPlayer->GetModuleLevel(a);
}

void ModulePanel::Update()
{
	bool hasModule = m_iModule != -1;
	m_pBuyButton->SetVisible(hasModule);
	m_pModuleIcon->SetVisible(hasModule);
	m_pNameLabel->SetVisible(hasModule);
	m_pTypeLabel->SetVisible(hasModule);
	m_pPowerLabel->SetVisible(hasModule);
	m_pCastTimeLabel->SetVisible(hasModule);
	m_pLevelLabel->SetVisible(hasModule);
	m_pCooldownLabel->SetVisible(hasModule);
	m_pModuleIcon->SetVisible(hasModule);
	m_pImageBackground->SetVisible(hasModule);

	if ( !hasModule )
	{// no module, special handling
		m_pDescriptionLabel->SetText("#nomodule_desc");
		SetMaxScrollValue(128);
		ScrollPanel::Update();
		return;
	}

	Module *a = GetModule(m_iModule);
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return;
	
	// set up the description label
	//if ( level == 0 )
		m_pDescriptionLabel->SetText(a->GetDescription(level));
	/*else if ( level == a->GetMaxLevel() )
	{// show Current level: & desc
		wchar_t temp[1024+64];
		_snwprintf(temp, sizeof(temp), L"Current level:\n%s", a->GetDescription(level));
		m_pDescriptionLabel->SetText(temp);
	}
	else
	{// show Current level & Next level, & desc
		wchar_t temp[1024*2+64*2], temp2[1024];
		_snwprintf(temp2, sizeof(temp2), a->GetDescription(level+1)); // because GetDescription always puts it in the same block, have to save one before getting the other
		_snwprintf(temp, sizeof(temp), L"Current level:\n%s\n\nNext level:\n%s", a->GetDescription(level), temp2);
		m_pDescriptionLabel->SetText(temp);
	}*/


	// and some stuff we do in each case
	if ( m_pDataTable != NULL )
		m_pDataTable->Update(a->ShouldShowLevel0OnDataTables() ? level : level-1);

	if ( a->ShouldShowCastTime() )
	{
		m_pCastTimeLabel->SetText(VarArgs("Cast time: %.1fs",a->GetCastTime(level)));
		m_pCastTimeLabel->SetVisible(true);
	}
	else
		m_pCastTimeLabel->SetVisible(false);

	// set up the level label
	m_pLevelLabel->SetText(VarArgs("Level: %i / %i",level, a->GetMaxLevel()));
	//SetBgColor(_brightbg);

	if ( a->ShouldShowCooldown() )
	{
		m_pCooldownLabel->SetText(VarArgs("Cooldown: %.0fs",a->GetCooldown(level>0 ? level : 1)));
		m_pCooldownLabel->SetVisible(true);
	}
	else
		m_pCooldownLabel->SetVisible(false);


	// set up the upgrade button
	m_pBuyButton->SetEnabled( level != a->GetMaxLevel() && m_pMenu->GetPlayerAP() >= a->GetUpgradeCost() );
	if ( level == 0 )
		m_pBuyButton->SetText(VarArgs("Buy\n%i point%s",a->GetUpgradeCost(),a->GetUpgradeCost()>1 ? "s" : ""));
	else if ( level == a->GetMaxLevel() )
		m_pBuyButton->SetText("Fully\nUpgraded");
	else
		m_pBuyButton->SetText(VarArgs("Upgrade\n%i point%s",a->GetUpgradeCost(),a->GetUpgradeCost()>1 ? "s" : ""));
	m_pBuyButton->SetContentAlignment(Label::a_center); // this seems to need updating
}


char duration[128];
const char *FormatDuration(float time)
{
	int days = (int)(time / 84600);
	time -= days * 84600;
	int hours = (int)(time / 3600);
	time -= hours * 3600;
	int minutes = (int)(time / 60);
	time -= minutes * 60;
	int seconds = (int)(time + 0.5f);

	if ( days > 0 )
		Q_snprintf(duration, sizeof(duration), "%i %s, %i %s, %i %s, %i %s",
				   days, days == 1 ? "day" : "days",
				   hours, hours == 1 ? "hour" : "hours",
				   minutes, minutes == 1 ? "minute" : "minutes",
				   seconds, seconds == 1 ? "second" : "seconds"
				   );
	else if ( hours > 0 )
		Q_snprintf(duration, sizeof(duration), "%i %s, %i %s, %i %s",
				   hours, hours == 1 ? "hour" : "hours",
				   minutes, minutes == 1 ? "minute" : "minutes",
				   seconds, seconds == 1 ? "second" : "seconds"
				   );
	else if ( minutes > 0 )
		Q_snprintf(duration, sizeof(duration), "%i %s, %i %s",
				   minutes, minutes == 1 ? "minute" : "minutes",
				   seconds, seconds == 1 ? "second" : "seconds"
				   );
	else
		Q_snprintf(duration, sizeof(duration), "%i %s",
				   seconds, seconds == 1 ? "second" : "seconds"
				   );

	return duration;
}

// ========================================================================
#define MAT_DIR		"playermodels/"

void CVortexMenu::UpdateCharacterInfo(char *model, char *created, int playerKills, int playerDeaths, int monsterKills, int monsterDeaths, int numSprees, int numSpreeWars, int biggestSpree, float playTime)
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return;

	m_pCharNameLabel->SetText(pPlayer->GetPlayerName());
	m_pCharLevelLabel->SetText(VarArgs("Level: %i",pPlayer->GetLevel()));
	m_pCharExpLabel->SetText(VarArgs("%i total experience",pPlayer->GetTotalExp()));
	m_pCharGameExpLabel->SetText(VarArgs("%i experience this game",pPlayer->GetGameExp()));
	m_pCharCreatedLabel->SetText(VarArgs("Created at %s",created));

	m_pPlayerKillsLabel->SetText(VarArgs("Player kills: %i", playerKills));
	m_pPlayerDeathsLabel->SetText(VarArgs("Player deaths: %i", playerDeaths));
	m_pMonsterKillsLabel->SetText(VarArgs("Monster kills: %i", monsterKills));
	m_pMonsterDeathsLabel->SetText(VarArgs("Monster deaths: %i", monsterDeaths));
	
	m_pSpreesLabel->SetText(VarArgs("Killing sprees: %i", numSprees));
	m_pSpreeWarsLabel->SetText(VarArgs("Spree wars: %i", numSpreeWars));
	m_pBiggestSpreeLabel->SetText(VarArgs("Biggest spree: %i", biggestSpree));
	m_pPlayTimeLabel->SetText(VarArgs("Play time: %s", FormatDuration(playTime)));
	
	char m_szMaterial[MODEL_LENGTH];
	Q_snprintf(m_szMaterial,sizeof(m_szMaterial),VarArgs("%s%s",MAT_DIR,model));
	m_pCharImage->setTexture( m_szMaterial );
}