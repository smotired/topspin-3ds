/// Source for the AnimationSystem

#include "../../include/systems.h"
#include "../../include/ecs.h"
#include "../../include/rendering.h"

AnimationFinishEvent animationFinishEvents[MAX_EVENTS];
unsigned char animationFinishEventCount;

BaseSystem* animationSystem;

void AnimationSystemUpdate(float dt) {
    // Reset queue
    animationFinishEventCount = 0;

    // Loop through entities tracked by this system
    for (unsigned int i = 1; i < animationSystem->entityCount; i++) {
        // Get entity and components
        Entity e = animationSystem->tracked[i];
        Animator* animator = GetAnimator(e);
        Sprite* sprite = GetSprite(e);
        Animation* anim = &animationIndex[animator->animation];

        // Update the animation time
        animator->time += dt;

        // Update sprite
        unsigned int frame = (unsigned int)(animator->time / anim->frameDuration) % anim->frameCount;
        sprite->spr = anim->frames[frame];

        // Transition
        if (animator->time >= anim->frameDuration * anim->frameCount) {
            // Add an animation finish event to the queue
            if (animationFinishEventCount < MAX_EVENTS) {
                animationFinishEvents[animationFinishEventCount++] = (AnimationFinishEvent){ e, animator->animation };
            }

            // Transition to next animation, only reset time if the animation is changing
            if (anim->nextAnimation != animator->animation)
                animator->time = 0;
            animator->animation = anim->nextAnimation;
        }
    }
}

// Definition of all animations, indexed by AnimationID
Animation animationIndex[] = {
    // -------------------------------- Player animations
    // Idle
    { 0, (unsigned char[]){ 'A', 'B' }, 2, 0.5f, 0 }
    // -------------------------------- Enemy animations
    // -------------------------------- Collectible animations
    // -------------------------------- Environmental animations
    // -------------------------------- Other animations
};