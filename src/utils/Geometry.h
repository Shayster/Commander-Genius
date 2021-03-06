/*
 * Geometry.h
 *
 *  Created on: 29.10.2011
 *      Author: gerstrong
 *  Header files for Geometry and some functions which will help in some calculations
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <SDL.h>

/**
 * This structure defines the resolution composed of width height and depth
 */
struct CRect : public SDL_Rect
{
	CRect( const Uint16 lwidth = 0, const Uint16 lheight = 0 )
		{ w=lwidth; h=lheight; };

	CRect( const SDL_VideoInfo* InfoPtr )
		{ w=InfoPtr->current_w; h=InfoPtr->current_w; };

	bool operator==( const CRect &target )
	{
		return (target.x == x && target.y == y &&
				target.w == w && target.h == h);
	}

	float aspectRatio() const
	{
		return (float(w)/float(h));
	}
};

#endif /* GEOMETRY_H_ */
