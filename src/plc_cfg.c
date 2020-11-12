/**
  * @file  plc_cfg.c
  * @brief     ����IEC61131-3��׼��PLC����Ԫ����ع���Դ�ļ�
  * @author    fengzhou
  */
/* includes -----------------------------------------------------------------*/
#include <string.h>
#include <rtthread.h>
#include "plc_mem.h"
#include "plc_cr.h"
#include "plc_debug.h"
#include "plc_define.h"
#include "plc_cfg.h"
#include "plc_timer.h"
#include "plc_io.h"
#include "plc_comm.h"
#include "plc_stat.h"

extern void _configuration_init_(void);

/* Private define -----------------------------------------------------------*/
/*����������ض���*/
#define TASK_SINGLE_EN                 0x8000  /*SINGLE����ʹ�ܱ�ʶ*/

#define TASK_SINGLE_PARA_TYPE_MASK     0x4000  /*SINGLE������������*/
#define TASK_SINGLE_PARA_TYPE_VR       0x4000  /*SINGLEΪ��������*/
#define TASK_SINGLE_PARA_TYPE_DV       0x0000  /*SINGLEΪֱ�ӱ�ʾ����*/

#define TASK_INTERVAL_PARA_TYPE_MASK   0x3000  /*INTERVAL������������*/
#define TASK_INTERVAL_PARA_CONST       0x2000  /*INTERVAL����Ϊ����*/
#define TASK_INTERVAL_PARA_VR          0x1000  /*INTERVAL����Ϊ��������*/
#define TASK_INTERVAL_PARA_DV          0x0000  /*INTERVAL����Ϊֱ�ӱ�ʾ����*/

#define TASK_PRIORITY_MASK             0x00FF  /*�������ȼ�����*/

#define TASK_ID_NULL                   0xFFFF  /*��Ч������ID*/
/* Private typedef ----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
CFG_CB_S CfgCB; /*PLC���ÿ��ƿ�*/


/*PLC�����ջ*/
struct rt_thread PlcTaskThread[MAX_TASK_NUM_PER_RSC];
unsigned char PlcTaskRunTaskStks[MAX_TASK_NUM_PER_RSC + 1][PLC_TASK_RUN_TASK_STK_SIZE];

struct rt_messagequeue PlcRscMsgQ;
unsigned char PlcRscMsgPool[MAX_SIZE_OF_RSC_MSG_QUEUE * RSC_MSG_SIZE];

/* Private function declaration ---------------------------------------------*/
/* Functions ----------------------------------------------------------------*/
void plcTaskStatDataOutput(TASK_CB_S* pTaskCB)
{
	statDataAdd((unsigned char*)&(pTaskCB->priority), 1);
	statDataAdd((unsigned char*)&(pTaskCB->stkUsedMax), 1);
	statDataAdd((unsigned char*)&(pTaskCB->trigCnt), 4);
	statDataAdd((unsigned char*)&(pTaskCB->runCnt), 4);
	statDataAdd((unsigned char*)&(pTaskCB->runTimeMin), 4);
	statDataAdd((unsigned char*)&(pTaskCB->runTimeMax), 4);
	statDataAdd((unsigned char*)&(pTaskCB->runTimeAverage), 4);
	statDataAdd((unsigned char*)&(pTaskCB->gapTimeMin), 4);
	statDataAdd((unsigned char*)&(pTaskCB->gapTimeMax), 4);
	statDataAdd((unsigned char*)&(pTaskCB->gapTimeAverage), 4);
	if(pTaskCB->trigCnt > 0){
		statDataAdd((unsigned char*)&(pTaskCB->delayTimeMin), 4);
		statDataAdd((unsigned char*)&(pTaskCB->delayTimeMax), 4);
		statDataAdd((unsigned char*)&(pTaskCB->delayTimeAverage), 4);
	}
}

/**
  * @brief ��Դ����ʱͳ�����������������
  */
