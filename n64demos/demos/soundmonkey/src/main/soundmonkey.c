
/*---------------------------------------------------------------------*
        Copyright (C) 1998 Nintendo. (Originated by SGI)
        
        $RCSfile: soundmonkey.c,v $
        $Revision: 1.40 $
        $Date: 1999/05/24 07:16:20 $
 *---------------------------------------------------------------------*/
/* sound player test */

#include <ultra64.h>
#include <assert.h>
#include <PR/ramrom.h>
#include "soundmonkey.h"

#define ACTIVE_SOUNDS   1
#define MAX_PITCH       1
#define MAX_INT         0x7fffffff
#define MAX_TRIES       20

OSPiHandle	*handler;

static s32 nActiveSounds(ALSndPlayer *sndp, ALInstrument  *inst, ALSndId *id);
static s32 myRandom(s32 min, s32 max);

#define MAX_VOICES      4
#define MAX_UPDATES     64
#define DMA_QUEUE_SIZE  50
#define EVT_COUNT       32
#define EXTRA_SAMPLES   80
#define OUTPUT_RATE     44100
#define NUM_FIELDS      1
#define MAX_AUDIO_LENGTH 2048
#define MAX_CLIST_SIZE  4096
#define FX_BUFFER_SIZE  8192
#define AUDIO_HEAP_SIZE 320000

/*
 * Symbol generated by "makerom" to indicate the end of the code segment
 * in virtual (and physical) memory
 */
extern u8 _seqSegmentRomStart[], _seqSegmentRomEnd[];
extern u8 _sfxbankSegmentRomStart[], _sfxbankSegmentRomEnd[];
extern u8 _sfxtableSegmentRomStart[], _sfxtableSegmentRomEnd[];
extern u8 _midibankSegmentRomStart[], _midibankSegmentRomEnd[];
extern u8 _miditableSegmentRomStart[], _miditableSegmentRomEnd[];

/*
 * Stacks for the threads as well as message queues for synchronization
 */

u64 	bootStack[STACKSIZE/8];

static void	mainproc(u8 *);
static void	gameproc(u8 *);

static OSThread	mainThread;
static u64	mainThreadStack[STACKSIZE/8];

static OSThread	gameThread;
static u64	gameThreadStack[STACKSIZE/8];

/*
 * For PI manager
 */
static OSMesg PiMessages[DMA_QUEUE_SIZE];
static OSMesgQueue PiMessageQ;

static OSMesgQueue	dmaMessageQ, taskMessageQ, retraceMessageQ;
static OSMesg		dmaMessageBuf[DMA_QUEUE_SIZE], taskMessageBuf, retraceMessageBuf;
static OSIoMesg	        dmaIOMessageBuf;
static ALHeap           hp;

static OSTask           *tlist[2];               /* global task list      */

static ALGlobals        g;
static ALSynConfig      c;
static ALSndpConfig     SPConfig;
static u8               audioHeap[AUDIO_HEAP_SIZE];

static OSIoMesg	        dmaIOMesgBuf[DMA_QUEUE_SIZE];
static s32              curBuf = 0;
static s32              curAudioBuf = 1;

/*
 * Double buffered storage for wavetable data
 */
static Acmd	        *cmdList[2];
static s16              audioSamples[3] = {0, 0, 0};
static s16              *audioBuffer[3];

/*
 * DMA Call Back Routine
 */

#define NBUFFERS            16
#define MAX_BUFFER_LENGTH 1024

typedef struct
{
    ALLink      node;
    int         startAddr;
    u32         lastFrame;
    char        *ptr;
} DMABuffer;

typedef struct
{
    u8          initialized;
    DMABuffer   *firstUsed;
    DMABuffer   *firstFree;

} DMAState;


