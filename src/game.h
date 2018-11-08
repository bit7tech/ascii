//----------------------------------------------------------------------------------------------------------------------
//! @file       game.h
//! @brief      Game interface for platform layer.
//! @author     Matt Davies
//! @copyright  Copyright (C)2018 Bit-7 Technology, all rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <kore/kore.h>

#define STRUCT_START(name) typedef struct _##name
#define STRUCT_END(name) name, *name##Ref

STRUCT_START(KeyState)
{
    bool down;
    bool shift;
    bool ctrl;
    bool alt;
    int vkey;
}
STRUCT_END(KeyState);

STRUCT_START(MouseState)
{
    bool leftDown;
    bool rightDown;
    int x;
    int y;
}
STRUCT_END(MouseState);

STRUCT_START(GameIn)
{
    // Timing
    f64                 dt;

    // Visuals
    int                 width;
    int                 height;
    u32*                foreImage;
    u32*                backImage;
    u32*                textImage;

    // Input
    Array(KeyState)     key;
    Array(MouseState)   mouse;
}
STRUCT_END(GameIn);

//----------------------------------------------------------------------------------------------------------------------
// Game API
//----------------------------------------------------------------------------------------------------------------------

void init();
void done();
bool tick(const GameIn* gameIn);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
