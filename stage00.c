/***************************************************************
                           stage00.c
                
  first acomplished attempt to render and move nick around
             and make camera follow him!
***************************************************************/

#include <math.h>
#include <nusys.h>
#include <string.h> // Needed for CrashSDK compatibility
#include "config.h"
#include "helper.h"
#include "sausage64.h"
#include "axisMdl.h"
#include "palette.h"
#include "nick.h"
#include "debug.h"


/*********************************
              Macros
*********************************/

#define USB_BUFFER_SIZE 256

/*********************************
        Function Prototypes
*********************************/

void draw_debug_data();
void nick_predraw(u16 part);
void nick_animcallback(u16 anim);

void matrix_inverse(float mat[4][4], float dest[4][4]);


/*********************************
             Globals
*********************************/

// Matricies and vectors
static Mtx projection, viewing, modeling;
static u16 normal;
// Lights
static Light light_amb;
static Light light_dir;

// Menu
static char menuopen = FALSE;
static s8   curx = 0;
static s8   cury = 0;

// Camera
static float campos[3] = {0, -100, -600};
static float camang[3] = {0, 0, -90};

// nick
Mtx nickMtx[MESHCOUNT_nick];

s64ModelHelper nick = {
    pos: { 20, 1, 0},
    dir: { -1, 0, 0},
};

float nick_animspeed;

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
    // Initialize nick
    sausage64_initmodel(&nick, MODEL_nick, nickMtx);
    sausage64_set_anim(&nick, ANIMATION_nick_idle); 
    sausage64_set_animcallback(&nick, nick_animcallback);
    
    // Set nick's animation speed based on region
    #if TV_TYPE == PAL
        nick_animspeed = 0.66;
    #else
        nick_animspeed = 0.5;
    #endif
}


/*==============================
    stage00_update
    Update stage variables every frame
==============================*/

void stage00_update(void)
{
    int i;
    
    // Poll for USB commands
    debug_pollcommands();  
    
    // Advance nick's animation
    sausage64_advance_anim(&nick, nick_animspeed);
    
    
    /* -------- Controller -------- */
    /* Nintendo's official button names */
    /*
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
    
    // Read the controller
    nuContDataGetEx(contdata, 0);
    
    // START is pressed
    if (contdata[0].trigger & START_BUTTON)
    {
    }
	
	if (fabs(contdata->stick_x) < 7){contdata->stick_x = 0;}
	if (fabs(contdata->stick_y) < 7){contdata->stick_y = 0;}
	
    if ((contdata->stick_x != 0 || contdata->stick_y != 0) && sausage64_get_currentanim(&nick) != ANIMATION_nick_run){
    	sausage64_set_anim(&nick, ANIMATION_nick_run); 
    }
    
    if ((contdata->stick_x == 0 && contdata->stick_y == 0) && sausage64_get_currentanim(&nick) != ANIMATION_nick_idle) {
    	sausage64_set_anim(&nick, ANIMATION_nick_idle);
    }

	 if ( contdata->stick_x != 0 || contdata->stick_y != 0) {
    	nick.yaw = atan2(contdata->stick_x, -contdata->stick_y) * (180 / M_PI); 
    }
    
    nick.pos[1] += contdata->stick_y / 20;
    nick.pos[0] += contdata->stick_x / 20;
    
    campos[2] += contdata->stick_y / 20;
    campos[0] -= contdata->stick_x / 20;
        
}


/*==============================
    stage00_draw
    Draw the stage
==============================*/

void stage00_draw(void)
{
    int i, ambcol = 100;
    float fmat1[4][4], fmat2[4][4], w;
    
    // Assign our glist pointer to our glist array for ease of access
    glistp = glist;

    // Initialize the RCP and framebuffer
    rcp_init();
    fb_clear(128, 128, 32);
    
    // Setup the projection matrix
    guPerspective(&projection, &normal, 45, (float)SCREEN_WD / (float)SCREEN_HT, 10.0, 1000.0, 0.01);
    
    // Rotate and position the view
    guMtxIdentF(fmat1);
    guRotateRPYF(fmat2, camang[2], camang[0], camang[1]);
    guMtxCatF(fmat1, fmat2, fmat1);
    guTranslateF(fmat2, campos[0], campos[1], campos[2]);
    guMtxCatF(fmat1, fmat2, fmat1);
    guMtxF2L(fmat1, &viewing);
    
    // Apply the projection matrix
    gSPMatrix(glistp++, &projection, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &viewing, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPPerspNormalize(glistp++, &normal);
    
    // Setup the Sausage64 camera for billboarding
    sausage64_set_camera(&viewing, &projection);
    
    // Setup the lights
    if (!uselight)
        ambcol = 255;
    for (i=0; i<3; i++)
    {
        light_amb.l.col[i] = ambcol;
        light_amb.l.colc[i] = ambcol;
        light_dir.l.col[i] = 255;
        light_dir.l.colc[i] = 255;
    }
    
    // Calculate the light direction so it's always projecting from the camera's position
    if (!freezelight)
    {
        light_dir.l.dir[0] = -127*sinf(camang[0]*0.0174532925);
        light_dir.l.dir[1] = 127*sinf(camang[2]*0.0174532925)*cosf(camang[0]*0.0174532925);
        light_dir.l.dir[2] = 127*cosf(camang[2]*0.0174532925)*cosf(camang[0]*0.0174532925);
    }
    
    // Send the light struct to the RSP
    gSPNumLights(glistp++, NUMLIGHTS_1);
    gSPLight(glistp++, &light_dir, 1);
    gSPLight(glistp++, &light_amb, 2);
    gDPPipeSync(glistp++);
    
    // Initialize the model matrix
    guMtxIdent(&modeling);
    gSPMatrix(glistp++, &modeling, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

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
    
    // Draw an axis on the floor for directional reference
    if (drawaxis)
        gSPDisplayList(glistp++, gfx_axis);

    guTranslate(&(nick.pos_mtx), nick.pos[0], nick.pos[1], nick.pos[2]);
    guRotate(&nick.rotx, nick.pitch, 1, 0, 0);
    guRotate(&nick.roty, nick.yaw, 0, 0, 1);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(nick.pos_mtx)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(nick.rotx)), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(nick.roty)), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    // Draw nick
    sausage64_drawmodel(&glistp, &nick);
    
    // Syncronize the RCP and CPU and specify that our display list has ended
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);
    
    // Ensure the chache lines are valid
    osWritebackDCache(&projection, sizeof(projection));
    osWritebackDCache(&modeling, sizeof(modeling));
    
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
    draw_debug_data
    Draws debug data
==============================*/


void draw_debug_data()
{
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 3, 4);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "contdata->stick_x: %d", contdata->stick_x);
    nuDebConTextPos(NU_DEB_CON_WINDOW0, 3, 5);
    nuDebConPrintf(NU_DEB_CON_WINDOW0, "contdata->stick_y: %d", contdata->stick_y);
}


/*********************************
     Model callback functions
*********************************/

/*==============================
    nick_animcallback
    Called before an animation finishes
    @param The animation that is finishing
==============================*/

void nick_animcallback(u16 anim)
{
    // Go to idle animation when we finished attacking
    switch(anim)
    {
        case ANIMATION_nick_run:
            sausage64_set_anim(&nick, ANIMATION_nick_idle);
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
            sausage64_set_anim(&nick, i);
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
    nick.interpolate = !nick.interpolate;
    return "Interpolation Toggled";
}


/*==============================
    command_toggleloop
    USB Command for toggling animation looping
==============================*/

char* command_toggleloop()
{
    nick.loop = !nick.loop;
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
