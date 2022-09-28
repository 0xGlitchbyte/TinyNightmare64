/*============================================================================
  NuSYSTEM sample program [SNAKE TAIL HACK]
  
  gamemain.c
  
  Copyright (C) 1997, NINTENDO Co,Ltd.
  ============================================================================*/

#include <assert.h>
#include <nusys.h>
#include <nualsgi_n.h>

#include "main.h"
#include "graphic.h"
#include "math.h"
#include "font.h"
#include "para.h"

#include "./kiyaata1.h"/*Head model data of player 1 */
#include "./kiyadou1.h"/*Tail model data of player 1 */
#include "./kiyaata2.h"/*Head model data of player 2 */
#include "./kiyadou2.h"/*Tail model data of player 2 */
#include "./kiyaata3.h"/*Head model data of player 3 */
#include "./kiyadou3.h"/*Tail model data of player 4 */
#include "./kiyaata4.h"/*Head model data of player 4 */
#include "./kiyadou4.h"/*Tail model data of player 4 */

#define START_TIME	100000000LL/* 100 */
#define TAIL_NUM	0
#define TIME_UP		1

/* Function declaration  */
void InitSnake( void );
void InitShotTail( void );
void DrawHead( Dynamic* , int );
void UpdateTailPos( int );
void RecordHeadPos( int );
void DrawTail( Dynamic* , int );
void ChgHeadDir( int, int, int);
void DrawShotTail( Dynamic* );
void EntryShotTail( int, int );
void EraseShotTail( int );
void ShotTailMove( void );
void SlideTail( int );
void ReadMovie( int , Dynamic* );
void DrawInfo( void );
int CheckTailNum( int );
void InitGame( void );

/* extern declaration */
/* nusys */
extern u32 nuScRetraceCounter;    /* The retrace counter */

/* gameover.c */
extern int SavePak;
extern int SavePakEnd;

/* map.c */
extern void DrawMap( Dynamic* dynamicp );
extern void DrawMapSaku( Dynamic* dynamicp );
extern void DrawMapObj( Dynamic* );
extern void DrawMovie( Dynamic* dynamicp, unsigned short * );

/* hitchk.c */
extern void Snake2Wall( int );
extern void Snake2Obj( int );
extern void Snake2Enemy( int );
extern int ShotTail2Wall( float, float );
extern int ShotTail2Obj( float, float );
extern void Snake2ShotTail( int );
extern void Snake2EnemyTail( int );

/* main.c */
extern void SaveContPak( void );
extern NUContData ContTriger[];
extern int TraceIdx;
extern int ReplayMode;
volatile extern int stage;

/* view.c */
extern void SetViewMtx( Dynamic* dynamicp );

/* spec */
extern u8 _moviedataSegmentRomStart[];
extern u8 _moviedataSegmentRomEnd[];


/* Variable declaration */
float View;			/* The camera position */
int MovieNumber;		/* The texture number for the movie */
SnakePara Snake;		/* Various kinds of parameters of each machine */
SnakeMove snakemove[4];	/* The buffer for drawing the body of each machine */
ShotTail mtx_shot[20];	/* Various kinds of parameters of shot tails */

int Pause;			/* The flag for the pause */
int MovieUpDate;		/* The counter for updating the movie */

OSTime	StartTime;		/* Start time of the game */
OSTime	NowTime;		/* The current time */
OSTime	NowPauseTime;	/* For keeping time at the pause */
OSTime	LapTime;		/* Elapsed time */

int Winner;			/* The winner number */
int UpTempo;		/* The flag for tempo-up sequence */

/*----------------------------------------------------------------*/
/*  makegameDL									*/
/*  Make the display list and activate the graphic task. 		*/
/*  IN:	None									*/
/*  RET:	None									*/
/*----------------------------------------------------------------*/
void
makegameDL(void)
{
    int idx;
    Dynamic* dynamicp;

    /* Specify the dynamic buffer */
    dynamicp = &gfx_dynamic[gfx_gtask_no];

    /* Specify the display list buffer */
    glistp = &gfx_glist[gfx_gtask_no][0];

    /* The initialization of RSP and RDP */
    gfxRCPInit();

    /* Clear the frame and Z-buffer */
    gfxClearCfb();

    /* Setting the view-matrix */
    SetViewMtx( dynamicp );

    /* Draw the map */
    DrawMap( dynamicp );
    DrawMapSaku( dynamicp );
    DrawMapObj( dynamicp );

    /* Read the texture for the movie by PI */
    ReadMovie( MovieNumber, dynamicp );

    /* Draw the texture read by MovieBuf (MovieBuf[0]~MovieBuf[3] is unused data) */
    DrawMovie( dynamicp, &(dynamicp->MovieBuf[4]));

    /* Draw the self-machine */
    for(idx = 0; idx < 4; idx++){
	UpdateTailPos( idx );
	DrawTail( dynamicp, idx );
	DrawHead( dynamicp, idx );
    }

    /* Draw the shot tail */
    DrawShotTail( dynamicp );

    /* Display screen information */
    DrawInfo();
    
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);

    assert((glistp - gfx_glist[gfx_gtask_no]) < GFX_GLIST_LEN);

    /* Activate the RSP task and switch display buffers */
    nuGfxTaskStart(&gfx_glist[gfx_gtask_no][0],
		   (s32)(glistp - gfx_glist[gfx_gtask_no]) * sizeof (Gfx),
		   NU_GFX_UCODE_F3DEX , NU_SC_SWAPBUFFER);

    /* Specify next graphic task number */
    gfx_gtask_no++;
    gfx_gtask_no %= GFX_GTASK_NUM;
}


