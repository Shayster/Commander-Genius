/*
 * CPlayerLevel.cpp
 *
 *  Created on: 06.08.2010
 *      Author: gerstrong
 */

#include "CPlayerLevel.h"
#include "CBullet.h"
#include "common/CBehaviorEngine.h"
#include "platform/CPlatform.h"
#include "sdl/CInput.h"
#include "sdl/sound/CSound.h"
#include "CVec.h"
#include "CLogFile.h"

namespace galaxy {


const int MAX_JUMPHEIGHT = 10;
const int MIN_JUMPHEIGHT = 5;

const int MAX_POGOHEIGHT = 20;
const int MIN_POGOHEIGHT = 5;
const int POGO_SLOWDOWN = 4;

const int POGO_START_INERTIA_VERT = -100;
const int POGO_START_INERTIA_MAX_VERT = -168;
const int POGO_START_INERTIA_IMPOSSIBLE_VERT = -180;
const int POGO_INERTIA_HOR_MAX = 64;
const int POGO_INERTIA_HOR_REACTION = 2;

const int FIRE_RECHARGE_TIME = 5;


CPlayerLevel::CPlayerLevel(CMap *pmap, Uint32 x, Uint32 y,
						std::vector<CObject*>& ObjectPtrs, direction_t facedir,
						CInventory &l_Inventory, stCheat &Cheatmode) :
CPlayerBase(pmap, x, y, ObjectPtrs, facedir, l_Inventory, Cheatmode),
m_jumpdownfromobject(false)
{
	mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;

	m_fire_recharge_time = 0;
	m_EnterDoorAttempt = false;
	m_ptogglingswitch = false;

	m_jumpheight = 0;
	m_climbing = false;
	m_pogotoggle = false;
	m_jumped = false;
	m_hanging = false;

	setupGalaxyObjectOnMap(0x98C, A_KEEN_STAND);
}




// This special code is important, so platforms in all cases will catch Keen when he is falling on them
void CPlayerLevel::processMoveBitDown()
{
	for( size_t i = 0 ; i<m_ObjectPtrs.size() ; i++ )
	{
		if(m_ObjectPtrs[i]->hitdetect(*this))
		{
			if( CPlatform *platform = dynamic_cast<CPlatform*>(m_ObjectPtrs[i]) )
			{
				platform->getTouchedBy(*this);
			}
		}
	}

	CObject::processMoveBitDown();
}





void CPlayerLevel::processInput()
{
	CPlayerBase::processInput();

	if(m_playcontrol[PA_Y] >= 0)
		m_EnterDoorAttempt = false;
}






void CPlayerLevel::tryToShoot( const VectorD2<int> &pos, const direction_t &dir )
{
	if(m_Inventory.Item.m_bullets > 0)
	{
		m_ObjectPtrs.push_back(new CBullet(mp_Map, pos.x, pos.y, dir));
		m_Inventory.Item.m_bullets--;
	}
	else
	{
		playSound( SOUND_GUN_CLICK );
	}
}




void CPlayerLevel::shootInAir()
{
	// process simple shooting
	if( m_playcontrol[PA_Y] < 0 )
	{
		setActionForce(A_KEEN_JUMP_SHOOTUP);
		const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYUpPos()-(16<<STC));
		tryToShoot(newVec, UP);
	}
	else if( m_playcontrol[PA_Y] > 0 )
	{
		setActionForce(A_KEEN_JUMP_SHOOTDOWN);
		const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYDownPos());
		tryToShoot(newVec, DOWN);
	}
	else
	{
		setActionForce(A_KEEN_JUMP_SHOOT);

		const VectorD2<int> newVec(getXPosition() + ((m_hDir == LEFT) ? -(16<<STC) : (16<<STC)),
									getYPosition()+(4<<STC));
		tryToShoot(newVec, m_hDir);
	}
	mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processFalling;
	m_fire_recharge_time = FIRE_RECHARGE_TIME;
}




bool CPlayerLevel::checkandtriggerforCliffHanging()
{
	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();
	if( m_playcontrol[PA_X]<0 && blockedl )
	{
		bool check_block = TileProperty[mp_Map->at((getXLeftPos()>>CSF)-1, getYUpPos()>>CSF)].bright;
		bool check_block_lower = TileProperty[mp_Map->at((getXLeftPos()>>CSF)-1, (getYUpPos()>>CSF)+1)].bright;
		if(!check_block && check_block_lower &&  mp_processState != (void (CPlayerBase::*)()) &CPlayerLevel::processPogo )
		{
			setAction(A_KEEN_HANG);
			setActionSprite();
			calcBouncingBoxes();
			Uint32 x = (((getXPosition()>>CSF))<<CSF)+(16<<STC);
			Uint32 y = (((getYPosition()>>CSF))<<CSF)+(8<<STC);
			moveTo(x,y);
			solid = false;
			yinertia = 0;
			m_hanging = false;
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processCliffHanging;
			return true;
		}
	}
	else if( m_playcontrol[PA_X]>0 && blockedr )
	{
		bool check_block = TileProperty[mp_Map->at((getXRightPos()>>CSF)+1, getYUpPos()>>CSF)].bleft;
		bool check_block_lower = TileProperty[mp_Map->at((getXRightPos()>>CSF)+1, (getYUpPos()>>CSF)+1)].bleft;
		if(!check_block && check_block_lower && mp_processState != (void (CPlayerBase::*)()) &CPlayerLevel::processPogo )
		{
			setAction(A_KEEN_HANG);
			setActionSprite();
			calcBouncingBoxes();
			Uint32 x = (((getXPosition()>>CSF)+1)<<CSF) + (2<<STC);
			Uint32 y = (((getYPosition()>>CSF)+1)<<CSF) - (5<<STC);
			moveTo(x,y);
			solid = false;
			yinertia = 0;
			m_hanging = false;
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processCliffHanging;
			return true;
		}
	}
	return false;
}







