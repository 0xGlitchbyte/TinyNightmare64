/***************************************************************
                           stage00.c
                
  first acomplished attempt to render and move nick around
             and make camera follow him!
***************************************************************/

#include <math.h>
#include <nusys.h>
#include <string.h> // Needed for CrashSDK compatibility
#include "config.h"
#include "structs.h"
#include "helper.h"
#include "sausage64.h"
#include "texcube.h"
#include "palette.h"
#include "nick.h"
#include "willy.h"
#include "ground_block.h"
#include "candy.h"
#include "pumpkin.h"
#include "gravestone.h"
#include "gravestone_cross.h"
#include "axisMdl.h"
#include "debug.h"


/*********************************
              Macros
*********************************/

#define USB_BUFFER_SIZE 256
#define COS_45 0.7071

/*********************************
        Function Prototypes
*********************************/

float get_time();
float rad(float angle);
float deg(float rad);
int lim(u32 input);

void time_management(TimeData *time);

void move_entity_analog_stick(Entity *entity, Camera camera, NUContData cont[1]);
void handle_camera_c_buttons(Camera *camera, NUContData cont[1]);

void move_entity_c_buttons(Entity *entity, Camera camera, NUContData cont[1]);
void handle_camera_analog_stick(Camera *camera, NUContData cont[1]);

void get_cam_position(Camera *camera, Entity *entity);
void move_cam(Camera *camera, Entity* entity, NUContData cont[1]);

void set_light(LightData *light);
void set_cam(Camera *camera, Entity entity);

void set_entity_state(AnimatedEntity * animated_entity, entity_state new_state);
void animate_nick(NUContData cont[1]);
void nick_animcallback(u16 anim);
void willy_animcallback(u16 anim);

void draw_animated_entity(AnimatedEntity *entity);
void draw_static_entity(StaticEntity *static_entity);
void draw_world(AnimatedEntity *entity, Camera *camera, LightData *light);

void draw_debug_data();

/*********************************
             Globals
*********************************/

//Variables
TimeData time_data = {
    FPS_index: 0,
};
float animspeed;

// Camera
Camera cam = {
    distance_from_entity: 700,
    pitch: 30,
    angle_around_entity: 0,
};

LightData light_data = {
    angle: { 0, 0, -90},
    ambcol: 100,
};


// Entities
AnimatedEntity nick = {
    entity: {
        pos: { 0, 0, 0},
        type: NICK
    }
};

Mtx nickMtx[MESHCOUNT_nick];

AnimatedEntity willy = {
    entity: {
        pos: { 400, 400, 0},
        yaw: 180,
        type: WILLY
    }
};

Mtx willyMtx[MESHCOUNT_willy];

StaticEntity axis = {
    entity: {
        pos: { 0, 0, 0},
    },
    mesh: gfx_axis,
};

StaticEntity ground = {
    entity: {
        pos: { -500, 500, 80},
    },
    mesh: gfx_ground,
};


StaticEntity candy = {
    entity: {
        pos: { -500, 500, 150},
    },
    mesh: gfx_candy,
};

StaticEntity pumpkin = {
    entity: {
        pos: { -300, 300, 50},
    },
    mesh: gfx_pumpkin,
};


StaticEntity gravestone = {
    entity: {
        pos: { 300, 300, 50},
    },
    mesh: gfx_gravestone,
};

StaticEntity gravestone_cross = {
    entity: {
        pos: { 300, -300, 50},
    },
    mesh: gfx_gravestone_cross,
};

// USB
static char uselight = TRUE;
static char drawaxis = TRUE;
static char freezelight = FALSE;
static char usb_buffer[USB_BUFFER_SIZE];


/*==============================
    get_time
    returns time in seconds
==============================*/

float get_time(){

    float time = (s32)OS_CYCLES_TO_USEC(osGetTime()) / 1000000.0f;
    return time;
}

/*==============================
    cycles_to_sec
    converts cycles to seconds
==============================*/

float cycles_to_sec(OSTime cycles){

    float time = (s32)OS_CYCLES_TO_USEC(cycles) / 1000000.0f;
    return time;
}

  
/*==============================
    rad & deg
    convert between angles and radians
==============================*/

float rad(float angle){
	float radian = M_PI / 180 * angle;
	return radian;
}

float deg(float rad){
	float angle = 180 / M_PI * rad;
	return angle;
}


/*==============================
    lim
    auxiliary function for 8 directional movement
==============================*/

int lim(u32 input){
    if (input == 0) {return 0;}
    else {return 1;}
}


