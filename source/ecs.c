// Source file for the Entity Component System.

#include "../include/ecs.h"; // TODO: create an actual include path
#include <stdlib.h>
#include <string.h>

// Global variable for ECS error codes.
// 0 - no error
// 1 - critical. cannot create an entity (overflow)
// 2 - access overflow
// 3 - requesting an entity that doesn't exist
// 4 - requesting a component that doesn't exist
// 5 - critical. cannot allocate memory
// 6 - invalid operations on a tag component
// 7 - adding a duplicate component (but it's fine to remove a nonexistent component)
unsigned char ecsErr = 0;

//---------------- EntityManager methods

void EntityManager_Init(EntityManager* manager) {
    // Initialize the ID stack by counting up from ID 0
    // and the signatures list by setting every entity signature to -1
    for (int i = 0; i < MAX_ENTITIES; i++) {
        manager->availableIds[i] = i;
        manager->entitySignatures[i] = NULL_SIGNATURE; 
    }

    // Set entity count to 1 so that we never give out id 0
    manager->entityCount = 1;
    // This means entityCount is always off by 1, but that's fine since
    // that value is only ever used as a pointer.
}

Entity EntityManager_CreateEntity(EntityManager* manager) {
    // If the pointer has reached the end of the list, critical failure
    if (manager->entityCount >= MAX_ENTITIES) {
        ecsErr = 1; // cannot create an entity
        return 0;
    }

    // Pop entity ID from the stack and set signature to 0
    Entity id = manager->availableIds[manager->entityCount++];
    manager->entitySignatures[id] = 0;
    return id;
}

void EntityManager_DestroyEntity(EntityManager* manager, Entity entity) {
    // If the signature is null, the entity is already destroyed, so do nothing
    if (manager->entitySignatures[entity] == NULL_SIGNATURE)
        return;
    // Push the entity back onto the stack and reset the signature
    manager->availableIds[--(manager->entityCount)] = entity;
    manager->entitySignatures[entity] = NULL_SIGNATURE;
}

int EntityManager_EntityExists(EntityManager* manager, Entity entity) {
    // Null signature is 0xFFFFFFFF, so this overflows to 0 if the
    // entity does not exist. Otherwise it will be a positive number.
    return manager->entitySignatures[entity] + 1;
}

Signature EntityManager_ApplyComponent(EntityManager* manager, Entity entity, ComponentID component) {
    if (manager->entitySignatures[entity] == NULL_SIGNATURE) {
        ecsErr = 3;
        return NULL_SIGNATURE;
    }
    // Apply the component ID to the signature
    manager->entitySignatures[entity] |= SG(component);
    return manager->entitySignatures[entity];
}

Signature EntityManager_DiscardComponent(EntityManager* manager, Entity entity, ComponentID component) {
    if (manager->entitySignatures[entity] == NULL_SIGNATURE) {
        ecsErr = 3;
        return NULL_SIGNATURE;
    }
    // Remove the component ID from the signature
    manager->entitySignatures[entity] ^= SG(component);
    return manager->entitySignatures[entity];
}

int EntityManager_HasComponent(EntityManager* manager, Entity entity, ComponentID component) {
    if (manager->entitySignatures[entity] == NULL_SIGNATURE) {
        ecsErr = 3;
        return 0;
    }
    // Positive if the component is in the signature
    return manager->entitySignatures[entity] & SG(component);
}

//---------------- ComponentManager methods

// These components don't get component lists.
#define TAGS (SG(C_PLATFORM) | SG(C_ENEMY))
#define IS_TAG(c) (SG(c) & TAGS)

void InitComponentList(ComponentManager* manager, ComponentID component, unsigned char componentSize) {
    // Allocate component list
    void* ptr = malloc(componentSize * MAX_ENTITIES);
    if (ptr == NULL) {
        ecsErr = 5;
        return;
    }
    // Set up the list itself
    ComponentList* list = &manager->componentLists[component];
    list->list = ptr;
    list->componentSize = componentSize;
    list->count = 1; // Reserve index 0 as with entity list
    for (int i = 0; i < MAX_ENTITIES; i++) {
        list->indexToEntity[i] = 0;
        list->entityToIndex[i] = 0;
    }
}