void CPlayerLevel::processCliffHanging()
{
	// In case you released the direction
	if( m_playcontrol[PA_Y] == 0 && m_playcontrol[PA_X] == 0)
		m_hanging = true;

	if(!m_hanging)
		return;

	// If you want to climb up
	if( ((m_hDir == LEFT) && (m_playcontrol[PA_X] < 0)) ||
		((m_hDir == RIGHT) && (m_playcontrol[PA_X] > 0))  )
	{
		setAction(A_KEEN_CLIMB);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processCliffClimbing;
		m_camera.m_freeze = true;
	}

	// If you want to fall down.
	else if( m_playcontrol[PA_Y] > 0 ||
		((m_hDir == LEFT) && (m_playcontrol[PA_X] > 0)) ||
		((m_hDir == RIGHT) && (m_playcontrol[PA_X] < 0))  )
	{
		setAction( A_KEEN_FALL );
		playSound( SOUND_KEEN_FALL );
		solid = true;
		const int dx = 8<<STC;
		moveXDir( (m_hDir == LEFT) ? dx : -dx, true);
		setActionSprite();
		calcBouncingBoxes();
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processFalling;
	}
}






void CPlayerLevel::processCliffClimbing()
{
	const int dy = 24;
	const int dx = dy/3;
	moveUp(dy);
	moveXDir( (m_hDir == LEFT) ? -dx : dx, true);

	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();
	if( getActionStatus(A_KEEN_STAND) )
	{
		const int target_x = getXMidPos()>>CSF;
		const int target_y = getYDownPos()>>CSF;
		const bool noblock = !TileProperty[mp_Map->at(target_x, target_y)].bup;
		if(noblock)
		{
			moveDown(1<<CSF);
			solid = true;
			setAction(A_KEEN_STAND);
			m_camera.m_freeze = false;
			setActionSprite();
			calcBouncingBoxes();
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
		}
	}
}





void CPlayerLevel::processMovingHorizontal()
{
	// Verify facing directions
	if(  m_playcontrol[PA_X]<0  ) // left
		m_hDir = LEFT;
	else if( m_playcontrol[PA_X]>0  ) // right
		m_hDir = RIGHT;

	moveXDir(m_playcontrol[PA_X]>>1);
}






// Here all the pogo code is processed
void CPlayerLevel::processPogo()
{
	if(!m_playcontrol[PA_POGO])
		m_pogotoggle = false;

	// process Shooting in air
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
	{
		xinertia = 0;
		shootInAir();
		return;
	}


	// while button is pressed, make the player jump higher
	if( yinertia<0 && !blockedu )
	{
		if( (m_playcontrol[PA_JUMP] && m_jumpheight <= MAX_POGOHEIGHT) || m_jumpheight <= MIN_POGOHEIGHT )
			m_jumpheight++;

		yinertia += POGO_SLOWDOWN;
	}
	else
	{
		if(blockedu)
			playSound( SOUND_KEEN_BUMPHEAD );

		CObject::processFalling();
		setAction(A_KEEN_POGO_HIGH);

		m_jumpheight = 0;
	}

	// pressed again will make keen fall until hitting the ground
	if(m_playcontrol[PA_POGO] && !m_pogotoggle)
	{
		m_jumpheight = 0;
		setAction(A_KEEN_FALL);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processFalling;
		m_pogotoggle = true;
		xinertia = 0;
	}

	// When keen hits the floor, start the same pogoinertia again!
	if(blockedd)
	{
		if(m_playcontrol[PA_JUMP])
		{
			yinertia = POGO_START_INERTIA_MAX_VERT;
		}
		else
		{
			yinertia = POGO_START_INERTIA_VERT;
		}

		setAction(A_KEEN_POGO_START);
		m_jumpheight = 0;
		playSound( SOUND_KEEN_POGO );
	}

	moveYDir(yinertia);

	// Verify facing directions. Here we build up the inertia
	if(  m_playcontrol[PA_X]<0  ) // left
	{
		m_hDir = LEFT;
		if( xinertia > -POGO_INERTIA_HOR_MAX)
			xinertia -= POGO_INERTIA_HOR_REACTION;
	}
	else if( m_playcontrol[PA_X]>0  ) // right
	{
		m_hDir = RIGHT;
		if( xinertia < +POGO_INERTIA_HOR_MAX)
			xinertia += POGO_INERTIA_HOR_REACTION;
	}


	// Check if we are hitting walls
	if( blockedr && xinertia > 0 ) // to the right
	{
		xinertia -= POGO_INERTIA_HOR_REACTION;
		if( xinertia < 0 )
			xinertia = 0;
	}
	else if( blockedl && xinertia < 0 ) // left
	{
		xinertia += POGO_INERTIA_HOR_REACTION;
		if( xinertia > 0 )
			xinertia = 0;
	}


	moveXDir(xinertia);
}










