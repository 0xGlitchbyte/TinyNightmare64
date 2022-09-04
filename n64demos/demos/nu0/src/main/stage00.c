#include <assert.h>
#include <nusys.h>
#include "graphic.h"
void shadetri(Dynamic* dynamicp);
void makeDL00(void){
  glistp = gfx_glist;
  gfxRCPInit();
  gfxClearCfb();
  guOrtho(&gfx_dynamic.projection,
	  -(float)SCREEN_WD/2.0F, (float)SCREEN_WD/2.0F,
	  -(float)SCREEN_HT/2.0F, (float)SCREEN_HT/2.0F,
	  1.0F, 10.0F, 1.0F);
  guRotate(&gfx_dynamic.modeling, 0.0F, 0.0F, 0.0F, 1.0F);
  shadetri(&gfx_dynamic);
  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);
  assert(glistp - gfx_glist < GFX_GLIST_LEN); nuGfxTaskStart(gfx_glist, (s32)(glistp - gfx_glist) * sizeof (Gfx), NU_GFX_UCODE_F3DEX , NU_SC_SWAPBUFFER); } static Vtx shade_vtx[] = { { -64, 64, -5, 0, 0, 0, 0, 0xff, 0, 0xff }, { 64, 64, -5, 0, 0, 0, 0, 0, 0, 0xff }, { 64, -64, -5, 0, 0, 0, 0, 0, 0xff, 0xff }, { -64, -64, -5, 0, 0, 0, 0xff, 0, 0, 0xff }, }; void shadetri(Dynamic* dynamicp) { gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->projection)),
		G_MTX_PROJECTION|G_MTX_LOAD|G_MTX_NOPUSH);
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicp->modeling)),
		G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);
  gSPVertex(glistp++,&(shade_vtx[0]),4, 0);
  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++,G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++,G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
  gSPClearGeometryMode(glistp++,0xFFFFFFFF);
  gSPSetGeometryMode(glistp++,G_SHADE| G_SHADING_SMOOTH);
  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
}