/*==============================
    time_management
    calculates FPS and frame_duration variable    
==============================*/

void time_management(TimeData *time){

    time->cur_frame = osGetTime();

    time->frame_duration = cycles_to_sec(time->cur_frame - time->last_frame);

    time->FPS = 1 / time->frame_duration;

    time->last_frame = time->cur_frame;

}

// https://en.wikipedia.org/wiki/Fast_inverse_square_root
float Q_rsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}



/*==============================
    move_entity
    Moves entity with analog stick
==============================*/

int curr_nick_state;

void move_entity_analog_stick(Entity *entity, Camera camera, NUContData cont[1]){
	
	if (fabs(cont->stick_x) < 7){cont->stick_x = 0;}
	if (fabs(cont->stick_y) < 7){cont->stick_y = 0;}

    int curr_state = entity->state;
    curr_nick_state = curr_state;
    if (curr_state != ROLL
        && curr_state != JUMP
        && curr_state != FALL
        && curr_state != MIDAIR 
        ) {
        if ((cont->stick_x != 0 || cont->stick_y != 0)) {
            entity->yaw = deg(atan2(cont->stick_x, -cont->stick_y) - rad(camera.angle_around_entity));
            entity->speed = 1/Q_rsqrt(cont->stick_x * cont->stick_x + cont->stick_y * cont->stick_y) * 12;
        }

        if ( cont->stick_x == 0 && cont->stick_y == 0) {
            entity->speed = 0;
        }
    }
}

void move_entity_one_frame(Entity *entity){

    float frame_distance = time_data.frame_duration * entity->speed;
    entity->pos[0] += frame_distance * sin(rad(entity->yaw));
    entity->pos[1] -= frame_distance * cos(rad(entity->yaw));
}

void move_animated_entity_one_frame(AnimatedEntity *animated_entity){
    Entity *entity = &animated_entity->entity;
    
    // apply some gravity
    if (entity->pos[2] > 0 || entity->vertical_speed > 0 || entity->vertical_speed < 0 ) {
        entity->vertical_speed -= 14;
        entity->pos[2] += time_data.frame_duration * entity->vertical_speed;
        if (entity->pos[2] < 0) {
            entity->vertical_speed = 0;
            entity->pos[2] = 0;
            set_entity_state(animated_entity, IDLE);
        }
    } 
    move_entity_one_frame(entity);
}

/*==============================
    handle_camera_c_buttons
    handles pitch, distance_from_entity 
    and angle_around_entity variables
==============================*/

void handle_camera_c_buttons(Camera *camera, NUContData cont[1]){

    if (cont[0].trigger & U_CBUTTONS && camera->distance_from_entity == 2000){
        camera->distance_from_entity = 1200;
        camera->pitch = 35;
    } else
    if (cont[0].trigger & U_CBUTTONS && camera->distance_from_entity == 1200){
        camera->distance_from_entity = 700;
        camera->pitch = 30;
    }

    if (cont[0].trigger & D_CBUTTONS && camera->distance_from_entity == 700){
        camera->distance_from_entity = 1200;
        camera->pitch = 35;
    } else
    if (cont[0].trigger & D_CBUTTONS && camera->distance_from_entity == 1200){
        camera->distance_from_entity = 2000;
        camera->pitch = 40;
    }

    if (cont[0].trigger & L_CBUTTONS){
        camera->angle_around_entity -= 15;
    }
   
    if (cont[0].trigger & R_CBUTTONS && camera->angle_around_entity == 0){
        camera->angle_around_entity = 360;
        camera->angle_around_entity += 15;
    }else
    if (cont[0].trigger & R_CBUTTONS){
        camera->angle_around_entity += 25;
    }

    if (camera->angle_around_entity == 360){
        camera->angle_around_entity = 0;
    }
}


/*==============================
    move_entity_c_buttons
    Moves entity with c buttons
==============================*/