/*----------------------------------------------------*/
/* updateGame							*/
/* The progressing process of the game main part	*/
/*  IN:	None							*/
/*  RET:	None							*/
/*----------------------------------------------------*/
void
updateGame(void)
{
    int padnum;
    int idx;
    int EnemyNum, EnemyTailNum;
    s32 tempo;

    /* PAUSE */
    if(ContTriger[0].trigger & START_BUTTON){
	nuAuSndPlayerPlay(EFF_PAUSE);
	nuAuSndPlayerSetPitch(44100.0/32000);
	Pause ^= 1;
    }

    
    /* Reset */
    if((ContTriger[0].trigger & L_TRIG) &&
       (ContTriger[0].trigger & R_TRIG)){
	/* Remove the call-back function */
	nuGfxFuncRemove();
	/* Return the title */
	stage = 0;
    }

    /* Update the timer */
    if( Pause == 0 ){
	NowTime = osGetTime();
	NowPauseTime = NowTime - StartTime;
	LapTime = START_TIME - OS_CYCLES_TO_USEC(NowTime - StartTime);
	LapTime /= 1000;
    }else{    /* Keep the time */
	NowTime = osGetTime();
	StartTime = NowTime - NowPauseTime;
	LapTime = START_TIME - OS_CYCLES_TO_USEC(NowTime - StartTime);
	LapTime /= 1000;
    }

    /* The end according to number of tails (when number of tails becomes 20) */
    if(CheckTailNum(TAIL_NUM)){
	/* Indicate next stage to main */
	stage = 2;
	SavePak = 0;
	SavePakEnd = 0;
	/* Remove the call-back function */
	nuGfxFuncRemove();
    }

    /* The end according to time-up */
    if(LapTime > NowTime){
	CheckTailNum(TIME_UP);
	LapTime = 0;
	/* Indicate next stage to main */
	stage = 2;
	SavePak = 0;
	SavePakEnd = 0;
	/* Remove the call-back function */
	nuGfxFuncRemove();
    }

    /* Change the camera position */
    if(nuContData[0].button & U_CBUTTONS){
	if((View += 1.0) > 89.0){
	    View = 89.0;
	}
    }
    /* The same as the above */
    if(nuContData[0].button & D_CBUTTONS){
	if((View -= 1.0) <= -89.0){
	    View = -89.0;
	}
    }

    /* If it is not on the pause */
    if(Pause == 0){
	/* The updating process of each pad */
	for(padnum = 0; padnum < NU_CONT_MAXCONTROLLERS; padnum++){

	    /* Go to the next if the controller pad is not connected */
	    /* Depend on the result of the initialization by nuContInit() of main.c */
	    if(nuContStatus[padnum].errno != 0){
		continue;
	    }

	    /* Go to the next if it's on fainting */
	    if(Snake.sleepcnt[padnum] > 0){
		continue;
	    }

	    /* If the head is not piled up on the opponent's tail */
	    if(Snake.enemytailhit[padnum] == -1 ){
		/* The button */
		if(ContTriger[padnum].trigger & A_BUTTON){
		    /* Do entry if the shooting tail can do entry */
		    if(Snake.tailcnt[padnum] > 0){
			nuAuSndPlayerPlay(EFF_SHOT_TAIL);
			nuAuSndPlayerSetPitch(44100.0/32000);
			EntryShotTail( padnum, Snake.tail_attr[padnum][1]);
		    }
		}
	    }else{		/* If the head is piled up on the opponent's tail */
		/* The button */
		EnemyNum = Snake.enemytailhit[padnum];
		EnemyTailNum = Snake.enemytailhitnum[padnum];

		/* Take away the opponent's tail */
		if(ContTriger[padnum].button & A_BUTTON){
		    nuAuSndPlayerPlay(EFF_EAT_TAIL);
		    nuAuSndPlayerSetPitch(44100.0/32000);
		    for(idx = EnemyTailNum; idx < Snake.tailcnt[EnemyNum]+1; idx++){
			Snake.tail_attr[padnum][++Snake.tailcnt[padnum]] =
			  Snake.tail_attr[EnemyNum][idx];
			Snake.tail_attr[EnemyNum][idx] = -1;
		    }
		    /* Shorten the tail that the one who is taken away his tail */
		    Snake.tailcnt[EnemyNum] = EnemyTailNum - 1;
		}
	    }

	    /* Don't move during each 3D stick of X and Y between -3 to 3 (play)*/
	    if(!((nuContData[padnum].stick_x/10 < 3 &&
		  nuContData[padnum].stick_x/10 > -3) &&
		 (nuContData[padnum].stick_y/10 < 3 &&
		  nuContData[padnum].stick_y/10 > -3 ))){

		/* Move the head by the 3D stick (directions of up, down, right and left) */
		if( Zettai(nuContData[padnum].stick_x/5) >
		   Zettai(nuContData[padnum].stick_y/5) ){
		    Snake.pos_x[padnum][0] += nuContData[padnum].stick_x/5;
		    nuContData[padnum].stick_y = 0;
		    /* Change the head direction corresponding to the direction of the 3D stick */
		    ChgHeadDir( padnum, nuContData[padnum].stick_x, 0 ); 
		}else{
		    Snake.pos_y[padnum][0] += nuContData[padnum].stick_y/5;
		    nuContData[padnum].stick_x = 0;
		    /* Change the head direction corresponding to the direction of the 3D stick */
		    ChgHeadDir( padnum, 0, nuContData[padnum].stick_y ); 
		}
		Snake.stickon[padnum] = 1;
	    }else{
		Snake.stickon[padnum] = 0;
	    }

	    Snake2Wall( padnum ); /* Collision detection:  the self-machine vs. map area */
	    Snake2Obj( padnum ); /* Collision detection:  the self-machine vs. map object */
	    Snake2Enemy( padnum ); /* Collision detection:  the self-machine vs. enemy */
	    Snake2EnemyTail( padnum ); /* Collision detection:  the self-machine vs. opponents */

	
	    /* Return to the beginning if it hits something */
	    if(Snake.wallhit[padnum] == 1 ||	/* Hit the wall */
	       Snake.objhit[padnum] != -1 ||	/* Hit the map object */
	       Snake.enemyhit[padnum] != -1){	/* Hit the enemy */
		Snake.pos_x[padnum][0] -= nuContData[padnum].stick_x/5;
		Snake.pos_y[padnum][0] -= nuContData[padnum].stick_y/5;
	    }else{
		if(Snake.stickon[padnum])  /* If control stick has collapsed some degree */
		  RecordHeadPos( padnum ); /* Record the head locus in the buffer */
	    }

	    /* Collision detection between the self-machine and the flown tail */
	    Snake2ShotTail( padnum );
	    if(Snake.shottailhit[padnum] != -1){
		nuAuSndPlayerPlay(EFF_CUR_MOVE);
		nuAuSndPlayerSetPitch(44100.0/32000);
		
		/* Increase one tail connected the self-machine (20 max.) */
		if((Snake.tailcnt[padnum]++) >= 20){
		    Snake.tailcnt[padnum] = 20;
		}
		/* Register the hit tail to the own rear side */
		Snake.tail_attr[padnum][Snake.tailcnt[padnum]] =
		  mtx_shot[Snake.shottailhit[padnum]].tail_pattern;
		/* Erase the register of the shot tail  */
		EraseShotTail( Snake.shottailhit[padnum] ); 
	    }
	}
	ShotTailMove();		/* Move if there are any shot tail  */

	/* Surplus to next texture number at each specifying retrace (0 to 15) */
	if((MovieUpDate % MOVIE_UPDATE) == 0) {
	    if((++MovieNumber) > 15){
		MovieNumber = 0;
	    }
	}
	MovieUpDate++;
	/* Tempo up music if the rest of the time becomes less than 30 seconds  */
	if(UpTempo == 0 && (LapTime/1000) < 30){
	    UpTempo = 1;
	    tempo = nuAuSeqPlayerGetTempo(0);
	    tempo /= 13;
	    tempo *= 10;
	    nuAuSeqPlayerSetTempo(0,tempo);
	}
	/* Count down when the rest of the time is less than 30 seconds  */
	if(((LapTime/1000) < 32) && ((LapTime/1000) > 1)){
	    if(((LapTime/100) % 10) == 0){
		nuAuSndPlayerPlay(EFF_TIME_DWN1);
		nuAuSndPlayerSetPitch(44100.0/32000);
	    }
	}
	if((LapTime/1000) == 1){
	    if(((LapTime/100) % 10) == 0){
		nuAuSndPlayerPlay(EFF_TIME_DWN2);
		nuAuSndPlayerSetPitch(44100.0/32000);
	    }
	}
    }
}