DMAState    dmaState;
DMABuffer   dmaBuffs[NBUFFERS];
static s32  nextDMA = 0;
static u32  gFrameCt = 0;

    
s32 dmaCallBack(s32 addr, s32 len, void *state)
{
    void        *freeBuffer;
    int         delta;
    DMABuffer   *dmaPtr,*lastDmaPtr;
    s32         addrEnd,buffEnd;


    lastDmaPtr = 0;
    dmaPtr = dmaState.firstUsed;
    addrEnd = addr + len;


    while(dmaPtr)  /* see if buffer is already set up */
    {

        buffEnd = dmaPtr->startAddr + MAX_BUFFER_LENGTH;
        if(dmaPtr->startAddr > addr) /* since buffers are ordered */
            break;                   /* abort if past possible */

        else if(addrEnd <= buffEnd) /* yes, found one */
        {
            dmaPtr->lastFrame = gFrameCt; /* mark it used */
            freeBuffer = dmaPtr->ptr + addr - dmaPtr->startAddr;
            return (int) osVirtualToPhysical(freeBuffer);
        }
        lastDmaPtr = dmaPtr;
        dmaPtr = (DMABuffer*)dmaPtr->node.next;
    }

    /* get here, and you didn't find a buffer, so dma a new one */

    /* get a buffer from the free list */
    dmaPtr = dmaState.firstFree;

#ifdef ULTRA_DEBUG
#ifndef __MWERKS__
    assert(dmaPtr);  /* be sure you have a buffer */
#endif
#endif

    dmaState.firstFree = (DMABuffer*)dmaPtr->node.next;
    alUnlink((ALLink*)dmaPtr);


    /* add it to the used list */
    if(lastDmaPtr) /* normal procedure */
    {
        alLink((ALLink*)dmaPtr,(ALLink*)lastDmaPtr);
    }
    else if(dmaState.firstUsed) /* jam at begining of list */
    {
        lastDmaPtr = dmaState.firstUsed;
        dmaState.firstUsed = dmaPtr;
        dmaPtr->node.next = (ALLink*)lastDmaPtr;
        dmaPtr->node.prev = 0;
        lastDmaPtr->node.prev = (ALLink*)dmaPtr;
    }
    else /* no buffers in list, this is the first one */
    {
        dmaState.firstUsed = dmaPtr;
        dmaPtr->node.next = 0;
        dmaPtr->node.prev = 0;
    }


    freeBuffer = dmaPtr->ptr;
    delta = addr & 0x1;
    addr -= delta;
    dmaPtr->startAddr = addr;
    dmaPtr->lastFrame = gFrameCt;  /* mark it */

    dmaIOMesgBuf[nextDMA].hdr.pri      = OS_MESG_PRI_NORMAL;
    dmaIOMesgBuf[nextDMA].hdr.retQueue = &dmaMessageQ;
    dmaIOMesgBuf[nextDMA].dramAddr     = freeBuffer;
    dmaIOMesgBuf[nextDMA].devAddr      = (u32)addr;
    dmaIOMesgBuf[nextDMA].size         = MAX_BUFFER_LENGTH;

    osEPiStartDma(handler, &dmaIOMesgBuf[nextDMA++], OS_READ);

    return (int) osVirtualToPhysical(freeBuffer) + delta;
}


ALDMAproc dmaNew(DMAState **state)
{
    int         i;

    if(!dmaState.initialized)  /* only do this once */
    {
        dmaState.firstFree = &dmaBuffs[0];
        for (i=0; i<NBUFFERS-1; i++)
        {
            alLink((ALLink*)&dmaBuffs[i+1],(ALLink*)&dmaBuffs[i]);
            dmaBuffs[i].ptr = alHeapAlloc(&hp, 1, MAX_BUFFER_LENGTH);
        }
	dmaBuffs[i].ptr = alHeapAlloc(&hp, 1, MAX_BUFFER_LENGTH);

        dmaState.initialized = 1;
    }

    *state = &dmaState;  /* state is never used in this case */

    return dmaCallBack;
}


