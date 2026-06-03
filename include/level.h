/// Definitions for the tilemap and game level system
#pragma once

#include "component.h"
#include "systems.h"

typedef const struct {
    // Each char represents a row of 8 cells. Written from left to right, i.e. greater magnitude bits are further left cells.
    // In row-major order from the top left corner of the world. The bottom left corner of the bottom left cell should be (0,0)
    unsigned char* data;
    // Size of the tilemap in cells (not bytes).
    Vec2Int levelSize;
} Tilemap;

// The current map of ground tiles
extern Tilemap groundTilemap;
// The current map of goop tiles
extern Tilemap goopTilemap;

// True if the ground tilemap has a block at this position
bool HasGroundTile(Vec2Int position);

// True if the goop tilemap has an entry at this position
bool HasGoopTile(Vec2Int position);

typedef const struct {
    // A list of c-strings defining the contents of the levels.
    //   - air
    // # - ground
    // - - semisolid platform
    // ~ - goop
    // P - player start point
    // O - coin
    // ^ - spike
    // 3 - 3 health breakable
    // 2 - 2 health breakable
    // 1 - 1 health breakable
    // V - gem
    // @ - star and star bubble
    // B - bumble
    // H - hat bumble
    // F - flying bumble
    // A - army bumble
    // S - slime
    // D - drillbug
    // W - boss
    // = - falling platform center
    // This approach means 2 things can't start overlapping but that was only used for hiding coins in breakables which is lame anyway
    char** data;
    // Size of the tilemap in cells.
    Vec2Int levelSize;
    // Name of the level
    char* name;
} LevelData;

// List of all levels. 1-indexed, with 0 as a testing/playground level
extern LevelData levels[20];

extern int currentLevelID;

// Load the level with this ID, populating the ECS system and the tilemaps.
void LoadLevel(int levelID);

// Clear the currently loaded level
void UnloadLevel();