void ComponentManager_Init(ComponentManager* manager) {
    // Call the helper for all component list types
    InitComponentList(manager, C_TRANSFORM, sizeof(Transform));
    InitComponentList(manager, C_SPRITE, sizeof(Sprite));
    InitComponentList(manager, C_ANIMATOR, sizeof(Animator));
    InitComponentList(manager, C_BOXCOLLIDER, sizeof(BoxCollider));
    InitComponentList(manager, C_CIRCLECOLLIDER, sizeof(CircleCollider));
    InitComponentList(manager, C_RIGIDBODY, sizeof(Rigidbody));
    InitComponentList(manager, C_HEALTH, sizeof(Health));
    InitComponentList(manager, C_COLLECTABLE, sizeof(Collectable));
    InitComponentList(manager, C_PLAYER, sizeof(Player));
    InitComponentList(manager, C_BUMBLE, sizeof(Bumble));
    InitComponentList(manager, C_DRILLBUG, sizeof(Drillbug));
    InitComponentList(manager, C_BOSS, sizeof(Boss));
    InitComponentList(manager, C_RAFT, sizeof(Raft));
    InitComponentList(manager, C_AUDIOSOURCE, sizeof(AudioSource));
    InitComponentList(manager, C_DESTROYOFFSCREEN, sizeof(DestroyOffscreen));
    // Tag types do not get a list
}

// Macro to convert a component list index to a pointer
#define CLP(l, i) (l->list + i * l->componentSize)

void* ComponentManager_AddComponent(ComponentManager* manager, Entity entity, ComponentID component) {
    // Make sure this is not a tag component
    if (IS_TAG(component)) {
        ecsErr = 6;
        return NULL;
    }
    // Check if the component is already in the list
    ComponentList* list = &manager->componentLists[component];
    if (list->entityToIndex[entity]) {
        ecsErr = 7;
        return NULL;
    }
    // Create a new component in the list with zeroed values
    int i = list->count++; // index of this new component
    list->entityToIndex[entity] = i;
    list->indexToEntity[i] = entity;
    void* ptr = CLP(list, i);
    memset(ptr, 0, list->componentSize);
    return ptr;
}

void ComponentManager_RemoveComponent(ComponentManager* manager, Entity entity, ComponentID component) {
    // If this is a tag component we don't need to do anything here
    if (IS_TAG(component)) return;
    ComponentList* list = &manager->componentLists[component];
    int i = list->entityToIndex[entity];
    // If this component isn't in the list, we don't need to do anything
    if (i == 0) return;
    // Replace this index with whatever's at the back to keep it tightly packed
    int back = --(list->count);
    memcpy(CLP(list, i), CLP(list, back), list->componentSize);
    list->indexToEntity[i] = list->indexToEntity[back];
    list->indexToEntity[back] = 0;
    list->entityToIndex[list->indexToEntity[i]] = i;
    list->entityToIndex[entity] = 0;
}

void* ComponentManager_GetComponent(ComponentManager* manager, Entity entity, ComponentID component) {
    // Make sure this is not a tag component
    if (IS_TAG(component)) {
        ecsErr = 6;
        return NULL;
    }
    // Check if the component is not already in the list
    ComponentList* list = &manager->componentLists[component];
    int i = list->entityToIndex[entity];
    if (i == 0) {
        ecsErr = 7;
        return NULL;
    }
    // Return a pointer to the component
    return CLP(list, i);
}

void ComponentManager_EntityDestroyed(ComponentManager* manager, Entity entity) {
    for (int i = 1; i <= COMPONENT_TYPE_COUNT; i++) {
        ComponentManager_RemoveComponent(manager, entity, i);
    }
}

void FreeComponentLists(ComponentManager* manager) {
    for (int i = 1; i <= COMPONENT_TYPE_COUNT; i++) {
        if (IS_TAG(i)) { continue; }
        free(manager->componentLists[i].list);
    }
}

//---------------- SystemManager methods

