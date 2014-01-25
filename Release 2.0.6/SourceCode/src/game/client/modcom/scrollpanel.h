#ifndef SCROLLPANEL_H
#define SCROLLPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ScrollBar.h>

class ScrollPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( ScrollPanel, EditablePanel );
public:
	ScrollPanel(vgui::Panel *parent, const char *name, bool isVertical)
		: EditablePanel(parent, name)
	{
		m_pScrollBar = new vgui::ScrollBar(this, "Scrollbar", isVertical);
		m_pScrollBar->AddActionSignalTarget(this);
		m_bIsVertical = isVertical;
		m_iMaxScrollValue = m_iPreviousValue = 0;
		m_bRejectSliderMove = false;
	}

	virtual void Update( void );
	//virtual void OnSizeChanged(int newWide, int newTall);
	virtual void OnMouseWheeled(int delta);

protected:
	MESSAGE_FUNC_INT( OnSliderMoved, "ScrollBarSliderMoved", position );
	//void MoveBy( int amount );
	int GetMaxScrollValue() { return m_iMaxScrollValue; }
	void SetMaxScrollValue(int i);
private:
	vgui::ScrollBar *m_pScrollBar;
	bool m_bIsVertical, m_bRejectSliderMove;
	int m_iMaxScrollValue, m_iPreviousValue;
};


#endif