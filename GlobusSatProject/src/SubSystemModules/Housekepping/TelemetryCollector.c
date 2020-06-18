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

time_unix tlm_save_periods[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS] = {5,5,5,5,5};
time_unix tlm_last_save_time[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS]= {0};

int GetTelemetryFilenameByType(tlm_type tlm_type, char filename[MAX_F_FILE_NAME_SIZE])
{
	if(NULL == filename){
		return -1;
	}
	switch (tlm_type) {
	case tlm_wod:
		strcpy(filename,FILENAME_WOD_TLM);
		break;
	case tlm_eps_eng:
		strcpy(filename,FILENAME_EPS_ENG_TLM);
		break;
	case tlm_solar:
		strcpy(filename,FILENAME_SOLAR_PANELS_TLM);
		break;
	case tlm_tx_revc:
		strcpy(filename,FILENAME_TX_REVC);
		break;
	case tlm_rx_revc:
		strcpy(filename,FILENAME_RX_REVC);
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

// TODO: Add log error to each telemetry save
void TelemetryCollectorLogic()
{
	if (CheckExecutionTime(tlm_last_save_time[eps_tlm],tlm_save_periods[eps_tlm])){
		printf("still going strong\n");
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

	// -- WOD files
	res = c_fileCreate(FILENAME_WOD_TLM,sizeof(WOD_Telemetry_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_wod)

	// -- EPS files
	res = c_fileCreate(FILENAME_EPS_ENG_TLM,sizeof(isis_eps__gethousekeepingeng__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_eng)

	// -- TRXVU files
	res = c_fileCreate(FILENAME_TX_REVC,sizeof(ISIStrxvuRxTelemetry));
	SAVE_FLAG_IF_FILE_CREATED(tlm_tx_revc);

	res = c_fileCreate(FILENAME_RX_REVC,sizeof(ISIStrxvuRxTelemetry));
	SAVE_FLAG_IF_FILE_CREATED(tlm_rx_revc);

	// -- ANT files
	res = c_fileCreate(FILENAME_ANTENNA_TLM,sizeof(ISISantsTelemetry));
	SAVE_FLAG_IF_FILE_CREATED(tlm_antenna);

	//-- SOLAR PANEL files
	res = c_fileCreate(FILENAME_SOLAR_PANELS_TLM,sizeof(int32_t)*ISIS_SOLAR_PANEL_COUNT);
	SAVE_FLAG_IF_FILE_CREATED(tlm_solar);
}

void TelemetrySaveEPS(){
	 isis_eps__gethousekeepingeng__from_t tlm_eng;

	 if(0 == isis_eps__gethousekeepingeng__tm(EPS_I2C_BUS_INDEX,&tlm_eng)){
		 c_fileWrite(FILENAME_EPS_ENG_TLM, &tlm_eng);
	 }
}

void TelemetrySaveTRXVU()
{
	int err = 0;
	ISIStrxvuTxTelemetry tx_tlm;
	err = IsisTrxvu_tcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX,
			&tx_tlm);
	if (err == 0)
	{
		c_fileWrite(FILENAME_TX_REVC, &tx_tlm);
	}
	ISIStrxvuRxTelemetry rx_tlm;
	err = IsisTrxvu_rcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX,
			&rx_tlm);
	if (err == 0)
	{
		c_fileWrite(FILENAME_RX_REVC, &rx_tlm);
	}
}

void TelemetrySaveANT()
{
	int err = 0;
	ISISantsTelemetry ant_tlmA, ant_tlmB;
	err = IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA,
			&ant_tlmA);
	if (err == 0)
	{
		c_fileWrite(FILENAME_ANTENNA_TLM, &ant_tlmA);
	}
	err += IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB,
			&ant_tlmB);
	if (err == 0)
	{
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

	// get the free space of the SD card
	err = f_getfreespace(drivenum, &space);
	if (err == F_NO_ERROR){
		wod->free_memory = space.free;
		wod->corrupt_bytes = space.bad;
	}
	//TODO: Add log error

	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);
	wod->sat_time = current_time;


	isis_eps__gethousekeepingeng__from_t hk_tlm;
	isis_eps__gethousekeepingengincdb__from_t hk_tlm_cdb;

	err =  isis_eps__gethousekeepingengincdb__tm(EPS_I2C_BUS_INDEX, &hk_tlm_cdb);
	err += isis_eps__gethousekeepingeng__tm(EPS_I2C_BUS_INDEX, &hk_tlm);

	if(err == 0){

		wod->vbat = hk_tlm_cdb.fields.dist_input.fields.volt;
		wod->current_3V3 = hk_tlm.fields.vip_obc05.fields.current;
		wod->current_5V = hk_tlm.fields.vip_obc01.fields.current;
		wod->volt_3V3 = hk_tlm.fields.vip_obc05.fields.volt;
		wod->volt_5V = hk_tlm.fields.vip_obc01.fields.volt;
		wod->mcu_temp = hk_tlm.fields.temp;
		wod->bat_temp = hk_tlm.fields.temp3;
		wod->mcu_temp = hk_tlm.fields.temp;
		// TODO set all solar panels temp values
		wod->solar_panels[0]=0;
		wod->solar_panels[1]=0;
		wod->solar_panels[2]=0;
		wod->solar_panels[3]=0;
		wod->solar_panels[4]=0;
		wod->solar_panels[5]=0;
		wod->charging_power = hk_tlm_cdb.fields.batt_input.fields.volt;
		wod->consumed_power = hk_tlm_cdb.fields.dist_input.fields.power;
	}
	// TODO: Add log error
	FRAM_read((unsigned char*)&wod->number_of_resets,
	NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
}

