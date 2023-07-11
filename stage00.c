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
#include "zombie.h"
#include "mummy.h"
#include "willy.h"
#include "ground_block.h"
#include "candy.h"
#include "pumpkin.h"
#include "gravestone.h"
#include "gravestone_cross.h"
#include "gravestone_flat.h"
#include "gravestone_flat_2.h"

#include "tree1clear.h"
#include "tree1grey.h"
#include "tree1dark.h"
#include "tree2clear.h"
#include "tree2grey.h"
#include "tree2dark.h"
#include "tree3.h"
#include "tree4.h"
#include "tree5.h"

#include "entrance.h"
#include "fence.h"

#include "shack.h"
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

void set_entity_state(AnimatedEntity * animated_entity, EntityState new_state);
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
    pitch: 20,
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
        type: NICK,
        health: 100,
        damage: 10,
        scale: 3,
        ammo: 10
    }
};

Mtx nickMtx[MESHCOUNT_nick];

AnimatedEntity zombie = {
    entity: {
        pos: { 400, 400, 0},
        yaw: 180,
        scale: 1,
        type: NICK 
    }
};

Mtx zombieMtx[MESHCOUNT_zombie];

AnimatedEntity mummy = {
    entity: {
        pos: { 400, 400, 0},
        yaw: 180,
        scale: 1,
        type: NICK 
    }
};

Mtx mummyMtx[MESHCOUNT_mummy];

AnimatedEntity willy = {
    entity: {
        pos: { 400, 400, 0},
        yaw: 180,
        scale: 1,
        type: WILLY
    }
};

Mtx willyMtx[MESHCOUNT_willy];

StaticEntity axis = {
    entity: {
        pos: { 0, 0, 0},
        scale: 1,
    },
    mesh: gfx_axis,
};

#define WIDTH_GROUND_SEGMENTS 5
#define HEIGHT_GROUND_SEGMENTS 5
#define GROUND_SEGMENTS_COUNT 25  // this should be the previous two multiplied together
StaticEntity ground_segments[GROUND_SEGMENTS_COUNT]= {};

StaticEntity candy = {
    entity: {
        pos: { -500, 500, 150},
        scale: 1,
    },
    mesh: gfx_candy,
};

