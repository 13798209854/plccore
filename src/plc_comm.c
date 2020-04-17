/**
  * @file  plc_comm.c
  * @brief  PLCͨ�Ź��ܵ�ʵ��
  * @author hyafz   
  */
/* includes -----------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <rtthread.h>
#ifdef RT_USING_DEVICE
#include <rtdevice.h>
#endif
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

/* Variables ----------------------------------------------------------------*/
static struct rt_semaphore CommRxSem;  /* ���ڽ�����Ϣ���ź��� */
static struct rt_thread PlcCommThread;
#define PLC_COMM_THREAD_STK_SIZE    256
static unsigned char PlcCommThreadStk[PLC_COMM_THREAD_STK_SIZE];

#ifdef RT_USING_DEVICE
static rt_device_t CommDev;            /* PLC�����豸���  */
#endif

unsigned char CommRxBuf[RX_DATA_BUF_SIZE];
unsigned int CommRxCnt = 0;
int CommRxTimeCnt = 0;
/* Private function declaration ---------------------------------------------*/
/* Functions ----------------------------------------------------------------*/
void plcCommRxTimeoutCheck(void)
{
/** ���ճ�ʱʱ������λms��        ÿ���������һ��ÿ�յ�һ���ֽڽ���ʱ�������㡣
      * �������������ʱֵ����Ϊ�������жϣ������ռ������㣬��ֹĳһ֡���մ���Ӱ����һ֡�Ľ��ա�
      * ����С����λ���ط����ʱ����ԭ��
      * ��λ��������һ֡�󣬵ȴ���λ����Ӧ��
      * �����λ��û���յ�������֡���򲻻�Ӧ�𣬹��˳�ʱʱ������������ջ����������յ����ֽڣ�������ǰ֡���ȴ�������һ֡��
      * ��λ��δ�յ�Ӧ����������ط���ʹ�շ����ָ̻�����״̬��
      * ����ط����ʱ��С�ڽ��ճ�ʱʱ��������λ���Ὣ�ط�������
      * �뻺�����ѽ���������ϳ�һ�������֡��ͬʱ�ƻ�����֡���ݡ�
      * Ŀǰ��λ���ط�ʱ������Ϊ3000ms
*/
#define RX_TIMEOUT  1000
    rt_base_t level;
    CommRxTimeCnt++;
    if(CommRxTimeCnt > RX_TIMEOUT){
        CommRxTimeCnt = 0;
        level = rt_hw_interrupt_disable();
        CommRxCnt = 0;
        rt_hw_interrupt_enable(level);
#ifndef RT_USING_DEVICE
        extern void devCommRxStart(void);
        devCommRxStart();
#endif
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
void plcCommDataFrameSend(unsigned char cmd, unsigned char* data, int length)
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
#ifdef RT_USING_DEVICE
	rt_device_write(CommDev, 0, buf, 5);
    rt_device_write(CommDev, 0, data, length);
    rt_device_write(CommDev, 0, &checksum, 1);
#else
    extern void devCommSend(unsigned char* pbuf, unsigned int size);
    devCommSend(buf, 5);
    devCommSend(data, length);
    devCommSend(&checksum, 1);
#endif
}

#define CMD_BUF_SIZE    64
static char CmdBuf[CMD_BUF_SIZE];
extern void iapModeEnter(void);
static unsigned char CmdDataBuf[CMD_RESPONSE_FRAME_MAX_LENGTH - MIN_CMD_FRAME_LENGTH];
/**
  * @brief �����
  */
void cmdProcess(void)
{
    int dataLen = 0;
    unsigned char cmd = CMD_NONE;
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

    plcCommDataFrameSend(cmd, CmdDataBuf, dataLen);
}

void plcCommDataReceived()
{
    rt_base_t level;
    unsigned int length = 0;
    unsigned char restartFlag = 0;

    CommRxTimeCnt = 0;    //������ڶ�֡�ĳ�ʱ����

    if(CommRxCnt > RX_DATA_BUF_SIZE){
        //���ճ������¿�ʼ���ա�
        restartFlag = 1;
    }else if(CommRxCnt >= 2){
        /* ����֡��ʽ��֡ͷ2�ֽڣ�0xa5, 0x5a�� + ֡����2�ֽ� + ������1�ֽ� + ��������n��n >= 0)�ֽ� + У���1�ֽ� */
        if((CommRxBuf[0] != (char)0xa5) || (CommRxBuf[1] != (char)0x5a)){
            /* ֡ͷ����ȷ�����¿�ʼ���� */
            restartFlag = 1;
        }else if(CommRxCnt >= 4){
            length = (unsigned int)CommRxBuf[2] \
                + ((unsigned int)CommRxBuf[3] << 8);
            if(length > RX_DATA_BUF_SIZE){
                /* ֡�����������֡������һ�ִ������¿�ʼ���� */
                restartFlag = 1;
            }else if(length == CommRxCnt){
                /* ����һ֡��ɣ�����У���봦�� */
                if(CommRxBuf[CommRxCnt - 1] == checkSumCalc(CommRxBuf, CommRxCnt)){
                    //У��ͨ��
                    memcpy(CmdBuf, CommRxBuf, CommRxCnt);
                    rt_sem_release(&CommRxSem);
                }
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

#ifdef RT_USING_DEVICE
/* �������ݻص����� */
static rt_err_t plcCommRxIndicate(rt_device_t dev, rt_size_t size)
{
    int rxCnt = 0;
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
    if (size > 0)
    {
        rxCnt = rt_device_read(CommDev, 0, &CommRxBuf[CommRxCnt], size);
        if (rxCnt > 0){
            CommRxCnt += rxCnt;
            plcCommDataReceived();
        }
    }
    return RT_EOK;
}
#endif

/**
 * @brief PLCͨ�Ŵ����߳�
 */
static void plcCommProcess(void *parameter)
{
    while (1)
    {
        rt_sem_control(&CommRxSem, RT_IPC_CMD_RESET, RT_NULL);
        rt_sem_take(&CommRxSem, RT_WAITING_FOREVER);
        /* ͨ��ָ��� */
        cmdProcess();
    }
}

/**
 * @brief PLCͨ�ų�ʼ��������
 */
void plcCommInit(void)
{
    rt_err_t err;
    /* ��ʼ���ź��� */
    rt_sem_init(&CommRxSem, "comm_rx_sem", 0, RT_IPC_FLAG_FIFO);

#ifdef RT_USING_DEVICE
    err = RT_EOK;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* ��ʼ�����ò��� */
    /* ����ϵͳ�еĴ����豸 */
    CommDev = rt_device_find(PLC_UART_NAME);
    if (!CommDev)
    {
        rt_kprintf("find %s failed!\n", PLC_UART_NAME);
        return;
    }

    /* �޸Ĵ������ò��� */
    config.baud_rate = BAUD_RATE_115200;      //�޸Ĳ�����Ϊ 115200
    config.data_bits = DATA_BITS_8;           //����λ 8
    config.stop_bits = STOP_BITS_1;           //ֹͣλ 1
    config.bufsz     = RX_DATA_BUF_SIZE;      //�޸Ļ����� buff size
    config.parity    = PARITY_NONE;           //����żУ��λ

    /* ���ƴ����豸��ͨ�����ƽӿڴ�����������֣�����Ʋ��� */
    err = rt_device_control(CommDev, RT_DEVICE_CTRL_CONFIG, &config);
    if(err != RT_EOK){
        return;
    }

    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(CommDev, plcCommRxIndicate);

    /* ���жϽ��ռ���ѯ����ģʽ�򿪴����豸 */
    rt_device_open(CommDev, RT_DEVICE_FLAG_INT_RX);
#else
    extern void devCommInit(void);
    extern void devCommRxStart(void);
    devCommInit();
    /* PLC��ͨ�Ŷ˿ڽ������� */
    devCommRxStart();

#endif

    /* ��ʼ�� plc_comm_process �߳� */
    err = rt_thread_init(&PlcCommThread,
                         "plc_comm_process",
                         plcCommProcess,
                         (void*)0,
                         (void*)PlcCommThreadStk,
                         (unsigned int)PLC_COMM_THREAD_STK_SIZE,
                         1,
                         10);

    RT_ASSERT(RT_EOK == err);
    /* �����ɹ��������߳� */
    rt_thread_startup(&PlcCommThread);
}

