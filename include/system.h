/// Declarations for each type of system.
#pragma once

#include "vec2.h"
#include "component.h"

typedef struct BaseSystem;

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
} CollisionEvent;

extern CollisionEvent collisionEvents[MAX_EVENTS];
extern unsigned char collisionEventCount;

// Represents a collision between an object and a tilemap
typedef struct {
    Entity object;
     // if it's colliding with the goop tilemap instead of the ground tilemap
    int goop;
} TilemapCollisionEvent;

extern TilemapCollisionEvent tilemapCollisionEvents[MAX_EVENTS];
extern unsigned char tilemapCollisionEventCount;

// Tick update for PhysicsSystem
// Move objects with rigidbodies
// Loop through the world and other rigidbodies to find collisions
// To avoid looping through almost every entity, we can only collide with rigidbodies and tilemaps, so static objects should have an immovable flag.
// Adds collisions to the CollisionEvent queue or TilemapCollisionEventQueue.
void PhysicsSystemUpdate(float dt);

#define S_PHYSICS 1
#define SIGNATURE_PHYSICS (SG(C_TRANSFORM) | SG(C_RIGIDBODY))
// does not need a collider (particles should be able to be affected by gravity)

// ---------------- AnimationSystem

// Tick update for AnimationSystem
void AnimationSystemUpdate(float dt);

#define S_ANIMATION 2
#define SIGNATURE_ANIMATION (SG(C_SPRITE) | SG(C_ANIMATOR))

// ---------------- EntityDamageSystem

// Represents an entity being killed
typedef struct {
    Entity killed;
} EntityKilledEvent;

extern EntityKilledEvent entityKilledEvents[MAX_EVENTS];
extern unsigned char entityKilledEventCount;

// Tick update for EntityDamageSystem
void EntityDamageSystemUpdate(float dt);

#define S_ENTITYDAMAGE 3
#define SIGNATURE_ENITTYDAMAGE (SG(C_HEALTH))

// ---------------- Extra stuff

// How many unique types of systems exist
#define SYSTEM_TYPE_COUNT 1

// System ID matching the macros in this file
typedef unsigned char SystemID;