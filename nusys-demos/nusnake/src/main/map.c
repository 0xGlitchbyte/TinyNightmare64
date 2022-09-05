/*============================================================================
  NuSYSTEM sample program [SNAKE TAIL HACK]
  
  map.c
  
  Copyright (C) 1997, NINTENDO Co,Ltd.
  ============================================================================*/

#include <assert.h>
#include <nusys.h>

#include "main.h"
#include "graphic.h"
#include "para.h"
#include "./tuti.h"	/*The texture for the map base */
#include "./kabe.h"	/*The object for the map base */
#include "./saku.h"	/*The object for the map base */

/*
   6: Once
   7: Twice 
   8: Four times
   9: Eight times
   10: Sixteen times 
   */
#define MAP_WRAP	9
#define MOV_WRAP	6


/*The position data of default of the map object */
float ObjPosTbl[OBJ_MAX][3] = {
    { -BASE_MAX_SIZE/5*3,  BASE_MAX_SIZE/5*3,  0.0 },
    {                0.0,  BASE_MAX_SIZE/5*3,  0.0 },
    {  BASE_MAX_SIZE/5*3,  BASE_MAX_SIZE/5*3,  0.0 },
    { -BASE_MAX_SIZE/5*3,                0.0,  0.0 },
    {  BASE_MAX_SIZE/5*3,                0.0,  0.0 },
    { -BASE_MAX_SIZE/5*3, -BASE_MAX_SIZE/5*3,  0.0 },
    {                0.0, -BASE_MAX_SIZE/5*3,  0.0 },
    {  BASE_MAX_SIZE/5*3, -BASE_MAX_SIZE/5*3,  0.0 },
    {                0.0,                0.0, 10.0 }
};

/*The vertex data of the game map */
Vtx base_vtx[] = {
    { -BASE_MAX_SIZE,-BASE_MAX_SIZE, 0.0, 0, 31<<MAP_WRAP, 31<<MAP_WRAP, 0,0,127,0xff},
    {  BASE_MAX_SIZE,-BASE_MAX_SIZE, 0.0, 0, 00<<MAP_WRAP, 31<<MAP_WRAP, 0,0,127,0xff},
    {  BASE_MAX_SIZE, BASE_MAX_SIZE, 0.0, 0, 00<<MAP_WRAP, 00<<MAP_WRAP, 0,0,127,0xff},
    { -BASE_MAX_SIZE, BASE_MAX_SIZE, 0.0, 0, 31<<MAP_WRAP, 00<<MAP_WRAP, 0,0,127,0xff},
};


void DrawMap( Dynamic* dynamicp );
void DrawMapSaku( Dynamic* dynamicp );
void DrawMapObj( Dynamic* );
void DrawMovie( Dynamic* dynamicp, unsigned short *);


/*------------------------------*/
/*  Draw the map base 	*/
/*  IN:	dp The dynamic pointer 	*/
/*  RET:None 			*/
/*------------------------------*/
void
DrawMap( Dynamic* dp )
{
    /* The calculation of the model coordinate system  */
    guTranslate( &dp->base_trans, 0.0, 0.0, 0.0 );
    guRotateRPY( &dp->base_rotate, 0.0, 0.0, 0.0 );
    guScale( &dp->base_scale, 1.0, 1.0, 1.0 );

    /* Setting of the model-matrix  */
    gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dp->base_trans)),
	      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dp->base_rotate)),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dp->base_scale)),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gDPSetCycleType(glistp++,G_CYC_1CYCLE);

    /* Clear and set up of geometry */
    gSPClearGeometryMode(glistp++,0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_LIGHTING | G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH |
		       G_CULL_BACK);

    gDPSetRenderMode(glistp++,G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

    gDPSetColorDither(glistp++,G_CD_DISABLE);

    gDPSetCombineMode (glistp++,G_CC_DECALRGBA, G_CC_DECALRGBA);
    gDPSetTexturePersp (glistp++,G_TP_PERSP);
    gDPSetTextureLOD (glistp++,G_TL_TILE);
    gDPSetTextureFilter (glistp++,G_TF_POINT);
    gDPSetTextureConvert(glistp++,G_TC_FILT);
    gDPSetTextureLUT (glistp++,G_TT_NONE);
    gSPTexture(glistp++,0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);

    /* Load the vertex  */
    gSPVertex(glistp++,&(base_vtx[0]), 4, 0);

    /* Read the texture  */
    gDPLoadTextureBlock(glistp++,
			tuti,
			G_IM_FMT_RGBA,
			G_IM_SIZ_16b,
			32, 32, 0,
			G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
			5, 5, G_TX_NOLOD, G_TX_NOLOD);

    /* Draw a quadrangle by two pieces of triangles  */
    gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
    gDPPipeSync(glistp++);
}


