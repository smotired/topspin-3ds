/// Header for the rendering functionality
#pragma once

#include "systems.h"

// Takes in world coordinates and converts them to screen coordinates
// based on the current camera data.
Vec2Int WorldToScreenPos(Vec2 world);

// Takes in screen coordinates in X and Y with the origin in the
// top left, and converts them to world coordinates based on the current
// camera position.
Vec2 ScreenToWorldPos(Vec2Int screen);

// World space position of the center of the camera
extern Vec2 cameraPos;
// How many complete tiles can fit on screen vertically (for a 240p screen that is 15 16x16 tiles)
#define CAMERA_SIZE 15

/// -------------------------------- For the animation system

typedef struct {
    // ID of this animation
    AnimationID id;
    // Pointer to the frames list
    unsigned char* frames;
    // Amount of frames in the animation
    unsigned char frameCount;
    // Time in seconds for each frame
    float frameDuration;
    // What animation should play when this finishes
    AnimationID nextAnimation;
} Animation;

// Definition of all animations, indexed by AnimationID
extern Animation animationIndex[];