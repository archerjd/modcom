#include "cbase.h"
#include "scrollpanel.h"

#include "tier0/memdbgon.h"

using namespace vgui;
void ScrollPanel::Update( void )
{
	int sixteen = scheme()->GetProportionalScaledValue(16);

	if ( m_bIsVertical )
		m_pScrollBar->SetBounds(GetWide() - sixteen, 0, sixteen, GetTall());
	else
		m_pScrollBar->SetBounds(0, GetTall() - sixteen, GetWide(), sixteen);

	int maxValue = GetMaxScrollValue();
	int windowSize = m_bIsVertical ? GetTall() : GetWide();
	m_pScrollBar->SetRange(0, maxValue);
	m_pScrollBar->SetRangeWindow(GetTall());
	m_pScrollBar->SetEnabled(maxValue > windowSize);
	
	//m_iPreviousValue = m_pScrollBar->GetValue();
	m_pScrollBar->Validate();
}
void ScrollPanel::SetMaxScrollValue(int i)
{
	//int value = m_pScrollBar->GetValue();
	m_iMaxScrollValue = i;
	OnSliderMoved(0);
	//m_pScrollBar->SetValue(0);
}

void ScrollPanel::OnMouseWheeled(int delta)
{
	if ( m_pScrollBar->IsEnabled() )
		OnSliderMoved(m_pScrollBar->GetValue()-15*delta);
}

void ScrollPanel::OnSliderMoved(int value)
{
	if ( m_bRejectSliderMove )
	{
		m_bRejectSliderMove = false;
		return;
	}

	value = max(value,0);
	value = min(value, m_iMaxScrollValue);

	m_pScrollBar->SetValue(value);
	int increment = value - m_iPreviousValue;
	m_iPreviousValue = value;
/*
	int min, max;
	m_pScrollBar->GetRange(min,max);
	Msg(VarArgs("Slider value set to %i, range [%i-%i, %i], tall %i\n", m_pScrollBar->GetValue(), min, max, m_pScrollBar->GetRangeWindow(), GetTall()));
*/
	int x,y, childCount = GetChildCount();
	for (int i=0; i<childCount; i++)
	{
		Panel *p = GetChild(i);
		if ( p == m_pScrollBar )
			continue;
		p->GetPos(x, y);
		if ( m_bIsVertical )
			p->SetPos(x, y-increment);
		else
			p->SetPos(x-increment, y);
	}

	//stops a loop happening where moveby updates slider and makes it move again.
	m_bRejectSliderMove = true;
	
	InvalidateLayout();
}