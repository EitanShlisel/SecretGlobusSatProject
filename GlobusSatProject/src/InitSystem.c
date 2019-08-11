#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>
#include <at91/utility/exithandler.h>
#include <string.h>
#include "GlobalStandards.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "InitSystem.h"
#include "TLM_management.h"

#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif
#ifdef ISISEPS
	#include <satellite-subsystems/IsisEPS.h>
#endif

#define I2c_SPEED_Hz 100000
#define I2c_Timeout 10


Boolean isFirstActivation()
{
	Boolean flag = FALSE;
	FRAM_read((unsigned char*) &flag, FIRST_ACTIVATION_FLAG_ADDR,
	FIRST_ACTIVATION_FLAG_SIZE);
	return flag;
}

void firstActivationProcedure()
{
#ifdef ISISEPS
	ieps_statcmd_t eps_cmd;
#endif

	time_unix current_time = 0;
	FRAM_read((unsigned char*)&current_time, MOST_UPDATED_SAT_TIME_ADDR,
	MOST_UPDATED_SAT_TIME_SIZE);

	time_unix deploy_time = 0;
	FRAM_read((unsigned char*) deploy_time, DEPLOYMENT_TIME_ADDR,
	DEPLOYMENT_TIME_SIZE);

	while (current_time - deploy_time > MINUTES_TO_SECONDS(30)) {
		vTaskDelay(SECONDS_TO_TICKS(10));

		SaveSatTimeInFRAM(MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE);

		TelemetryCollectorLogic();


		//TODO: add more to this...
#ifdef ISISEPS
		IsisEPS_resetWDT(EPS_I2C_BUS_INDEX, &eps_cmd);
#endif
#ifdef GOMEPS
	GomEpsResetWDT(EPS_I2C_BUS_INDEX);

#endif
	}
	//TODO: finish 'firstActivationProcedure'
}

void WriteDefaultValuesToFRAM()
{
	//TODO: write to FRAM all default values (like threshold voltages...)

	time_unix default_no_comm_thresh = DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write((unsigned char*)&default_no_comm_thresh, NO_COMM_WDT_KICK_TIME_ADDR,
	NO_COMM_WDT_KICK_TIME_SIZE);

	voltage_t def_thresg_volt[NUMBER_OF_THRESHOLD_VOLTAGES] =
			DEFAULT_EPS_THRESHOLD_VOLTAGES;
	FRAM_write((unsigned char*) def_thresg_volt, EPS_THRESH_VOLTAGES_ADDR,
	EPS_THRESH_VOLTAGES_SIZE);

	float def_alpha = DEFAULT_ALPHA_VALUE;
	FRAM_write((unsigned char*)&def_alpha, EPS_ALPHA_FILTER_VALUE_ADDR,
			EPS_ALPHA_FILTER_VALUE_SIZE);

	time_unix tlm_save_period = 0;
	tlm_save_period = DEFAULT_EPS_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&tlm_save_period,EPS_SAVE_TLM_PERIOD_ADDR,sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_TRXVU_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&tlm_save_period,TRXVU_SAVE_TLM_PERIOD_ADDR,sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_ANT_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&tlm_save_period,ANT_SAVE_TLM_PERIOD_ADDR,sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_SOLAR_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&tlm_save_period,SOLAR_SAVE_TLM_PERIOD_ADDR,sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&tlm_save_period,WOD_SAVE_TLM_PERIOD_ADDR,sizeof(tlm_save_period));

	time_unix beacon_interval = 0;
	beacon_interval = DEFAULT_BEACON_INTERVAL_TIME;
	FRAM_write((unsigned char*)&beacon_interval, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE);

}

int StartFRAM()
{
	int error = FRAM_start();
	return error;
}

int StartI2C()
{
	int error = I2C_start(I2c_SPEED_Hz, I2c_Timeout);
	return error;
}

int StartSPI()
{
	int error = SPI_start(bus1_spi, slave1_spi);
	return error;
}

int StartTIME()
{
	int error = 0;
	Time initial_time_jan2000 = UNIX_DATE_JAN_D1_Y2000;
	error = Time_start(&initial_time_jan2000, 0);
	if (0 != error) {
		return error;
	}
	time_unix time_before_wakeup = 0;
	if (!isFirstActivation()) {
		FRAM_read((unsigned char*) &time_before_wakeup,
		MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);

		Time_setUnixEpoch(time_before_wakeup);
	}
	else{
		FRAM_write((unsigned char*) &time_before_wakeup, DEPLOYMENT_TIME_ADDR,
						DEPLOYMENT_TIME_SIZE);
	}
	return 0;
}

int DeploySystem()
{
	Boolean first_activation = isFirstActivation();

	if (first_activation) {

		time_unix deploy_time = 0;
		Time_getUnixEpoch(&deploy_time);
		FRAM_write((unsigned char*) deploy_time, DEPLOYMENT_TIME_ADDR,
		DEPLOYMENT_TIME_SIZE);

		firstActivationProcedure();

		first_activation = FALSE; //TODO: set 'first_activation' to TRUE before launch
		FRAM_write((unsigned char*) &first_activation,
		FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);

	}
	return 0;
}

#define PRINT_IF_ERR(method) if(0 != err)printf("error in '" #method  "' err = %d\n",err);
int InitSubsystems()
{
	//TODO: check for return value errors
	int err = 0;

	err = StartI2C();
	PRINT_IF_ERR(StartI2C)

	err = StartSPI();
	PRINT_IF_ERR(StartSPI)

	err = StartFRAM();
	PRINT_IF_ERR(StartFRAM)

	err = StartTIME();
	PRINT_IF_ERR(StartTIME)

	err = InitializeFS(isFirstActivation());
	PRINT_IF_ERR(InitializeFS)

	err = EPS_Init();
	PRINT_IF_ERR(EPS_Init)

	err = InitTrxvu();
	PRINT_IF_ERR(InitTrxvu)

	err = DeploySystem();
	PRINT_IF_ERR(DeploySystem)
	return 0;
}

