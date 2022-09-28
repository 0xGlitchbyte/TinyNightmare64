#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_
#define SCREEN_HT        240
#define SCREEN_WD        320
#define GFX_GLIST_LEN     2048
typedef struct {
  Mtx     projection;
  Mtx     modeling;
} Dynamic;
extern Dynamic gfx_dynamic;
extern Gfx* glistp;
extern Gfx gfx_glist[GFX_GLIST_LEN];
extern void gfxRCPInit(void);
extern void gfxClearCfb(void);
extern Gfx setup_rdpstate[];
extern Gfx setup_rspstate[];
#endif
