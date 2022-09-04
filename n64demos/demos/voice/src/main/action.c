/*---------------------------------------------------------------------*
	Copyright (C) 1998, Nintendo.
	
	File		action.c
	Coded    by	Tetsuyuki Ootsuka.	July, 1998.

	$Id: action.c,v 1.1.1.1 2002/05/02 03:27:29 blythe Exp $
 *---------------------------------------------------------------------*/
#include	<ultra64.h>
#include        "action.h"
#include	"nu64sys.h"

/*-- Action ��¤�� --*/

Action  Ac;

/*-- Action ��¤�Τν���� --*/

void    actionInit(void)
{
  int   i;

  for (i = 0; i < MAXCONTROLLERS; i ++){
    Ac.pad[i].c = contPad + i;
    Ac.pad[i].button = Ac.pad[i].push = Ac.pad[i].release = 0;
  }
  return;
}

/*-- Action ��¤�ΤΥ��Ф򹹿����� --*/

void    actionUpdate(void)
{
  int   i;
  u16   button, change;
  
  /*-- ����ȥ���Υȥꥬ�η׻� --*/
  for (i = 0; i < MAXCONTROLLERS; i ++){
    button = Ac.pad[i].c->button;
    change = Ac.pad[i].button ^ button;
    Ac.pad[i].release = (Ac.pad[i].push = change & button) ^ change;
    Ac.pad[i].button = button;
  }
  return;
}

/*======== End of action.c ========*/
