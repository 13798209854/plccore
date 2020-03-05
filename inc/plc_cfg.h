/**
 * @file  plc_cfg.h
 * @brief plc_cfg.c��ͷ�ļ�
 * @author hyafz  
 */

#ifndef _PLC_CFG_H_
#define _PLC_CFG_H_

#include <rtthread.h>
#include "plc_type_define.h"
#include "plc_port.h"

typedef enum Task_Status_Enum{
	TASK_UNAVAILABLE = 0,
	TASK_READY = 1,
	TASK_RUNNING = 2,
	TASK_ERROR = 3
}TASK_STA_E;

/* ����ʱ��ͳ�ƽṹ�� */
typedef struct Task_Time_Statistics_Struct{
	USINT       priority;    /* ����������� */
	UDINT       trigTime;    /* ���񴥷���ʱ�� */
	UDINT       beginTime;   /* ����ʼִ�е�ʱ�� */
	UDINT       endTime;     /* ����ִ�н�����ʱ�� */
	UDINT       runTime;     /* �������е�ʱ�� */
	struct Task_Time_Statistics_Struct* pNext;
}TASK_TIME_STAT_S;

/*����PLC������ƿ�ṹ��*/
typedef struct Task_Ctrl_Block_Struct{
    UINT        id;
	USINT       priority;              /**< ���ȼ���0��ߣ�255���*/
    BOOL (*singleGet)(void);       /**< ���single��ֵ*/
    TIME (*intervalGet)(void);     /**< ���interval��ֵ*/
	void (*taskRun)(void);
    /*����ʱ����*/
	TASK_STA_E  taskSta;
	struct rt_semaphore	semTask;
	BOOL        singlePrevious;    /**< ǰһ��single����ֵ�������ж�single�����������*/
	TIME        intervalBegin;     /**< interval��ʼʱ��*/

	/* ����ͳ�� */
	UDINT       trigCnt;		   /**< �������� */
	UDINT       runCnt;            /**< ���д��� */

	/*����ʱ��ͳ�ƣ�����΢��(usec)Ϊ��λ*/
	UDINT       trigTime;          /**<  �������񴥷�ʱ�� */
	UDINT       beginTime;         /**<  �����������п�ʼʱ�� */
	UDINT       endTime;           /**<  �����������н���ʱ�� */
	
	UDINT       runTime;           /**<  ������������ʱ�� = endTime - beginTime */
	UDINT       runTimeMin;
	UDINT       runTimeMax;
	UDINT       runTimeAverage;
	UINT64      runTimeSum;
	
	UDINT       gapTime;           /**<  ��������������ʱ�� =     current beginTime - previous beginTime */
	UDINT       gapTimeMin;
	UDINT       gapTimeMax;       
	UDINT       gapTimeAverage;
	UINT64      gapTimeSum;
	
	UDINT       delayTime;         /**<  ��������Ӵ�������ʼ���е��ӳ�ʱ�� = beginTime - trigTime */
	UDINT       delayTimeMin;
	UDINT       delayTimeMax;
	UDINT       delayTimeAverage;
	UINT64      delayTimeSum;	
	/* ջ�ռ���ʹ�õ����ٷֱ�ͳ�� */
	USINT       stkUsedMax;
}TASK_CB_S;

#define MAX_SIZE_OF_RSC_MSG_QUEUE      16  /*��Դ�¼������������Ϣ��Ŀ*/

/*����PLC��Դ�¼�*/
#define PLC_RSC_EVT_NULL               0
#define PLC_RSC_EVT_ORDER_TO_RUN       1  /*������Դ����*/
#define PLC_RSC_EVT_ORDER_TO_STOP      2  /*������Դֹͣ*/
#define PLC_RSC_EVT_TASK_READY         3  /*����׼������*/
#define PLC_RSC_EVT_TASK_TRIGGED       4  /*���񴥷�*/
#define PLC_RSC_EVT_TASK_RETURN        5  /*����ִ�н�������*/
#define PLC_RSC_EVT_TASK_ERROR         6  /*����ִ�г���*/

