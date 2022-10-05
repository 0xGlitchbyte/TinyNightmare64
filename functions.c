/***************************************************************
                       functions.c
                               
             Contains some convenience functions
***************************************************************/
#include "functions.h"


/*==============================
    set_cam
    Sets the cam
==============================*/

void set_cam(Camera *camera, Entity entity){

    // Setup the cam.projection matrix
    guPerspective(
    	&camera->projection, &camera->normal, 
        45, (float)SCREEN_WD / (float)SCREEN_HT, 
    	10.0, 10000.0, 0.01);
    
    guLookAt(
    	&camera->viewing,
    	camera->pos[0], camera->pos[1], camera->pos[2],
    	entity.pos[0], entity.pos[1], entity.pos[2],
    	0, 0, 1
  	);
}