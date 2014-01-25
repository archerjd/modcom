//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef HL2MP_PLAYER_H
#define HL2MP_PLAYER_H
#pragma once

class C_HL2MP_Player;
#include "c_basehlplayer.h"
#include "hl2mp_player_shared.h"
#include "beamdraw.h"
#include "in_buttons.h"
#include "modcom/modules.h"
#include "modcom/buffs.h"

extern int ExpForLevelUp( int level );
extern int TotExpForLevelUp( int level );

// disorientation
//#define NUM_RANDOM_KEYS 8

//=============================================================================
// >> HL2MP_Player
//=============================================================================
class C_HL2MP_Player : public C_BaseHLPlayer
{
public:
	DECLARE_CLASS( C_HL2MP_Player, C_BaseHLPlayer );

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();


	C_HL2MP_Player();
	~C_HL2MP_Player( void );

	void ClientThink( void );

	static C_HL2MP_Player* GetLocalHL2MPPlayer();
	
	virtual int DrawModel( int flags );
	virtual void AddEntity( void );

	QAngle GetAnimEyeAngles( void ) { return m_angEyeAngles; }
	Vector GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget = NULL );


	// Should this object cast shadows?
	virtual ShadowType_t		ShadowCastType( void );
	virtual C_BaseAnimating *BecomeRagdollOnClient();
	virtual const QAngle& GetRenderAngles();
	virtual bool ShouldDraw( void );
	virtual void OnDataChanged( DataUpdateType_t type );
	virtual float GetFOV( void );
	virtual CStudioHdr *OnNewModel( void );
	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual void ItemPreFrame( void );
	virtual void ItemPostFrame( void );
	virtual float GetMinFOV()	const { return 5.0f; }
	virtual Vector GetAutoaimVector( float flDelta );
	virtual void NotifyShouldTransmit( ShouldTransmitState_t state );
	virtual void CreateLightEffects( void ) {}
	virtual bool ShouldReceiveProjectedTextures( int flags );
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
	virtual void PreThink( void );
	virtual void DoImpactEffect( trace_t &tr, int nDamageType );
	IRagdoll* GetRepresentativeRagdoll() const;
	virtual void CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov );
	virtual const QAngle& EyeAngles( void );

	virtual int GetPlayerIndex() { return entindex(); }
	
	bool	CanSprint( void );
	void	StartSprinting( void );
	void	StopSprinting( void );
	void	HandleSpeedChanges( void );
	void	UpdateLookAt( void );
	void	Initialize( void );
	int		GetIDTarget() const;
	void	UpdateIDTarget( void );
	void	PrecacheFootStepSounds( void );
	const char	*GetPlayerModelSoundPrefix( void );

	HL2MPPlayerState State_Get() const;

	virtual	bool ShouldCollide( int collisionGroup, int contentsMask, int playerIndex ) const;
	int NumAlliesInRange();

	int GetMaxAuxPower() { return m_iMaxAuxPower; }

	// Walking
	void StartWalking( void );
	void StopWalking( void );
	bool IsWalking( void ) { return m_fIsWalking; }

	virtual int GetModuleLevel(Module *a);
	virtual int GetModuleLevel(int index);
	virtual bool HasModule(Module *a);
	virtual bool HasModule(int index);
	virtual bool IsModuleActive(Module *a);
	virtual bool IsModuleActive(int index);
	virtual float GetCooldownEnd(Module *a);

	// Level & Exp functions
	int GetTotalExp() { return m_iTotalExp; }
	int GetGameExp() { return m_iTotalExp - m_iSessionStartExp; }
	void ResetSessionExp() { m_iSessionStartExp = m_iTotalExp; m_flUpdateSessionExpTime = gpGlobals->curtime + 2.0f; }

	int GetAP() { return m_iModulePoints; } 
	int GetWP() { return m_iWeaponPoints; } 
	int GetCredits() { return m_iCredits; } 

	// login control
	bool IsLoggedOn() { return m_iLoggedOn > 0; }
	bool IsInCharacter();
	int GetFaction();

	int NumMinions() { return m_iNumMinions; }
	//bool InSpawnProtect() { return m_bInSpawnProtect; }

/*	// disorientation
	void C_HL2MP_Player::SetupDisorientation();
	void DoDisorientationRandomize(int mask, bool linearSearch=false);
	int HandleDisorientationForButtons( int nInputButtons );
*/

	// these are only valid for the local player (not networked for other players)
	int GetBuffLevel(int i) { return m_iBuffs[i]; }
	float GetBuffEndTime(int i) { return m_flBuffEndTimes[i]; }

	virtual bool IsBuffActive(int i); // this overrides the base version to also check the above for the local player
	virtual int	GetNumScoreTokens() { return m_iNumScoreTokens; }

private:
	
	C_HL2MP_Player( const C_HL2MP_Player & );

	CPlayerAnimState m_PlayerAnimState;

	QAngle	m_angEyeAngles;

	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	EHANDLE	m_hRagdoll;

	int	m_headYawPoseParam;
	int	m_headPitchPoseParam;
	float m_headYawMin;
	float m_headYawMax;
	float m_headPitchMin;
	float m_headPitchMax;

	bool m_isInit;
	Vector m_vLookAtTarget;

	float m_flLastBodyYaw;
	float m_flCurrentHeadYaw;
	float m_flCurrentHeadPitch;

	int	  m_iIDEntIndex;

	CountdownTimer m_blinkTimer;

	int	  m_iSpawnInterpCounter;
	int	  m_iSpawnInterpCounterCache;

	int	  m_iPlayerSoundType;
	int	  m_iNumScoreTokens;

	void ReleaseFlashlight( void );
	Beam_t	*m_pFlashlightBeam;

	CNetworkVar( HL2MPPlayerState, m_iPlayerState );	

	//bool m_bInSpawnProtect;
	bool m_fIsWalking;
	int m_iSprintSpeed;
	int m_iMaxAuxPower;

	// Level and Experience
	int m_iTotalExp;
	int m_iSessionStartExp;
	float m_flUpdateSessionExpTime;

	int m_iFaction;

 	int m_iModulePoints; 
 	int m_iWeaponPoints; 
 	int m_iCredits;

	int m_iNumMinions;

	// logon & character
	int m_iLoggedOn;
	bool m_bShowFirstModeHint;

	int m_iBuffs[NUM_BUFFS];
	float m_flBuffEndTimes[NUM_BUFFS];
};

inline C_HL2MP_Player *ToHL2MPPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<C_HL2MP_Player*>( pEntity );
}


class C_HL2MPRagdoll : public C_BaseAnimatingOverlay
{
public:
	DECLARE_CLASS( C_HL2MPRagdoll, C_BaseAnimatingOverlay );
	DECLARE_CLIENTCLASS();
	
	C_HL2MPRagdoll();
	~C_HL2MPRagdoll();

	virtual void OnDataChanged( DataUpdateType_t type );

	int GetPlayerEntIndex() const;
	IRagdoll* GetIRagdoll() const;

	void ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName );
	void UpdateOnRemove( void );
	virtual void SetupWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights );
	
private:
	
	C_HL2MPRagdoll( const C_HL2MPRagdoll & ) {}

	void Interp_Copy( C_BaseAnimatingOverlay *pDestinationEntity );
	void CreateHL2MPRagdoll( void );

private:

	EHANDLE	m_hPlayer;
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

#endif //HL2MP_PLAYER_H
