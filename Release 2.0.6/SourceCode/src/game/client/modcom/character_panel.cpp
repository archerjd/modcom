#include "cbase.h"
#include "character_panel.h"
#include "vgui/ILocalize.h"
#include "filesystem.h"

#include "c_hl2mp_player.h"
#include "msg_panel.h"
#include "confirm_panel.h"

#include "hud_modules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAT_DIR		"playermodels/"

CPickCharPanel::CPickCharPanel(IViewPort *pViewPort) : Frame(NULL, PANEL_PICKCHAR )
{
	m_pViewPort = pViewPort;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	m_pNewCharButton = new vgui::Button(this,"btnNewChar","");
	m_pBodyLabel1 = new vgui::Label(this,"bodyLabel1","");
	m_pBodyLabel2 = new vgui::Label(this,"bodyLabel2","");

	m_pButtonPrev = new vgui::Button(this,"btnPrev","");
	m_pButtonNext = new vgui::Button(this,"btnNext","");

	for ( int i=0; i<CHARS_PER_PAGE; i++ )
	{
		m_pCharButtons[i] = new vgui::Button(this,VarArgs("btnChar%i",i+1),"");
		m_pCharImages[i] = new ButtonImagePanel( this, VarArgs("imgChar%i",i+1), m_pCharButtons[i] );
	}

	m_pDeleteBtn = new vgui::ToggleButton(this,"btnDelete","");

	LoadControlSettings("resource/ui/PickCharPanel.res");

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

//	m_iAwaitDelete = 0;

	ResetCharInfo();
}

CPickCharPanel::~CPickCharPanel()
{

}

extern wchar_t *toStr(int i);

void CPickCharPanel::Update()
{
	// make the hud forget any abilities it has selected
	GetModuleHud()->Reset();

	// if we had them confirming a delete, and they said yes, clear the button
	// (the command will have been sent already, but this is the only way this panel can tell)
/*	if ( m_iAwaitDelete > 0 && CConfPanel::GetLastConfirm() )
	{// so slide them up to keep the gap on the end
//		Msg("Deletion detected for slot %i, updating\n",m_iAwaitDelete);
		m_iIterator --;
		char szText[128];
		for ( int i=m_iAwaitDelete-1; i<CHARS_PER_PAGE-1; i++ )
		{
			m_pCharButtons[i+1]->GetText( szText, sizeof(szText) );
			m_pCharButtons[i]->SetText(szText);
			Q_snprintf(m_szMaterials[i],sizeof(m_szMaterials[i]),m_szMaterials[i+1]);
			m_pCharImages[i]->setTexture( m_szMaterials[i] );
			m_iCharacterIDs[i] = m_iCharacterIDs[i+1];

			m_pCharImages[i]->AddActionSignalTarget(m_pCharButtons[i]);
		}
		m_iCharacterIDs[CHARS_PER_PAGE-1] = -1;
		m_pCharButtons[CHARS_PER_PAGE-1]->SetText("#click_for_new_char");
		m_pCharImages[CHARS_PER_PAGE-1]->SetVisible(false);
		Q_snprintf(m_szMaterials[CHARS_PER_PAGE-1],sizeof(m_szMaterials[CHARS_PER_PAGE-1]),"");
	}
	m_iAwaitDelete = 0;*/
}

void CPickCharPanel::ShowPanel( bool bShow )
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();
	}

//	SetPaintBackgroundEnabled(bShow);
//	SetPaintBorderEnabled(bShow);
	SetMouseInputEnabled(bShow);
	SetVisible(bShow);
	m_pViewPort->ShowBackGround( bShow );

	if ( !bShow )
	{
		Close();
	}
}

void CPickCharPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CPickCharPanel::OnKeyCodePressed(vgui::KeyCode code)
{
/*	if ( ( m_iBuyModulesKey != BUTTON_CODE_INVALID && m_iBuyModulesKey == code ) || code == KEY_ESCAPE )
	{
		ShowPanel(false);
		return;
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}*/
	BaseClass::OnKeyCodePressed( code );
}