void plcRscStatDataOutput(RSC_CB_S* pRscCB)
{
	int i;
	statDataAdd((unsigned char*)&(pRscCB->cycleCnt), 4);
	statDataAdd((unsigned char*)&(pRscCB->cycleTimeMin), 4);
	statDataAdd((unsigned char*)&(pRscCB->cycleTimeMax), 4);
	statDataAdd((unsigned char*)&(pRscCB->cycleTimeAverage), 4);
	statDataAdd((unsigned char*)&(pRscCB->gapTimeMin), 4);
	statDataAdd((unsigned char*)&(pRscCB->gapTimeMax), 4);
	statDataAdd((unsigned char*)&(pRscCB->gapTimeAverage), 4);
	statDataAdd((unsigned char*)&(pRscCB->taskNum), 4);
	for(i = 0; i < pRscCB->taskNum; i++){
		plcTaskStatDataOutput(&(pRscCB->taskCB[i]));
	}
	plcTaskStatDataOutput(&(pRscCB->taskDefaultCB));
}

/**
  * @brief PLC��Դɨ�����ڿ�ʼʱ�Ĵ���.
  */
static void plcRscRunCycleBegin(RSC_CB_S* pRsc)
{
	UDINT begin;
	UDINT end;
	UDINT prevBegin;

	/* ��¼��Դ��ѭ�����ڿ�ʼʱ�� */
	prevBegin = pRsc->cycleBegin;
	//pRsc->cycleBegin = statEventAdd(PLC_TASK_LOWEST_PRIORITY + 1, SE_RSC_CYCLE_BEGIN);
	pRsc->cycleBegin = plcTimeUsecGet();
	
	if(pRsc->cycleCnt > 0){
		/* ��¼��������ѭ�����ʱ�䣬���������ֵ����Сֵ��ƽ��ֵ */
		pRsc->gapTime = timeConsumedCalc(prevBegin, pRsc->cycleBegin);
		if(pRsc->gapTime > pRsc->gapTimeMax){
			pRsc->gapTimeMax = pRsc->gapTime;
		}
		if(pRsc->gapTime < pRsc->gapTimeMin){
			pRsc->gapTimeMin = pRsc->gapTime;
		}
		pRsc->gapTimeSum += pRsc->gapTime;
		pRsc->gapTimeAverage = pRsc->gapTimeSum / pRsc->cycleCnt;
	}
	pRsc->cycleCnt ++;
	
	/* DI/AI refresh */
	begin = pRsc->cycleBegin;
    plcLocalDiRefresh();
    plcLocalAiRefresh();
	end = plcTimeUsecGet();
	pRsc->diRefreshTime = timeConsumedCalc(begin, end);

	/* ����Ĭ������ִ�� */	
	rt_sem_release(&(pRsc->taskDefaultCB.semTask));
	pRsc->taskRunningCnt ++;
}

/**
  * @brief PLC��Դɨ�����ڽ���ʱ�Ĵ���.
  */
static void plcRscRunCycleOver(RSC_CB_S* pRsc)
{
	UDINT begin, end;

	/* DQ/AQ refresh */
	begin = plcTimeUsecGet();
	plcLocalDqRefresh();
	plcLocalAqRefresh();
	plcLocalPwmOutputRefresh();
	
	/* ��¼��Դ��ѭ�����ڽ���ʱ�� */
	//pRsc->cycleEnd = statEventAdd(PLC_TASK_LOWEST_PRIORITY + 1, SE_RSC_CYCLE_OVER);
	pRsc->cycleEnd = plcTimeUsecGet();

	/*����DQ���µ�ʱ��*/
	end = pRsc->cycleEnd;
	pRsc->dqRefreshTime = timeConsumedCalc(begin, end);
	
	/* ������ѭ������ʱ�� */
	pRsc->cycleTime = timeConsumedCalc(pRsc->cycleBegin, pRsc->cycleEnd);
	if(pRsc->cycleTime > pRsc->cycleTimeMax)
	{
		pRsc->cycleTimeMax = pRsc->cycleTime;
	}
	if(pRsc->cycleTime < pRsc->cycleTimeMin)
	{
		pRsc->cycleTimeMin = pRsc->cycleTime;
	}
	pRsc->cycleTimeSum += pRsc->cycleTime;
	pRsc->cycleTimeAverage = pRsc->cycleTimeSum / pRsc->cycleCnt;

	statOutput();	/* ÿ��ִ�����ڽ������ͳ�����ݣ�Ȼ������� */
	statClear();
}

void plcRscRunStatusUpdate(RSC_CB_S* pRscCB)
{
	unsigned int msTime = plcTimeMsecGet();
#define PLC_RSC_RUN_STATE_UPDATE_PERIOD 1000	/**< PLC��Դ����ʱ״̬�������ڣ���λms */
	unsigned int updateGapTime = timeConsumedCalc(pRscCB->updateTime, msTime);
	if(updateGapTime >= PLC_RSC_RUN_STATE_UPDATE_PERIOD){
		updateGapTime = 0;
		pRscCB->updateTime = msTime;
		/* ��������ʱͳ������ */
		//statEventAdd(PLC_TASK_LOWEST_PRIORITY + 1, SE_RSC_UPDATE);
		//plcRscStatDataOutput(pRscCB);
		/* ����M������ */
		//plcCommDataFrameSend(CMD_M_OUTPUT, M, sizeof(M));
	}
}

/**
  * @brief ��ʼ�����������ʱ����
  * @param ������ƿ�ָ��
  */
void plcTaskRunTimeInit(TASK_CB_S* pTaskCB)
{
	pTaskCB->singlePrevious = 0;
	pTaskCB->intervalBegin = plcTimeMsecGet();

	pTaskCB->trigCnt = 0;
	pTaskCB->runCnt = 0;
	
	pTaskCB->runTimeMin = 0xFFFFFFFF;
	pTaskCB->runTimeMax = 0;
	pTaskCB->runTimeAverage = 0;
	pTaskCB->runTimeSum = 0;
	
	pTaskCB->gapTimeMin = 0xFFFFFFFF;
	pTaskCB->gapTimeMax = 0;
	pTaskCB->gapTimeAverage = 0;
	pTaskCB->gapTimeSum = 0;
	
	pTaskCB->delayTimeMin = 0xFFFFFFFF;
	pTaskCB->delayTimeMax = 0;
	pTaskCB->delayTimeAverage = 0;
	pTaskCB->delayTimeSum = 0;
}

/**
  * @brief PLC�����߳���ں���
  * @note �����յ������ź����󣬻�һֱ����ֱ�����������Ǳ��������ȼ������жϣ�������������������CPU����Ȩ��
  *       ��������ȼ�����ִ����ɷ��غ󣬱���ѭ��������
  */
