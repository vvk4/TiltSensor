/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */


/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

#define SENSITIVITY		0.06
#define TICK_PERIOD 	0.004


#define HEADER1			0x39
#define HEADER2			0xC3


enum CommandsToPC {TRM_DATA_PACKET=1};
enum CommandsFromPC {K_ACCEL_GYRO=2,CALIBRATE_ACCEL_GYRO};


//Pins:
#define LIS3DSH_CS GPIOE, GPIO_PIN_3
#define MPU6500_CS GPIOE, GPIO_PIN_6

//LIS3DSH registers:
#define OUT_T				0xc
#define INFO1				0xd
#define INFO2				0xe
#define WHO_AM_I 			0xf
#define CTRL_REG4			0x20
#define CTRL_REG3			0x23
#define CTRL_REG5			0x24
#define OUT_X_L				0x28


#define SMPLRT_DIV		25
#define CONFIG			26
#define GYRO_CONFIG		27
#define ACCEL_CONFIG	28
#define ACCEL_CONFIG_2	29
#define LP_ACCEL_ODR	30
#define FIFO_EN			35

#define INT_PIN_CFG		55
#define INT_ENABLE		56
#define INT_STATUS		58

#define INT_STATUS		58
#define ACCEL_XOUT_H	59
#define ACCEL_XOUT_L	60
#define ACCEL_YOUT_H	61
#define ACCEL_YOUT_L	62
#define ACCEL_ZOUT_H	63
#define ACCEL_ZOUT_L	64
#define TEMP_OUT_H		65
#define TEMP_OUT_L		66
#define GYRO_XOUT_H		67
#define GYRO_XOUT_L		68
#define GYRO_YOUT_H		69
#define GYRO_YOUT_L		70
#define GYRO_ZOUT_H		71
#define GYRO_ZOUT_L		72

#define SIGNAL_PATH_RESET 104
#define USER_CTRL		106
#define PWR_MGMT_1		107
#define PWR_MGMT_2		108

#define SIZE_OF_REC_ARRAY	100


extern uint8_t SPI1ReceiveArray[100],MPU6500Initialized;
extern osMessageQueueId_t QueueSPI1Handle;
extern volatile uint32_t TimestampCounter,Timestamp;
extern uint8_t PacketRec[SIZE_OF_REC_ARRAY];
extern osMessageQueueId_t QueueUART2Handle;
extern float AccXfl, AccYfl, AccZfl;
extern float GyroXfl, GyroYfl, GyroZfl;
extern float TiltX, TiltY;
extern float KGyro, KAcc;

extern unsigned char CalcCheckSumm(unsigned int N, unsigned char *Array);
extern void CalcullateTilts(void);


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
