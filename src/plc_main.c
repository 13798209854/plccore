/**
  * @file  plc_main.c
  * @brief  ʵ�ַ���IEC61131��׼��PLC����ʱϵͳ
  * @author hyafz   
  */
/* includes -----------------------------------------------------------------*/
#include "plc_cfg.h"
#include "plc_comm.h"
#include "plc_timer.h"
#include "plc_io.h"
#include "plc_stat.h"

/* Private define -----------------------------------------------------------*/
/* Private typedef ----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
/* Private function declaration ---------------------------------------------*/
/* Functions ----------------------------------------------------------------*/
extern void plcProcessorInit(void);
/**
  * @brief PLC������
  */
void plcMain(void)
{
	/*��ʼ��PLC*/
    plcProcessorInit();

    devIOInit();

    /* PLC��ͨ�Ŷ˿ڽ������� */
    devCommRxStart();

    /*PLC����ʱ������*/
    plcTimerStart();

    /*��ʼ��ͳ�ƹ���*/
    statInit();

    /*����ͳ�ƹ���*/
    statBegin();
                          
	/*ִ��PLC�������к��������᷵�ء�*/
	plcCfgRun();
}



