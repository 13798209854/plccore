/**
  * @file  plc_timer.c
  * @brief  PLC�붨ʱ��ص�ʵ�֣�PLC��ʱ����ʱ���̶�Ϊ1���롣
  * @author  hyafz    
  */
/* includes -----------------------------------------------------------------*/
#include <rtthread.h>
#ifdef RT_USING_DEVICE
#include <rtdevice.h>
#endif
#include "plc_port.h"
#include "plc_cfg.h"
#include "plc_io.h"
#include "plc_comm.h"

/* Private define -----------------------------------------------------------*/
/* Private typedef ----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
#ifdef RT_USING_DEVICE
rt_device_t plc_hwtimer_dev;                     /* PLC��ʱ���豸��� */
#else
unsigned int PlcTimeMsec = 0; /*PLC����ʱ��*/
#endif

/* Private function declaration ---------------------------------------------*/
/* Functions ----------------------------------------------------------------*/

/**
  * @brief PLC��ʱ����ʱ�ص�������ÿ�������һ�Ρ�
  */
void plcTimeoutCallback(void)
{
    int8_t runMode = PLC_SW_STOP;
#ifndef RT_USING_DEVICE
    rt_base_t level = rt_hw_interrupt_disable();
    PlcTimeMsec++;
    rt_hw_interrupt_enable(level);
#endif

    /* ����ɨ�� */
    plcInputScan();

    /* ͨ�Ž��ճ�ʱ�ж��봦�� */
    plcCommRxTimeoutCheck();

    /*�жϿ����Ƿ��ж���*/
    runMode = isRunMode();
    if(isRunModeSwitched()){
        if((runMode == PLC_SW_RUN)){
            plcSendMsgToRsc(PLC_RSC_EVT_ORDER_TO_RUN, (TASK_CB_S*)0);
        }else if(runMode == PLC_SW_STOP){
            plcSendMsgToRsc(PLC_RSC_EVT_ORDER_TO_STOP, (TASK_CB_S*)0);
        }
    }

    /* ������ȴ��� */
    plcRscTaskSched(&(CfgCB.rscCB));
}

#ifdef RT_USING_DEVICE
rt_err_t plcTimeoutIndicate(rt_device_t dev, rt_size_t size)
{
    plcTimeoutCallback();
    return RT_EOK;
}
#endif

/**
 * @brief PLC��ʱ����ʼ��
 */
void plcTimerInit(void)
{
#ifdef RT_USING_DEVICE
    rt_err_t ret = RT_EOK;
    rt_hwtimer_mode_t mode;         /* ��ʱ��ģʽ */
    rt_uint32_t freq = 1000000;       /* ����Ƶ�� */
    /* ���Ҷ�ʱ���豸 */
    plc_hwtimer_dev = rt_device_find(PLC_HWTIMER_DEV_NAME);
    if (plc_hwtimer_dev == RT_NULL)
    {
        rt_kprintf("plc hwtimer run failed! can't find %s device!\n", PLC_HWTIMER_DEV_NAME);
        return;
    }
    /* �Զ�д��ʽ���豸 */
    ret = rt_device_open(plc_hwtimer_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        rt_kprintf("open %s device failed!\n", PLC_HWTIMER_DEV_NAME);
        return;
    }
    /* ���ó�ʱ�ص����� */
    rt_device_set_rx_indicate(plc_hwtimer_dev, plcTimeoutIndicate);
    /* ���ü���Ƶ�� ( Ĭ�� 1MHz ��֧�ֵ���С����Ƶ�� )  */
    rt_device_control(plc_hwtimer_dev, HWTIMER_CTRL_FREQ_SET, &freq);
    /* ����ģʽΪ�����Զ�ʱ�� */
    mode = HWTIMER_MODE_PERIOD;
    ret = rt_device_control(plc_hwtimer_dev, HWTIMER_CTRL_MODE_SET, &mode);
    if (ret != RT_EOK)
    {
        rt_kprintf("set mode failed! ret is :%d\n", ret);
        return;
    }
#else
    extern void devTimerInit();
    devTimerInit();
#endif
}

/**
  * @brief PLC��ʱ������
  */
void plcTimerStart(void)
{
#ifdef RT_USING_DEVICE
    rt_hwtimerval_t timeout_s;      /* ��ʱ����ʱֵ */
    /* ���ö�ʱ����ʱֵΪ1000us��������ʱ�� */
    timeout_s.sec = 0;      /* �� */
    timeout_s.usec = 1000;     /* ΢�� */
    if (rt_device_write(plc_hwtimer_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
    {
        rt_kprintf("set timeout value failed\n");
    }
#else
    extern void devTimerInit(void);
    extern void devTimerStart(void);
    devTimerInit();
    devTimerStart();
#endif
}

void plcTimerStop(void)
{
    /* ���ö�ʱ��ֹͣ */
#ifdef RT_USING_DEVICE
    rt_device_control(plc_hwtimer_dev, HWTIMER_CTRL_STOP, (void*)0);
#else
    extern void devTimerStop(void);
    devTimerStop();
#endif
}

/**
  * @brief ��ȡPLC����ʱ��
  */
unsigned int plcTimeMsecGet(void)
{
    unsigned int msec = 0;
#ifdef RT_USING_DEVICE
    rt_hwtimerval_t time;
    /* ��ȡ��ʱ����ǰֵ */
    rt_device_read(plc_hwtimer_dev, 0, &time, sizeof(time));
    msec = (unsigned int)time.sec * 1000 + time.usec / 1000;
#else
    rt_base_t level = rt_hw_interrupt_disable();
    msec = PlcTimeMsec;
    rt_hw_interrupt_enable(level);
#endif
    return msec;
}

/**
  * @brief ��ȡPLC΢��ʱ��
  */
unsigned int plcTimeUsecGet(void)
{
    unsigned int usec = 0;
    unsigned int usec1 = 0;
    rt_base_t level;
#ifdef RT_USING_DEVICE
    rt_hwtimerval_t time;
    rt_hwtimerval_t time1;
    level = rt_hw_interrupt_disable();
    /* ��ȡ��ʱ����ǰֵ */
    rt_device_read(plc_hwtimer_dev, 0, &time, sizeof(rt_hwtimerval_t));
    rt_hw_interrupt_enable(level);

    level = rt_hw_interrupt_disable();
    /* �ٴζ�ȡ��ʱ����ǰֵ */
    rt_device_read(plc_hwtimer_dev, 0, &time1, sizeof(rt_hwtimerval_t));
    rt_hw_interrupt_enable(level);

    usec = (unsigned int)time.sec * 1000000 + (unsigned int)time.usec;
    usec1 = (unsigned int)time1.sec * 1000000 + (unsigned int)time1.usec;
#else
    extern unsigned int devTimerCountGet(void);
    level = rt_hw_interrupt_disable();
    usec = PlcTimeMsec * 1000 + devTimerCountGet();
    rt_hw_interrupt_enable(level);
    /*�˳��ٽ����������ʱ���жϵ����������Ӧ�жϣ�����PlcTimeMSec��*/
    level = rt_hw_interrupt_disable();
    usec1 = PlcTimeMsec * 1000 + devTimerCountGet();
    rt_hw_interrupt_enable(level);
#endif
    if(usec1 > usec){
        usec = usec1;
    }
    return usec;
}

/**
  * @ brief �������ʼ�������ĺ�ʱ
  */
unsigned int timeConsumedCalc(unsigned int begin, unsigned int end)
{
	return (end - begin);
}