static void plcTaskThreadEntry(void *pArg)
{
	rt_err_t err;
	UDINT prevBeginTime;

	TASK_CB_S* pTaskCB = (TASK_CB_S*)pArg;
	RT_ASSERT(pTaskCB != (void*)0);

	/* ��ʼ�������ź��� */
	err = rt_sem_init(&(pTaskCB->semTask), 
	                  "", 
					  0,
					  RT_IPC_FLAG_FIFO);
	RT_ASSERT(RT_EOK == err);

	/*��ʼ������ʱ����*/
	plcTaskRunTimeInit(pTaskCB);

	pTaskCB->taskSta = TASK_READY;

	/*����׼������,����Ϣ֪ͨ��Դ*/
	plcSendMsgToRsc(PLC_RSC_EVT_TASK_READY, pTaskCB);
	
	while(1)
	{
		err = rt_sem_take(&(pTaskCB->semTask), RT_WAITING_FOREVER);
		if(err != RT_EOK){
			continue;
		}

		pTaskCB->taskSta = TASK_RUNNING;
		
		/* ��¼���񵱴ο�ʼ����ʱ�䣬��Ĭ��������Ҫ��¼��ʼ�¼���Ĭ�����񲻼�¼�� */
		prevBeginTime = pTaskCB->beginTime;
		//if(pTaskCB->priority == PLC_TASK_LOWEST_PRIORITY){
			pTaskCB->beginTime = plcTimeUsecGet();
		//}else{
		//	pTaskCB->beginTime = statEventAdd(pTaskCB->priority, SE_TASK_BEGIN);
		//}
		/* �����ӳ�ʱ�䣬Ĭ�����񲻿����ӳ�ʱ�� */
		if(pTaskCB->trigCnt > 0){
			pTaskCB->delayTime = timeConsumedCalc(pTaskCB->trigTime, pTaskCB->beginTime);
			if(pTaskCB->delayTime > pTaskCB->delayTimeMax){
				pTaskCB->delayTimeMax = pTaskCB->delayTime;
			}
			if(pTaskCB->delayTime < pTaskCB->delayTimeMin){
				pTaskCB->delayTimeMin = pTaskCB->delayTime;
			}
			pTaskCB->delayTimeSum += pTaskCB->delayTime;
			pTaskCB->delayTimeAverage = pTaskCB->delayTimeSum / pTaskCB->trigCnt;
		}

		/* ������ʱ�� */
		if(pTaskCB->runCnt > 0){
			pTaskCB->gapTime = timeConsumedCalc(prevBeginTime, pTaskCB->beginTime);
			if(pTaskCB->gapTime > pTaskCB->gapTimeMax){
				pTaskCB->gapTimeMax = pTaskCB->gapTime;
			}
			if(pTaskCB->gapTime < pTaskCB->gapTimeMin){
				pTaskCB->gapTimeMin = pTaskCB->gapTime;
			}
			pTaskCB->gapTimeSum += pTaskCB->gapTime;
			pTaskCB->gapTimeAverage = pTaskCB->gapTimeSum / pTaskCB->runCnt;
		}
		pTaskCB->runCnt++;
		
		/* ִ�������� */
		pTaskCB->taskRun();

		/* ��¼���񵱴����н���ʱ�䣬��Ĭ��������Ҫ��¼�����¼���Ĭ�������¼�� */
		//if(pTaskCB->priority == PLC_TASK_LOWEST_PRIORITY){
			pTaskCB->endTime = plcTimeUsecGet();
		//}else{
		//	pTaskCB->endTime = statEventAdd(pTaskCB->priority, SE_TASK_OVER);
		//}
		
		/*������������ʱ��*/
		pTaskCB->runTime = timeConsumedCalc(pTaskCB->beginTime, pTaskCB->endTime);
		
		/*ͳ��������������������ʱ�䡢ƽ��ʱ�� */
		if(pTaskCB->runTime > pTaskCB->runTimeMax)
		{
			pTaskCB->runTimeMax = pTaskCB->runTime;
		}
		if(pTaskCB->runTime < pTaskCB->runTimeMin)
		{
			pTaskCB->runTimeMin = pTaskCB->runTime;
		}		
		pTaskCB->runTimeSum += pTaskCB->runTime;
		pTaskCB->runTimeAverage = pTaskCB->runTimeSum / pTaskCB->runCnt;

		/** @todo ͳ������ջʹ�������RTT�ݲ�֧�� */

		pTaskCB->taskSta = TASK_READY;
		
		/*����ִ�����,����Ϣ֪ͨ��Դ*/
		plcSendMsgToRsc(PLC_RSC_EVT_TASK_RETURN, pTaskCB);
	}
}

/**
  * @brief ��ʼ��PLC��Դ����ʱ����
  * @param ��Դ���ƿ�ָ��
  */
void plcRscRunTimeInit(RSC_CB_S* pRscCB)
{
	int i = 0;
	
	pRscCB->cycleCnt = 0;
	pRscCB->taskRunningCnt = 0;
	
	pRscCB->cycleBegin = 0;
	pRscCB->cycleEnd = 0;
	pRscCB->cycleTime = 0;
	pRscCB->cycleTimeMin = 0xFFFFFFFF;
	pRscCB->cycleTimeMax = 0;
	pRscCB->cycleTimeAverage = 0;
	pRscCB->cycleTimeSum = 0;
	
	pRscCB->gapTime = 0;
	pRscCB->gapTimeMin = 0xFFFFFFFF;
	pRscCB->gapTimeMax = 0;
	pRscCB->gapTimeAverage = 0;
	pRscCB->gapTimeSum = 0;

	pRscCB->diRefreshTime = 0;
	pRscCB->dqRefreshTime = 0;

	for(i = 0; i < pRscCB->taskNum; i++){
		plcTaskRunTimeInit(&(pRscCB->taskCB[i]));
	}
	plcTaskRunTimeInit(&(pRscCB->taskDefaultCB));
	
	plcMemInit();
}

