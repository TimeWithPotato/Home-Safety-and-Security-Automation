/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "fonts.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_PIN_9
#define LED_PORT GPIOC
#define LED_PIN GPIO_PIN_13
#define TRIG_PORT GPIOA
#define TRIG_PIN GPIO_PIN_11
#define ECHO_PORT GPIOA
#define ECHO_PIN GPIO_PIN_10
#define IR_PORT GPIOB
#define IR_PIN GPIO_PIN_0
#define BUZZER_PORT GPIOB
#define BUZZER_PIN GPIO_PIN_12
#define GREEN_LED_PORT GPIOB
#define GREEN_LED_PIN GPIO_PIN_14
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
uint8_t RHI, RHD, TCI, TCD, SUM;
uint32_t pMillis, cMillis, Value1=0, Value2=0, Distance=0;
float tCelsius = 0, RH = 0;
char strCopy[20];

// Gate & Password Variables
char password[] = "331A";
char input_buffer[5];
int input_index = 0;
uint32_t gas_val = 0;

typedef enum { STATE_LOCKED, STATE_DASHBOARD } SystemState;
SystemState current_state = STATE_LOCKED;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
void microDelay(uint16_t delay);
uint8_t DHT11_Start(void);
uint8_t DHT11_Read(void);
uint32_t Get_Distance(void);
void Handle_Gate(void);
void Handle_Environment(void);
char getPressedKey(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

char getPressedKey(void)
{
    char key = 0;

    GPIO_TypeDef *portRow = GPIOA;
    GPIO_TypeDef *portCol = GPIOA;

    uint16_t rowPins[4] = {GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_5, GPIO_PIN_4};
    uint16_t colPins[4] = {GPIO_PIN_3, GPIO_PIN_2, GPIO_PIN_1, GPIO_PIN_0};

    // -------- ROW 1 --------
    HAL_GPIO_WritePin(portRow, rowPins[0], GPIO_PIN_SET);
    HAL_GPIO_WritePin(portRow, rowPins[1], GPIO_PIN_RESET);
    HAL_GPIO_WritePin(portRow, rowPins[2], GPIO_PIN_RESET);
    HAL_GPIO_WritePin(portRow, rowPins[3], GPIO_PIN_RESET);

    if (HAL_GPIO_ReadPin(portCol, colPins[0]) == GPIO_PIN_SET) key = '1';
    else if (HAL_GPIO_ReadPin(portCol, colPins[1]) == GPIO_PIN_SET) key = '2';
    else if (HAL_GPIO_ReadPin(portCol, colPins[2]) == GPIO_PIN_SET) key = '3';
    else if (HAL_GPIO_ReadPin(portCol, colPins[3]) == GPIO_PIN_SET) key = 'A';

    // -------- ROW 2 --------
    if (key == 0) {
        HAL_GPIO_WritePin(portRow, rowPins[0], GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portRow, rowPins[1], GPIO_PIN_SET);
        HAL_GPIO_WritePin(portRow, rowPins[2], GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portRow, rowPins[3], GPIO_PIN_RESET);

        if (HAL_GPIO_ReadPin(portCol, colPins[0]) == GPIO_PIN_SET) key = '4';
        else if (HAL_GPIO_ReadPin(portCol, colPins[1]) == GPIO_PIN_SET) key = '5';
        else if (HAL_GPIO_ReadPin(portCol, colPins[2]) == GPIO_PIN_SET) key = '6';
        else if (HAL_GPIO_ReadPin(portCol, colPins[3]) == GPIO_PIN_SET) key = 'B';
    }

    // -------- ROW 3 --------
    if (key == 0) {
        HAL_GPIO_WritePin(portRow, rowPins[0], GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portRow, rowPins[1], GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portRow, rowPins[2], GPIO_PIN_SET);
        HAL_GPIO_WritePin(portRow, rowPins[3], GPIO_PIN_RESET);

        if (HAL_GPIO_ReadPin(portCol, colPins[0]) == GPIO_PIN_SET) key = '7';
        else if (HAL_GPIO_ReadPin(portCol, colPins[1]) == GPIO_PIN_SET) key = '8';
        else if (HAL_GPIO_ReadPin(portCol, colPins[2]) == GPIO_PIN_SET) key = '9';
        else if (HAL_GPIO_ReadPin(portCol, colPins[3]) == GPIO_PIN_SET) key = 'C';
    }

    // -------- ROW 4 --------
    if (key == 0) {
        HAL_GPIO_WritePin(portRow, rowPins[0], GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portRow, rowPins[1], GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portRow, rowPins[2], GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portRow, rowPins[3], GPIO_PIN_SET);

        if (HAL_GPIO_ReadPin(portCol, colPins[0]) == GPIO_PIN_SET) key = '*';
        else if (HAL_GPIO_ReadPin(portCol, colPins[1]) == GPIO_PIN_SET) key = '0';
        else if (HAL_GPIO_ReadPin(portCol, colPins[2]) == GPIO_PIN_SET) key = '#';
        else if (HAL_GPIO_ReadPin(portCol, colPins[3]) == GPIO_PIN_SET) key = 'D';
    }

    // -------- DEBOUNCE --------
    if (key != 0) {
        HAL_Delay(20);
        return key;
    }

    return 0;
}

uint32_t Get_Distance(void) {
    uint32_t local_val1 = 0, local_val2 = 0;
    
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    microDelay(10);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

    // Wait for Echo to go HIGH with a short timeout
    uint32_t timeout = 0xFFFF;
    while (!(HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN)) && timeout--);
    if (timeout == 0) return 999; // Return high distance if sensor fails
    
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    local_val1 = __HAL_TIM_GET_COUNTER(&htim1);

    // Wait for Echo to go LOW
    timeout = 0xFFFF;
    while ((HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN)) && timeout--);
    local_val2 = __HAL_TIM_GET_COUNTER(&htim1);

    Distance = (local_val2 - local_val1) * 0.034 / 2;
    return Distance;
}

