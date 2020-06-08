
#include "EPSOperationModes.h"
#include "GlobalStandards.h"

#include <satellite-subsystems/isis_eps_driver.h>


//TODO: update functions to only the relevant channels
channel_t g_system_state;
EpsState_t state;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

int EnterFullMode(){
	int err = 0;
	if(state == FullMode)
		return 0;
	err = SetEPS_ChannelsOn((channel_t) CHANNELS_OFF);
	err = SetEPS_ChannelsOff((channel_t) ~CHANNELS_OFF);
	state = FullMode;
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCruiseMode(){
	int err = 0;
	if(state == CruiseMode)
		return 0;
	err = SetEPS_ChannelsOn((channel_t) CHANNELS_OFF);
	err = SetEPS_ChannelsOff((channel_t) ~CHANNELS_OFF);
	state = CruiseMode;
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterSafeMode(){
	int err = 0;
	if(state == SafeMode)
		return 0;
	err = SetEPS_ChannelsOn((channel_t) CHANNELS_OFF);
	err = SetEPS_ChannelsOff((channel_t) ~CHANNELS_OFF);
	state = SafeMode;
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCriticalMode(){
	int err = 0;
	if(state == CriticalMode)
		return 0;
	err = SetEPS_ChannelsOn((channel_t) CHANNELS_OFF);
	err = SetEPS_ChannelsOff((channel_t) ~CHANNELS_OFF);
	state = CriticalMode;

	EpsSetLowVoltageFlag(TRUE);
	return err;
}

int SetEPS_ChannelsOn(channel_t channel)
{
	(void)channel;
	isis_eps__outputbuschannelon__to_t params;
	isis_eps__outputbuschannelon__from_t response;

	params.fields.obc_idx = (isis_eps__imeps_channel_t)channel;

	int error = isis_eps__outputbuschannelon__tmtc(EPS_I2C_BUS_INDEX,&params,&response);

	return error;
}

int SetEPS_ChannelsOff(channel_t channel){
	int error = 0;
	isis_eps__outputbuschanneloff__to_t params;
	isis_eps__outputbuschanneloff__from_t response;

	params.fields.obc_idx = (isis_eps__imeps_channel_t)channel;

	error = isis_eps__outputbuschanneloff__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return error;
}

EpsState_t GetSystemState()
{
	return state;
}

channel_t GetSystemChannelState()
{
	return g_system_state;
}

Boolean EpsGetLowVoltageFlag()
{
	return g_low_volt_flag;
}

void EpsSetLowVoltageFlag(Boolean low_volt_flag)
{
	g_low_volt_flag = low_volt_flag;
}

