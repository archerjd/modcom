#ifndef DATATABLE_H
#define DATATABLE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include "modcom/modules.h"

#define MAX_COLUMNS 12
#define MAX_ROWS 11

// should be given name, a function reference? start, end & current values
class DataTable : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE( DataTable, vgui::EditablePanel );
public:
	DataTable( vgui::Panel *pParent, int columns, int rows );
	void SetRowName(int row, wchar_t *name);
	void SetValue(int column, int row, wchar_t *value);
	void Update(int highlightColumn);
protected:
	//virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint();

private:
	int m_iColumns, m_iRows, m_iNameWidthOffset, m_iCellWidth;
	vgui::Label *m_pRowHeadings[MAX_ROWS];
	vgui::Label *m_pFieldLabels[MAX_COLUMNS][MAX_ROWS];
};

#endif