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

STRUCT_START(Command)
{
    Region doCmd;
    Region undoCmd;
    int width;
    int height;
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
    int             cmdIndex;
}
STRUCT_END(World);

World gWorld;

//----------------------------------------------------------------------------------------------------------------------
// Region control
//----------------------------------------------------------------------------------------------------------------------

// Copies a rectangle from a source 2D array with width 'srcWidth' to a destination 2D array with dimensions w, h.
void copyToRegion(int x, int y, int w, int h, int srcWidth, int srcHeight, const u32* src, u32* dst)
{
    blit(dst, sizeMake(w, h), src, sizeMake(srcWidth, srcHeight), 0, 0, x, y, w, h, (int)sizeof(u32));
}

void copyFromRegion(int x, int y, int w, int h, int dstWidth, int dstHeight, const u32* src, u32* dst)
{
    blit(dst, sizeMake(dstWidth, dstHeight), src, sizeMake(w, h), x, y, 0, 0, w, h, (int)sizeof(u32));
}

// Initialise a region and optionally copy data from the screen
void newRegion(Region* reg, int x, int y, int w, int h, bool copyScreen)
{
    reg->fore = K_ALLOC(w * h * sizeof(u32));
    reg->back = K_ALLOC(w * h * sizeof(u32));
    reg->text = K_ALLOC(w * h * sizeof(u32));
    reg->x = x;
    reg->y = y;
    reg->w = w;
    reg->h = h;
    if (copyScreen)
    {
        copyToRegion(x, y, w, h, gWorld.screen.w, gWorld.screen.h, gWorld.screen.fore, reg->fore);
        copyToRegion(x, y, w, h, gWorld.screen.w, gWorld.screen.h, gWorld.screen.back, reg->back);
        copyToRegion(x, y, w, h, gWorld.screen.w, gWorld.screen.h, gWorld.screen.text, reg->text);
    }
}

internal void killRegion(RegionRef region)
{
    K_FREE(region->fore, region->w * region->h * sizeof(u32));
    K_FREE(region->back, region->w * region->h * sizeof(u32));
    K_FREE(region->text, region->w * region->h * sizeof(u32));
}

void applyRegion(Region* reg)
{
    copyFromRegion(reg->x, reg->y, reg->w, reg->h, gWorld.screen.w, gWorld.screen.h, reg->fore, gWorld.screen.fore);
    copyFromRegion(reg->x, reg->y, reg->w, reg->h, gWorld.screen.w, gWorld.screen.h, reg->back, gWorld.screen.back);
    copyFromRegion(reg->x, reg->y, reg->w, reg->h, gWorld.screen.w, gWorld.screen.h, reg->text, gWorld.screen.text);
}

void prepareScreen(int x, int y, int w, int h)
{
    int newW = x + w;
    int newH = y + h;
    if (newW > gWorld.screen.w ||
        newH > gWorld.screen.h)
    {
        newW = K_MAX(newW, gWorld.screen.w);
        newH = K_MAX(newH, gWorld.screen.h);
        u32* fore = K_ALLOC(newW * newH * sizeof(u32));
        u32* back = K_ALLOC(newW * newH * sizeof(u32));
        u32* text = K_ALLOC(newW * newH * sizeof(u32));

        u32* f = fore;
        u32* b = back;
        u32* t = text;

        for (int i = 0; i < (newW * newH); ++i)
        {
            *f++ = 0xff0000ff;
            *b++ = 0xff000000;
            *t++ = (u32)' ';
        }

        if (gWorld.screen.fore)
        {
            blit(
                fore, sizeMake(newW, newH),
                gWorld.screen.fore, sizeMake(gWorld.screen.w, gWorld.screen.h),
                0, 0, 0, 0, gWorld.screen.w, gWorld.screen.h, sizeof(u32));
            blit(
                back, sizeMake(newW, newH),
                gWorld.screen.back, sizeMake(gWorld.screen.w, gWorld.screen.h),
                0, 0, 0, 0, gWorld.screen.w, gWorld.screen.h, sizeof(u32));
            blit(
                text, sizeMake(newW, newH),
                gWorld.screen.text, sizeMake(gWorld.screen.w, gWorld.screen.h),
                0, 0, 0, 0, gWorld.screen.w, gWorld.screen.h, sizeof(u32));
        }

        killRegion(&gWorld.screen);
        gWorld.screen.w = newW;
        gWorld.screen.h = newH;
        gWorld.screen.fore = fore;
        gWorld.screen.back = back;
        gWorld.screen.text = text;
    }
}

internal void deleteCommand(int index)
{
    Command* cmd = &gWorld.commands[index];
    killRegion(&cmd->doCmd);
    killRegion(&cmd->undoCmd);
}

Command* newCommand(int x, int y, int w, int h)
{
    prepareScreen(x, y, w, h);

    // Delete all commands after the current index
    while (arrayCount(gWorld.commands) > gWorld.cmdIndex)
    {
        deleteCommand(gWorld.cmdIndex);
    }

    Command* cmd = arrayNew(gWorld.commands);
    gWorld.cmdIndex = (int)arrayCount(gWorld.commands);

    newRegion(&cmd->undoCmd, x, y, w, h, YES);
    newRegion(&cmd->doCmd, x, y, w, h, NO);

    return cmd;
}

//----------------------------------------------------------------------------------------------------------------------
// Commands
//----------------------------------------------------------------------------------------------------------------------

void commandLetter(int x, int y, char c)
{
    Command* cmd = newCommand(x, y, 1, 1);
    *cmd->doCmd.fore = 0xffffffff;
    *cmd->doCmd.back = 0xff000000;
    *cmd->doCmd.text = (u32)c;
    applyRegion(&cmd->doCmd);
    ++gWorld.x;
}

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
    killRegion(&gWorld.screen);
    arrayFor(gWorld.commands)
    {
        killRegion(&gWorld.commands[i].doCmd);
        killRegion(&gWorld.commands[i].undoCmd);
    }
    arrayDone(gWorld.commands);
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

                if (!kev->vkey && (kev->ch >= ' ' && kev->ch < 127))
                {
                    commandLetter(gWorld.x, gWorld.y, (char)kev->ch);
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
