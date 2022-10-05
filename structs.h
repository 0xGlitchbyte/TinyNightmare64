/*********************************
            Structs
*********************************/

#ifndef STRUCTS_H
#define STRUCTS_H


typedef struct {
	
	Mtx	pos_mtx;
	Mtx	rotx;
	Mtx roty;
	Mtx rotz;
	Mtx scale;
	float pos[3];
	float dir[3];
	float pitch;
	float yaw;
	float size[3];
	float speed;
	Mtx modeling;
	s64ModelHelper helper;
} Entity;

typedef struct {    
	Mtx projection;
	Mtx viewpoint;
	Mtx camRot;
	u16 normal;
	float pos[3];
	float pitch;
	float yaw;
	float camang[3];
} Camera;

#endif
