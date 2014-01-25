//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "modcom/hud_buffs.h"
#include "hud_macros.h"
#include "hl2mp/c_hl2mp_player.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CHudBuffs );

// instance ... we need to be able to get this panel from anywhere to tell it to reset abilities 
CHudBuffs *CHudBuffs::instance = NULL;
CHudBuffs *GetModuleHud()
{
	return CHudBuffs::instance;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudBuffs::CHudBuffs( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudBuffs" )
{
	instance = this;

	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
	
	m_iNumBuffsBeingShown = 0;
	for ( int i=0; i<NUM_BUFFS; i++ )
	{
		m_iBuffsBeingShown[i] = BUFF_NONE;
		m_bScheduledRemovals[i] = false;
	}

	SetPaintBackgroundEnabled(false);
}

void CHudBuffs::VidInit( void )
{
	m_GoodColor = gHUD.m_clrNormal;
	m_BadColor = gHUD.m_clrCaution;
	m_NeutralColor = gHUD.m_clrNormal;
	
	m_WipeTexture = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_WipeTexture, "vgui/gfx/buff_wipe", true, false );

	m_LevelTexture = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_LevelTexture, "vgui/gfx/buff_level", true, false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudBuffs::Init( void )
{
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudBuffs::Reset( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudBuffs::ShouldDraw()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return false;
		
	return CHudElement::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudBuffs::OnThink( void )
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return;
	
	// we need to scan the player to see if any of the buffs currently being shown are no longer active (and remove them if so) ... removal should include calling the relevant "removed" hud animation
	for ( int i=0; i<m_iNumBuffsBeingShown; i++ )
	{
		if ( !m_bScheduledRemovals[i] && !pPlayer->IsBuffActive(m_iBuffsBeingShown[i]) )
		{// this buff we're showing has been disabled on the player! shunt everything after it up to leave no gaps
			// play the removal animation to let the visual gap close over a short period ... only if there are any buffs below this one
			if ( i < m_iNumBuffsBeingShown-1 )
				AnimateItemRemoval(i);

			// but don't actually remove the item from the list until the next frame - cos sometimes, even when set to start instantly, the animation doesn't begin until 1 frame late.
			m_bScheduledRemovals[i] = true;
		}
		else if ( m_bScheduledRemovals[i] )
		{
			m_bScheduledRemovals[i] = false;
			m_iNumBuffsBeingShown--;

			for ( int j=i; j<m_iNumBuffsBeingShown; j++ )
				m_iBuffsBeingShown[j] = m_iBuffsBeingShown[j+1];
			m_iBuffsBeingShown[m_iNumBuffsBeingShown] = BUFF_NONE;
		}
	}
		
	// then we need to scan again and see if any buffs that aren't in our list (but should be shown) are now active - and add them if so.
	for ( int i=0; i<NUM_BUFFS; i++ )
		if ( pPlayer->IsBuffActive(i) )
		{// check if its in our list ... if it is, continue;
			bool bAlreadyShown = false;
			for ( int j=0; j<m_iNumBuffsBeingShown; j++ )
				if ( m_iBuffsBeingShown[j] == i )
				{
					bAlreadyShown = true;
					break;
				}
			if ( bAlreadyShown )
				continue; // give up on this one, we're showing it already
				
			// check if this is a "hidden" buff
			Buff *b = GetBuff(i);
			if ( !b->ShouldShowOnHud(pPlayer) )
				continue; // this buff shouldn't show, give up on it.
			
			// want to add this buff to the list.
			m_iBuffsBeingShown[m_iNumBuffsBeingShown] = i;
			m_iNumBuffsBeingShown ++;
			
			// do we want buffs to slide on from the side or fade in?
		}
}

extern float FitVarToRange(float minRange, float maxRange, float minVar, float maxVar, float position);

//-----------------------------------------------------------------------------
// Purpose: draws the power bar
//-----------------------------------------------------------------------------
void CHudBuffs::Paint()
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer || m_iNumBuffsBeingShown == 0 )
		return;
	
	// the buffs' indexes are sorted in the m_iBuffsBeingShown array, in the order we want to display them in.
	// I guess the simplest thing to do for now is to just display the names of each of them in a big text bit, for now.
	surface()->DrawSetTextFont( m_hFont );
	float yPos = item_seperator * 0.5f;
	float itemSeperation = item_height + item_seperator;
	
	surface()->DrawSetTextColor( m_TextColor );
	for ( int i=0; i<m_iNumBuffsBeingShown; i++ )
	{
		// hmm ... if we can detect when an item is removed, and call the "BuffBarItem<N>Deactivated" animation, that'll work for single items disappearing.
		// But it would involve jumping by a whole item position if two or more items disappearing at once, or very close together.
		// ...that could perhaps be got around if instead of triggering the animation, we simply added 1 to the relevant item<N>_extra value rather than triggering the animation, if the extra value was non-zero.
		switch ( i )
		{
		case 0: yPos += item1_extra * itemSeperation; break;
		case 1: yPos += item2_extra * itemSeperation; break;
		case 2: yPos += item3_extra * itemSeperation; break;
		case 3: yPos += item4_extra * itemSeperation; break;
		case 4: yPos += item5_extra * itemSeperation; break;
		case 5: yPos += item6_extra * itemSeperation; break;
		case 6: yPos += item7_extra * itemSeperation; break;
		case 7: yPos += item8_extra * itemSeperation; break;
		case 8: yPos += item9_extra * itemSeperation; break;
		case 9: yPos += item10_extra * itemSeperation; break;
		case 10: yPos += item11_extra * itemSeperation; break;
		case 11: yPos += item12_extra * itemSeperation; break;
		case 12: yPos += item13_extra * itemSeperation; break;
		case 13: yPos += item14_extra * itemSeperation; break;
		case 14: yPos += item15_extra * itemSeperation; break;
		}
		
		int extraInset = (int)((item_height + item_seperator) * 0.68f);

		// first, fill in the background rectangle
		surface()->DrawSetColor(m_BackColor);
		surface()->DrawFilledRect(extraInset, yPos, extraInset+item_width, yPos + item_height);
		Color effectColor;


		Buff *b = GetBuff(m_iBuffsBeingShown[i]);
		// select color to use depending on whether its a good / bad / neutral buff
		switch ( b->GetBuffType() )
		{
			case BUFF_TYPE_GOOD:
				effectColor =  m_GoodColor;
				break;
			case BUFF_TYPE_BAD:
				effectColor = m_BadColor;
				break;
			default:
				effectColor = m_NeutralColor;
				break;
		}

		// draw rectangular outline
		surface()->DrawSetColor( effectColor );
		surface()->DrawOutlinedRect(extraInset, yPos, extraInset+item_width, yPos + item_height);

			
		if ( !b->IsPermenant() )
		{// draw a "wipe" depicting this buff's current progress through its duration.
			surface()->DrawSetTexture(m_WipeTexture);
			surface()->DrawSetColor( effectColor.r(), effectColor.g(), effectColor.b(), (int)(effectColor.a() * 0.65f) );
			float fraction = GetProgressFraction(pPlayer, b);
			surface()->DrawTexturedSubRect(extraInset,yPos,extraInset+item_width*fraction,yPos+item_height,1.0f-fraction, 0.0f, 1.0f, 1.0f);
		}
		
		if ( b->ShowLevelOnHud() )
		{// indicate severity / level / stack somehow - need a number somewhere in all this.
			// given the lack of any more specific instruction, let's draw a circle with a border on the left-hand side.
			surface()->DrawSetTexture(m_LevelTexture);
			surface()->DrawSetColor(effectColor);
			surface()->DrawTexturedRect(0, yPos-item_seperator/2, item_height+item_seperator/2, yPos+item_height+item_seperator/2);
			
			int level = pPlayer->GetBuffLevel(b->GetBuffIndex());
			if ( level > 9 )
				surface()->DrawSetTextPos( item_height*0.33f, yPos + text_yoffset );
			else
				surface()->DrawSetTextPos( item_height*0.41f, yPos + text_yoffset );

			wchar_t unicode[10];
			swprintf(unicode, L"%d", level);
			surface()->DrawUnicodeString( unicode );
		}
		
		surface()->DrawSetTextPos( extraInset+text_xoffset, yPos + text_yoffset );
		surface()->DrawUnicodeString( b->GetDisplayNameUnicode() );

		yPos += itemSeperation;
	}
}

void CHudBuffs::AnimateItemRemoval(int i)
{
	switch ( i )
	{
		case 0:
			if ( item1_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem1Deactivated");
			else
				item1_extra += 1.0f;
			break;
		case 1:
			if ( item2_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem2Deactivated");
			else
				item2_extra += 1.0f;
			break;
		case 2:
			if ( item3_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem3Deactivated");
			else
				item3_extra += 1.0f;
			break;
		case 3:
			if ( item4_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem4Deactivated");
			else
				item4_extra += 1.0f;
			break;
		case 4:
			if ( item5_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem5Deactivated");
			else
				item5_extra += 1.0f;
			break;
		case 5:
			if ( item6_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem6Deactivated");
			else
				item6_extra += 1.0f;
			break;
		case 6:
			if ( item7_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem7Deactivated");
			else
				item7_extra += 1.0f;
			break;
		case 7:
			if ( item8_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem8Deactivated");
			else
				item8_extra += 1.0f;
			break;
		case 8:
			if ( item9_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem9Deactivated");
			else
				item9_extra += 1.0f;
			break;
		case 9:
			if ( item10_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem10Deactivated");
			else
				item10_extra += 1.0f;
			break;
		case 10:
			if ( item11_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem11Deactivated");
			else
				item11_extra += 1.0f;
			break;
		case 11:
			if ( item12_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem12Deactivated");
			else
				item12_extra += 1.0f;
			break;
		case 12:
			if ( item13_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem13Deactivated");
			else
				item13_extra += 1.0f;
			break;
		case 13:
			if ( item14_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem14Deactivated");
			else
				item14_extra += 1.0f;
			break;
		case 14:
			if ( item15_extra == 0.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BuffBarItem15Deactivated");
			else
				item15_extra += 1.0f;
			break;
	}
}

float CHudBuffs::GetProgressFraction(C_HL2MP_Player *pPlayer, Buff *b)
{
	int index = b->GetBuffIndex();
	if ( !pPlayer->IsBuffActive(index) || b->IsPermenant() )
		return 0.0f;
	
	return 1.0f - min(1.0f, max(0.0f, (pPlayer->GetBuffEndTime(index) - gpGlobals->curtime) / b->GetDuration(pPlayer->GetBuffLevel(index))));
}

/*
float CHudBuffs::CalcCooldownFraction(Buff *b)
{
	C_HL2MP_Player *pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if ( !pPlayer )
		return 1.0f;

	float end = pPlayer->GetCooldownEnd(a);
	if ( end <= gpGlobals->curtime )
		return 1.0f;

	int level = pPlayer->GetModuleLevel(a);
	float duration = max(a->GetCooldown(level) + a->GetCastTime(level), a->GetMaintainCooldown(level));
	float start = end - duration;
	return ( gpGlobals->curtime - start ) / ( end - start );
}*/