/**
  * @brief PLC��Դ�ڲ�������Ⱥ�������PLCϵͳ��ʱ���жϵ��á�
  */
void plcRscTaskSched(RSC_CB_S* pRscCB)
{
	UDINT cnt;
	TASK_CB_S* pTaskCB;
	BOOL singleCurrent;
	TIME intervalSet;
	TIME intervalTime;
	unsigned int currentTimeMsec;

	if(pRscCB->rscState != RSC_RUNNING)
	{
		return;
	}
	
	for(cnt = 0; cnt < pRscCB->taskNum; cnt++)
	{
		pTaskCB = &(pRscCB->taskCB[cnt]);
		
		/*��������Ƿ�׼����*/
		if(pTaskCB->taskSta == TASK_UNAVAILABLE)
		{
			pTaskCB->singlePrevious = 0;
			pTaskCB->intervalBegin = plcTimeMsecGet();
			continue;
		}
		
		/*���single����������*/
		singleCurrent = pTaskCB->singleGet();
		if((pTaskCB->singlePrevious == 0) && 
			(singleCurrent == 1))
		{
			/*��⵽single���������أ����񱻴�����֪ͨ��Դ���е��ȡ�*/
			plcSendMsgToRsc(PLC_RSC_EVT_TASK_TRIGGED, pTaskCB);
		}
		/*����single���뵱ǰ״̬*/
		pTaskCB->singlePrevious = singleCurrent;
		
		/*���single���벻Ϊ0�������ڵ������õ�interval���벻������*/
		if(singleCurrent != 0)
		{
			pTaskCB->intervalBegin = plcTimeMsecGet();
			continue;
		}

		/*���single����Ϊ0������Ҫ������ڵ��õ�intervalʱ���Ƿ񵽴*/
		
		/*���intervalʱ��*/
		intervalSet = pTaskCB->intervalGet();
		if(intervalSet > 0)
		{
			currentTimeMsec = plcTimeMsecGet();
			intervalTime = timeConsumedCalc(pTaskCB->intervalBegin, currentTimeMsec);
			if(intervalTime >= intervalSet)
			{
				/*��⵽���ڵ���intervalʱ�䵽�����񱻴�����֪ͨ��Դ���е��ȡ�*/
				plcSendMsgToRsc(PLC_RSC_EVT_TASK_TRIGGED, pTaskCB);
				/*���浱ǰPLC����ʱ��Ϊ��һ�����ڵ��ü����ʼʱ��*/
				pTaskCB->intervalBegin = currentTimeMsec;
			}
		}
	}
}

void plcRscRun(RSC_CB_S* pRscCB)
{
	/*��Դ����ʱ������ʼ��*/
	plcRscRunTimeInit(pRscCB);
	LED_RUN_ON();
	LED_STOP_OFF();
	debugOutput(PLC_EVT_BEGIN_TO_RUN);	
		
	/* ��¼��Դ����ʱ�� */
	//pRscCB->runTime = statEventAdd(PLC_TASK_LOWEST_PRIORITY + 1, SE_RSC_RUN);
	pRscCB->runTime = plcTimeUsecGet();

	/* ��¼��Դ����״̬����ʱ�� */
	pRscCB->updateTime = plcTimeMsecGet();
	
	/* ������������ */
	plcRscRunCycleBegin(pRscCB);
}

