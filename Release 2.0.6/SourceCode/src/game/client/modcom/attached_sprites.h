#ifndef ATTACHED_SPRITES_H
#define ATTACHED_SPRITES_H
#pragma once


#define MAX_SCORE_TOKEN_INDICATOR 6

class CAttachedSpriteManager
{
public:
				CAttachedSpriteManager();
	virtual		~CAttachedSpriteManager();

// CHudBase overrides.
public:
	
	// Initialize the cl_dll's voice manager.
	virtual int Init();
	
	// ackPosition is the bottom position of where CVoiceStatus will draw the voice acknowledgement labels.
	virtual void VidInit();

public:

	// Call from the HUD_CreateEntities function so it can add sprites above player heads.
	void	DrawHeadLabels();
	void	SetHeadLabelsDisabled( bool bDisabled ) { m_bHeadLabelsDisabled = bDisabled; }

	IMaterial *GetHeadLabelMaterial( void ) { return m_pVoiceLabelMaterial; }

private:

	void DrawIcon(C_BaseCombatCharacter *pTarget, IMaterial *pMaterial, float verticalOffsetFromEyes, float iconSize, double r=1.0, double g=1.0, double b=1.0);
	void ShowScoreTokens(C_BaseCombatCharacter *pBCC, bool isAlly);

	IMaterial			*m_pVoiceLabelMaterial;	// For labels above players' heads.
	IMaterial			*m_pScoreTokenLabelMaterials[MAX_SCORE_TOKEN_INDICATOR];
	
	bool				m_bHeadLabelsDisabled;
};

// Get the (global) voice manager. 
CAttachedSpriteManager* GetAttachedSpriteManager();
void AttachedSpriteManager_Init();
void AttachedSpriteManager_Shutdown();

#endif // ATTACHED_SPRITES_H