/*----------------------------------------------*/
/*  The initialization of various parameters	*/
/*  IN:	None						*/
/*  RET:	None						*/
/*----------------------------------------------*/
void
InitSnake( void )
{
    int idx, idx2;
    /* Initialization of self-machine (for the amount of parameters vary for each player) */
    for(idx = 0; idx < SNAKE_MAX_LEN + 1; idx++){
	/* The position of player 1 */
	Snake.pos_x[0][idx] = -BASE_MAX_SIZE+50;
	Snake.pos_y[0][idx] = BASE_MAX_SIZE-50;
	Snake.pos_z[0][idx] = 30.0;
	/* The head direction */
	Snake.head_dir[0] = 0.0;

	/* The position of player 1 */
	Snake.pos_x[1][idx] = -BASE_MAX_SIZE+50;
	Snake.pos_y[1][idx] = -BASE_MAX_SIZE+50;
	Snake.pos_z[1][idx] = 30.0;
	/* The head direction */
	Snake.head_dir[1] = 90.0;

	/* The position of player 1 */
	Snake.pos_x[2][idx] = BASE_MAX_SIZE-50;
	Snake.pos_y[2][idx] = BASE_MAX_SIZE-50;
	Snake.pos_z[2][idx] = 30.0;
	/* The head direction */
	Snake.head_dir[2] = 270.0;

	/* The position of player 1 */
	Snake.pos_x[3][idx] = BASE_MAX_SIZE-50;
	Snake.pos_y[3][idx] = -BASE_MAX_SIZE+50;
	Snake.pos_z[3][idx] = 30.0;
	/* The head direction */
	Snake.head_dir[3] = 180.0;
    }
    /* Positions of the Z-axis are all common */
    Snake.pos_z[0][0] = 40.0;
    Snake.pos_z[1][0] = 40.0;
    Snake.pos_z[2][0] = 40.0;
    Snake.pos_z[3][0] = 40.0;

    for(idx = 0; idx < SNAKE_MAX; idx++){
	for(idx2 = 0; idx2 < SNAKE_MAX_LEN + 1; idx2++){
	    /* The direction of the snake (the same direction as that of the head) */
	    Snake.rot_x[idx][idx2] = 0.0;
	    Snake.rot_y[idx][idx2] = 0.0;
	    Snake.rot_z[idx][idx2] = Snake.head_dir[idx];

	    /* The drawing size */
	    Snake.size_x[idx][idx2] = 2.0;
	    Snake.size_y[idx][idx2] = 2.0;
	    Snake.size_z[idx][idx2] = 2.0;
	}
    }
    
    for(idx = 0; idx < SNAKE_MAX; idx++){
	Snake.tailcnt[idx] = 5;
	Snake.wallhit[idx] = 0;
	Snake.objhit[idx] = -1;
	Snake.enemyhit[idx] = -1;
	Snake.bufidx[idx] = TAIL_MOVE_BUF-1;
	Snake.enemytailhit[idx] = -1;
	Snake.enemytailhitnum[idx] = -1;
	Snake.sleepcnt[idx] = 0;
    }

    /* Setting the kind of the tail */
    for(idx = 1; idx < 6; idx++){
	Snake.tail_attr[0][idx] = 0;
	Snake.tail_attr[1][idx] = 1;
	Snake.tail_attr[2][idx] = 2;
	Snake.tail_attr[3][idx] = 3;
    }

    /* The initialization of the buffer for drawing the body of the self-machine */
    /* Set the default position, first */
    for(idx2 = 0; idx2 < 4; idx2++){
	for(idx = 0; idx < TAIL_MOVE_BUF; idx++){
	    snakemove[idx2].SnakeX[idx] = Snake.pos_x[idx2][0];
	    snakemove[idx2].SnakeY[idx] = Snake.pos_y[idx2][0];
	    snakemove[idx2].SnakeRotZ[idx] = Snake.rot_z[idx2][0];
	}
    }
}


