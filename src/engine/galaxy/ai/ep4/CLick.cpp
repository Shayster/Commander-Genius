/*
 * CLick.cpp
 *
 *  Created on: 04.08.2011
 *      Author: gerstrong
 */

#include "CLick.h"

#include "engine/galaxy/ai/CPlayerBase.h"

namespace galaxy {

#define A_LICK_HOP			0
#define A_LICK_LAND			3
#define A_LICK_BREATHE		4
#define A_LICK_STUNNED		12

const int CSF_MIN_DISTANCE_TO_BREATHE = 2<<CSF;
const int CSF_DISTANCE_TO_FOLLOW_TOLERANCE = 2<<CSF;

const int LICK_HOP_X_SPEED = 50;
const int LICK_BREATHE_TIMER = 100;

CLick::CLick(CMap *pmap, Uint32 x, Uint32 y) :
CObject(pmap, x, y, OBJ_NONE),
CStunnable(pmap, x, y, OBJ_NONE),
m_timer(0)
{
	setupGalaxyObjectOnMap(0x2FC6, A_LICK_HOP);
	mp_processState = (void (CStunnable::*)()) (&CLick::processHop);
}

void CLick::process()
{
	(this->*mp_processState)();

	processActionRoutine();

	processFalling();
}

void CLick::getTouchedBy(CObject &theObject)
{
	if(dead || theObject.dead)
		return;

	CStunnable::getTouchedBy(theObject);

	// Was it a bullet? Than make it stunned.
	if( dynamic_cast<CBullet*>(&theObject) )
	{
		mp_processState = &CStunnable::processGettingStunned;
		setAction( A_LICK_STUNNED );
		theObject.dead = true;
		dead = true;
	}

	if( CPlayerBase *player = dynamic_cast<CPlayerBase*>(&theObject) )
	{
		if(getActionNumber(A_LICK_BREATHE))
		{
			player->kill();
		}
	}
}


bool CLick::isNearby(CObject &theObject)
{
	if( CPlayerBase *player = dynamic_cast<CPlayerBase*>(&theObject) )
	{
		const int dx = player->getXMidPos() - getXMidPos();

		if( dx<-CSF_DISTANCE_TO_FOLLOW_TOLERANCE )
			m_hDir = LEFT;
		else if( dx>+CSF_DISTANCE_TO_FOLLOW_TOLERANCE )
			m_hDir = RIGHT;

		if(getActionNumber(A_LICK_LAND))
		{
			int absdx = (dx<0) ? -dx : dx;

			if( absdx < CSF_MIN_DISTANCE_TO_BREATHE )
			{
				setAction(A_LICK_BREATHE);
				playSound(SOUND_LICK_FIREBREATH);
				mp_processState = (void (CStunnable::*)()) (&CLick::processBreathe);
				m_timer = LICK_BREATHE_TIMER;
			}
		}

	}

	return true;
}

void CLick::processHop()
{
	// Move left or right according to set direction
	if(m_hDir == RIGHT)
		moveRight(LICK_HOP_X_SPEED);
	else if(m_hDir == LEFT)
		moveLeft(LICK_HOP_X_SPEED);

	if(blockedd)
	{
		mp_processState = (void (CStunnable::*)()) (&CLick::processLand);
		setAction( A_LICK_LAND );
	}
}

void CLick::processLand()
{
	// After a moment he might hop again
	mp_processState = (void (CStunnable::*)()) (&CLick::processHop);
	setAction( A_LICK_HOP );

	yinertia = -100;
	blockedd = false;
}

void CLick::processBreathe()
{
	// Breathe for a brief moment
	m_timer--;
	if(getActionStatus(A_LICK_HOP+2))
	{
		mp_processState = (void (CStunnable::*)()) (&CLick::processHop);
		setAction( A_LICK_HOP );
	}
}


} /* namespace galaxy */
