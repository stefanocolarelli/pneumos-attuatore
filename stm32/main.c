#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>  // Per la funzione `sin()`
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;  // Gestore della periferica UART (USART2)
/* USER CODE BEGIN PV */
double frequency = 3.43;  // Frequenza iniziale (atti al minuto = 14)
double omega;             // Pulsazione calcolata dall'attuale frequenza
double Amplitude = 0.9;   // Escursione massima iniziale
double phase = 0.0;       // Fase corrente per mantenere la continuità
int cheneyStokesMode = 0; // Flag per indicare la modalità Cheyne-Stokes
int cheneyStokesStep = 0; // Step corrente nella sequenza Cheyne-Stokes
double targetFrequency = 3.43; // Frequenza di destinazione per la transizione graduale
double targetAmplitude = 0.9;  // Amplitude di destinazione per la transizione graduale
double baseFrequency = 3.43;  // Frequenza di base (ad esempio quella iniziale)
double baseAmplitude = 0.9;   // Ampiezza di base
/* USER CODE END PV */

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
void SetFrequencyFromSerialInput(void);  // Prototipo della funzione per la gestione della frequenza
void PerformCheyneStokesSequence(void);  // Prototipo della funzione per gestire la sequenza di Cheyne-Stokes
void PerformBreathCycle(void);           // Prototipo della funzione per gestire un ciclo di respirazione

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_USART2_UART_Init();
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    while (1)
    {
        // Controllo la frequenza e l'ampiezza dall'input seriale
        SetFrequencyFromSerialInput();

        if (cheneyStokesMode)
        {
            // Modalità Cheyne-Stokes
            PerformCheyneStokesSequence();
        }
        else
        {
            // Modalità normale
            PerformBreathCycle();  // Esegui il ciclo di respirazione normale
        }
    }
}

/**
* @brief Funzione per eseguire un ciclo di respirazione normale (inspirazione ed espirazione)
* @retval None
*/
void PerformBreathCycle(void)
{
    int32_t rangeMax = 135135;
    int32_t rangeMin = 77220;
    int32_t range = rangeMax - rangeMin;

    // Aggiornamento graduale della frequenza e ampiezza verso i nuovi target
    // Questo garantisce che, se i target cambiano, il movimento avviene gradualmente
    frequency += (targetFrequency - frequency) * 0.01;  // Avvicinamento graduale
    Amplitude += (targetAmplitude - Amplitude) * 0.01;  // Avvicinamento graduale

    // Calcola la nuova pulsazione basata sulla frequenza aggiornata
    omega = 2 * 3.14159 * frequency;

    // Aggiorna la fase per mantenere la continuità e determinare se si è completato un ciclo
    double deltaTime = 0.0005;   // Incremento di tempo (0.5 ms)
    double inspirationRatio = 0.4;  // Durata della fase di inspirazione
    double expirationRatio = 0.6;   // Durata della fase di espirazione
    double totalDuration = inspirationRatio + expirationRatio;

    // Determina se è inspirazione o espirazione e aggiorna la fase
    if (sin(phase) >= 0)
    {
        // Inspirazione (più veloce)
        phase += omega * deltaTime * (totalDuration / inspirationRatio);
    }
    else
    {
        // Espirazione (più lenta)
        phase += omega * deltaTime * (totalDuration / expirationRatio);
    }

    // Mantieni la fase nell'intervallo [0, 2π]
    if (phase > 2 * 3.14159)
    {
        phase -= 2 * 3.14159;

        // Aggiungi una variazione randomica alla frequenza e all'ampiezza alla fine di ogni ciclo completo
        double randomFrequencyOffset = ((rand() % 60) - 30) / 100.0;  // Variazione random tra -0.3 e +0.3
        double randomAmplitudeOffset = ((rand() % 20) - 10) / 100.0;  // Variazione random tra -0.1 e +0.1

        // Calcola il nuovo target basato sui valori di base
        targetFrequency = baseFrequency + randomFrequencyOffset;
        targetAmplitude = baseAmplitude + randomAmplitudeOffset;

        // Mantieni i valori di target in un range valido
        if (targetFrequency < 1.0) targetFrequency = 1.0;  // Limite minimo per la frequenza
        if (targetAmplitude < 0.1) targetAmplitude = 0.1;  // Limite minimo per l'ampiezza
    }

    // Genera il segnale PWM con la frequenza e ampiezza aggiornata
    double sinValue = sin(phase);
    static double previousCCR1 = 0;
    double targetCCR1 = rangeMin + 0.5 * range + 0.5 * Amplitude * range * sinValue;
    double filteredCCR1 = previousCCR1 + 0.1 * (targetCCR1 - previousCCR1);  // Filtro per smussare i cambiamenti
    TIM2->CCR1 = (int)filteredCCR1;
    previousCCR1 = filteredCCR1;

    HAL_Delay(1);  // 1 millisecondo di ritardo
}

