#include "../../include/smath.h"
#include <math.h>

Vec2 v2add(Vec2 a, Vec2 b) {
    return (Vec2) { a.x + b.x, a.y + b.y };
}

Vec2 v2sub(Vec2 a, Vec2 b) {
    return (Vec2) { a.x - b.x, a.y - b.y };
}

Vec2 v2scale(Vec2 a, float k) {
    return (Vec2) { a.x * k, a.y * k };
}

float v2dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float v2sqmag(Vec2 a) {
    return v2dot(a, a);
}

float v2mag(Vec2 a) {
    return sqrt(v2sqmag(a));
}

Vec2 v2norm(Vec2 a) {
    return v2scale(a, 1.0f / v2mag(a));
}

Vec2Int v2i(Vec2 a) {
    return (Vec2Int) { (int)a.x, (int)a.y };
}

Vec2Int v2iadd(Vec2Int a, Vec2Int b) {
    return (Vec2Int) { a.x + b.x, a.y + b.y };
}

Vec2Int v2iscale(Vec2Int a, float k) {
    return (Vec2Int) { (int) (a.x * k), (int) (a.y * k) };
}
