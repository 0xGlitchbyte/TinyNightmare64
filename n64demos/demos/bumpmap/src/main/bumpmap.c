/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1995, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright  law.  They  may not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 *************************************************************************/

/*---------------------------------------------------------------------*
        Copyright (C) 1997,1998 Nintendo. (Originated by SGI)
        
        $RCSfile: bumpmap.c,v $
        $Revision: 1.15 $
        $Date: 1999/04/16 07:03:16 $
 *---------------------------------------------------------------------*/

/*
 * File:	bumpmap.c
 * Create Date:	Mon Apr 17 11:45:57 PDT 1995
 *
 * VERY simple app, draws a couple triangles spinning.
 *
 */

#include <ultra64.h>
#include <PR/ramrom.h>	/* needed for argument passing into the app */
#include <assert.h>

#include "bumpmap.h"

extern float fsin( float ), fcos( float );

/*
 * Symbol genererated by "makerom" to indicate the end of the code segment
 * in virtual (and physical) memory
 */
extern char _codeSegmentBssEnd[];
extern char _codeSegmentBssEnd[];

/*
 * Symbols generated by "makerom" to tell us where the static segment is
 * in ROM.
 */
extern char _staticSegmentRomStart[], _staticSegmentRomEnd[];

/*
 * Stacks for the threads as well as message queues for synchronization
 * This stack is ridiculously large, and could also be reclaimed once
 * the main thread is started.
 */
u64	bootStack[STACKSIZE/sizeof(u64)];

static void	idle(void *);
static void	mainproc(void *);

static OSThread	idleThread;
static u64	idleThreadStack[STACKSIZE/sizeof(u64)];

static OSThread	mainThread;
static u64	mainThreadStack[STACKSIZE/sizeof(u64)];

/* this number (the depth of the message queue) needs to be equal
 * to the maximum number of possible overlapping PI requests.
 * For this app, 1 or 2 is probably plenty, other apps might
 * require a lot more.
 */
#define NUM_PI_MSGS     8

static OSMesg PiMessages[NUM_PI_MSGS];
static OSMesgQueue PiMessageQ;

OSMesgQueue	dmaMessageQ,
		rdpMessageQ,
		retraceMessageQ;

OSMesg		dmaMessageBuf,
		rdpMessageBuf,
		retraceMessageBuf;

OSIoMesg	dmaIOMessageBuf;	/* see man page to understand this */

/*
 * Dynamic data.
 */
Dynamic dynamic;

/*
 * necessary for RSP tasks:
 */
    
/*
 * Task descriptor.
 */
OSTask	tlist =
{
    M_GFXTASK,			/* task type */
    OS_TASK_DP_WAIT,		/* task flags */
    NULL,			/* boot ucode pointer (fill in later) */
    0,				/* boot ucode size (fill in later) */
    NULL,			/* task ucode pointer (fill in later) */
    SP_UCODE_SIZE,		/* task ucode size */
    NULL,			/* task ucode data pointer (fill in later) */
    SP_UCODE_DATA_SIZE,		/* task ucode data size */
    &dram_stack[0],		/* task dram stack pointer */
    SP_DRAM_STACK_SIZE8,	/* task dram stack size */
    &rdp_output[0],		/* task fifo buffer ptr start */
    &rdp_output[0]+RDP_OUTPUT_LEN, /* task fifo buffer ptr end */
    NULL,			/* task data pointer (fill in later) */
    0,				/* task data size (fill in later) */
    NULL,			/* task yield buffer ptr (not used here) */
    0				/* task yield buffer size (not used here) */
};

Gfx		*glistp;	/* global for test case procs */
    
/*
 * global variables
 */
static float	theta = 0.0;
static int	rdp_flag = 0;
static int      draw_buffer = 0;

OSPiHandle	*handler;

void
boot(void)
{


    /* notice that you can't call osSyncPrintf() until you set
     * up an idle thread.
     */
    
    osInitialize();

    handler = osCartRomInit();


    osCreateThread(&idleThread, 1, idle, (void *)0,
		   idleThreadStack+STACKSIZE/sizeof(u64), 10);
    osStartThread(&idleThread);

    /* never reached */
}

