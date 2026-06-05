/// Soruce for the RenderingSystem

#include <citro2d.h>

#include "systems.h"
#include "ecs.h"

BaseSystem* renderSystem;

// TEMPORARY IMPLEMENTATION
Vec2Int WorldToScreenPos(Vec2 world) {
    return v2i(v2scale(world, 16)); // scale up so that each unit is 16 pixels
}

void RenderSystemUpdate(float dt) {
    // For every object with a transform and a sprite
    for (unsigned int i = 1; i < renderSystem->entityCount; i++) {
        Entity e = renderSystem->tracked[i];
        Transform* transform = GetTransform(e);
        Sprite* sprite = GetSprite(e);

        // TEMPORARY: Fill in with white based on collider
        u32 white = C2D_Color32f(1.0f, 1.0f, 1.0f, 1.0f);

        // If it has a box collider, draw a rectangle over the bounds
        if (HasComponent(e, C_BOXCOLLIDER)) {
            BoxCollider* col = GetBoxCollider(e);

            Vec2 p = v2add(transform->pos, col->offset);
            Vec2 h = v2scale(col->size, 0.5f);
            Vec2Int tl = WorldToScreenPos((Vec2) { p.x - h.x, p.y - h.y });
            Vec2Int br = WorldToScreenPos((Vec2) { p.x + h.x, p.y + h.y }); // TODO: FLIP Y WHEN WORLDTOSCREENPOSISIMPLEMENTED
            Vec2Int screenSize = (Vec2Int) { br.x - tl.x, br.y - tl.y };

            C2D_DrawRectSolid(tl.x, tl.y, 0, screenSize.x, screenSize.y, white);
        }

        // If it has a circle collider, draw a circle over the bounds
        else if (HasComponent(e, C_CIRCLECOLLIDER)) {
            CircleCollider* col = GetCircleCollider(e);

            Vec2 center = v2add(transform->pos, col->offset);
            Vec2Int screenCenter = WorldToScreenPos(center);
            Vec2Int screenEdge = WorldToScreenPos(v2add(center, (Vec2) { col->radius, 0 }));
            int screenRadius = screenEdge.x - screenCenter.x;
            C2D_DrawCircleSolid(screenCenter.x, screenCenter.y, 0, screenRadius, white);
        }

        // If it has no collider, draw a point
        else {
            printf("\x1b[13;1HHas no collider");
            Vec2Int p = WorldToScreenPos(transform->pos);
            C2D_DrawRectSolid(p.x, p.y, 0, 1, 1, white);
        }
    }
}