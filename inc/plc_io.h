/**
  * @file  plc_io.h
  * @brief     plc_io.c��ͷ�ļ�
  * @author    hyafz
  */

#ifndef _PLC_IO_H
#define _PLC_IO_H

#include <rtthread.h>
#include "board.h"
#include "plc_port.h"

/* define -------------------------------------------------------------------*/

#define PLC_SW_STOP             0    /* PLC���ش���ͣ��״̬ */
#define PLC_SW_RUN              1    /* PLC���ش�������״̬ */

#define DEV_OUTPUT_OFF          0
#define DEV_OUTPUT_ON           1

#define DEV_INPUT_NUM       (1 + LOC_DI_NUM)
#define DEV_OUTPUT_NUM      (3 + LOC_DQ_NUM)

/* Input/Output Device IDs */
#define DEV_RUN_SW_ID           0
#define DEV_IX0_ID              1

#define DEV_LED_RUN_ID          0
#define DEV_LED_STOP_ID         1
#define DEV_LED_ERR_ID          2
#define DEV_QX0_ID              3

#define DEV_FIRST_OUTPUT_ID     DEV_LED_RUN_ID
#define DEV_LAST_OUTPUT_ID      (DEV_QX0_ID + LOC_DQ_NUM - 1)

extern unsigned char devInputGet(unsigned int x);
extern void devOutputSet(int8_t dev_id, uint8_t sw);
#define LED_RUN_ON()            devOutputSet(DEV_LED_RUN_ID, DEV_OUTPUT_ON)
#define LED_RUN_OFF()           devOutputSet(DEV_LED_RUN_ID, DEV_OUTPUT_OFF)
// #define LED_ERR_ON()             devOutputSet(DEV_LED_ERR_ID, DEV_OUTPUT_ON)
// #define LED_ERR_OFF()            devOutputSet(DEV_LED_ERR_ID, DEV_OUTPUT_OFF)

/* typedef ------------------------------------------------------------------*/
/**
  * @struct Device Input_Control_Struct
  * @brief �����豸���ƽṹ�����Ͷ���
  */
typedef struct Device_Input_Control_Struct{
    GPIO_TypeDef*       gpio;                     /**< �豸��Ӧ��GPIO */
    uint16_t            pin;                      /**< �豸��Ӧ��PIN */
    GPIO_PinState       default_state;            /**< �����豸��Ĭ��״̬ */
    GPIO_PinState       prev_input_state;         /**< ��һ��������״̬ */
    GPIO_PinState       state;                    /**< �豸��ǰ��Ч״̬ */
    uint8_t             state_debounce_time;      /**< �л�״̬ȥ������ */
    uint32_t            state_stay_time;          /**< ��ǰ״̬����ʱ�� */
}DEV_IN_CTRL_S;

/**
  * @struct Device Output_Control_Struct
  * @brief ����豸���ƽṹ�����Ͷ���
  */
typedef struct Device_Output_Control_Struct{
    GPIO_TypeDef*       gpio;                     /**< �豸��Ӧ��GPIO */
    uint16_t            pin;                      /**< �豸��Ӧ��PIN */
    GPIO_PinState       off_value;                /**< ����ر�ʱGPIO PIN״̬ */
    GPIO_PinState       on_value;                 /**< �����ʱGPIO PIN״̬ */
    GPIO_PinState       out_value;                /**< ��ǰ���GPIO PIN״̬ */
}DEV_OUT_CTRL_S;

/* Export variables declaration ---------------------------------------------*/
/* Export function declaration ----------------------------------------------*/
void devIOInit(void);
void devInputScan(void);
int8_t isRunMode(void);
int8_t isRunModeSwitched(void);
void plcLocalDiRefresh(void);
void plcLocalDqRefresh(void);
void plcLocalDqOutputWhenStopped(void);
void plcLocalAiRefresh(void);
void plcLocalAqRefresh(void);


#endif

