#include <hcc/api_fat.h>

#include "GlobalStandards.h"

#include <satellite-subsystems/isis_eps_driver.h>
#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/Timing/Time.h>

#include <string.h>

#include "TelemetryCollector.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"
#include "SubSystemModules/Maintenance/Maintenance.h"


typedef enum{
	eps_tlm,
	trxvu_tlm,
	ant_tlm,
	solar_panel_tlm,
	wod_tlm
}subsystem_tlm;

time_unix tlm_save_periods[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS] = {1,1,1,1,1};
time_unix tlm_last_save_time[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS]= {0};

/*




	*/
int GetTelemetryFilenameByType(tlm_type tlm_type, char filename[MAX_F_FILE_NAME_SIZE])
{
	if(NULL == filename){
		return -1;
	}
	switch (tlm_type) {
	case tlm_wod:
		strcpy(filename,FILENAME_WOD_TLM);
		break;
	case tlm_eps_raw_hk:
		strcpy(filename,FILENAME_EPS_HK_RAW_TLM);
		break;
	case tlm_eps_raw_cdb:
		strcpy(filename,FILENAME_EPS_HK_RAW_CDB_TLM);
		break;
	case tlm_eps_eng:
		strcpy(filename,FILENAME_EPS_ENG_TLM);
		break;
	case tlm_eps_eng_cdb:
		strcpy(filename,FILENAME_EPS_ENG_CDB_TLM);
		break;
	case tlm_eps_running_avg:
		strcpy(filename,FILENAME_EPS_RUN_AVG_TLM);
		break;
	case tlm_eps_eng_avg:
		strcpy(filename,FILENAME_EPS_ENG_AVG_CDB);
		break;
	case tlm_solar:
		strcpy(filename,FILENAME_SOLAR_PANELS_TLM);
		break;
	case tlm_tx:
		strcpy(filename,FILENAME_TX_TLM);
		break;

	case tlm_rx:
		strcpy(filename,FILENAME_RX_TLM);
		break;

	case tlm_rx_frame:
		strcpy(filename,FILENAME_RX_FRAME);
		break;
	case tlm_antenna:
		strcpy(filename,FILENAME_ANTENNA_TLM);
		break;
		default:
			return -2;
	}
	return 0;
}

int InitTelemetryCollrctor()
{
	return FRAM_read((unsigned char*)tlm_save_periods,TLM_SAVE_PERIOD_START_ADDR,NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS*sizeof(time_unix));
}
void TelemetryCollectorLogic()
{
	if (CheckExecutionTime(tlm_last_save_time[eps_tlm],tlm_save_periods[eps_tlm])){
		TelemetrySaveEPS();
		Time_getUnixEpoch(&tlm_last_save_time[eps_tlm]);
	}

	if (CheckExecutionTime(tlm_last_save_time[trxvu_tlm],tlm_save_periods[trxvu_tlm])){
		TelemetrySaveTRXVU();
		Time_getUnixEpoch(&tlm_last_save_time[trxvu_tlm]);
	}

	if (CheckExecutionTime(tlm_last_save_time[ant_tlm],tlm_save_periods[ant_tlm])){
		TelemetrySaveANT();
		Time_getUnixEpoch(&tlm_last_save_time[ant_tlm]);
	}

	if (CheckExecutionTime(tlm_last_save_time[solar_panel_tlm],tlm_save_periods[solar_panel_tlm])){
		TelemetrySaveSolarPanels();
		Time_getUnixEpoch(&tlm_last_save_time[solar_panel_tlm]);
	}

	if (CheckExecutionTime(tlm_last_save_time[wod_tlm],tlm_save_periods[wod_tlm])){
		TelemetrySaveWOD();
		Time_getUnixEpoch(&tlm_last_save_time[wod_tlm]);
	}

}

#define SAVE_FLAG_IF_FILE_CREATED(type)	if(FS_SUCCSESS != res &&NULL != tlms_created){tlms_created[(type)] = FALSE_8BIT;}