/**
* @brief  Funzione per impostare la frequenza basata sull'input seriale
* @retval None
*/
void SetFrequencyFromSerialInput(void)
{
    uint8_t receivedData;
    // Controlla se c'è un nuovo dato sulla porta seriale
    if (HAL_UART_Receive(&huart2, &receivedData, 1, 10) == HAL_OK)
    {
        if (receivedData >= '0' && receivedData <= '6')  // Stato 0 a 6
        {
            baseFrequency = (receivedData == '0') ? 2.57 :
                            (receivedData == '1') ? 3.86 :
                            (receivedData == '2') ? 4.71 :
                            (receivedData == '3') ? 6.42 :
                            (receivedData == '4') ? 7.71 :
                            (receivedData == '5') ? baseFrequency :  // Respiro di Cheyne-Stokes
                            3.00;

            baseAmplitude = (receivedData == '0') ? 0.92 :
                            (receivedData == '1') ? 0.80 :
                            (receivedData == '2') ? 0.74 :
                            (receivedData == '3') ? 0.68 :
                            (receivedData == '4') ? 0.60 :
                            (receivedData == '5') ? baseAmplitude :  // Respiro di Cheyne-Stokes
                            0.86;

            cheneyStokesMode = (receivedData == '5') ? 1 : 0;
            cheneyStokesStep = 0;  // Reset dello step per uscire da Cheyne-Stokes se necessario
        }
    }
}

/*
 *
 *
 * case '0':
                    frequency = 2.57;
                    Amplitude = 0.92;
                    cheneyStokesMode = 0;
                    cheneyStokesStep = 0;  // Reset dello step
                    break;  // 12 atti al minuto
                case '1':
                    frequency = 3.86;
                    Amplitude = 0.80;
                    cheneyStokesMode = 0;
                    cheneyStokesStep = 0;  // Reset dello step
                    break;  // 18 atti al minuto
                case '2':
                    frequency = 4.71;
                    Amplitude = 0.74;
                    cheneyStokesMode = 0;
                    cheneyStokesStep = 0;  // Reset dello step
                    break;  // 22 atti al minuto
                case '3':
                    frequency = 6.42;
                    Amplitude = 0.68;
                    cheneyStokesMode = 0;
                    cheneyStokesStep = 0;  // Reset dello step
                    break;  // 30 atti al minuto
                case '4':
                    frequency = 7.71;
                    Amplitude = 0.60;
                    cheneyStokesMode = 0;
                    cheneyStokesStep = 0;  // Reset dello step
                    break;  // 36 atti al minuto
                case '5':
                    cheneyStokesMode = 1;
                    cheneyStokesStep = 0;
                    break;  // Respiro di Cheyne-Stokes
 */