void CleanDMABuffs(void)
{
    DMABuffer  *dmaPtr,*nextPtr;

    dmaPtr = dmaState.firstUsed;
    while(dmaPtr)
    {
        nextPtr = (DMABuffer*)dmaPtr->node.next;

        /* Can change this value.  Should be at least one.  */
        /* Larger values mean more buffers needed, but fewer DMA's */

        if(dmaPtr->lastFrame + 2  < gFrameCt) /* remove from used list */
	{
            if(dmaState.firstUsed == dmaPtr)
                dmaState.firstUsed = (DMABuffer*)dmaPtr->node.next;
            alUnlink((ALLink*)dmaPtr);
            if(dmaState.firstFree)
                alLink((ALLink*)dmaPtr,(ALLink*)dmaState.firstFree);
            else
	    {
                dmaState.firstFree = dmaPtr;
                dmaPtr->node.next = 0;
                dmaPtr->node.prev = 0;
	    }
	}
        dmaPtr = nextPtr;
    }
}


void boot(void *arg)
{

    osInitialize();
  
    handler = osCartRomInit();

    osCreateThread(&mainThread, 1, (void(*)(void *))mainproc, arg,
		  ((u8 *) mainThreadStack+STACKSIZE), 10);
    osStartThread(&mainThread);
}

static void mainproc(u8 *argv) 
{
    osCreateThread(&gameThread, 3, (void(*)(void *))gameproc, argv,
                   ((u8 *) gameThreadStack) + STACKSIZE, 10);
    /* Initialize video */

    osCreateViManager(OS_PRIORITY_VIMGR);
    osViSetMode(&osViModeTable[OS_VI_NTSC_LAN1]);

    /*
     * Start PI Mgr for access to cartridge - start before debugger?
     */
    osCreatePiManager((OSPri) 150, &PiMessageQ, PiMessages, DMA_QUEUE_SIZE);

    osStartThread(&gameThread);

    /*
     * Become the idle thread
     */
    osSetIntMask( OS_IM_ALL );
    osSetThreadPri( 0, 0 );
    for (;;);

}

