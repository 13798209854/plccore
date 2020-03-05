/**
  * @file  plc_timer.c
  * @brief  PLC�붨ʱ��ص�ʵ�֣�PLC��ʱ����ʱ���̶�Ϊ1���롣
  * @author  hyafz    
  */
/* includes -----------------------------------------------------------------*/
#include <rtthread.h>
#include "plc_port.h"
#include "plc_cfg.h"
#include "plc_io.h"
#include "plc_comm.h"

extern void devTimerStart(void);
extern void devTimerStop(void);
extern unsigned int devTimerCountGet(void);

/* Private define -----------------------------------------------------------*/
/* Private typedef ----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
unsigned int PlcTimeMsec = 0; /*PLC����ʱ��*/

/* Private function declaration ---------------------------------------------*/
/* Functions ----------------------------------------------------------------*/
/**
  * @brief PLC��ʱ������
  */
void plcTimerStart(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    PlcTimeMsec = 0;
    rt_hw_interrupt_enable(level);
    devTimerStart();
}

void plcTimerStop(void)
{
    devTimerStop();
}

/**
  * @brief ��ȡPLC����ʱ��
  */
unsigned int plcTimeMsecGet(void)
{
    unsigned int msec = 0;
    rt_base_t level = rt_hw_interrupt_disable();
    msec = PlcTimeMsec;
    rt_hw_interrupt_enable(level);
    return msec;
}

/**
  * @brief ��ȡPLC΢��ʱ��
  */
unsigned int plcTimeUsecGet(void)
{
    rt_base_t level;
    unsigned int msec1 = 0;
    unsigned int count1 = 0;
    unsigned int msec2 = 0;
    unsigned int count2 = 0;
    unsigned int usec = 0;
#if 0
    unsigned int usec1 = 0;
    unsigned int usec2 = 0;
#endif

    level = rt_hw_interrupt_disable();
    msec1 = PlcTimeMsec;
    count1 = devTimerCountGet();
    rt_hw_interrupt_enable(level);
    /*�˳��ٽ����������ʱ���жϵ����������Ӧ�жϣ�����PlcTimeMSec��*/
    level = rt_hw_interrupt_disable();
    msec2 = PlcTimeMsec;
    count2 = devTimerCountGet();
    rt_hw_interrupt_enable(level);

#if 1
    if(msec1 == msec2) /* msec��ȣ���ʾ��һ�ζ�ȡ��ֵû�����䣬���õ�һ�ζ�ȡ��ֵ */
    {
        usec = msec1 * (unsigned int)1000 + count1;
    }
    else /* msec����ȣ���ʾ�ڶ��ζ�ȡʱֵ�Ѿ�������ɣ����õڶ��ζ�ȡ��ֵ */
    {
        usec = msec2 * (unsigned int)1000 + count2;
    }
#else
    usec1 = msec1 * (unsigned int)1000 + count1;
    usec2 = msec2 * (unsigned int)1000 + count2;
    if(usec2 >= usec1){
        usec = usec2;
    }else{
        usec = (msec2 + 1) * (unsigned int)1000 + count2;
    }
#endif
    return usec;
}

/**
  * @brief PLC��ʱ���жϴ���HOOK��ÿ�������һ�Ρ�
  */
void plcTimerHook(void)
{
    int8_t runMode = PLC_SW_STOP;
    rt_base_t level = rt_hw_interrupt_disable();
    PlcTimeMsec++;
    rt_hw_interrupt_enable(level);

    /* ����ɨ�� */
    devInputScan();

    /*�жϿ����Ƿ��ж���*/
    runMode = isRunMode();
    if(isRunModeSwitched()){
        if((runMode == PLC_SW_RUN)){
            plcSendMsgToRsc(PLC_RSC_EVT_ORDER_TO_RUN, (TASK_CB_S*)0);
        }else if(runMode == PLC_SW_STOP){
            plcSendMsgToRsc(PLC_RSC_EVT_ORDER_TO_STOP, (TASK_CB_S*)0);
        }
    }

    /* COMMͨ�Ž��ճ�ʱ��� */
    commRxTimeoutCheck();

    /* ͨ��ָ��� */
    cmdProcess();

    /* ������ȴ��� */
    plcRscTaskSched(&(CfgCB.rscCB));
}

/**
  * @ brief �������ʼ�������ĺ�ʱ
  */
unsigned int timeConsumedCalc(unsigned int begin, unsigned int end)
{
	unsigned int consumedTime = 0;
	/* Ҫ���Ǽ�ʱ����ת����� */
	if(end >= begin)
	{
		consumedTime = end - begin;
	}
	else
	{
		consumedTime = 0xFFFFFFFF - begin + end;
	}
	return consumedTime;
}

