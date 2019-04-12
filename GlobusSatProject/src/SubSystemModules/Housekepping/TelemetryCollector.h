
#ifndef TELEMETRYCOLLECTOR_H_
#define TELEMETRYCOLLECTOR_H_

#include "GlobalStandards.h"



//TODO: finish WOD telemetry according to requirements... TRX TLM...
typedef struct __attribute__ ((__packed__)) WOD_Telemetry_t
{
	voltage_t current_vbat;				///< the current voltage on the battery [mV]
	current_t present_electric_current;	///< the up-to-date electric current of the battery [mA]
	current_t present_3V3_current;		///< the up-to-date 3.3 Volt bus current of the battery [mA]
	current_t present_5V_current;		///< the up-to-date 5 Volt bus current of the battery [mA]
	time_unix current_sat_time;			///< current Unix time of the satellites clock [sec]
	unsigned int free_memory;			///< number of bytes free in the satellites SD [byte]
	unsigned short number_of_resets;	///< counts the number of resets the satellite has gone through [#]
}WOD_Telemetry_t;


int TelemetryCollectorLogic();

int GetCurrentWODTelemetry(WOD_Telemetry_t *wod);

#endif /* TELEMETRYCOLLECTOR_H_ */
