//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_AI_BASENPC_H
#define C_AI_BASENPC_H
#ifdef _WIN32
#pragma once
#endif


#include "c_basecombatcharacter.h"
#include "modcom/monsters.h"

// NOTE: MOved all controller code into c_basestudiomodel
class C_AI_BaseNPC : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_AI_BaseNPC, C_BaseCombatCharacter );

public:
	DECLARE_CLIENTCLASS();

	C_AI_BaseNPC();
	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const;
	virtual int GetPlayerIndex();

	virtual bool			IsNPC( void ) { return true; }
//	virtual bool					IsMoving( void ){ return m_bIsMoving; }
	bool					ShouldAvoidObstacle( void ){ return m_bPerformAvoidance; }
	virtual bool			AddRagdollToFadeQueue( void ) { return m_bFadeCorpse; }

	virtual void			GetRagdollInitBoneArrays( matrix3x4_t *pDeltaBones0, matrix3x4_t *pDeltaBones1, matrix3x4_t *pCurrentBones, float boneDt );

//	int						GetDeathPose( void ) { return m_iDeathPose; }

	bool					ShouldModifyPlayerSpeed( void ) { return m_bSpeedModActive;	}
	int						GetSpeedModifyRadius( void ) { return m_iSpeedModRadius; }
	int						GetSpeedModifySpeed( void ) { return m_iSpeedModSpeed;	}

	virtual void			ClientThink( void );
	void					OnDataChanged( DataUpdateType_t type );
	bool					ImportantRagdoll( void ) { return m_bImportanRagdoll;	}

	virtual int				GetHealth() { return m_iHealth; }
	//virtual int			GetMaxHealth() const { return m_iMaxHealth; }
	bool					LikesMaster() { return m_bLikesMaster; }
	EHANDLE					GetMasterPlayer() { return m_pMyPlayer; }
	virtual bool			CanBeBouncedOn() { return false; }

	virtual bool ShouldCollide( int collisionGroup, int contentsMask, int playerIndex ) const;
	virtual wchar_t *		CustomStatusMessage() { return L""; }
	const char *			GetTypeName() { return m_szTypeName; }
	void					SetTypeName(const char *name);

	int						GetSkinOverride() { return m_iSkinOverride; }
	virtual int				GetNumScoreTokens() { return m_iNumScoreTokens; }
private:
	C_AI_BaseNPC( const C_AI_BaseNPC & ); // not defined, not accessible
//	float m_flTimePingEffect;
	int  m_iDeathPose;
	int	 m_iDeathFrame;

	int m_iSpeedModRadius;
	int m_iSpeedModSpeed;

	int m_iNumScoreTokens;

	bool m_bPerformAvoidance;
//	bool m_bIsMoving;
	bool m_bFadeCorpse;
	bool m_bSpeedModActive;
	bool m_bImportanRagdoll;

	//int m_iMaxHealth;
	bool m_bLikesMaster;
	int	 m_iTypeName, m_iSkinOverride;
	char m_szTypeName[MAX_NPC_TYPE_NAME];
	CHandle< CBasePlayer > m_pMyPlayer;
};


#endif // C_AI_BASENPC_H
