/**
  * @file  plc_mem.c
  * @brief  PLC�û��洢ӳ������ص�ʵ��
  * @author  hyafz
  */
#include "plc_mem.h"
#include <string.h>

BYTE I[MAX_I_RANGE];	/**< ����ӳ���� */
BYTE Q[MAX_Q_RANGE];	/**< ���ӳ���� */
BYTE M[MAX_M_RANGE];	/**< �洢ӳ���� */

/**
  * @brief PLC���롢������洢ӳ������ʼ��
  */
void plcMemInit(void)
{
	memset((void*)I, 0, sizeof(I));
	memset((void*)Q, 0, sizeof(Q));
	memset((void*)M, 0, sizeof(M));
}



