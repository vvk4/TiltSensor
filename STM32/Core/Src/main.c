/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.priority = (osPriority_t) osPriorityNormal, .stack_size = 128 * 4 };
/* Definitions for MPU6500Task */
osThreadId_t MPU6500TaskHandle;
const osThreadAttr_t MPU6500Task_attributes = { .name = "MPU6500Task",
		.priority = (osPriority_t) osPriorityLow, .stack_size = 128 * 4 };
/* Definitions for UART2ReceivedCm */
osThreadId_t UART2ReceivedCmHandle;
const osThreadAttr_t UART2ReceivedCm_attributes = { .name = "UART2ReceivedCm",
		.priority = (osPriority_t) osPriorityLow, .stack_size = 128 * 4 };
/* Definitions for QueueSPI1 */
osMessageQueueId_t QueueSPI1Handle;
const osMessageQueueAttr_t QueueSPI1_attributes = { .name = "QueueSPI1" };
/* Definitions for QueueUART2 */
osMessageQueueId_t QueueUART2Handle;
const osMessageQueueAttr_t QueueUART2_attributes = { .name = "QueueUART2" };
/* USER CODE BEGIN PV */
uint8_t SPI1ReceiveArray[100], MPU6500Initialized;
uint8_t UART2TransmittArray[200];
uint8_t CalibratingStateMachine, Calibrate;
int16_t AccX, AccY, AccZ;
int16_t GyroX, GyroY, GyroZ;
int16_t AccXZero, AccYZero, AccZZero;
int16_t GyroXZero, GyroYZero, GyroZZero;
int32_t SummAccX, SummAccY, SummAccZ, SummGyroX, SummGyroY, SummGyroZ;
int16_t CalibrateCnt;
float AccXfl, AccYfl, AccZfl;
float GyroXfl, GyroYfl, GyroZfl;
int16_t Temperature, CntSPI;
volatile uint32_t TimestampCounter, Timestamp;
uint8_t PacketRec[SIZE_OF_REC_ARRAY];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);
void MPU6500TaskEntry(void *argument);
void UART2ReceivedCmdEntry(void *argument);

