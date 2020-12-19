/*
	Name: Keston Smith
	ID: 816001854
	Copyright: 
	Author: Keston Smith
	Date: 18/12/20 02:44
	Description: LAB 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/semphr.h"

#define configUSE_COUNTING_SEMAPHORES           1	// enable the use of counting semaphores
#define configUSE_IDLE_HOOK                     1  // enable the use of idle hooks
#define configUSE_MUTEXES                       1 //enable the use of mutexes

#define configUSE_IDLE_HOOK                     1
#define configUSE_TICK_HOOK                     1

//static xQueueHandle gpio_evt_queue = NULL;
static SemaphoreHandle_t xSemaphore = NULL;  //initialising semaphore 
TaskHandle_t turn_on;
TaskHandle_t turn_off;
#define GPIO_OUTPUT_IO_0    2
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO_0)

unsigned int sleep(unsigned int seconds);
uint32_t io_num=2;
static const char *TAG = "main";
TickType_t xLastWakeTime;

#define mainINTERRUPT_NUMBER    3


static void task_turn_on()
{  // extern SemaphoreHandle_t xMutex;
	

   	ESP_LOGI(TAG, "Task turn_on accessed! \n");
    
 //	const TickType_t xFrequency = pdMSTOTICKS( 500 );
for( ;; ){

    if( xSemaphore != NULL )
    {
        /* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
        if( xSemaphoreTake( xSemaphore, ( TickType_t ) 500 ) == pdTRUE )
        {	
            ESP_LOGI(TAG, "Turning on GPIO[%d], this task turn_on currently holds the mutex\n", io_num);
			gpio_set_level(GPIO_OUTPUT_IO_0, 1); // set gpio high
			//vTaskDelay(1000 / portTICK_RATE_MS); // task delay of 1 second or 1000ms
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS( 1000 )); 
            /* We have finished accessing the shared resource.  Release the
            semaphore. */
            xSemaphoreGive( xSemaphore );
        }
        else
        {
           	ESP_LOGI(TAG, "Task turn_off has the mutex currently\n");
        }
    }
    
    
    if(turn_on != NULL )
     {
         vTaskDelete(turn_on);
     }
     
    }//end for
}

static void task_turn_off()
{
	//extern SemaphoreHandle_t xMutex;
	
   	ESP_LOGI(TAG, "Task turn_off accessed! \n");
    
 //	const TickType_t xFrequency = pdMSTOTICKS( 500 );
for( ;; ){
    if( xSemaphore != NULL )
    {
        /* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
        if( xSemaphoreTake( xSemaphore, ( TickType_t ) 500 ) == pdTRUE )
        {	
            ESP_LOGI(TAG, "Turning off GPIO[%d], this task turn_off currently holds the mutex\n", io_num);
			gpio_set_level(GPIO_OUTPUT_IO_0, 0); // set gpio high
			//vTaskDelay(1000 / portTICK_RATE_MS); // task delay of 1 second or 1000ms
			vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS( 1000)); 
            /* We have finished accessing the shared resource.  Release the
            semaphore. */
            xSemaphoreGive( xSemaphore );
        }
        else
        {
           	ESP_LOGI(TAG, "Task turn_on has the mutex currently\n");
        }
        
       
    if(turn_on != NULL )
     {
         vTaskDelete(turn_off);
     }
    }
    
   } //end for
}

static void task_output(){
	for(;;){
	
	ESP_LOGI(TAG,"STATUS MESSAGE: GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
	}	
	//vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS( 1000)); 
}
/*Testing idle hook out*/
static void IDLE(){
	for(int i=0;i<100;i++){
	
	}	
	//vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS( 1000)); 
}
void app_main(void)
{
	gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

   
    xSemaphore = xSemaphoreCreateMutex();
   	
	xTaskCreate(task_turn_on, "task_turn_on", 2048, NULL, 6, NULL);

//	pvPortMalloc();
	xTaskCreate(task_turn_off, "task_turn_off", 2048, NULL, 5, NULL);

//	pvPortMalloc();
	xTaskCreate(task_output, "task_output", 2048, NULL, 10, NULL);
	
    xTaskCreate(IDLE, "IDLE", 768, NULL, 0, NULL);  //creating idle hook task
	vTaskStartScheduler();
	ESP_LOGI(TAG, "COMPLETED TASKS \n");
//	vPortFree();
    for(;;);
}