void microDelay(uint16_t delay) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < delay);
}

uint8_t DHT11_Start(void) {
    uint8_t Response = 0;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 0);
    HAL_Delay(18);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 1);
    microDelay(30);
    
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    microDelay(40);
    if (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))) {
        microDelay(80);
        if ((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))){ 
					Response = 1;
				}else{
					SSD1306_GotoXY(0,52);
					SSD1306_Puts("DEBUG: STUCK LOW", &Font_7x10,1);
					SSD1306_UpdateScreen();
				}
    } else{
			SSD1306_GotoXY(0,52);
			SSD1306_Puts("DEBUG: NO REPS", &Font_7x10, 1);
			SSD1306_UpdateScreen();
		}
		
		// Final wait for handshake to end
		pMillis = HAL_GetTick();
		cMillis = HAL_GetTick();
		while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
		{
			cMillis = HAL_GetTick();
		}
		
    return Response;
}

uint8_t DHT11_Read (void){
	
  uint8_t a,b;
  for (a=0;a<8;a++)
  {
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
    {  // wait for the pin to go high
      cMillis = HAL_GetTick();
    }
    microDelay (40);   // wait for 40 us
    if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))   // if the pin is low
      b&= ~(1<<(7-a));
    else
      b|= (1<<(7-a));
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
    {  // wait for the pin to go low
      cMillis = HAL_GetTick();
    }
  }
  return b;
}