/*----------------------------------------------*/
/*  The initialization of the shooting tail	*/
/*  IN:	None						*/
/*  RET:	None						*/
/*----------------------------------------------*/
void
InitShotTail( void )
{
    int idx;

    for( idx = 0; idx < 20; idx++ ){
	/* The position */
	mtx_shot[idx].pos_x = 0.0;
	mtx_shot[idx].pos_y = 0.0;
	mtx_shot[idx].pos_z = 20.0;
	
	/* The rotation */
	mtx_shot[idx].rot_x = 0.0;
	mtx_shot[idx].rot_y = 0.0;
	mtx_shot[idx].rot_z = 0.0;
	
	/* The size  */
	mtx_shot[idx].size_x = 2.0;
	mtx_shot[idx].size_y = 2.0;
	mtx_shot[idx].size_z = 2.0;
	
	/* The kind */
	mtx_shot[idx].tail_pattern = -1;
	
	/* The direction */
	mtx_shot[idx].tail_dir = 0.0;

	/* The status */
	mtx_shot[idx].status = TAIL_NOTHING;

	/* The one who shot */
	mtx_shot[idx].user = -1;
    }
}
/*----------------------------------------------------*/
/*  Entry of the tail for shooting 				*/
/*  IN:	PadNum	The registering player number	*/
/*  		TailAttr	The kind of registering tail 	*/
/*  RET:	None							*/
/*----------------------------------------------------*/
void
EntryShotTail( int PadNum, int TailAttr )
{
    int idx;

    for(idx = 0; idx < 20; idx++){
	if(mtx_shot[idx].tail_pattern == -1){
	    mtx_shot[idx].pos_x =
	      Snake.pos_x[PadNum][0] + cosf(2.0*M_PI/(360/Snake.head_dir[PadNum]))*HEAD_SIZE;
	    mtx_shot[idx].pos_y =
	      Snake.pos_y[PadNum][0] + sinf(2.0*M_PI/(360/Snake.head_dir[PadNum]))*HEAD_SIZE;

	    /* Don't enter if the entry location is out of range of the map */
	    /* Don't enter if the entry location is on the map object  */
	    if(ShotTail2Wall( mtx_shot[idx].pos_x,mtx_shot[idx].pos_y ) ||
	       ShotTail2Obj( mtx_shot[idx].pos_x,mtx_shot[idx].pos_y ) != -1){
		return;
	    }

	    mtx_shot[idx].pos_z = Snake.pos_z[PadNum][0];
	    mtx_shot[idx].tail_pattern = TailAttr;

	    mtx_shot[idx].rot_x = Snake.rot_x[PadNum][0];
	    mtx_shot[idx].rot_y = Snake.rot_y[PadNum][0];
	    mtx_shot[idx].rot_z = Snake.rot_z[PadNum][0];

	    mtx_shot[idx].tail_dir = Snake.head_dir[PadNum];
	    mtx_shot[idx].status = TAIL_MOVING;
	    mtx_shot[idx].user = PadNum;

	    SlideTail( PadNum );
	    if((--Snake.tailcnt[PadNum]) < 0){
		Snake.tailcnt[PadNum] = 0;
	    }
	    return;
	}
    }
}

