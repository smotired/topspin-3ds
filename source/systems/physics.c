/// Physics system implementation. Handles movement and collisions.
// All entities tracked by the physics system have a Transform and Rigidbody component.

#include "../../include/systems.h"
#include "../../include/ecs.h"
#include <stdlib.h>
#include <math.h>

CollisionEvent collisionEvents[MAX_EVENTS];
unsigned char collisionEventCount;

TilemapCollisionEvent tilemapCollisionEvents[MAX_EVENTS];
unsigned char tilemapCollisionEventCount;

BaseSystem* physicsSystem;

// Arbitrary gravity constant
#define GRAVITY 9.8f

/// Method to find the collision between two entities with colliders in adjacent spatial partitions.
bool FindCollision(Entity e1, Entity e2, CollisionEvent* event) {
    // Update entities
    event->object1 = e1;
    event->object2 = e2;

    // Determine the type of colliders
    if (HasComponent(e1, C_BOXCOLLIDER) && HasComponent(e2, C_BOXCOLLIDER)) {
        // AABB collision - assume entity transform rotation doesn't affect collider (for now)
        Transform* t1 = GetTransform(e1);
        BoxCollider* c1 = GetBoxCollider(e1);
        Transform* t2 = GetTransform(e2);
        BoxCollider* c2 = GetBoxCollider(e2);

        // Collision = overlap in both x and y axes
        Vec2 t1Half = v2scale(c1->size, 0.5f);
        Vec2 t1Min = v2sub(v2add(t1->pos, c1->offset), t1Half);
        Vec2 t1Max = v2add(t1Min, c1->size);

        Vec2 t2Half = v2scale(c2->size, 0.5f);
        Vec2 t2Min = v2sub(v2add(t2->pos, c2->offset), t2Half);
        Vec2 t2Max = v2add(t2Min, c2->size);

        // Collision happens if both boxes overlap
        if (t1Min.x < t2Max.x && t1Max.x > t2Min.x && t1Min.y < t2Max.y && t1Max.y > t2Min.y) {
            // Collision point is the center of the overlap area
            event->position.x = (fmax(t1Min.x, t2Min.x) + fmin(t1Max.x, t2Max.x)) / 2;
            event->position.y = (fmax(t1Min.y, t2Min.y) + fmin(t1Max.y, t2Max.y)) / 2;

            // Penetration distance is the minimum distance to each corner
            Vec2 penBL = v2sub(event->position, t2Min);
            Vec2 penTR = v2sub(t2Min, event->position);
            event->penetration = min(min(penBL.x, penTR.x), min(penBL.y, penTR.y));
            
            // Collision normal is the normal of the face on c2 with the least penetration
            if      (event->penetration == penBL.x) event->normal = (Vec2) { -1,  0 };
            else if (event->penetration == penTR.x) event->normal = (Vec2) {  1,  0 };
            else if (event->penetration == penBL.y) event->normal = (Vec2) {  0, -1 };
            else                                    event->normal = (Vec2) {  0,  1 };
            // that shortcut is only incorrect when the more robust massive if/else tree would also be incorrect

            return true;
        }

        return false;
    }
    // Otherwise, exactly one entity has a circle collider
    else if (HasComponent(e1, C_BOXCOLLIDER) || HasComponent(e2, C_BOXCOLLIDER)) {
        // Swap so that e1 is the circle collider
        if (HasComponent(e2, C_CIRCLECOLLIDER)) { Entity temp = e1; e1 = e2; e2 = temp; }

        // Circle vs AABB collision with same assumption as above.
        Transform* t1 = GetTransform(e1);
        CircleCollider* c1 = GetCircleCollider(e1);
        Transform* t2 = GetTransform(e2);
        BoxCollider* c2 = GetBoxCollider(e2);

        // Find the closest point on the AABB to the circle center
        Vec2 c1Center = v2add(t1->pos, c1->offset);
        Vec2 t2Half = v2scale(c2->size, 0.5f);
        Vec2 t2Min = v2sub(v2add(t2->pos, c2->offset), t2Half);
        Vec2 t2Max = v2add(t2Min, c2->size);
        Vec2 closestPoint;
        closestPoint.x = fmax(t2Min.x, fmin(c1Center.x, t2Max.x));
        closestPoint.y = fmax(t2Min.y, fmin(c1Center.y, t2Max.y));

        // Collision if the closest point is within the circle radius
        Vec2 offset = v2sub(closestPoint, c1Center);
        float distSq = v2sqmag(offset);
        if (distSq < c1->radius * c1->radius) {
            // Collision normal is direction from the circle center to the closest point on the box
            event->normal = v2scale(v2norm(offset), -1); // make sure it points away from c2

            // Penetration is half the distance between box point and circle edge
            float dist = sqrtf(distSq);
            event->penetration = 0.5f * (c1->radius - dist);

            // If collision point is the center of the overlap, then it's the closest point plus that penetration distance along the normal
            event->position = v2add(closestPoint, v2scale(event->normal, -event->penetration));
            return true;
        }

        return false;
    }
    // Otherwise both entities have circle colliders (player dashing into a coin)
    else {
        Transform* t1 = GetTransform(e1);
        CircleCollider* c1 = GetCircleCollider(e1);
        Transform* t2 = GetTransform(e2);
        CircleCollider* c2 = GetCircleCollider(e2);

        // Collision if the distance between centers is less than the sum of the radii
        Vec2 c1Center = v2add(t1->pos, c1->offset);
        Vec2 c2Center = v2add(t2->pos, c2->offset);
        Vec2 offset = v2sub(c1Center, c2Center);
        float distSq = v2sqmag(offset);
        float radiusSum = c1->radius + c2->radius;
        if (distSq < radiusSum * radiusSum) {
            // Normal points away from C2
            event->normal = v2norm(offset);
            // Penetration is half of the overlap amount
            float dist = sqrtf(distSq);
            event->penetration = 0.5f * (radiusSum - dist);
            // Collision point is either radius minus the penetration amount
            event->position = v2add(c2Center, v2scale(event->normal, c2->radius - event->penetration));
            return true;
        }

        return false;
    }
}

