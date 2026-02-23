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
#include "i2c.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdint.h> 
#include "task.h"
#include "bsp_can.h"
#include "bsp_usart.h"
#include "ist8310driver.h"
#include "BMI088.h"
#include "BMI088driver.h"
#include "Motor.h"
#include "Remote.h"
#include "PID.h"
#include "LED.h"
#include "Buzzer.h"
#include "Gimbal_Yaw_Small.h"
#include "Gimbal_Trigger.h"
#include "Gimbal_Shoot.h"
#include "Gimbal_Pitch.h"
#include "Gimbal_PoseCalc.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//===============文件内部类型和宏

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//===============文件内部的宏定义

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//===============文件内部宏定义的标记

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//===============私有变量

extern uint8_t rx_data[8];//can_receive.c
extern PID_PositionInitTypedef Trigger_SpeedPID;
extern PID_PositionInitTypedef BigYaw_SpeedPID;
extern PID_PositionInitTypedef BigYaw_PositionPID;
extern PID_PositionInitTypedef SmallYaw_SpeedPID;
extern PID_PositionInitTypedef SmallYaw_PositionPID;
extern PID_PositionInitTypedef SmallYaw_GyroscopePID;
extern PID_PositionInitTypedef ShootLeft_SpeedPID;
extern PID_PositionInitTypedef ShootRight_SpeedPID;
extern M3508_Motor Can1_M3508_MotorStatus[8];//M3508电机状态数组
extern M3508_Motor Can2_M3508_MotorStatus[8];//M3508电机状态数组
extern M6020_Motor Can1_M6020_MotorStatus[7];//GM6020电机状态数组
extern M6020_Motor Can2_M6020_MotorStatus[7];//GM6020电机状态数组
extern M2006_Motor Can1_M2006_MotorStatus[8];//M2006电机状态数组
extern M2006_Motor Can2_M2006_MotorStatus[8];//M2006电机状态数组
extern RC_ctrl_t *local_rc_ctrl;		
extern BMI088_Init_typedef BMI088_Data;
extern BMI088_Init_typedef Can_BMI088_Data;
extern BMI088_Init_typedef BigYaw_BMI088_Data;		//大yaw轴解算的陀螺仪数据
extern BMI088_Init_typedef SmallYaw_BMI088_Data;	//小yaw轴解算的陀螺仪数据


extern float gyro_needvalue ;//用户目标角度

uint8_t rx_byte;//串口中断回调函数缓冲区
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//===============私有函数原型
void RM_debug(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

	void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
	{
			if(GPIO_Pin == IST8310_DRDY_Pin)
			{
//					ist8310_read_mag(mag);
			}
	
	}
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
  MX_I2C3_Init();
  MX_SPI1_Init();
  MX_TIM10_Init();
  MX_TIM5_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim6);
  Can_Filter_Init();
	
  Remote_Init();
	LED_Init();
	Buzzer_Init();
  Gimbal_YawSmall_Init();
  Gimbal_YawBig_Init();
	Gimbal_Trigger_Init();
	Gimbal_Shoot_Init();
	Gimbal_Pitch_Init();
	
	//=================
  ist8310_init();
	
	UART2_SendString("RM GOOOO!");
//	Buzzer_On();
	
	//=================
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		RM_debug();
		Gimbal_Warning_Remote();
//    Gimbal_Warning_Music();

		Gimbal_PoseCalc();
		
		
		// ============磁力计和陀螺仪数据============
//		UART2_SendByte(',');
		UART2_SendFloat_Sign(Can_BMI088_Data.Yaw,4);
		UART2_SendByte(',');
		UART2_SendFloat_Sign(BigYaw_BMI088_Data.Yaw,4);
		UART2_SendByte(',');
		UART2_SendFloat_Sign(SmallYaw_BMI088_Data.Yaw,4);
		UART2_SendByte(',');
		
		
		UART2_SendFloat_Sign(gyro_needvalue,4);//用户目标角度
		UART2_SendByte(',');
		UART2_SendFloat_Sign(SmallYaw_BMI088_Data.Yaw,4);//实际角度
		UART2_SendByte(',');
		UART2_SendFloat_Sign(SmallYaw_GyroscopePID.Need_Value,4);//电机目标角度
//		UART2_SendByte(',');
		// ============遥控器数据============
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
		}

		// ============电机PID数据============
		{
//		UART2_SendNumber(BigYaw_SpeedPID.Need_Value,4);//大yaw
//		UART2_SendByte(',');
//		UART2_SendNumber(BigYaw_SpeedPID.Now_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(BigYaw_PositionPID.Need_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(BigYaw_PositionPID.Now_Value,4);
//		UART2_SendByte(',');
//		UART2_SendFloat_Sign(SmallYaw_SpeedPID.Need_Value,4);//小waw
//		UART2_SendByte(',');
//		UART2_SendFloat_Sign(SmallYaw_SpeedPID.Now_Value,4);
//		UART2_SendByte(',');
//		UART2_SendFloat_Sign(SmallYaw_GyroscopePID.Need_Value,4);
//		UART2_SendByte(',');
//		UART2_SendFloat_Sign(SmallYaw_GyroscopePID.Now_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(ShootRight_SpeedPID.Need_Value,4);//发射机构
//		UART2_SendByte(',');
//		UART2_SendNumber(ShootRight_SpeedPID.Now_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(ShootLeft_SpeedPID.Need_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(ShootRight_SpeedPID.Now_Value,4);
//		UART2_SendByte(',');
//		UART2_SendNumber(Trigger_SpeedPID.Need_Value,4);//拨弹盘
//		UART2_SendByte(',');
		}



		UART2_SendByte('\n');
//		HAL_Delay(10);
		

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
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    // 读取接收到的消息
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
        return; // 安全检查

    // 只处理标准帧
    if (rx_header.IDE != CAN_ID_STD)
        return;

    // 根据 CAN 外设实例区分总线
    if (hcan == &hcan1)
    {
        switch (rx_header.StdId)
			{
					case 0x201:
							CAN1_M3508_DataProcess(0x201,rx_data);break;
					case 0x202:
							CAN1_M3508_DataProcess(0x202,rx_data);break;
					case 0x203:
							break;
					case 0x204:
							break;
					case 0x205:
							break;
					case 0x206:
							CAN1_M6020_DataProcess(0x206,rx_data);break;
					case 0x207:
							CAN1_M2006_DataProcess(0x207,rx_data);break;
					case 0x208:
							break;
					default:
					{
							break;
					}
			}
    }
    else if (hcan == &hcan2)
    {
				switch (rx_header.StdId)
			{
					case 0x201:
							break;
					case 0x202:
							break;
					case 0x203:
							break;
					case 0x204:
							break;
					case 0x205:
							CAN2_M6020_DataProcess(0x205,rx_data);break;
					case 0x206:
							CAN2_M6020_DataProcess(0x206,rx_data);break;
					case 0x207:
							break;
					case 0x208:
							break;
					case 0x146:
							CToC_AngleProcess(0x146,rx_data,&Can_BMI088_Data);break;
					default:
					{
							break;
					}
			}
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
		
    HAL_UART_Transmit(&huart1, &rx_byte, 1, HAL_MAX_DELAY);
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    HAL_UART_Receive_DMA(&huart1, &rx_byte, 1);
  }
}

void RM_debug(void)
{
  PROCESSOR(1000);
  LED_R_Toggle();
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
