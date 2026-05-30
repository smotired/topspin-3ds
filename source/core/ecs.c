// Source file for the Entity Component System.

#include "../include/ecs.h" // TODO: create an actual include path
#include <stdlib.h>
#include <string.h>

// Global variable for ECS error codes.
// 0 - no error
// 1 - critical. cannot create an entity (overflow)
// 2 - reserved
// 3 - requesting an entity that doesn't exist
// 4 - requesting a component that doesn't exist
// 5 - critical. cannot allocate memory
// 6 - invalid operations on a tag component
// 7 - adding a duplicate component (but it's fine to remove a nonexistent component)
unsigned char ecsErr = 0;

//---------------- System structs

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
    // If a signature is -1 (or 0xFFFFFFFF), that means the entity doesn't exist.
    Signature entitySignatures[MAX_ENTITIES];
} EntityManager;

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
    ComponentList componentLists[COMPONENT_TYPE_COUNT + 1]; // 0 is unused
} ComponentManager;

// References which entities are tracked by which systems.
// Should only be used by the ECS object.
typedef struct {
    // A list of base core objects.
    BaseSystem systems[SYSTEM_TYPE_COUNT + 1]; // 0 is unused
} SystemManager;

// The extra unused ComponentList and BaseSystem might use
// a lot of wasted memory so we can remove them if needed

// Global singleton EntityManager
EntityManager em;

// Global singleton ComponentManager
ComponentManager cm;

// Global singleton SystemManager
SystemManager sm;

//---------------- EntityManager methods

// Initialize the entity manager. Resets to no entities.
void EntityManager_Init() {
    // Initialize the ID stack by counting up from ID 0
    // and the signatures list by setting every entity signature to -1
    for (int i = 0; i < MAX_ENTITIES; i++) {
        em.availableIds[i] = i;
        em.entitySignatures[i] = NULL_SIGNATURE; 
    }

    // Set entity count to 1 so that we never give out id 0
    em.entityCount = 1;
    // This means entityCount is always off by 1, but that's fine since
    // that value is only ever used as a pointer.
}

// Create an entity from an unused ID, and return that ID.
Entity EntityManager_CreateEntity() {
    // If the pointer has reached the end of the list, critical failure
    if (em.entityCount >= MAX_ENTITIES) {
        ecsErr = 1; // cannot create an entity
        return 0;
    }

    // Pop entity ID from the stack and set signature to 0
    Entity id = em.availableIds[em.entityCount++];
    em.entitySignatures[id] = 0;
    return id;
}

// Immediately destroy the entity with this ID.
void EntityManager_DestroyEntity(Entity entity) {
    // If the signature is null, the entity is already destroyed, so do nothing
    if (em.entitySignatures[entity] == NULL_SIGNATURE)
        return;
    // Push the entity back onto the stack and reset the signature
    em.availableIds[--(em.entityCount)] = entity;
    em.entitySignatures[entity] = NULL_SIGNATURE;
}

// Return positive if an entity exists, 0 otherwise.
int EntityManager_EntityExists(Entity entity) {
    // Null signature is 0xFFFFFFFF, so this overflows to 0 if the
    // entity does not exist. Otherwise it will be a positive number.
    return em.entitySignatures[entity] + 1;
}

// Update an entity's signature to include a component.
Signature EntityManager_ApplyComponent(Entity entity, ComponentID component) {
    if (em.entitySignatures[entity] == NULL_SIGNATURE) {
        ecsErr = 3;
        return NULL_SIGNATURE;
    }
    // Apply the component ID to the signature
    em.entitySignatures[entity] |= SG(component);
    return em.entitySignatures[entity];
}

// Update an entity's signature to exclude a component.
Signature EntityManager_DiscardComponent(Entity entity, ComponentID component) {
    if (em.entitySignatures[entity] == NULL_SIGNATURE) {
        ecsErr = 3;
        return NULL_SIGNATURE;
    }
    // Remove the component ID from the signature
    em.entitySignatures[entity] &= ~SG(component);
    return em.entitySignatures[entity];
}

// Return a positive number if an entity has a component, or 0 otherwise.
int EntityManager_HasComponent(Entity entity, ComponentID component) {
    if (em.entitySignatures[entity] == NULL_SIGNATURE) {
        ecsErr = 3;
        return 0;
    }
    // Positive if the component is in the signature
    return em.entitySignatures[entity] & SG(component);
}

//---------------- ComponentManager methods

// These components don't get component lists.
#define TAGS (SG(C_PLATFORM) | SG(C_ENEMY))
// Return a positive number if a component type is a tag
#define IS_TAG(c) (SG(c) & TAGS)