BaseSystem* SystemManager_InitSystem(SystemManager* manager, SystemID id, Signature signature) {
    // Initialize the system and return a pointer
    BaseSystem* system = &manager->systems[id];
    system->signature = signature;
    system->entityCount = 1; // 0 is reserved
    for (int i = 0; i < MAX_ENTITIES; i++) {
        system->tracked[i] = 0;
        system->entityToIndex[i] = 0;
    }
    return system;
}

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
        system->tracked[index] = system->tracked[back];
        system->entityToIndex[system->tracked[index]] = index;
        system->tracked[back] = 0;
        system->entityToIndex[entity] = 0;
    }
}

void SystemManager_EntityUpdated(SystemManager* manager, Entity entity, Signature entitySignature) {
    for (int i = 0; i < SYSTEM_TYPE_COUNT; i++) {
        SystemEntityUpdated(&manager->systems[i], entity, entitySignature);
    }
}

//---------------- EntityComponentSystem integration methods

// Forward declaration of the global ECS
extern ECS ecs;

void ECSInit() {
    ecsErr = 0;
    EntityManager_Init(&ecs.entityManager);
    ComponentManager_Init(&ecs.componentManager);
}

Entity CreateEntity() {
    ecsErr = 0;
    // Creating an entity doesn't interface with any other systems
    // or require special error handling
    return EntityManager_CreateEntity(&ecs.entityManager);
}

void DestroyEntity(Entity entity) {
    ecsErr = 0;
    // Try to destroy the entity, which is idempotent
    EntityManager_DestroyEntity(&ecs.entityManager, entity);
    // Remove from all component lists, which is idempotent
    ComponentManager_EntityDestroyed(&ecs.componentManager, entity);
    // Update systems with a 0 signature to remove from tracking, idempotent
    SystemManager_EntityUpdated(&ecs.systemManager, entity, 0);
}

int EntityExists(Entity entity) {
    ecsErr = 0;
    // Safe and idempotent
    return EntityManager_EntityExists(&ecs.entityManager, entity);
}

void* AddComponent(Entity entity, ComponentID component) {
    ecsErr = 0;
    // Add the component to the entity, and return early on failure
    Signature newSignature = EntityManager_ApplyComponent(&ecs.entityManager, entity, component);
    if (newSignature == NULL_SIGNATURE) return NULL;
    // Try to create the component
    void* component = ComponentManager_AddComponent(&ecs.componentManager, entity, component);
    if (ecsErr != 0) return component;
    // Update systems
    SystemManager_EntityUpdated(&ecs.systemManager, entity, newSignature);
    return component;
}

void RemoveComponent(Entity entity, ComponentID component) {
    ecsErr = 0;
    // Remove the component from the entity, and return early on failure
    Signature newSignature = EntityManager_DiscardComponent(&ecs.entityManager, entity, component);
    if (newSignature == NULL_SIGNATURE) return;
    // Try to remove the component
    ComponentManager_RemoveComponent(&ecs.componentManager, entity, component);
    if (ecsErr != 0) return;
    // Update systems
    SystemManager_EntityUpdated(&ecs.systemManager, entity, newSignature);
    return component;
}

void* GetComponent(Entity entity, ComponentID component) {
    ecsErr = 0;
    // Try to get the component
    return ComponentManager_GetComponent(&ecs.componentManager, entity, component);
}

int HasComponent(Entity entity, ComponentID component) {
    ecsErr = 0;
    // Safe and idempotent
    return EntityManager_HasComponent(&ecs.entityManager, entity, component);
}

BaseSystem* InitSystem(SystemID system, Signature signature) {
    // Initialize the system
    BaseSystem* system = SystemManager_InitSystem(&ecs.systemManager, system, signature);
    // Go through and track all existing entities if needed
    for (int i = 1; i < MAX_ENTITIES; i++)
        if (&ecs.entityManager.entitySignatures[i] != NULL_SIGNATURE)
            SystemEntityUpdated(system, i, &ecs.entityManager.entitySignatures[i]);
}

void ECSCleanup() {
    ecsErr = 0;
    FreeComponentLists(&ecs.componentManager);
}