static
void gameproc(u8 *argv)
{

    ALSndPlayer
        Sndp,
        *sndp = &Sndp;
    
    ALSound
        *snd;
    
    ALSeqPlayer
        sequencePlayer,
        *seqp = &sequencePlayer;
    
    ALSeqpConfig
        seqc;
    
    ALBank
        *midiBank,
        *sfxBank;
    
    ALSeq
        sequence,
        *seq = &sequence;

    ALSeqFile
        *sfile;
    
    s32
        nTries = 0,
        buf,
        frame=0,
        clcount,
        bankLen,
        len,
        seqNo = 0,
        samplesLeft = 0,
        frameSize,
        minFrameSize,
        numAllocated,
        i,
        j;
    
    u8
        *ptr,
        pan = 0,
        fxmix = 0,
        *seqPtr,
        *midiBankPtr,
        *sfxBankPtr;
    
    ALInstrument
        *inst;

    OSTask
        *tlistp;
    
    Acmd
        *cmdlp;

    s16
        *audioOp,
        vol;
    f32
        fsize,
        pitch;

    ALSndId
        *idPtr;
    
    /*
     * Message queue for PI manager return messages
     */
    osCreateMesgQueue(&dmaMessageQ, dmaMessageBuf, DMA_QUEUE_SIZE);

    osCreateMesgQueue(&taskMessageQ, &taskMessageBuf, 1);
    osSetEventMesg(OS_EVENT_SP, &taskMessageQ, NULL);

    /*
     * Prime the task queue
     */
    osSendMesg(&taskMessageQ, NULL, OS_MESG_BLOCK);

    osCreateMesgQueue(&retraceMessageQ, &retraceMessageBuf, 1);
    osViSetEvent(&retraceMessageQ, NULL, NUM_FIELDS);

    alHeapInit(&hp, audioHeap, AUDIO_HEAP_SIZE);

    /*
     * Create command list buffers
     */
    cmdList[0] = alHeapAlloc(&hp, 1, MAX_CLIST_SIZE*sizeof(Acmd));
    cmdList[1] = alHeapAlloc(&hp, 1, MAX_CLIST_SIZE*sizeof(Acmd));
    tlist[0] = alHeapAlloc(&hp, 1, sizeof(OSTask));
    tlist[1] = alHeapAlloc(&hp, 1, sizeof(OSTask));
    audioBuffer[0] = alHeapAlloc(&hp, 1, sizeof(s32)*MAX_AUDIO_LENGTH);
    audioBuffer[1] = alHeapAlloc(&hp, 1, sizeof(s32)*MAX_AUDIO_LENGTH);
    audioBuffer[2] = alHeapAlloc(&hp, 1, sizeof(s32)*MAX_AUDIO_LENGTH);
    
    /*
     * Copy over the midi bank and the special effects bank
     */
    bankLen = _midibankSegmentRomEnd - _midibankSegmentRomStart;
    midiBankPtr = alHeapAlloc(&hp, 1, bankLen);
    osWritebackDCacheAll();

    dmaIOMessageBuf.hdr.pri      = OS_MESG_PRI_NORMAL;
    dmaIOMessageBuf.hdr.retQueue = &dmaMessageQ;
    dmaIOMessageBuf.dramAddr     = midiBankPtr;
    dmaIOMessageBuf.devAddr      = (u32)_midibankSegmentRomStart;
    dmaIOMessageBuf.size         = bankLen;

    osEPiStartDma(handler, &dmaIOMessageBuf, OS_READ);
    (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);

    bankLen = _sfxbankSegmentRomEnd - _sfxbankSegmentRomStart;
    sfxBankPtr = alHeapAlloc(&hp, 1, bankLen);
    osWritebackDCacheAll();

    dmaIOMessageBuf.hdr.pri      = OS_MESG_PRI_NORMAL;
    dmaIOMessageBuf.hdr.retQueue = &dmaMessageQ;
    dmaIOMessageBuf.dramAddr     = sfxBankPtr;
    dmaIOMessageBuf.devAddr      = (u32)_sfxbankSegmentRomStart;
    dmaIOMessageBuf.size         = bankLen;

    osEPiStartDma(handler, &dmaIOMessageBuf, OS_READ);
    (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
    
    /*
     * Copy over the sequence file header
     */
    sfile = alHeapAlloc(&hp, 1, 4);
    osWritebackDCacheAll();

    dmaIOMessageBuf.hdr.pri      = OS_MESG_PRI_NORMAL;
    dmaIOMessageBuf.hdr.retQueue = &dmaMessageQ;
    dmaIOMessageBuf.dramAddr     = sfile;
    dmaIOMessageBuf.devAddr      = (u32)_seqSegmentRomStart;
    dmaIOMessageBuf.size         = 8;

    osEPiStartDma(handler, &dmaIOMessageBuf, OS_READ);
    (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
    
    len = 4 + sfile->seqCount*sizeof(ALSeqData);
    sfile = alHeapAlloc(&hp, 1, 4 + sfile->seqCount*sizeof(ALSeqData));
    osWritebackDCacheAll();


    dmaIOMessageBuf.hdr.pri      = OS_MESG_PRI_NORMAL;
    dmaIOMessageBuf.hdr.retQueue = &dmaMessageQ;
    dmaIOMessageBuf.dramAddr     = sfile;
    dmaIOMessageBuf.devAddr      = (u32)_seqSegmentRomStart;
    dmaIOMessageBuf.size         = len;

    osEPiStartDma(handler, &dmaIOMessageBuf, OS_READ);
    (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);

    alSeqFileNew(sfile, _seqSegmentRomStart);
    if (seqNo > sfile->seqCount)
        seqNo = 0;

    /*
     * Copy over the requested sequence
     */
    ptr = sfile->seqArray[seqNo].offset;
    len = sfile->seqArray[seqNo].len;
    if (len & 0x1)
        len++;

    seqPtr = alHeapAlloc(&hp, 1, len);
    osWritebackDCacheAll();

    dmaIOMessageBuf.hdr.pri      = OS_MESG_PRI_NORMAL;
    dmaIOMessageBuf.hdr.retQueue = &dmaMessageQ;
    dmaIOMessageBuf.dramAddr     = seqPtr;
    dmaIOMessageBuf.devAddr      = (u32)ptr;
    dmaIOMessageBuf.size         = len;

    osEPiStartDma(handler, &dmaIOMessageBuf, OS_READ);
    (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);

    /*
     * Audio synthesizer initialization
     */
    /* Initialize the DACs */
    c.outputRate = osAiSetFrequency(OUTPUT_RATE);
    fsize = (f32) NUM_FIELDS * c.outputRate / (f32) 60;
    
    frameSize = (s32) fsize;
    if (frameSize < fsize)
        frameSize++;
    if (frameSize & 0xf)
        frameSize = (frameSize & ~0xf) + 0x10;
    minFrameSize = frameSize - 16;

    /*
     * initialize audio synthesizer
     */
    c.maxVVoices = MAX_VOICES;
    c.maxPVoices = MAX_VOICES;
    c.maxUpdates = MAX_UPDATES;
    c.dmaproc    = &dmaNew;
    c.fxType	 = AL_FX_SMALLROOM;
    c.heap       = &hp;

    alInit(&g, &c);

    /*
     * Initialize the sequence player
     */
    seqc.maxVoices      = MAX_VOICES;
    seqc.maxEvents      = EVT_COUNT;
    seqc.maxChannels    = 16;
    seqc.heap           = &hp;
    seqc.initOsc        = 0;
    seqc.updateOsc      = 0;
    seqc.stopOsc        = 0;

#ifdef DEBUG
    seqc.debugFlags     = NO_VOICE_ERR_MASK |NOTE_OFF_ERR_MASK | NO_SOUND_ERR_MASK;    
#endif

    alSeqpNew(seqp, &seqc);
    
    alSeqNew(seq, seqPtr, len);    

    alSeqpSetSeq(seqp, seq);

    alBnkfNew((ALBankFile *)midiBankPtr, _miditableSegmentRomStart);
    midiBank = ((ALBankFile *)midiBankPtr)->bankArray[0];
    alSeqpSetBank(seqp, midiBank);

    /*
     * Initialize the soundplayer
     */
    SPConfig.maxSounds = MAX_VOICES;
    SPConfig.maxEvents = EVT_COUNT;
    SPConfig.heap  = &hp;

    alSndpNew(sndp, &SPConfig);

    /*
     * Allocate all the sounds
     */
    alBnkfNew((ALBankFile *)sfxBankPtr, _sfxtableSegmentRomStart);
    sfxBank = ((ALBankFile *)sfxBankPtr)->bankArray[0];

    /*
     * Better make sure number of sounds is at least equal
     * to the number of effects
     */
    inst = sfxBank->instArray[0];
    idPtr = alHeapAlloc(&hp, 1, inst->soundCount*sizeof(ALSndId));
    for (i=0, numAllocated = 0; i<inst->soundCount; i++){
        snd = inst->soundArray[i];
        if ((idPtr[i] = alSndpAllocate(sndp, snd)) != -1)
            numAllocated++;
    }
    
    /*
     * Start up ACTIVE_SOUNDS sounds selected randomly
     */
    for (j=0; j<ACTIVE_SOUNDS; j++){
        i = myRandom(0,numAllocated-1);
        alSndpSetSound(sndp, idPtr[i]);
        pitch = 0.5;
        alSndpSetPitch(sndp, pitch);
        fxmix = 0;
        alSndpSetFXMix(sndp, fxmix);
        pan = 64;
        alSndpSetPan(sndp, pan);
        vol = 20000;
        alSndpSetVol(sndp, vol);
        alSndpPlay(sndp);
    }

    (void)osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);
    (void)osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);
    
    do  {
        if (seqp->state == AL_STOPPED){
            alSeqNew(seq, seqPtr, len);    
            alSeqpSetSeq(seqp, seq);
            alSeqpPlay(seqp);
        }
        frame++;

        tlistp = tlist[curBuf];
	cmdlp = cmdList[curBuf];       
        
        /*
         * How many samples I need
         */
        buf = curAudioBuf % 3;
        audioOp = (s16 *) osVirtualToPhysical(audioBuffer[buf]);
        audioSamples[buf] = 16 + (frameSize - samplesLeft + EXTRA_SAMPLES) & ~0xf;
        if (audioSamples[buf] < minFrameSize)
            audioSamples[buf] = minFrameSize;

        /*
         * Call the frame handler ask for samples 
         */
        cmdlp = alAudioFrame(cmdlp, &clcount, audioOp, audioSamples[buf]);

        /*
         * Check number of active sounds and start a new one
         * if its less than ACTIVE_SOUNDS
         */
        if (nActiveSounds(sndp, inst, idPtr) < ACTIVE_SOUNDS){
            do {
                i = myRandom(0,numAllocated-1);
                alSndpSetSound(sndp, idPtr[i]);
            } while (alSndpGetState(sndp) != AL_STOPPED);
            pitch = (f32) myRandom(200, 2000)/2000;;
            alSndpSetPitch(sndp, pitch);
            fxmix = myRandom(0, 127);
            alSndpSetFXMix(sndp, fxmix);
            pan = myRandom(0, 127);
            alSndpSetPan(sndp, pan);
            vol = myRandom(20000, 32000); vol = 20000;
            alSndpSetVol(sndp, vol);
            alSndpPlay(sndp);
        }

        /*
         * Randomly decide to stop a sound
         */
        if (myRandom(0, 1000) > 900){
            if (nActiveSounds(sndp, inst, idPtr) == ACTIVE_SOUNDS){
                nTries = 0;
                do {
                    if (nTries++ > MAX_TRIES)
                        break;
                    i = myRandom(0,numAllocated-1);
                    alSndpSetSound(sndp, idPtr[i]);
                } while (alSndpGetState(sndp) != AL_PLAYING);
                if (nTries < MAX_TRIES)
                    alSndpStop(sndp);
            }
        }
                
        /*
         * Build the audio task
         */       
        tlistp->t.type = M_AUDTASK;
        tlistp->t.flags = 0x0;
        tlistp->t.ucode_boot = (u64 *) rspbootTextStart;
	tlistp->t.ucode_boot_size = ((s32)rspbootTextEnd -
                                     (s32)rspbootTextStart);
        tlistp->t.ucode = (u64 *) aspMainTextStart;
        tlistp->t.ucode_data = (u64 *) aspMainDataStart;
	tlistp->t.ucode_size = 4096;
            tlistp->t.ucode_data_size = sizeof(u64)*(aspMainDataEnd - aspMainDataStart); 
	tlistp->t.data_ptr = (u64 *) cmdList[curBuf];
        tlistp->t.data_size = (cmdlp - cmdList[curBuf]) * sizeof(Acmd);

        /*
         * Video does nothing - just syncs up on the frame boundary.
         */
	(void)osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);

        /*
         * Find out how many samples left in the currently running
         * audio buffer
         */
        samplesLeft = IO_READ(AI_LEN_REG)>>2;
        
        /*
         * The last task should have finished before the frame message
         * so this just clears the message queue
         */
        (void)osRecvMesg(&taskMessageQ, NULL, OS_MESG_BLOCK);

        /*
         * Stick the task output in files - or to the DAC
         */
        buf = (curAudioBuf-1) % 3;
        osAiSetNextBuffer(audioBuffer[buf], audioSamples[buf]<<2);

        /*
         * Empty the dma queue to make sure all DMAs have completed
         */
        for (i=0; i<nextDMA; i++)
            (void) osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        
        osWritebackDCacheAll();
        osSpTaskStart(tlistp);
        
	CleanDMABuffs();
        /*
         * Swap buffers for wavetable storage and output
         */
	gFrameCt++;
        curBuf ^= 1; 
        curAudioBuf++; 
        nextDMA = 0;
    } while (1); 
    
    alSndpDelete(sndp);
    alClose(&g);
}

static s32
nActiveSounds(ALSndPlayer *sndp, ALInstrument  *inst, ALSndId *idPtr)

{
    s32
        i,
        nactive = 0;

    for (i=0; i<sndp->maxSounds; i++){
        alSndpSetSound(sndp, idPtr[i]);
        if (alSndpGetState(sndp) != AL_STOPPED)
            nactive++;
    }
    return nactive;
}

static s32
myRandom(s32 min, s32 max) 
{
    f32 frand;
    static s32 seed = 1;

    seed = seed * 1103515245 + 12345;
    
    frand = (f32)(seed & MAX_INT)/(f32)MAX_INT; 
    frand = frand * (max - min) + min;
    
    return (s32)(frand + 0.5);
}