/*----------------------------------------------*/
/*  Erase the register of the tail for shooting */
/*  IN:	ShotTailNum	Registered tail number 	*/
/*  RET:	None						*/
/*----------------------------------------------*/
void
EraseShotTail( int ShotTailNum )
{
    mtx_shot[ShotTailNum].pos_x = 0.0;
    mtx_shot[ShotTailNum].pos_y = 0.0;
    mtx_shot[ShotTailNum].pos_z = 20.0;
    
    mtx_shot[ShotTailNum].rot_x = 0.0;
    mtx_shot[ShotTailNum].rot_y = 0.0;
    mtx_shot[ShotTailNum].rot_z = 0.0;
    
    mtx_shot[ShotTailNum].size_x = 2.0;
    mtx_shot[ShotTailNum].size_y = 2.0;
    mtx_shot[ShotTailNum].size_z = 2.0;
    
    mtx_shot[ShotTailNum].tail_pattern = -1;
    
    mtx_shot[ShotTailNum].tail_dir = 0.0;

    /* The status */
    mtx_shot[ShotTailNum].status = TAIL_NOTHING;
}

/*----------------------------------------*/
/*  Rotate the tail of the self-machine 	*/
/*  IN:	PadNum 	Player number	*/
/*  RET:	None					*/
/*----------------------------------------*/
void
SlideTail( int PadNum )
{
    int idx;
    if(Snake.tailcnt[PadNum] >= 2){
	for(idx = 1; idx < Snake.tailcnt[PadNum]+1; idx++){
	    Snake.tail_attr[PadNum][idx] = Snake.tail_attr[PadNum][idx+1];
	}
    }
}

