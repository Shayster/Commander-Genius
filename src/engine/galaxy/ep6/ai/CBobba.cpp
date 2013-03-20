#include "CBobba.h"
#include <engine/galaxy/common/ai/CPlayerLevel.h>
#include <misc.h>

/*
$2D86W  #Bobba jumping 0
$2DA4W  #Bobba jumping 
$2DC2W  #Bobba standing 2
$2DE0W  #Bobba standing [Shoot]
$2DFEW  #Bobba eye twinkle 4
$2E1CW  #Bobba eye twinkle
$2E3AW  #Bobba eye twinkle
$2E58W  #Bobba eye twinkle
*/

namespace galaxy
{
    
enum BOBBAACTIONS
{
A_BOBBA_JUMP = 0,
A_BOBBA_SIT = 2,
A_BOBBA_SHOOT = 3,
A_BOBBA_EYE_TWINKLE = 4
};    
    

const int MOVE_X_SPEED = 30;

const int SIT_TIME = 60;

const int MAX_JUMP_INERTIA = -170;

CBobba::CBobba(CMap* pmap, const Uint16 foeID, const Uint32 x, const Uint32 y) : 
CGalaxyActionSpriteObject(pmap, foeID, x, y)
{
	mActionMap[A_BOBBA_JUMP] = (void (CGalaxyActionSpriteObject::*)()) &CBobba::processJumping;
	mActionMap[A_BOBBA_SIT] = (void (CGalaxyActionSpriteObject::*)()) &CBobba::processSitting;
	mActionMap[A_BOBBA_SHOOT] = (void (CGalaxyActionSpriteObject::*)()) &CBobba::processShooting;
	mActionMap[A_BOBBA_EYE_TWINKLE] = (void (CGalaxyActionSpriteObject::*)()) &CBobba::processEyeTwinkle;
	
	setupGalaxyObjectOnMap(0x2D86, A_BOBBA_JUMP);
	
	xDirection = LEFT;
	yinertia = MAX_JUMP_INERTIA;
}


void CBobba::processJumping()
{
	// Move normally in the direction
	if( xDirection == RIGHT )
	{
		moveRight( MOVE_X_SPEED );
	}
	else
	{
		moveLeft( MOVE_X_SPEED );
	}
	
	if(blockedd && yinertia >= 0)
	    setAction(A_BOBBA_SIT);
}


void CBobba::processSitting()
{
    mTimer++;

    if(mTimer < SIT_TIME)
	return;
    
    mTimer = 0;
    
    yinertia = MAX_JUMP_INERTIA;
    
    setAction(A_BOBBA_JUMP);
}


void CBobba::processShooting()
{

}


void CBobba::processEyeTwinkle()
{

}


bool CBobba::isNearby(CSpriteObject& theObject)
{
	if( !getProbability(30) )
		return false;

	if( CPlayerLevel *player = dynamic_cast<CPlayerLevel*>(&theObject) )
	{
		if( player->getXMidPos() < getXMidPos() )
			xDirection = LEFT;
		else
			xDirection = RIGHT;
	}

	return true;
}


void CBobba::getTouchedBy(CSpriteObject& theObject)
{
	if(dead || theObject.dead)
		return;

	if( CPlayerBase *player = dynamic_cast<CPlayerBase*>(&theObject) )
	{
		player->kill();
	}
}


void CBobba::process()
{
	performCollisions();
	
	performGravityHigh();

	if( blockedl )
		xDirection = RIGHT;
	else if(blockedr)
		xDirection = LEFT;

	if(!processActionRoutine())
	    exists = false;
	
	(this->*mp_processState)();
}

  
// TODO: Bobba Fireball
/*
$2E76W  #Bobba fireball [From twinkle] 8
$2E94W  #Bobba fireball
$2EB2W  #Bobba fireball
$2ED0W  #Bobba fireball
$2EEEW  #Bobba fireball 2
$2F0CW  #Bobba fireball 2
$2F2AW  #Bobba fireball 2
 */
  
  
};
