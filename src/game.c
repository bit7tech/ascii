//----------------------------------------------------------------------------------------------------------------------
//! @file       game.c
//! @brief      Game main loop.
//! @author     Matt Davies
//! @copyright  Copyright (C)2018 Bit-7 Technology, all rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include <game.h>

//----------------------------------------------------------------------------------------------------------------------
// World
//----------------------------------------------------------------------------------------------------------------------

STRUCT_START(Region)
{
    u32*    fore;
    u32*    back;
    u32*    text;
    int     x, y;
    int     w, h;
}
STRUCT_END(Region);

typedef enum _CommandType
{
    COMMAND_Letter,
}
CommandType;

STRUCT_START(Command)
{
    CommandType type;
    Region doCmd;
    Region undoCmd;
}
STRUCT_END(Command);

STRUCT_START(World)
{
    f64             t;          // Timer
    int             x, y;       // Cursor coords
    bool            showHelp;   // YES = show help
    bool            cursorOn;   // Cursor is currently flashing on.
    Region          screen;     // Current screen
    Array(Command)  commands;   // Undo stack
}
STRUCT_END(World);

World gWorld;

//----------------------------------------------------------------------------------------------------------------------
// Commands
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// Initialisation
//----------------------------------------------------------------------------------------------------------------------

void init()
{
    memoryClear(&gWorld, sizeof(World));
}

//----------------------------------------------------------------------------------------------------------------------
// Shutdown
//----------------------------------------------------------------------------------------------------------------------

void done()
{
    K_FREE(gWorld.screen.fore, gWorld.screen.w * gWorld.screen.h * sizeof(u32));
    K_FREE(gWorld.screen.back, gWorld.screen.w * gWorld.screen.h * sizeof(u32));
    K_FREE(gWorld.screen.text, gWorld.screen.w * gWorld.screen.h * sizeof(u32));
}

//----------------------------------------------------------------------------------------------------------------------
// Simulate
//----------------------------------------------------------------------------------------------------------------------

bool simulate(const SimulateIn* sim)
{
    bool result = YES;
    static const f64 clock = 0.25;

    gWorld.t += sim->dt;
    if (gWorld.t > clock)
    {
        //t -= clock;
        gWorld.t = 0.0;
        gWorld.cursorOn = !gWorld.cursorOn;
    }

    i64 numKeyEvents = arrayCount(sim->key);
    if (numKeyEvents)
    {
        //
        // Handle keyboard
        //
        for (i64 i = 0; i < numKeyEvents; ++i)
        {
            KeyState* kev = &sim->key[i];
            if (kev->down)
            {
                if (!kev->shift && !kev->ctrl && !kev->alt) switch(kev->vkey)
                {
                case VK_ESCAPE:
                    result = NO;
                    break;

                case VK_LEFT:   --gWorld.x;     break;
                case VK_RIGHT:  ++gWorld.x;     break;
                case VK_UP:     --gWorld.y;     break;
                case VK_DOWN:   ++gWorld.y;     break;
                }

                if (kev->shift && !kev->ctrl && !kev->alt) switch (kev->vkey)
                {
                case VK_LEFT:   gWorld.x -= 10;     break;
                case VK_RIGHT:  gWorld.x += 10;     break;
                case VK_UP:     gWorld.y -= 10;     break;
                case VK_DOWN:   gWorld.y += 10;     break;
                }
            }
        }
        
        // Keep cursor in bounds
        if (gWorld.x < 0) gWorld.x = 0;
        if (gWorld.y < 0) gWorld.y = 0;
        if (gWorld.x >= sim->width) gWorld.x = sim->width - 1;
        if (gWorld.y >= sim->height) gWorld.y = sim->height - 1;
    }

    return result;
}

//----------------------------------------------------------------------------------------------------------------------
// Present
//----------------------------------------------------------------------------------------------------------------------

void present(const PresentIn* pin)
{
    int row, col;
    // f = fore, b = back, t = text
    // d = destination, s = source
    u32* fd = pin->foreImage;
    u32* bd = pin->backImage;
    u32* td = pin->textImage;
    u32* fs = gWorld.screen.fore;
    u32* bs = gWorld.screen.back;
    u32* ts = gWorld.screen.text;

    int dStride = gWorld.screen.w < pin->width ? (pin->width - gWorld.screen.w) : 0;
    int sStride = gWorld.screen.w < pin->width ? 0 : (gWorld.screen.w - pin->width);

    for (row = 0; row < K_MIN(gWorld.screen.h, pin->height); ++row)
    {
        for (col = 0; col < K_MIN(gWorld.screen.w, pin->width); ++col)
        {
            *fd++ = *fs++;
            *bd++ = *bs++;
            *td++ = *ts++;
        }
        for (; col < pin->width; ++col)
        {
            *fd++ = 0xff0000ff;
            *bd++ = 0xff000000;
            *td++ = (u32)'.';
        }
        fd += dStride;
        bd += dStride;
        td += dStride;
        fs += sStride;
        bs += sStride;
        ts += sStride;
    }
    for (; row < pin->height; ++row)
    {
        for (col = 0; col < pin->width; ++col)
        {
            *fd++ = 0xff0000ff;
            *bd++ = 0xff000000;
            *td++ = (u32)'.';
        }
    }

    if (gWorld.cursorOn)
    {
        if (gWorld.x >= 0 && gWorld.y >= 0 && gWorld.x < pin->width && gWorld.y < pin->height)
        {
            pin->foreImage[gWorld.y * pin->width + gWorld.x] = 0xffffffff;
            pin->backImage[gWorld.y * pin->width + gWorld.x] = 0xff0000ff;
        }
    }
}