static void
idle(void *arg)
{
    /* Initialize video */
    osCreateViManager(OS_PRIORITY_VIMGR);
    osViSetMode(&osViModeTable[OS_VI_NTSC_LAN1]);

    osViSetSpecialFeatures( OS_VI_GAMMA_OFF | OS_VI_GAMMA_DITHER_OFF );
    
    /*
     * Start PI Mgr for access to cartridge
     */
    osCreatePiManager((OSPri)OS_PRIORITY_PIMGR, &PiMessageQ, PiMessages, 
		      NUM_PI_MSGS);
    
    /*
     * Create main thread
     */
    osCreateThread(&mainThread, 3, mainproc, arg,
		   mainThreadStack+STACKSIZE/sizeof(u64), 10);
    
    osStartThread(&mainThread);

    /*
     * Become the idle thread
     */
    osSetThreadPri(0, 0);

    for (;;);
}

float
fpow( float x, float pw )
{
    float res = 1.0;

    res = sqrtf( x );
    res = sqrtf( x * res );

    return( x );
}


#define RGBA5551(r,g,b,a) (((r & 0xf8)<<8)| \
                           ((g & 0xf8)<<3)| \
                           ((b & 0xf8)>>2)| \
                           ((a & 0x80)>>7)    )


typedef float Vertex[3];
typedef float V6[6];

#include "obj_real.h"

void
do_lighting( void )
{
    unsigned short *tlut, sht;
    V6     *litclrs;
    Vertex *dirs;
    float s, fr, fg, fb;
    float mat_r, mat_g, mat_b;
    int r,g,b;
    int lnum, vnum;
    int lmax, vmax;

    dirs = obj0_vects_real;
    tlut = &(obj0_tlut_real[0]);
    litclrs = &(obj0_lightclrs_real[0]);
    lmax = obj0_num_lights_real;
    vmax = obj0_num_vects_real;

    mat_r = litclrs[0][0];
    mat_g = litclrs[0][1];
    mat_b = litclrs[0][2];

    for(vnum=0; vnum<vmax; vnum++ ) {

	fr = litclrs[0][3] * mat_r;
	fg = litclrs[0][4] * mat_g;
	fb = litclrs[0][5] * mat_b;

	for(lnum=1; lnum<lmax; lnum++ ) {
	    s = dirs[0][0] * litclrs[lnum][0] +
		dirs[0][1] * litclrs[lnum][1] +
		dirs[0][2] * litclrs[lnum][2];
	    if( s > 0 ) {
		fr += s * litclrs[lnum][3] * mat_r;
		fg += s * litclrs[lnum][4] * mat_g;
		fb += s * litclrs[lnum][5] * mat_b;
	    };
	};

	r = 4 + 256 * fpow( fr / 256.0, 0.75 );
	g = 4 + 256 * fpow( fg / 256.0, 0.75 );
	b = 4 + 256 * fpow( fb / 256.0, 0.75 );

	if( r > 255 ) r = 255;
	if( g > 255 ) g = 255;
	if( b > 255 ) b = 255;

	if( r < 0 ) r = 0;
	if( g < 0 ) g = 0;
	if( b < 0 ) b = 0;

	sht = RGBA5551(r,g,b,255);

	*tlut++ = sht;
	*tlut++ = sht;
	*tlut++ = sht;
	*tlut++ = sht;

	dirs++;
    };


}

float dtheta = 1.0;

/*
 * This is the main routine of the app.
 */