/// Tick update for PhysicsSystem
void PhysicsSystemUpdate(float dt) {
    // Reset queues
    collisionEventCount = 0;
    tilemapCollisionEventCount = 0;

    // For all entities, apply velocity and then gravity if applicable, and update spatial partition
    for (unsigned int i = 1; i < physicsSystem->entityCount; i++) {
        Entity e = physicsSystem->tracked[i];
        Transform* transform = GetTransform(e);
        Rigidbody* rigidbody = GetRigidbody(e);
        
        if (!(rigidbody->flags & 1)) { // if not immovable
            // Apply velocity
            transform->pos = v2add(transform->pos, v2scale(rigidbody->velocity, dt));

            // Apply gravity
            if (!(rigidbody->flags & 2)) { // if not unaffected by gravity
                rigidbody->velocity.y += GRAVITY * dt;
            }
        }

        // Update spatial partition
        rigidbody->partition = v2i(v2scale(transform->pos, 1.0f / 16.0f));

        // TODO: I don't think that calculation is exactly correct for negative positions, but it's close enough and only wrong on exact integers.
        // Negative positions should be offscreen anyway.
    }

    // Loop through all pairs of entities with both rigidbodies and colliders.
    for (unsigned int i = 1; i < physicsSystem->entityCount; i++) {
        Entity e1 = physicsSystem->tracked[i];
        Transform* t1 = GetTransform(e1);
        Rigidbody* r1 = GetRigidbody(e1);

        // Check if this entity has a collider
        if (!HasComponent(e1, C_BOXCOLLIDER) && !HasComponent(e1, C_CIRCLECOLLIDER)) continue;

        // TODO: Check ground tilemap

        // TODO: Check goop tilemap

        // Loop through all other rigidbodies
        for (unsigned int j = i + 1; j < physicsSystem->entityCount; j++) {
            Entity e2 = physicsSystem->tracked[j];
            Transform* t2 = GetTransform(e2);
            Rigidbody* r2 = GetRigidbody(e2);

            // Check if the other entity has a collider
            if (!HasComponent(e2, C_BOXCOLLIDER) && !HasComponent(e2, C_CIRCLECOLLIDER)) continue;

            // Skip if not in adjacent partitions
            // Fails for anything with a collider larger than like 32 or whatever so don't do that
            if (abs(r1->partition.x - r2->partition.x) > 1 || abs(r1->partition.y - r2->partition.y) > 1) continue;

            // Find the collision point. Writes to the queue regardless and only increments if a collision is found.
            if (collisionEventCount < MAX_EVENTS && FindCollision(e1, e2, &collisionEvents[collisionEventCount]))
                collisionEventCount++;
        }
    }

    // TODO: Resolve tilemap collisions

    // Resolve basic collisions with the following assumptions:
    // - All collisions result in velocity along the normal being set to 0
    // - All collisions between SOLID colliders are resolved as expected
    // - SEMISOLID colliders are always on immovable objects and thus only one of the colliding objects will be semisolid
    // - SEMISOLID colliders are always box colliders and the collision point is along the top edge
    // - ZONE colliders don't do anything here, just the events will be processed
    for (int i = 0; i < collisionEventCount; i++) {
        CollisionEvent* e = &collisionEvents[i];

        // If Entity 1 is a semisolid, swap to make sure e2 is the semisolid, before we do any pointer dereferencing
        if (HasComponent(e->object1, C_BOXCOLLIDER) && GetBoxCollider(e->object1)->type == COL_SEMISOLID ||
            HasComponent(e->object1, C_CIRCLECOLLIDER) && GetCircleCollider(e->object1)->type == COL_SEMISOLID) {
            // Swap object 1 and object 2 in the event
            Entity temp = e->object1;
            e->object1 = e->object2;
            e->object2 = temp;
        }

        // Get colliders
        Transform* t1 = GetTransform(e->object1);
        Rigidbody* r1 = GetRigidbody(e->object1);
        BoxCollider* cb1 = HasComponent(e->object1, C_BOXCOLLIDER) ? GetBoxCollider(e->object1) : NULL;
        CircleCollider* cc1 = HasComponent(e->object1, C_CIRCLECOLLIDER) ? GetCircleCollider(e->object1) : NULL;
        void* col1 = cb1 != NULL ? (void*)cb1 : (void*)cc1;
        ColliderType type1 = cb1 != NULL ? cb1->type : cc1->type;

        Transform* t2 = GetTransform(e->object2);
        Rigidbody* r2 = GetRigidbody(e->object2);
        BoxCollider* cb2 = HasComponent(e->object2, C_BOXCOLLIDER) ? GetBoxCollider(e->object2) : NULL;
        CircleCollider* cc2 = HasComponent(e->object2, C_CIRCLECOLLIDER) ? GetCircleCollider(e->object2) : NULL;
        void* col2 = cb2 != NULL ? (void*)cb2 : (void*)cc2;
        ColliderType type2 = cb2 != NULL ? cb2->type : cc2->type;

        // Skip zone collisions
        if (type1 == COL_ZONE || type2 == COL_ZONE) continue;

        // Handle semisolid collisions, having set e2 to be the semisolid
        if (type2 == COL_SEMISOLID) {
            float c1Bottom = cb1 != NULL ?
                t1->pos.y + cb1->offset.y - cb1->size.y * 0.5f :
                t1->pos.y + cc1->offset.y - cc1->radius;
            float topFace = t2->pos.y + cb2->offset.y + cb2->size.y * 0.5f;
            // Collision if object is moving downward and the collision normal is mostly pointing up
            if (v2dot(e->normal, (Vec2){ 0, 1 }) >= 0.707f && r1->velocity.y < 0) {
                // Update t1 position such that the collider bottom is exactly on the top face
                float distance = topFace - c1Bottom;
                t1->pos.y += distance;
                r1->velocity.y = 0;
            }
        }

        // Handle solid collisions
        else {
            // Move objects backward on the collision normal such that they don't overlap
            bool imm1 = r1->flags & 1;
            bool imm2 = r2->flags & 1;
            if (!imm1 && !imm2) {
                t1->pos = v2add(t1->pos, v2scale(e->normal, e->penetration));
                t2->pos = v2add(t2->pos, v2scale(e->normal, -e->penetration));
            }
            else if (!imm1)
                t1->pos = v2add(t1->pos, v2scale(e->normal, 2 * e->penetration));
            else if (!imm2)
                t2->pos = v2add(t2->pos, v2scale(e->normal, 2 * -e->penetration));

            // Update velocities to remove the component pointing toward the normal, which should assume that entities always fully stop
            // in that direction when colliding.
            // The normal should be pointing away from object 2float dot1 = v2dot(r1->velocity, normal);
            float dot1 = v2dot(r1->velocity, e->normal);
            if (dot1 < 0)
                r1->velocity = v2sub(r1->velocity, v2scale(e->normal, dot1));
            float dot2 = v2dot(r2->velocity, e->normal);
            if (dot2 > 0)
                r2->velocity = v2sub(r2->velocity, v2scale(e->normal, dot2));
        }
    }
}