// Processes the jumping of the player
void CPlayerLevel::processJumping()
{
	// while button is pressed, make the player jump higher.
	// The jump is limited unless he is in jump cheat mode
	if( (m_playcontrol[PA_JUMP] && (m_Cheatmode.jump || m_jumpheight <= MAX_JUMPHEIGHT))
			|| m_jumpheight <= MIN_JUMPHEIGHT )
	{
		m_jumpheight++;
		yinertia = -140;
	}
	else
	{
		if( yinertia<0 && !blockedu )
			yinertia+=10;
		else
			m_jumpheight = 0;
	}

	// Set another jump animation if Keen is near yinertia == 0
	if( yinertia > -10 )
	{
		if( getActionNumber(A_KEEN_JUMP) )
			setAction(A_KEEN_JUMP+1);
	}

	moveYDir(yinertia);

	// If the max. height is reached or the player cancels the jump by release the button
	// make keen fall
	if( m_jumpheight == 0 )
	{
		// Check whether we should bump the head
		if( blockedu )
			playSound( SOUND_KEEN_BUMPHEAD );

		yinertia -= 20;
		if( getActionNumber(A_KEEN_JUMP) )
		{
			setAction(A_KEEN_FALL);
		}
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processFalling;
	}

	processMovingHorizontal();

	// Now in this mode we could switch to pogoing
	if( !m_pogotoggle && m_playcontrol[PA_POGO] )
	{
		m_jumpheight = 0;
		setAction(A_KEEN_POGO_UP);
		m_pogotoggle = true;
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processPogo;
		return;
	}

	// Check if keen should stick to the pole
	if( m_playcontrol[PA_Y] < 0 )
	{
		verifyforPole();
	}

	// process Shooting in air
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
		shootInAir();
}






bool CPlayerLevel::canFallThroughTile()
{
	const int &tile = mp_Map->getPlaneDataAt(1, getXMidPos(), getYDownPos()+(2<<STC));
	const CTileProperties &TileProp = g_pBehaviorEngine->getTileProperties(1)[tile];
	return ( TileProp.bdown == 0 && TileProp.bup != 0 );
}





const int MAX_SCROLL_VIEW = (8<<CSF);

void CPlayerLevel::processLookingUp()
{
	// While looking up, Keen could shoot up!
	// He could shoot
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
	{
		setActionForce(A_KEEN_SHOOT+2);
		const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYUpPos()-(16<<STC));
		tryToShoot(newVec, UP);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processShootWhileStanding;
		m_fire_recharge_time = FIRE_RECHARGE_TIME;
		return;
	}

	if( m_camera.m_relcam.y > -MAX_SCROLL_VIEW )
		m_camera.m_relcam.y -= (2<<STC);

	if( m_playcontrol[PA_Y]<0 )
		return;

	setAction(A_KEEN_STAND);
	mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
}







// Processes the exiting of the player. Here all cases are held
void CPlayerLevel::processExiting()
{
	Uint32 x = getXMidPos();
	if( ((mp_Map->m_width-2)<<CSF) < x || (2<<CSF) > x )
	{
		CEventContainer& EventContainer = g_pBehaviorEngine->m_EventList;
		EventContainer.add( new EventExitLevel(mp_Map->getLevel(), true) );
	}
}








#define		MISCFLAG_SWITCHPLATON 5
#define 	MISCFLAG_SWITCHPLATOFF 6
#define		MISCFLAG_SWITCHBRIDGE 15
#define		MISCFLAG_DOOR		2
#define		MISCFLAG_KEYCARDDOOR		32

