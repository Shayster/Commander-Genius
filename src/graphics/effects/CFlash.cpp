/*
 * CFlash.cpp
 *
 *  Created on: 14.12.2009
 *      Author: gerstrong
 */

#include "CFlash.h"
#include "../../sdl/CVideoDriver.h"

/*#if SDL_VERSION_ATLEAST(2, 0, 0)
CFlash::CFlash(Uint32 msecs, Uint8 speed, Uint32 color) :
    m_StartTime(gTimer.getTicks()),
    m_RunTime(msecs),
    m_Speed(speed),
    m_Color(color),
    m_Alpha(0),
    m_FadeDir(FADE_IN),
    m_Style(FADE_PULSE),
    m_MaxAlpha(255)//,
  //mpFadeSurface(SDL_DisplayFormat(g_pVideoDriver->getBlitSurface()), &SDL_FreeSurface)
{
    SDL_FillRect(mpFadeSurface.get(), NULL, m_Color);
}
#else*/
CFlash::CFlash(Uint32 msecs, Uint8 speed, Uint32 color) :
    m_StartTime(gTimer.getTicks()),
    m_RunTime(msecs),
    m_Speed(speed),
    m_Color(color),
    m_Alpha(0),
    m_FadeDir(FADE_IN),
    m_Style(FADE_PULSE),
    m_MaxAlpha(255),
    mpFadeSurface(g_pVideoDriver->convertThroughBlitSfc(g_pVideoDriver->getBlitSurface()), &SDL_FreeSurface)
{
    SDL_FillRect(mpFadeSurface.get(), NULL, m_Color);
}
//#endif



/*#if SDL_VERSION_ATLEAST(2, 0, 0)
CFlash::CFlash(Uint32 msecs, Uint8 speed, Uint32 color, Uint8 m_maxalpha) :
    m_StartTime(gTimer.getTicks()),
    m_RunTime(msecs),
    m_Speed(speed),
    m_Color(color),
    m_Alpha(0),
    m_FadeDir(FADE_IN),
    m_Style(FADE_NORMAL),
    m_MaxAlpha(m_maxalpha)//,
  //mpFadeSurface(SDL_DisplayFormat(g_pVideoDriver->getBlitSurface()), &SDL_FreeSurface)
{
    SDL_FillRect(mpFadeSurface.get(), NULL, m_Color);
}
#else*/
CFlash::CFlash(Uint32 msecs, Uint8 speed, Uint32 color, Uint8 m_maxalpha) :
    m_StartTime(gTimer.getTicks()),
    m_RunTime(msecs),
    m_Speed(speed),
    m_Color(color),
    m_Alpha(0),
    m_FadeDir(FADE_IN),
    m_Style(FADE_NORMAL),
    m_MaxAlpha(m_maxalpha),
    mpFadeSurface(g_pVideoDriver->convertThroughBlitSfc(g_pVideoDriver->getBlitSurface()), &SDL_FreeSurface)
{
    SDL_FillRect(mpFadeSurface.get(), NULL, m_Color);
}
//#endif

// Process the flashing effect here
void CFlash::ponder()
{
    Uint32 ElapsedTime = gTimer.getTicks() - m_StartTime;

#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_SetSurfaceAlphaMod(mpFadeSurface.get(), m_Alpha);
#else
    SDL_SetAlpha(mpFadeSurface.get(), SDL_SRCALPHA, m_Alpha);
#endif

    if(m_FadeDir == FADE_IN)
    {
        if(m_Alpha+m_Speed > m_MaxAlpha) m_Alpha = m_MaxAlpha;
        else m_Alpha+=m_Speed;
    }
    if(m_FadeDir == FADE_OUT)
    {
        if(m_Alpha+m_Speed < 0) m_Alpha = 0;
        else m_Alpha-=m_Speed;
    }

    if(m_Style == FADE_PULSE)
    {
        if(m_Alpha == 255) m_FadeDir = FADE_OUT;
        else if(m_Alpha == 0) m_FadeDir = FADE_IN;
    }
    else if(m_Style == FADE_NORMAL)
    {
        if(ElapsedTime >= m_RunTime/2 ) m_FadeDir = FADE_OUT;
        if(m_FadeDir == FADE_OUT && m_Alpha == 0) mFinished = true;
    }

    // The developer set a time in the constructor. This effect will last for the given time.
    if(ElapsedTime >= m_RunTime)
        mFinished = true;
}

void CFlash::render(const float deltaT)
{
    // Blit it and free temp surface
    SDL_BlitSurface( mpFadeSurface.get(), NULL,
                     g_pVideoDriver->getBlitSurface(), NULL );
}
