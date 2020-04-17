/**
  * @file     plc_stat.c
  * @brief    PLC����ͳ�ƹ��ܵ�ʵ��.
  * @author   hyafz
**/
/* includes -----------------------------------------------------------------*/
#include <string.h>
#include <rtthread.h>
#include "plc_debug.h"
#include "plc_stat.h"
#include "plc_timer.h"
#include "plc_comm.h"

/* Private define -----------------------------------------------------------*/
/* Private typedef ----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
static struct rt_semaphore	SemStat;                 /**<  ����ͳ�ƹ��ܵ��ź��� */
unsigned char StatDataRegion[MAX_STAT_DATA_REGION_SIZE];  /**< ͳ�������� */
unsigned int StatDataCnt = 0;      /**< ͳ���������ֽڼ���  */
unsigned int StatBeginTime = 0;    /**< ͳ�ƹ���������ʱ�� */
unsigned int StatLastEvtTime = 0;  /**< ��һ���¼�������ʱ�� */
unsigned char StatFlag = 0;        /**< ͳ�ƹ��ܱ�־ 0: ͳ��ֹͣ��1: ͳ�ƽ���. */
/* Private function declaration ---------------------------------------------*/
/* Functions ----------------------------------------------------------------*/
/**
  * @brief ͳ�ƹ��ܳ�ʼ������.
  */
void statInit(void)
{
	/* ����ͳ�ƹ����ź��� */
	rt_err_t err;
	err = rt_sem_init(&SemStat, 
	                  "", 
					  1, 
					  RT_IPC_FLAG_FIFO);
	RT_ASSERT(RT_EOK == err);
	memset((void*)StatDataRegion, 0, sizeof(StatDataRegion));
	StatDataCnt = 0;
	StatFlag = 0;
	StatBeginTime = 0;
	StatLastEvtTime = 0;
}

/**
  * @brief ͳ�ƿ�ʼ���������ڳ�ʼ��ͳ����������.
  */
void statBegin(void)
{
	rt_err_t err;

	err = rt_sem_take(&SemStat, RT_WAITING_FOREVER);
	RT_ASSERT(RT_EOK == err);
	
	//memset((void*)StatDataRegion, 0, sizeof(StatDataRegion));
	StatDataCnt = 0;
	debugOutput(PLC_EVT_STAT_START);
	StatBeginTime = plcTimeUsecGet();
	StatLastEvtTime = StatBeginTime;
	StatFlag = 1;

	rt_sem_release(&SemStat);
}

/**
  * @brief ͳ�ƽ�������.
  */
void statStop(void)
{
	rt_err_t err;

	err = rt_sem_take(&SemStat, RT_WAITING_FOREVER);
	RT_ASSERT(RT_EOK == err);
	StatFlag = 0;
	rt_sem_release(&SemStat);
	debugOutput(PLC_EVT_STAT_STOP);
}

/**
  * @brief ͳ���������������ֻ�������
  */
void statClear(void)
{
	rt_err_t err;
	if((StatFlag == 1) && (StatDataCnt > 0))
	{
		/* �ȴ��ź��� */
		err = rt_sem_take(&SemStat, RT_WAITING_FOREVER);
		RT_ASSERT(RT_EOK == err);
		StatDataCnt = 0;
		/* �ͷ��ź��� */
		rt_sem_release(&SemStat);
	}
}

/**
  * @brief  ͳ���¼����Ӻ���.
  * @param  label �¼������ǩ
  * @param  evt   �¼�����
  * @return �¼�������ʱ��
  */
unsigned int statEventAdd(unsigned char label, STAT_EVT_TYPE_E evt)
{
	rt_err_t err;
	unsigned int evtTime;  /* �¼�������ʱ�� */
#if (STAT_DATA_SAVE_MODE == STAT_DATA_COMPACT_MODE) /* �Խ���ģʽ����ͳ������ */
	unsigned int timeInc;  /* ʱ������ */
	unsigned char timeLen = 0;
#endif
	/* ��ȡ�¼�������ʱ�� */
	evtTime = plcTimeUsecGet();

	if((StatFlag == 0) || (StatDataCnt >= (MAX_STAT_DATA_REGION_SIZE - SINGLE_STAT_DATA_SIZE)))
	{
		/* δ����ͳ�ƹ��ܣ��򲻹��ռ�洢�µ��¼� */
		return evtTime;
	}

	/* �ȴ��ź��� */
	err = rt_sem_take(&SemStat, RT_WAITING_FOREVER);
	RT_ASSERT(RT_EOK == err);

	/* д��ͳ�������� */
	StatDataRegion[StatDataCnt++] = label;
	StatDataRegion[StatDataCnt] = evt;

#if (STAT_DATA_SAVE_MODE == STAT_DATA_COMPACT_MODE)	/* �Խ���ģʽ����ͳ������ */	
	/* ����ʱ������ */
	timeInc = timeConsumedCalc(StatLastEvtTime, evtTime);
	/*�����ϴ��¼�ʱ��*/
	StatLastEvtTime = evtTime;
	/* ����ʱ���򳤶� */
	if(timeInc < 0x100){
		timeLen = 0;
	}else if(timeInc < 0x10000){
		timeLen = 1;
	}else if(timeInc < 0x1000000){
		timeLen = 2;
	}else{
		timeLen = 3;
	}
	StatDataRegion[StatDataCnt++] |= (timeLen << 6);
	do{
		StatDataRegion[StatDataCnt++] = (unsigned char)(timeInc & 0xFF);
		timeInc >>= 8;
	}while(timeLen--);
#else	/* ����ͨģʽ����ͳ������ */
	StatDataCnt++;
	StatDataRegion[StatDataCnt++] = ((unsigned char)evtTime) & 0xFF;
	StatDataRegion[StatDataCnt++] = ((unsigned char)(evtTime >> 8)) & 0xFF;
	StatDataRegion[StatDataCnt++] = ((unsigned char)(evtTime >> 16)) & 0xFF;
	StatDataRegion[StatDataCnt++] = ((unsigned char)(evtTime >> 24)) & 0xFF;
#endif
	
	/* �ͷ��ź��� */
	rt_sem_release(&SemStat);

	return evtTime;
}

/**
  * @brief  ͳ���������Ӻ��������ͳ�����ݴ洢�ռ䲻�������ضϲ������ݡ�
  * @param data ����ָ��
  * @param len  ���ݳ���
  * @return �ɹ�д����ֽ���
  */
unsigned int statDataAdd(unsigned char* data, unsigned int len)
{
	unsigned int num = 0;
	unsigned int i;

	rt_err_t err;
	if((StatFlag == 0) || (StatDataCnt >= MAX_STAT_DATA_REGION_SIZE))
	{
		/* δ����ͳ�ƹ��ܣ��򲻹��ռ�洢�µ��¼� */
		return num;
	}

	/* �ȴ��ź��� */
	err = rt_sem_take(&SemStat, RT_WAITING_FOREVER);
	RT_ASSERT(RT_EOK == err);
	
	for(i = 0; i < len; i++){
		if(StatDataCnt < MAX_STAT_DATA_REGION_SIZE){
			StatDataRegion[StatDataCnt++] = data[i];
			num++;
		}else{
			break;
		}
	}
	/* �ͷ��ź��� */
	rt_sem_release(&SemStat);
	return num;
}


/**
  * @brief ��ͳ�����ݷ��͸���λ��
  */
void statOutput(void)
{
	if((StatFlag == 1) && (StatDataCnt > 0))
	{
	    plcCommDataFrameSend(CMD_STAT_OUTPUT, StatDataRegion, StatDataCnt);
	}
}