/*----------------------------*/
/*  Move of the shot tail 	*/
/*  IN:	None			*/
/*  RET:	None			*/
/*----------------------------*/
void
ShotTailMove( void )
{
    int idx;
    float ChkX,ChkY;

    for(idx = 0; idx < 20; idx++){
	if( mtx_shot[idx].tail_pattern == -1){
	    continue;
	}
	/* Get the vector of the shot tail from the head direction */
	ChkX = cosf(2.0*M_PI/(360/mtx_shot[idx].tail_dir))*SHOT_SPEED;
	ChkY = sinf(2.0*M_PI/(360/mtx_shot[idx].tail_dir))*SHOT_SPEED;

	/* Move it practically */
	mtx_shot[idx].pos_x += ChkX;
	mtx_shot[idx].pos_y += ChkY;
    
	/* Hit the wall or obstacle */
	if(ShotTail2Wall( mtx_shot[idx].pos_x,mtx_shot[idx].pos_y ) ||
	   ShotTail2Obj( mtx_shot[idx].pos_x,mtx_shot[idx].pos_y ) != -1){
	    /* Return the beginning */
	    mtx_shot[idx].pos_x -= ChkX;
	    mtx_shot[idx].pos_y -= ChkY;
	    mtx_shot[idx].status = TAIL_WAITING;
	}
	/* Make the shot tail rotate */
	mtx_shot[idx].rot_z += 10.0;
    }
}


/*----------------------------------------*/
/*  Display the self-machine (the head)	*/
/*  IN:	dp	The dynamic pointer 	*/
/*      Padnum	The player number 	*/
/*  RET:	None					*/
/*----------------------------------------*/
void
DrawHead( Dynamic* dp, int PadNum )
{
    /* The calculation of the model coordinate system */
    guTranslate(&(dp->snake_trans[PadNum][0]),
		Snake.pos_x[PadNum][0],
		Snake.pos_y[PadNum][0],
		Snake.pos_z[PadNum][0]);
    guScale(&(dp->snake_scale[PadNum][0]),
	    Snake.size_x[PadNum][0],
	    Snake.size_y[PadNum][0],
	    Snake.size_z[PadNum][0]);

    /* On fainting */
    if((--Snake.sleepcnt[PadNum]) > 0){
	if(0x2 > (nuScRetraceCounter & 0x2)) {
	    Snake.rot_y[PadNum][0] += 3.0;
	}else{
	    Snake.rot_y[PadNum][0] -= 3.0;
	}
    }else{
	Snake.sleepcnt[PadNum] = 0;
	Snake.rot_y[PadNum][0] = 0.0;
    }

    guRotateRPY(&(dp->snake_rotate[PadNum][0]),
		Snake.rot_x[PadNum][0]+90.0,
		Snake.rot_y[PadNum][0],
		Snake.rot_z[PadNum][0]+90.0);

    /* Setting the model-matrix */

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->snake_trans[PadNum][0])),
	      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->snake_scale[PadNum][0])),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->snake_rotate[PadNum][0])),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    /* No anti-ailiasing, the opaque surface */
    gDPSetRenderMode(glistp++,G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

    /* Clear and set up of geometry */
    gSPClearGeometryMode(glistp++,0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_LIGHTING |
		       G_CULL_BACK);

    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
    /* Setting the combiner */
    gDPSetCombineMode(glistp++,G_CC_DECALRGB, G_CC_DECALRGB);
    
    /* The display list of the model */
    switch(PadNum){
      case 0:
	gSPDisplayList(glistp++,kiyaata1_mdl_model0);
	break;
      case 1:
	gSPDisplayList(glistp++,kiyaata2_mdl_model0);
	break;
      case 2:
	gSPDisplayList(glistp++,kiyaata3_mdl_model0);
	break;
      case 3:
	gSPDisplayList(glistp++,kiyaata4_mdl_model0);
	break;
      default:
	break;
    }
	 
    gDPPipeSync(glistp++);
}


/*----------------------------------------*/
/*  Display the self-machine (the tail)	*/
/*  IN:	dp	The dynamic pointer 	*/
/*      Padnum	The player number 	*/
/*  RET:	None					*/
/*----------------------------------------*/
void
DrawTail( Dynamic* dp, int PadNum )
{
    int MaxTail,idx;

    MaxTail = Snake.tailcnt[PadNum];

    /* Draw from the most rear tail */
    for(idx = MaxTail; idx >= 1; idx--){
	/* The calculation of the model coordinate system */
	guTranslate(&(dp->snake_trans[PadNum][idx]),
		    Snake.pos_x[PadNum][idx],
		    Snake.pos_y[PadNum][idx],
		    Snake.pos_z[PadNum][idx]
		    );
	guScale(&(dp->snake_scale[PadNum][idx]),
		Snake.size_x[PadNum][idx],
		Snake.size_y[PadNum][idx],
		Snake.size_z[PadNum][idx]
		);

	guRotateRPY(&(dp->snake_rotate[PadNum][idx]),
		    Snake.rot_x[PadNum][idx]+90.0,
		    Snake.rot_y[PadNum][idx],
		    Snake.rot_z[PadNum][idx]+90.0
		    );

	
	/* Setting the model-matrix */
	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->snake_trans[PadNum][idx])),
		  G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->snake_scale[PadNum][idx])),
		  G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

	gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->snake_rotate[PadNum][idx])),
		  G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

	/* No anti-aliasing, the opaque surface */
	gDPSetRenderMode(glistp++,G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

	/* Clear and set up the geometry */
	gSPClearGeometryMode(glistp++,0xFFFFFFFF);
	gSPSetGeometryMode(glistp++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH |
			   G_LIGHTING | G_CULL_BACK);

	gDPSetCycleType(glistp++, G_CYC_1CYCLE);
	/* Setting the combiner */
	gDPSetCombineMode(glistp++,G_CC_DECALRGB, G_CC_DECALRGB);

	/* The display list of the model */
	switch(Snake.tail_attr[PadNum][idx]){
	  case 0:
	    gSPDisplayList(glistp++,kiyadou1_mdl_model0);
	    break;
	  case 1:
	    gSPDisplayList(glistp++,kiyadou2_mdl_model0);
	    break;
	  case 2:
	    gSPDisplayList(glistp++,kiyadow3_mdl_model0);
	    break;
	  case 3:
	    gSPDisplayList(glistp++,kiyadow4_mdl_model0);
	    break;
	  default:
	    break;
	}
	gDPPipeSync(glistp++);
    }
}

