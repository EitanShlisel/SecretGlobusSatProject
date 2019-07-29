
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/errors.h>

#include <string.h>

#include "EPS.h"
#ifdef ISISEPS
	#include <satellite-subsystems/IsisEPS.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

// y[i] = a * x[i] +(1-a) * y[i-1]
voltage_t prev_avg = 0;		// y[i-1]
float alpha = 0;			//<! smoothing constant

voltage_t eps_threshold_voltages[NUMBER_OF_THRESHOLD_VOLTAGES];	// saves the current EPS logic threshold voltages

int GetBatteryVoltage(voltage_t *vbatt)
{
	int err = 0;
#ifdef ISISEPS
	ieps_enghk_data_cdb_t hk_tlm;
	ieps_statcmd_t cmd;
	ieps_board_t board = ieps_board_cdb1;
	err = IsisEPS_getEngHKDataCDB(EPS_I2C_BUS_INDEX, board, &hk_tlm, &cmd);
	*vbatt = hk_tlm.fields.bat_voltage;
#endif
#ifdef GOMEPS
	gom_eps_hk_t hk_tlm;
	err = GomEpsGetHkData_general(EPS_I2C_BUS_INDEX,&hk_tlm);
	*vbatt = hk_tlm.fields.vbatt;
#endif
	return err;
}

int EPS_Init()
{
	unsigned char i2c_address = EPS_I2C_ADDR;
	int rv;
#ifdef ISISEPS
	rv = IsisEPS_initialize(&i2c_address, 1);
#endif
#ifdef GOMEPS
	rv = GomEpsInitialize(&i2c_address, 1);
#endif

	if (rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED) {
		return -1;
	}
	rv = IsisSolarPanelv2_initialize(slave0_spi);
	if (rv != 0) {
		return -2;
	}
	IsisSolarPanelv2_sleep();

	rv = FRAM_read((unsigned char*) eps_threshold_voltages,
			EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
	if (0 != rv) {
		return -1;
	}

	rv = FRAM_read((unsigned char*) &alpha,
			EPS_ALPHA_FILTER_VALUE_ADDR, sizeof(alpha));
	if (0 != rv) {
		return -1;
	}
	prev_avg = 0;	//y[i-1]
	GetBatteryVoltage(&prev_avg);

	EPS_Conditioning();

	return 0;
}

int EPS_Conditioning()
{
	voltage_t curr_voltage = 0;				// x[i]
	GetBatteryVoltage(&curr_voltage);

	voltage_t curr_avg = 0;					// the currently filtered voltage; y[i]

	curr_avg = (voltage_t) (alpha * curr_voltage + (1 - alpha) * prev_avg);

	// discharging
	if (curr_avg < prev_avg) {
		if (curr_avg < eps_threshold_voltages[INDEX_DOWN_SAFE]) {
			EnterCriticalMode();
		}
		else if (curr_avg < eps_threshold_voltages[INDEX_DOWN_CRUISE]) {
			EnterSafeMode();
		}
		else if (curr_avg < eps_threshold_voltages[INDEX_DOWN_FULL]) {
			EnterCruiseMode();
		}

	}
	// charging
	else if (curr_avg > prev_avg) {

		if (curr_avg > eps_threshold_voltages[INDEX_UP_FULL]) {
			EnterFullMode();
		}
		else if (curr_avg > eps_threshold_voltages[INDEX_UP_CRUISE]) {
			EnterCruiseMode();
		}
		else if (curr_avg > eps_threshold_voltages[INDEX_UP_SAFE]) {
			EnterSafeMode();
		}
	}
	prev_avg = curr_avg;
	return 0;
}

int UpdateAlpha(float new_alpha)
{
	if (new_alpha > 1 || new_alpha < 0) {
		return -2;
	}
	int err = FRAM_write((unsigned char*) &new_alpha,
			EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE);
	if (0 != err) {
		return err;
	}

	alpha = new_alpha;
	return 0;
}

int UpdateThresholdVoltages(voltage_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES])
{
	if (NULL == thresh_volts) {
		return E_INPUT_POINTER_NULL;
	}

	Boolean valid_dependancies = (thresh_volts[INDEX_UP_SAFE] 	< thresh_volts[INDEX_UP_CRUISE]
	                           && thresh_volts[INDEX_UP_CRUISE] < thresh_volts[INDEX_UP_FULL]);

	Boolean valid_regions = (thresh_volts[INDEX_DOWN_FULL] 		< thresh_volts[INDEX_UP_FULL])
						&&  (thresh_volts[INDEX_DOWN_CRUISE]	< thresh_volts[INDEX_UP_CRUISE])
						&&  (thresh_volts[INDEX_DOWN_SAFE]		< thresh_volts[INDEX_UP_SAFE]);

	if (!(valid_dependancies && valid_regions)) {
		return -2;
	}
	int err = FRAM_write((unsigned char*) thresh_volts,
			EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
	if (0 != err) {
		return err;
	}
	memcpy(eps_threshold_voltages, thresh_volts, EPS_THRESH_VOLTAGES_SIZE);
	return E_NO_SS_ERR;
}

int GetThresholdVoltages(voltage_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES])
{
	if (NULL == thresh_volts) {
		return E_INPUT_POINTER_NULL;
	}
	int err = FRAM_read((unsigned char*) thresh_volts, EPS_THRESH_VOLTAGES_ADDR,
			EPS_THRESH_VOLTAGES_SIZE);
	return err;
}

int GetAlpha(float *alpha)
{
	if (NULL == alpha) {
		return E_INPUT_POINTER_NULL;
	}
	int err = FRAM_read((unsigned char*) alpha, EPS_ALPHA_FILTER_VALUE_ADDR,
			EPS_ALPHA_FILTER_VALUE_SIZE);
	return err;
}

int RestoreDefaultAlpha()
{
	int err = 0;
	float def_alpha = DEFAULT_ALPHA_VALUE;
	err = UpdateAlpha(def_alpha);
	return err;
}

int RestoreDefaultThresholdVoltages()
{
	int err = 0;
	voltage_t def_thresh[NUMBER_OF_THRESHOLD_VOLTAGES] =
			DEFAULT_EPS_THRESHOLD_VOLTAGES;
	err = UpdateThresholdVoltages(def_thresh);
	return err;
}

