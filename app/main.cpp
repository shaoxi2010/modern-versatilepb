#include "FreeRTOS.h"
#include "projdefs.h"
#include "task.h"
#include <cstddef>
#include <stdio.h>

typedef struct _paramStruct {
	portCHAR *text;	   /* text to be printed by the task */
	UBaseType_t delay; /* delay in milliseconds */
} paramStruct;

static const portCHAR defaultText[] = "<NO TEXT>\r\n";
static const UBaseType_t defaultDelay = 1000;

void vTaskFunction(void *pvParameters)
{
	const portCHAR *taskName;
	UBaseType_t delay;
	paramStruct *params = (paramStruct *)pvParameters;

	taskName =
		(NULL == params || NULL == params->text ? defaultText : params->text);
	delay = (NULL == params ? defaultDelay : params->delay);

	for (;;) {
		/* Print out the name of this task. */

		printf(taskName);

		vTaskDelay(delay / portTICK_RATE_MS);
	}

	/*
	 * If the task implementation ever manages to break out of the
	 * infinite loop above, it must be deleted before reaching the
	 * end of the function!
	 */
	vTaskDelete(NULL);
}

int main()
{
	if (pdTRUE != xTaskCreate(vTaskFunction, "task1", configMINIMAL_STACK_SIZE,
							  NULL, 0, NULL)) {
		printf("task error\n");
	}
	vTaskStartScheduler();
	printf("exit!!!\n");
	return 0;
}