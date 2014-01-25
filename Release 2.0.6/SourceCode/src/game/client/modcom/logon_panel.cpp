#include "cbase.h"
#include "logon_panel.h"
#include "vgui/ILocalize.h"
#include "filesystem.h"

#include "util/md5/md5wrapper.h"

#include "tier0/icommandline.h"

#include "hud_modules.h"

#include "steam/steam_api.h"
#include "msg_panel.h"
#include "util/sqlite/dbhandler.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DB_LOGONS	"mylogons.db"

CLogonPanel::CLogonPanel(IViewPort *pViewPort) : Frame(NULL, PANEL_LOGON )
{
	m_pViewPort = pViewPort;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	m_pTitleLabel = new vgui::Label(this,"titleLabel","");
	m_pExistingButton = new TogglePairButton(this,"existingAccBtn","",false);
	m_pNewButton = new TogglePairButton(this,"newAccBtn","",true);
	m_pExistingButton->SetPartner(m_pNewButton);
	m_pNewButton->SetPartner(m_pExistingButton);

	m_bHasStoredPassword = false;
	m_pUsername = new vgui::TextEntry(this, "username");
	m_pPassword = new NotifyingTextEntry(this, "password");
	m_pConfirmPassword = new vgui::TextEntry(this, "confirmPassword");
	m_pConfirmPasswordLabel = new vgui::Label(this,"passConfLabel","");

	m_pRememberMe = new vgui::CheckButton(this,"rememberMe","");

	LoadControlSettings("resource/ui/LogonPanel.res");

	m_pUsername->SetMultiline(false);
	m_pUsername->SetMaximumCharCount(NAME_LENGTH);
	m_pPassword->SetMultiline(false);
	m_pPassword->SetTextHidden(true);
	m_pPassword->SetMaximumCharCount(NAME_LENGTH);
	m_pConfirmPassword->SetMultiline(false);
	m_pConfirmPassword->SetTextHidden(true);
	m_pConfirmPassword->SetMaximumCharCount(NAME_LENGTH);
	m_pRememberMe->SetSelected(true);

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

	ListenForGameEvent( "server_spawn" );
	Q_snprintf(m_szGameAddress,sizeof(m_szGameAddress),"");
}

CLogonPanel::~CLogonPanel()
{

}

// return a valid IP, even if the parameter given isn't.
// for listen server hosts, it returns a non-ip string
// (i keep getting "loopback") - so if this is the case,
// return 127.0.0.1
const char *ValidateIP(const char *ip)
{
	// basically, if string contains 3 dots, its an IP
	// inaccurate definition, but it should be good enough

	int num = 0;
	const char *pch;
	pch=strchr(ip,'.');
	while (pch!=NULL)
	{
		num++;
		pch=strchr(pch+1,'.');
	}

	if ( num == 3 )
		return ip;
	return "127.0.0.1";
}

void CLogonPanel::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( FStrEq(type, "server_spawn") )
	{//	read off the server name and IP
		const char *szName = event->GetString( "hostname" );
//		wchar_t titleText[128];//, serverName[96];
//		g_pVGuiLocalize->ConvertANSIToUnicode(szName,serverName,sizeof(serverName));
//		g_pVGuiLocalize->ConstructString( titleText, sizeof(titleText), g_pVGuiLocalize->Find( "#logon_title" ), 1, szName );
		m_pTitleLabel->SetText(VarArgs("Log in to %s",szName));

		const char *szAddress = ValidateIP(event->GetString( "address" ));
		Q_snprintf(m_szGameAddress,sizeof(m_szGameAddress),szAddress);
	}	
}

void CLogonPanel::Update()
{
	// make the hud forget any abilities it has selected
	GetModuleHud()->Reset();

	m_pExistingButton->Enable();
	
	// clear our entries, then if we can load remembered ones, great, if not, at least its tidy
	if ( steamapicontext->SteamFriends() )
		m_pUsername->SetText( steamapicontext->SteamFriends()->GetPersonaName() ); // by default, suggest using steam friends name as logon
	else
		m_pUsername->SetText("");
	m_pPassword->SetText("");
	m_pConfirmPassword->SetText("");

	// open the database when we open our panel
	//"The opening and/or creating of the database file is deferred until the file is actually needed"
	g_pDB = new dbHandler(DB_LOGONS);
	g_pDB->Command("CREATE TABLE IF NOT EXISTS Logons ( Address varchar, Name varchar, Pass varchar )");
	g_pDB->Command("CREATE UNIQUE INDEX IF NOT EXISTS IX_Logons ON Logons (Address)");

	// see if we can read any logon details for this server IP from our database
	dbReadResult *logon = g_pDB->ReadMultiple("select Name, Pass from Logons WHERE Address='%s'",m_szGameAddress);
	if ( logon->Count() > 0 ) // we got data
	{
		m_pUsername->SetText(logon->Element(0).text);
		m_pPassword->SetText("********");
		Q_snprintf(m_szStoredPassword, sizeof(m_szStoredPassword), logon->Element(1).text);
		m_bHasStoredPassword = true;
	}
	else
		m_bHasStoredPassword = false;
	logon->Purge();
	delete logon;

//	this fixes its refusal to forget logins, and also stops us keeping the password in memory
//	Q_snprintf(name,sizeof(name),"");
//	Q_snprintf(pass,sizeof(pass),"");
}