/* USER CODE BEGIN PFP */
void InitMPU6500(void);
void WriteRegisterMPU6500(uint8_t Addr, uint8_t Data);
uint8_t ReadRegisterMPU6500(uint8_t Addr);
unsigned char CalcCheckSumm(unsigned int N, unsigned char *Array);
void Calibrating(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	MX_SPI1_Init();
	MX_USART2_UART_Init();
	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(LIS3DSH_CS, GPIO_PIN_SET); //CS=1 for onoard accelerometer
	HAL_GPIO_WritePin(MPU6500_CS, GPIO_PIN_SET); //CS=1 for MPU6500

	InitMPU6500();
	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* creation of QueueSPI1 */
	QueueSPI1Handle = osMessageQueueNew(16, sizeof(uint16_t),
			&QueueSPI1_attributes);

	/* creation of QueueUART2 */
	QueueUART2Handle = osMessageQueueNew(16, sizeof(uint16_t),
			&QueueUART2_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	/* creation of MPU6500Task */
	MPU6500TaskHandle = osThreadNew(MPU6500TaskEntry, NULL,
			&MPU6500Task_attributes);

	/* creation of UART2ReceivedCm */
	UART2ReceivedCmHandle = osThreadNew(UART2ReceivedCmdEntry, NULL,
			&UART2ReceivedCm_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	CalibratingStateMachine = 0; //Старт калибровки
	//Calibrate = 1;//Калибровка по включению
	/* USER CODE END RTOS_EVENTS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

	/* USER CODE BEGIN SPI1_Init 0 */

	/* USER CODE END SPI1_Init 0 */

	/* USER CODE BEGIN SPI1_Init 1 */

	/* USER CODE END SPI1_Init 1 */
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 460800;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	huart2.Instance->CR1 |= USART_CR1_RXNEIE;

	/* USER CODE END USART2_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream6_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
	/* DMA2_Stream0_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
	/* DMA2_Stream3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3 | GPIO_PIN_6, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);

	/*Configure GPIO pins : PE3 PE6 */
	GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : PE4 PE0 */
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : PD15 */
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void InitMPU6500(void) {
	uint8_t Ch;

	//WriteRegisterMPU6500(CTRL_REG3,0x1);//soft reset

	Ch = ReadRegisterMPU6500(117);
	asm("Nop");

	WriteRegisterMPU6500(PWR_MGMT_1, 0x80);
	HAL_Delay(110);
	WriteRegisterMPU6500(SIGNAL_PATH_RESET, 0x07);
	HAL_Delay(110);
	WriteRegisterMPU6500(USER_CTRL, 0x10);
	HAL_Delay(110);

	WriteRegisterMPU6500(ACCEL_CONFIG, 0x0);
	WriteRegisterMPU6500(ACCEL_CONFIG_2, 0x0);
	WriteRegisterMPU6500(GYRO_CONFIG, 0x0);

	WriteRegisterMPU6500(PWR_MGMT_1, 0x0);
	WriteRegisterMPU6500(PWR_MGMT_2, 0x0);
	//WriteRegisterMPU6500(LP_ACCEL_ODR, 10);

	WriteRegisterMPU6500(FIFO_EN, 0x0);
	WriteRegisterMPU6500(CONFIG, 0x0);

	WriteRegisterMPU6500(INT_PIN_CFG, 0x30);
	WriteRegisterMPU6500(INT_ENABLE, 0x1);

	WriteRegisterMPU6500(SMPLRT_DIV, 0x0);

	MPU6500Initialized = 1;
}

void WriteRegisterMPU6500(uint8_t Addr, uint8_t Data) {
	uint8_t TrmBytes[2];

	HAL_GPIO_WritePin(MPU6500_CS, GPIO_PIN_RESET);
	TrmBytes[0] = Addr;
	TrmBytes[1] = Data;
	HAL_SPI_Transmit(&hspi1, TrmBytes, 2, 1000);
	HAL_GPIO_WritePin(MPU6500_CS, GPIO_PIN_SET);
}

uint8_t ReadRegisterMPU6500(uint8_t Addr) {
	uint8_t TrmByte, RecByte;

	HAL_GPIO_WritePin(MPU6500_CS, GPIO_PIN_RESET);
	TrmByte = Addr;
	TrmByte |= 0x80;
	HAL_SPI_Transmit(&hspi1, &TrmByte, 1, 1000);
	HAL_SPI_Receive(&hspi1, &RecByte, 1, 1000);

	HAL_GPIO_WritePin(MPU6500_CS, GPIO_PIN_SET);

	return RecByte;
}

void ProcessDataMPU6500(void) {

	AccX = (((int16_t) SPI1ReceiveArray[1]) << 8)
			+ (int16_t) SPI1ReceiveArray[2];
	AccY = (((int16_t) SPI1ReceiveArray[3]) << 8)
			+ (int16_t) SPI1ReceiveArray[4];
	AccZ = (((int16_t) SPI1ReceiveArray[5]) << 8)
			+ (int16_t) SPI1ReceiveArray[6];

	Temperature = (((int16_t) SPI1ReceiveArray[7]) << 8)
			+ (int16_t) SPI1ReceiveArray[8];

	GyroX = (((int16_t) SPI1ReceiveArray[9]) << 8)
			+ (int16_t) SPI1ReceiveArray[10];
	GyroY = (((int16_t) SPI1ReceiveArray[11]) << 8)
			+ (int16_t) SPI1ReceiveArray[12];
	GyroZ = (((int16_t) SPI1ReceiveArray[13]) << 8)
			+ (int16_t) SPI1ReceiveArray[14];

	Calibrating();

	AccXfl = AccXZero - AccX;//меняем знак оси, чтобы он соответствовал матрице поворота
	AccYfl = AccYZero - AccY;//меняем знак оси, чтобы он соответствовал матрице поворота
	AccZfl = AccZ - AccZZero;
	GyroXfl = GyroX - GyroXZero;
	GyroYfl = GyroY - GyroYZero;
	GyroZfl = GyroZ - GyroZZero;

}

void Calibrating(void) {
	if (!Calibrate)
		return;

	switch (CalibratingStateMachine) {
	case 0:
		CalibrateCnt = 250;
		SummAccX = 0;
		SummAccY = 0;
		SummAccZ = 0;
		SummGyroX = 0;
		SummGyroY = 0;
		SummGyroZ = 0;
		CalibratingStateMachine = 1;
		break;
	case 1:
		CalibrateCnt--;
		if (!CalibrateCnt)
			CalibratingStateMachine = 2;
		break;
	case 2:
		SummAccX = SummAccX + AccX;
		SummAccY = SummAccY + AccY;
		SummAccZ = SummAccZ + AccZ;
		SummGyroX = SummGyroX + GyroX;
		SummGyroY = SummGyroY + GyroY;
		SummGyroZ = SummGyroZ + GyroZ;

		CalibrateCnt++;
		if (CalibrateCnt >= 64) {
			AccXZero = (int16_t) (SummAccX >> 6);
			AccYZero = (int16_t) (SummAccY >> 6);
			AccZZero = 16384 + (int16_t) (SummAccZ >> 6);
			GyroXZero = (int16_t) (SummGyroX >> 6);
			GyroYZero = (int16_t) (SummGyroY >> 6);
			GyroZZero = (int16_t) (SummGyroZ >> 6);
			CalibratingStateMachine = 0;
			Calibrate = 0;
		}
		break;

	default:
		CalibratingStateMachine = 0;
		break;
	}
}

void TrmDataToUART(void) {
	int16_t BytesCounter = 4;
	int16_t *Ptr16;
	uint32_t *Ptr32;
	float *PtrFl;

	UART2TransmittArray[0] = HEADER1; //Заголовок пакета из 2-х байт
	UART2TransmittArray[1] = HEADER2;

	UART2TransmittArray[3] = TRM_DATA_PACKET; //Команда

	Ptr32 = (uint32_t*) &UART2TransmittArray[BytesCounter];
	*Ptr32 = Timestamp;
	BytesCounter = BytesCounter + sizeof(uint32_t);

	Ptr16 = (int16_t*) &UART2TransmittArray[BytesCounter];
	*Ptr16 = (int16_t) AccXfl;
	BytesCounter = BytesCounter + sizeof(int16_t);

	Ptr16 = (int16_t*) &UART2TransmittArray[BytesCounter];
	*Ptr16 = (int16_t) AccYfl;
	BytesCounter = BytesCounter + sizeof(int16_t);

	Ptr16 = (int16_t*) &UART2TransmittArray[BytesCounter];
	*Ptr16 = (int16_t) AccZfl;
	BytesCounter = BytesCounter + sizeof(int16_t);

	Ptr16 = (int16_t*) &UART2TransmittArray[BytesCounter];
	*Ptr16 = (int16_t) GyroXfl;
	BytesCounter = BytesCounter + sizeof(int16_t);

	Ptr16 = (int16_t*) &UART2TransmittArray[BytesCounter];
	*Ptr16 = (int16_t) GyroYfl;
	BytesCounter = BytesCounter + sizeof(int16_t);

	Ptr16 = (int16_t*) &UART2TransmittArray[BytesCounter];
	*Ptr16 = (int16_t) GyroZfl;
	BytesCounter = BytesCounter + sizeof(int16_t);

	PtrFl = (float*) &UART2TransmittArray[BytesCounter];
	*PtrFl = (float) TiltX;
	BytesCounter = BytesCounter + sizeof(float);

	PtrFl = (float*) &UART2TransmittArray[BytesCounter];
	*PtrFl = (float) TiltY;
	BytesCounter = BytesCounter + sizeof(float);

	UART2TransmittArray[2] = BytesCounter + 1; //Число передаваемых байт

	UART2TransmittArray[BytesCounter++] = CalcCheckSumm(
			UART2TransmittArray[2] - 3, &UART2TransmittArray[2]);

	HAL_UART_Transmit_DMA(&huart2, (uint8_t*) UART2TransmittArray,
			UART2TransmittArray[2]);
}

unsigned char CalcCheckSumm(unsigned int N, unsigned char *Array) {
	unsigned int Summ = 0, j;

	for (j = 0; j < N; j++)
		Summ = Summ + Array[j];

	Summ = ~Summ;

	return (unsigned char) Summ;

}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
	/* USER CODE BEGIN 5 */

	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_MPU6500TaskEntry */
/**
 * @brief Function implementing the MPU6500Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_MPU6500TaskEntry */
void MPU6500TaskEntry(void *argument) {
	/* USER CODE BEGIN MPU6500TaskEntry */
	short msg;

	/* Infinite loop */
	for (;;) {
		osStatus_t msg_Result = osMessageQueueGet(QueueSPI1Handle, &msg, 0, 0);
		if (msg_Result == osOK) {
			switch (msg) {
			case 1234:
				HAL_GPIO_WritePin (GPIOD, GPIO_PIN_15,GPIO_PIN_SET ); //светодиод
				ProcessDataMPU6500();
				CalcullateTilts();
				TrmDataToUART();
				HAL_GPIO_WritePin (GPIOD, GPIO_PIN_15,GPIO_PIN_RESET ); //светодиод
				break;
			default:
				break;
			}
		}

		osDelay(1);
	}
	/* USER CODE END MPU6500TaskEntry */
}

/* USER CODE BEGIN Header_UART2ReceivedCmdEntry */
/**
 * @brief Function implementing the UART2ReceivedCm thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_UART2ReceivedCmdEntry */
void UART2ReceivedCmdEntry(void *argument) {
	/* USER CODE BEGIN UART2ReceivedCmdEntry */
	int16_t Tmp;
	/* Infinite loop */
	for (;;) {
		short msg;

		osStatus_t msg_Result = osMessageQueueGet(QueueUART2Handle, &msg, 0, 0);
		if (msg_Result == osOK) {
			switch (msg) {
			case 1235:
				switch (PacketRec[1]) {
				case K_ACCEL_GYRO:
					Tmp = (int16_t) PacketRec[2]
							+ (((int16_t) PacketRec[3]) << 8);
					KGyro = (float) Tmp / 1000;
					KAcc = 1 - KGyro;
					break;
				case CALIBRATE_ACCEL_GYRO:
					Calibrate = 1;//Калибровка
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		} else
			asm("Nop");
		osDelay(1);
	}
	/* USER CODE END UART2ReceivedCmdEntry */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM14 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */
	uint8_t TrmByte;
	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM14) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */
	TimestampCounter++;

	if (CntSPI > 3) {
		CntSPI = 0;
		if (MPU6500Initialized) {
			TrmByte = ACCEL_XOUT_H;		//Start MPU6500 data transfer   250Hz
			TrmByte |= 0x80;
			HAL_GPIO_WritePin(MPU6500_CS, GPIO_PIN_RESET);
			HAL_SPI_TransmitReceive_DMA(&hspi1, &TrmByte, SPI1ReceiveArray, 16);
//			HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15); //светодиод
		}
	}

	CntSPI++;

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
