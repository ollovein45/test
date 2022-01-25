#include "esp_system.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_event_base.h"

#define BLINK_GPIO GPIO_NUM_2

static const char* TAG = "Debug";

QueueHandle_t xQueue = NULL;

uint32_t time_last, time_new, result ;
uint32_t (*tbt)(void);
uint32_t time_between_tasks (void)//a function that determines the amount of time since the last call
{
	time_last = time_new;
	time_new = esp_log_early_timestamp();
	if(time_new<time_last)
	{
		return result;
	}
	else
		{
			return result = time_new - time_last;
		}
}


/*------------------------------------------------------------------------*/
static void ReceiverTask(void *pvParameter)
{
	uint8_t counter = 0;
	tbt = time_between_tasks;
	for( ;; )
	{
		if( xQueue != NULL )
		{
			if(xQueueReceive(xQueue, &counter, (TickType_t) (100/portTICK_PERIOD_MS)))
			{
				ESP_LOGI(TAG, "Task [RX]: received:    %u TBT: %u", counter, tbt());
			}
//			else ESP_LOGI(TAG, "Value don't received ");

		}
//		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
/*------------------------------------------------------------------------*/
static void TransmitterTask(void *pvParameter)
{
	uint8_t counter;
	for( ;; )
	{
	    xQueueSend(xQueue, (void *)&counter, (TickType_t) 100/portTICK_PERIOD_MS);//send the counter value to the queue
	    if(counter < 255)
	    	{
	    		counter++;
	    	}
	    else counter = 0;

	    ESP_LOGI(TAG, "Task [TX]: transmitted: %u", counter);
	    vTaskDelay(5000/portTICK_PERIOD_MS);
	}
}
/*------------------------------------------------------------------------*/
void app_main(void)
{


    xQueue = xQueueCreate(5, sizeof(uint8_t));//create a queue for 5 values
	if(xQueue != NULL)
	{
		ESP_LOGI(TAG, "Queue is created");
		xTaskCreate(&TransmitterTask, "TX", 2048, NULL, 1, NULL );//create transmitter task
		ESP_LOGI(TAG, "TX task is created");
		xTaskCreate(&ReceiverTask, "RX", 2048, NULL, 1, NULL);//create receiver task
		ESP_LOGI(TAG, "RX task is created");

	}
	else {
		ESP_LOGW(TAG, "Queue is not created \n");
	}
}

