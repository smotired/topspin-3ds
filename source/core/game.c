/// Handles systems and interactions

#include <citro2d.h>
#include "game.h"
#include "ecs.h"
#include "systems.h"

Entity thePlayer;

void GameInit() {
    ECSInit();

    // Initialize systems
    renderSystem = InitSystem(S_RENDER, SIGNATURE_RENDER);
    printf("\x1b[10;1HrenderSystem: %p", renderSystem);
    physicsSystem = InitSystem(S_PHYSICS, SIGNATURE_PHYSICS);
    printf("\x1b[11;1HphysicsSystem: %p", physicsSystem);

    // Create player entity
    thePlayer = CreateEntity();

    Transform* playerTransform = (Transform*)AddComponent(thePlayer, C_TRANSFORM);
    playerTransform->pos = (Vec2) { 12.5, 7.5 };

    BoxCollider* playerCol = (BoxCollider*)AddComponent(thePlayer, C_BOXCOLLIDER);
    playerCol->size = (Vec2) { 1, 2 };

    AddComponent(thePlayer, C_SPRITE); // make it get drawn by the render system

    AddComponent(thePlayer, C_RIGIDBODY);

    // Create other default entities

    Entity block = CreateEntity();

    Transform* b_t = (Transform*)AddComponent(block, C_TRANSFORM);
    b_t->pos = (Vec2) { 12.5, 12.5 };

    BoxCollider* b_col = (BoxCollider*)AddComponent(block, C_BOXCOLLIDER);
    b_col->size = (Vec2) { 9, 1 };

    Rigidbody* b_rb = (Rigidbody*)AddComponent(block, C_RIGIDBODY);
    b_rb -> flags |= RBF_IMMOBILE; // immobile

    AddComponent(block, C_SPRITE);

    Entity circle = CreateEntity();

    Transform* c_t = (Transform*)AddComponent(circle, C_TRANSFORM);
    c_t->pos = (Vec2) { 11.9, 2.5 };

    CircleCollider* c_col = (CircleCollider*)AddComponent(circle, C_CIRCLECOLLIDER);
    c_col->radius = 0.5f;

    AddComponent(circle, C_RIGIDBODY);

    AddComponent(circle, C_SPRITE);
}

void GameTick(float dt) {
    RenderSystemUpdate(dt);
    PhysicsSystemUpdate(dt);

    u32 kHeld = hidKeysHeld();
    if ((kHeld & KEY_UP)) {
        GetRigidbody(thePlayer)->velocity.y = -5;
    }
}

void GameDeinit() {
    ECSCleanup();
}