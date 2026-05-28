/// Contains definitions for components.
#pragma once;

#include <vector.h>

// ID of an entity
typedef unsigned short Entity;

// Spatial data for entities
// Component ID: 1
struct Transform {
    // Position in world space
    Vec2 pos;
    // Rotation about z axis
    float rotation;
};
#define C_TRANSFORM 1

// Appearance data for entities
// Component ID: 2
struct Sprite {
    // The actual sprite
    char spr;
    // If the sprite should be flipped horizontally
    bool flipX;
};
#define C_SPRITE 2

// Handles changing sprite over time
// Component ID: 3
struct Animator {
    // ID of the animation, which is defined globally.
    int animation;
    // Time in seconds through this animation
    float time;
};
#define C_ANIMATOR 3

// Collider information for an AABB
// Component ID: 4
struct BoxCollider {
    // Offset with regard to the transform component center
    Vec2 offset;
    // Width and height of the box
    Vec2 size;
    // Solidity. 0 = solid, 1 = zone, 2 = semisolid (only collide from top)
    char type;
};
#define C_BOXCOLLIDER 4

// Collider information for a circle collider, used for the player when they are dashing
// Component ID: 5
struct CircleCollider {
    // Offset with regard to the transform component center
    Vec2 offset;
    // Radius of the circle
    float radius;
};
#define C_CIRCLECOLLIDER 5

// Deals with velocity and gravity
// Component ID: 6
struct Rigidbody {
    // How quickly the object is moving
    Vec2 velocity;
    // If the object should be affected by gravity
    bool useGravity;
};
#define C_RIGIDBODY 6

// Shows how much health an enemy or breakable object has
// Component ID: 7
struct Health {
    // How much health is left
    char health;
};
#define C_HEALTH 7

// Coins, gems, and stars have animations
// Component ID: 8
struct Collectable {
    // If the object has been collected and is currently playing the animation
    bool collected;
    // Type of the collectable. 0 = coin, 1-3 = types of gem, 4 = star
    char type;
};
#define C_COLLECTABLE 8

// Falling platforms just store if they are triggered or not
// Component ID: 9
// Just as a tag. When triggered, remove collider and add rigidbody.

#define C_PLATFORM 9

// Stores all data about a player
// Component ID: 10
struct Player {
    // Enum for player state
    // 0 - standing
    // 1 - walking
    // 2 - midair
    // 3 - midair, jump/dash expended
    // 4 - wallslide
    // 5 - crouching
    // 6 - preparing dash
    // 7 - dash
    // 8 - dead
    char state;
    // If dashing, this is the normalized vector we are dashing toward
    Vec2 dashDirection;
};
#define C_PLAYER 10

// Defines an object as an enemy
// Component ID: 11
// Just a tag

#define C_ENEMY 11

// Bumble definition
// Component ID: 12
struct Bumble {
    // Type of bumble
    // 0 - regular
    // 1 - dumble (2 health, converted to bumble when jumping on head)
    // 2 - fumble (2 health, not affected by gravity, converted to bumble when jumping on head)
    // 3 - gumble (unkillable, unaffected when jumping on head)
    char type;
    // Direction
    float direction;
};
#define C_BUMBLE 12

// Drillbugs are thwomps
// Component ID: 13
struct Drillbug {
    // The height the drillbug starts at and will try to return to upon being stopped
    float defaultHeight;
    // ID of the ground-like collider that moves with the drillbug and allows the player to stand
    Entity platform;
};
#define C_DRILLBUG 13

// The bossfight enemy, King Wumble
// Component ID: 14
struct Boss {
    // How many shields the boss has left
    char shieldsLeft;
    // How far left the boss should move
    float leftBound;
    // How far right the boss should move
    float rightBound;
    // How long until the boss spawns random enemies
    float spawnTimer;
    // Direction of current movement
    float direction;
};
#define C_BOSS 14

// A raft that moves along the surface of goop, and
// stops if too many enemies are on it at once
// Component ID: 15
struct Raft {
    // How many enemies are currently on the raft
    int count;
};
#define C_RAFT 15

// An audio source
// Component ID: 16
struct AudioSource {
    // ID of the audio clip
    char clip;
    // If the clip is currently playing
    bool playing;
    // If the clip should loop
    bool loop;
    // Current play time of the clip
    float time;
};
#define C_AUDIO 16

// Tells particles and dead enemies to be destroyed when offscreen
// Component ID: 17
struct DestroyOffscreen {
    // Circumscribed radius of the object to ensure that it's fully offscreen when destroyed
    float radius;
};
#define C_DESTROYOFFSCREEN 17

// Bit flags that define what components an entity has or a system uses
typedef unsigned int Signature;