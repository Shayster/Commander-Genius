/*
 * CLoadMenu.h
 *
 *  Created on: 22.05.2010
 *      Author: gerstrong
 */

#ifndef CLOADMENU_H_
#define CLOADMENU_H_

#include "CBaseMenu.h"
#include "fileio/CSavedGame.h"

class CLoadMenu : public CBaseMenu
{
public:
	CLoadMenu(Uint8 dlg_theme, CSavedGame &SavedGame);
	void processSpecific();
	virtual ~CLoadMenu();

private:
	CSavedGame &m_SavedGame;
};

#endif /* CLOADMENU_H_ */