void CPlayerLevel::processPressUp() {

	std::vector<CTileProperties> &Tile = g_pBehaviorEngine->getTileProperties(1);
	const int x_left = getXLeftPos();
	const int x_right = getXRightPos();
	const int x_mid = (x_left+x_right)/2;
	const int up_y = getYUpPos()+(3<<STC);

	const Uint32 tile_no = mp_Map->getPlaneDataAt(1, x_mid, up_y);
	int flag = Tile[tile_no].behaviour;

	// pressing a switch
	if (flag==MISCFLAG_SWITCHPLATON || flag == MISCFLAG_SWITCHPLATOFF ||
		flag == MISCFLAG_SWITCHBRIDGE)
	{
		playSound( SOUND_GUN_CLICK );
		setAction(A_KEEN_SLIDE);
		if(flag == MISCFLAG_SWITCHBRIDGE)
		{
			Uint32 newtile;
			if(Tile[tile_no+1].behaviour == MISCFLAG_SWITCHBRIDGE)
				newtile = tile_no+1;
			else
				newtile = tile_no-1;

			mp_Map->setTile( x_mid>>CSF, up_y>>CSF, newtile, true, 1); // Wrong tiles, those are for the info plane
			PressBridgeSwitch(x_mid, up_y);
		}
		else
		{
			const Uint32 newtile = (flag == MISCFLAG_SWITCHPLATON) ? tile_no+1 : tile_no-1 ;
			mp_Map->setTile( x_mid>>CSF, up_y>>CSF, newtile, true, 1); // Wrong tiles, those are for the info plane
			PressPlatformSwitch(x_mid, up_y);
		}
		setAction(A_KEEN_SLIDE);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processSliding;
		m_timer = 0;
		return;
	 }
/*		var2 = o->boxTXmid*256-64;
		if (o->xpos == var2) {
			setAction(ACTION_KEENENTERSLIDE);
			setAction(A_KEEN_SLIDE);
		} else {
			o->time = var2;
			//setAction(ACTION_KEENPRESSSWITCH);
			setAction(ACTION_KEENENTERSLIDE);
		}
		EnterDoorAttempt = 1;
		return 1;
	} */

	int flag_left = Tile[mp_Map->getPlaneDataAt(1, x_left, up_y)].behaviour;

	// entering a door

	if ( !m_EnterDoorAttempt && (flag_left == MISCFLAG_DOOR || flag_left == MISCFLAG_KEYCARDDOOR) )
	{
		//int var2 = mid_x * 256+96;
		int flag_right = Tile[mp_Map->getPlaneDataAt(1, x_left+(1<<CSF), up_y)].behaviour;
		//if (flag2 == MISCFLAG_DOOR || flag2 == MISCFLAG_KEYCARDDOOR) var2-=256;
		//if (getXPosition() == var2) {
		if(flag_right == MISCFLAG_DOOR || flag_right == MISCFLAG_KEYCARDDOOR) {
			/*if (flag == MISCFLAG_KEYCARDDOOR) {
				if (security_card) {
					security_card = 0;
					SD_PlaySound(SOUND_OPENSECURITYDOOR);
					GetNewObj(0);
					new_object->xpos = o->boxTXmid-2;
					new_object->ypos = o->boxTY2-4;
					new_object->active = 2;
					new_object->clipping = 0;
					new_object->type = 1;
					new_object->action = ACTION_SECURITYDOOROPEN;
					check_ground(new_object, ACTION_SECURITYDOOROPEN);
					o->action = ACTION_KEENENTERDOOR0;
					o->int16 = 0;
					return 1;
				} else {
					SD_PlaySound(SOUND_NOOPENSECURITYDOOR);
					o->action = ACTION_KEENSTAND;
					EnterDoorAttempt = 1;
					return 0;
				}
			} else {*/
				setAction(A_KEEN_ENTER_DOOR);
				setActionSprite();
				CSprite &rSprite = g_pGfxEngine->getSprite(sprite);

				// Here the Player will be snapped to the center

				const int x_l = (x_left>>CSF);
				const int x_r = x_l+1;
				const int x_mid = ( ((x_l+x_r)<<CSF) - (rSprite.getWidth()<<STC)/2 )/2;

				moveToHorizontal(x_mid);

				mp_processState = static_cast<void (CPlayerBase::*)()>(&CPlayerLevel::processEnterDoor);
				m_EnterDoorAttempt = true;
				return;
				//PlayLoopTimer = 110;
				//o->action = ACTION_KEENENTERDOOR1
				//o->int16 = 0;
				//if (*MAPSPOT(o->boxTXmid, o->boxTY1, INFOPLANE) == 0) sub_1FE94();
			//}
		}// else {
			//o->time = var2;
			//o->action = ACTION_KEENENTERSLIDE;
		//}
		//EnterDoorAttempt = 1;
	}

	// If the above did not happen, then just look up
	mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processLookingUp;
	setAction(A_KEEN_LOOKUP);
}






void CPlayerLevel::processPressDucking()
{
	if( m_playcontrol[PA_Y]>0 )
	{
		const bool jumpdowntile = canFallThroughTile();
		if ( m_playcontrol[PA_JUMP] > 0 && ( supportedbyobject || jumpdowntile )  )
		{
			m_jumpdownfromobject = supportedbyobject;
			m_jumpdown = jumpdowntile;
			supportedbyobject = false;
			blockedd = false;
			setAction(A_KEEN_FALL);
			playSound( SOUND_KEEN_FALL );
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processFalling;
		}

		if( m_camera.m_relcam.y < MAX_SCROLL_VIEW )
			m_camera.m_relcam.y += (2<<STC);

		return;
	}

	setAction(A_KEEN_STAND);
	mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
}







void CPlayerLevel::processSliding()
{
	if(m_timer < 10)
	{
		m_timer++;
		return;
	}
	else
	{
		m_timer = 0;
		setAction(A_KEEN_STAND);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
	}
}







