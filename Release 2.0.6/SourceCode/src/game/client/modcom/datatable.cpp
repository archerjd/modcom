#include "cbase.h"
#include "iclientmode.h"
#include "datatable.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define COLUMN_WIDTH	vgui::scheme()->GetProportionalScaledValue(24)
#define ROW_HEIGHT		vgui::scheme()->GetProportionalScaledValue(10)
DataTable::DataTable( vgui::Panel *pParent, int columns, int rows)
	: EditablePanel( pParent, "dataTable", true )
{
	SetScheme("ClientScheme");

	m_iColumns = columns; m_iRows = rows;
	SetPaintBackgroundEnabled(false);
	m_iCellWidth = COLUMN_WIDTH;
	m_iNameWidthOffset = m_iCellWidth * 2.8f;
	//LoadControlSettings("resource/ui/DataTable.res");
	
	SetSize(m_iCellWidth * m_iColumns + 1 + m_iNameWidthOffset, ROW_HEIGHT * m_iRows + 1);

	for ( int j=0; j<m_iRows; j++ )
	{
		m_pRowHeadings[j] = NULL;
		for ( int i=0; i<m_iColumns; i++ )
			m_pFieldLabels[i][j] = NULL;
	}
}

void DataTable::SetRowName(int row, wchar_t *name)
{
	if ( m_pRowHeadings[row] != NULL )
		m_pRowHeadings[row]->SetText(name);
	else
	{
		m_pRowHeadings[row] = new vgui::Label(this,VarArgs("row%i",row), name);
		m_pRowHeadings[row]->SetContentAlignment(vgui::Label::a_center);
		if ( row == 0 )
			m_pRowHeadings[row]->SetTextColorState(vgui::Label::CS_BRIGHT);

		int x=0, y=ROW_HEIGHT*row, w=m_iNameWidthOffset, h=ROW_HEIGHT;//, pw = GetWide(), ph = GetTall();
		m_pRowHeadings[row]->SetBounds(x,y,w,h);
		//m_pRowHeadings[row]->SetAutoResize(vgui::Panel::PIN_TOPLEFT, AUTORESIZE_RIGHT, x, y, (x + w) - pw, (y + h) - ph);
	}

	// recalculate name width offset if this is the widest name label we have
	/*int width = m_pRowHeadings[row]->GetWide() + 2;
	if ( width > m_iNameWidthOffset )
	{
		m_iNameWidthOffset = width;
		Repaint();
	}*/
}

void DataTable::SetValue(int column, int row, wchar_t *value)
{
	if ( m_pFieldLabels[column][row] != NULL )
	{
		m_pFieldLabels[column][row]->SetText(value);
	}
	else
	{
		m_pFieldLabels[column][row] = new vgui::Label(this,VarArgs("val%i%j",column,row), value);
		m_pFieldLabels[column][row]->SetBounds(m_iCellWidth*column + m_iNameWidthOffset, ROW_HEIGHT * row, m_iCellWidth, ROW_HEIGHT);
		//m_pFieldLabels[column][row]->SetAutoResize(vgui::Panel::PIN_TOPLEFT, AUTORESIZE_RIGHT, m_iCellWidth*column + m_iNameWidthOffset, ROW_HEIGHT * row, m_iCellWidth*(column+1) + m_iNameWidthOffset, ROW_HEIGHT * (row+1));
		m_pFieldLabels[column][row]->SetContentAlignment(vgui::Label::a_center);
	}
}

void DataTable::Paint()
{
	BaseClass::Paint();

	vgui::surface()->DrawSetColor(GetFgColor());

	for ( int i=0; i<=m_iRows; i++ )
		vgui::surface()->DrawLine(0, ROW_HEIGHT*i, GetWide(), ROW_HEIGHT*i);
	for ( int i=0; i<=m_iColumns; i++ )
		vgui::surface()->DrawLine(i*m_iCellWidth + m_iNameWidthOffset, 0, i*m_iCellWidth + m_iNameWidthOffset, GetTall());
	vgui::surface()->DrawLine(0, 0, 0, GetTall());
}

void DataTable::Update(int highlightColumn)
{
	vgui::IScheme *scheme = vgui::scheme()->GetIScheme( GetScheme() );
	for ( int i=0; i<m_iColumns; i++ )
	{
		vgui::Label::EColorState state = i == highlightColumn ? vgui::Label::CS_BRIGHT : vgui::Label::CS_DULL;
		for ( int j=0; j<m_iRows; j++ )
			if ( m_pFieldLabels[i][j] != NULL )
			{
				m_pFieldLabels[i][j]->SetTextColorState(state);

				// that's not enough to refresh the color until next time the page is re-drawn, though, so also manually force the color change
				if ( state == vgui::Label::CS_BRIGHT )
					m_pFieldLabels[i][j]->SetFgColor(GetSchemeColor("Label.TextBrightColor",  scheme));
				else
					m_pFieldLabels[i][j]->SetFgColor(GetSchemeColor("Label.TextDullColor",  scheme));
			}
	}
}