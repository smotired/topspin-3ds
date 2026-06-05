/// Declarations for each type of system.
#pragma once

#include "smath.h"
#include "component.h"
#include "ecs.h"

// Maximum events that can occur per frame.
#define MAX_EVENTS 32
// The expectation is that each system processes all events on each frame,
// and then clears its own queue when processing.
// Meaning based on system update order, some systems might see events
// from the previous frame, but will definitely see each event exactly once.

// ---------------- Physics System

// Represents a collision between two objects
typedef struct {
    Entity object1;
    Entity object2;
    // Center of the collision point
    Vec2 position;
    // Collision normal
    Vec2 normal;
    // Penetration (how far along the normal the collision point is into each object)
    float penetration;
} CollisionEvent;

extern CollisionEvent collisionEvents[MAX_EVENTS];
extern unsigned char collisionEventCount;

// Represents a collision between an object and a tilemap
typedef struct {
    Entity object;
     // if it's colliding with the goop tilemap instead of the ground tilemap
    bool goop;
} TilemapCollisionEvent;

extern TilemapCollisionEvent tilemapCollisionEvents[MAX_EVENTS];
extern unsigned char tilemapCollisionEventCount;

extern BaseSystem* physicsSystem;

// Tick update for PhysicsSystem
// Move objects with rigidbodies
// Loop through the world and other rigidbodies to find collisions
// To avoid looping through almost every entity, we can only collide with rigidbodies and tilemaps, so static objects should have an immovable flag.
// Adds collisions to the CollisionEvent queue or TilemapCollisionEventQueue.
void PhysicsSystemUpdate(float dt);

#define S_PHYSICS 1
#define SIGNATURE_PHYSICS (SG(C_TRANSFORM) | SG(C_RIGIDBODY))
// does not need a collider (particles should be able to be affected by gravity)

// ---------------- RenderSystem
// Draws sprites to the screen

extern BaseSystem* renderSystem;

// Tick update for RenderSystem
void RenderSystemUpdate(float dt);

#define S_RENDER 7
#define SIGNATURE_RENDER (SG(C_TRANSFORM) | SG(C_SPRITE))

// ---------------- AnimationSystem

typedef unsigned int AnimationID;

// Represents an animation finishing and possibly looping
typedef struct {
    // The entity whose animation finished
    Entity entity;
    // The ID of the animation that finished
    AnimationID animation;
} AnimationFinishEvent;

extern AnimationFinishEvent animationFinishEvents[MAX_EVENTS];
extern unsigned char animationFinishEventCount;

extern BaseSystem* animationSystem;

// Tick update for AnimationSystem
void AnimationSystemUpdate(float dt);

#define S_ANIMATION 3
#define SIGNATURE_ANIMATION (SG(C_SPRITE) | SG(C_ANIMATOR))

// ---------------- PlayerControllerSystem
// Handles all the player control

extern BaseSystem* playerControllerSystem;

// Tick update for PlayerControllerSystem
void PlayerControllerSystemUpdate(float dt);

#define S_PLAYERCONTROLLERSYSTEM 4
#define SIGNATURE_PLAYERCONTROLLER (SG(C_PLAYER))

// ---------------- EntityDamageSystem

// Represents an entity being killed
typedef struct {
    Entity killed;
} EntityKilledEvent;

extern EntityKilledEvent entityKilledEvents[MAX_EVENTS];
extern unsigned char entityKilledEventCount;

extern BaseSystem* entityDamageSystem;

// Tick update for EntityDamageSystem
void EntityDamageSystemUpdate(float dt);

#define S_ENTITYDAMAGE 5
#define SIGNATURE_ENTITYDAMAGE (SG(C_HEALTH))

// ---------------- BreakableSystem
// Breakables have a transform/rigidbody, health, sprite, and either type of collider.
// They also don't have the Enemy tag.
// Must decrement health with the sprite.

extern BaseSystem* breakableSystem;

// Tick update for BreakableSystem
void BreakableSystemUpdate(float dt);

#define S_BREAKABLESYSTEM 6
#define SIGNATURE_BREAKABLE (SG(C_TRANSFORM) | SG(C_RIGIDBODY) | SG(C_HEALTH) | SG(C_SPRITE))

// ---------------- EnemyControllerSystem
// All enemies other than the boss are relatively simple so this just handles all of em

extern BaseSystem* enemyControllerSystem;

// Tick update for EnemyControllerSystem
void EnemyControllerSystemUpdate(float dt);

#define S_ENEMYCONTROLLERSYSTEM 7
#define SIGNATURE_ENEMYCONTROLLER (SG(C_ENEMY))

// ---------------- BossControllerSystem
// Controls the boss

extern BaseSystem* bossControllerSystem;

// Tick update for BossControllerSystem
void BossControllerSystemUpdate(float dt);

#define S_BOSSCONTROLLERSYSTEM 8
#define SIGNATURE_BOSSCONTROLLER (SG(C_BOSS))

// ---------------- RaftControllerSystem
// Controls the raft

extern BaseSystem* raftControllerSystem;

// Tick update for RaftControllerSystem
void RaftControllerSystemUpdate(float dt);

#define S_RAFTCONTROLLERSYSTEM 9
#define SIGNATURE_RAFTCONTROLLER (SG(C_RAFT))

// ---------------- FallingPlatformSystem
// Controls falling platforms to enable gravity on player collision

extern BaseSystem* fallingPlatformSystem;

// Tick update for FallingPlatformSystem
void FallingPlatformSystemUpdate(float dt);

#define S_FALLINGPLATFORM 10
#define SIGNATURE_FALLINGPLATFORM (SG(C_PLATFORM))

// ---------------- CollectibleSystem
// Triggers collectibles on collision

extern BaseSystem* collectibleSystem;

// Tick update for CollectibleSystem
void CollectibleSystemUpdate(float dt);

#define S_COLLECTIBLE 11
#define SIGNATURE_COLLECTIBLE (SG(C_COLLECTIBLE))

// ---------------- DestroyOffscreenSystem
// Destroys objects when they fall offscreen

extern BaseSystem* destroyOffscreenSystem;

// Tick update for DestroyOffscreenSystem
void DestroyOffscreenSystemUpdate(float dt);

#define S_DESTROYOFFSCREEN 12
#define SIGNATURE_DESTROYOFFSCREEN (SG(C_DESTROYOFFSCREEN))

// ---------------- AudioSystem
// Handles playing and looping audio sources

extern BaseSystem* audioSystem;

// Tick update for AudioSystem
void AudioSystemUpdate(float dt);

#define S_AUDIO 13
#define SIGNATURE_AUDIO (SG(C_AUDIOSOURCE))