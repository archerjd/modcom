#ifndef HUD_HINTDISPLAY_H
#define HUD_HINTDISPLAY_H

class IGameEvent;

//-----------------------------------------------------------------------------
// Purpose: Displays hints across the center of the screen
//-----------------------------------------------------------------------------
class CHudHintDisplay : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudHintDisplay, vgui::Panel );

public:
	CHudHintDisplay( const char *pElementName );

	void Init();
	void Reset();
	void MsgFunc_HintText( bf_read &msg );
	void FireGameEvent( IGameEvent * event);

	void ShowHint(const char *text, bool alwaysShow);
	void ClearHint();	

	virtual void PerformLayout();

protected:
	void LocalizeAndDisplay( const char *pszHudTxtMsg, const char *szRawString );
	bool SetHintText( wchar_t *text );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();

protected:
	vgui::HFont m_hFont;
	Color		m_bgColor;
	vgui::Label *m_pLabel;
	CUtlVector<vgui::Label *> m_Labels;
	CPanelAnimationVarAliasType( int, m_iTextX, "text_xpos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextY, "text_ypos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iCenterX, "center_x", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iCenterY, "center_y", "0", "proportional_int" );

	bool		m_bLastLabelUpdateHack;
	CPanelAnimationVar( float, m_flLabelSizePercentage, "HintSize", "0" );
};


//-----------------------------------------------------------------------------
// Purpose: Displays small key-centric hints on the right hand side of the screen
//-----------------------------------------------------------------------------
class CHudHintKeyDisplay : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudHintKeyDisplay, vgui::Panel );

public:
	CHudHintKeyDisplay( const char *pElementName );
	void Init();
	void Reset();
	void MsgFunc_KeyHintText( bf_read &msg );
	bool ShouldDraw();

	void ShowHint(const char *text);
	void ClearHint();

protected:
	
	bool SetHintText( const char *text );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();

private:
	CUtlVector<vgui::Label *> m_Labels;
	vgui::HFont m_hSmallFont, m_hLargeFont;
	char m_szHintText[128];
	int		m_iBaseY;

	CPanelAnimationVarAliasType( float, m_iTextX, "text_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iTextY, "text_ypos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iTextGapX, "text_xgap", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iTextGapY, "text_ygap", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iYOffset, "YOffset", "0", "proportional_float" );
};

#define SHOW_KEY_HINT( message ) ((CHudHintKeyDisplay*)GET_HUDELEMENT(CHudHintKeyDisplay))->ShowHint(message)
#define CLEAR_KEY_HINT() ((CHudHintKeyDisplay*)GET_HUDELEMENT(CHudHintKeyDisplay))->ClearHint()

#define SHOW_HINT( message ) ((CHudHintDisplay*)GET_HUDELEMENT(CHudHintDisplay))->ShowHint(message, false);
#define SHOW_HINT_ALWAYS( message ) ((CHudHintDisplay*)GET_HUDELEMENT(CHudHintDisplay))->ShowHint(message, true);
#define CLEAR_HINT() ((CHudHintDisplay*)GET_HUDELEMENT(CHudHintDisplay))->ClearHint();

#endif