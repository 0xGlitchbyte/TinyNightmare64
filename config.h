#ifndef UNFLEX_CONFIG_H
#define UNFLEX_CONFIG_H

#include "sausage64.h"

    /*********************************
           Configuration Macros
    *********************************/

    // TV Types
    #define NTSC    0
    #define PAL     1
    #define MPAL    2
    
    // TV setting
    #define TV_TYPE NTSC

    // Rendering resolution
    #define SCREEN_WD 320
    #define SCREEN_HT 240

    // Array sizes
    #define GLIST_LENGTH 4096
    #define HEAP_LENGTH  1024
    
    
    /*********************************
                 Globals
    *********************************/
    
    extern Gfx glist[];
    extern Gfx* glistp;
    
    extern Gfx rspinit_dl[];
    extern Gfx rdpinit_dl[];
    
    extern NUContData contdata[1];

    /*********************************
                 Variables
    *********************************/


    typedef struct {    
    //moved the entity variables that Jason added
	//Camera params
	Mtx projection;
	Mtx modeling;
	Mtx viewing;
	Mtx camRot;   
	//Cube-specific params
	Mtx	rotx;
	Mtx roty;
	Mtx	pos_mtx;
	Mtx scale;
	float pos[3];
	float dir[3];
	float speed;
	float pitch;
	float yaw;

    s64ModelHelper helper;
    //I changed the name "pan" to "pitch", this struct is inspired by the
    //"Dynamic" struct from our earlier example, but the word "pan"
    //means something else in cameras/graphics
    //https://en.wikipedia.org/wiki/Aircraft_principal_axes
    //also from the N64 library docs:
    //*Return rotation matrix given roll, pitch, and yaw in degrees*
    //void guRotateRPYF(float mf[4][4], float r, float p, float h)
    } Entity;

    
#endif