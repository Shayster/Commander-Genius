/*
 * CDiveSuit.cpp
 *
 *  Created on: 25.06.2011
 *      Author: gerstrong
 */

#include "CDiveSuit.h"
#include "engine/galaxy/ai/CPlayerLevel.h"

namespace galaxy {

CDiveSuit::CDiveSuit(CMap *pmap, Uint32 x, Uint32 y) :
CObject(pmap, x, y, OBJ_NONE),
taken(false)
{
	sprite = 306;
	calcBouncingBoxes();
}

void CDiveSuit::process()
{}

void CDiveSuit::getTouchedBy(CObject &theObject)
{
	if(taken)
		return;

	// When Keen touches the Council Member exit the level and add one to the council list
	if(theObject.m_type == OBJ_PLAYER)
	{
		CPlayerLevel &Player = static_cast<CPlayerLevel&>(theObject);
		bool &swimsuit = Player.m_Inventory.Item.m_special.ep4.swimsuit;

		CEventContainer& EventContainer = g_pBehaviorEngine->m_EventList;

		g_pSound->playSound( SOUND_GET_WETSUIT, PLAY_PAUSEALL );
		taken = swimsuit = true;

		EventContainer.add( new EventSendBitmapDialogMsg(106,
				g_pBehaviorEngine->getString("SWIM_SUIT_TEXT"), LEFT) );

		EventContainer.add( new EventExitLevel(mp_Map->getLevel(), true) );
	}
}


} /* namespace galaxy */
