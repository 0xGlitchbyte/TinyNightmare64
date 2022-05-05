
#include <assert.h>
#include <nusys.h>
#include <nustd/math.h>
#include <string.h>

#include "graphic.h"
#include "Celebi.h"
#include "math.h"

void draw_cube(Dynamic *dynamicp, float t);
void SetViewMtx(Dynamic *);
void debug_console_int(char *name, int variable, int pos);
void debug_console_float(char *name, float variable, int pos);

void set_angle(float xangle_diff, float yangle_diff);
void move_to(float view_speed, float forward_speed, float up_speed);

int lim(u32 input);
int t = 1;
char conbuf[20];

static float cubescale;
float cubepan;
float cubeyaw;

typedef struct
{
  float x;
  float y;
  float z;
} Vec3;

typedef struct Position
{
  Vec3 pos;
  Vec3 forward;
  float xangle;
  float yangle;
} Position;
Position cam;

typedef struct
{
  float xview;
  float yview;
  float forward;
  float side;
  float up;
} Movement;
Movement move;

void set_angle(float xangle_diff, float yangle_diff)
{
  cam.xangle += xangle_diff;
  cam.yangle += yangle_diff;
  get_forward(cam.xangle, cam.yangle, &cam.forward.x, &cam.forward.y, &cam.forward.z);
}

void move_to(float side_speed, float forward_speed, float up_speed)
{
  cam.pos.x += cam.forward.x * forward_speed;
  cam.pos.z += cam.forward.z * forward_speed;

  if (side_speed != 0)
  {
    float x, y, z;
    get_forward(cam.xangle, cam.yangle + RAD_90, &x, &y, &z);
    cam.pos.x += x * side_speed;
    cam.pos.z += z * side_speed;
  }

  if (up_speed != 0)
  {
    cam.pos.y += up_speed; 
  }
}

NUContData contdata[1];

void initStage00()
{
  nuDebConDisp(NU_SC_SWAPBUFFER);
  cubescale = 1;
  cubepan = 0;
  cubeyaw = 0;
  cam.pos.x = 300;
  cam.pos.y = 10;
  cam.pos.z = 300;
  cam.forward.x = 0;
  cam.forward.y = 0;
  cam.forward.z = 0;
  cam.xangle = 0;
  cam.yangle = 0;
}

void SetViewMtx(Dynamic *dp)
{
  u16 perspNorm;

  /* The calculation and set-up of the projection-matrix  */
  guPerspective(
    &dp->projection,                     // Mtx *m
    &perspNorm,                          // u16 *perspNorm,
    50,                                  // FOV
    (float)SCREEN_WD / (float)SCREEN_HT, // ASPECT
    10,                                  // near plane clicaming
    10000,                               // far plane clicaming
    1.0F                                 // matrix object scaling
  );

  guLookAt(&dp->viewing,
    cam.pos.x,
    cam.pos.y,
    cam.pos.z,
    cam.pos.x + cam.forward.x,
    cam.pos.y + cam.forward.y,
    cam.pos.z + cam.forward.z,
    0, 1, 0
  );

  gSPPerspNormalize(glistp++, perspNorm);
  gSPLookAt(glistp++, &dp->viewing);
  gSPMatrix(glistp++, &(dp->projection), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPMatrix(glistp++, &(dp->viewing), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
}

void makeDL00(void)
{
  /* Specify the display list buffer  */
  glistp = gfx_glist;

  /*  The initialization of RCP  */
  gfxRCPInit();

  /* Clear the frame buffer and the Z-buffer  */
  gfxClearCfb();

  SetViewMtx(&gfx_dynamic);

  // guRotate(&gfx_dynamic.modeling, 0.0F, 0.0F, 0.0F, 0.0F);

  /* Draw a square  */
  draw_cube(&gfx_dynamic, t);

  /* End the construction of the display list  */
  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);

  /* Check if all are put in the array  */
  assert(glistp - gfx_glist < GFX_GLIST_LEN);

  /* Activate the RSP task.  Switch display buffers at the end of the task. */
  nuGfxTaskStart(&gfx_glist[0], (s32)(glistp - gfx_glist) * sizeof(Gfx), NU_GFX_UCODE_F3DEX, NU_SC_NOSWAPBUFFER);

  /* DEBUG CONSOLE!!!!!!!!!!!! Display characters on the frame buffer, debug console only */
  nuDebConDisp(NU_SC_SWAPBUFFER);
  debug_console_float("eyex", cam.pos.x + cam.forward.x, 1);
  debug_console_float("eyey", cam.forward.y, 2);
  debug_console_float("eyez", cam.pos.x + cam.forward.z, 3);
  debug_console_float("posx", cam.pos.x, 4);
  debug_console_float("posz", cam.pos.z, 5);
  //debug_console_float("xangle", cam.xangle, 6);
  //debug_console_float("yangle", cam.yangle, 7);
  // debug_console_float("posy",cam.pos.y,6);
  
  nuDebConDisp(NU_SC_SWAPBUFFER);
  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);
}

