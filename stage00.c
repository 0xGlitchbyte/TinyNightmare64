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
#include "axisMdl.h"
#include "debug.h"


/*********************************
              Macros
*********************************/

#define USB_BUFFER_SIZE 256
#define FRAMETIME_COUNT 30

/*********************************
        Function Prototypes
*********************************/

f32 get_fps();

void animate_nick(NUContData cont[1]);
void animate_willy(NUContData cont[1]);
void nick_animcallback(u16 anim);
void willy_animcallback(u16 anim);

void move_entity(Entity *entity, NUContData cont[1]);

float rad(float angle);
float deg(float rad);

void handle_zoom(Camera *camera, NUContData cont[1]);
void handle_angle_around_entity(Camera *camera, NUContData cont[1]);
void get_cam_position(Camera *camera, Entity entity);
void move_cam(Camera *camera, Entity entity, NUContData cont[1]);
void set_lights(Camera *camera);
void set_cam(Camera *camera, Entity entity);

void draw_animated_entity(AnimatedEntity *entity);
void draw_static_entity(StaticEntity *static_entity);
void draw_world(AnimatedEntity entity, Camera *camera);

void draw_debug_data();

/*********************************
             Globals
*********************************/

//Variables
OSTime frameTimes[FRAMETIME_COUNT];
u8 curFrameTimeIndex = 0;
f32 gFPS = 0;
float animspeed;

// Camera
Camera cam = {
    distance_from_entity : 700,
    pitch : 40,
    angle_around_entity : 0,
    //pos: {0, -800, 1000},
    camang: {0, 0, -90},
};


// Entities
AnimatedEntity nick = {
    entity: {
        pos: { 0, 0, 0},
    }
};

Mtx nickMtx[MESHCOUNT_nick];

AnimatedEntity willy = {
    entity: {
        pos: { 400, 400, 0},
    }
};

Mtx willyMtx[MESHCOUNT_willy];

StaticEntity axis = {
    entity: {
        pos: { 0, 0, 0},
    }
};


// USB
static char uselight = TRUE;
static char drawaxis = TRUE;
static char freezelight = FALSE;
static char usb_buffer[USB_BUFFER_SIZE];


/*==============================
    stage00_init
    Initialize the stage
==============================*/

void stage00_init(void)
{
    // Initialize entities
    sausage64_initmodel(&nick.helper, MODEL_nick, nickMtx);
    sausage64_set_anim(&nick.helper, ANIMATION_nick_idle); 
    sausage64_set_animcallback(&nick.helper, nick_animcallback);

    sausage64_initmodel(&willy.helper, MODEL_willy, willyMtx);
    sausage64_set_anim(&willy.helper, ANIMATION_willy_idle); 
    sausage64_set_animcallback(&willy.helper, willy_animcallback);
    
    // Set nick's animation speed based on region
    #if TV_TYPE == PAL
        animspeed = 0.66;
    #else
        animspeed = 0.5;
    #endif
}


/*==============================
    get_fps
    calculates fps count
==============================*/

// Call once per frame
f32 get_fps() {
    OSTime newTime = osGetTime();
    OSTime oldTime = frameTimes[curFrameTimeIndex];
    frameTimes[curFrameTimeIndex] = newTime;

    curFrameTimeIndex++;
    if (curFrameTimeIndex >= FRAMETIME_COUNT) {
        curFrameTimeIndex = 0;
    }
    gFPS = ((f32)FRAMETIME_COUNT * 1000000.0f) / (s32)OS_CYCLES_TO_USEC(newTime - oldTime);
    return gFPS;
}


/*==============================
    move_entity
    Moves entity with controller
==============================*/

void move_entity(Entity *entity, NUContData cont[1]){
	
	if (fabs(cont->stick_x) < 7){cont->stick_x = 0;}
	if (fabs(cont->stick_y) < 7){cont->stick_y = 0;}

	if ( cont->stick_x != 0 || cont->stick_y != 0) {
    	entity->yaw = deg(atan2(cont->stick_x, -cont->stick_y)); 
    }

    entity->pos[0] += cont->stick_x / 20;
    entity->pos[1] += cont->stick_y / 20;

}

   
/*==============================
    rad & deg
    convert angles to radians
==============================*/

float rad(float angle){
	float radian = M_PI / 180 * angle;
	return radian;
}

float deg(float rad){
	float angle = 180 / M_PI * rad;
	return angle;
}


 /* 
     Nintendo's official button names 
    U_JPAD
    L_JPAD
    R_JPAD
    D_JPAD
    START_BUTTON
    A_BUTTON
    B_BUTTON
    U_CBUTTONS
    L_CBUTTONS
    R_CBUTTONS
    D_CBUTTONS
    L_TRIG
    R_TRIG
    Z_TRIG
*/

/*==============================
    handle_zoom
    handles pitch and distance_from_entity variables
==============================*/