void CPlayerLevel::processEnterDoor()
{
	moveUp(16);

	if(!getActionStatus(A_KEEN_STAND))
		return;

	setAction(A_KEEN_STAND);
	mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;

	int xmid = getXMidPos();
	int y1 = getYMidPos();

	Uint32 t = mp_Map->getPlaneDataAt(2, xmid, y1);
	if (t == 0) {
		//level_state = 13;
		//o->action = ACTION_KEENENTEREDDOOR;
		// TODO: Figure out what this does
		return;
	}

	if (t == 0xB1B1) {
		//level_state = 2;
		//o->action = ACTION_KEENENTEREDDOOR;
		// TODO: Figure out what this does
		return;
	}

	const int ypos = ((t%256 - 1))<<CSF;
	const int xpos = (t >> 8)<<CSF;

	VectorD2<int> new_pos(xpos, ypos);
	moveToForce(new_pos);
	new_pos.x += ((m_BBox.x2-m_BBox.x1)/2);
	new_pos.y += ((m_BBox.y2-m_BBox.y1)/2);
	m_camera.setPosition(new_pos);

	//o->ypos = TILE2MU(*t%256 - 1) + 15;
	//o->xpos = (*t >> 8 << 8);
	//o->int16 = 1;
	//o->clipping = 0;
	//set_sprite_action(o, (o->action)->next);
	//o->clipping = 1;
	//sub_183F1(o);
	return;

}










/**
 * This function checks whether a bridge must be opened or closed and does this kind of work
 * I'm not really happy with that part of the code and I know that it works for Keen 4. Not sure about the
 * other episodes, but it's well though and should...
 */


void CPlayerLevel::PressBridgeSwitch(const Uint32 lx, const Uint32 ly)
{
	Uint32 targetXY = mp_Map->getPlaneDataAt(2, lx, ly);

	Uint32 newX = targetXY >> 8;
	Uint32 newY = targetXY & 0xFF;

	const int start_tile = mp_Map->getPlaneDataAt(1, newX<<CSF, newY<<CSF)-1;
	const int end_tile = start_tile+3;

	/// We found the start of the row, that need to be changed.
	/// Let apply it to the rest of the bridge
	// Apply to the borders

	// bridge opened or closed?
	const bool b_opened = (start_tile%8 < 4) ?true : false;

	int x = newX;
	for(int t = start_tile ;  ; x++ )
	{
		// Now decide whether the tile is a piece or borders of the bridge
		const Uint32 type = t%18;

		if(type < 16)
		{
			if(b_opened)
				t += 4;
			else
				t -= 4;
		}
		else
		{
			// It is just a normal piece remove
			t = (t/18)*18;
			if(b_opened)
				t+=16;
			else
				t+=17;
		}
		const Uint32 NewTile = t;
		t = mp_Map->getPlaneDataAt(1, x<<CSF, newY<<CSF);

		mp_Map->setTile(x-1, newY, NewTile, true, 1);
		mp_Map->setTile(x-1, newY+1, NewTile+18, true, 1);

		if(t == end_tile)
		{
			if(t%8 < 4)
				// This bridge is opened, close it!
				t += 4;
			else
				// This bridge is closed, open it!
				t -= 4;

			Uint32 new_lasttile = end_tile;
			if(b_opened)
				new_lasttile += 4;
			else
				new_lasttile -= 4;

			mp_Map->setTile(x-1, newY+1, new_lasttile+17, true, 1);
			mp_Map->setTile(x, newY, new_lasttile, true, 1);
			mp_Map->setTile(x, newY+1, new_lasttile+18, true, 1);
			break;
		}
	}

	return;
}






/**
 * This function will put/release the blockers of some platforms used in Keen Galaxy
 */
void CPlayerLevel::PressPlatformSwitch(const Uint32 lx, const Uint32 ly)
{
	Uint32 targetXY = mp_Map->getPlaneDataAt(2, lx, ly);

	Uint32 newX = targetXY >> 8;
	Uint32 newY = targetXY & 0xFF;

	if(mp_Map->getPlaneDataAt(2, newX<<CSF, newY<<CSF) == 31)
		mp_Map->setTile(newX, newY, 0, true, 2);
	else
		mp_Map->setTile(newX, newY, 31, true, 2);

	return;
}



void CPlayerLevel::openDoorsTile()
{
	int lx = getXMidPos();
	int ly = getYDownPos()-(3<<STC);
	Uint32 targetXY = mp_Map->getPlaneDataAt(2, lx, ly);

	Uint32 newX = targetXY >> 8;
	Uint32 newY = targetXY & 0xFF;
	Uint32 tileno, next_tileno;

	do
	{
		tileno = mp_Map->getPlaneDataAt(1, newX<<CSF, newY<<CSF);
		mp_Map->setTile(newX, newY, tileno+1, true, 1);
		newY++;
		next_tileno = mp_Map->getPlaneDataAt(1, newX<<CSF, newY<<CSF);
	}while(next_tileno == tileno || next_tileno == tileno+18 || next_tileno == tileno+2*18);
}



