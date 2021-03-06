/*
 * CMessageBoxVort.h
 *
 *  Created on: 30.03.2011
 *      Author: gerstrong
 */

#ifndef CMESSAGEBOXVORT_H_
#define CMESSAGEBOXVORT_H_

#include "dialog/CMessageBox.h"

class CMessageBoxVort : public CMessageBox {
public:
	CMessageBoxVort(const std::string& Text, bool lower = false, bool keymsg = false, bool leftbound = false);

	virtual ~CMessageBoxVort();

	/**
	 * \brief This will add an extra tile to the message box. Mostly used by the dialog "Ship missing" in Keen 1
	 */
	void addTileAt(Uint16 tile, Uint16 x, Uint16 y);

	/**
	 * \brief This is called every time, the box needs to be redrawn.
	 */
	void process();

private:
	std::vector<TileHolder> m_Tiles;

	Uint8 m_twirltimer;
	Uint8 m_twirlframe;

	SDL_Surface *MsgBoxSfc;
};

#endif /* CMESSAGEBOXVORT_H_ */