void move_entity_c_buttons(Entity *entity, Camera camera, NUContData cont[1]){

    entity->forward_speed = lim(contdata[0].button & U_CBUTTONS) - lim(contdata[0].button & D_CBUTTONS);
    entity->side_speed = lim(contdata[0].button & L_CBUTTONS) - lim(contdata[0].button & R_CBUTTONS);

	if (entity->forward_speed != 0 || entity->side_speed != 0) {
    	entity->yaw = deg(atan2(-entity->side_speed, -entity->forward_speed) - rad(camera.angle_around_entity));
    }

    if (entity->forward_speed != 0 && entity->side_speed != 0){

        entity->forward_speed = entity->forward_speed * COS_45;
        entity->side_speed = entity->side_speed * COS_45;
    }

    float frame_distance_forward = time_data.frame_duration * entity->forward_speed * 500;
    float frame_distance_side = time_data.frame_duration * entity->side_speed * 500;


    entity->pos[0] += frame_distance_forward * sin(rad(camera.angle_around_entity));
    entity->pos[1] += frame_distance_forward * cos(rad(camera.angle_around_entity));

    entity->pos[0] += frame_distance_side * sin(rad(camera.angle_around_entity - 90));
    entity->pos[1] += frame_distance_side * cos(rad(camera.angle_around_entity - 90));
}


/*==============================
    handle_camera_analog_stick
    moves camera with analog stick
==============================*/

void handle_camera_analog_stick(Camera *camera, NUContData cont[1]){

    if (fabs(cont->stick_x) < 7){cont->stick_x = 0;}
    if (fabs(cont->stick_y) < 7){cont->stick_y = 0;}

    camera->angle_around_entity += cont->stick_x / 40;
    camera->pitch += cont->stick_y / 40;

    if (cam.angle_around_entity > 360) {cam.angle_around_entity  = 0;}
    if (cam.angle_around_entity < 0) {cam.angle_around_entity  = 360;}

    if (cam.angle_around_entity  > 360) {cam.angle_around_entity  = 0;}
    if (cam.angle_around_entity  < 0) {cam.angle_around_entity  = 360;}

    if (cam.pitch > 85) {cam.pitch = 85;}
    if (cam.pitch < -85) {cam.pitch = -85;}
}


/*==============================
    get_cam_position
    calculates camera coordinates
==============================*/

void get_cam_position(Camera *camera, Entity *entity){

    camera->horizontal_distance_from_entity = camera->distance_from_entity * cos(rad(camera->pitch));
	camera->vertical_distance_from_entity = camera->distance_from_entity * sin(rad(camera->pitch));

    camera->pos[0] = entity->pos[0] - camera->horizontal_distance_from_entity * sin(rad(camera->angle_around_entity));
    camera->pos[1] = entity->pos[1] - camera->horizontal_distance_from_entity * cos(rad(camera->angle_around_entity));
    camera->pos[2] = camera->vertical_distance_from_entity + 1/Q_rsqrt(entity->pos[2]);

    if ((camera->vertical_distance_from_entity + entity->pos[2]) < 5){cam.pos[2] = 5;}
}


/*==============================
    move_cam
    Controls camera movement
==============================*/

void move_cam(Camera *camera, Entity *entity, NUContData cont[1]){

    handle_camera_c_buttons(camera, cont);
    //handle_camera_analog_stick(camera, cont);
    get_cam_position(camera, entity);
}


/*==============================
    set_light
    Sets the lights 
==============================*/

void set_light(LightData *light){

    int i;
    
    // Setup the lights
    if (!uselight)
        light->ambcol = 255;
    for (i=0; i<3; i++){
        light->amb.l.col[i] = light->ambcol;
        light->amb.l.colc[i] = light->ambcol;
        light->dir.l.col[i] = 255;
        light->dir.l.colc[i] = 255;
    }
    // handle the light direction so it's always projecting from the camera's position
    if (!freezelight){
        light->dir.l.dir[0] = -127*sinf(light->angle[0]*0.0174532925);
        light->dir.l.dir[1] = 127*sinf(light->angle[2]*0.0174532925)*cosf(light->angle[0]*0.0174532925);
        light->dir.l.dir[2] = 127*cosf(light->angle[2]*0.0174532925)*cosf(light->angle[0]*0.0174532925);
    }
    // Send the light struct to the RSP
    gSPNumLights(glistp++, NUMLIGHTS_1);
    gSPLight(glistp++, &light->dir, 1);
    gSPLight(glistp++, &light->amb, 2);
    gDPPipeSync(glistp++);
}


/*==============================
    set_cam
    Sets the camera 
==============================*/