/*----------------------------------------*/
/*  Record the head locus in the buffer	*/
/*  IN: Padnum	The player number 	*/
/*  RET:	None					*/
/*----------------------------------------*/
void
RecordHeadPos( int PadNum )
{
    snakemove[PadNum].SnakeX[Snake.bufidx[PadNum]] = Snake.pos_x[PadNum][0];
    snakemove[PadNum].SnakeY[Snake.bufidx[PadNum]] = Snake.pos_y[PadNum][0];
    snakemove[PadNum].SnakeRotZ[Snake.bufidx[PadNum]] = Snake.rot_z[PadNum][0];

    if((--Snake.bufidx[PadNum]) < 0){
	Snake.bufidx[PadNum] = TAIL_MOVE_BUF-1;
    }
}

/*----------------------------------------------*/
/*  Move of the tail (chase the head locus) 	*/
/*  IN: Padnum	The player number		 		*/
/*  RET:	None								*/
/*----------------------------------------------*/
void
UpdateTailPos( int PadNum )
{
    int MaxTail,idx;

    MaxTail = Snake.tailcnt[PadNum];

    for(idx = 1; idx <= MaxTail; idx++){
	if((Snake.bufidx[PadNum] + SUB*idx) > TAIL_MOVE_BUF-1 ){
	    Snake.bufidx_wk[PadNum] = Snake.bufidx[PadNum] + SUB*idx - TAIL_MOVE_BUF;
	}else{
	    Snake.bufidx_wk[PadNum] = Snake.bufidx[PadNum] + SUB*idx;
	}
 
	Snake.pos_x[PadNum][idx] = snakemove[PadNum].SnakeX[Snake.bufidx_wk[PadNum]];
	Snake.pos_y[PadNum][idx] = snakemove[PadNum].SnakeY[Snake.bufidx_wk[PadNum]];
	Snake.rot_z[PadNum][idx] = snakemove[PadNum].SnakeRotZ[Snake.bufidx_wk[PadNum]];
    }
}


/*----------------------------------------*/
/*  Display the shot tail			*/
/*  IN:	dp	The dynamic pointer	*/
/*  RET:	None					*/
/*----------------------------------------*/
void
DrawShotTail( Dynamic* dp )
{
    int idx;

    for(idx = 0; idx < 20; idx++){
	if(mtx_shot[idx].tail_pattern == -1){
	    continue;
	}else{
	    guTranslate(&(dp->shot_trans[idx]),
			mtx_shot[idx].pos_x,
			mtx_shot[idx].pos_y,
			mtx_shot[idx].pos_z
			);
		    
	    guScale(&(dp->shot_scale[idx]),
		    mtx_shot[idx].size_x,
		    mtx_shot[idx].size_y,
		    mtx_shot[idx].size_z
		    );

	    guRotateRPY(&(dp->shot_rotate[idx]),
			mtx_shot[idx].rot_x+90.0,
			mtx_shot[idx].rot_y,
			mtx_shot[idx].rot_z
			);

	
	    /* Setting the model-matrix */
	    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->shot_trans[idx])),
		      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->shot_rotate[idx])),
		      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

	    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dp->shot_scale[idx])),
		      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

	    /* No anti-aliasing, the opaque surface */
	    gDPSetRenderMode(glistp++,G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

	    /* Clear and set up the geometry */
	    gSPClearGeometryMode(glistp++,0xFFFFFFFF);
	    gSPSetGeometryMode(glistp++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH |
			       G_LIGHTING | G_CULL_BACK);

	    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
	    /* Setting the combiner */
	    gDPSetCombineMode(glistp++,G_CC_DECALRGB, G_CC_DECALRGB);

	    /* The display list of the model */
	    switch(mtx_shot[idx].tail_pattern){
	      case 0:
		gSPDisplayList(glistp++,kiyadou1_mdl_model0);
		break;
	      case 1:
		gSPDisplayList(glistp++,kiyadou2_mdl_model0);
		break;
	      case 2:
		gSPDisplayList(glistp++,kiyadow3_mdl_model0);
		break;
	      case 3:
		gSPDisplayList(glistp++,kiyadow4_mdl_model0);
		break;
	      default:
		break;
	    }
	    gDPPipeSync(glistp++);
	}
    }
}