void CPickCharPanel::OnCommand( const char *command )
{
	if ( FStrEq( command, "newchar" ) )
	{
		ShowPanel(false);
		gViewPortInterface->ShowPanel( PANEL_MAKECHAR, true );
		return;
	}
	else if ( FStrEq( command, "prev" ) )
	{
		engine->ClientCmd(VarArgs("pickchar %i",m_iPageNumber-2));
		return;
	}
	else if ( FStrEq( command, "next" ) )
	{
		engine->ClientCmd(VarArgs("pickchar %i",m_iPageNumber));
		return;
	}

	for ( int i=1; i<=CHARS_PER_PAGE; i++ )
		if ( FStrEq( command, VarArgs("char%i",i) ) )
		{
			if ( m_iIterator < i )
			{// create a character, regardless of delete button state
				m_pCharButtons[i-1]->ForceDepressed(false);
				ShowPanel(false);
				gViewPortInterface->ShowPanel( PANEL_MAKECHAR, true );
			}
			else if ( !m_pDeleteBtn->IsSelected() )
			{// select a character
				m_pCharButtons[i-1]->ForceDepressed(false);
				ShowPanel(false);
				engine->ClientCmd(VarArgs("use_char %i",m_iCharacterIDs[i-1]));
				C_HL2MP_Player::GetLocalHL2MPPlayer()->ResetSessionExp();
			}
			else
			{// show delete confirmation dialog
				m_pDeleteBtn->DoClick(); // clear this
				ShowPanel(false);
				CConfPanel::SetYesAction(VarArgs("del_char %i\npickchar %i",m_iCharacterIDs[i-1], m_iPageNumber-1));
				//CConfPanel::SetYesPanel(PANEL_PICKCHAR);
				CConfPanel::SetNoPanel(PANEL_PICKCHAR);
				CConfPanel::ShowMessage("#delete_char_title","#delete_char_body");
				//m_iAwaitDelete = i;
			}
		}

	if ( FStrEq( command, "logoff" ) )
	{
		ShowPanel(false);
		engine->ClientCmd("logoff");
	}
	else if ( FStrEq( command, "changepass" ) )
	{
		ShowPanel(false);
	}
	else if ( FStrEq( command, "spec" ) )
	{
		ShowPanel(false);
		engine->ClientCmd("spectate");
	}
	else if ( FStrEq( command, "delete" ) )
	{
		ShowPanel(false);
	}

	BaseClass::OnCommand(command);
}


void CPickCharPanel::ResetCharInfo()
{
	for ( int i=0; i<CHARS_PER_PAGE; i++ )
	{
		m_pCharButtons[i]->SetText("#click_for_new_char");
		m_pCharImages[i]->SetVisible(false);
		Q_snprintf(m_szMaterials[i],sizeof(m_szMaterials[i]),"");
		m_pCharButtons[i]->SetZPos(-1);
		m_pCharImages[i]->SetZPos(0);
	}
	m_iIterator = 0;

	m_pButtonPrev->SetZPos(5);
	m_pButtonNext->SetZPos(5);
}

void CPickCharPanel::Setup(int page, int totNumChars)
{
	m_iTotNumChars = totNumChars; m_iPageNumber = page;
	m_pNewCharButton->SetEnabled(totNumChars != MAX_CHARS);

	int numPages = (float)m_iTotNumChars/(float)CHARS_PER_PAGE + 0.9999f;
	m_pButtonPrev->SetVisible( page > 1 );
	m_pButtonNext->SetVisible( page < numPages );

	wchar_t bodyText[128], totChars[16], maxChars[16], numPage[16], maxPage[16];

	_snwprintf(totChars, sizeof(totChars), L"%i", m_iTotNumChars);
	_snwprintf(maxChars, sizeof(maxChars), L"%i", MAX_CHARS);
	_snwprintf(numPage, sizeof(numPage), L"%i", page);
	_snwprintf(maxPage, sizeof(maxPage), L"%i", numPages);

	g_pVGuiLocalize->ConstructString( bodyText, sizeof(bodyText), g_pVGuiLocalize->Find( "#pickchar_body1" ), 2, totChars, maxChars);
	m_pBodyLabel1->SetText(bodyText);
	g_pVGuiLocalize->ConstructString( bodyText, sizeof(bodyText), g_pVGuiLocalize->Find( "#pickchar_body2" ), 2, numPage, maxPage);
	m_pBodyLabel2->SetText(bodyText);
}

void CPickCharPanel::AddCharacterInfo(int id,const char *name,const char *model,int iLevel, const char *created, const char *lastAccess)
{
	if ( m_iIterator >= CHARS_PER_PAGE )
		return; // done them all, gone too far

	char btnText[128];
	Q_snprintf(btnText,sizeof(btnText),"%s\nLevel %i\nCreated: %s\nLast active: %s",name,iLevel,created, lastAccess);
	m_pCharButtons[m_iIterator]->SetText(btnText);
	Q_snprintf(m_szMaterials[m_iIterator],sizeof(m_szMaterials[m_iIterator]),VarArgs("%s%s",MAT_DIR,model));
	m_pCharImages[m_iIterator]->setTexture(m_szMaterials[m_iIterator]); // dont work, but it did using the VarArgs :S
	m_pCharImages[m_iIterator]->SetVisible(true);
	m_iCharacterIDs[m_iIterator] = id;
//	Msg("Testing material buffer %s & %s\n",VarArgs("%s%s",MAT_DIR,model),m_szMaterials[m_iIterator]);
	m_iIterator ++; // next time, will do the next button
}