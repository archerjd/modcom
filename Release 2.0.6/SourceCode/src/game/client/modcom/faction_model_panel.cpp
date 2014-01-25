#include "cbase.h"
#include "modcom/faction_model_panel.h"
#include "vgui/ILocalize.h"
#include "filesystem.h"
#include "commandmenu.h"

#include "msg_panel.h"
#include "c_hl2mp_player.h"
#include "modcom/mc_shareddefs.h"
#include "modcom/mcconvar.h"

#include "hud_modules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CFactionModelPanel::CFactionModelPanel(IViewPort *pViewPort) : Frame(NULL, PANEL_MAKECHAR )
{
/*	m_pViewPort = pViewPort;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	m_pCharname = new vgui::TextEntry(this, "charname");
	m_pCharImage = new vgui::CBitmapImagePanel( this, "charimage" );

	m_pStatValues[0] = new vgui::Label(this,"valueHealth","");
	m_pStatValues[1] = new vgui::Label(this,"valueArmor","");
	m_pStatValues[2] = new vgui::Label(this,"valueAuxPower","");
	m_pStatValues[3] = new vgui::Label(this,"valueSprint","");
	m_pStatValues[4] = new vgui::Label(this,"valueAmmo","");

	m_pPointsRemaining = new vgui::Label(this,"pointsRemaining","");
	m_pFactionDesc = new vgui::Label(this,"factionDesc","");

	m_pFactionSelect = new vgui::ComboBox(this, "ddlFaction", 3, false);
	m_pProgressionSelect = new vgui::ComboBox(this, "ddlProgression", 3, false);

	KeyValues *kv1 = new KeyValues("UserData", "command", "faction 1");
	KeyValues *kv2 = new KeyValues("UserData", "command", "faction 2");
	KeyValues *kv3 = new KeyValues("UserData", "command", "faction 3");	

	m_pFactionSelect->AddItem( "#faction_combine", kv1 );
	m_pFactionSelect->AddItem( "#faction_resistance", kv2 );
	m_pFactionSelect->AddItem( "#faction_aperture", kv3 );
	
	kv1->deleteThis();
	kv2->deleteThis();
	kv3->deleteThis();

	LoadControlSettings("resource/ui/MakeCharPanel.res");
	m_pFactionSelect->SetEditable(false);
	m_pProgressionSelect->SetEditable(false);

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

	// if we're to load all model materials, should do so here
	m_iNumModelsCombine = m_iNumModelsResistance = m_iNumModelsAperture = m_iModel = 0;
	m_iFaction = FACTION_NONE;
	
	m_pCharname->SetMaximumCharCount(NAME_LENGTH);

	//LoadAllFilesRecursive(FULL_DIR,"GAME","vmt"); // for some reason this doesn't find Group03 when Group01 is present, even when i *force* it

	// so i'm just hardcoding it instead
	AddMaterialFile("humans/Group03/female_01.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/female_02.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/female_03.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/female_04.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/female_06.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/female_07.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/Male_01.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/Male_02.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/Male_03.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/Male_04.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/Male_05.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/Male_06.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/Male_07.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/Male_08.vmt", FACTION_RESISTANCE);
	AddMaterialFile("humans/Group03/Male_09.vmt", FACTION_RESISTANCE);

	AddMaterialFile("humans/Group01/female_01.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/female_02.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/female_03.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/female_04.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/female_06.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/female_07.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/Male_01.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/Male_02.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/Male_03.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/Male_04.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/Male_05.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/Male_06.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/Male_07.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/Male_08.vmt", FACTION_APERTURE);
	AddMaterialFile("humans/Group01/Male_09.vmt", FACTION_APERTURE);

	AddMaterialFile("combine_soldier.vmt", FACTION_COMBINE);
	AddMaterialFile("combine_soldier_prisonguard.vmt", FACTION_COMBINE);
	AddMaterialFile("combine_super_soldier.vmt", FACTION_COMBINE);
	AddMaterialFile("police.vmt", FACTION_COMBINE);
	AddMaterialFile("combine_soldier2.vmt", FACTION_COMBINE);
	AddMaterialFile("combine_soldier_prisonguard2.vmt", FACTION_COMBINE);*/
}

CFactionModelPanel::~CFactionModelPanel()
{

}