/* --- Core System Logic --- */
void Run_System_Logic(void) {
    uint32_t dist = Get_Distance();
    // Read IR Sensor (PB0) - Low typically means object detected
    uint8_t ir_detected = (HAL_GPIO_ReadPin(IR_PORT, IR_PIN) == GPIO_PIN_RESET);

    SSD1306_Fill(0);

    if (current_state == STATE_LOCKED) {
        // FORCE PHYSICAL LOCK: LED OFF and Servo at 250
        HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET);
        //__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 250);

        if ((dist > 10 || dist == 0) && !ir_detected) { 
            SSD1306_GotoXY(45, 10);
            SSD1306_Puts("VAIYA", &Font_11x18, 1);
            SSD1306_GotoXY(25, 35);
					SSD1306_Puts("GIVE BONUS PLS", &Font_7x10, 1);
            
            // IR DEBUG MESSAGE
            /*if (ir_detected) {
                SSD1306_GotoXY(0, 53);
                SSD1306_Puts("IR: Someone is there", &Font_7x10, 1);
            }*/
            input_index = 0;
        } else {
            // PASSWORD PROMPT
            SSD1306_GotoXY(0, 0);
            SSD1306_Puts("Enter Password:", &Font_7x10, 1);
            
            char mask[5] = "----";
            for(int i=0; i<input_index; i++) mask[i] = '*';
            SSD1306_GotoXY(48, 30);
            SSD1306_Puts(mask, &Font_11x18, 1);

            char key = getPressedKey();
            if (key != 0 && key != '#' && key != '*') {
                input_buffer[input_index++] = key;
                if (input_index == 4) {
                    input_buffer[4] = '\0';
                    if (strcmp(input_buffer, password) == 0) {
                        current_state = STATE_DASHBOARD;
											  HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET);
											  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 750);
                    }
                    input_index = 0;
                    memset(input_buffer, 0, 4);
                }
            }
        }
    } 
    else if (current_state == STATE_DASHBOARD) {
        // FORCE PHYSICAL OPEN: LED ON and Servo at 750
       // __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 750);

        // Environment Sensors
        HAL_ADC_Start(&hadc1);
        if(HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK) gas_val = HAL_ADC_GetValue(&hadc1);
        
        int dht_ok = 0;
        if(DHT11_Start()) {
            RHI = DHT11_Read(); RHD = DHT11_Read(); 
            TCI = DHT11_Read(); TCD = DHT11_Read(); SUM = DHT11_Read(); 
            if (RHI + RHD + TCI + TCD == SUM) {
                tCelsius = (float)TCI + (float)(TCD/10.0);
                RH = (float)RHI + (float)(RHD/10.0);
                dht_ok = 1;
            }
        }

        SSD1306_GotoXY(0, 0);
        SSD1306_Puts("--- DASHBOARD ---", &Font_7x10, 1);

        if (gas_val > 1500) { 
            SSD1306_GotoXY(10, 25);
            SSD1306_Puts("GAS ALERT!", &Font_11x18, 1);
						HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
        } else if (dht_ok){
            char buf[20];
            sprintf(buf, "Temp: %.1f C", tCelsius);
            SSD1306_GotoXY(0, 20); SSD1306_Puts(buf, &Font_7x10, 1);
            sprintf(buf, "Hum:  %.1f %%", RH);
            SSD1306_GotoXY(0, 40); SSD1306_Puts(buf, &Font_7x10, 1);
						HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
        }

        // AUTO-LOCK LOGIC: Distance clear (>15) and IR path clear (Not Detected)
        if (dist > 15 && !ir_detected) {
            //SSD1306_GotoXY(0, 53);
            //SSD1306_Puts("Locking in 2s...", &Font_7x10, 1);
            //SSD1306_UpdateScreen();
            //HAL_Delay(2000); 
            //current_state = STATE_LOCKED;
					 HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET);
					__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 250);
        } else if (ir_detected) {
            SSD1306_GotoXY(0, 53);
            SSD1306_Puts("Object in Path...", &Font_7x10, 1);
        }
    }
    SSD1306_UpdateScreen();
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
	//__HAL_RCC_AFIO_CLK_ENABLE();
	//__HAL_AFIO_REMAP_TIM3_PARTIAL();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
	// Close Gate initially
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 250); 
	

  HAL_TIM_Base_Start(&htim1);               // Delay timer
	//HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3); // Servo PWM

	
	uint8_t oled_status = SSD1306_Init();
	if(oled_status !=1){
		while(1){
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			HAL_Delay(200);
		}
	}
	
	SSD1306_Fill(0);
	SSD1306_GotoXY(0,0);
	SSD1306_Puts("OLED OK", &Font_7x10, 1);
	SSD1306_UpdateScreen();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
			/*
				int x;
				for(int x = 250; x <= 1250; x += 5)
				{
						__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, x);
						HAL_Delay(15);
				}
				
				HAL_Delay(500);

				for(int x = 1250; x >= 250; x -= 5)
				{
						__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, x);
						HAL_Delay(15);
				}
				
				HAL_Delay(500);
			*/
		Run_System_Logic();
		HAL_Delay(50);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 143;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 9999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	
	/*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_9, GPIO_PIN_RESET);



  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7
                           PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/*Configure GPIO pin : PB5 */
	//GPIO_InitStruct.Pin = GPIO_PIN_5;
	//GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // Must be Alternate Function
	//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	//HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