void CPlayerLevel::processPlaceGem()
{
	if(m_timer == 0 || m_timer == 5 || m_timer == 8)
	{
		openDoorsTile();
	}

	if(m_timer < 10)
	{
		m_timer++;
		return;
	}

	m_timer = 0;

	setAction(A_KEEN_STAND);
	mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;

	int lx = getXMidPos();
	int ly = getYDownPos()-(3<<STC);

	Uint32 tileno = mp_Map->getPlaneDataAt(1, lx, ly);
	mp_Map->setTile(lx>>CSF, ly>>CSF, tileno+18, true, 1);
}









void CPlayerLevel::processPoleClimbing()
{
	// If player is still pressing the fire button wait until he releases it!
	if(m_fire_recharge_time)
		return;

	// This will cancel the pole process and make Keen jump
	if( !m_jumped && m_playcontrol[PA_JUMP] > 0 )
	{
		setAction(A_KEEN_JUMP);

		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
		m_climbing = false;
		m_jumped = true;
		yinertia = 0;
		m_vDir = NONE;
		solid = true;
		return;
	}
	else if(m_playcontrol[PA_JUMP] == 0)
	{
		// Ensures that the button must be pressed again for another jump
		m_jumped = false;
	}

	// Lets check if Keen can move up, down or reaches the end of the pole
	Uint32 l_x = ( getXLeftPos() + getXRightPos() ) / 2;
	Uint32 l_y_up = getYUpPos()-(7<<STC);

	if( m_playcontrol[PA_Y] < 0 )
	{
		// First check player pressed shoot button
		if( m_playcontrol[PA_FIRE] )
		{
			m_fire_recharge_time = FIRE_RECHARGE_TIME;
			setActionForce(A_KEEN_POLE_SHOOTUP);
			const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYUpPos()-(16<<STC));
			tryToShoot(newVec, UP);
			return;
		}


		// Check for the upper side and don't let him move if the pole ends
		if( hitdetectWithTileProperty(1, l_x, l_y_up) )
		{
			setAction(A_KEEN_POLE_CLIMB);
			m_vDir = UP;
		}
		else
		{
			setAction(A_KEEN_POLE);
			m_vDir = NONE;
		}
	}
	else if( m_playcontrol[PA_Y] > 0 )
	{
		// First check player pressed shoot button
		if( m_playcontrol[PA_FIRE] )
		{
			m_fire_recharge_time = FIRE_RECHARGE_TIME;
			setActionForce(A_KEEN_POLE_SHOOTDOWN);
			const VectorD2<int> newVec(getXMidPos()-(3<<STC), getYDownPos());
			tryToShoot(newVec, DOWN);
			return;
		}

		l_y_up = getYUpPos()+(16<<STC);

		Uint32 l_y_down = getYDownPos()/*+(7<<STC)*/;
		if(!hitdetectWithTileProperty(1, l_x, l_y_down))
			solid = true;
		else
			solid = false;

		// Check for the and upper and lower side, upper because the hand can touch the edge in that case
		const bool up = hitdetectWithTileProperty(1, l_x, l_y_up);
		if( up && !blockedd )
		{
			// Slide down if there is more of the pole
			setAction(A_KEEN_POLE_SLIDE);
			m_vDir = DOWN;
		}
		else
		{
			// Fall down if there isn't any pole to slide down
			m_climbing = false;
			m_vDir = NONE;
			yinertia = 0;
			solid = true;

			const bool down = mp_Map->at(l_x>>CSF, l_y_down>>CSF);

			if(!blockedd && !down)
			{
				setAction(A_KEEN_FALL);
				playSound( SOUND_KEEN_FALL );
				mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processFalling;
			}
			else
			{
				blockedd = true;
				moveUp(1<<CSF);
				moveDown(1<<CSF);

				setAction(A_KEEN_STAND);
				mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
			}
		}
	}
	else
	{
		// First check player pressed shoot button
		if( m_playcontrol[PA_FIRE] )
		{
			m_fire_recharge_time = FIRE_RECHARGE_TIME;
			setActionForce(A_KEEN_POLE_SHOOT);
			const VectorD2<int> newVec(getXPosition() + ((m_hDir == LEFT) ? -(16<<STC) : (16<<STC)),
										getYPosition()+(4<<STC));
			tryToShoot(newVec, m_hDir);
			return;
		}

		setAction(A_KEEN_POLE);
		m_vDir = NONE;
	}
}






bool CPlayerLevel::verifyforPole()
{
	Uint32 l_x = ( getXLeftPos() + getXRightPos() ) / 2;
	Uint32 l_y_up = ( getYUpPos() );
	Uint32 l_y_down = ( getYDownPos() );


	// Now check if Player has the chance to climb a pole or something similar
	if( ( m_playcontrol[PA_Y] < 0 && hitdetectWithTileProperty(1, l_x, l_y_up) ) ||
		( m_playcontrol[PA_Y] > 0 && hitdetectWithTileProperty(1, l_x, l_y_down) ) ) // 1 -> stands for pole Property
	{
		// Hit pole!
		// calc the proper coord of that tile
		l_x = (l_x>>CSF)<<CSF;
		if( ( m_playcontrol[PA_Y] < 0 && hitdetectWithTileProperty(1, l_x, l_y_up) ) ||
			( m_playcontrol[PA_Y] > 0 && hitdetectWithTileProperty(1, l_x, l_y_down) ) )
		{
			// Move to the proper X Coordinates, so Keen really grabs it!
			moveTo(VectorD2<int>(l_x - (7<<STC), getYPosition()));
			xinertia = 0;

			// Set Keen in climb mode
			setAction(A_KEEN_POLE);
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processPoleClimbing;
			m_climbing = true;
			solid = false;
			return true;
		}
	}
	return false;
}






