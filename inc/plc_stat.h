/**
  * @file  plc_stat.h
  * @brief     plc_stat.c��ͷ�ļ�
  * @author    hyafz
  */

#ifndef _PLC_STAT_H
#define _PLC_STAT_H

#include "plc_port.h"

/* define -------------------------------------------------------------------*/
/* ��ͳ�ƹ�����ص�ѡ�� */
#define MAX_STAT_DATA_REGION_SIZE      300    /**< ͳ��������������ֽ��� */

#define STAT_DATA_NORMAL_MODE           0   /**< ��ͨģʽ����ͳ������ */
#define STAT_DATA_COMPACT_MODE			1   /**< ����ģʽ����ͳ������ */
#define STAT_DATA_SAVE_MODE             STAT_DATA_NORMAL_MODE   /**< ͳ�����ݱ���ģʽ */
/**
  * ͳ�����ݰ�����label + evt (+ time length) + time
  * ��ͨģʽ�£�ͳ�����ݽṹ���̶�6���ֽڣ���
  * Byte 1: 	label  	�¼������ǩ.
  *                		��������label = �������ȼ�
  *                		������Դ��label = ����������ȼ�PLC_TASK_LOWEST_PRIORITY + 1
  * Byte 2: 	evt    	�¼����ͣ�ȡֵ��Χ[0 ~ 255]���ܹ����Ա�ʾ256���¼�����ÿ���¼�������Զ���256���¼�����.
  * Byte 3 ~ 6 	time 	��ʾ��ǰ�¼����ϴ��¼���ʱ����������λ: ΢��(us).
  * ����ģʽ�£�ͳ�����ݽṹ�����ȿɱ䣩:
  * Byte 1: 	label  				�¼������ǩ.
  *                					��������label = �������ȼ�
  *                					������Դ��label = ����������ȼ�PLC_TASK_LOWEST_PRIORITY + 1
  * Byte 2: 	evt + time length  	�¼����� + ʱ���򳤶�.
  *                					��6λΪ�¼����ͣ�ȡֵ��Χ[0 ~ 63]���ܹ����Ա�ʾ64���¼�����ÿ���¼�������Զ���64���¼�����.
  *                					��2λΪʱ������ֽڳ��ȣ�ȡֵ��Χ[0 ~ 3]���ֱ��ʾ: 0(1�ֽ�)��1(2�ֽ�)��2(3�ֽ�)��3(4�ֽ�).
  * Byte 3 ~   	time   				ʱ�������1�ֽ�(time length == 0)���4�ֽ�(time length == 3)��
  *                                 ��ʾ��ǰ�¼����ϴ��¼���ʱ����������λ: ΢��(us).
  */

#define SINGLE_STAT_DATA_SIZE        6                 /**< ÿ��ͳ����������ֽ���. 
                                                            1byte(label) + 1byte(evt + time length) + 4bytes(time zone)  */

#define PLC_TASK_STAT_DATA_SIZE	(12 * 4)
#define PLC_RSC_STAT_DATA_SIZE	(8 * 4 + PLC_TASK_STAT_DATA_SIZE * (MAX_TASK_NUM_PER_RSC + 1))
#if ((PLC_RSC_STAT_DATA_SIZE + SINGLE_STAT_DATA_SIZE) > MAX_STAT_DATA_REGION_SIZE)
#error "Stat data region size is too small."
#endif
/* typedef ------------------------------------------------------------------*/
/**
  * @brief ͳ���¼�����ö��
  */
typedef enum Stat_Event_Type_Enum{
	SE_TYPE_NONE = 0,
	SE_RSC_RUN,
	SE_RSC_STOP,
	SE_RSC_UPDATE,
	SE_RSC_CYCLE_BEGIN,
	SE_RSC_CYCLE_OVER,
	SE_RSC_DI_REFRESH_BEGIN,
	SE_RSC_DI_REFRESH_OVER,
	SE_RSC_DQ_REFRESH_BEGIN,
	SE_RSC_DQ_REFRESH_OVER,
	SE_TASK_TRIGGED,
	SE_TASK_BEGIN,
	SE_TASK_OVER
}STAT_EVT_TYPE_E;
/* Export variables declaration ---------------------------------------------*/
/* Export function declaration ----------------------------------------------*/
void plcStatBssZeroTest(void);
void statInit(void);
void statBegin(void);
void statStop(void);
void statClear(void);
unsigned int statEventAdd(unsigned char label, STAT_EVT_TYPE_E evt);
unsigned int statDataAdd(unsigned char* data, unsigned int len);
void statOutput(void);

#endif