void set_cam(Camera *camera, Entity entity){

    // Setup the cam.projection matrix
    guPerspective(
    	&camera->projection, &camera->normal, 
        45, (float)SCREEN_WD / (float)SCREEN_HT, 
    	10.0, 10000.0, 0.01);
    
    guLookAt(
    	&camera->viewpoint,
    	camera->pos[0], camera->pos[1], camera->pos[2],
    	entity.pos[0], entity.pos[1], 1/Q_rsqrt(entity.pos[2]) + 120,
    	0, 0, 1
  	);

    // Apply the cam.projection matrix
    gSPMatrix(glistp++, &camera->projection, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &camera->viewpoint, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPPerspNormalize(glistp++, &camera->normal);

    // Initialize the model matrix
    guMtxIdent(&camera->modeling);
    gSPMatrix(glistp++, &camera->modeling, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
}

void update_animation_based_on_state(AnimatedEntity * animated_entity) {
    entity_state new_state = animated_entity->entity.state;
    s64ModelHelper* helper = &animated_entity->helper;
    if (animated_entity->entity.type == NICK) {
        if (new_state == JUMP) sausage64_set_anim(helper, ANIMATION_nick_jump);
        if (new_state == ROLL) sausage64_set_anim(helper, ANIMATION_nick_roll);
        if (new_state == FALL) sausage64_set_anim(helper, ANIMATION_nick_fall);
        if (new_state == MIDAIR) sausage64_set_anim(helper, ANIMATION_nick_midair);
        if (new_state == IDLE) sausage64_set_anim(helper, ANIMATION_nick_idle);
        if (new_state == WALK) sausage64_set_anim(helper, ANIMATION_nick_walk);
        if (new_state == RUN) sausage64_set_anim(helper, ANIMATION_nick_run);
    } else if (animated_entity->entity.type == WILLY) {
        // TODO - handle states that willy can't be in somewhere
        if (new_state == JUMP) sausage64_set_anim(helper, ANIMATION_willy_jump);
        //if (new_state == ROLL) sausage64_set_anim(helper, ANIMATION_willy_roll);
        if (new_state == IDLE) sausage64_set_anim(helper, ANIMATION_willy_idle);
        if (new_state == RUN) sausage64_set_anim(helper, ANIMATION_willy_run);
    }
}

void set_entity_state(AnimatedEntity * animated_entity, entity_state new_state) {

    Entity * entity = &animated_entity->entity;
    int curr_state = entity->state;

    if (curr_state == new_state) {
        return;
    }

    if (new_state == JUMP && 
            (    curr_state == IDLE 
              || curr_state == WALK 
              || curr_state == RUN)) {
        entity->state = new_state;
        update_animation_based_on_state(animated_entity);
        animated_entity->entity.vertical_speed = 600;
    }

    if (new_state == ROLL && 
            (    curr_state == IDLE 
              || curr_state == WALK 
              || curr_state == RUN )) {
        entity->state = new_state;
        update_animation_based_on_state(animated_entity);
        animated_entity->entity.speed = 800;
    }

    if (new_state == WALK && curr_state == IDLE) {
        entity->state = new_state;
        update_animation_based_on_state(animated_entity);
    }
    if (new_state == RUN && 
            ( curr_state == IDLE || curr_state == WALK)) {
        entity->state = new_state;
        update_animation_based_on_state(animated_entity);
        // TODO - just to make willy move, gets overriden by controller for user
        animated_entity->entity.speed = 800;
    }

    if (new_state == IDLE
        && (     curr_state == WALK 
              || curr_state == RUN 
              || curr_state == ROLL 
              || curr_state == FALL 
              || curr_state == JUMP 
            )
            ) {
        entity->state = new_state;
        update_animation_based_on_state(animated_entity);
        animated_entity->entity.speed = 0;
    }

    if (new_state == MIDAIR) {
        entity->state = new_state;
        update_animation_based_on_state(animated_entity);
    }

    if (new_state == FALL) {
        entity->state = new_state;
        update_animation_based_on_state(animated_entity);
    }
}

/*==============================
    animate_nick & animate_willy
    link entity animations to controller input
==============================*/

void handle_controller_input(NUContData cont[1], AnimatedEntity* entity){
    if (cont[0].trigger & R_TRIG) {
        candy.entity.pos[0] = entity->entity.pos[0];
        candy.entity.pos[1] = entity->entity.pos[1];
        candy.entity.pos[2] = entity->entity.pos[2] + 50;
        candy.entity.speed = 3000;
        candy.entity.yaw = entity->entity.yaw;
    }
    if (cont[0].trigger & A_BUTTON) set_entity_state(entity, JUMP);
    if (cont[0].trigger & B_BUTTON) set_entity_state(entity, ROLL);
    if (entity->entity.speed > 900) {
        set_entity_state(entity, RUN);
    } else if (cont->stick_x != 0 || cont->stick_y != 0) {
        set_entity_state(entity, WALK);
    }

    if (cont->stick_x == 0 && cont->stick_y == 0
        && entity->entity.state != JUMP 
        && entity->entity.state != ROLL
        && entity->entity.state != FALL 
        && entity->entity.state != MIDAIR 
        ) {
        set_entity_state(entity, IDLE);
    }

    //handle movement
    move_entity_analog_stick(&entity->entity, cam, contdata);
    //move_entity_c_buttons(entity.entity, cam, contdata);

    move_cam(&cam, &entity->entity, contdata);
}

void when_animation_completes(AnimatedEntity * animated_entity) {
    // Go to idle animation when we finished attacking
    switch(animated_entity->entity.state)
    {
        case JUMP:
            set_entity_state(animated_entity, MIDAIR);
            break;
        case MIDAIR:
            set_entity_state(animated_entity, FALL);
            break;
        case FALL:
            set_entity_state(animated_entity, IDLE);
            break;
        case ROLL:
            set_entity_state(animated_entity, IDLE);
            animated_entity->entity.speed = 0;
            break;
    }
}


/*==============================
    animcallback
    Called before an animation finishes
==============================*/

void nick_animcallback(u16 anim){
    // yes this currently ignores the passed in animation, might not be bad to verify we are finishing
    // the animation we are expecting, but this way the logic for where we go when transitioning out of
    // a given state can be shared across different animated entities, that can do some subset, of move/roll/run/etc.
    when_animation_completes(&nick);
}

void willy_animcallback(u16 anim)
{
    // Go to idle animation when we finished attacking
    switch(willy.entity.state)
    {
        case JUMP:
            set_entity_state(&willy, IDLE);
            break;
        case ROLL:
            set_entity_state(&willy, IDLE);
            willy.entity.speed = 0;
            break;
    }
}


/*==============================
    draw_animated_entity
    draws animated entities
==============================*/

void draw_animated_entity(AnimatedEntity *animated_entity){

    guTranslate(&animated_entity->entity.pos_mtx, animated_entity->entity.pos[0], animated_entity->entity.pos[1], animated_entity->entity.pos[2]);
    guRotate(&animated_entity->entity.rot_mtx[0], animated_entity->entity.pitch, 1, 0, 0);
    guRotate(&animated_entity->entity.rot_mtx[1], animated_entity->entity.yaw, 0, 0, 1);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&animated_entity->entity.pos_mtx), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&animated_entity->entity.rot_mtx[0]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&animated_entity->entity.rot_mtx[1]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    sausage64_drawmodel(&glistp, &animated_entity->helper);
}


/*==============================
    draw_static_entity
    draws static entities
==============================*/

void draw_static_entity(StaticEntity *static_entity){

    guTranslate(&static_entity->entity.pos_mtx, static_entity->entity.pos[0], static_entity->entity.pos[1], static_entity->entity.pos[2]);
    guRotate(&static_entity->entity.rot_mtx[0], 0, 1, 0, 0);
    guRotate(&static_entity->entity.rot_mtx[1], 0, 0, 0, 1);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&static_entity->entity.pos_mtx), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&static_entity->entity.rot_mtx[0]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&static_entity->entity.rot_mtx[1]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    
    gSPDisplayList(glistp++, static_entity->mesh);
}


/*==============================
    draw_world
    Draws entities 
==============================*/

void draw_world(AnimatedEntity *highlighted, Camera *camera, LightData *light){

    // Initialize the RCP to draw stuff nicely
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
    gDPSetDepthSource(glistp++, G_ZS_PIXEL);
    gSPClearGeometryMode(glistp++,0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_SHADE | G_ZBUFFER | G_CULL_BACK | G_SHADING_SMOOTH | G_LIGHTING);
    gSPTexture(glistp++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetRenderMode(glistp++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF);
    gDPSetCombineMode(glistp++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetTexturePersp(glistp++, G_TP_PERSP);
    gDPSetTextureFilter(glistp++, G_TF_BILERP);
    gDPSetTextureConvert(glistp++, G_TC_FILT);
    gDPSetTextureLOD(glistp++, G_TL_TILE);
    gDPSetTextureDetail(glistp++, G_TD_CLAMP);
    gDPSetTextureLUT(glistp++, G_TT_NONE);

    //set view matrix and lights
    set_cam(camera, highlighted->entity);

    set_light(light);

    //draw the entities
    draw_static_entity(&axis);
    draw_static_entity(&ground);
    draw_static_entity(&candy);
    draw_static_entity(&pumpkin);
    draw_static_entity(&gravestone);
    draw_static_entity(&gravestone_cross);

    draw_animated_entity(&nick);

    draw_animated_entity(&willy);


    // Syncronize the RCP and CPU and specify that our display list has ended
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);

    // Ensure the cache lines are valid
    osWritebackDCache(&camera->projection, sizeof(&camera->projection));
    osWritebackDCache(&camera->modeling, sizeof(camera->modeling));
}


/*==============================
    draw_debug_data
    Draws debug data
==============================*/

void draw_debug_data(){

    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 1);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "FPS %d", (int)time_data.FPS);

    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 2);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "nick anim state %d", curr_nick_state);
    
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 3);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "willy speed %d", (int)willy.entity.speed);
    /*
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 4);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "cur_frame %llu", time_data.cur_frame);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 5);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "horizontal distance %d", (int)cam.horizontal_distance_from_entity);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 6);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "vertical distance %d", (int)cam.vertical_distance_from_entity);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 7);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "cam x %d", (int)cam.pos[0]);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 8);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "cam y %d", (int)cam.pos[1]);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 9);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "cam z %d", (int)cam.pos[2]);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 10);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "time %d", (int)get_time());
    */
}