/*------------------------------*/
/*  Draw the map fence 	*/
/*  IN:	dp The dynamic pointer 	*/
/*  RET:None 			*/
/*------------------------------*/
void 
DrawMapSaku( Dynamic* dp )
{
    /* The calculation of the model coordinate system  */
    guTranslate( &dp->saku_trans,0.0, -BASE_MAX_SIZE, 40.0);
    guRotateRPY( &dp->saku_rotate, 90.0, 0.0, 0.0 );
    guScale( &dp->saku_scale, 0.33, 0.66, 0.33 );
    
    /* Setting the model-matrix  */
    gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dp->saku_trans)),
	      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    
    gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dp->saku_rotate)),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    
    gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dp->saku_scale)),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    
    gDPSetRenderMode(glistp++,G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_TEX_EDGE2);
    
    gSPClearGeometryMode(glistp++,0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_LIGHTING );
    
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
    gDPSetCombineMode(glistp++,G_CC_DECALRGB, G_CC_DECALRGB);
    
    gSPDisplayList(glistp++,saku_mdl_model0);
    gDPPipeSync(glistp++);
}


/*--------------------------------------*/
/*  Draw the object on the map 	*/
/*  IN:	dp The dynamic pointer 		*/
/*  RET:None 				*/
/*--------------------------------------*/
void
DrawMapObj( Dynamic* dp )
{
    int idx;

    for(idx = 0; idx < 8; idx++){
	/* The calculation of the model coordinate system  */
	guTranslate( &dp->obj_trans[idx],
		    ObjPosTbl[idx][0],
		    ObjPosTbl[idx][1],
		    ObjPosTbl[idx][2]+120.0*OBJ_SCALE
		    );
	guScale( &dp->obj_scale[idx], OBJ_SCALE, OBJ_SCALE, OBJ_SCALE );

	guRotateRPY( &dp->obj_rotate[idx], 0.0, 0.0, 0.0 );

	/* Setting the model-matrix  */
	gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dp->obj_trans[idx])),
		  G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dp->obj_rotate[idx])),
		  G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

	gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dp->obj_scale[idx])),
		  G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    
	gDPSetRenderMode(glistp++,G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    
	gSPClearGeometryMode(glistp++,0xFFFFFFFF);
	gSPSetGeometryMode(glistp++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH |
			   G_LIGHTING | G_CULL_BACK);

	gDPSetCycleType(glistp++, G_CYC_1CYCLE);
	gDPSetCombineMode(glistp++,G_CC_DECALRGB, G_CC_DECALRGB);

	gSPDisplayList(glistp++,kabe_mdl_model0);
	gDPPipeSync(glistp++);
    }
}



Vtx movie_vtx[] = {
    {          0.0, MOVIE_SIZE, 0.0,  0, 31 << MOV_WRAP, 00 << MOV_WRAP, 0, 0, 127, 0xff},
    {  -MOVIE_SIZE, MOVIE_SIZE, 0.0,  0, 00 << MOV_WRAP, 00 << MOV_WRAP, 0, 0, 127, 0xff},
    {  -MOVIE_SIZE,        0.0, 0.0,  0, 00 << MOV_WRAP, 31 << MOV_WRAP, 0, 0, 127, 0xff},
    {         0.0,         0.0, 0.0,  0, 31 << MOV_WRAP, 31 << MOV_WRAP, 0, 0, 127, 0xff},

    {   MOVIE_SIZE, MOVIE_SIZE, 0.0,  0, 63 << MOV_WRAP, 00 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0, MOVIE_SIZE, 0.0,  0, 32 << MOV_WRAP, 00 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0,        0.0, 0.0,  0, 32 << MOV_WRAP, 31 << MOV_WRAP, 0, 0, 127, 0xff},
    {   MOVIE_SIZE,        0.0, 0.0,  0, 63 << MOV_WRAP, 31 << MOV_WRAP, 0, 0, 127, 0xff},

    {          0.0,         0.0, 0.0,  0, 31 << MOV_WRAP, 32 << MOV_WRAP, 0, 0, 127, 0xff},
    {  -MOVIE_SIZE,         0.0, 0.0,  0, 00 << MOV_WRAP, 32 << MOV_WRAP, 0, 0, 127, 0xff},
    {  -MOVIE_SIZE, -MOVIE_SIZE, 0.0,  0, 00 << MOV_WRAP, 63 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0, -MOVIE_SIZE, 0.0,  0, 31 << MOV_WRAP, 63 << MOV_WRAP, 0, 0, 127, 0xff},

    {   MOVIE_SIZE,         0.0, 0.0,  0, 63 << MOV_WRAP, 32 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0,         0.0, 0.0,  0, 32 << MOV_WRAP, 32 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0, -MOVIE_SIZE, 0.0,  0, 32 << MOV_WRAP, 63 << MOV_WRAP, 0, 0, 127, 0xff},
    {   MOVIE_SIZE, -MOVIE_SIZE, 0.0,  0, 63 << MOV_WRAP, 63 << MOV_WRAP, 0, 0, 127, 0xff},
};


