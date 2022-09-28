#include <nusys.h>
void stage00(int);
void makeDL00(void);
void mainproc(void){
	nuGfxInit();
	nuGfxFuncSet((NUGfxFunc)stage00);
	nuGfxDisplayOn();
	while(1)
		;
}
void stage00(int pendingGfx) {
	if(pendingGfx < 1)
		makeDL00();
}
