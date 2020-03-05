/**
  * @file  plc_io.c
  * @brief  PLC�����������ص�ʵ��
  * @author  hyafz
  */
/* includes -----------------------------------------------------------------*/
#include <string.h>
#include <rtthread.h>
#include "plc_type_define.h"
#include "plc_mem.h"
#include "plc_io.h"

/* Private define -----------------------------------------------------------*/
#define DEV_INPUT_SCAN_GAP_TIME     10  /**< �����豸ɨ����ʱ�䣬��λΪSysTick */
#define DEV_INPUT_DEBOUNCE_TIMES    3   /**< �����豸���ȥ������ */
/* Private typedef ----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
extern DEV_IN_CTRL_S DevInput[];
extern DEV_OUT_CTRL_S DevOutput[];
/* Private function declaration ---------------------------------------------*/
/* Functions ----------------------------------------------------------------*/
extern unsigned char devInputPinRead(int id);
extern void devOutputPinWrite(int id);

/**
  * @brief ����IO�豸�ĳ�ʼ��
*/
void devIOInit(void)
{
    int i;

    for(i = 0; i < DEV_OUTPUT_NUM; i++){
        /* Ϊ����豸���ó�ʼ״̬ */
        DevOutput[i].out_value = DevOutput[i].off_value;
        devOutputPinWrite(i);
    }
}

/**
  * @todo ����������������Ϊ��̬�ľֲ�������ʹ��gcc�������map�ļ���û��Ϊ�������ַ��
  *       ��keil��ʹ��armcc�������Ϊ������һ��data����ַ����������´�����HardFault����Ҫ�������ԭ��
  */
static int ScanTimeCnt = 0;         /* ɨ��ʱ����� */
/**
  * @brief �����豸״̬ɨ��
  */
void devInputScan(void)
{
    int i;
    GPIO_PinState curr_input_state;

    ScanTimeCnt++;
    if(ScanTimeCnt < DEV_INPUT_SCAN_GAP_TIME){
        return;
    }
    ScanTimeCnt = 0;

    for(i = 0; i < DEV_INPUT_NUM; i++){
        curr_input_state = devInputPinRead(i);
        if(curr_input_state == DevInput[i].state){
            //����״̬�뵱ǰ״̬һ�£�û�б仯���ж���һ������㡣
            continue;
        }
        //����״̬�뵱ǰ״̬��һ��
        if(curr_input_state == DevInput[i].prev_input_state){
            //����״̬����һ������״̬һ�£��Ѵ���ȥ���׶�
            if(DevInput[i].state_debounce_time < DEV_INPUT_DEBOUNCE_TIMES){
                //ȥ��δ��ɣ�����ȥ��
                DevInput[i].state_debounce_time++;
            }else{
                //ȥ����ɣ�״̬��Ч
                DevInput[i].state = curr_input_state;
            }
        }else{
            //����״̬����һ������״̬��һ�£���ʼȥ��
            DevInput[i].prev_input_state = curr_input_state;
            DevInput[i].state_debounce_time = 0;
        }
    }
}

/**
  * @brief  ��ȡָ�������Ӧ�Ĺܽ�״̬�ĺ���
  * @param  x DI���
  * @return 0 or 1
  */
unsigned char devInputGet(unsigned int x)
{
    return (DevInput[x].state != DevInput[x].default_state);
}

/**
  * @brief ��������豸״̬
  * @param dev_id ����豸ID
  * @param sw ״̬���� 0 �رգ�1 ��
  */
void devOutputSet(int8_t dev_id, uint8_t sw)
{
  if((dev_id < DEV_FIRST_OUTPUT_ID) || (dev_id > DEV_LAST_OUTPUT_ID)){
    return;
  }

  if(sw == DEV_OUTPUT_ON){
    DevOutput[dev_id].out_value = DevOutput[dev_id].on_value;
  }else{
    DevOutput[dev_id].out_value = DevOutput[dev_id].off_value;
  }
  devOutputPinWrite(dev_id);
}

/**
  * @brief ����Ƿ�������ģʽ������1��ʾ��������ģʽ������0��ʾ����ֹͣģʽ��
  * @retval 1 ��������ģʽ
  * @retval 0 ����ֹͣģʽ
  */
int8_t isRunMode(void)
{
	int8_t run_mode = 0;
	if(DevInput[DEV_RUN_SW_ID].state != DevInput[DEV_RUN_SW_ID].default_state){
		run_mode = 1;
	}else{
		run_mode = 0;
	}
	run_mode = 1;   //test
	return run_mode;
}

static int8_t PrevRunMode = -1;
int8_t isRunModeSwitched(void)
{
	int8_t mode_switched = 0;
	uint8_t run_mode = isRunMode();
	if(PrevRunMode != run_mode){
		if(PrevRunMode != -1){
			mode_switched = 1;
		}
		PrevRunMode = run_mode;
	}
	return mode_switched;
}

int LedTogglePeriod = 0;
/**
  * @brief ����LED��˸���ڣ��������Ϊ0��ʾ����˸��
  */
void ledToggleSet(int nms)
{
	LedTogglePeriod = nms;
}

static int ToggleTimeCnt = 0;
static uint8_t ToggleState = 0;
void ledToggleUpdate(void)
{
	
	if(LedTogglePeriod == 0){
		LED_RUN_OFF();
		return;
	}
	
	ToggleTimeCnt++;
	if(ToggleTimeCnt >= LedTogglePeriod){
		ToggleTimeCnt = 0;
		if(ToggleState == 0){
			ToggleState = 1;
			//LED_RUN_ON();
		}else{
			ToggleState = 0;
			//LED_RUN_OFF();
		}
	}
}

/**
  * @brief PLC���������������ӳ����ˢ�¡�
  */
void plcLocalDiRefresh(void)
{
#if (OS_CRITICAL_METHOD == 3)
	OS_CPU_SR cpu_sr = 0u;
#endif

	int x;
	unsigned char diTmp;
	unsigned int offset;

	rt_enter_critical();
	for(x = 0; x < LOC_DI_NUM; x++){
		offset = x % 8;
		if(offset == 0){
			diTmp = 0;
		}
		diTmp |= (devInputGet(DEV_IX0_ID + x) << offset);
		if((offset == 7) || (x == LOC_DI_NUM - 1)){
			I[x / 8] = diTmp;
		}
	}
	rt_exit_critical();
}

/**
  * @brief PLC���������������״̬ˢ�¡�
  */
void plcLocalDqRefresh(void)
{
	unsigned int x;
	for(x = 0; x < LOC_DQ_NUM; x++)
	{
	    devOutputSet(DEV_QX0_ID + x, (Q[(x >> 3)] >> (x & 0x07)) & 0x01);
	}
}

/**
  * @brief PLC�����������������ͣ��ʱȫ������͡�
  */
void plcLocalDqOutputWhenStopped(void)
{
	memset((void*)Q, 0, sizeof(Q));
	plcLocalDqRefresh();
}


