/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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
#include "gpio.h"
#include "i2c.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "vl53lx_api.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void RangingLoop(VL53LX_DEV Dev);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int fputc(int ch, FILE* f)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 0xFFFF);
    return ch;
}

#define panic_and_stop(...) \
    do { \
        printf(__VA_ARGS__); \
        HAL_Delay(1000); \
    } while (1);

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
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    /* USER CODE BEGIN 2 */

    // Init Dev to i2c1 and default i2c address
    static VL53LX_Dev_t dev; // takes 10872B of data, lets put it in static
    VL53LX_DEV pdev = &dev;
    pdev->I2cHandle = &hi2c1;
    pdev->I2cDevAddr = 0x52;

    printf("VL53L1X Example starting!\n\r");

    /*
        I'm not using the fancy "platform" code.
        This includes the commented code below (init reset).
        Also, it uses some kind of i2c expander,
        which I don't have on a nucleo board or
        VL53L4CX arduino shield. It just has integrated
        sensors and two headers for two more. Nothing else.
        The purpose is to toggle the reset pin to get
        the sensor in correct state. Let's just use the GPIO.
    */
    // uint8_t ToFSensor = 1; // Select ToFSensor: 0=Left, 1=Center, 2=Right
    // status = XNUCLEO53L3A2_ResetId(ToFSensor, 0); // Reset ToF sensor
    // HAL_Delay(2);
    // status = XNUCLEO53L3A2_ResetId(ToFSensor, 1); // Reset ToF sensor
    // HAL_Delay(2);

    HAL_GPIO_WritePin(XSHUT_GPIO_Port, XSHUT_Pin, 0);
    HAL_Delay(2);
    HAL_GPIO_WritePin(XSHUT_GPIO_Port, XSHUT_Pin, 1);
    HAL_Delay(2);

    // Init
    uint8_t status = VL53LX_WaitDeviceBooted(pdev);
    if (status || VL53LX_DataInit(pdev))
    {
        panic_and_stop("Init failed!");
    }

    // Read sensor information
    uint8_t byteData = 0;
    uint16_t wordData = 0;
    VL53LX_RdByte(pdev, 0x010F, &byteData);
    printf("VL53LX Model_ID: %02X\n\r", byteData);
    VL53LX_RdByte(pdev, 0x0110, &byteData);
    printf("VL53LX Module_Type: %02X\n\r", byteData);
    VL53LX_RdWord(pdev, 0x010F, &wordData);
    printf("VL53LX: %02X\n\r", wordData);

    // Corrections and calibrations (non interactive)
    if (VL53LX_SmudgeCorrectionEnable(pdev, VL53LX_SMUDGE_CORRECTION_CONTINUOUS))
    {
        panic_and_stop("Smudge correction enable failed!");
    }
    if (VL53LX_SetXTalkCompensationEnable(pdev, 1))
    {
        panic_and_stop("XTalk compensation enable failed!");
    }
    if (VL53LX_PerformXTalkCalibration(pdev))
    {
        panic_and_stop("XTalk calibration failed");
    }
    if (VL53LX_PerformRefSpadManagement(pdev))
    {
        panic_and_stop("RefSpad management failed!");
    }
    if (VL53LX_SetOffsetCorrectionMode(pdev, VL53LX_OFFSETCORRECTIONMODE_STANDARD))
    {
        panic_and_stop("Offset correction mode enable failed!");
    }
    // Interactive calibrations
    // Need a target at an accurate distance
    // if (VL53LX_PerformOffsetSimpleCalibration(pdev, 50))
    // {
    //     panic_and_stop("");
    // }
    // Use cover glass and a solid object touching it
    // if (VL53LX_PerformOffsetZeroDistanceCalibration(pdev))
    // {
    //     panic_and_stop("");
    // }

    // Main example application
    RangingLoop(pdev);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
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
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 84;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

void RangingLoop(VL53LX_DEV Dev)
{
    printf("Ranging loop starts\n\r");

    // Init sensor and start measurements
    int status = 0;
    status = VL53LX_StartMeasurement(Dev);
    if (status)
    {
        panic_and_stop("VL53LX_StartMeasurement failed: error = %d \n\r", status);
    }

    while (1)
    {
        VL53LX_MultiRangingData_t MultiRangingData;
        VL53LX_MultiRangingData_t* pMultiRangingData = &MultiRangingData;
        size_t no_of_object_found = 0;
        uint8_t NewDataReady = 0;

        // Poll for conversion
        status = VL53LX_GetMeasurementDataReady(Dev, &NewDataReady);
        HAL_Delay(1); // 1 ms polling period, could be longer.
        if (!(!status && (NewDataReady != 0)))
        {
            continue;
        }

        // Get state and number of found objects, some may be failed on each run
        status = VL53LX_GetMultiRangingData(Dev, pMultiRangingData);
        no_of_object_found = pMultiRangingData->NumberOfObjectsFound;

        // Get indices and of good objects and sum them up
        size_t no_of_good_object_found = 0;
        size_t good_object_indices[no_of_object_found];
        for (size_t i = 0; i < no_of_object_found; ++i)
        {
            size_t const good = pMultiRangingData->RangeData[i].RangeStatus == 0;
            no_of_good_object_found += good;
            good_object_indices[i] = good;
        }

        // Loop over found objects and only print valid ones
        size_t no_of_good_object_downcounter = no_of_good_object_found;
        for (size_t i = 0; i < no_of_object_found; ++i)
        {
            if (good_object_indices[i] != 0)
            {
                if (no_of_good_object_downcounter == no_of_good_object_found)
                {
                    // Print header line
                    printf(
                        "Count=%5d, #Objs=%1d: ",
                        pMultiRangingData->StreamCount,
                        no_of_good_object_found);
                }
                else
                {
                    // Space for second found object
                    printf("                      ");
                }
                --no_of_good_object_downcounter;
                printf(
                    "Object number: %1u, status=%2u, D=%5dmm, Signal=%2.2f Mcps, "
                    "Ambient=%2.2f Mcps\n\r",
                    i,
                    pMultiRangingData->RangeData[i].RangeStatus,
                    pMultiRangingData->RangeData[i].RangeMilliMeter,
                    pMultiRangingData->RangeData[i].SignalRateRtnMegaCps / 65536.0,
                    pMultiRangingData->RangeData[i].AmbientRateRtnMegaCps / 65536.0);
            }
        }
        if (status == 0)
        {
            status = VL53LX_ClearInterruptAndStartMeasurement(Dev);
        }
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
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
