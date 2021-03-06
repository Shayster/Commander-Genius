/*
 * CPassiveVort.cpp
 *
 *  Created on: 07.03.2010
 *      Author: gerstrong
 */

#include "CPassiveVort.h"

#include "graphics/CGfxEngine.h"
#include "engine/vorticon/CMenuVorticon.h"
#include "common/CMapLoader.h"
#include "common/CTileProperties.h"
#include "sdl/CVideoDriver.h"
#include "sdl/CInput.h"

namespace vorticon
{

CPassiveVort::CPassiveVort() :
mp_Map(NULL),
mp_Option(g_pBehaviorEngine->m_option),
m_RestartVideo(false)
{
	mp_IntroScreen = NULL;
	mp_TitleScreen = NULL;
	mp_PressAnyBox=NULL;
	m_GoDemo = false;
	m_hideobjects = false;
	m_textsize = 0;
}

bool CPassiveVort::init(char mode)
{
	mp_Scrollsurface = g_pVideoDriver->mp_VideoEngine->getScrollSurface();
	m_mode = mode;
	if( m_mode == INTRO )
	{
		mp_IntroScreen = new CIntro();
		mp_Map = new CMap;
		CMapLoader MapLoader( mp_Map );
		MapLoader.load( m_Episode, 90, m_DataDirectory);
		mp_Map->gotoPos( 64+5*320, 32); // Coordinates of star sky
		mp_Map->drawAll();
		mp_IntroScreen->init();
		mp_Map->changeTileArrayY(8, 15, 2, 560);
	}
	else if( m_mode == TITLE )
	{
		mp_Map = new CMap;
		CMapLoader MapLoader( mp_Map );
		MapLoader.load( m_Episode, 90, m_DataDirectory);
		mp_Map->gotoPos( 32, 32 ); // Coordinates of title screen
		mp_Map->drawAll();
		mp_TitleScreen = new CTitle(m_object, *mp_Map);
		mp_TitleScreen->init(m_Episode);
	}
	else if( m_mode == DEMO )
	{
		// TODO: Setup the demo environment
	}
	else
		return false;

	return true;
}

void CPassiveVort::process()
{
	// Open the Main-Menu or close the opened one?
	if( !mp_Menu )
	{
		if (mp_PressAnyBox==NULL && m_mode == TITLE)
		{
			mp_PressAnyBox = new CTextBox(150, 10," PRESS ANY KEY ");
			mp_PressAnyBox->setAttribs(0, true);
			mp_PressAnyBox->enableBorders(true);
		}

		if ( g_pInput->getPressedAnyKey() || g_pInput->getPressedAnyCommand() )
		{
			// Close the "Press Any Key" box
			if(g_pGfxEngine->Effect()) // some effect is running? Close it!
				g_pGfxEngine->killEffect();

			g_pInput->flushAll();
			if (m_mode != TITLE)
			{
				cleanup();
				init(TITLE);
			}
			else
			{
				SAFE_DELETE(mp_PressAnyBox);
				mp_Menu = new CMenuVorticon( PASSIVE, *mp_Map, m_SavedGame,
						m_RestartVideo, m_hideobjects);
			}
		}
	}
	else // Close menu
	{
		if ( mp_Menu->m_demoback )
		{
			SAFE_DELETE(mp_Menu);
			mp_Map->drawAll();
		}
		else if( m_RestartVideo )
		{
			cleanup();
			init(m_mode);
			m_RestartVideo = false;
		}
	}

	// Animate the tiles
	mp_Map->animateAllTiles();

	// If Menu is open show it!
	if( mp_Menu )
	{
		mp_Menu->processSpecific();

		if(mp_Menu->mustStartGame() || m_SavedGame.getCommand() == CSavedGame::LOAD) // Start a normal game
		{
			m_NumPlayers = mp_Menu->getNumPlayers();
			m_Difficulty = mp_Menu->getDifficulty();
			SAFE_DELETE(mp_Menu);
			cleanup();
			m_mode = STARTGAME;
		}
		else if(mp_Menu->getExitEvent())
		{
			SAFE_DELETE(mp_Menu);
			cleanup();
			m_mode = SHUTDOWN;
		}
		else if(mp_Menu->getChooseGame())
		{
			SAFE_DELETE(mp_Menu);
			m_modeg = true;
		}
		else if( m_RestartVideo ) // When some video settings has been changed
		{
			cleanup();
			init(m_mode);
			m_RestartVideo = false;
		}
	}

	// Blit the background
	g_pVideoDriver->blitScrollSurface();

	// Modes. We have three: Intro, Main-tile and Demos. We could add more.
	if( m_mode == INTRO )
	{
		// Intro code goes here!
		mp_IntroScreen->process();

		if( mp_IntroScreen->isFinished() )
		{
			// Shutdown mp_IntroScreen and show load Title Screen
			cleanup();
			init(TITLE);
		}
	}
	else if( m_mode == TITLE )
	{
		mp_TitleScreen->process();

		if( !mp_Menu )
		{
			if( mp_TitleScreen->isFinished() )
			{
				// The Title screen was shown enough time, shut it down
				// and load Demo environment
				cleanup();
				init(DEMO);
				return;
			}
		}
	}
	else if( m_mode == DEMO )
	{
		// TODO: Demo modes are processed here!
		// TODO: Implement Demos here!
		cleanup();
		init(TITLE);
	}

	if(!m_hideobjects)
	{
		// Make the Objects do its jobs
		std::vector<CObject*>::iterator i;
		for( i=m_object.begin() ; i!=m_object.end() ; i++ )
		{
			(*i)->process();
		}
	}

	// If Menu is not open show "Press Any Key"
	if(mp_PressAnyBox != NULL)
		mp_PressAnyBox->process();

}

void CPassiveVort::cleanup()
{
	if(!m_object.empty())
	{
		for(std::vector<CObject*>::iterator obj = m_object.begin() ; obj!=m_object.end() ; obj++)
			SAFE_DELETE(*obj);
		m_object.clear();
	}

	SAFE_DELETE(mp_IntroScreen);
	SAFE_DELETE(mp_TitleScreen);
	SAFE_DELETE(mp_Map);
}

}
