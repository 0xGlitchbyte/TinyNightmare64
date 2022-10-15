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

f32 calculate_fps();
void draw_debug_data();
void nick_animcallback(u16 anim);
void willy_animcallback(u16 anim);
void move_entity(Entity *entity, Camera *camera, NUContData cont[1]);
void set_lights(Camera *camera);
void set_cam(Camera *camera, Entity *entity);
void draw_world(AnimatedEntity *entity, Camera *camera);
void draw_animated_entity(AnimatedEntity *entity);
void draw_static_entity(StaticEntity *static_entity);


/*********************************
             Globals
*********************************/

//Variables
OSTime frameTimes[FRAMETIME_COUNT];
u8 curFrameTimeIndex = 0;
f32 gFPS = 0;

// Camera
Camera cam = {
    pos: {0, -800, 500},
    camang: {0, 0, -90},
};


// Entities
AnimatedEntity nick = {
    entity: {
        pos: { -100, -100, 0},
    }
};

Mtx nickMtx[MESHCOUNT_nick];

AnimatedEntity willy = {
    entity: {
        pos: { 200, 200, 0},
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
        nick.animspeed = 0.66;
        willy.animspeed = 0.66;
    #else
        nick.animspeed = 0.5;
        willy.animspeed = 0.5;
    #endif
}


/*==============================
    move_entity
    Moves entity with controller
==============================*/

void move_entity(Entity *entity, Camera *camera, NUContData cont[1]){
	
	if (fabs(cont->stick_x) < 7){cont->stick_x = 0;}
	if (fabs(cont->stick_y) < 7){cont->stick_y = 0;}

	 if ( cont->stick_x != 0 || cont->stick_y != 0) {
    	nick.entity.yaw = atan2(contdata->stick_x, -contdata->stick_y) * (180 / M_PI); 
    }

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
    
    entity->pos[1] += contdata->stick_y / 20;
    entity->pos[0] += contdata->stick_x / 20;
    
    camera->pos[1] += contdata->stick_y / 20;
    camera->pos[0] += contdata->stick_x / 20;
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
    // Calculate the light direction so it's always projecting from the camera's position
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

void set_cam(Camera *camera, Entity *entity){

       int i, ambcol = 100;

    // Setup the cam.projection matrix
    guPerspective(
    	&camera->projection, &camera->normal, 
        45, (float)SCREEN_WD / (float)SCREEN_HT, 
    	10.0, 10000.0, 0.01);
    
    guLookAt(
    	&camera->viewpoint,
    	camera->pos[0], camera->pos[1], camera->pos[2],
    	entity->pos[0], entity->pos[1], entity->pos[2],
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

void draw_world(AnimatedEntity *highlighted_entity, Camera *camera){
  
    set_cam(camera, &highlighted_entity->entity);

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
    stage00_update
    Update stage variables every frame
==============================*/

void stage00_update(void){
    
    // Poll for USB commands
    debug_pollcommands();  
    
    // Advance nick's animation
    sausage64_advance_anim(&nick.helper, nick.animspeed);

    // Advacnce Willy's animation
    sausage64_advance_anim(&willy.helper, willy.animspeed);

    // Read the controller
    nuContDataGetEx(contdata, 0);
    
    move_entity(&nick.entity, &cam, contdata);     
}


/*==============================
    stage00_draw
    Draw the stage
==============================*/

void stage00_draw(void){

    //calculates fps
    calculate_fps();
    
    // Assign our glist pointer to our glist array for ease of access
    glistp = glist;

    // Initialize the RCP and framebuffer
    rcp_init();
    fb_clear(128, 128, 32);

    draw_world(&nick, &cam);    

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


/*==============================
    calculate_fps
    Calculates and updates fps
==============================*/

// Call once per frame
f32 calculate_fps() {
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
    draw_debug_data
    Draws debug data
==============================*/

void draw_debug_data()
{
    int FPS = (int)gFPS;
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 1);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "FPS = %d", FPS);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 2);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "stick x: %d", contdata->stick_x);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 1, 3);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "stick y: %d", contdata->stick_y);
}



/*********************************
     Model callback functions
*********************************/


/*==============================
    animcallback
    Called before an animation finishes
    @param The animation that is finishing
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

/*********************************
      USB Command Functions
*********************************/

/*==============================
    command_listanims
    USB Command for listing animations
==============================*/

char* command_listanims()
{
    int i;
    memset(usb_buffer, 0, USB_BUFFER_SIZE);

    // Go through all the animations names and append them to the string
    for (i=0; i<ANIMATIONCOUNT_nick; i++)
        sprintf(usb_buffer, "%s%s\n", usb_buffer, MODEL_nick->anims[i].name);

        // Return the string of animation names
    return usb_buffer;
}


/*==============================
    command_setanim
    USB Command for setting animations
==============================*/

char* command_setanim()
{
    int i;
    memset(usb_buffer, 0, USB_BUFFER_SIZE);
    
    // Check the animation name isn't too big
    if (debug_sizecommand() > USB_BUFFER_SIZE)
        return "Name larger than USB buffer";
    debug_parsecommand(usb_buffer);
    
    // Compare the animation names
    for (i=0; i<ANIMATIONCOUNT_nick; i++)
    {
        if (!strcmp(MODEL_nick->anims[i].name, usb_buffer))
        {
            sausage64_set_anim(&nick.helper, i);
            return "Animation set.";
        }
    }

    // No animation found
    return "Unkown animation name";
}


/*==============================
    command_togglelight
    USB Command for toggling lighting
==============================*/

char* command_togglelight()
{
    uselight = !uselight;
    return "Light Toggled";
}


/*==============================
    command_freezelight
    USB Command for freezing lighting
==============================*/

char* command_freezelight()
{
    freezelight = !freezelight;
    return "Light state altered";
}


/*==============================
    command_togglelerp
    USB Command for toggling lerp
==============================*/

char* command_togglelerp()
{
    nick.helper.interpolate = !nick.helper.interpolate;
    return "Interpolation Toggled";
}


/*==============================
    command_toggleloop
    USB Command for toggling animation looping
==============================*/

char* command_toggleloop()
{
    nick.helper.loop = !nick.helper.loop;
    return "Loop Toggled";
}


/*==============================
    command_toggleaxis
    USB Command for toggling the floor axis
==============================*/

char* command_toggleaxis()
{
    drawaxis = !drawaxis;
    return "Axis Toggled";
}
