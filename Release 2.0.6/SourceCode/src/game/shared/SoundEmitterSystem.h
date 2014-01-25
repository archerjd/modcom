#ifndef SOUND_EMITTER_SYSTEM_H
#define SOUND_EMITTER_SYSTEM_H
#ifdef _WIN32
#pragma once
#endif


#include "cbase.h"
#include <ctype.h>
#include <KeyValues.h>
#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "igamesystem.h"
#include "soundchars.h"
#include "filesystem.h"
#include "tier0/vprof.h"
#include "checksum_crc.h"
#include "tier0/icommandline.h"

#ifndef CLIENT_DLL
#include "envmicrophone.h"
#include "sceneentity.h"
#else
#include <vgui_controls/Controls.h>
#include <vgui/IVGui.h>
#include "hud_closecaption.h"
#define CRecipientFilter C_RecipientFilter
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef _XBOX
int LookupStringFromCloseCaptionToken( char const *token );
const wchar_t *GetStringForIndex( int index );
#endif

#if !defined( CLIENT_DLL )

void ClearModelSoundsCache();

#endif // !CLIENT_DLL

class IRecipientFilter;
struct CSoundParameters;
struct EmitSound_t;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CSoundEmitterSystem : public CBaseGameSystem
{
public:
	virtual char const *Name() { return "CSoundEmitterSystem"; }

#if !defined( CLIENT_DLL )
	bool			m_bLogPrecache;
	FileHandle_t	m_hPrecacheLogFile;
	CUtlSymbolTable m_PrecachedScriptSounds;
public:
	CSoundEmitterSystem( char const *pszName ) :
		m_bLogPrecache( false ),
		m_hPrecacheLogFile( FILESYSTEM_INVALID_HANDLE )
	{
	}

	void LogPrecache( char const *soundname );
	void StartLog();
	void FinishLog();
#else
	CSoundEmitterSystem( char const *name )
	{
	}

#endif

	// IServerSystem stuff
	virtual bool Init();
	virtual void Shutdown();

	virtual void TraceEmitSound( char const *fmt, ... );

	// Precache all wave files referenced in wave or rndwave keys
	virtual void LevelInitPreEntity();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPostEntity();
		
	void InternalPrecacheWaves( int soundIndex );

	void InternalPrefetchWaves( int soundIndex );

	HSOUNDSCRIPTHANDLE PrecacheScriptSound( const char *soundname );

	void PrefetchScriptSound( const char *soundname );
public:
	void EmitSoundByHandle( IRecipientFilter& filter, int entindex, const EmitSound_t & ep, HSOUNDSCRIPTHANDLE& handle );
	void EmitSound( IRecipientFilter& filter, int entindex, const EmitSound_t & ep );

	void EmitCloseCaption( IRecipientFilter& filter, int entindex, bool fromplayer, char const *token, CUtlVector< Vector >& originlist, float duration, bool warnifmissing /*= false*/ );
	void EmitCloseCaption( IRecipientFilter& filter, int entindex, const CSoundParameters & params, const EmitSound_t & ep );

	void EmitAmbientSound( int entindex, const Vector& origin, const char *soundname, float flVolume, int iFlags, int iPitch, float soundtime /*= 0.0f*/, float *duration /*=NULL*/ );

	void StopSoundByHandle( int entindex, const char *soundname, HSOUNDSCRIPTHANDLE& handle );
	void StopSound( int entindex, const char *soundname );
	void StopSound( int iEntIndex, int iChannel, const char *pSample );

	void EmitAmbientSound( int entindex, const Vector &origin, const char *pSample, float volume, soundlevel_t soundlevel, int flags, int pitch, float soundtime /*= 0.0f*/, float *duration /*=NULL*/ );
};


extern IGameSystem *SoundEmitterSystem();

#endif