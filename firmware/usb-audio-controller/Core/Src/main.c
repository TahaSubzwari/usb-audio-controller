/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : USB Audio Controller — main application entry point.
  *                   Handles rotary encoder polling, USB HID volume reporting,
  *                   and LCD display updates.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "GUI_Paint.h"
#include "fonts.h"
#include "image.h"
#include "LCD_2inch.h"
#include "volume_display.h"
#include "usb_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern USBD_HandleTypeDef hUsbDeviceFS;
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
int last_drawn = -1;
uint8_t last_clk = 1;
uint8_t last_dt = 1;
int8_t pending_delta = 0;
uint32_t last_detent_tick = 0;
uint32_t last_encoder_tick = 0;
uint8_t lcd_update_pending = 0;
uint32_t last_sw_tick = 0;

char uart_buf[8];
uint8_t uart_idx = 0;
volatile int pc_volume = -1;
int muted = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  printf("Starting...\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      int clk = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15);
      int dt  = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14);
      int sw = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);

      //Mute when pressing rotary encoder
      if (sw == 0)  // pressed (pull-up, active low)
      {
          uint32_t now = HAL_GetTick();
          if (now - last_sw_tick >= 300)  // debounce
          {
              uint8_t mute[2] = {0x01, 0x04};
              USBD_HID_SendReport(&hUsbDeviceFS, mute, 2);
              HAL_Delay(15);
              uint8_t release[2] = {0x01, 0x00};
              USBD_HID_SendReport(&hUsbDeviceFS, release, 2);
              muted = !muted;  // toggle flag
              last_sw_tick = now;

              // Force immediate LCD redraw to show mute state
			  DrawVolumeDial(pc_volume, muted);
			  last_drawn = pc_volume;
			  lcd_update_pending = 0;
          }
      }

      //Encoder Polling
      if (clk != last_clk || dt != last_dt)	//Change in encoder
      {
    	  //Allow the encoder time to settle and reread it (filters noise spikes)
          HAL_Delay(1);
          clk = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15);
          dt  = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14);

          //Detent (click) complete - Both signals are back to high (reset) and we have a pending rotation to send
          if (clk == 1 && dt == 1 && pending_delta != 0)
          {
        	  //Ignore debounce by checking if it has been <10ms since last registered detent
              uint32_t now = HAL_GetTick();
              if (now - last_detent_tick >= 10)
              {
                  // Send Press HID report
                  USBD_HID_SendVolumeReport(&hUsbDeviceFS, pending_delta);

                  // Send HID release report immediately after
                  HAL_Delay(15);
                  uint8_t release[2] = {0x01, 0x00};
                  USBD_HID_SendReport(&hUsbDeviceFS, release, 2);

                  // Flag LCD for update, but don't redraw yet
                  last_encoder_tick = HAL_GetTick();
                  lcd_update_pending = 1;
                  last_detent_tick = now;
              }
              pending_delta = 0;
          }
          // Falling edge on clock: record direction for detent to be sent in next iteration
          else if (clk == 0 && last_clk == 1)
          {
        	  pending_delta = (dt == 1) ? 1 : -1;
          }

          last_clk = clk;
          last_dt = dt;
      }

      // --- READ PC VOLUME FROM SERIAL ---
      uint8_t byte;
      if (HAL_UART_Receive(&huart2, &byte, 1, 0) == HAL_OK)
      {
          if (byte == 'V')  // start of new message — reset buffer (ensures that the STM32 knows that we are receiving a new volume level)
          {
              uart_idx = 0;
              memset(uart_buf, 0, sizeof(uart_buf));
          }
          else if (byte == '\n')	//End of message, convert the array into an integer representing the volume and flag LCD to redraw
          {
              uart_buf[uart_idx] = '\0';
              if (uart_idx > 0)
              {
                  int received = atoi(uart_buf);
                  if (received >= 0 && received <= 100)
                  {
                      if (received != pc_volume)	//If received volume is different from pc_volume, flag to redraw
                      {
                          pc_volume = received;
                          if (HAL_GetTick() - last_encoder_tick >= 500)
                          {
                              lcd_update_pending = 1;
                          }
                      }
                      else
                      {
                          pc_volume = received;
                      }
                  }
              }
              uart_idx = 0;
              memset(uart_buf, 0, sizeof(uart_buf));
          }
          else if (byte >= '0' && byte <= '9')	//Digit received, store into array
          {
              if (uart_idx < 3)	//uart_buf array should be no bigger than 3 elements (stores digits 0-100)
              {
                  uart_buf[uart_idx++] = byte;
              }
          }
      }

      // LCD draws when flagged and encoder is idle
      if (lcd_update_pending && HAL_GetTick() - last_encoder_tick >= 500)
      {
          if (pc_volume != last_drawn)
          {
              DrawVolumeDial(pc_volume, muted);
              last_drawn = pc_volume;
          }
          lcd_update_pending = 0;
      }
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