#define SCENERY_COUNT 35
StaticEntity scenery[SCENERY_COUNT] = {
    {entity : {pos : {-300, 300, 30}, scale : 1}, mesh : gfx_pumpkin},
    {entity : {pos : {-300, 350, 30}, scale : 1}, mesh : gfx_pumpkin},
    {entity : {pos : {-240, 350, 30}, scale : 1}, mesh : gfx_pumpkin},
    {entity : {pos : {-120, 250, 30}, scale : 1}, mesh : gfx_pumpkin},
    {entity : {pos : {-300, 300, 30}, scale : 1}, mesh : gfx_pumpkin},
    {entity : {pos : {-300, 350, 30}, scale : 1}, mesh : gfx_pumpkin},
    {entity : {pos : {-240, 350, 30}, scale : 1}, mesh : gfx_pumpkin},
    {entity : {pos : {-120, 250, 30}, scale : 1}, mesh : gfx_pumpkin},
    {entity : {pos : {300, 300, 30}, scale : 1}, mesh : gfx_gravestone},
    {entity : {pos : {300, -300, 30}, scale : 1}, mesh : gfx_gravestone_cross},
    {entity : {pos : {300, -600, 30}, scale : 1}, mesh : gfx_gravestone_flat},
    {entity : {pos : {300, 500, 30}, scale : 1}, mesh : gfx_gravestone_flat_2},
    {entity : {pos : {-200, 400, 30}, scale : 15}, mesh : gfx_tree1clear},
    {entity : {pos : {-100, 800, 30}, scale : 15}, mesh : gfx_tree2clear},
    {entity : {pos : {0, -300, 30}, scale : 15}, mesh : gfx_tree3},
    {entity : {pos : {-600, 200, 30}, scale : 15}, mesh : gfx_tree4},
    {entity : {pos : {-1500, 900, 30}, scale : 15}, mesh : gfx_tree5},

    {entity : {pos : {600, -4325, 30}, scale : 2, yaw : 180}, mesh : gfx_fence},
    {entity : {pos : {-350, -4325, 30}, scale : 2, yaw : 0}, mesh : gfx_fence},
    {entity : {pos : {600, -3325, 30}, scale : 2, yaw : 270}, mesh : gfx_fence},
    {entity : {pos : {-350, -3325, 30}, scale : 2, yaw : 270}, mesh : gfx_fence},
    {entity : {pos : {1610, -3325, 30}, scale : 2, yaw : 180}, mesh : gfx_fence},
    {entity : {pos : {-1350, -3325, 30}, scale : 2, yaw : 0}, mesh : gfx_fence},
    {entity : {pos : {2200, -2500, 30}, scale : 2, yaw : 235}, mesh : gfx_fence},
    {entity : {pos : {-2150, -2500, 30}, scale : 2, yaw : 315}, mesh : gfx_fence},
    {entity : {pos : {2200, -1500, 30}, scale : 2, yaw : 270}, mesh : gfx_fence},
    {entity : {pos : {-2150, -1500, 30}, scale : 2, yaw : 270}, mesh : gfx_fence},
    {entity : {pos : {-2150, -500, 30}, scale : 2, yaw : 270}, mesh : gfx_fence},
    {entity : {pos : {2200, -500, 30}, scale : 2, yaw : 270}, mesh : gfx_fence},
    {entity : {pos : {2200, -500, 30}, scale : 2, yaw : 135}, mesh : gfx_fence},
    {entity : {pos : {-1450, 200, 30}, scale : 2, yaw : 225}, mesh : gfx_fence},
    {entity : {pos : {-450, 200, 30}, scale : 2, yaw : 180}, mesh : gfx_fence},
    {entity : {pos : {1500, 200, 30}, scale : 2, yaw : 180}, mesh : gfx_fence},
    {entity : {pos : {0, 200, 30}, scale : 2}, mesh : gfx_entrance},
    {entity : {pos : {2000, 2000, 0}, scale: 1}, mesh : gfx_shack},
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


void set_anim(s64ModelHelper* mdl, u16 anim) {
    sausage64_set_anim(mdl, anim);
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
        && curr_state != FALLBACK 
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

    int GRAVITY = 14;
    // apply some gravity
    if (entity->pos[2] > 0 || entity->vertical_speed > 0 || entity->vertical_speed < 0 ) {

        if (entity->type == WILLY && entity->state == FALLBACK) {
            // TODO hacky standin "animation" for willy getting hit, jump up and fall back fast
            //entity->vertical_speed -= 1200;
        } else {
            entity->vertical_speed -= GRAVITY;
        }
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
        camera->pitch = 20;
    } else
    if (cont[0].trigger & U_CBUTTONS && camera->distance_from_entity == 1200){
        camera->distance_from_entity = 700;
        camera->pitch = 20;
    }

    if (cont[0].trigger & D_CBUTTONS && camera->distance_from_entity == 700){
        camera->distance_from_entity = 1200;
        camera->pitch = 20;
    } else
    if (cont[0].trigger & D_CBUTTONS && camera->distance_from_entity == 1200){
        camera->distance_from_entity = 2000;
        camera->pitch = 20;
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
    EntityState new_state = animated_entity->entity.state;
    Entity* entity = &animated_entity->entity;
    s64ModelHelper* helper = &animated_entity->helper;
    if (animated_entity->entity.type == NICK) {
        /*
        if (entity->state  == CROUCH_IDLE) set_anim(&animated_entity->helper, ANIMATION_nick_crouch_idle_left);
        if (entity->state  == CROUCH_TO_RUN) set_anim(&animated_entity->helper, ANIMATION_nick_crouch_to_run_left);
        if (entity->state  == CROUCH_TO_STAND) set_anim(&animated_entity->helper, ANIMATION_nick_crouch_to_stand_left);
        if (entity->state  == FALL_IDLE) set_anim(&animated_entity->helper, ANIMATION_nick_fall_idle_left);
        if (entity->state  == FALL_TO_STAND) set_anim(&animated_entity->helper, ANIMATION_nick_fall_to_stand_left);
        if (entity->state  == JOG) set_anim(&animated_entity->helper, ANIMATION_nick_jog_left);
        if (entity->state  == JUMP) set_anim(&animated_entity->helper, ANIMATION_nick_jump_left);
        if (entity->state  == LOOK_AROUND) set_anim(&animated_entity->helper, ANIMATION_nick_look_around_left);
        if (entity->state  == RUN_ARC) set_anim(&animated_entity->helper, ANIMATION_nick_run_arc_left);
        if (entity->state  == RUN) set_anim(&animated_entity->helper, ANIMATION_nick_run_arc_left);
        if (entity->state  == RUN_TO_ROLL) set_anim(&animated_entity->helper, ANIMATION_nick_run_to_roll_right);
        if (entity->state  == RUN_TO_STAND) set_anim(&animated_entity->helper, ANIMATION_nick_run_to_stand_left);
        if (entity->state  == SPRINT) set_anim(&animated_entity->helper, ANIMATION_nick_sprint_left);
        if (entity->state  == STAND_IDLE) set_anim(&animated_entity->helper, ANIMATION_nick_stand_idle_left);
        if (entity->state  == STAND_TO_CROUCH) set_anim(&animated_entity->helper, ANIMATION_nick_stand_to_crouch_left);
        if (entity->state  == STAND_TO_JUMP) set_anim(&animated_entity->helper, ANIMATION_nick_stand_to_jump_left);
        if (entity->state  == STAND_TO_ROLL) set_anim(&animated_entity->helper, ANIMATION_nick_stand_to_roll_left);
        if (entity->state  == STAND_TO_RUN) set_anim(&animated_entity->helper, ANIMATION_nick_stand_to_run_left);
        if (entity->state  == TAP_SHOE) set_anim(&animated_entity->helper, ANIMATION_nick_tap_shoe_left);
        if (entity->state  == WALK) set_anim(&animated_entity->helper, ANIMATION_nick_walk_left);
        */
    } else if (animated_entity->entity.type == WILLY) {
        // TODO - handle states that willy can't be in somewhere
        /*
        if (new_state == JUMP) sausage64_set_anim(helper, ANIMATION_willy_jump);
        if (new_state == ROLL) sausage64_set_anim(helper, ANIMATION_willy_spinattack);
        if (new_state == FALLBACK) sausage64_set_anim(helper, ANIMATION_willy_fall_ahead);
        if (new_state == IDLE) sausage64_set_anim(helper, ANIMATION_willy_idle);
        if (new_state == RUN) sausage64_set_anim(helper, ANIMATION_willy_run);
        */
    }
}

void set_entity_state(AnimatedEntity * animated_entity, EntityState new_state) {

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
        set_anim(&animated_entity->helper, ANIMATION_nick_jump_left);
        animated_entity->entity.vertical_speed = 600;
    }

    if (new_state == ROLL && 
            (    curr_state == IDLE 
              || curr_state == WALK 
              || curr_state == RUN )) {
        entity->state = new_state;
        set_anim(&animated_entity->helper, ANIMATION_nick_stand_to_roll_left);
        animated_entity->entity.speed = 800;
    }

    if (new_state == WALK && curr_state == IDLE) {
        entity->state = new_state;
        // TODO - just to make the zombie move, gets overriden by controller for user
        animated_entity->entity.speed = 400;
        set_anim(&animated_entity->helper, ANIMATION_nick_walk_left);
    }
    if (new_state == RUN && 
            ( curr_state == IDLE || curr_state == WALK)) {
        entity->state = new_state;
        set_anim(&animated_entity->helper, ANIMATION_nick_run_left);
        // TODO - just to make willy move, gets overriden by controller for user
        animated_entity->entity.speed = 600;
    }

    if (new_state == IDLE
        && (     curr_state == WALK 
              || curr_state == RUN 
              || curr_state == ROLL 
              || curr_state == FALL 
              || curr_state == FALLBACK
              || curr_state == JUMP 
            )
            ) {
        entity->state = new_state;
        set_anim(&animated_entity->helper, ANIMATION_nick_stand_idle_left);
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

    if (new_state == FALLBACK) {
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
        && entity->entity.state != FALLBACK
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
        case FALLBACK:
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

void zombie_animcallback(u16 anim){
    // yes this currently ignores the passed in animation, might not be bad to verify we are finishing
    // the animation we are expecting, but this way the logic for where we go when transitioning out of
    // a given state can be shared across different animated entities, that can do some subset, of move/roll/run/etc.
    when_animation_completes(&zombie);
}

void mummy_animcallback(u16 anim){
    // yes this currently ignores the passed in animation, might not be bad to verify we are finishing
    // the animation we are expecting, but this way the logic for where we go when transitioning out of
    // a given state can be shared across different animated entities, that can do some subset, of move/roll/run/etc.
    when_animation_completes(&mummy);
}

void willy_animcallback(u16 anim)
{
    // Go to idle animation when we finished attacking
    switch(willy.entity.state)
    {
        case JUMP:
            set_entity_state(&willy, IDLE);
            break;
        case FALLBACK:
            set_entity_state(&willy, IDLE);
            willy.entity.speed = 0;
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
    float scale = animated_entity->entity.scale;
    guScale(&animated_entity->entity.scale_mtx, scale, scale, scale);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&animated_entity->entity.pos_mtx), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&animated_entity->entity.rot_mtx[0]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&animated_entity->entity.rot_mtx[1]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&animated_entity->entity.scale_mtx), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    sausage64_drawmodel(&glistp, &animated_entity->helper);

    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
}

/*==============================
    draw_static_entity
    draws static entities
==============================*/

void draw_static_entity(StaticEntity *static_entity){

    guTranslate(&static_entity->entity.pos_mtx, static_entity->entity.pos[0], static_entity->entity.pos[1], static_entity->entity.pos[2]);
    guRotate(&static_entity->entity.rot_mtx[0], static_entity->entity.pitch, 1, 0, 0);
    guRotate(&static_entity->entity.rot_mtx[1], static_entity->entity.yaw, 0, 0, 1);
    float scale = static_entity->entity.scale;
    guScale(&static_entity->entity.scale_mtx, scale, scale, scale);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&static_entity->entity.pos_mtx), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&static_entity->entity.rot_mtx[0]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&static_entity->entity.rot_mtx[1]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&static_entity->entity.scale_mtx), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    
    gSPDisplayList(glistp++, static_entity->mesh);

    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
}

void set_pt(float* dest, float* src) {
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
}

int get_static_entity_width(StaticEntity* static_entity) {
    if (static_entity->mesh == gfx_shack) return 800;
}

int get_static_entity_depth(StaticEntity* static_entity) {
    if (static_entity->mesh == gfx_shack) return 700;
}

float dot(float *u, float *v) {
    return u[0] * v[0] + u[1] * v[1]; 
}

void vector(float *dest, float *p1, float *p2) {
    dest[0] = p2[0] - p1[0];
    dest[1] = p2[1] - p1[1];
}

// https://stackoverflow.com/questions/2752725/finding-whether-a-point-lies-inside-a-rectangle-or-not/37865332#37865332
// currently only looks at x and y, despite passing in 3d points
int pt_in_rect(float* pos1, Entity *entity, int width, int depth) {

    float A[3]; float B[3]; float C[3]; float D[3];

    float AB[3];float AM[3];float BC[3];float BM[3];

    A[0] = entity->pos[0] + width / 2;
    A[1] = entity->pos[1] + depth / 2;
    
    B[0] = entity->pos[0] - width / 2;
    B[1] = entity->pos[1] + depth / 2;
    
    C[0] = entity->pos[0] - width / 2;
    C[1] = entity->pos[1] - depth / 2;
    
    D[0] = entity->pos[0] + width / 2;
    D[1] = entity->pos[1] - depth / 2;

    vector(AB, A, B);
    vector(AM, A, pos1);
    vector(BC, B, C);
    vector(BM, B, pos1);

    float dotABAM = dot(AB, AM);
    float dotABAB = dot(AB, AB);
    float dotBCBM = dot(BC, BM);
    float dotBCBC = dot(BC, BC);

    return 0 <= dotABAM && dotABAM <= dotABAB && 0 <= dotBCBM && dotBCBM <= dotBCBC;

    // javascript for reference from the link above
    /*
        function pointInRectangle(m, r) {
            var AB = vector(r.A, r.B);
            var AM = vector(r.A, m);
            var BC = vector(r.B, r.C);
            var BM = vector(r.B, m);
            var dotABAM = dot(AB, AM);
            var dotABAB = dot(AB, AB);
            var dotBCBM = dot(BC, BM);
            var dotBCBC = dot(BC, BC);
            return 0 <= dotABAM && dotABAM <= dotABAB && 0 <= dotBCBM && dotBCBM <= dotBCBC;
        }

        function vector(p1, p2) {
            return {
                    x: (p2.x - p1.x),
                    y: (p2.y - p1.y)
            };
        }

        function dot(u, v) {
            return u.x * v.x + u.y * v.y; 
        }

        var r = {
            A: {x: 50, y: 0},
            B: {x: 0, y: 20},
            C: {x: 10, y: 50},
            D: {x: 60, y: 30}
        };

        var m = {x: 40, y: 20};

        pointInRectangle(m, r); // returns true.
    */
}

// show the bounding rectangle for an object (that is not rotated)
void debug_entity_collision_rect(StaticEntity* static_entity) {
    scenery[0].entity.pos[0] =  static_entity->entity.pos[0] + get_static_entity_width(static_entity) / 2;
    scenery[0].entity.pos[1] =  static_entity->entity.pos[1] + get_static_entity_depth(static_entity) / 2;
    scenery[0].entity.pos[2] =  static_entity->entity.pos[2];

    scenery[1].entity.pos[0] =  static_entity->entity.pos[0] - get_static_entity_width(static_entity) / 2;
    scenery[1].entity.pos[1] =  static_entity->entity.pos[1] + get_static_entity_depth(static_entity) / 2;
    scenery[1].entity.pos[2] =  static_entity->entity.pos[2];

    scenery[2].entity.pos[0] =  static_entity->entity.pos[0] - get_static_entity_width(static_entity) / 2;
    scenery[2].entity.pos[1] =  static_entity->entity.pos[1] - get_static_entity_depth(static_entity) / 2;
    scenery[2].entity.pos[2] =  static_entity->entity.pos[2];

    scenery[3].entity.pos[0] =  static_entity->entity.pos[0] + get_static_entity_width(static_entity) / 2;
    scenery[3].entity.pos[1] =  static_entity->entity.pos[1] - get_static_entity_depth(static_entity) / 2;
    scenery[3].entity.pos[2] =  static_entity->entity.pos[2];
}

float distance(float* pos1, float* pos2) {
    return 1 / Q_rsqrt( 
          (pos1[0] - pos2[0]) * (pos1[0] - pos2[0])
        + (pos1[1] - pos2[1]) * (pos1[1] - pos2[1])
        + (pos1[2] - pos2[2]) * (pos1[2] - pos2[2])
    );
}

// length of a line segment in x and y directions
// this is only 2D for now, but takes 3d pts
float length_squared(float *pos1, float *pos2) {
    return
          (pos1[0] - pos2[0]) * (pos1[0] - pos2[0])
        + (pos1[1] - pos2[1]) * (pos1[1] - pos2[1]);
}

float distance_2d(float* pos1, float* pos2) {
    return 1 / Q_rsqrt( 
          (pos1[0] - pos2[0]) * (pos1[0] - pos2[0])
        + (pos1[1] - pos2[1]) * (pos1[1] - pos2[1])
    );
}

float max(float a, float b) {
    if (a > b) return a;
    else return b;
}

float min(float a, float b) {
    if (a < b) return a;
    else return b;
}

float minimum_distance(float *v, float *w, float *p) {
  // Return minimum distance between line segment vw and point p
  const float l2 = length_squared(v, w);  // i.e. |w-v|^2 -  avoid a sqrt
  if (l2 == 0.0) return distance_2d(p, v);   // v == w case
  // Consider the line extending the segment, parameterized as v + t (w - v).
  // We find projection of point p onto the line. 
  // It falls where t = [(p-v) . (w-v)] / |w-v|^2
  // We clamp t from [0,1] to handle points outside the segment vw.
  float p_v[3] = {};
  p_v[0] = p[0] - v[0];
  p_v[1] = p[1] - v[1];
  float w_v[3] = {};
  w_v[0] = w[0] - v[0];
  w_v[1] = w[1] - v[1];
  const float t = max(0, min(1, dot(p_v, w_v) / l2));
  float projection[3];
  projection[0] = v[0] + t * (w[0] - v[0]);
  projection[1] = v[1] + t * (w[1] - v[1]);
  //const vec2 projection = v + t * (w - v);  // Projection falls on the segment
  return distance_2d(p, projection);
}

float min_dist_to_wall_old;
float min_dist_to_wall_new;

void detect_collisions() {
    StaticEntity *shack = &scenery[SCENERY_COUNT - 1];
    if ( pt_in_rect(nick.entity.pos, &scenery[SCENERY_COUNT - 1].entity, get_static_entity_width(shack), get_static_entity_depth(shack))) {
        // just do the oposite of what we did to move this frame to get out of the wall
        Entity *entity = &nick.entity;
        // calculate if the next frame will move us out of the box
        float new_pos[3] = {};
        float frame_distance = time_data.frame_duration * entity->speed;
        new_pos[0] = entity->pos[0] + frame_distance * sin(rad(entity->yaw));
        new_pos[1] = entity->pos[1] - frame_distance * cos(rad(entity->yaw));


        float A[3]; float B[3]; float C[3]; float D[3];

        int width = get_static_entity_width(shack);
        int depth = get_static_entity_depth(shack);

        A[0] = entity->pos[0] + width / 2;
        A[1] = entity->pos[1] + depth / 2;
        
        B[0] = entity->pos[0] - width / 2;
        B[1] = entity->pos[1] + depth / 2;
        
        C[0] = entity->pos[0] - width / 2;
        C[1] = entity->pos[1] - depth / 2;
        
        D[0] = entity->pos[0] + width / 2;
        D[1] = entity->pos[1] - depth / 2;

        min_dist_to_wall_old = 
        min(
            minimum_distance(A, D, nick.entity.pos),
            min(
                minimum_distance(C, D, nick.entity.pos),
                min(
                    minimum_distance(A, B, nick.entity.pos), 
                    minimum_distance(B, C, nick.entity.pos)
                )));

        min_dist_to_wall_new = 
        min(
            minimum_distance(A, D, new_pos),
            min(
                minimum_distance(C, D, new_pos),
                min(
                    minimum_distance(A, B, new_pos), 
                    minimum_distance(B, C, new_pos)
                )));

        if (pt_in_rect(new_pos, &scenery[SCENERY_COUNT - 1].entity, get_static_entity_width(shack), get_static_entity_depth(shack))
        ) {
            if (min_dist_to_wall_new != min_dist_to_wall_old) {
                nick.entity.speed = 0;
            }
        }
    }
    if ( distance(nick.entity.pos, willy.entity.pos) < 150) {
        nick.entity.speed = -800;
        set_entity_state(&nick, FALLBACK);
    }

    if ( distance(candy.entity.pos, willy.entity.pos) < 150) {
        //willy.entity.vertical_speed = 4000;
        willy.entity.speed = 800;
        set_entity_state(&willy, FALLBACK);
    }

    if ( distance(candy.entity.pos, zombie.entity.pos) < 100) {
        //willy.entity.vertical_speed = 4000;
        zombie.entity.speed = -300;
        set_entity_state(&zombie, FALLBACK);
    }

    if ( distance(candy.entity.pos, mummy.entity.pos) < 100) {
        //willy.entity.vertical_speed = 4000;
        mummy.entity.speed = -300;
        set_entity_state(&mummy, FALLBACK);
    }
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
    set_pt(axis.entity.pos, willy.entity.pos);
    //draw_static_entity(&axis);

    for (int i = 0; i < GROUND_SEGMENTS_COUNT; i++) {
        draw_static_entity(&ground_segments[i]);
    }

    draw_static_entity(&candy);

    debug_entity_collision_rect(&scenery[SCENERY_COUNT - 1]);

    for (int i = 0; i < SCENERY_COUNT; i++) {
        draw_static_entity(&scenery[i]);
    }


    draw_animated_entity(&nick);

    draw_animated_entity(&willy);

    draw_animated_entity(&zombie);

    draw_animated_entity(&mummy);

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


int angle_to_player;

void draw_debug_data(){

    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 1);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "FPS %d", (int)time_data.FPS);

    //int angle_to_player = - deg(atan2(-(nick.entity.pos[1] - willy.entity.pos[1] ), (nick.entity.pos[0] - willy.entity.pos[0]))) + 90;
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 2);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "angle to player %d", angle_to_player);

    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 3);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "willy yaw %d", (int) (willy.entity.yaw));

    /*
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 2);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "willy yaw %d", (int) willy.entity.yaw);
    
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 3);
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
    sausage64_set_anim(&nick.helper, ANIMATION_nick_stand_idle_left); 
    sausage64_set_animcallback(&nick.helper, nick_animcallback);

    sausage64_initmodel(&willy.helper, MODEL_willy, willyMtx);
    //sausage64_set_anim(&willy.helper, ANIMATION_willy_run); 
    sausage64_set_animcallback(&willy.helper, willy_animcallback);

    sausage64_initmodel(&zombie.helper, MODEL_zombie, zombieMtx);
    sausage64_set_animcallback(&zombie.helper, zombie_animcallback);

    sausage64_initmodel(&mummy.helper, MODEL_mummy, mummyMtx);
    sausage64_set_animcallback(&mummy.helper, mummy_animcallback);

    // the side length of one panel of ground
    int ground_size = 5000;
    // these are declared about with the ground_segments array
    // it is the size of the grid of ground tiles we are creating
    // WIDTH_GROUND_SEGMENTS, HEIGHT_GROUND_SEGMENTS 
    // setup the ground
    for (int i = 0; i < WIDTH_GROUND_SEGMENTS; i++) {
        for (int j = 0; j < HEIGHT_GROUND_SEGMENTS; j++) {
            ground_segments[i * WIDTH_GROUND_SEGMENTS + j].entity.pos[0] =  i * ground_size - (WIDTH_GROUND_SEGMENTS / 2) * ground_size;
            ground_segments[i * WIDTH_GROUND_SEGMENTS + j].entity.pos[1] =  j * ground_size - (HEIGHT_GROUND_SEGMENTS / 2) * ground_size;
            ground_segments[i * WIDTH_GROUND_SEGMENTS + j].entity.pos[2] = 800;
            ground_segments[i * WIDTH_GROUND_SEGMENTS + j].mesh = gfx_ground;
            ground_segments[i * WIDTH_GROUND_SEGMENTS + j].entity.scale = 10;
        }
    }

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

    detect_collisions();

    move_animated_entity_one_frame(&nick);
    move_animated_entity_one_frame(&willy);
    move_animated_entity_one_frame(&zombie);
    move_animated_entity_one_frame(&mummy);
    move_entity_one_frame(&candy.entity);
   
    // Advacnce animations
    sausage64_advance_anim(&willy.helper, animspeed);
    
    sausage64_advance_anim(&nick.helper, animspeed);

    sausage64_advance_anim(&zombie.helper, animspeed);

    sausage64_advance_anim(&mummy.helper, animspeed);


    // make willy do different stuff    

    // when close make him chase nick
    if ( distance(nick.entity.pos, willy.entity.pos) < 4000
            && willy.entity.state != FALLBACK) {

        set_entity_state(&willy, RUN);

        angle_to_player = - deg(atan2(-(nick.entity.pos[1] - willy.entity.pos[1] ), (nick.entity.pos[0] - willy.entity.pos[0]))) + 90;
        float angle_diff = willy.entity.yaw - angle_to_player;
        if (fabs(angle_diff) < 180) willy.entity.yaw -= angle_diff / 20;
        else {
            // something close to the max turning speed with the division by 20 of the angle difference above
            willy.entity.yaw += angle_diff > 0 ? 1.5 : -1.5; 
            if (willy.entity.yaw > 270) willy.entity.yaw = -90 + fmod(willy.entity.yaw, 270.0);
            else if (willy.entity.yaw < -90) willy.entity.yaw = 270 + fmod(willy.entity.yaw, 90.0);
        }

        if ( distance(nick.entity.pos, willy.entity.pos) < 500) {
            if (time_data.cur_frame % 1200 < 5) set_entity_state(&willy, ROLL);
        }
    } else {
        // make willy change direction randomly
        if (time_data.cur_frame % 1200 < 30) set_entity_state(&willy, RUN);
        else if (time_data.cur_frame % 1200 < 31) set_entity_state(&willy, IDLE);
        //else if (time_data.cur_frame % 1200 < 42) set_entity_state(&willy, JUMP);

        if (time_data.cur_frame % 1200 < 35) willy.entity.yaw += 3 * (time_data.cur_frame % 10);
        else if (time_data.cur_frame % 1200 < 40) willy.entity.yaw -= 3 * (time_data.cur_frame % 10);

    }


    // make zombie do different stuff    

    if (time_data.cur_frame % 1356 < 30) set_entity_state(&zombie, WALK);
    else if (time_data.cur_frame % 1356 < 35) zombie.entity.yaw += 3 * (time_data.cur_frame % 10);
    else if (time_data.cur_frame % 1356 < 40) zombie.entity.yaw -= 3 * (time_data.cur_frame % 10);
    //if (time_data.cur_frame % 30 == 6) set_entity_state(&willy, ROLL);
    else if (time_data.cur_frame % 1356 < 42) set_entity_state(&zombie, JUMP);
    else if (time_data.cur_frame % 1356 < 44) set_entity_state(&zombie, IDLE);

    // make mummy do different stuff    

    if (time_data.cur_frame % 9821 < 30) set_entity_state(&mummy, WALK);
    else if (time_data.cur_frame % 9821 < 35) mummy.entity.yaw += 3 * (time_data.cur_frame % 10);
    else if (time_data.cur_frame % 9821 < 40) mummy.entity.yaw -= 3 * (time_data.cur_frame % 10);
    //if (time_data.cur_frame % 30 == 6) set_entity_state(&willy, ROLL);
    else if (time_data.cur_frame % 9821 < 42) set_entity_state(&mummy, JUMP);
    else if (time_data.cur_frame % 9821 < 44) set_entity_state(&mummy, IDLE);
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
    fb_clear(16, 32, 32);

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
