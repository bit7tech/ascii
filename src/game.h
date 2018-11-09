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
    char ch;
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

STRUCT_START(SimulateIn)
{
    // Timing
    f64                 dt;

    // Screen meta-data
    int                 width;
    int                 height;

    // Input
    Array(KeyState)     key;
    Array(MouseState)   mouse;
}
STRUCT_END(SimulateIn);

STRUCT_START(PresentIn)
{
    // Visuals
    int                 width;
    int                 height;
    u32*                foreImage;
    u32*                backImage;
    u32*                textImage;
}
STRUCT_END(PresentIn);

//----------------------------------------------------------------------------------------------------------------------
// Game API
//----------------------------------------------------------------------------------------------------------------------

void init();
void done();
bool simulate(const SimulateIn* sim);
void present(const PresentIn* pin);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