void TelemetryCreateFiles(Boolean8bit tlms_created[NUMBER_OF_TELEMETRIES]){
	FileSystemResult res;
	FRAM_read((unsigned char*)tlm_save_periods,TLM_SAVE_PERIOD_START_ADDR,NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS*sizeof(time_unix));

	// -- EPS files
	res = c_fileCreate(FILENAME_EPS_HK_RAW_TLM,sizeof(isis_eps__gethousekeepingraw__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_raw_hk)

	res = c_fileCreate(FILENAME_EPS_HK_RAW_CDB_TLM,sizeof(isis_eps__gethousekeepingrawincdb__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_raw_cdb)

	res = c_fileCreate(FILENAME_EPS_ENG_TLM,sizeof(isis_eps__gethousekeepingeng__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_eng)

	res = c_fileCreate(FILENAME_EPS_ENG_CDB_TLM,sizeof(isis_eps__gethousekeepingengincdb__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_eng_cdb)

	res = c_fileCreate(FILENAME_EPS_RUN_AVG_TLM,sizeof(isis_eps__gethousekeepingrunningavg__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_running_avg)

	res = c_fileCreate(FILENAME_EPS_ENG_AVG_CDB,sizeof(isis_eps__gethousekeepingengrunningavgincdb__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_eng_avg)


	// -- TRXVU files
	res = c_fileCreate(FILENAME_TX_TLM,sizeof(ISIStrxvuTxTelemetry));
	SAVE_FLAG_IF_FILE_CREATED(tlm_tx);

	res = c_fileCreate(FILENAME_RX_TLM,sizeof(ISIStrxvuRxTelemetry));
	SAVE_FLAG_IF_FILE_CREATED(tlm_rx);

	// -- ANT files
	res = c_fileCreate(FILENAME_ANTENNA_TLM,sizeof(ISISantsTelemetry));
	SAVE_FLAG_IF_FILE_CREATED(tlm_antenna);

	//-- SOLAR PANEL files
	res = c_fileCreate(FILENAME_SOLAR_PANELS_TLM,sizeof(int32_t)*ISIS_SOLAR_PANEL_COUNT);
	SAVE_FLAG_IF_FILE_CREATED(tlm_solar);
}

void TelemetrySaveEPS(){
	 isis_eps__gethousekeepingraw__from_t tlm_raw;
	 isis_eps__gethousekeepingrawincdb__from_t tlm_raw_cdb;
	 isis_eps__gethousekeepingeng__from_t tlm_eng;
	 isis_eps__gethousekeepingengincdb__from_t tlm_eng_cdb;
	 isis_eps__gethousekeepingrunningavg__from_t tlm_run_avg;
	 isis_eps__gethousekeepingengrunningavgincdb__from_t tlm_eng_avg_cdb;

	 if(0 == isis_eps__gethousekeepingraw__tm(EPS_I2C_BUS_INDEX,&tlm_raw)){
		 c_fileWrite(FILENAME_EPS_HK_RAW_TLM, &tlm_raw);
	 }
	 if(0 == isis_eps__gethousekeepingrawincdb__tm(EPS_I2C_BUS_INDEX,&tlm_raw_cdb)){
		 c_fileWrite(FILENAME_EPS_HK_RAW_CDB_TLM, &tlm_raw_cdb);
	 }
	 if(0 == isis_eps__gethousekeepingeng__tm(EPS_I2C_BUS_INDEX,&tlm_eng)){
		 c_fileWrite(FILENAME_EPS_ENG_TLM, &tlm_eng);
	 }
	 if(0 == isis_eps__gethousekeepingengincdb__tm(EPS_I2C_BUS_INDEX,&tlm_eng_cdb)){
		 c_fileWrite(FILENAME_EPS_ENG_CDB_TLM, &tlm_eng_cdb);
	 }
	 if(0 == isis_eps__gethousekeepingrunningavg__tm(EPS_I2C_BUS_INDEX,&tlm_run_avg)){
		 c_fileWrite(FILENAME_EPS_RUN_AVG_TLM, &tlm_run_avg);
	 }
	 if(0 == isis_eps__gethousekeepingengrunningavgincdb__tm(EPS_I2C_BUS_INDEX,&tlm_eng_avg_cdb)){
		 c_fileWrite(FILENAME_EPS_ENG_AVG_CDB, &tlm_eng_avg_cdb);
	 }

}

void TelemetrySaveTRXVU()
{
	int err = 0;
	ISIStrxvuTxTelemetry tx_tlm;
	err = IsisTrxvu_tcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX, &tx_tlm);
	if (err == 0)
	{
		c_fileWrite(FILENAME_TX_TLM, &tx_tlm);
	}


	ISIStrxvuRxTelemetry rx_tlm;
	err = IsisTrxvu_rcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX, &rx_tlm);
	if (err == 0)
	{
		c_fileWrite(FILENAME_RX_TLM, &rx_tlm);
	}



}

void TelemetrySaveANT()
{
	int err = 0;
	ISISantsTelemetry ant_tlmA, ant_tlmB;
	err = IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA,
			&ant_tlmA);
	err += IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB,
			&ant_tlmB);
	if (err == 0)
	{
		c_fileWrite(FILENAME_ANTENNA_TLM, &ant_tlmA);
		c_fileWrite(FILENAME_ANTENNA_TLM, &ant_tlmB);
	}

}

void TelemetrySaveSolarPanels()
{
	int err = 0;
	int32_t t[ISIS_SOLAR_PANEL_COUNT];
	uint8_t fault;
	if (IsisSolarPanelv2_getState() == ISIS_SOLAR_PANEL_STATE_AWAKE)
	{
		err =  IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_0, &t[0],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_1, &t[1],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_2, &t[2],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_3, &t[3],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_4, &t[4],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_5, &t[5],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_6, &t[6],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_7, &t[7],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_8, &t[8],
				&fault);

		if (err == ISIS_SOLAR_PANEL_STATE_AWAKE * ISIS_SOLAR_PANEL_COUNT)
		{
			c_fileWrite(FILENAME_SOLAR_PANELS_TLM, t);
		}
	}
}

void TelemetrySaveWOD()
{
	WOD_Telemetry_t wod = { 0 };
	GetCurrentWODTelemetry(&wod);
	c_fileWrite(FILENAME_WOD_TLM, &wod);
}

void GetCurrentWODTelemetry(WOD_Telemetry_t *wod){
	if (NULL == wod){
		return;
	}
	memset(wod,0,sizeof(*wod));
	int err = 0;

	FN_SPACE space = { 0 };
	int drivenum = f_getdrive();

	err = f_getfreespace(drivenum, &space);
	if (err == F_NO_ERROR){
		wod->free_memory = space.free;
		wod->corrupt_bytes = space.bad;
	}
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);
	wod->sat_time = current_time;


	isis_eps__gethousekeepingrunningavg__from_t tlm_eng;
	err = isis_eps__gethousekeepingrunningavg__tm(EPS_I2C_BUS_INDEX,&tlm_eng);
	if(0 == err){
		wod->vbat = tlm_eng.fields.batt_input.fields.volt;
		wod->charging_power = tlm_eng.fields.batt_input.fields.power;
		wod->voltage_channel_0 = tlm_eng.fields.vip_obc00.fields.volt;
		wod->voltage_channel_1 = tlm_eng.fields.vip_obc01.fields.volt;
		wod->voltage_channel_2 = tlm_eng.fields.vip_obc02.fields.volt;
		wod->voltage_channel_3 = tlm_eng.fields.vip_obc03.fields.volt;
		wod->voltage_channel_4 = tlm_eng.fields.vip_obc04.fields.volt;
		wod->voltage_channel_5 = tlm_eng.fields.vip_obc05.fields.volt;
		wod->voltage_channel_6 = tlm_eng.fields.vip_obc06.fields.volt;
		wod->voltage_channel_7 = tlm_eng.fields.vip_obc07.fields.volt;

		wod->current_channel_0 = tlm_eng.fields.vip_obc00.fields.current;
		wod->current_channel_1 = tlm_eng.fields.vip_obc01.fields.current;
		wod->current_channel_2 = tlm_eng.fields.vip_obc02.fields.current;
		wod->current_channel_3 = tlm_eng.fields.vip_obc03.fields.current;
		wod->current_channel_4 = tlm_eng.fields.vip_obc04.fields.current;
		wod->current_channel_5 = tlm_eng.fields.vip_obc05.fields.current;
		wod->current_channel_6 = tlm_eng.fields.vip_obc06.fields.current;
		wod->current_channel_7 = tlm_eng.fields.vip_obc07.fields.current;

	}

	FRAM_read((unsigned char*)&wod->number_of_resets,
	NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
}

