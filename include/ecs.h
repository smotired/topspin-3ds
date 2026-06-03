/// Declares the manager structs for Entities, Components, and Systems.
/// The manager structs and methods should only be used by the ECS object.
#pragma once

#include "component.h"
#include "systems.h"

// The maximum count of entities that can exist at one time
// If static ground and goop tiles are stored as tilemaps unrelated to the ECS,
// this should be more than enough.
// This number is used to initialize many arrays so it should be kept relatively small.
#define MAX_ENTITIES 4096

// Signature of a destroyed entity (which limits unique component count to 31)
#define NULL_SIGNATURE 0xFFFFFFFF

// Common information to all systems. Each system should keep a reference to this object.
typedef struct {
    // Defines what components this system cares about. All entities that
    // match this signature have all required components.
    Signature signature;
    // List of which entity IDs are tracked.
    Entity tracked[MAX_ENTITIES];
    // Mapping of entity IDs to tracked list indices, used for tight packing
    Entity entityToIndex[MAX_ENTITIES];
    // Amount of entities tracked by this system.
    Entity entityCount;
} BaseSystem;

// Methods for each required function of the ECS

// Initalize the full ECS.
// Error 5 - Cannot allocate memory for component lists.
void ECSInit();

// Create an entity in the ECS
// Error 1 - There are more than the maximum number of entities.
Entity CreateEntity();

// Destroy an entity in the ECS
void DestroyEntity(Entity entity);

// Return true if an entity exists and has components
int EntityExists(Entity entity);

// Add a component to the specified entity and return a pointer to the component.
// Error 3 - The entity does not exist
// Error 7 - The entity already has the component (still returns the component)
// Adding a tag component returns null with no error
void* AddComponent(Entity entity, ComponentID component);

// Remove a component from the specified entity.
// Error 3 - The entity does not exist
void RemoveComponent(Entity entity, ComponentID component);

// Get a pointer to the specified component on the entity.
// Error 3 - The entity does not exist
// Error 6 - The provided component is a tag (no data to get)
// Error 4 - The entity does not have the component
void* GetComponent(Entity entity, ComponentID component);

// Return true if the entity has the specified component.
// Error 3 - The entity does not exist
int HasComponent(Entity entity, ComponentID component);

// Set up and get a reference to a system. Populates with entities that already have that signature.
BaseSystem* InitSystem(SystemID id, Signature signature);

// Cleanup to avoid memory leaks
void ECSCleanup();

// Macros for getting specific components from entities to reduce repetition
#define GetTransform(e) ((Transform*)GetComponent(e, C_TRANSFORM))
#define GetSprite(e) ((Sprite*)GetComponent(e, C_SPRITE))
#define GetAnimator(e) ((Animator*)GetComponent(e, C_ANIMATOR))
#define GetBoxCollider(e) ((BoxCollider*)GetComponent(e, C_BOXCOLLIDER))
#define GetCircleCollider(e) ((CircleCollider*)GetComponent(e, C_CIRCLECOLLIDER))
#define GetRigidbody(e) ((Rigidbody*)GetComponent(e, C_RIGIDBODY))
#define GetHealth(e) ((Health*)GetComponent(e, C_HEALTH))
#define GetCollectible(e) ((Collectible*)GetComponent(e, C_COLLECTIBLE))
#define GetPlayer(e) ((Player*)GetComponent(e, C_PLAYER))
#define GetBumble(e) ((Bumble*)GetComponent(e, C_BUMBLE))
#define GetDrillbug(e) ((Drillbug*)GetComponent(e, C_DRILLBUG))
#define GetBoss(e) ((Boss*)GetComponent(e, C_BOSS))
#define GetRaft(e) ((Raft*)GetComponent(e, C_RAFT))
#define GetAudioSource(e) ((AudioSource*)GetComponent(e, C_AUDIOSOURCE))
#define GetDestroyOffscreen(e) ((DestroyOffscreen*)GetComponent(e, C_DESTROYOFFSCREEN))