static void
mainproc(void *arg)
{
    OSTask		*tlistp;
    Dynamic		*dynamicp;
    char		*staticSegment;
    u16   perspnorm;
    float	ex, ey, ez, w0, w1;

    /*
     * Setup the message queues
     */
    osCreateMesgQueue(&dmaMessageQ, &dmaMessageBuf, 1);
    
    osCreateMesgQueue(&rdpMessageQ, &rdpMessageBuf, 1);
    osSetEventMesg(OS_EVENT_DP, &rdpMessageQ, NULL);
    
    osCreateMesgQueue(&retraceMessageQ, &retraceMessageBuf, 1);
    osViSetEvent(&retraceMessageQ, NULL, 1);
    
    /*
     * Stick the static segment right after the code/data segment
     */
    staticSegment = _codeSegmentBssEnd;

    dmaIOMessageBuf.hdr.pri      = OS_MESG_PRI_NORMAL;
    dmaIOMessageBuf.hdr.retQueue = &dmaMessageQ;
    dmaIOMessageBuf.dramAddr     = staticSegment;
    dmaIOMessageBuf.devAddr      = (u32)_staticSegmentRomStart;
    dmaIOMessageBuf.size         = (u32)_staticSegmentRomEnd-(u32)_staticSegmentRomStart;

    osEPiStartDma(handler, &dmaIOMessageBuf, OS_READ);
    
    /*
     * Wait for DMA to finish
     */
    (void)osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);

    /*
     * Main game loop
     */
    while (1) {

	/*
	 * pointers to build the display list.
	 */
	tlistp = &tlist;
	dynamicp = &dynamic;

	glistp = dynamicp->glist;

	/*
	 * Tell RCP where each segment is
	 */
	gSPSegment(glistp++, 0, 0x0);	/* Physical address segment */
	gSPSegment(glistp++, STATIC_SEGMENT, OS_K0_TO_PHYSICAL(staticSegment));
	gSPSegment(glistp++, CFB_SEGMENT, OS_K0_TO_PHYSICAL(cfb[draw_buffer]));

	/*
	 * Initialize RDP state.
	 */
	gSPDisplayList(glistp++, rdpinit_dl);

	/*
	 * Initialize RSP state.
	 */
	gSPDisplayList(glistp++, rspinit_dl);

	/*
	 * Clear color framebuffer.
	 */
	gSPDisplayList(glistp++, clearcfb_dl);


  /*
   * Set up matrices
   */
  guPerspective(&(dynamicp->projection), &perspnorm,
                30.0, 320.0/240.0, 50.0, 8000.0, 1.0);
  gSPPerspNormalize(glistp++, perspnorm);

  w0 = fcos((theta/180*3.14159));
  w1 = fsin((theta/180*3.14159));

  ex = w0 * 500.0;
  ey = w0 * 100.0;
  ez = w1 * 500.0;
  
  theta += dtheta;
  if (theta >= 360.0)
      theta -= 360.0;

  guLookAt(&(dynamicp->viewing),
	   ex, ey, ez,
	   0, 0, 0,
	   0, 1, 0);

/*
  guRotate(&(dynamicp->modeling_rotate1), 0.0, 1.0, 1.0, 1.0);
  guRotate(&(dynamicp->modeling_rotate2), 30+theta/7, 0.0, 1.0, 0.0);
  guTranslate(&(dynamicp->modeling_translate), 0.0, 0.0, -550.0);
*/
  guTranslate(&(dynamicp->modeling_translate), 0.0, 0.0, 0.0);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)),
            G_MTX_PROJECTION|G_MTX_LOAD|G_MTX_NOPUSH);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->viewing)),
            G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->modeling_translate)),
            G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_NOPUSH);
/*
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->modeling_rotate1)),
            G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_NOPUSH);
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->modeling_rotate2)),
            G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_NOPUSH);
*/
 
  gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER |
                     G_CULL_BACK);

	do_lighting();

	/* simple triangle: NOT!!! rww 8aug95 */

	gSPDisplayList(glistp++, textri_dl);

	gDPFullSync(glistp++);
	gSPEndDisplayList(glistp++);

#ifdef DEBUG
#ifndef __MWERKS__
	assert((glistp-dynamicp->glist) < GLIST_LEN);
#endif
#endif

	/* 
	 * Build graphics task:
	 *
	 */
	tlistp->t.ucode_boot = (u64 *) rspbootTextStart;
	tlistp->t.ucode_boot_size = (u32)rspbootTextEnd - (u32)rspbootTextStart;

	/*
	 * choose which ucode to run:
	 */
	if (rdp_flag) {
	    /* RSP output over fifo to RDP: */
		tlistp->t.ucode = (u64 *) gspF3DEX2_fifoTextStart;
		tlistp->t.ucode_data = (u64 *) gspF3DEX2_fifoDataStart; 
	} else {
	    /* RSP output over XBUS to RDP: */
		tlistp->t.ucode = (u64 *) gspF3DEX2_xbusTextStart;
		tlistp->t.ucode_data = (u64 *) gspF3DEX2_xbusDataStart;
	}
	
	/* initial display list: */
	tlistp->t.data_ptr = (u64 *) dynamicp->glist;
	tlistp->t.data_size = (u32)((glistp - dynamicp->glist) * sizeof(Gfx));

	/*
	 * Write back dirty cache lines that need to be read by the RCP.
	 */
	osWritebackDCache(&dynamic, sizeof(dynamic));
	
	/*
	 * start up the RSP task
	 */
	osSpTaskStart(tlistp);
	
	/* wait for SP completion */

	(void)osRecvMesg(&rdpMessageQ, NULL, OS_MESG_BLOCK);

	/* setup to swap buffers */
	osViSwapBuffer(cfb[draw_buffer]);

	/* Make sure there isn't an old retrace in queue 
	 * (assumes queue has a depth of 1) 
	 */
	if (MQ_IS_FULL(&retraceMessageQ))
	    (void)osRecvMesg(&retraceMessageQ, NULL, OS_MESG_NOBLOCK);
	
	/* Wait for Vertical retrace to finish swap buffers */
	(void)osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);
	draw_buffer ^= 1;

    }
}
