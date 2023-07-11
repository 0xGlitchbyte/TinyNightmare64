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

/*

static s64Animation anims_nick[] = {
    {"_0_t_pose", 1, anim_nick__0_t_pose_keyframes},
    CROUCH_IDLE
    CROUCH_TO_RUN
    CROUCH_TO_STAND
    FALL_IDLE
    FALL_TO_STAND
    JOG
    JUMP
    LOOK_AROUND
    RUN_ARC
    RUN
    RUN_TO_ROLL
    RUN_TO_STAND
    SPRINT
    STAND_IDLE
    STAND_TO_CROUCH
    STAND_TO_JUMP
    STAND_TO_ROLL
    STAND_TO_RUN
    TAP_SHOE
    WALK
*/
 typedef enum { 
         
    CROUCH,
    //CROUCH_TO_RUN,
    //CROUCH_TO_STAND,
    FALL,
    //FALL_TO_STAND,
    JOG,
    JUMP,
    //LOOK_AROUND,
    //RUN_ARC,
    RUN,
    //RUN_TO_ROLL,
    ROLL,
    //RUN_TO_STAND,
    SPRINT,
    //STAND_IDLE,
    IDLE, // TODO - maybe rename to STAND
    //STAND_TO_CROUCH,
    //STAND_TO_JUMP,
    //STAND_TO_ROLL,
    //STAND_TO_RUN,
    //TAP_SHOE,
    WALK,
		FALLBACK, // TODO - maybe delete
		MIDAIR // TODO - maybe delete
 } EntityState;

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
	EntityState state;
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