void CLogonPanel::ShowPanel( bool bShow )
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();
//		Update();  // this is already called when showing
	}

//	SetPaintBackgroundEnabled(bShow);
//	SetPaintBorderEnabled(bShow);
	SetMouseInputEnabled(bShow);
	SetVisible(bShow);
	m_pViewPort->ShowBackGround( bShow );

	if ( !bShow )
	{
		Close();
		delete g_pDB; // close the database when we close our panel
	}
}

void CLogonPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CLogonPanel::OnKeyCodePressed(vgui::KeyCode code)
{
	BaseClass::OnKeyCodePressed( code );
}

extern bool IsValidInputString(const char *test);
extern std::string Escape(const char *instr);

void CLogonPanel::OnCommand( const char *command )
{
	if ( FStrEq( command, "btnlogin" ) )
	{
		md5wrapper md5;
		std::string hash;
		char szUsername[MAX_DB_STRING];
		char szPassword[MAX_DB_STRING];
		char szPassConf[MAX_DB_STRING];
		m_pUsername->GetText( szUsername, sizeof( szUsername ) );
		if ( m_bHasStoredPassword )
		{
			Q_strcpy(szPassword, m_szStoredPassword);
			hash = szPassword;
		}
		else
		{
			m_pPassword->GetText( szPassword, sizeof( szPassword ) );
			hash = md5.getHashFromString(szPassword);
		}
		m_pConfirmPassword->GetText( szPassConf, sizeof( szPassConf ) );

		if ( Q_strlen(szUsername) == 0 || Q_strlen(szPassword) == 0 )
			return; // we need a username & password

//		only allow alphanumeric, space, and . , _ -
		if ( !IsValidInputString(szUsername) )
		{
			m_pUsername->SetText("");
			CMsgPanel::ShowMessage("#bad_username_title","#bad_username_body");	
			
			return;
		}

//		don't allow multiple consecutive spaces in a name, that would bork us
		const char *prev = NULL;
		const char *pch;
		pch=strchr(szUsername,' ');
		while (pch!=NULL)
		{
			if ( prev == pch-1 )
			{
				m_pUsername->SetText("");
				CMsgPanel::ShowMessage("#consecutive_spaces_title","#consecutive_spaces_body");	
				return;
			}

			prev = pch;
			pch=strchr(pch+1,' ');
		}

		if ( m_pNewButton->IsSelected() && !FStrEq(szPassword,szPassConf) )
		{// its a new account and passwords dont match - show popup and clear them
			m_pPassword->SetText("");
			m_pConfirmPassword->SetText("");

			CMsgPanel::ShowMessage("#pass_mismatch_title","#pass_mismatch_body");
			return; // don't close this window, send command, or save details!
		}

//		encrypt the password
		std::string doublehash = md5.getHashFromString(DoubleHashPassword(hash.c_str()));
		ClearStoredPasswordHash();

//		different commands for new or existing accounts
		const char *szCommand, *passHash;
		if ( m_pNewButton->IsSelected() )
		{
			szCommand = "make_logon";
			passHash = hash.c_str(); // first time, send normal hash (to save in server db)
		}
		else
		{
			szCommand = "do_logon";
			passHash = doublehash.c_str(); // other times, send double hash
		}
		engine->ClientCmd(VarArgs("%s %s %s",szCommand,szUsername,passHash));
		
		g_pDB->Command("delete from Logons WHERE Address = '%s'",m_szGameAddress); // if they don't want remembered, we'll forget them
		if ( m_pRememberMe->IsSelected() ) // save logon & password locally
			g_pDB->Command("insert into Logons (Address, Name, Pass) values ('%s', '%s', '%s')", m_szGameAddress, Escape(szUsername).c_str(), hash.c_str());
		ShowPanel(false);
	}

	BaseClass::OnCommand(command);
}

void CLogonPanel::SetConfirmVisible(bool b)
{
	m_pConfirmPassword->SetVisible(b);
	m_pConfirmPasswordLabel->SetVisible(b);

	if ( b )
	{
		m_pConfirmPassword->SetText("");
		ForgetPassword();
	}
}

void CLogonPanel::ForgetPassword()
{
	if ( !m_bHasStoredPassword )
		return;
	m_bHasStoredPassword = false;
	m_pPassword->SetText("");
}

// =================================================

void TogglePairButton::DoClick()
{
	if ( IsSelected() )
		return; // if already selected, do nothing!

	Enable();
	FireActionSignal();

	// whatever the heck this was, it started causing crashes
//	KeyValues *msg = new KeyValues("ButtonToggled");
//	msg->SetInt("state", (int)IsSelected());
//	PostActionSignal(msg);
	
	Repaint();
}

void TogglePairButton::Enable()
{
	ForceDepressed(true);
	SetSelected(true);
	
	m_pParent->SetConfirmVisible(m_bShowExtra);

	m_pPartner->Disable();
}

void TogglePairButton::Disable()
{
	ForceDepressed(false);
	SetSelected(false);
}

void NotifyingTextEntry::OnKeyCodePressed(vgui::KeyCode code)
{
	m_pParent->ForgetPassword();
}