void CPlayerLevel::processStanding()
{
	/// Keen is standing

	// Center the view after Keen looked up or down
	centerView();

	// He could walk
	if(  m_playcontrol[PA_X]<0  )
	{
		if( !blockedl )
		{
			// prepare him to walk to the left
			m_hDir = LEFT;
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processRunning;
			setAction(A_KEEN_RUN);
		}
	}
	else if(  m_playcontrol[PA_X]>0  )
	{
		if( !blockedr )
		{
			// prepare him to walk to the right
			m_hDir = RIGHT;
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processRunning;
			setAction(A_KEEN_RUN);
		}
	}

	// He could jump
	if( m_playcontrol[PA_JUMP] )
	{
		// Not jumping? Let's see if we can prepare the player to do so
		yinertia = -140;
		m_jumpheight = 0;
		setAction(A_KEEN_JUMP);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
		m_jumped = true;
		m_climbing = false;
		m_vDir = NONE;
		playSound( SOUND_KEEN_JUMP );
	}


	// He could duck or use the pole
	if( m_playcontrol[PA_Y] > 0 )
	{
		if(!verifyforPole())
		{
			setAction(A_KEEN_LOOKDOWN);
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processPressDucking;
		}
	}

	// He could press up and do further actions
	if( m_playcontrol[PA_Y] < 0 )
	{
		if(!verifyforPole())
		{
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processPressUp;
		}
	}

	// He could shoot
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
	{
		setAction(A_KEEN_SHOOT);
		const VectorD2<int> newVec(getXPosition() + ((m_hDir == LEFT) ? -(16<<STC) : (16<<STC)),
									getYPosition()+(4<<STC));
		tryToShoot(newVec, m_hDir);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processShootWhileStanding;
		m_fire_recharge_time = FIRE_RECHARGE_TIME;
		return;
	}

	// He could use pogo
	if( m_playcontrol[PA_POGO] )
	{
		if(m_playcontrol[PA_JUMP])
			yinertia = POGO_START_INERTIA_IMPOSSIBLE_VERT;
		else
			yinertia = POGO_START_INERTIA_VERT;

		m_jumpheight = 0;
		setAction(A_KEEN_POGO_START);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processPogo;
		playSound( SOUND_KEEN_POGO );
		m_pogotoggle = true;
	}

	CObject::processFalling();
}




void CPlayerLevel::processShootWhileStanding()
{
	// while until player releases the button and get back to stand status
	if( !m_playcontrol[PA_FIRE] )
	{
		if(getActionNumber(A_KEEN_SHOOT+2))
		{
			setAction(A_KEEN_LOOKUP);
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processLookingUp;
		}
		else
		{
			setAction(A_KEEN_STAND);
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
		}
	}
}






void CPlayerLevel::processRunning()
{
	// Most of the walking routine is done by the action script itself

	// Center the view after Keen looked up or down
	centerView();

	// He could stand again, if player doesn't move the dpad
	if( m_playcontrol[PA_X] == 0 )
	{
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
		setAction(A_KEEN_STAND);
	}
	// or he could change the walking direction
	else if( m_playcontrol[PA_X]<0 ) // left
	{
		// Is he blocked make him stand, else continue walking
		if( blockedl )
		{
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
			setAction(A_KEEN_STAND);
		}
		else
		{
			// walk to the left
			m_hDir = LEFT;
			makeWalkSound();
		}
	}
	else if( m_playcontrol[PA_X]>0 ) // right
	{
		// Is he blocked make him stand, else continue walking
		if( blockedr )
		{
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
			setAction(A_KEEN_STAND);
		}
		else
		{
			// walk to the right
			m_hDir = RIGHT;
			makeWalkSound();
		}
	}

	if( verifyForFalling() )
	{
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processFalling;
		setAction(A_KEEN_FALL);
		playSound( SOUND_KEEN_FALL );
	}


	// He could jump
	if( m_playcontrol[PA_JUMP] )
	{
		// Not jumping? Let's see if we can prepare the player to do so
		yinertia = -140;
		m_jumpheight = 0;
		setAction(A_KEEN_JUMP);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
		m_jumped = true;
		m_climbing = false;
		m_vDir = NONE;
		playSound( SOUND_KEEN_JUMP );
	}

	// He could shoot
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
	{
		const int newx = getXPosition() + ((m_hDir == LEFT) ? -(16<<STC) : (16<<STC));
		const int newy = getYPosition()+(4<<STC);

		const VectorD2<int> newVec(newx, newy);
		tryToShoot(newVec, m_hDir);

		setAction(A_KEEN_SHOOT);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processShootWhileStanding;
		m_fire_recharge_time = FIRE_RECHARGE_TIME;
		return;
	}

	// He could use pogo
	if( m_playcontrol[PA_POGO] )
	{
		if(m_playcontrol[PA_JUMP])
			yinertia = POGO_START_INERTIA_IMPOSSIBLE_VERT;
		else
			yinertia = POGO_START_INERTIA_VERT;

		m_jumpheight = 0;
		setAction(A_KEEN_POGO_START);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processPogo;
		playSound( SOUND_KEEN_POGO );
		m_pogotoggle = true;
	}

	// He could place a gem
	for( Uint32 i=7 ; i<=10 ; i++ )
	{
		const int l_x = getXMidPos();
		const int l_y = getYDownPos()-(3<<STC);

		// This will change the gemholder to a holder with the gem
		if( hitdetectWithTileProperty(i, l_x, l_y) )
		{
			stItemGalaxy &m_Item = m_Inventory.Item;

			if(i == 7 && m_Item.m_gem.red > 0)
				m_Item.m_gem.red--;
			else if(i == 8 && m_Item.m_gem.yellow > 0)
				m_Item.m_gem.yellow--;
			else if(i == 9 && m_Item.m_gem.blue > 0)
				m_Item.m_gem.blue--;
			else if(i == 10 && m_Item.m_gem.green > 0)
				m_Item.m_gem.green--;
			else
				break;

			moveToHorizontal((l_x>>CSF)<<CSF);
			setAction(A_KEEN_SLIDE);
			mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processPlaceGem;
			playSound( SOUND_DOOR_OPEN );
		}
	}

}







