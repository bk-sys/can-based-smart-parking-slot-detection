/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c (RX BOARD)
  * @brief          : Receives CAN data, prints on UART, controls LEDs
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>

/* USER CODE BEGIN PV */
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

int availableSlots = 2;
int slot1Status = 0;
int slot2Status = 0;
/* USER CODE END PV */

void SystemClock_Config(void);
void Error_Handler(void);

/* USER CODE BEGIN 0 */
void printUART(char *msg)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  HAL_CAN_Start(&hcan1);

  // Configure CAN Filter
  CAN_FilterTypeDef canFilter;
  canFilter.FilterActivation     = CAN_FILTER_ENABLE;
  canFilter.FilterBank           = 0;
  canFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
  canFilter.FilterIdHigh         = 0x0000;
  canFilter.FilterIdLow          = 0x0000;
  canFilter.FilterMaskIdHigh     = 0x0000;
  canFilter.FilterMaskIdLow      = 0x0000;
  canFilter.FilterMode           = CAN_FILTERMODE_IDMASK;
  canFilter.FilterScale          = CAN_FILTERSCALE_32BIT;
  HAL_CAN_ConfigFilter(&hcan1, &canFilter);

  // Startup message
  printUART("=== Smart Parking System ===\r\n");
  printUART("Waiting for data...\r\n\r\n");

  // Blink PD15 to show board is alive
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
  HAL_Delay(300);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE BEGIN 3 */

    // ✅ Check if CAN message received
    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0)
    {
      HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData);

      // Read data from TX board
      availableSlots = RxData[0];
      slot1Status    = RxData[1];
      slot2Status    = RxData[2];

      // ✅ Build and send UART message
      char msg[100];

      printUART("----------------------------\r\n");

      sprintf(msg, "Available Slots: %d\r\n", availableSlots);
      printUART(msg);

      if (slot1Status == 1)
        printUART("Slot 1: Occupied\r\n");
      else
        printUART("Slot 1: Available\r\n");

      if (slot2Status == 1)
        printUART("Slot 2: Occupied\r\n");
      else
        printUART("Slot 2: Available\r\n");

      if (availableSlots == 0)
        printUART("*** PARKING FULL! ***\r\n");
      else if (availableSlots == 2)
        printUART("All Slots Free!\r\n");

      printUART("----------------------------\r\n\r\n");

      // ✅ LED Control
      // PD12 → Slot 1 (ON = Occupied)
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12,
        slot1Status ? GPIO_PIN_SET : GPIO_PIN_RESET);

      // PD13 → Slot 2 (ON = Occupied)
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13,
        slot2Status ? GPIO_PIN_SET : GPIO_PIN_RESET);

      // PD14 → Parking FULL (ON = Full)
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14,
        (availableSlots == 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);

      // PD15 → Blink = data received
      HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
    }

    HAL_Delay(100);

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
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM      = 8;
  RCC_OscInitStruct.PLL.PLLN      = 336;
  RCC_OscInitStruct.PLL.PLLP      = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ      = 7;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    Error_Handler();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif