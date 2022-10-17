/*********************************
            Structs
*********************************/

#ifndef STRUCTS_H
#define STRUCTS_H


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

typedef struct{
    Light amb;
    Light dir;
	float angle[3];
	int ambcol;

}LightData;

typedef struct {
	Mtx	pos_mtx;
	Mtx	rotx;
	Mtx roty;
	Mtx rotz;
	Mtx scale;
	float size[3];
	float pos[3];
	float dir[3];
	float pitch;
	float yaw;
	float speed;
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
