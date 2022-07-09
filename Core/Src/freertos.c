/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Serial.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SERIAL_RX_QUEUE_LEN		8
#define SERIAL_RX_QUEUE_SIZE		(3 + SERIAL_DATABYTE)
#define JOINT_QUEUE_LEN			8
#define JOINT_QUEUE_SIZE			5 // 1 + 2 + 2
#define COOR_QUEUE_LEN			8
#define COOR_QUEUE_SIZE			(SERIAL_DATABYTE - 2)
#define PRESSURE_QUEUE_LEN		2
#define PRESSURE_QUEUE_SIZE		4
#define RANGE_QUEUE_LEN			2
#define RANGE_QUEUE_SIZE			4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
QueueHandle_t Serial_Queue;
QueueHandle_t JointDrive_Queue;
QueueHandle_t Coordinate_Queue;
QueueHandle_t PressureSensor_Queue;
QueueHandle_t RangingSensor_Queue;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId JointTaskHandle;
osThreadId RangingTaskHandle;
osThreadId SerialTaskHandle;
osThreadId WeighingTaskHandle;
osThreadId AppTaskHandle;
osSemaphoreId SYNC_SignalHandle;
osSemaphoreId CameraOpen_SigalHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void JointTaskFunc(void const * argument);
void RangingTaskFunc(void const * argument);
void SerialTaskFunc(void const * argument);
void WeighingTaskFunc(void const * argument);
void AppTaskFunc(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of SYNC_Signal */
  osSemaphoreDef(SYNC_Signal);
  SYNC_SignalHandle = osSemaphoreCreate(osSemaphore(SYNC_Signal), 1);

  /* definition and creation of CameraOpen_Sigal */
  osSemaphoreDef(CameraOpen_Sigal);
  CameraOpen_SigalHandle = osSemaphoreCreate(osSemaphore(CameraOpen_Sigal), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	Serial_Queue = xQueueCreate((UBaseType_t)SERIAL_RX_QUEUE_LEN,(UBaseType_t)SERIAL_RX_QUEUE_SIZE);
	if(NULL == Serial_Queue)
	{
	  // Queue Create Error
	}
	JointDrive_Queue = xQueueCreate((UBaseType_t)JOINT_QUEUE_LEN,(UBaseType_t)JOINT_QUEUE_SIZE);
	if(NULL == JointDrive_Queue)
	{
	  // Queue Create Error
	}
	Coordinate_Queue = xQueueCreate((UBaseType_t)COOR_QUEUE_LEN,(UBaseType_t)COOR_QUEUE_SIZE);
	if(NULL == Coordinate_Queue)
	{
	  // Queue Create Error
	}
	PressureSensor_Queue = xQueueCreate((UBaseType_t)PRESSURE_QUEUE_LEN,(UBaseType_t)PRESSURE_QUEUE_SIZE);
	if(NULL == PressureSensor_Queue)
	{
	  // Queue Create Error
	}
	RangingSensor_Queue = xQueueCreate((UBaseType_t)RANGE_QUEUE_LEN,(UBaseType_t)RANGE_QUEUE_SIZE);
	if(NULL == RangingSensor_Queue)
	{
	  // Queue Create Error
	}
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityIdle, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of JointTask */
  osThreadDef(JointTask, JointTaskFunc, osPriorityAboveNormal, 0, 128);
  JointTaskHandle = osThreadCreate(osThread(JointTask), NULL);

  /* definition and creation of RangingTask */
  osThreadDef(RangingTask, RangingTaskFunc, osPriorityNormal, 0, 128);
  RangingTaskHandle = osThreadCreate(osThread(RangingTask), NULL);

  /* definition and creation of SerialTask */
  osThreadDef(SerialTask, SerialTaskFunc, osPriorityNormal, 0, 128);
  SerialTaskHandle = osThreadCreate(osThread(SerialTask), NULL);

  /* definition and creation of WeighingTask */
  osThreadDef(WeighingTask, WeighingTaskFunc, osPriorityNormal, 0, 128);
  WeighingTaskHandle = osThreadCreate(osThread(WeighingTask), NULL);

  /* definition and creation of AppTask */
  osThreadDef(AppTask, AppTaskFunc, osPriorityHigh, 0, 256);
  AppTaskHandle = osThreadCreate(osThread(AppTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_JointTaskFunc */
/**
* @brief Function implementing the JointTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_JointTaskFunc */
__weak void JointTaskFunc(void const * argument)
{
  /* USER CODE BEGIN JointTaskFunc */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END JointTaskFunc */
}

/* USER CODE BEGIN Header_RangingTaskFunc */
/**
* @brief Function implementing the RangingTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RangingTaskFunc */
__weak void RangingTaskFunc(void const * argument)
{
  /* USER CODE BEGIN RangingTaskFunc */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END RangingTaskFunc */
}

/* USER CODE BEGIN Header_SerialTaskFunc */
/**
* @brief Function implementing the SerialTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SerialTaskFunc */
__weak void SerialTaskFunc(void const * argument)
{
  /* USER CODE BEGIN SerialTaskFunc */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END SerialTaskFunc */
}

/* USER CODE BEGIN Header_WeighingTaskFunc */
/**
* @brief Function implementing the WeighingTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_WeighingTaskFunc */
__weak void WeighingTaskFunc(void const * argument)
{
  /* USER CODE BEGIN WeighingTaskFunc */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END WeighingTaskFunc */
}

/* USER CODE BEGIN Header_AppTaskFunc */
/**
* @brief Function implementing the AppTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AppTaskFunc */
__weak void AppTaskFunc(void const * argument)
{
  /* USER CODE BEGIN AppTaskFunc */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END AppTaskFunc */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