/*==============================
    stage00_init
    Initialize the stage
==============================*/

void stage00_init(void){

    // Initialize entities
    sausage64_initmodel(&nick.helper, MODEL_nick, nickMtx);
    sausage64_set_anim(&nick.helper, ANIMATION_nick_idle); 
    sausage64_set_animcallback(&nick.helper, nick_animcallback);

    sausage64_initmodel(&willy.helper, MODEL_willy, willyMtx);
    //sausage64_set_anim(&willy.helper, ANIMATION_willy_run); 
    sausage64_set_animcallback(&willy.helper, willy_animcallback);
    
    // Set nick's animation speed based on region
    #if TV_TYPE == PAL    
        animspeed = 0.66;
    #else
        animspeed = 0.5;
    #endif
}

/*==============================
    stage00_update
    Update stage variables every frame
==============================*/

void stage00_update(void){
    
    // Poll for USB commands
    debug_pollcommands();  

    //alculate fps
    time_management(&time_data);

    // Read the controller
    nuContDataGetEx(contdata, 0);

    //Handle animation
    handle_controller_input(contdata, &nick);

    move_animated_entity_one_frame(&nick);
    move_animated_entity_one_frame(&willy);
    move_entity_one_frame(&candy.entity);
   
    // Advacnce animations
    sausage64_advance_anim(&willy.helper, animspeed);
    
    sausage64_advance_anim(&nick.helper, animspeed);

    // make willy do different stuff    

    if (time_data.cur_frame % 1200 < 30) set_entity_state(&willy, RUN);
    else if (time_data.cur_frame % 1200 < 35) willy.entity.yaw += 3 * (time_data.cur_frame % 10);
    else if (time_data.cur_frame % 1200 < 40) willy.entity.yaw -= 3 * (time_data.cur_frame % 10);
    //if (time_data.cur_frame % 30 == 6) set_entity_state(&willy, ROLL);
    else if (time_data.cur_frame % 1200 < 42) set_entity_state(&willy, JUMP);
    else if (time_data.cur_frame % 1200 < 44) set_entity_state(&willy, IDLE);
}


/*==============================
    stage00_draw
    Draw the stage
==============================*/

void stage00_draw(void){
    
    // Assign our glist pointer to our glist array for ease of access
    glistp = glist;

    // Initialize the RCP and framebuffer
    rcp_init();
    fb_clear(16, 132, 132);

    draw_world(&nick, &cam, &light_data);    

    // Ensure we haven't gone over the display list size and start the graphics task
    debug_assert((glistp-glist) < GLIST_LENGTH);
    #if TV_TYPE != PAL
        nuGfxTaskStart(glist, (s32)(glistp - glist) * sizeof(Gfx), NU_GFX_UCODE_F3DEX, NU_SC_NOSWAPBUFFER);
    #else
        nuGfxTaskStart(glist, (s32)(glistp - glist) * sizeof(Gfx), NU_GFX_UCODE_F3DEX, NU_SC_SWAPBUFFER);
    #endif
    
    // Draw the menu (doesn't work on PAL)
    #if TV_TYPE != PAL
        nuDebConClear(NU_DEB_CON_WINDOW0);
        draw_debug_data();
        nuDebConDisp(NU_SC_SWAPBUFFER);
    #endif
}