#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hal/Utility/util.h>

#include "TelemetryTestingDemo.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"

Boolean PrintWodTest()
{
	WOD_Telemetry_t wod = { 0 };
	GetCurrentWODTelemetry(&wod);
	printf("Current Unix time of the satellites clock [sec]: %d\n\r", wod.sat_time);
	printf("The charging power [mW]: %d\n\r", wod.charging_power);
	printf("The voltage on the battery [mV]: %d\n\r", wod.vbat);

	printf("The output voltage on channel 0 [mV]: %d\n\r", wod.voltage_channel_0);
	printf("The output voltage on channel 1 [mV]: %d\n\r", wod.voltage_channel_1);
	printf("The output voltage on channel 2 [mV]: %d\n\r", wod.voltage_channel_2);
	printf("The output voltage on channel 3 [mV]: %d\n\r", wod.voltage_channel_3);
	printf("The output voltage on channel 4 [mV]: %d\n\r", wod.voltage_channel_4);
	printf("The output voltage on channel 5 [mV]: %d\n\r", wod.voltage_channel_5);
	printf("The output voltage on channel 6 [mV]: %d\n\r", wod.voltage_channel_6);
	printf("The output voltage on channel 7 [mV]: %d\n\r", wod.voltage_channel_7);

	printf("The output current on channel 0 [mA]: %d\n\r", wod.current_channel_0);
	printf("The output current on channel 1 [mA]: %d\n\r", wod.current_channel_1);
	printf("The output current on channel 2 [mA]: %d\n\r", wod.current_channel_2);
	printf("The output current on channel 3 [mA]: %d\n\r", wod.current_channel_3);
	printf("The output current on channel 4 [mA]: %d\n\r", wod.current_channel_4);
	printf("The output current on channel 5 [mA]: %d\n\r", wod.current_channel_5);
	printf("The output current on channel 6 [mA]: %d\n\r", wod.current_channel_6);
	printf("The output current on channel 7 [mA]: %d\n\r", wod.current_channel_7);

	printf("Number of bytes free in the satellites SD [byte]: %d\n\r", wod.free_memory);
	printf("Number of currpted bytes in the memory	[bytes]: %d\n\r", wod.corrupt_bytes);
	printf("Counts the number of resets the satellite has gone through [#]: %d\n\r", wod.number_of_resets);
	return TRUE;
}

static Boolean TestTelemetryLogic()
{
	printf("\nPlease insert number of minutes to test(0 to 10)\n");
	int minutes = 0;
	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&minutes,0,10) == 0);

	portLONG curr_time = xTaskGetTickCount();
	portLONG end_time = MINUTES_TO_TICKS(minutes) + curr_time;

	while(end_time - curr_time > 0)
	{
		curr_time = xTaskGetTickCount();

		TelemetryCollectorLogic();
		vTaskDelay(500);
		printf("time left: %d", end_time - curr_time);
	}
	return TRUE;
}

static Boolean PrintFile()
{
	printf("\nPlease insert number matching c file (just like dump requests)\n");
	int err;
	int type = 0;
	char filename[MAX_F_FILE_NAME_SIZE] = { 0 };
	//TODO: Add deinit and tellemetry collector logic
	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&type,0,12) == 0);

	err = GetTelemetryFilenameByType(type, filename);
	if (0 != err) {
		printf("could not get filename for type %d \n", type);
		return TRUE;
	}
	printf("---START PRINT---\n");
	print_file(filename);
	printf("---END PRINT---\n");
	return TRUE;
}

Boolean selectAndExecuteTelemetryDemoTest()
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Print WOD \n\r");
	printf("\t 2) Test Telemetry Collector Logic \n\r");
	printf("\t 3) Print c_file \n\r");

	unsigned int number_of_tests = 3;
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = PrintWodTest();
		break;
	case 2:
		offerMoreTests = TestTelemetryLogic();
		break;
	case 3:
		offerMoreTests = PrintFile();
		break;
	default:
		break;
	}
	return offerMoreTests;
}

Boolean MainTelemetryTestBench()
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTelemetryDemoTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
	return FALSE;
}