void plcRscStop(RSC_CB_S* pRscCB)
{
	plcLocalDqOutputWhenStopped();
	/* ��¼��Դͣ��ʱ�� */
	pRscCB->stopTime = statEventAdd(PLC_TASK_LOWEST_PRIORITY + 1, SE_RSC_STOP);
	plcRscStatDataOutput(pRscCB);	
	statOutput();	/* ��Դͣ��ʱ���ͳ�����ݣ�Ȼ������� */
	statClear();
	
	LED_RUN_OFF();
	LED_STOP_ON();
	debugOutput(PLC_EVT_STOPPED);
}


/**
  * @brief PLC��Դ������ں���
  * @param p_arg reserved
  */
static void plcRscRunEntry(void* p_arg)
{
	rt_err_t err;
	UDINT i;
	RSC_CB_S* pRscCB;
	TASK_CB_S* pTaskCB;
	RSC_MSG_S rscMsg;
	/*��ָ��ָ����Դ,���ڷ���*/
	pRscCB = &(CfgCB.rscCB);

	/* ��ʼ����Դ��Ϣ���� */
	err = rt_mq_init(&PlcRscMsgQ, 
					"", 
					(void*)PlcRscMsgPool, 
					RSC_MSG_SIZE, 
					sizeof(PlcRscMsgPool), 
					RT_IPC_FLAG_FIFO);
	
	/*���������ڱ���Դ���������������*/
	for(i = 0; i < pRscCB->taskNum; i++)
	{
		pTaskCB = &(pRscCB->taskCB[i]);
		RT_ASSERT(pTaskCB->priority + PLC_TASK_HIGHEST_PRIORITY < PLC_TASK_LOWEST_PRIORITY);
		err = rt_thread_init(&PlcTaskThread[i],
		                     "",
							 plcTaskThreadEntry,
							 (void*)pTaskCB,
							 (void*)&(PlcTaskRunTaskStks[i][0]),
							 (unsigned int)PLC_TASK_RUN_TASK_STK_SIZE,
							 PLC_TASK_HIGHEST_PRIORITY + pTaskCB->priority,
							 100);
	    RT_ASSERT(RT_EOK == err);
	    rt_thread_startup(&PlcTaskThread[i]);

	}
	/*��������Դ��Ĭ������*/
	pTaskCB = &(pRscCB->taskDefaultCB);

	err = rt_thread_init(&PlcTaskThread[i],
		                 "",
						 plcTaskThreadEntry,
						 (void*)pTaskCB,
						 (void*)&(PlcTaskRunTaskStks[i][0]),
						 (unsigned int)PLC_TASK_RUN_TASK_STK_SIZE,
						 pTaskCB->priority,
						 100);
    RT_ASSERT(RT_EOK == err);
    rt_thread_startup(&PlcTaskThread[i]);

	/*��ʼ������ʱ����*/
	plcRscRunTimeInit(pRscCB);
	pRscCB->rscState = RSC_INITIALIZING;	/* ��Դ����״̬����Ϊ���ڳ�ʼ��״̬ */
	
	while(1)
	{		
		/*�ȴ���Ϣ����*/
		err = rt_mq_recv(&PlcRscMsgQ, (void*)&rscMsg, RSC_MSG_SIZE, RT_WAITING_FOREVER);
	    RT_ASSERT(RT_EOK == err);
		if(err != RT_EOK){
			continue;
		}
		//DBG_PRINTF("(RSC)Message received, evt = %d\r\n", rscMsg.evt);

		//��Դ����״̬������
		switch(pRscCB->rscState)
		{
		case RSC_INITIALIZING:
			if(PLC_RSC_EVT_TASK_READY == rscMsg.evt)
			{
				if(PLC_TASK_LOWEST_PRIORITY == rscMsg.ptaskcb->priority)
                {
					debugOutput(PLC_EVT_READY_TO_RUN);
                    /*������ȼ�����׼��������������Դ�����õ�����������׼����������Դ��ʼ����ɡ�*/
					if(isRunMode() == 0){
						pRscCB->rscState = RSC_READY_TO_RUN;
					}else{
						pRscCB->rscState = RSC_RUNNING;
						plcRscRun(pRscCB);
					}
                }
			}
			break;
		case RSC_READY_TO_RUN:
		case RSC_STOP:
			if(PLC_RSC_EVT_ORDER_TO_RUN == rscMsg.evt)
			{
				pRscCB->rscState = RSC_RUNNING;
				plcRscRun(pRscCB);
			}
			break;
		case RSC_RUNNING:
			if((PLC_RSC_EVT_TASK_RETURN == rscMsg.evt) 
				|| (PLC_RSC_EVT_TASK_ERROR == rscMsg.evt))
			{
				pRscCB->taskRunningCnt --;
				if(0 == pRscCB->taskRunningCnt)
                {
                	/* û��������ִ���� */
                	/* ���������������� */
                	plcRscRunCycleOver(pRscCB);
					/* ���������������� */
               		plcRscRunCycleBegin(pRscCB);
                }
			}
			else if(PLC_RSC_EVT_TASK_TRIGGED == rscMsg.evt)
			{
				/* �����񱻴����������ź������������� */
				//rscMsg.ptaskcb->trigTime = statEventAdd(rscMsg.ptaskcb->priority, SE_TASK_TRIGGED);
				rscMsg.ptaskcb->trigTime = plcTimeUsecGet();
				rscMsg.ptaskcb->trigCnt++;
				err = rt_sem_release(&(rscMsg.ptaskcb->semTask));
				pRscCB->taskRunningCnt ++;
			}
			else if(PLC_RSC_EVT_ORDER_TO_STOP == rscMsg.evt)
			{
				/* �յ�ͣ���¼�������׼��ͣ��״̬ */
				pRscCB->rscState = RSC_GOTO_STOP;
			}
			plcRscRunStatusUpdate(pRscCB);
			break;
		case RSC_GOTO_STOP:
			if((PLC_RSC_EVT_TASK_RETURN == rscMsg.evt) 
				|| (PLC_RSC_EVT_TASK_ERROR == rscMsg.evt))
			{
				pRscCB->taskRunningCnt --;				
				if(0 == pRscCB->taskRunningCnt)
                {
                	/* û��������ִ���ˣ����������������� */
                	plcRscRunCycleOver(pRscCB);
                }
			}
			if(0 == pRscCB->taskRunningCnt)
			{
				//�������������ͣ��				
				pRscCB->rscState = RSC_STOP;
				plcRscStop(pRscCB);
			}
			break;
		case RSC_ERROR:
			debugOutput(PLC_EVT_ERROR);
			break;
		default:
			break;
		}
	}
}

