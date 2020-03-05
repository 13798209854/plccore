/**
  * @file  plc_comm.c
  * @brief  PLCͨ�Ź��ܵ�ʵ��
  * @author hyafz   
  */
/* includes -----------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <rtthread.h>
#include "plc_port.h"
#include "plc_debug.h"
#include "plc_cfg.h"
#include "plc_timer.h"
#include "plc_io.h"
#include "plc_stat.h"
#include "plc_comm.h"

/* Private define -----------------------------------------------------------*/
/* Private typedef ----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
#define MIN_CMD_FRAME_LENGTH            6	/**< ֡ͷ(2bytes) + ֡��(2bytes) + cmd(1byte) + data(0bytes) + checksum(1byte) */
#define CMD_RESPONSE_FRAME_MAX_LENGTH	32

/** ���ճ�ʱʱ������λms��        ÿ���������һ��ÿ�յ�һ���ֽڽ���ʱ�������㡣
  * �������������ʱֵ����Ϊ�������жϣ������ռ������㣬��ֹĳһ֡���մ���Ӱ����һ֡�Ľ��ա�
  * ����С����λ���ط����ʱ����ԭ��
  * ��λ��������һ֡�󣬵ȴ���λ����Ӧ��
  * �����λ��û���յ�������֡���򲻻�Ӧ�𣬹��˳�ʱʱ������������ջ����������յ����ֽڣ�������ǰ֡���ȴ�������һ֡��
  * ��λ��δ�յ�Ӧ����������ط���ʹ�շ����ָ̻�����״̬��
  * ����ط����ʱ��С�ڽ��ճ�ʱʱ��������λ���Ὣ�ط�������
  * �뻺�����ѽ���������ϳ�һ�������֡��ͬʱ�ƻ�����֡���ݡ�
  */
#define RX_TIMEOUT  1000

/* Variables ----------------------------------------------------------------*/
uint8_t CommTxBuf[TX_DATA_BUF_SIZE];
int CommTxLength = 0;
uint8_t CommTxFinished = 0;

unsigned short CommRxData;
unsigned char CommRxBuf[RX_DATA_BUF_SIZE];
unsigned int CommRxCnt = 0;
int CommRxTimeCnt = 0;
/* Private function declaration ---------------------------------------------*/
/* Functions ----------------------------------------------------------------*/
void devCommSend(unsigned char* pbuf, unsigned int size);

void commRxTimeoutCheck(void)
{
    rt_base_t level;
    CommRxTimeCnt++;
    if(CommRxTimeCnt > RX_TIMEOUT){
        CommRxTimeCnt = 0;
        level = rt_hw_interrupt_disable();
        CommRxCnt = 0;
        rt_hw_interrupt_enable(level);
    }
}

/**
 * @brief ����У���
 * @param frame
 * @return
 */
unsigned char checkSumCalc(unsigned char* frame, int length)
{
    unsigned char checksum = 0;
	int i;
    for(i = 2; i < length - 1; i++){
        checksum += frame[i];
    }
    return checksum;
}

/**
  * @brief ���ɲ���������֡
  * @param cmd ������
  * @param data ��������
  * @param length �������ݳ���
  */
void dataFrameSend(unsigned char cmd, unsigned char* data, int length)
{
	int i;
	unsigned char checksum = 0;
	int frameLen = MIN_CMD_FRAME_LENGTH + length;
	unsigned char buf[5];
	buf[0] = 0xa5;
	buf[1] = 0x5a;
	buf[2] = (unsigned char)(frameLen & 0xFF);
	buf[3] = (unsigned char)((frameLen >> 8) & 0xFF);
	buf[4] = cmd;
	for(i = 2; i < 5; i++){
		checksum += buf[i];
	}
	for(i = 0; i < length; i++){
		checksum += data[i];
	}
	/* �����ַ��� */
	devCommSend(buf, 5);
    devCommSend(data, length);
    devCommSend(&checksum, 1);
}