// Initialize a component list for a specific component type
void InitComponentList(ComponentID component, unsigned char componentSize) {
    // Allocate component list
    void* ptr = malloc(componentSize * MAX_ENTITIES);
    if (ptr == NULL) {
        ecsErr = 5;
        return;
    }
    // Set up the list itself
    ComponentList* list = &cm.componentLists[component];
    list->list = ptr;
    list->componentSize = componentSize;
    list->count = 1; // Reserve index 0 as with entity list
    for (int i = 0; i < MAX_ENTITIES; i++) {
        list->indexToEntity[i] = 0;
        list->entityToIndex[i] = 0;
    }
}

// Initialize the Component Manager with component lists for all non-tag component types
void ComponentManager_Init() {
    // Call the helper for all component list types
    InitComponentList(C_TRANSFORM, sizeof(Transform));
    InitComponentList(C_SPRITE, sizeof(Sprite));
    InitComponentList(C_ANIMATOR, sizeof(Animator));
    InitComponentList(C_BOXCOLLIDER, sizeof(BoxCollider));
    InitComponentList(C_CIRCLECOLLIDER, sizeof(CircleCollider));
    InitComponentList(C_RIGIDBODY, sizeof(Rigidbody));
    InitComponentList(C_HEALTH, sizeof(Health));
    InitComponentList(C_COLLECTABLE, sizeof(Collectable));
    InitComponentList(C_PLAYER, sizeof(Player));
    InitComponentList(C_BUMBLE, sizeof(Bumble));
    InitComponentList(C_DRILLBUG, sizeof(Drillbug));
    InitComponentList(C_BOSS, sizeof(Boss));
    InitComponentList(C_RAFT, sizeof(Raft));
    InitComponentList(C_AUDIOSOURCE, sizeof(AudioSource));
    InitComponentList(C_DESTROYOFFSCREEN, sizeof(DestroyOffscreen));
    // Tag types do not get a list
}

// Macro to convert a component list index to a pointer
#define CLP(l, i) (l->list + i * l->componentSize)

// Try to create a zeroed-out component for an entity
void* ComponentManager_AddComponent(Entity entity, ComponentID component) {
    // Return success and a null pointer if this is a tag component
    if (IS_TAG(component))
        return NULL;
    // Check if the component is already in the list
    ComponentList* list = &cm.componentLists[component];
    if (list->entityToIndex[entity]) {
        ecsErr = 7;
        return CLP(list, list->entityToIndex[entity]);
    }
    // Create a new component in the list with zeroed values
    int i = list->count++; // index of this new component
    list->entityToIndex[entity] = i;
    list->indexToEntity[i] = entity;
    void* ptr = CLP(list, i);
    memset(ptr, 0, list->componentSize);
    return ptr;
}

// Try to remove a component from an entity
void ComponentManager_RemoveComponent(Entity entity, ComponentID component) {
    // If this is a tag component we don't need to do anything here
    if (IS_TAG(component)) return;
    ComponentList* list = &cm.componentLists[component];
    int i = list->entityToIndex[entity];
    // If this component isn't in the list, we don't need to do anything
    if (i == 0) return;
    // Replace this index with whatever's at the back to keep it tightly packed
    int back = --(list->count);
    if (i != back) {
        memcpy(CLP(list, i), CLP(list, back), list->componentSize);
        list->indexToEntity[i] = list->indexToEntity[back];
        list->entityToIndex[list->indexToEntity[i]] = i;
    }
    list->indexToEntity[back] = 0;
    list->entityToIndex[entity] = 0;
}

// Try to get a pointer to an entity component
void* ComponentManager_GetComponent(Entity entity, ComponentID component) {
    // Make sure this is not a tag component
    if (IS_TAG(component)) {
        ecsErr = 6;
        return NULL;
    }
    // Check if the component is not already in the list
    ComponentList* list = &cm.componentLists[component];
    int i = list->entityToIndex[entity];
    if (i == 0) {
        ecsErr = 4;
        return NULL;
    }
    // Return a pointer to the component
    return CLP(list, i);
}

// Update all component lists when an entity is destroyed
void ComponentManager_EntityDestroyed(Entity entity) {
    for (int i = 1; i <= COMPONENT_TYPE_COUNT; i++) {
        ComponentManager_RemoveComponent(entity, i);
    }
}

// Frees component lists to avoid a memory leak
void ComponentManager_Cleanup() {
    for (int i = 1; i <= COMPONENT_TYPE_COUNT; i++) {
        if (cm.componentLists[i].list != NULL)
            free(cm.componentLists[i].list);
    }
}

