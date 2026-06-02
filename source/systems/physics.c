/// Physics system implementation. Handles movement and collisions.
// All entities tracked by the physics system have a Transform and Rigidbody component.

#include "../../include/systems.h"
#include "../../include/ecs.h"
#include <stdlib.h>

CollisionEvent collisionEvents[MAX_EVENTS];
unsigned char collisionEventCount;

TilemapCollisionEvent tilemapCollisionEvents[MAX_EVENTS];
unsigned char tilemapCollisionEventCount;

BaseSystem* physicsSystem;

// Arbitrary gravity constant
#define GRAVITY 9.8f

/// Method to find the collision between two entities with colliders in adjacent spatial partitions.
bool FindCollision(Entity e1, Entity e2, Vec2* collisionPoint) {
    collisionPoint->x = 0;
    collisionPoint->y = 0;

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

        // Collision point is the center of the overlap area
        if (t1Min.x < t2Max.x && t1Max.x > t2Min.x && t1Min.y < t2Max.y && t1Max.y > t2Min.y) {
            collisionPoint->x = (fmax(t1Min.x, t2Min.x) + fmin(t1Max.x, t2Max.x)) / 2;
            collisionPoint->y = (fmax(t1Min.y, t2Min.y) + fmin(t1Max.y, t2Max.y)) / 2;
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
        float distSq = v2sqmag(v2sub(closestPoint, c1Center));
        if (distSq < c1->radius * c1->radius) {
            collisionPoint->x = closestPoint.x;
            collisionPoint->y = closestPoint.y;
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
        float distSq = v2sqmag(v2sub(c1Center, c2Center));
        float radiusSum = c1->radius + c2->radius;
        if (distSq < radiusSum * radiusSum) {
            collisionPoint->x = (c1Center.x + c2Center.x) / 2;
            collisionPoint->y = (c1Center.y + c2Center.y) / 2;
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

            // Find the collision point
            Vec2 collisionPoint;
            bool collided = FindCollision(e1, e2, &collisionPoint);
            if (collided && collisionEventCount < MAX_EVENTS) {
                collisionEvents[collisionEventCount++] = (CollisionEvent){ e1, e2, collisionPoint };
            }
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
            // Only resolve if e1's collider bottom is above the top face and e1 is moving downward
            float c1Bottom = cb1 != NULL ?
                t1->pos.y + cb1->offset.y - cb1->size.y * 0.5f :
                t1->pos.y + cc1->offset.y - cc1->radius;
            float topFace = t2->pos.y + cb2->offset.y + cb2->size.y * 0.5f;
            if (c1Bottom < topFace + 0.1f && r1->velocity.y < 0) {
                // Update t1 position such that the collider bottom is exactly on the top face
                float distance = topFace - c1Bottom;
                t1->pos.y += distance;
                r1->velocity.y = 0;
            }
            // if we include collision normal in events it would be easier to just check if the normal points upward enough
        }

        // Handle solid collisions
        else {
            Vec2 normal; // Points away from C2
            float p; // Penetration, how far the collision point is into each object. We will move each object apart by this distance along the normal

            // Honestly it might have been beneficial to calculate normal when doing the collision.
            // Will move it there if we need it in any other events otherwise it's better here

            // If c2 is a box and c1 is a box
            if (cb1 != NULL && cb2 != NULL) {
                // Collision point should be close to the edges so find the axis with the least overlap
                Vec2 h = v2scale(cb2->size, 0.5f);
                float pL = e->position.x - (t2->pos.x + cb2->offset.x - h.x);
                float pR = (t2->pos.x + cb2->offset.x + h.x) - e->position.x;
                float pB = e->position.y - (t2->pos.y + cb2->offset.y - h.y);
                float pT = (t2->pos.y + cb2->offset.y + h.y) - e->position.y;
                p = min(min(pL, pR), min(pB, pT)); // shortcut is inaccurate if penetration distance is the same in 2 directions but that would cause issues w/o the shortcut anyway
                if      (p == pL) normal = (Vec2) { -1,  0 };
                else if (p == pR) normal = (Vec2) {  1,  0 };
                else if (p == pB) normal = (Vec2) {  0, -1 };
                else              normal = (Vec2) {  0,  1 };
                // p is correct because box-box collisions give us the exact midpoint
            }
            // If c1 is a box and c2 is a circle
            else if (cb1 != NULL) {
                // Collision normal is direction from the circle center to the closest point on the box, which is our collision poisition
                Vec2 center = v2add(t2->pos, cc2->offset);
                Vec2 offset = v2sub(e->position, center);
                normal = v2norm(offset);

                // Since the collision point is on the box, penetration for each is half of the distance to the circle
                p = cc2->radius - v2mag(offset);
            }
            // If c2 is a box and c1 is a circle
            else if (cb2 != NULL) {
                Vec2 center = v2add(t1->pos, cc1->offset);
                Vec2 offset = v2sub(e->position, center);
                normal = v2scale(v2norm(offset), -1); // flip so it points away from obj2
                p = cc1->radius - v2mag(offset);
            }
            // If both are circles
            else {
                // Collision point should be right in the middle
                Vec2 center2 = v2add(t2->pos, cc2->offset);
                Vec2 offset = v2sub(e->position, center2);
                normal = v2norm(offset);
                p = v2mag(offset);
            }

            // Move objects backward on the collision normal such that they don't overlap
            bool imm1 = r1->flags & 1;
            bool imm2 = r2->flags & 1;
            if (!imm1 && !imm2) {
                t1->pos = v2add(t1->pos, v2scale(normal, p));
                t2->pos = v2add(t2->pos, v2scale(normal, -p));
            }
            else if (!imm1)
                t1->pos = v2add(t1->pos, v2scale(normal, 2 * p));
            else if (!imm2)
                t2->pos = v2add(t2->pos, v2scale(normal, 2 * -p));

            // Update velocities to remove the component pointing toward the normal, which should assume that entities always fully stop
            // in that direction when colliding.
            // The normal should be pointing away from object 2float dot1 = v2dot(r1->velocity, normal);
            float dot1 = v2dot(r1->velocity, normal);
            if (dot1 < 0)
                r1->velocity = v2sub(r1->velocity, v2scale(normal, dot1));
            float dot2 = v2dot(r2->velocity, normal);
            if (dot2 > 0)
                r2->velocity = v2sub(r2->velocity, v2scale(normal, dot2));
        }
    }
}