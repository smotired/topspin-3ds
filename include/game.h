/// Functionality and variables necessary for the game itself, i.e. not the menus or anything
#pragma once

#include "component.h"

extern Entity thePlayer;

// Initialize the game
void GameInit();

// Run one tick of the game
void GameTick(float dt);

// Clean up the game
void GameDeinit();