//---------------- SystemManager methods

// Initializes a base core with no entities
BaseSystem* SystemManager_InitSystem(SystemID id, Signature signature) {
    // Initialize the core and return a pointer
    BaseSystem* system = &sm.systems[id];
    system->signature = signature;
    system->entityCount = 1; // 0 is reserved
    for (int i = 0; i < MAX_ENTITIES; i++) {
        system->tracked[i] = 0;
        system->entityToIndex[i] = 0;
    }
    return system;
}

// Signals to a specific core that an entity's components changed and it should start/stop tracking it
void SystemEntityUpdated(BaseSystem* system, Entity entity, Signature entitySignature) {
    // Determine if the entity is tracked and if it should be tracked
    int index = system->entityToIndex[entity];
    int shouldTrack = (entitySignature & system->signature) == system->signature; // has all required components
    // Add if not tracked and mismatch
    if (!index && shouldTrack) {
        system->entityToIndex[entity] = system->entityCount++;
        system->tracked[system->entityToIndex[entity]] = entity;
    }
    // Remove if tracked and mismatch
    else if (index && !shouldTrack) {
        // Move the last tracked element into this spot to keep it tight
        int back = --(system->entityCount);
        if (index != back) {
            system->tracked[index] = system->tracked[back];
            system->entityToIndex[system->tracked[index]] = index;
        }
        system->tracked[back] = 0;
        system->entityToIndex[entity] = 0;
    }
}

// Signal to all systems that an entity had components added or removed
void SystemManager_EntityUpdated(Entity entity, Signature entitySignature) {
    for (int i = 0; i < SYSTEM_TYPE_COUNT; i++) {
        SystemEntityUpdated(&sm.systems[i], entity, entitySignature);
    }
}

//---------------- EntityComponentSystem integration methods

void ECSInit() {
    ecsErr = 0;
    ECSCleanup();
    EntityManager_Init();
    ComponentManager_Init();
}

Entity CreateEntity() {
    ecsErr = 0;
    // Creating an entity doesn't interface with any other systems
    // or require special error handling
    return EntityManager_CreateEntity();
}

void DestroyEntity(Entity entity) {
    ecsErr = 0;
    // Try to destroy the entity, which is idempotent
    EntityManager_DestroyEntity(entity);
    // Remove from all component lists, which is idempotent
    ComponentManager_EntityDestroyed(entity);
    // Update systems with a 0 signature to remove from tracking, idempotent
    SystemManager_EntityUpdated(entity, 0);
}

int EntityExists(Entity entity) {
    ecsErr = 0;
    // Safe and idempotent
    return EntityManager_EntityExists(entity);
}

void* AddComponent(Entity entity, ComponentID component) {
    ecsErr = 0;
    // Add the component to the entity, and return early on failure
    Signature newSignature = EntityManager_ApplyComponent(entity, component);
    if (newSignature == NULL_SIGNATURE) return NULL;
    // Try to create the component
    void* added = ComponentManager_AddComponent(entity, component);
    if (ecsErr != 0) return added;
    // Update systems
    SystemManager_EntityUpdated(entity, newSignature);
    return added;
}

void RemoveComponent(Entity entity, ComponentID component) {
    ecsErr = 0;
    // Remove the component from the entity, and return early on failure
    Signature newSignature = EntityManager_DiscardComponent(entity, component);
    if (newSignature == NULL_SIGNATURE) return;
    // Try to remove the component
    ComponentManager_RemoveComponent(entity, component);
    if (ecsErr != 0) return;
    // Update systems
    SystemManager_EntityUpdated(entity, newSignature);
}

void* GetComponent(Entity entity, ComponentID component) {
    ecsErr = 0;
    // Try to get the component
    return ComponentManager_GetComponent(entity, component);
}

int HasComponent(Entity entity, ComponentID component) {
    ecsErr = 0;
    // Safe and idempotent
    return EntityManager_HasComponent(entity, component);
}

BaseSystem* InitSystem(SystemID id, Signature signature) {
    ecsErr = 0;
    // Initialize the core
    BaseSystem* system = SystemManager_InitSystem(id, signature);
    // Go through and track all existing entities if needed
    for (int i = 1; i < MAX_ENTITIES; i++)
        if (em.entitySignatures[i] != NULL_SIGNATURE)
            SystemEntityUpdated(system, i, em.entitySignatures[i]);
    return system;
}

void ECSCleanup() {
    ecsErr = 0;
    ComponentManager_Cleanup();
}