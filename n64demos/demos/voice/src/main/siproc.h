/*---------------------------------------------------------------------
        Copyright (C) 1998 Nintendo.
        
        File            cont.h
        Coded    by     Tetsuyuki Ootsuka.      July, 1998.
        Comments        Header file for SI Procedure
   
        $Id: 
   ---------------------------------------------------------------------*/
#ifndef	_siproc_h_
#define	_siproc_h_

/*--------------------------------------------------------------------
                              �ޥ������
  --------------------------------------------------------------------*/

#define	SI_MSG_NUM	1
#define	RET_MSG_NUM	1

/*--------------------------------------------------------------------
                              ��¤�����
  --------------------------------------------------------------------*/

typedef struct{				/* siproc �ؤΥ�å����� */
  u8    mode;
  u8	si_mode;
} sendMsg;

typedef struct{				/* siproc ����Υ�å����� */
  s32	ret;
  u8	mode;
  u8    si_mode;
} retMsg;

/*--------------------------------------------------------------------
                              �������
  --------------------------------------------------------------------*/

extern 	OSMesgQueue	sendMsgQ;	/* siproc �ؤΥ�å������Ϥ��� */
extern 	OSMesgQueue	retMsgQ;	/* siproc ����Υ�å������֤��� */

extern	void	siproc(void);

#endif /* _siproc_h_ */