typedef struct{
	BYTE       evt;                          /*�¼�����*/
	BYTE       reserved[3];                  /*�����������ֽڶ���*/
    TASK_CB_S* ptaskcb;                      /*  */
}RSC_MSG_S;
#define RSC_MSG_SIZE                   sizeof(RSC_MSG_S)

typedef enum Resource_Status_Enum{
	RSC_INITIALIZING = 0,	/**< ���ڳ�ʼ��״̬ */
	RSC_READY_TO_RUN,		/**< ׼������״̬ */
	RSC_STOP,				/**< ͣ��״̬ */
	RSC_RUNNING,			/**< ����״̬ */
	RSC_GOTO_STOP,    		/**< ����ֹͣ״̬ */
	RSC_ERROR				/**< ����״̬ */
}RSC_STA_E;

/*������Դ���ƿ�*/
typedef struct Resource_Ctrl_Block_Struct{
    UDINT       taskNum;                /**< �û�������Ŀ��������Ĭ������ */
	TASK_CB_S   taskCB[MAX_TASK_NUM_PER_RSC]; /**< �û�������ƿ��б�������Ĭ������ */
    TASK_CB_S   taskDefaultCB;          /**< Ĭ��������ƿ� */
    /*����ʱ����*/
	RSC_STA_E   rscState;			/**< ��Դ����״̬ */
	UDINT       cycleCnt;         /**< ɨ��ѭ���������ӵ��ο�ʼ���м��� */
	UDINT       taskRunningCnt;   /**< �������е�������Ŀ������Ĭ������ */
	
	/*���ʱ��صı���������΢��(usec)Ϊ��λ*/
	UDINT       runTime;           /**< ������������ʱ�� */
	UDINT       stopTime;          /**< ��������ͣ����ʱ�� */
	UDINT       updateTime;        /**< �ϴ�����״̬����ʱ�� */
	/* ��ѭ����ʱ */
	UDINT       cycleBegin;        /**< ������ѭ����ʼʱ�� */
	UDINT       cycleEnd;          /**< ������ѭ������ʱ�� */
	
	UDINT       cycleTime;         /**< ������ѭ������ʱ�� = endTime - beginTime */
	UDINT       cycleTimeMin;      /**< ������ѭ���������ʱ�� */
	UDINT       cycleTimeMax;      /**< ������ѭ�������ʱ�� */
	UDINT       cycleTimeAverage;  /**< ������ѭ��ƽ������ʱ�� */
	UINT64      cycleTimeSum;
	
	UDINT       gapTime;	       /**< ����������ѭ�����ʱ�� = current cycleBegin - perious cycleBegin */
	UDINT       gapTimeMin;        /**< ����������ѭ����̼��ʱ�� */
	UDINT       gapTimeMax;        /**< ����������ѭ������ʱ�� */
	UDINT       gapTimeAverage;    /**< ����������ѭ��ƽ�����ʱ�� */
	UINT64      gapTimeSum;
	/* �����ʱ */
	UDINT       diRefreshTime;    /**< DIˢ�º�ʱ */
	UDINT       dqRefreshTime;    /**< DQˢ�º�ʱ */

	TASK_TIME_STAT_S* ttsListHeader;  /**< �����������������ʱ��ͳ�Ʊ�ͷָ�� */
	
}RSC_CB_S;

typedef enum Configuration_Status_Enum{
	CFG_UNAVAILABLE = 0,
	CFG_READY = 1,
	CFG_RUNNING = 2,
	CFG_ERROR = 3
}CFG_STA_E;

/*�������ÿ��ƿ�*/
typedef struct{
	RSC_CB_S    rscCB;                         /*��ǰ��Դ���ƿ�*/
}CFG_CB_S;

extern CFG_CB_S CfgCB;

/* Export function declaration ----------------------------------------------*/
void plcCfgRun(void);
void plcRscTaskSched(RSC_CB_S* pRscCB);
void plcSendMsgToRsc(BYTE evt, TASK_CB_S* pTaskCB);

#endif

