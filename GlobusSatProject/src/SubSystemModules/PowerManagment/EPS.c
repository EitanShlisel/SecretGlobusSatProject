#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/errors.h>

#include <string.h>

#include "EPS.h"
#include <satellite-subsystems/isis_eps_driver.h>

// y[i] = a * x[i] +(1-a) * y[i-1]
voltage_t prev_filtered_voltage = 0;		// y[i-1]

float alpha = DEFAULT_ALPHA_VALUE;			//<! smoothing constant
EpsThreshVolt_t eps_threshold_voltages = {.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};	// saves the current EPS logic threshold voltages

int EPS_Init()
{
	int rv = 0;
	ISIS_EPS_t subsystem; // One instance to be initialised.
	subsystem.i2cAddr = EPS_I2C_ADDR;
	rv = ISIS_EPS_Init(&subsystem, 1);
	if(isis_eps__error__reinit != rv || isis_eps__error__none != rv){
		return  -1;
	}

	rv = IsisSolarPanelv2_initialize(slave0_spi);
	if (rv != 0) {
		return -2;
	}
	IsisSolarPanelv2_sleep();

	rv = GetThresholdVoltages(&eps_threshold_voltages);
	if (0 != rv) {
		return -3;
	}

	rv= GetAlpha(&alpha);
	if (0 != rv) {
		alpha = DEFAULT_ALPHA_VALUE;
		return -4;
	}
	prev_filtered_voltage = 0;	//y[i-1]
	GetBatteryVoltage(&prev_filtered_voltage);

	EPS_Conditioning();

	return 0;
}
#define GetFilterdVoltage(curr_voltage) (voltage_t) (alpha * curr_voltage + (1 - alpha) * prev_filtered_voltage)

int EPS_Conditioning()
{
	voltage_t curr_voltage = 0;				// x[i]
	GetBatteryVoltage(&curr_voltage);

	voltage_t filtered_voltage = 0;					// the currently filtered voltage; y[i]

	filtered_voltage = GetFilterdVoltage(curr_voltage);
	if(	eps_threshold_voltages.fields.Vup_full <= filtered_voltage){
		EnterFullMode();
	}
	if(	eps_threshold_voltages.fields.Vup_cruise <= filtered_voltage &&
		filtered_voltage <= eps_threshold_voltages.fields.Vdown_full){
		EnterCruiseMode();
	}
	if(	eps_threshold_voltages.fields.Vup_safe <= filtered_voltage &&
		filtered_voltage <= eps_threshold_voltages.fields.Vdown_cruise){
		EnterSafeMode();
	}
	if(	filtered_voltage <= eps_threshold_voltages.fields.Vdown_safe){
		EnterCriticalMode();
	}

	prev_filtered_voltage = filtered_voltage;
	return 0;
}

int GetBatteryVoltage(voltage_t *vbatt)
{
	int err = 0;
	return err;
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

int UpdateThresholdVoltages(EpsThreshVolt_t *thresh_volts)
{
	if (NULL == thresh_volts) {
		return E_INPUT_POINTER_NULL;
	}

	Boolean valid_dependancies = (thresh_volts->fields.Vup_safe 	< thresh_volts->fields.Vup_cruise
	                           && thresh_volts->fields.Vup_cruise	< thresh_volts->fields.Vup_full);

	Boolean valid_regions = (thresh_volts->fields.Vdown_full 	< thresh_volts->fields.Vup_full)
						&&  (thresh_volts->fields.Vdown_cruise	< thresh_volts->fields.Vup_cruise)
						&&  (thresh_volts->fields.Vdown_safe	< thresh_volts->fields.Vup_safe);

	if (!(valid_dependancies && valid_regions)) {
		return -2;
	}
	int err = FRAM_write((unsigned char*) thresh_volts,
			EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
	if (0 != err) {
		return err;
	}
	memcpy(eps_threshold_voltages.raw, thresh_volts, EPS_THRESH_VOLTAGES_SIZE);
	return E_NO_SS_ERR;
}

int GetThresholdVoltages(EpsThreshVolt_t *thresh_volts)
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
	EpsThreshVolt_t def_thresh =
	{.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
	err = UpdateThresholdVoltages(&def_thresh);
	return err;
}