void draw_cube(Dynamic *dynamicp, float t)
{
  int i = 0;

  /* Create matrices for mult */
  /* CUBE IS AT CENTER OF EARTH */
  guTranslate(&dynamicp->pos, 0, 0, 0);
  guRotate(&dynamicp->rotx, cubepan, 1, 0, 0);
  guRotate(&dynamicp->roty, cubeyaw, 0, 1, 0);
  guScale(&dynamicp->scale, cubescale, cubescale, cubescale);

  /* apply transformation matrices, to stack */
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->pos)), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->scale)), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->rotx)), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->roty)), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

  /* Rendering setup */
  gDPSetRenderMode(glistp++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
  gSPTexture(glistp++, 0x8000, 0x8000, 0, 0, G_ON);
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  gDPSetCombineMode(glistp++, G_CC_DECALRGBA, G_CC_DECALRGBA);
  gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
  gSPSetGeometryMode(glistp++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);

  /* DRAW OBJECT
  ====================================================
  ====================================================*/
  gSPDisplayList(glistp++, Wtx_Celebi);
  /*=================================================
  ====================================================*/

  /* Finalise and exit drawing */
  gSPTexture(glistp++, 0, 0, 0, 0, G_OFF);
  gDPPipeSync(glistp++);
}

void updateGame00()
{
  /* Data reading of controller 1 */
  nuContDataGetEx(contdata, 0);
  move.xview = contdata->stick_y;
  move.yview = contdata->stick_x;
  move.xview *= fabs(move.xview);
  move.yview *= fabs(move.yview);
  move.forward = (lim(contdata[0].button & U_CBUTTONS) - lim(contdata[0].button & D_CBUTTONS));
  move.side = (lim(contdata[0].button & R_CBUTTONS) - lim(contdata[0].button & L_CBUTTONS));
  move.up = (lim(contdata[0].button & A_BUTTON) - lim(contdata[0].button & B_BUTTON));

  /*if(contdata[0].button & START_BUTTON){}*/
  if(contdata[0].button & A_BUTTON){
    cubescale += 0.001;
  }
  if(contdata[0].button & B_BUTTON){
    cubescale -= 0.001;
  }
  set_angle(-move.xview / 100000.0, move.yview / 100000.0);
  move_to(move.side * 5.0, move.forward * 5.0, move.up * 5.0);
}

void debug_console_int(char *name, int variable, int pos)
{
  nuDebConTextPos(0, 1, 1 * pos);
  sprintf(conbuf, "%s=%i", name, variable);
  nuDebConCPuts(0, conbuf);
}

void debug_console_float(char *name, float variable, int pos)
{
  nuDebConTextPos(0, 1, 1 * pos);
  sprintf(conbuf, "%s=%f", name, variable);
  nuDebConCPuts(0, conbuf);
}

int lim(u32 input)
{
  if (input == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
