/*
 * CBubbles.cpp
 *
 *  Created on: 29.07.2011
 *      Author: gerstrong
 */

#include "CBubbles.h"

namespace galaxy {

#define A_BUBBLE_MOVE		0

const int BUBBLE_FLOAT_SPEED = 40;

CBubbles::CBubbles(CMap *pmap, Uint32 x, Uint32 y, const bool bigbubble) :
CObject(pmap, x, y, OBJ_NONE)
{

	if(bigbubble)
	{
		// The huge Burp Bubble by the Dopefish
		setupGalaxyObjectOnMap(0x36EC, A_BUBBLE_MOVE);
	}
	else
	{
		// The bubbles spawn by Keen. There are 4 Types which are random
		int action_type = rand() % 4;
		setupGalaxyObjectOnMap(0x1A64, action_type);
	}

	m_hDir = NONE;
	m_vDir = UP;
	honorPriority = false;
	solid = false;
}

void CBubbles::process()
{
	moveUp(BUBBLE_FLOAT_SPEED);

	processActionRoutine();
}

} /* namespace galaxy */