void handle_zoom(Camera *camera, NUContData cont[1]){

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
}


/*==============================
    handle_angle_around_entity
    handles angle_around_entity variable
==============================*/

void handle_angle_around_entity(Camera *camera, NUContData cont[1]){

    if (cont[0].trigger & L_CBUTTONS){
        camera->angle_around_entity += 45;
    }
   
    if (cont[0].trigger & R_CBUTTONS && camera->angle_around_entity == 0){
        camera->angle_around_entity = 360;
        camera->angle_around_entity -= 45;
    }else
    if (cont[0].trigger & R_CBUTTONS){
        camera->angle_around_entity -= 45;
    }

    if (camera->angle_around_entity == 360){
        camera->angle_around_entity = 0;
    }
}


/*==============================
    get_distances
    calculates vertical and horizontal
    distances from entity
==============================*/

void get_distances(Camera *camera){

    camera->horizontal_distance_from_entity = camera->distance_from_entity * cos(rad(camera->pitch));
	camera->vertical_distance_from_entity = camera->distance_from_entity * sin(rad(camera->pitch));
}


/*==============================
    get_cam_position
    calculates camera coordinates
==============================*/

void get_cam_position(Camera *camera, Entity entity){

    camera->pos[0] = entity.pos[0] - camera->horizontal_distance_from_entity * sin(rad(camera->angle_around_entity));
    camera->pos[1] = entity.pos[1] - camera->horizontal_distance_from_entity * cos(rad(camera->angle_around_entity));
    camera->pos[2] = camera->vertical_distance_from_entity + entity.pos[2];
}


/*==============================
    move_cam
    Controls camera movement
==============================*/

void move_cam(Camera *camera, Entity entity, NUContData cont[1]){

    handle_zoom(camera, cont);
    handle_angle_around_entity(camera, cont);
    get_distances(camera);
    get_cam_position(camera, entity);
}


/*==============================
    set_lights
    Sets the lights 
==============================*/

void set_lights(Camera *camera){

    static Light light_amb;
    static Light light_dir;
    int i, ambcol = 100;
    
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
    
    // Setup the lights
    if (!uselight)
        ambcol = 255;
    for (i=0; i<3; i++){
        light_amb.l.col[i] = ambcol;
        light_amb.l.colc[i] = ambcol;
        light_dir.l.col[i] = 255;
        light_dir.l.colc[i] = 255;
    }
    // handle the light direction so it's always projecting from the camera's position
    if (!freezelight){
        light_dir.l.dir[0] = -127*sinf(camera->camang[0]*0.0174532925);
        light_dir.l.dir[1] = 127*sinf(camera->camang[2]*0.0174532925)*cosf(cam.camang[0]*0.0174532925);
        light_dir.l.dir[2] = 127*cosf(camera->camang[2]*0.0174532925)*cosf(cam.camang[0]*0.0174532925);
    }
    // Send the light struct to the RSP
    gSPNumLights(glistp++, NUMLIGHTS_1);
    gSPLight(glistp++, &light_dir, 1);
    gSPLight(glistp++, &light_amb, 2);
    gDPPipeSync(glistp++);
}


/*==============================
    set_cam
    Sets the camera 
==============================*/