/*--------------------------------------------------------------*/
/*  Draw the movie part of the center of the map 			*/
/*  IN:	dp The dynamic pointer 	*bmp The texture header address 	*/
/*  RET:None 							*/
/*--------------------------------------------------------------*/
void
DrawMovie( Dynamic* dp, unsigned short *bmp )
{
    /* The calculation of the model coordinate system  */
    guTranslate( &dp->obj_trans[8], 0.0, 0.0, 1.0 );
    guRotateRPY( &dp->obj_rotate[8], 0.0, 0.0, 0.0 );
    guScale( &dp->obj_scale[8], 1.0, 1.0, 1.0 );
    
    /* Setting the model-matrix  */
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->obj_trans[8])),
	      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->obj_rotate[8])),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->obj_scale[8])),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gDPSetCycleType(glistp++,G_CYC_1CYCLE);

    gSPClearGeometryMode(glistp++,0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_LIGHTING | G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH |
		       G_CULL_BACK);


    /*Paste to the base by the decal mode */
    gDPSetRenderMode(glistp++,G_RM_AA_ZB_XLU_DECAL, G_RM_AA_ZB_XLU_DECAL2);

    gDPSetColorDither(glistp++,G_CD_DISABLE);

    gDPSetCombineMode (glistp++,G_CC_DECALRGBA, G_CC_DECALRGBA);

    gDPSetTexturePersp (glistp++,G_TP_PERSP);
    gDPSetTextureLOD (glistp++,G_TL_TILE);
    gDPSetTextureFilter (glistp++,G_TF_BILERP);
    gDPSetTextureConvert(glistp++,G_TC_FILT);
    gDPSetTextureLUT (glistp++,G_TT_NONE);
    gSPTexture(glistp++,0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);

    
    /* Load vertex data  */
    gSPVertex(glistp++,&(movie_vtx[0]), 16, 0);

    /* Read the 64-dot X 64-dot texture by dividing to four  */
    /* In other words, read four pieces of 32-dot X 32-dot textures  */
    /* Read texture data (the upper-left part) */
    gDPLoadTextureTile(glistp++,
		       bmp,
		       G_IM_FMT_RGBA,
		       G_IM_SIZ_16b,
		       64, 64,
		       0, 0, 31, 31,
		       0,
		       G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
		       0, 0,
		       G_TX_NOLOD, G_TX_NOLOD);
    gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);

    /* Read texture data (the upper-right part) */
    gDPLoadTextureTile(glistp++,
		       bmp,
		       G_IM_FMT_RGBA,
		       G_IM_SIZ_16b,
		       64, 64,
		       32, 0, 63, 31,
		       0,
		       G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
		       0, 0,
		       G_TX_NOLOD, G_TX_NOLOD);
    gSP2Triangles(glistp++,4,5,6,0,4,6,7,0);

    /* Read texture data (the bottom-left part) */
    gDPLoadTextureTile(glistp++,
		       bmp,
		       G_IM_FMT_RGBA,
		       G_IM_SIZ_16b,
		       64, 64,
		       0, 32, 31, 63,
		       0,
		       G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
		       0, 0,
		       G_TX_NOLOD, G_TX_NOLOD);
    gSP2Triangles(glistp++,8,9,10,0,8,10,11,0);

    /* Read texture data (the bottom-right part) */
    gDPLoadTextureTile(glistp++,
		       bmp,
		       G_IM_FMT_RGBA,
		       G_IM_SIZ_16b,
		       64, 64,
		       32, 32, 63, 63,
		       0,
		       G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
		       0, 0,
		       G_TX_NOLOD, G_TX_NOLOD);
    gSP2Triangles(glistp++,12,13,14,0,12,14,15,0);

    gDPPipeSync(glistp++);
}
