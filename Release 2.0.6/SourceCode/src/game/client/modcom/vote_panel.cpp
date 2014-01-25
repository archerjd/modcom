#include "cbase.h"
#include <vgui/ILocalize.h>
#include "vote_panel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CVotePanel::CVotePanel(IViewPort *pViewPort) : Frame(NULL, PANEL_VOTE)
{
	m_pViewPort = pViewPort;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	m_pMessageLabel = new vgui::Label(this,"messageLabel","");

	LoadControlSettings("resource/ui/VotePanel.res");
	m_pMessageLabel->SetContentAlignment( vgui::Label::a_northwest );

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);
}

CVotePanel::~CVotePanel()
{

}

void CVotePanel::Update()
{

}

void CVotePanel::ShowPanel( bool bShow )
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
	//SetMouseInputEnabled(bShow);
	SetVisible(bShow);
	//m_pViewPort->ShowBackGround( bShow );

	if ( !bShow )
	{
		Close();
	}
}

void CVotePanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CVotePanel::OnKeyCodePressed(vgui::KeyCode code)
{
	if ( code == KEY_ESCAPE )
	{
		ShowPanel(false);
		return;
	}

	BaseClass::OnKeyCodePressed( code );
}

void CVotePanel::OnCommand( const char *command )
{
	BaseClass::OnCommand(command);
}

const char *ParseTextKeyBindings(const char *pMsg);

void CVotePanel::SetMessage(const char *pMsg)
{
	// need to do that thing where we look up bound keys and insert the key name into the text
	m_pMessageLabel->SetText(ParseTextKeyBindings(pMsg));
}

char parsedMessage[256];
const char *ParseTextKeyBindings(const char *pMsg)
{
	int sourcePos = 0, outPos = 0, sourceLength = strlen(pMsg);

// parse out the text into a label set
	while ( sourcePos < sourceLength && outPos < 256 )
	{
		char token[256];
		bool isVar = false;

		// check for variables
		if ( pMsg[0] == '%' )
		{
			isVar = true;
			++pMsg;
			sourcePos ++;
		}

		// parse out the string
		const char *end = strchr( pMsg, '%' );
		if ( end )
		{
			int len = end - pMsg + 1;
			Q_strncpy( token, pMsg, min(len, sizeof(token)-1) );
			token[len] = 0;
		}
		else
			Q_strncpy( token, pMsg, sizeof(token) );

		int tokenSize = strlen( token );
		pMsg += tokenSize;
		sourcePos += tokenSize;
		if ( isVar )
		{
			++pMsg; // move over the end of the variable
			sourcePos ++;
		}

		// add the key name into the output text
		// modify the label if necessary
		if ( isVar )
		{
			// lookup key names, change some key names into better names
			char friendlyName[64];
			const char *key = engine->Key_LookupBinding( token[0] == '+' ? token + 1 : token );
			if ( !key )
			{
				Msg("%s is not bound\n", token);
				key = "< not bound >";
			}

			Q_snprintf( friendlyName, sizeof(friendlyName), "#%s", key );
			Q_strupr( friendlyName );

			wchar_t *locName = g_pVGuiLocalize->Find( friendlyName );
			if ( locName )
				g_pVGuiLocalize->ConvertUnicodeToANSI( locName, friendlyName, sizeof(friendlyName) );
			else
				Q_snprintf( friendlyName, sizeof(friendlyName), key ); // don't keep the # on the front

			Q_strncpy(parsedMessage+outPos, friendlyName, sizeof(parsedMessage)-outPos);
			outPos += strlen(friendlyName);
		}
		else
		{
			// add token on directly
			Q_strncpy(parsedMessage+outPos, token, sizeof(parsedMessage)-outPos);
			outPos += strlen(token);
		}
	}

	return parsedMessage;
}