void set_cam(Camera *camera, Entity entity){

    int i, ambcol = 100;

    // Setup the cam.projection matrix
    guPerspective(
    	&camera->projection, &camera->normal, 
        45, (float)SCREEN_WD / (float)SCREEN_HT, 
    	10.0, 10000.0, 0.01);
    
    guLookAt(
    	&camera->viewpoint,
    	camera->pos[0], camera->pos[1], camera->pos[2],
    	entity.pos[0], entity.pos[1], entity.pos[2],
    	0, 0, 1
  	);

    // Apply the cam.projection matrix
    gSPMatrix(glistp++, &camera->projection, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &camera->viewpoint, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPPerspNormalize(glistp++, &camera->normal);

    // Initialize the model matrix
    guMtxIdent(&camera->modeling);
    gSPMatrix(glistp++, &camera->modeling, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    
    set_lights(camera);
}


/*==============================
    animate_nick & animate_willy
    link entity animations to controller input
==============================*/

void animate_nick(NUContData cont[1]){

    if (cont[0].trigger & A_BUTTON && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_roll && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_jumpUP){
        sausage64_set_anim(&nick.helper, ANIMATION_nick_jumpUP);
    }

    if (cont[0].trigger & B_BUTTON && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_roll && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_jumpUP){
        sausage64_set_anim(&nick.helper, ANIMATION_nick_roll);
    }
	
    if (((cont->stick_x != 0 || cont->stick_y != 0) && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_roll ) && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_run  && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_jumpUP){
    	sausage64_set_anim(&nick.helper, ANIMATION_nick_run); 
    }
    
    if (((cont->stick_x == 0 && cont->stick_y == 0) && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_roll ) && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_idle  && sausage64_get_currentanim(&nick.helper) != ANIMATION_nick_jumpUP) {
    	sausage64_set_anim(&nick.helper, ANIMATION_nick_idle);
    }
    
}


void animate_willy(NUContData cont[1]){

    if (cont[0].trigger & A_BUTTON && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_roll && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_jump){
        sausage64_set_anim(&willy.helper, ANIMATION_willy_jump);
    }

    if (cont[0].trigger & B_BUTTON && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_roll && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_jump){
        sausage64_set_anim(&willy.helper, ANIMATION_willy_roll);
    }
	
    if (((cont->stick_x != 0 || cont->stick_y != 0) && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_roll ) && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_run  && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_jump){
    	sausage64_set_anim(&willy.helper, ANIMATION_willy_run); 
    }
    
    if (((cont->stick_x == 0 && cont->stick_y == 0) && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_roll ) && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_idle  && sausage64_get_currentanim(&willy.helper) != ANIMATION_willy_jump) {
    	sausage64_set_anim(&willy.helper, ANIMATION_willy_idle);
    }
}


/*==============================
    animcallback
    Called before an animation finishes
==============================*/

void nick_animcallback(u16 anim)
{
    // Go to idle animation when we finished attacking
    switch(anim)
    {
        case ANIMATION_nick_roll:
        case ANIMATION_nick_jumpUP:
            sausage64_set_anim(&nick.helper, ANIMATION_nick_idle);
            break;
    }
}

void willy_animcallback(u16 anim)
{
    // Go to idle animation when we finished attacking
    switch(anim)
    {
        case ANIMATION_willy_roll:
        case ANIMATION_willy_jump:
            sausage64_set_anim(&willy.helper, ANIMATION_willy_idle);
            break;
    }
}


/*==============================
    draw_animated_entity
    draws animated entities
==============================*/

void draw_animated_entity(AnimatedEntity *animated_entity){

    Entity *entity = &animated_entity->entity;
    guTranslate(&entity->pos_mtx, entity->pos[0], entity->pos[1], entity->pos[2]);
    guRotate(&entity->rotx, entity->pitch, 1, 0, 0);
    guRotate(&entity->roty, entity->yaw, 0, 0, 1);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&entity->pos_mtx), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&entity->rotx), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&entity->roty), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    sausage64_drawmodel(&glistp, &animated_entity->helper);
}


/*==============================
    draw_static_entity
    draws static entities
==============================*/

void draw_static_entity(StaticEntity *static_entity){
    
    Entity *entity = &static_entity->entity;

    guTranslate(&entity->pos_mtx, entity->pos[0], entity->pos[1], entity->pos[2]);
    guRotate(&entity->rotx, 0, 1, 0, 0);
    guRotate(&entity->roty, 0, 0, 0, 1);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&entity->pos_mtx), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&entity->rotx), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&entity->roty), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    
    gSPDisplayList(glistp++, gfx_axis);
}


/*==============================
    draw_world
    Draws entities 
==============================*/

void draw_world(AnimatedEntity highlighted, Camera *camera){
  
    set_cam(camera, highlighted.entity);

    draw_static_entity(&axis);

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

void draw_debug_data()
{
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 1);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "cam distance %d", (int)cam.distance_from_entity);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 2);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "pitch %d", (int)cam.pitch);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 3);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "angle around player %d", (int)cam.angle_around_entity);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 4);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "horizontal distance %d", (int)cam.horizontal_distance_from_entity);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 5);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "vertical distance %d", (int)cam.vertical_distance_from_entity);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 6);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "cam x %d", (int)cam.pos[0]);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 7);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "cam y %d", (int)cam.pos[1]);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 8);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "cam z %d", (int)cam.pos[2]);

}


/*==============================
    stage00_update
    Update stage variables every frame
==============================*/

void stage00_update(void){
    
    // Poll for USB commands
    debug_pollcommands();  
   
    // Advacnce Willy's animation
    sausage64_advance_anim(&willy.helper, animspeed);
    
    // Advance nick's animation
    sausage64_advance_anim(&nick.helper, animspeed);

    // Read the controller
    nuContDataGetEx(contdata, 0);

    animate_nick(contdata);
    
    move_entity(&nick.entity, contdata);

    move_cam(&cam, nick.entity, contdata);
    
    //move_willy(&willy.entity, &cam, contdata);

}


/*==============================
    stage00_draw
    Draw the stage
==============================*/

void stage00_draw(void){

    //handles fps
    get_fps();
    
    // Assign our glist pointer to our glist array for ease of access
    glistp = glist;

    // Initialize the RCP and framebuffer
    rcp_init();
    fb_clear(128, 128, 32);

    draw_world(nick, &cam);    

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