#define CMD_BUF_SIZE	64
static char CmdBuf[CMD_BUF_SIZE];
static unsigned int CmdBufLength = 0;
static unsigned char CmdIsProcessing = 0;
/**
  * @brief �����������
  * @param pbuf ����֡������
  * @param length ����֡����
  */
void cmdReceiveAndCheck(unsigned char* pbuf, int length)
{
	if((unsigned char)pbuf[length - 1] == checkSumCalc(pbuf, length)){
		//У��ͨ��
		memcpy((void*)CmdBuf, (void*)pbuf, length);
		CmdBufLength = length;
		CmdIsProcessing = 1;
	}else{
		CmdBufLength = 0;
		CmdIsProcessing = 0;
	}
}

void dataReceive(unsigned char data)
{
    rt_base_t level;
    unsigned int length = 0;
    unsigned char restartFlag = 0;

    CommRxTimeCnt = 0;    //������ڶ�֡�ĳ�ʱ����

    CommRxBuf[CommRxCnt] = data;
    CommRxCnt++;

    devCommRxStart();   //������������

    if(CommRxCnt > RX_DATA_BUF_SIZE){
        //���ճ������¿�ʼ���ա�
        restartFlag = 1;
    }else{
        /* ����֡��ʽ��֡ͷ2�ֽڣ�0xa5, 0x5a�� + ֡����2�ֽ� + ������1�ֽ� + ��������n��n >= 0)�ֽ� + У���1�ֽ� */
        if(((CommRxCnt == 1) && (CommRxBuf[0] != (char)0xa5))
            || ((CommRxCnt == 2) && (CommRxBuf[1] != (char)0x5a))){
            /* ֡ͷ����ȷ�����¿�ʼ���� */
            restartFlag = 1;
        }else if(CommRxCnt > 3){
            length = (unsigned int)CommRxBuf[2] \
                + ((unsigned int)CommRxBuf[3] << 8);
            if(length > RX_DATA_BUF_SIZE){
                /* ֡�����������֡������һ�ִ������¿�ʼ���� */
                restartFlag = 1;
            }else if(length == CommRxCnt){
                /* ����һ֡��ɣ����м���봦�� */
                cmdReceiveAndCheck(CommRxBuf, CommRxCnt);
                restartFlag = 1;
            }
        }
    }
    if(restartFlag == 1){
        memset(CommRxBuf, 0, CommRxCnt);
        level = rt_hw_interrupt_disable();
        CommRxCnt = 0;
        rt_hw_interrupt_enable(level);
    }
}

extern void iapModeEnter(void);
static unsigned char CmdDataBuf[CMD_RESPONSE_FRAME_MAX_LENGTH - MIN_CMD_FRAME_LENGTH];
/**
  * @brief �����
  */
void cmdProcess(void)
{	
	int dataLen = 0;
	unsigned char cmd = CMD_NONE;
	if(CmdIsProcessing == 0){
		return;
	}
	cmd = (unsigned char)CmdBuf[4];
	switch(cmd){
		case CMD_ENTER_IAP_MODE:
			if(isRunMode() == 1){
				//���ش�������ģʽ���������IAPģʽ
				CmdDataBuf[0] = 0x01;
				dataLen = 1;
			}else{
				//���ش���ֹͣģʽ�������IAPģʽ
			    iapModeEnter();
			}
			break;
		case CMD_PROGRAM_READY:
		case CMD_PROGRAMMING:
		case CMD_PROGRAM_OVER:
		case CMD_JUMP_TO_APP:
			/* IAPģʽ�²Ż��յ����������Ҫ���κδ�������ֻ��Ҫ�ظ���ǰ������IAPģʽ�� */
			break;
		default:
			dataLen = 0;
			break;
	}
	
	dataFrameSend(cmd, CmdDataBuf, dataLen);
	CmdIsProcessing = 0;
}

