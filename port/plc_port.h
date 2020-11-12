/**
 * @file plc_port.h
 * @brief ��Դ�������ӿ����ѡ�����ã�������ͨ���޸ı��ļ����Լ��Ĵ��������вü����ơ�
 * @author fengzhou
 */

#ifndef PLC_PORT_H
#define PLC_PORT_H

#include <rtthread.h>
#ifdef RT_USING_DEVICE
#define PLC_UART_NAME          "uart1"      /* �����豸���� */
#define PLC_HWTIMER_DEV_NAME   "timer3"     /* ��ʱ���豸���� */
#endif

/* ����-����ܽ���غ궨�� */
/* �����豸 */
#define RUN_SW_PIN      GET_PIN(A, 0)
#define IX0_PIN         GET_PIN(B, 0)
#define IX1_PIN         GET_PIN(A, 7)
#define IX2_PIN         GET_PIN(A, 6)
#define IX3_PIN         GET_PIN(A, 5)
#define IX4_PIN         GET_PIN(A, 4)
#define IX5_PIN         GET_PIN(A, 3)
#define IX6_PIN         GET_PIN(A, 2)
#define IX7_PIN         GET_PIN(A, 1)

#define DI_INIT_INFO \
{ /*    pin, default_state, prev_input_state, state, state_debounce_time, state_stay_time*/ \
    { RUN_SW_PIN,   1,   1,   1,   0,  0}, /* RUN Switch */ \
    { IX0_PIN,      1,   1,   1,   0,  0}, /* IX0_PIN */ \
    { IX1_PIN,      1,   1,   1,   0,  0}, /* IX1_PIN */ \
    { IX2_PIN,      1,   1,   1,   0,  0}, /* IX2_PIN */ \
    { IX3_PIN,      1,   1,   1,   0,  0}, /* IX3_PIN */ \
    { IX4_PIN,      1,   1,   1,   0,  0}, /* IX4_PIN */ \
    { IX5_PIN,      1,   1,   1,   0,  0}, /* IX5_PIN */ \
    { IX6_PIN,      1,   1,   1,   0,  0}, /* IX6_PIN */ \
    { IX7_PIN,      1,   1,   1,   0,  0}, /* IX7_PIN */ \
}

/* ����豸 */
#define LED_RUN_PIN     GET_PIN(C, 15)
#define LED_STOP_PIN    GET_PIN(C, 14)
#define LED_ERR_PIN     GET_PIN(C, 13)
#define QX0_PIN         GET_PIN(A, 15)
#define QX1_PIN         GET_PIN(B, 3)
#define QX2_PIN         GET_PIN(B, 4)
#define QX3_PIN         GET_PIN(B, 5)
#define QX4_PIN         GET_PIN(B, 6)
#define QX5_PIN         GET_PIN(B, 7)
#define DO_INIT_INFO \
{ /* pin, off_value, on_value, out_value */ \
    { LED_RUN_PIN,   0 , 1 , 0 }, /* LED RUN */ \
    { LED_STOP_PIN,  0 , 1 , 0 }, /* LED STOP */ \
    { LED_ERR_PIN,   0 , 1 , 0 }, /* LED ERR */ \
    { QX0_PIN,       0 , 1 , 0 }, /* QX0 */ \
    { QX1_PIN,       0 , 1 , 0 }, /* QX1 */ \
    { QX2_PIN,       0 , 1 , 0 }, /* QX2 */ \
    { QX3_PIN,       0 , 1 , 0 }, /* QX3 */ \
    { QX4_PIN,       0 , 1 , 0 }, /* QX4 */ \
    { QX5_PIN,       0 , 1 , 0 }, /* QX5 */ \
}

#define MAX_I_RANGE             256         /**< ����ӳ�����ֽ��� */
#define MAX_Q_RANGE             256         /**< ���ӳ�����ֽ��� */
#define MAX_M_RANGE             256         /**< �ڴ�ӳ�����ֽ��� */

#define LOC_DI_NUM              8           /**< ����8������������ */
#define LOC_DQ_NUM              6           /**< ����6����������� */

#define LOC_AI_NUM              1
#define LOC_AQ_NUM              1

#define LOC_PWM_NUM             2           /**< ����PWM���ͨ����Ŀ */
#define LOC_PWM_CTRL_BASE       248         /**< ����PWM������Ʋ�����Q������ʼӳ���ַ�����Ʋ�����ռ��4�ֽڣ�
                                                 Byte 1 ~ 2: PWM���ڣ���λus��Ϊ0��ֹͣ���
                                                 Byte 3    : PWMռ�ձ� 0 ~ 100��Ϊ0��ֹͣ���
                                                 Byte 4    : PWMģʽ */

/**
  * @brief ֧�ֵ���������ʹ��ѡ��
  <pre>
  ���л����������Ϳ��������Ƿ�֧��,����������������Ĭ��֧�֡�
	LINT
	ULINT
	REAL
	LREAL
	LWORD
	STRING
	WSTRING
	TIME
	DATE
	TOD
	DT
  �����������Ϳ��������Ƿ�֧��
  </pre>
  */
#define DT_REAL_EN                     1

#define DT_64BIT_EN                    0  /**< ֧��64λ�����������͵�ѡ��(0: ��֧��, 1: ֧��) */
#if (DT_64BIT_EN > 0)
#define DT_LINT_EN                     0
#define DT_ULINT_EN                    0
#define DT_LREAL_EN                    0
#define DT_LWORD_EN                    0
#else
#define DT_LINT_EN                     0
#define DT_ULINT_EN                    0
#define DT_LREAL_EN                    0
#define DT_LWORD_EN                    0
#endif

#define DT_STRING_EN                   0
#define DT_WSTRING_EN                  0
#define DT_TIME_EN                     1
#define DT_DATE_EN                     0
#define DT_TOD_EN                      0
#define DT_DT_EN                       0

#define DT_DVD_EN                      0  /**< ֧�ֵ����������͵�ѡ��(0: ��֧��, 1: ֧��) ��Ŀǰ����Ϊ0*/

/*��������ص�ѡ��*/

/*����Դ��ص�ѡ��*/
#define MAX_TASK_NUM_PER_RSC            4   /**< ÿ����Դ����������������Ŀ*/
#define PLC_TASK_RUN_TASK_STK_SIZE      512
#define PLC_TASK_HIGHEST_PRIORITY       2
#define PLC_TASK_LOWEST_PRIORITY        30
/*��POU��ص�ѡ��*/

/* ������������ص�ѡ�� */

/* ����������ص�ѡ�� */

/* ��IL������ص�ѡ�� */

/* ��LD������ص�ѡ�� */

/* ����ѡ�� */

#endif /* PLC_PORT_H */
