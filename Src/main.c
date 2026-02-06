/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "dma.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdint.h> 
#include "bsp_can.h"
#include "bsp_usart.h"
#include "Motor.h"
#include "Remote.h"
#include "PID.h"
#include "Gimbal_Yaw_Small.h"
#include "Gimbal_to_Chassis.h"
#include "Gimbal_Trigger.h"
#include "Gimbal_Shoot.h"
#include "Gimbal_Pitch.h"



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


extern uint8_t rx_data[8];//can_receive.c
extern motor_measure_t Can1_Rx_Data[8];
extern motor_measure_t Can2_Rx_Data[8];
extern PID_PositionInitTypedef Trigger_SpeedPID;
extern PID_PositionInitTypedef SmallYaw_SpeedPID;
extern PID_PositionInitTypedef SmallYaw_PositionPID;
extern PID_PositionInitTypedef ShootLeft_SpeedPID;
extern PID_PositionInitTypedef ShootRight_SpeedPID;
extern RC_ctrl_t *local_rc_ctrl;		

uint16_t RC_Rx_Len = 0;
uint8_t  RC_Rx_Flag = 0;
uint8_t rx_byte;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM6_Init();
//  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim6);
  Can_Filter_Init();
	
  Remote_Init();
  Gimbal_Yaw_Small_Init();
	Gimbal_Trigger_Init();
	Gimbal_Shoot_Init();
	Gimbal_Pitch_Init();
	
	//=================
	//=================
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		UART2_SendNumber(local_rc_ctrl->rc.ch[0]+1024,4);//右摇杆左右
//		UART2_SendByte(',');
//		UART2_SendNumber(local_rc_ctrl->rc.ch[1]+1024,4);//右摇杆上下
//		UART2_SendByte(',');
//		UART2_SendNumber(local_rc_ctrl->rc.ch[2]+1024,4);//左摇杆左右
//		UART2_SendByte(',');
//		UART2_SendNumber(local_rc_ctrl->rc.ch[3]+1024,4);//左摇杆上下
//		UART2_SendByte(',');
//		UART2_SendNumber(local_rc_ctrl->rc.ch[4]+1024,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(local_rc_ctrl->rc.s[0],4);
//		UART2_SendByte(',');
//		UART2_SendNumber(local_rc_ctrl->rc.s[1],4);
//		UART2_SendByte(',');


//		UART2_SendNumber(SmallYaw_SpeedPID.Need_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(SmallYaw_SpeedPID.Now_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(SmallYaw_PositionPID.Need_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(SmallYaw_PositionPID.Now_Value,4);
//		UART2_SendByte(',');
		UART2_SendNumber(ShootRight_SpeedPID.Need_Value,4);
		UART2_SendByte(',');
		UART2_SendNumber(ShootRight_SpeedPID.Now_Value,4);
		UART2_SendByte(',');
		UART2_SendNumber(ShootLeft_SpeedPID.Need_Value,4);
		UART2_SendByte(',');
		UART2_SendNumber(ShootRight_SpeedPID.Now_Value,4);


extern M3508_Motor Can1_M3508_MotorStatus[8];//M3508电机状态数组
extern M3508_Motor Can2_M3508_MotorStatus[8];//M3508电机状态数组
extern M6020_Motor Can1_M6020_MotorStatus[7];//GM6020电机状态数组
extern M6020_Motor Can2_M6020_MotorStatus[7];//GM6020电机状态数组
extern M2006_Motor Can1_M2006_MotorStatus[8];//M2006电机状态数组
extern M2006_Motor Can2_M2006_MotorStatus[8];//M2006电机状态数组

//		UART2_SendNumber(Trigger_SpeedPID.Need_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(Can1_M2006_MotorStatus[6].RotorSpeed,4);

		UART2_SendByte('\n');
		HAL_Delay(10);
		

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
		
    HAL_UART_Transmit(&huart1, &rx_byte, 1, HAL_MAX_DELAY);
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    HAL_UART_Receive_DMA(&huart1, &rx_byte, 1);
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