/**
  * @brief PLC�������к���
  * @details
  * <pre>
  * PLC����������Ҫ��ʼ������ȫ�ֱ�����������������Դ��ע��ͬ����
  * PLC����������Ҫ��ʱˢ������ȫ�ֱ����������ȡ·�����ʡ�
  * PLC����ֹͣ��Ҫֹͣ������Դ��ע��ͬ����
  * </pre>
  */
void plcCfgRun(void)
{	
    /*�����û�����*/
    _configuration_init_();
   
	/*Ŀǰֻ֧��һ����Դ��ֱ��ִ����Դ�������񣬲��᷵�ء� */
    plcRscRunEntry((void*)0);
}

/**
  * @brief PLC����ʱ����Դ������Ϣ����
  * @param evt �¼�ID
  * @param pTaskCB ������ƿ�ָ�룬���Ϊ�գ���ʾ�¼����������������
  */
void plcSendMsgToRsc(BYTE evt, TASK_CB_S* pTaskCB)
{
	rt_err_t err;
	RSC_MSG_S rscMsg;
	//DBG_PRINTF("Message need to be send to RSC, evt = %d\r\n", evt);
	//if(isNotNull(pTaskCB))
	//{
	//	DBG_PRINTF("Message from task(%d)\r\n", pTaskCB->priority);
	//}
    rscMsg.evt = evt;
	rscMsg.ptaskcb = pTaskCB;
    //DBG_PRINTF("Send message to RSC, evt = %d\r\n", rscMsg.evt);
    err = rt_mq_send(&PlcRscMsgQ, (void*)&rscMsg, RSC_MSG_SIZE);
    RT_ASSERT(RT_EOK == err);
}
