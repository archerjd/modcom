#include "cbase.h"
#include "make_char_panel.h"
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

#define MAT_ROOT	"materials/VGUI/"
#define MAT_DIR		"playermodels/"
#define FULL_DIR	VarArgs("%s%s",MAT_ROOT,MAT_DIR)

extern McConVar mc_perlevel_modulepoints;

CMakeCharPanel::CMakeCharPanel(IViewPort *pViewPort) : Frame(NULL, PANEL_MAKECHAR )
{
	m_pViewPort = pViewPort;

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);
	SetTitleBarVisible(false);

	m_pCharname = new vgui::TextEntry(this, "charname");
	m_pCharImage = new vgui::CBitmapImagePanel( this, "charimage" );

	m_pFactionDesc = new vgui::Label(this,"factionDesc","");

	m_pFactionSelect = new vgui::ComboBox(this, "ddlFaction", 3, false);
/*	
	// create faction selection menu
	CommandMenu * menu = new CommandMenu(m_pFactionSelect, "factionselect", gViewPortInterface);
	menu->LoadFromFile( "resource/factions.res" );
	m_pFactionSelect->SetMenu( menu );	// attach menu to combo box
*/
	KeyValues *kv1 = new KeyValues("UserData", "command", "faction 1");
	KeyValues *kv2 = new KeyValues("UserData", "command", "faction 2");
	KeyValues *kv3 = new KeyValues("UserData", "command", "faction 3");	

	m_pFactionSelect->AddItem( "#faction_combine", /*m_pFactionSelect->GetActiveItemUserData()*/kv1 );
	m_pFactionSelect->AddItem( "#faction_resistance", kv2 );
	m_pFactionSelect->AddItem( "#faction_aperture", kv3 );
	
	kv1->deleteThis();
	kv2->deleteThis();
	kv3->deleteThis();

	LoadControlSettings("resource/ui/MakeCharPanel.res");
	m_pFactionSelect->SetEditable(false);

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
	AddMaterialFile("combine_soldier_prisonguard2.vmt", FACTION_COMBINE);
}

CMakeCharPanel::~CMakeCharPanel()
{

}

/*
void CMakeCharPanel::LoadAllFilesRecursive(const char *rootdir, const char *pathID, const char *extension,const char *reldir)
{
	FileFindHandle_t fh;
	char path[ 512 ];
	Q_snprintf( path, sizeof( path ), "%s%s*", rootdir,reldir);
//	Msg("Seeking %s... ",path);
	char const *fn = g_pFullFileSystem->FindFirstEx( path, pathID, &fh );
	if ( fn )
	{
//		Msg("\n");
		do
		{
			if ( fn[0] != '.'  )
			{
				if ( filesystem->FindIsDirectory( fh ) )
				{
					char subdir[ 512 ];
					Q_snprintf( subdir, sizeof( subdir ), "%s%s/", reldir,fn);
					//Msg("Found subdir: %s (%s, %s)\n",subdir,reldir,fn);
					LoadAllFilesRecursive(rootdir,pathID,extension,subdir);
				}
				else
				{
					char ext[ 10 ];
					Q_ExtractFileExtension( fn, ext, sizeof( ext ) );
	
					if ( FStrEq( ext, extension ) )
					{
						AddMaterialFile(VarArgs("%s%s",reldir,fn));
					}
				}
			}
//			else
//				Msg("Ignoring fn = %s\n",fn);

			fn = g_pFullFileSystem->FindNext( fh );

		} while ( fn );

		g_pFullFileSystem->FindClose( fh );
	}
//	else
//		Msg("found nothing!\n");
}
*/

void CMakeCharPanel::AddMaterialFile(const char *filename, int faction)
{
	//Msg("Found material file: %s\n",filename);
	int len = Q_strlen(filename) - 4; // except for .vmt
//	Q_memcpy(m_szModels[m_iNumModels],filename,min(len,sizeof(m_szModels[m_iNumModels])));
//	Q_strcat(m_szModels[m_iNumModels],"\0",sizeof(m_szModels[m_iNumModels]));
	switch ( faction )
	{
	case FACTION_COMBINE:
		for ( int i=0; i<len; i++ )
			m_szCombineModels[m_iNumModelsCombine][i] = filename[i];
		m_szCombineModels[m_iNumModelsCombine][len] = '\0';
		m_iNumModelsCombine ++;
		break;
	case FACTION_RESISTANCE:
		for ( int i=0; i<len; i++ )
			m_szResistanceModels[m_iNumModelsResistance][i] = filename[i];
		m_szResistanceModels[m_iNumModelsResistance][len] = '\0';
		m_iNumModelsResistance ++;
		break;
	case FACTION_APERTURE:
		for ( int i=0; i<len; i++ )
			m_szApertureModels[m_iNumModelsAperture][i] = filename[i];
		m_szApertureModels[m_iNumModelsAperture][len] = '\0';
		m_iNumModelsAperture ++;
		break;
	}
}

void CMakeCharPanel::Update()
{
	// make the hud forget any abilities it has selected
	GetModuleHud()->Reset();
	m_pFactionSelect->ActivateItemByRow(random->RandomInt(0,2));

	m_pCharname->SetText("");

	
	if ( m_iFaction > FACTION_NONE )
	{
		switch ( m_iFaction )
		{
			case FACTION_COMBINE:
				m_iModel = random->RandomInt(0,m_iNumModelsCombine-1); break;
			case FACTION_RESISTANCE:
				m_iModel = random->RandomInt(0,m_iNumModelsResistance-1); break;
			case FACTION_APERTURE:
				m_iModel = random->RandomInt(0,m_iNumModelsAperture-1); break;
		}
		m_pCharImage->setTexture( GetTextureName() );
	}
}