/**
* @brief Funzione per eseguire la sequenza di Cheyne-Stokes
* @retval None
*/
void PerformCheyneStokesSequence(void)
{
    // Array for the amplitude and speed of each breath cycle (inspiration and expiration)
	// First
    static double cheneyAmplitudes[] = 	{0.10, 0.20, 0.20, 0.30, 0.30, 0.40, 0.40, 0.50, 0.60, 0.50, 0.40, 0.20, 0.20, 0.10};
    static double cheneySpeeds[] = 		{5.90, 5.80, 5.90, 6.60, 7.20, 8.40, 9.30, 9.10, 8.90, 9.20, 9.50, 8.30, 8.00, 8.00};
    static int cheneyPause = 0;

    // Gestione della pausa di apnea
    if (cheneyPause > 0)
    {
        if (!cheneyStokesMode)  // Interrompe la pausa se la modalità cambia
        {
            cheneyPause = 0;
            return;
        }
        HAL_Delay(1000);  // 1 secondo di ritardo
        cheneyPause--;
        return;
    }

    if (!cheneyStokesMode)  // Se la modalità cambia, interrompi immediatamente
    {
        cheneyStokesStep = 0;
        cheneyPause = 0;
        return;
    }

    // Amplitude = cheneyAmplitudes[cheneyStokesStep];  // Seleziona l'ampiezza corrente
    //omega = cheneySpeeds[cheneyStokesStep];          // Seleziona la velocità corrente

    Amplitude = cheneyAmplitudes[cheneyStokesStep] - ((rand() % 30) / 100);  // Seleziona l'ampiezza corrente e sottrai un valore randomico tra 0.01 e 0.30  // Seleziona l'ampiezza corrente
    omega = cheneySpeeds[cheneyStokesStep] - ((rand() % 30) / 100);  // Seleziona la velocità corrente e sottrai un valore randomico tra 0.01 e 0.30          // Seleziona la velocità corrente

    // Esegui un atto completo (inspirazione ed espirazione)
    phase = 0.0;

    // Esegui il ciclo completo (ispirazione ed espirazione)
    while (phase < 2.0 * 3.14159)
    {
        if (!cheneyStokesMode)  // Controlla frequentemente la modalità per uscire
        {
            cheneyStokesStep = 0;
            cheneyPause = 0;
            return;
        }

        double sinValue = sin(phase);
        static double previousCCR1 = 0;
        double targetCCR1 = 77220 + 0.5 * (135135 - 77220) + 0.5 * Amplitude * (135135 - 77220) * sinValue;
        double filteredCCR1 = previousCCR1 + 0.05 * (targetCCR1 - previousCCR1);  // Filtro per smussare i cambiamenti
        TIM2->CCR1 = (int)filteredCCR1;
        previousCCR1 = filteredCCR1;

        // Incremento della fase per mantenere una velocità costante
        phase += omega * 0.001;  // Incrementa fase con un passo costante
        HAL_Delay(1);
    }

    // Passa alla prossima fase della sequenza
    cheneyStokesStep++;
    if (cheneyStokesStep >= sizeof(cheneyAmplitudes) / sizeof(cheneyAmplitudes[0]))
    {
        cheneyStokesStep = 0;  // Reset to the beginning of the amplitude sequence
        cheneyPause = (rand() % 5 + 12);  // Apnea phase (random between 12 and 17 seconds pause)
    }
}

/**
* @brief UART2 Initialization Function
* @param None
* @retval None
*/
static void MX_USART2_UART_Init(void)
{
huart2.Instance = USART2;
//huart2.Init.BaudRate = 115200;  // Imposta il baud rate a 9600 (modificabile in base alle tue esigenze)
huart2.Init.BaudRate = 9600;  // Imposta il baud rate a 9600 (modificabile in base alle tue esigenze)
huart2.Init.WordLength = UART_WORDLENGTH_8B;
huart2.Init.StopBits = UART_STOPBITS_1;
huart2.Init.Parity = UART_PARITY_NONE;
huart2.Init.Mode = UART_MODE_TX_RX;
huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart2.Init.OverSampling = UART_OVERSAMPLING_16;
if (HAL_UART_Init(&huart2) != HAL_OK)
{
Error_Handler();
}
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
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
RCC_OscInitStruct.HSIState = RCC_HSI_ON;
RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
RCC_OscInitStruct.PLL.PLLM = 16;
RCC_OscInitStruct.PLL.PLLN = 144;
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
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
{
Error_Handler();
}
}
/**
* @brief TIM2 Initialization Function
* @param None
* @retval None
*/
static void MX_TIM2_Init(void)
{
/* USER CODE BEGIN TIM2_Init 0 */
/* USER CODE END TIM2_Init 0 */
TIM_ClockConfigTypeDef sClockSourceConfig = {0};
TIM_MasterConfigTypeDef sMasterConfig = {0};
TIM_OC_InitTypeDef sConfigOC = {0};
/* USER CODE BEGIN TIM2_Init 1 */
/* USER CODE END TIM2_Init 1 */
htim2.Instance = TIM2;
htim2.Init.Prescaler = 0;
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
htim2.Init.Period = 216216;
htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
{
Error_Handler();
}
sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
{
Error_Handler();
}
if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
{
Error_Handler();
}
sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
{
Error_Handler();
}
sConfigOC.OCMode = TIM_OCMODE_PWM1;
sConfigOC.Pulse = 0;
sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
{
Error_Handler();
}
/* USER CODE BEGIN TIM2_Init 2 */
/* USER CODE END TIM2_Init 2 */
HAL_TIM_MspPostInit(&htim2);
}
/**
* @brief GPIO Initialization Function
* @param None
* @retval None
*/
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */
/* GPIO Ports Clock Enable */
__HAL_RCC_GPIOH_CLK_ENABLE();
__HAL_RCC_GPIOA_CLK_ENABLE();
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
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
