#include <freertos/FreeRTOS.h>

#include "TelemetryTestingDemo.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"

Boolean MainTelemetryTestBench()
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
