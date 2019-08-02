#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/IsisTRXVU.h>

#include <hal/Timing/Time.h>
#include <hal/Storage/FRAM.h>

#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"

#include "GlobalStandards.h"
#include "FRAM_FlightParameters.h"

#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SPL.h"
#include "Beacon.h"
#include "SatDataTx.h"

time_unix g_prev_beacon_time = 0;				// the time at which the previous beacon occured
time_unix g_beacon_interval_time = 0;			// seconds between each beacon
unsigned char g_current_beacon_period = 0;		// marks the current beacon cycle(how many were transmitted before change in baud)
unsigned char g_beacon_change_baud_period = 0;	// every 'g_beacon_change_baud_period' beacon will be in 1200Bps and not 9600Bps

void InitBeaconParams()
{
	if(0!= FRAM_read(&g_beacon_change_baud_period,BEACON_BITRATE_CYCLE_ADDR,BEACON_BITRATE_CYCLE_SIZE))
	{
		g_beacon_change_baud_period = DEFALUT_BEACON_BITRATE_CYCLE;
	}

	if (0	!= FRAM_read((unsigned char*) &g_beacon_interval_time,BEACON_INTERVAL_TIME_ADDR,BEACON_INTERVAL_TIME_SIZE))
	{
		g_beacon_interval_time = DEFAULT_BEACON_INTERVAL_TIME;
	}
}

//Sets the bitrate to 1200 every third beacon and to 9600 otherwise
int BeaconSetBitrate()
{

	int err = 0;
	if (g_current_beacon_period % g_beacon_change_baud_period == 0) {
		err = IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,
				trxvu_bitrate_1200);
		g_current_beacon_period = 0;
	}
	else {
		err = IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,
				trxvu_bitrate_9600);
	}
	g_current_beacon_period++;
	return err;
}

void BeaconLogic()
{
	int err = 0;

	if (!CheckExecutionTime(g_prev_beacon_time, g_beacon_interval_time)) {
		return;
	}

	WOD_Telemetry_t wod = { 0 };
	GetCurrentWODTelemetry(&wod);

	sat_packet_t cmd = { 0 };
	err = AssembleCommand((unsigned char*) &wod, sizeof(wod), trxvu_cmd_type,
			BEACON_SUBTYPE, 0xFFFFFFFF, &cmd);
	if (0 != err) {
		return;
	}

	Time_getUnixEpoch(&g_prev_beacon_time);

	BeaconSetBitrate();

	TransmitSplPacket(&cmd, NULL);
	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);

#ifdef TESTING
	printf("beacon was transmitted, timestamp: %lu\n", g_prev_beacon_time);
#endif

}

int UpdateBeaconBaudCycle(unsigned char cycle)
{
	if (cycle < DEFALUT_BEACON_BITRATE_CYCLE) {
		return E_PARAM_OUTOFBOUNDS;
	}
	int err = FRAM_write((unsigned char*) &cycle, BEACON_BITRATE_CYCLE_ADDR,
	BEACON_BITRATE_CYCLE_SIZE);
	if (0 != err) {
		return err;
	}
	g_beacon_change_baud_period = cycle;
	return E_NO_SS_ERR;
}

int UpdateBeaconInterval(time_unix intrvl)
{
	if (intrvl > MAX_BEACON_INTERVAL || intrvl < MIN_BEACON_INTERVAL) {
		return E_PARAM_OUTOFBOUNDS;
	}
	int err = FRAM_write((unsigned char*) &intrvl, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);
	if (0 != err) {
		return err;
	}
	g_beacon_interval_time = intrvl;
	return E_NO_SS_ERR;
}
