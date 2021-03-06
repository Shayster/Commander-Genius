/*
 * CSelectionMenu.h
 *
 *  Created on: 14.08.2010
 *      Author: gerstrong
 *
 *  Based on the work of Pizza2004
 *  This is template for normal selection menus. It can tell what item the user selected
 */

#ifndef CSELECTIONMENU_H_
#define CSELECTIONMENU_H_

#include <string>
#include <list>
#include "CBaseMenu.h"

template <class T_Out>
class CSelectionMenu : public CBaseMenu {
public:
	CSelectionMenu(T_Out &output_selection,
			const std::list<std::string> item_list,
			const Uint8 dlg_theme,
			const Uint8 defaultselection = 0) :
		CBaseMenu(dlg_theme),
		m_selecteditem(output_selection)
	{
		Uint16 width, height;

		width = 0;
		std::list<std::string>::const_iterator item = item_list.begin();
		for(; item != item_list.end() ; item++)
			if( width < item->size()+5 )
				width = item->size()+5;

		height = item_list.size()+2;

		mp_Dialog = new CDialog(width, height, INPUT_MODE_UP_DOWN, dlg_theme);

		item = item_list.begin();
		for(size_t pos=1; item != item_list.end() ; item++, pos++)
			mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, pos, *item);

		mp_Dialog->setSelection(defaultselection);
	}

	void processSpecific()
	{
		if( m_selection != NO_SELECTION)
		{
			m_selecteditem = m_selection + 1;
			m_mustclose = true;
		}
	}

	virtual ~CSelectionMenu()
	{
		if(mp_Dialog)
			delete mp_Dialog;
		mp_Dialog = NULL;
	}

private:
	T_Out &m_selecteditem;
};


#endif /* CSELECTIONMENU_H_ */
