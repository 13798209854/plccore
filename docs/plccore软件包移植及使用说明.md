# plccore�������ֲ��ʹ��˵��

## ׼��

1. RT-Thread Studio��������ص�ַ��https://www.rt-thread.org/page/studio.html��
2. Stm32CubeMX��������ص�ַ��https://www.st.com/content/st_com/zh/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stm32cubemx.html��
3. ������PLC IDE���ٶ���������: https://pan.baidu.com/s/1wCqSnEMLSoyQzc4SO8ZDXA ��ȡ��: aud4��

## ��ȡplccore�����

��ʽһ����github���أ�https://github.com/hyafz/plccore

## ʹ��RT-Thread Studio����Ŀ����BSP����

-�½�RT-Thread����
���빤������
ѡ�񹤳��ļ����·��
ѡ��nano�汾
ѡ�����оƬ
ѡ���Ӧ��оƬ�ͺ�
ѡ����Դ�ӡ�������
ѡ����Կڣ�JTAG or SWD��

## ʹ��Stm32CubeMX����Ŀ������ù���

����Ҫ��BSP���̷���ͬһĿ¼�£�
- ����RCCʱ��
- ��������λ��ͨ�ŵ�ר������
������115200��8λ����λ��1λֹͣλ����У��
������RTT���Դ�ӡ������ڲ�ͬ��
������Ӧ����ȫ���жϣ������ж����ȼ���
- ����PLCר��Ӳ����ʱ��
��Ƶ��ʱ��Ϊ1MHz����ʱֵ1000����1ms�ж�һ�Ρ�
������Ӧ��ʱ��ȫ���жϣ������ж����ȼ���
- ���ùܽ����ԣ���ת���ʡ��������ȣ�
����/ֹͣ����
����/����ָʾ��
���롢�����
- ���湤�̲����ɴ���

## ���Ʋ��޸�stm32f1xx_hal_msp.c�ļ�

��Stm32CubeMX���ɹ��̵�srcĿ¼�¸���stm32f1xx_hal_msp.c�ļ���BSP����driversĿ¼���������޸ġ�
��stm32f1xx_hal_msp.c�ļ��е�
```
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
```
�޸�Ϊ
```
void HAL_UART_MspInitEx(UART_HandleTypeDef* huart)
```
��Ϊdrv_usart.c���Ѿ�ʵ����HAL_UART_MspInit()�������ظ����塣
��
```
#include "main.h"
```
�޸�Ϊ��
```
#include "board.h"
```
## �޸�drv_usart.c

> ע�⣺�ٷ��ṩ��drv_usart.c�ļ�ʵ���е����⣬��Ҫ�޸ĺ����֧�ֶ�������豸��

��
```
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
```
����
```
#endif /* RT_USING_CONSOLONE */
```
��֮�󣬲��޸ĳ����£�

```
#endif /* RT_USING_CONSLONE */

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
#ifdef RT_USING_CONSOLE
    /* if this uart is shell function */
    if(huart == &handle)
    {
        stm32_gpio_configure(_uart_config);
        return;
    }
#endif
    extern void HAL_UART_MspInitEx(UART_HandleTypeDef *huart);
    HAL_UART_MspInitEx(huart);
}
```
## �޸�stm32f1xx_hal_conf.h�ļ�

�޸�BSP����driversĿ¼�µ�stm32f1xx_hal_conf.h��ʹ����Ӧ���������ģ�顣����ʹ�ܶ�ʱ��ģ��:
��
```
/*#define HAL_TIM_MODULE_ENABLED */
```
�޸�Ϊ
```
#define HAL_TIM_MODULE_ENABLED
```

## �޸�board.c�е�ʱ�����ú���

RT-Thread Studio���ɵĹ���Ĭ��ʹ��HSIʱ�ӣ�ʵ�����ʹ����HSE��LSEʱ�ӣ�Ӧ���޸�ʱ�����ã�������CubeMX�����úú󣬽����ɵ�main.c�е�SystemClock_Config()��������board.c�е�SystemClock_Config()������
```
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}
```
## ��board.c�������ж����������ú���

> ע�⣺�����ʹ��IAP����ͨ���������س�����Ŀ�������Ҫִ�д˲������ʹ��JLINK��ST-Link���س��������˲���

```
#define VECT_TAB_OFFSET  0x00006000U /*!< Vector Table base offset field.
                                  This value must be a multiple of 0x200. */
void VectorTable_Config(void)
{
#ifdef VECT_TAB_SRAM
  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif
}
```
## �޸�drv_common.c

> ע�⣺�����ʹ��IAP����ͨ���������س�����Ŀ�������Ҫִ�д˲������ʹ��JLINK��ST-Link���س��������˲���

