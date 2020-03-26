/**
 * @file plc_port.h
 * @brief ��Դ�������ӿ����ѡ�����ã�������ͨ���޸ı��ļ����Լ��Ĵ��������вü����ơ�
 * @author hyafz
 */

#ifndef PLC_PORT_H
#define PLC_PORT_H

#define MAX_I_RANGE         256         /**< ����ӳ�����ֽ��� */
#define MAX_Q_RANGE         256         /**< ���ӳ�����ֽ��� */
#define MAX_M_RANGE         256         /**< �ڴ�ӳ�����ֽ��� */

#define LOC_DI_NUM          8           /**< ����8������������ */
#define LOC_DQ_NUM          6           /**< ����6����������� */

#define LOC_AI_NUM          1
#define LOC_AQ_NUM          1

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
#define MAX_TASK_NUM_PER_RSC           4   /**< ÿ����Դ����������������Ŀ*/
#define PLC_TASK_RUN_TASK_STK_SIZE		512
#define PLC_TASK_HIGHEST_PRIORITY           1
#define PLC_TASK_LOWEST_PRIORITY        30
/*��POU��ص�ѡ��*/

/* ������������ص�ѡ�� */

/* ����������ص�ѡ�� */

/* ��IL������ص�ѡ�� */

/* ��LD������ص�ѡ�� */

/* ����ѡ�� */

#endif /* PLC_PORT_H */
