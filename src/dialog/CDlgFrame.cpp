/*
 * CDlgFrame.cpp
 *
 *  Created on: 20.08.2009
 *      Author: gerstrong
 */

#include "CDlgFrame.h"
#include "graphics/CGfxEngine.h"



CDlgFrame::CDlgFrame(int x, int y, int w, int h, Uint8 theme, int tilewidth, int tileheight) {
	
	m_8x8tileheight = tilewidth;
	m_8x8tilewidth = tileheight;
	
	m_x = x;
	m_y = y;
	resize(w, h);
	
	m_theme = theme;
}


int CDlgFrame::TileWidth()
{
	return m_8x8tilewidth;
}


void CDlgFrame::draw(SDL_Surface *dst)
{
	if( m_theme == DLG_THEME_VORTICON )
	{
		drawOldSchoolFrame(dst);
	}
	else if(m_theme == DLG_THEME_RED)
	{
		drawRedFrame(dst);
	}
	else if(m_theme == DLG_THEME_GALAXY)
	{
		drawGalaxyFrame(dst);
	}
}



void CDlgFrame::resize(const Uint32 width, const Uint32 height)
{
	m_w = width; m_h = height;
}




void CDlgFrame::setPos(const Uint32 x, const Uint32 y)
{
	m_x = x ; m_y = y;
}




void CDlgFrame::drawOldSchoolFrame(SDL_Surface *dst)
{
	CFont &Font = g_pGfxEngine->getFont(0);

	// first draw the blank rect
	int i, j;

	SDL_FillRect(dst, NULL, 0xFFFFFFFF);

	for(j = 0 ; j < m_h - m_8x8tileheight ; j+= m_8x8tileheight )
		Font.drawCharacter(dst, 32, m_w - m_8x8tilewidth, m_y + j); // for the last tile

	// then the borders
	for( i = m_8x8tilewidth ; i < m_w-m_8x8tilewidth ; i+= m_8x8tilewidth )
	{
		Font.drawCharacter(dst, 2,  i, 0); // 2 is one upper-border
		Font.drawCharacter(dst, 7,  i, m_h - m_8x8tileheight); // 7 is also the lower-border
	}
	Font.drawCharacter(dst, 2,  m_w - m_8x8tilewidth, 0); // for the last tile
	Font.drawCharacter(dst, 2,  m_w - m_8x8tilewidth, m_h - m_8x8tileheight); // for the last tile
	for( j = m_8x8tileheight ; j < m_h-m_8x8tileheight ; j+= m_8x8tileheight )
	{
		Font.drawCharacter(dst, 4, 0, j ); // 4 is one left-border
		Font.drawCharacter(dst, 5, 0 + m_w - m_8x8tilewidth, j ); // 5 is the right-border
	}

	// At last the corners
	Font.drawCharacter(dst, 1, 0, 0 ); // Upper-Left corner
	Font.drawCharacter(dst, 3, m_w - m_8x8tilewidth, 0 ); // Upper-Right corner
	Font.drawCharacter(dst, 6, 0, m_h - m_8x8tileheight ); // Lower-Left corner
	Font.drawCharacter(dst, 8, m_w - m_8x8tilewidth, m_h - m_8x8tileheight ); // Lower-Right corner
}

void CDlgFrame::drawGalaxyFrame(SDL_Surface *dst)
{
	// first draw the blank rect
	SDL_Rect rect;
	rect.x = m_x + m_8x8tilewidth;
	rect.y = m_y + m_8x8tileheight;
	rect.w = m_w - m_8x8tilewidth*2;
	rect.h = m_h - m_8x8tileheight*2;

	SDL_FillRect(dst, &rect, 0xFFFFFF);

	CTilemap &Tilemap = g_pGfxEngine->getTileMap(3);

	/// Now draw the borders
	// Upper Left corner
	Tilemap.drawTile(dst, m_x, m_y, 0);

	// Upper border
	for(int x=m_8x8tilewidth ; x<(m_w-m_8x8tilewidth) ; x+=m_8x8tilewidth)
		Tilemap.drawTile(dst, m_x+x, m_y, 1);

	// Upper Right corner
	Tilemap.drawTile(dst, m_x+m_w-m_8x8tilewidth, m_y, 2);

	// Left border
	for(int y=m_8x8tileheight ; y<(m_h-m_8x8tileheight) ; y+=m_8x8tileheight)
		Tilemap.drawTile(dst, m_x, m_y+y, 3);

	// Right border
	for(int y=m_8x8tileheight ; y<(m_h-m_8x8tileheight) ; y+=m_8x8tileheight)
		Tilemap.drawTile(dst, m_x+m_w-m_8x8tilewidth, m_y+y, 5);

	// Lower Left corner
	Tilemap.drawTile(dst, m_x, m_y+m_h-m_8x8tileheight, 6);

	// Lower border
	for(int x=m_8x8tilewidth ; x<(m_w-m_8x8tilewidth) ; x+=m_8x8tilewidth)
		Tilemap.drawTile(dst, m_x+x, m_y+m_h-m_8x8tileheight, 7);

	// Lower Right corner
	Tilemap.drawTile(dst, m_x+m_w-m_8x8tilewidth, m_y+m_h-m_8x8tileheight, 8);
}


void CDlgFrame::drawRedFrame(SDL_Surface *dst)
{
	CFont &Font = g_pGfxEngine->getFont(0);
	
	// first draw the blank rect
	int i, j;
	for(j = 0 ; j < m_h - m_8x8tileheight ; j+= m_8x8tileheight )
	{
		for(i = 0 ; i < m_w - m_8x8tilewidth ; i+= m_8x8tilewidth )
			Font.drawCharacter(dst, 32, m_x + i, m_y + j); // 32 is a blank tile
		Font.drawCharacter(dst, 32,m_x + m_w - m_8x8tilewidth, m_y + j); // for the last tile
	}
	
	// then the borders
	for( i = m_8x8tilewidth ; i < m_w-m_8x8tilewidth ; i+= m_8x8tilewidth )
	{
		Font.drawCharacter(dst, 24, m_x + i, m_y); // 2 is one upper-border
		Font.drawCharacter(dst, 29, m_x + i, m_y + m_h - m_8x8tileheight); // 7 is also the lower-border
	}
	Font.drawCharacter(dst, 24, m_x + m_w - m_8x8tilewidth, m_y); // for the last tile
	Font.drawCharacter(dst, 24, m_x + m_w - m_8x8tilewidth, m_y + m_h - m_8x8tileheight); // for the last tile
	for( j = m_8x8tileheight ; j < m_h-m_8x8tileheight ; j+= m_8x8tileheight )
	{
		Font.drawCharacter(dst, 26, m_x, m_y + j ); // 4 is one left-border
		Font.drawCharacter(dst, 27, m_x + m_w - m_8x8tilewidth, m_y + j ); // 5 is the right-border
	}
	
	// At last the corners
	Font.drawCharacter(dst, 23, m_x, m_y ); // Upper-Left corner
	Font.drawCharacter(dst, 25, m_x + m_w - m_8x8tilewidth, m_y ); // Upper-Right corner
	Font.drawCharacter(dst, 28, m_x, m_y + m_h - m_8x8tileheight ); // Lower-Left corner
	Font.drawCharacter(dst, 30, m_x + m_w - m_8x8tilewidth, m_y + m_h - m_8x8tileheight ); // Lower-Right corner
}