// Falling code
void CPlayerLevel::processFalling()
{
	CObject::processFalling();

	// If keen is jumping down, not because he did from an object like a platform,
	// but a tile where Keen can fall through, process this part of code and
	// check if Keen is still jumpinto through any object
	if(!supportedbyobject && m_jumpdown)
	{
		if(!canFallThroughTile())
			m_jumpdown = false;
	}

	if(blockedd)
	{
		setAction(A_KEEN_STAND);
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processStanding;
	}

	processMovingHorizontal();

	// If Jump mode is enabled he can jump again
	// This will cancel the pole process and make Keen jump
	if( m_Cheatmode.jump && m_playcontrol[PA_JUMP] > 0 )
	{
		setAction(A_KEEN_JUMP);

		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processJumping;
		m_climbing = false;
		m_jumped = true;
		yinertia = 0;
		m_vDir = NONE;
		return;
	}

	/// While falling Keen could switch to pogo again anytime
	// but first the player must release the pogo button
	if( !m_playcontrol[PA_POGO] )
		m_pogotoggle = false;

	// Now we can check if player wants to use it again
	if( !m_pogotoggle && m_playcontrol[PA_POGO] )
	{
		m_jumpheight = 0;
		yinertia = 0;
		setAction(A_KEEN_POGO_START);
		m_pogotoggle = true;
		mp_processState = (void (CPlayerBase::*)()) &CPlayerLevel::processPogo;
	}

	// Check if keen should stick to the pole
	if( m_playcontrol[PA_Y] < 0 )
	{
		verifyforPole();
	}

	// Check Keen could hang on a cliff and do so if possible
	if(checkandtriggerforCliffHanging())
		return;
	if( m_playcontrol[PA_FIRE] && !m_fire_recharge_time )
		shootInAir();

}



void CPlayerLevel::centerView()
{
	// If keen looked up or down, this will return the camera to initial position
	if( m_camera.m_relcam.y < 0 )
	{
		m_camera.m_relcam.y += (4<<STC);
		if( m_camera.m_relcam.y > 0 )
			m_camera.m_relcam.y = 0;
	}
	else if( m_camera.m_relcam.y > 0 )
	{
		m_camera.m_relcam.y -= (4<<STC);
		if( m_camera.m_relcam.y < 0 )
			m_camera.m_relcam.y = 0;
	}
}




void CPlayerLevel::process()
{
	if(!m_dying)
	{
		processInput();

		// If no clipping was triggered change solid state of keen
		if(m_Cheatmode.noclipping)
		{
			solid = !solid;
			m_Cheatmode.noclipping = false;
		}

		if(supportedbyobject)
		{
			blockedd = true;
		}
	}

	(this->*mp_processState)();

	// make the fire recharge time decreased if player is not pressing firing button
	if(m_fire_recharge_time && !m_playcontrol[PA_FIRE])
	{
		m_fire_recharge_time--;
	}

	processLevelMiscFlagsCheck();

	processActionRoutine();

	if(!m_dying)
	{
		processExiting();

		m_camera.process();
		m_camera.processEvents();

		performCollisions();

		void (CPlayerBase::*PogoProcess)() = static_cast<void (CPlayerBase::*)()>(&CPlayerLevel::processPogo);
		void (CPlayerBase::*FallProcess)() = static_cast<void (CPlayerBase::*)()>(&CPlayerLevel::processFalling);

		// It's not always desired to push out
		if( (mp_processState != PogoProcess) &&
			(mp_processState != FallProcess) )
		{
			processPushOutCollision();
		}
	}
}


}
