#pragma once;

// 2D float vector struct
typedef struct {
    float x;
    float y;
} Vec2;

Vec2 v2add(Vec2 a, Vec2 b);
Vec2 v2sub(Vec2 a, Vec2 b);
Vec2 v2scale(Vec2 a, float k);
float v2dot(Vec2 a, Vec2 b);
float v2sqmag(Vec2 a);
float v2mag(Vec2 a);
Vec2 v2norm(Vec2 a);

// 2D integer vector struct
typedef struct {
    int x;
    int y;
} Vec2Int;

Vec2Int v2i(Vec2 a);
Vec2Int v2iadd(Vec2Int a, Vec2Int b);
Vec2Int v2iscale(Vec2Int a, float k);