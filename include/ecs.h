/// Declares the manager structs for Entities, Components, and Systems.
/// The manager structs and methods should only be used by the ECS object.
#pragma once

#include "component.h"
#include "system.h"

// The maximum count of entities that can exist at one time
// If static ground and goop tiles are stored as tilemaps unrelated to the ECS,
// this should be more than enough.
// This number is used to initialize many arrays so it should be kept relatively small.
#define MAX_ENTITIES 4096

// The EntityManager is in charge of adding and removing entities,
// and managing which components an entity has.
// Should only be used by the ECS object.
typedef struct {
    // Stack of which entity IDs are unused, to keep IDs tightly packed.
    // Entity IDs act as an index into signature or component arrays.
    Entity availableIds[MAX_ENTITIES];
    // Functions as a pointer to the top of the availableIDs stack.
    // Using the latest freed ID for the next ID should ensure that the
    // max ID is always less than or equal to the total amount of created entities
    Entity entityCount;
    // List of signatures corresponding to entity IDs. If a component's bit is set,
    // the entity has that component.
    // If a signature is 0, that means the entity doesn't exist.
    Signature entitySignatures[MAX_ENTITIES];
} EntityManager;

// Initialize an EntityManager and the ID and Signature lists.
void EntityManager_Init(EntityManager* manager);

// Create and return an entity.
Entity EntityManager_CreateEntity(EntityManager* manager);

// Destroy an entity from the list.
void EntityManager_DestroyEntity(EntityManager* manager, Entity entity);

// Returns true if the entity exists and has components
bool EntityManager_EntityExists(EntityManager* manager, Entity entity);

// Update an EntityManager's signature to include a component and return the updated signature.
Signature EntityManager_ApplyComponent(EntityManager* manager, Entity entity, ComponentID component);
// hopefully the signature makes it clear that it doesn't actually create a component

// Update an EntityManager's signature to not include a component and return the updated signature.
Signature EntityManager_DiscardComponent(EntityManager* manager, Entity entity, ComponentID component);
// hopefully the signature makes it clear that it doesn't actually create a component

// Returns true if the entity has a component.
bool EntityManager_HasComponent(EntityManager* manager, Entity entity, ComponentID component);

// An individual list of components
typedef struct {
    // Size in bytes of individual components in this list
    unsigned char componentSize;
    // Pointer to list of component structs on heap
    void* list;
    // Amount of entities that have this component
    Entity count;
    // Mapping of component list indices to entity IDs, used for tight packing
    Entity indexToEntity[MAX_ENTITIES];
    // Mapping of entity IDs to component list indices, used for tight packing
    Entity entityToIndex[MAX_ENTITIES];
} ComponentList;

// The component manager tracks references to entities' components.
// Should only be used by the ECS object.
typedef struct {
    ComponentList* componentLists[COMPONENT_TYPE_COUNT];
} ComponentManager;

// Initialize a ComponentManager and create lists for each component type.
void ComponentManager_Init(ComponentManager* manager);

// Add a component to an entity and return a pointer to the component.
void* ComponentManager_AddComponent(ComponentManager* manager, Entity entity, ComponentID component);

// Remove a component from an entity
void ComponentManager_RemoveComponent(ComponentManager* manager, Entity entity, ComponentID component);

// Return a pointer to an entity's instance of a component.
void* ComponentManager_GetComponent(ComponentManager* manager, Entity entity, ComponentID component);

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

// References which entities are tracked by which systems.
// Should only be used by the ECS object.
typedef struct {
    // A list of base system objects.
    BaseSystem systems[SYSTEM_TYPE_COUNT];
} SystemManager;

// Initialize the given system with the given signature, and populate with the entities tracked.
// Return a reference to the system.
BaseSystem* SystemManager_InitSystem(SystemManager* manager, SystemID id, Signature signature);

// When an entity's signature is changed, revalidate all systems to decide if it should be tracked.
// When an entity is destroyed, this should be called with a signature of zero to remove from all systems.
void SystemManager_EntityUpdated(SystemManager* manager, Entity entity, Signature entitySignature);
// This does not work if a system tracks entities with no components but that shouldn't be allowed anyway

// Facilitates communcation between the three systems.
typedef struct {
    // The EntityManager tracks which entities exist and what components they have.
    EntityManager entityManager;
    // The ComponentManager stores the actual component data for entities.
    ComponentManager componentManager;
    // The SystemManager stores what components each system uses and what entities have all those components.
    SystemManager systemManager;
} ECS;

// Global methods for the ECS, assuming a singleton global ECS exists somewhere.

// Initalize the full ECS
void ECSInit();

// Create an entity in the ECS
Entity CreateEntity();

// Destroy an entity in the ECS
void DestroyEntity(Entity entity);

// Return true if an entity exists and has components
bool EntityExists(Entity entity);

// Add a component to the specified entity and return a pointer to the component.
void* AddComponent(Entity entity, ComponentID component);

// Remove a component from the specified entity.
void RemoveComponent(Entity entity, ComponentID component);

// Get a pointer to the specified component on the entity.
void* GetComponent(Entity entity, ComponentID component);

// Return true if the entity has the specified component.
bool HasComponent(Entity entity, ComponentID component);

// Set up and get a reference to a system.
BaseSystem* InitSystem(SystemID system, Signature signature);

// Macros for getting specific components from entities to reduce repetition
#define GetTransform(e) ((Transform*)GetComponent(e, C_TRANSFORM))
#define GetSprite(e) ((Sprite*)GetComponent(e, C_SPRITE))
#define GetAnimator(e) ((Animator*)GetComponent(e, C_ANIMATOR))
#define GetBoxCollider(e) ((BoxCollider*)GetComponent(e, C_BOXCOLLIDER))
#define GetCircleCollider(e) ((CircleCollider*)GetComponent(e, C_CIRCLECOLLIDER))
#define GetRigidbody(e) ((Rigidbody*)GetComponent(e, C_RIGIDBODY))
#define GetHealth(e) ((Health*)GetComponent(e, C_HEALTH))
#define GetCollectable(e) ((Collectable*)GetComponent(e, C_COLLECTABLE))
#define GetPlayer(e) ((Player*)GetComponent(e, C_PLAYER))
#define GetBumble(e) ((Bumble*)GetComponent(e, C_BUMBLE))
#define GetDrillbug(e) ((Drillbug*)GetComponent(e, C_DRILLBUG))
#define GetBoss(e) ((Boss*)GetComponent(e, C_BOSS))
#define GetRaft(e) ((Raft*)GetComponent(e, C_RAFT))
#define GetAudioSource(e) ((AudioSource*)GetComponent(e, C_AUDIOSOURCE))
#define GetDestroyOffscreen(e) ((DestroyOffscreen*)GetComponent(e, C_DESTROYOFFSCREEN))