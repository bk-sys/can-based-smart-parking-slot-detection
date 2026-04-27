/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "tim.h"
#include "gpio.h"
#include <string.h>

/* USER CODE BEGIN PV */
CAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];
uint32_t TxMailbox;

int totalSlots = 2;
int availableSlots = 2;
int slot1Status = 0;  // 0=free, 1=occupied
int slot2Status = 0;  // 0=free, 1=occupied

int ir1LastState = 1;
int ir2LastState = 1;
/* USER CODE END PV */

void SystemClock_Config(void);
void Error_Handler(void);

/* USER CODE BEGIN 0 */
void delay_us(uint16_t us)
{
  __HAL_TIM_SET_COUNTER(&htim1, 0);
  while(__HAL_TIM_GET_COUNTER(&htim1) < us);
}

uint32_t getDistance(GPIO_TypeDef* trigPort, uint16_t trigPin,
                     GPIO_TypeDef* echoPort, uint16_t echoPin)
{
  uint32_t distance = 0;

  // Trigger
  HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);
  delay_us(2);
  HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_SET);
  delay_us(10);
  HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);

  // Wait for echo HIGH
  uint32_t timeout = 0;
  while(HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_RESET)
  {
    timeout++;
    if(timeout > 30000) return 999;
  }

  // Measure echo HIGH time
  __HAL_TIM_SET_COUNTER(&htim1, 0);
  while(HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_SET)
  {
    if(__HAL_TIM_GET_COUNTER(&htim1) > 30000) return 999;
  }

  distance = __HAL_TIM_GET_COUNTER(&htim1) / 58;
  return distance;
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_TIM1_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim1);
  HAL_CAN_Start(&hcan1);

  TxHeader.StdId = 0x123;
  TxHeader.ExtId = 0x00;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = 3;
  TxHeader.TransmitGlobalTime = DISABLE;
  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE BEGIN 3 */

    // ✅ Read IR1 (Entry)
    int ir1Current = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);

    if (ir1Current == GPIO_PIN_RESET && ir1LastState == GPIO_PIN_SET)
    {
      // Car entered
      if (availableSlots > 0)
      {
        availableSlots--;
        if (slot1Status == 0) slot1Status = 1;
        else if (slot2Status == 0) slot2Status = 1;
      }
    }
    ir1LastState = ir1Current;

    // ✅ Read IR2 (Exit)
    int ir2Current = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

    if (ir2Current == GPIO_PIN_RESET && ir2LastState == GPIO_PIN_SET)
    {
      // Car exited
      if (availableSlots < totalSlots)
      {
        availableSlots++;
        if (slot2Status == 1) slot2Status = 0;
        else if (slot1Status == 1) slot1Status = 0;
      }
    }
    ir2LastState = ir2Current;

    // ✅ Read Ultrasonic 1 (Slot 1 confirmation)
    uint32_t dist1 = getDistance(GPIOB, GPIO_PIN_0, GPIOB, GPIO_PIN_1);
    if (dist1 < 20) slot1Status = 1;
    else slot1Status = 0;

    // ✅ Read Ultrasonic 2 (Slot 2 confirmation)
    uint32_t dist2 = getDistance(GPIOB, GPIO_PIN_4, GPIOB, GPIO_PIN_5);
    if (dist2 < 20) slot2Status = 1;
    else slot2Status = 0;

    // ✅ Update available slots based on ultrasonic
    availableSlots = totalSlots - (slot1Status + slot2Status);

    // ✅ Send via CAN
    TxData[0] = availableSlots;
    TxData[1] = slot1Status;
    TxData[2] = slot2Status;

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
    {
      HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
    }

    HAL_Delay(500);

    /* USER CODE END 3 */
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif