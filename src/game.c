//----------------------------------------------------------------------------------------------------------------------
//! @file       game.c
//! @brief      Game main loop.
//! @author     Matt Davies
//! @copyright  Copyright (C)2018 Bit-7 Technology, all rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include <game.h>

//----------------------------------------------------------------------------------------------------------------------
// Initialisation
//----------------------------------------------------------------------------------------------------------------------

void init()
{

}

//----------------------------------------------------------------------------------------------------------------------
// Shutdown
//----------------------------------------------------------------------------------------------------------------------

void done()
{

}

//----------------------------------------------------------------------------------------------------------------------
// Tick
//----------------------------------------------------------------------------------------------------------------------

bool tick(const GameIn* gameIn)
{
    bool result = YES;
    static f64 t = 0;
    static int x = 0;
    static const f64 clock = 0.1;

    for (int i = 0; i < gameIn->width * gameIn->height; ++i)
    {
        gameIn->foreImage[i] = 0xff0000ff;
        gameIn->backImage[i] = 0xff000000;
        gameIn->textImage[i] = (u32)'.';
    }

    t += gameIn->dt;
    if (t > clock)
    {
        //t -= clock;
        t = 0.0;
        ++x;
        if (x >= gameIn->width) x = 0;
    }

    gameIn->textImage[gameIn->width * 2 + x] = (u32)'@';
    gameIn->foreImage[gameIn->width * 2 + x] = 0xff00ff00;

    i64 numKeyEvents = arrayCount(gameIn->key);
    if (numKeyEvents)
    {
        for (i64 i = 0; i < numKeyEvents; ++i)
        {
            KeyState* kev = &gameIn->key[i];
            if (kev->down && kev->vkey == VK_ESCAPE) result = NO;
        }
    }

    return result;
}