void CMakeCharPanel::ShowPanel( bool bShow )
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
	}
}

void CMakeCharPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	m_pFactionSelect->GetMenu()->MakeReadyForUse();
	BaseClass::ApplySchemeSettings(pScheme);
}

void CMakeCharPanel::OnKeyCodePressed(vgui::KeyCode code)
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

extern bool IsValidInputString(const char *test);
extern bool g_bJustCameDirectlyFromMakeCharPanel;

void CMakeCharPanel::OnCommand( const char *command )
{
	if ( FStrEq( command, "cancel" ) )
	{
		// show character selection panel
		ShowPanel(false);
		//gViewPortInterface->ShowPanel( PANEL_PICKCHAR, true );
		g_bJustCameDirectlyFromMakeCharPanel = true;
		engine->ClientCmd("pickchar");
	}
	else if ( FStrEq( command, "create" ) )
	{
		// if its a valid name & class, then send it off to the server,
		// that will see if the name's already taken. If it isn't, will show this again,
		// otherwise, will spawn them properly.
		char szCharname[NAME_LENGTH];
		char szCharModel[MODEL_LENGTH];
		m_pCharname->GetText( szCharname, sizeof( szCharname ) );
		Q_snprintf(szCharModel,sizeof(szCharModel),GetModelName());

		if ( !IsValidInputString( szCharname ) )
		{
			m_pCharname->SetText("");
			CMsgPanel::ShowMessage("#bad_charname_title","#bad_charname_body");	
			return;
		}
		engine->ClientCmd(VarArgs("make_char %s %s",szCharModel, szCharname));
		ShowPanel(false);
		C_HL2MP_Player::GetLocalHL2MPPlayer()->ResetSessionExp();
	}
	else if ( FStrEq( command, "prev" ) )
	{
		m_iModel --;
		if ( m_iModel < 0 )
		{
			switch ( m_iFaction )
			{
				case FACTION_COMBINE:
					m_iModel = m_iNumModelsCombine-1; break;
				case FACTION_RESISTANCE:
					m_iModel = m_iNumModelsResistance-1; break;
				case FACTION_APERTURE:
					m_iModel = m_iNumModelsAperture-1; break;
			}
		}
		m_pCharImage->setTexture( GetTextureName() );
	}
	else if ( FStrEq( command, "next" ) )
	{
		m_iModel ++;
		switch ( m_iFaction )
		{
			case FACTION_COMBINE:
				if ( m_iModel >= m_iNumModelsCombine )
					m_iModel = 0;
				break;
			case FACTION_RESISTANCE:
				if ( m_iModel >= m_iNumModelsResistance )
					m_iModel = 0;
				break;
			case FACTION_APERTURE:
				if ( m_iModel >= m_iNumModelsAperture )
					m_iModel = 0;
				break;
		}
		m_pCharImage->setTexture( GetTextureName() );
	}
	/*else
	{
		Msg("OOOH ITS A COMMAND!!!\n");
		Msg(command);
	}
	else if ( FStrEq( command, "faction" ) )
	{
		
		return;
	}*/
	BaseClass::OnCommand(command);
}

void CMakeCharPanel::OnTextChanged(KeyValues *data)
{
	Panel *panel = reinterpret_cast<vgui::Panel *>( data->GetPtr("panel") );
	vgui::ComboBox *box = dynamic_cast<vgui::ComboBox *>( panel );

	if( box == m_pFactionSelect)
	{
		switch ( m_pFactionSelect->GetActiveItem() )
		{
		case FACTION_COMBINE-1:
			m_pFactionDesc->SetText("#faction_combine_desc");
			m_iFaction = FACTION_COMBINE;
			break;
		case FACTION_RESISTANCE-1:
			m_pFactionDesc->SetText("#faction_resistance_desc");
			m_iFaction = FACTION_RESISTANCE;
			break;
		case FACTION_APERTURE-1:
			m_pFactionDesc->SetText("#faction_aperture_desc");
			m_iFaction = FACTION_APERTURE;
			break;
		}

		// faction changed, randomize model
		switch ( m_iFaction )
		{
			case FACTION_COMBINE:
				m_iModel = random->RandomInt(0,m_iNumModelsCombine-1); break;
			case FACTION_RESISTANCE:
				m_iModel = random->RandomInt(0,m_iNumModelsResistance-1); break;
			case FACTION_APERTURE:
				m_iModel = random->RandomInt(0,m_iNumModelsAperture-1); break;
		}
		m_pCharImage->setTexture( GetTextureName() );
	}
}

const char *CMakeCharPanel::GetModelName()
{
	switch ( m_iFaction )
	{
		case FACTION_COMBINE:
			return VarArgs("models/%s.mdl",m_szCombineModels[m_iModel]);
		case FACTION_RESISTANCE:
			return VarArgs("models/%s.mdl",m_szResistanceModels[m_iModel]);
		case FACTION_APERTURE:
			return VarArgs("models/%s.mdl",m_szApertureModels[m_iModel]);
		default:
			return "";
	}
}

const char *CMakeCharPanel::GetTextureName()
{
	switch ( m_iFaction )
	{
		case FACTION_COMBINE:
			return VarArgs("%s%s.vmt",MAT_DIR,m_szCombineModels[m_iModel]);
		case FACTION_RESISTANCE:
			return VarArgs("%s%s.vmt",MAT_DIR,m_szResistanceModels[m_iModel]);
		case FACTION_APERTURE:
			return VarArgs("%s%s.vmt",MAT_DIR,m_szApertureModels[m_iModel]);
		default:
			return "";
	}

	
}