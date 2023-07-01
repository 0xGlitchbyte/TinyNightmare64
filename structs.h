/*********************************
            Structs
*********************************/

#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct{
	OSTime cur_frame;
	OSTime last_frame;
	float frame_duration;
	f32 FPS;
	u8 FPS_index;
	f32 FPS_average[10];
}TimeData;

typedef struct{
    Light amb;
    Light dir;
	float angle[3];
	int ambcol;
}LightData;

typedef struct {
	Mtx modeling;
	Mtx projection;
	Mtx viewpoint;
	Mtx camRot;
	u16 normal;

	float distance_from_entity;
	float horizontal_distance_from_entity;
	float vertical_distance_from_entity;
    float angle_around_entity;

	float pos[3];
	float pitch;
	float yaw;
	float roll;
} Camera;

typedef enum { 
    IDLE, 
    WALK, 
   	RUN,
	ROLL,
   	JUMP,
	FALL,
	MIDAIR,
	FALLBACK,	
} entity_state;

typedef enum {
	NICK,
	WILLY,
	SKELLY
} entity_type;

typedef struct {
	int health;
	int damage;
	int ammo;
} BaseMechanics;

typedef struct {
	Mtx	pos_mtx;
	Mtx	rot_mtx[3];
	Mtx scale_mtx;
	float size[3];
	float pos[3];
	float dir[3];
	float scale;
	float pitch;
	float yaw;
	float speed;
	float vertical_speed;
	float forward_speed;
	float side_speed;
	entity_state state;
	entity_type type;
	BaseMechanics health;
	BaseMechanics damage;
	BaseMechanics ammo;	
} Entity;

typedef struct {
	Entity entity;
	s64ModelHelper helper;
} AnimatedEntity;


typedef struct {
	Entity entity;
	Gfx *mesh;
} StaticEntity;


#endif
