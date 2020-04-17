/******************************************************************************
** �� �� ��:  plc_timer.h
** ˵ ��:     plc_timer.c��ͷ�ļ�
** ������:    hyafz
** �޸���Ϣ�� 
**         �޸���    �޸�����       �޸�����
**       
******************************************************************************/

#ifndef _PLC_TIMER_H
#define _PLC_TIMER_H

/* define -------------------------------------------------------------------*/
/* typedef ------------------------------------------------------------------*/
/* Export variables declaration ---------------------------------------------*/
/* Export function declaration ----------------------------------------------*/
void plcTimerInit(void);
void plcTimerStart(void);
/*PLC��ʱ���жϴ���HOOK*/
void plcTimeoutCallback(void);
/*��ȡPLC����ʱ��*/
unsigned int plcTimeMsecGet(void);
/*��ȡPLC΢��ʱ��*/
unsigned int plcTimeUsecGet(void);
unsigned int timeConsumedCalc(unsigned int begin, unsigned int end);

#endif