�ں���rt_hw_board_init()�е���SystemClock_Config()֮�������������룺
```
    extern void VectorTable_Config(void);
    VectorTable_Config();
```
����
```
    ...
    /* enable interrupt */
    __set_PRIMASK(0);
    /* System clock initialization */
    SystemClock_Config();

    /* NVIC Vector table config */
    extern void VectorTable_Config(void);
    VectorTable_Config();

    /* disbale interrupt */
    __set_PRIMASK(1);
    ...
```
## �޸�link.lds

> ע�⣺�����ʹ��IAP����ͨ���������س�����Ŀ�������Ҫִ�д˲������ʹ��JLINK��ST-Link���س��������˲���

ROM ��ORIGIN��0x8000000�޸�Ϊ0x8006000��LENGTH����MCUʵ��ROM��С��д��

```
/* Program Entry, set to mark it as "used" and avoid gc */
MEMORY
{
    ROM (rx) : ORIGIN = 0x8006000, LENGTH =  232K /* 232K flash */
    RAM (rw) : ORIGIN = 0x20000000, LENGTH =  48k /* 48K sram */
}
```

## ���Ʋ��޸�plccore����

### ��plccore�ļ��и�����RTT Studio����Ŀ¼��

### ��examples/Ŀ¼�¼���main.c��ճ����RTT Studio����Ŀ¼��

main.c
```
/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-09     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

extern void plcMain(void);

int main(void)
{
    plcMain();

    while (1)
    {
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}

```

### �޸�plccore/port/plc_port.h��ѡ������

����Ŀ��弰ʵ������������ã�
����Iӳ������С
����Qӳ������С
����Mӳ������С
...

### �޸�plccore/port/plc_port.c�ļ�

- ��Stm32CubeMX���ɹ��̵�srcĿ¼�µ�main.c�ļ��и���Ƭ���������ú�����plc_port.c
���磺
```
static void MX_TIM2_Init(void)
{
    ...
}

static void MX_USART1_UART_Init(void)
{
    ...
}

static void MX_GPIO_Init(void)
{
    ...
}

```
- �����жϴ�����غ�����plc_port.c
���磺
```

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart1)
    {
        dataReceive((unsigned char)CommRxData);
    }
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    plcTimerHook();
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}
```
- �޸�DevInput[]��DevOutput[]���������
- ����ʵ������ʹ������޸�����������ʵ��

### ��examples/Ŀ¼�¼���plcapp�ļ��У�ճ����RTT Studio����Ŀ¼��

plcapp�����PLC���ɿ���������������û��߼���������ͼ��ָ������ɵ�C���Դ����ļ������临����RT-Thread Studio����Ŀ¼Ȼ����RT-Thread Studio����ˢ����Ŀ����

## ���ð���·��

��RT-Thread Studio���˵�����Ŀ��->�����ԡ�->��C/C++���桱->��·���ͷ��š������hyacore��plctoc_output�İ���·����

## �޸�Ŀ���ļ���

��RT-Thread Studio���˵�����Ŀ��->�����ԡ�->��C/C++������->�����á�->������������->��Artifact name:����д���µ����֡�

## ����RT-Thread Studio����

����ɹ�����ֲ��ɡ�

## ��PLC IDE���ʹ��

��ֲ��ɺ󣬽�RT-Thread Studio�����ļ������帴����PLC IDEִ��Ŀ¼�µ�processorsĿ¼�У������Ϳ�����Ϊһ���µ�PLC������ʹ�á�
��PLC IDE�У��û����Խ���PLCӦ�ù��̣�ʹ��PLC������ԣ�����ָ�������ͼ��ʵ���û��߼���Ȼ��ֱ�ӱ��롢���ӣ���������Ŀ��壬����ʵ���߼����ơ�

## ע������

Ŀǰ��֧��STM32ϵ��MCU��
Ŀǰ��֧��RT-Thread nano�汾��

## ����PLC IDE

PLC IDE��һ��ɱ�̿������ļ��ɿ���������ʵ���˷���IEC61131-3��׼�����ģ�ͺͱ��ģ�ͣ�������ϵͳ��չ�Ժá�֧�ֱ�׼��ָ���IL�����ṹ���ı���ST��������ͼ��LD����PLC������ԣ��ܹ���PLC�û��߼��������ΪĿ��Ӳ���Ķ�����Ŀ����룬ʵ�ֱ�����PLC������ڴ�ͳ�Ľ�����PLC��ָ��ִ��Ч�ʸ��ߣ��洢�ܶȸ��ߣ��Ҿ��и��ߵĿɿ��ԡ