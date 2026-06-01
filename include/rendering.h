/// Header for the rendering functionality
#pragma once

#include "systems.h"

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