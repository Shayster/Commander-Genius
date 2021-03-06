/*
 * CPlayerBase.h
 *
 *  Created on: 19.06.2011
 *      Author: gerstrong
 *
 *  This is the base class for the Player. This class is very new and I thought of it for
 *  the following reason. We have Keen on the Map in level and when diving.
 *  Those AIs are different of course, but still many things in common like dying
 *  (See the puzzle Keen 5 mod versions by Mink) and collecting items.
 *  It will not only reduce the code of some repeating routines, but also
 *  provide better overview over the whole thing.
 */

#ifndef CPLAYERBASE_H_
#define CPLAYERBASE_H_

#include "common/CObject.h"
#include "common/Playerdefines.h"
#include "engine/CEvent.h"
#include "common/Cheat.h"
#include "engine/CCamera.h"
#include "engine/galaxy/CInventory.h"

namespace galaxy
{


#define A_KEEN_DIE				29


class CPlayerBase : public CObject {
public:
	/**
	 * \description Basic Constructor. An Object always need the pointer to the Map of the level,
	 * 				Coordinates as also the type of the Object
	 */
	CPlayerBase(CMap *pmap,
				Uint32 x,
				Uint32 y,
				std::vector<CObject*>& ObjectPtrs,
				direction_t facedir,
				CInventory &l_Inventory,
				stCheat &Cheatmode);

	/**
	 * \brief The Player will get 1 UP when that function is launched
	 */
	void getAnotherLife(const int &lc_x, const int &lc_y);

	/**
	 * \description Read the Input of the Player and sets the variables accordingly
	 */
	virtual void processInput();

	void processLevelMiscFlagsCheck();

	/**
	 * Produces the walking sound. To produce that sound correctly, it uses a special timer.
	 * That's the reason we need that aiding function
	 */
	void makeWalkSound();

	/**
	 * Same here but with swimming
	 */
	void makeSwimSound();


	void processDead();
	virtual void processDying();
	void kill();

	/**
	 * Happens when usually the Dopefish eats Keen
	 */
	void getEaten();

	/**
	 * Process the cycle when keen is getting eaten
	 */
	void processGetEaten();

	CInventory &m_Inventory;

	CCamera m_camera;

protected:

	std::vector<CObject*>& m_ObjectPtrs;

	char m_playcontrol[PA_MAX_ACTIONS];
	int m_timer;

	//CCamera m_camera;
	bool m_dying;

	stCheat& m_Cheatmode;

	void (CPlayerBase::*mp_processState)();

private:
	unsigned char m_walktimer;
};

};

#endif /* CPLAYERBASE_H_ */