/*----------------------------------------------------------------------------------*/
/*  Make the head turn to the direction of the 3D stick (the units of 90 degree)	*/
/*  IN:	PadNum	The registering player number 					*/
/*  RET:	None												*/
/*----------------------------------------------------------------------------------*/
void
ChgHeadDir( int PadNum, int stickx, int sticky )
{
    Snake.head_dir[PadNum] = Atan2f((float)sticky,
				    (float)stickx);

    if(stickx < 0 && sticky == 0){
	Snake.head_dir[PadNum] += 180.0;
    }
    Snake.rot_z[PadNum][0] = Snake.head_dir[PadNum];
}

/*----------------------------------------------*/
/*  Read the movie data 				*/
/*  IN: MovieNum	The scene number of the movie */
/*      dp		The dynamic pointer 		*/
/*  RET:	None						*/
/*----------------------------------------------*/
void
ReadMovie( int MovieNum, Dynamic* dp )
{
    u32 r2rlen;
    u32 onecutlen;

    /* Compute the stored segment heads of the movie data */
    r2rlen = _moviedataSegmentRomEnd - _moviedataSegmentRomStart;

    /* Compute the length of each scene */
    onecutlen = r2rlen / 16;

    /* Copy data from ROM to RDRAM, practically */
    nuPiReadRom( (u32)(_moviedataSegmentRomStart + onecutlen * MovieNum),
		(void *)&(dp->MovieBuf[0]),
		onecutlen
		);
}

/*------------------------------*/
/*  Display screen information  */
/*  IN:	None			  */
/*  RET:	None			  */
/*------------------------------*/
void
DrawInfo( void )
{
    sprintf(outstring,"TIME:%03llu", LapTime/1000);
    Draw8Font(20,20, TEX_COL_WHITE, 0);

    if(ReplayMode){
	sprintf(outstring,"NOW PLAYING TRACE DATA");
	Draw8Font(20,30, TEX_COL_WHITE, 0);
    }

    if(Pause){
	if(0x30 > (nuScRetraceCounter & 0x30)){
	    sprintf(outstring,"PAUSE");
	    Draw8Font(141,111, TEX_COL_BLACK, 0);

 	    sprintf(outstring,"PAUSE");
	    Draw8Font(140,110, TEX_COL_GREEN, 0);
	}
    }
}


/*----------------------------------------------------------------------------------*/
/* Count the number of tails of each snake (for the judge of the game end)		*/
/*  IN:	 ChkStat	Checking contents (for time-up, all got tails)			*/
/*  RET: Result	-1:A tie game	0:The normal game end	1:Win with perfect 	*/
/*     : Outside work Winner	The winner number 						*/
/*----------------------------------------------------------------------------------*/
int
CheckTailNum( int ChkStat )
{
    int idx, TailNumWk = 0;

    if(ChkStat == TIME_UP){
	if(Snake.tailcnt[0] == 5 &&
	   Snake.tailcnt[1] == 5 &&
	   Snake.tailcnt[2] == 5 &&
	   Snake.tailcnt[3] == 5){
	    Winner = -1;
	    return(-1);
	}
	for(idx = 0; idx < 4; idx++){
	    if(TailNumWk <  Snake.tailcnt[idx]){
		TailNumWk = Snake.tailcnt[idx];
		Winner = idx;
	    }
	}
	return(0);
    }else{
	for(idx = 0; idx < 4; idx++){
	    if(Snake.tailcnt[idx] == 20){
		Winner = idx;
		return(1);
	    }
	}
	return(0);
    }
}

/*----------------------------------------------------*/
/* Each initialization of parts of the main game 	*/
/*  IN:	None							*/
/*  RET:	None							*/
/*----------------------------------------------------*/
void
InitGame( void )
{
    InitSnake();
    InitShotTail();
    View = 0.0;
    MovieNumber = 0;
    StartTime = osGetTime();
    NowTime = osGetTime();
    NowPauseTime = 0;
    Pause = 1;
    TraceIdx = 0;
    UpTempo = 0;
}
