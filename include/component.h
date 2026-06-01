/// Contains definitions for components.
#pragma once;

#include "smath.h"

// ID of an entity.
// Importantly: the id 0 is reserved.
typedef unsigned short Entity;

// Spatial data for entities
// Component ID: 1
typedef struct {
    // Position in world space
    Vec2 pos;
    // Rotation about z axis
    float rotation;
} Transform;
#define C_TRANSFORM 1

// Appearance data for entities
// Component ID: 2
typedef struct {
    // The actual sprite
    unsigned char spr;
    // If the sprite should be flipped horizontally
    bool flipX;
} Sprite;
#define C_SPRITE 2

// Handles changing sprite over time
// Component ID: 3
typedef struct {
    // ID of the animation, which is defined globally.
    unsigned int animation;
    // Time in seconds through this animation
    float time;
} Animator;
#define C_ANIMATOR 3

typedef enum { COL_SOLID, COL_ZONE, COL_SEMISOLID } ColliderType;

// Collider information for an AABB
// Component ID: 4
typedef struct {
    // Offset with regard to the transform component center
    Vec2 offset;
    // Width and height of the box
    Vec2 size;
    // Solidity. 0 = solid, 1 = zone, 2 = semisolid (only collide from top)
    ColliderType type;
} BoxCollider;
#define C_BOXCOLLIDER 4

// Collider information for a circle collider, used for the player when they are dashing
// Component ID: 5
typedef struct {
    // Offset with regard to the transform component center
    Vec2 offset;
    // Radius of the circle
    float radius;
} CircleCollider;
#define C_CIRCLECOLLIDER 5

// Deals with velocity and gravity
// Component ID: 6
typedef struct {
    // How quickly the object is moving
    Vec2 velocity;
    // Boolean object flags
    // 1 - immovable
    // 2 - unaffected by gravity
    unsigned char flags;
} Rigidbody;
#define C_RIGIDBODY 6

// Shows how much health an enemy or breakable object has
// Component ID: 7
typedef struct {
    // How much health is left
    char health;
} Health;
#define C_HEALTH 7

// Coins, gems, and stars have similar properties
// Component ID: 8
typedef struct {
    // If the object has been collected and is currently playing the animation
    bool collected;
    // Type of the collectible. 0 = coin, 1-3 = types of gem, 4 = star
    unsigned char type;
} Collectible;
#define C_COLLECTIBLE 8

// Falling platforms just store if they are triggered or not
// Component ID: 9
// Just as a tag. When triggered, remove collider and add rigidbody.

#define C_PLATFORM 9

typedef enum { PLR_STANDING, PLR_WALKING, PLR_MIDAIR, PLR_MIDAIR_JUMPDASH, PLR_WALLSLIDE, PLR_CROUCHING, PLR_PREPDASH, PLR_DASH, PLR_DEAD } PlayerState;

// Stores all data about a player
// Component ID: 10
typedef struct {
    // Enum for player state (not bit flags)
    // 0 - standing
    // 1 - walking
    // 2 - midair
    // 3 - midair, jump/dash expended
    // 4 - wallslide
    // 5 - crouching
    // 6 - preparing dash
    // 7 - dash
    // 8 - dead
    PlayerState state;
    // If dashing, this is the normalized vector we are dashing toward
    Vec2 dashDirection;
} Player;
#define C_PLAYER 10

// Defines an object as an enemy
// Component ID: 11
// Just a tag

#define C_ENEMY 11

typedef enum { BUMBLE_REGULAR, BUMBLE_HAT, BUMBLE_FLYING, BUMBLE_HELMET } BumbleType;

// Bumble definition
// Component ID: 12
typedef struct {
    // Type of bumble
    // 0 - regular
    // 1 - dumble (2 health, converted to bumble when jumping on head)
    // 2 - fumble (2 health, not affected by gravity, converted to bumble when jumping on head)
    // 3 - gumble (unkillable, unaffected when jumping on head)
    BumbleType type;
    // Direction
    float direction;
} Bumble;
#define C_BUMBLE 12

// Drillbugs are thwomps that you can stand on top of
// Component ID: 13
typedef struct {
    // The height the drillbug starts at and will try to return to upon being stopped
    float defaultHeight;
    // ID of the ground-like collider that moves with the drillbug and allows the player to stand
    Entity platform;
    // ID of the drillbug's detection zone, a box collider reaching from slightly above the defaultHeight to the nearest ground
} Drillbug;
#define C_DRILLBUG 13

// The bossfight enemy, King Wumble
// Component ID: 14
typedef struct {
    // How many shields the boss has left
    unsigned char shieldsLeft;
    // How far left the boss should move
    float leftBound;
    // How far right the boss should move
    float rightBound;
    // How long until the boss spawns random enemies
    float spawnTimer;
    // Direction of current movement
    float direction;
} Boss;
#define C_BOSS 14

// A raft that moves along the surface of goop, and
// stops if too many enemies are on it at once
// Component ID: 15
typedef struct {
    // How many enemies are currently on the raft
    unsigned int count;
} Raft;
#define C_RAFT 15

// An audio source
// Component ID: 16
typedef struct {
    // ID of the audio clip
    char clip;
    // Current play time of the clip
    float time;
    // Flags for this audio source
    // 1 = if this audio source is paused
    // 2 = if the audio source should loop
    unsigned char flags
} AudioSource;
#define C_AUDIOSOURCE 16

// Tells particles and dead enemies to be destroyed when offscreen
// Component ID: 17
typedef struct {
    // Circumscribed radius of the object to ensure that it's fully offscreen when destroyed
    float radius;
} DestroyOffscreen;
#define C_DESTROYOFFSCREEN 17

// How many unique components there are
#define COMPONENT_TYPE_COUNT 17

// Component ID matching the macros in this file
typedef unsigned char ComponentID;

// Bit flags that define what components an entity has or a system uses
typedef unsigned int Signature;

// Creates a signature from a component ID.
// Example for a signature that involves 4 components
// signature = SG(C_TRANSFORM) | SG(C_RIGIDBODY) | SG(C_HEALTH) | SG(C_SPRITE)
#define SG(a) (1u << ((a)-1))