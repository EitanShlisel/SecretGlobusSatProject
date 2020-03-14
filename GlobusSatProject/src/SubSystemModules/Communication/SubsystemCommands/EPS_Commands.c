#include "GlobalStandards.h"

#include <satellite-subsystems/isis_eps_driver.h>

#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <stdlib.h>
#include <string.h>

#include  "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/PowerManagment/EPSOperationModes.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "EPS_Commands.h"
#include <hal/errors.h>
int CMD_UpdateThresholdVoltages(sat_packet_t *cmd)
{
	int err = 0;
	if (NULL == cmd || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	EpsThreshVolt_t thresh_voltages;;
	memcpy(thresh_voltages.raw, cmd->data,
			sizeof(thresh_voltages));
	err = UpdateThresholdVoltages(&thresh_voltages);
	return err;
}

int CMD_GetThresholdVoltages(sat_packet_t *cmd)
{
	int err = 0;
	EpsThreshVolt_t thresh_voltages;
	err = GetThresholdVoltages(&thresh_voltages);
	if (err == 0)
	{
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) thresh_voltages.raw,
				sizeof(thresh_voltages));
	}
	return err;
}

int CMD_UpdateSmoothingFactor(sat_packet_t *cmd)
{
	int err = 0;
	if (NULL == cmd)
	{
		return E_INPUT_POINTER_NULL;
	}
	float alpha;
	memcpy((unsigned char*)&alpha, cmd->data, sizeof(alpha));
	err = UpdateAlpha(alpha);
	return err;
}

int CMD_RestoreDefaultAlpha(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	err = RestoreDefaultAlpha();
	return err;
}

int CMD_RestoreDefaultThresholdVoltages(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	err = RestoreDefaultThresholdVoltages();
	return err;
}

int CMD_GetSmoothingFactor(sat_packet_t *cmd)
{
	int err = 0;
	float alpha;
	err = GetAlpha(&alpha);

	if (err == 0){
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &alpha,
				sizeof(alpha) * NUMBER_OF_THRESHOLD_VOLTAGES);
	}
	return err;
}

int CMD_EnterCruiseMode(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	err = EnterCruiseMode();
	return err;
}

int CMD_EnterFullMode(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	err = EnterFullMode();
	return err;
}

int CMD_EnterCriticalMode(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	err = EnterCriticalMode();
	return err;
}

int CMD_EnterSafeMode(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	err = EnterSafeMode();
	return err;
}

int CMD_GetCurrentMode(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	channel_t state = GetSystemChannelState();
	TransmitDataAsSPL_Packet(cmd, &state, sizeof(state));
	return err;
}

int CMD_EPS_NOP(sat_packet_t *cmd)
{
	if (NULL == cmd->data || NULL == cmd){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__nop__from_t response;
	err = isis_eps__nop__tm(EPS_I2C_BUS_INDEX,&response);
	return err;
}

int CMD_EPS_ResetWDT(sat_packet_t *cmd)
{
	if (NULL == cmd->data || NULL == cmd){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__watchdog__from_t response;
	err = isis_eps__watchdog__tm(EPS_I2C_BUS_INDEX,&response);
	return err;
}

int CMD_EPS_SetChannels(sat_packet_t *cmd)
{
	if (NULL == cmd->data || NULL == cmd){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__outputbusgroupstate__to_t params;
	isis_eps__outputbusgroupstate__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	err = isis_eps__outputbusgroupstate__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return err;
}

int CMD_SetChannels_On(sat_packet_t *cmd)
{
	if (NULL == cmd->data || NULL == cmd){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__outputbuschannelon__to_t params;
	isis_eps__outputbuschannelon__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	isis_eps__outputbuschannelon__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return err;
}

int CMD_SetChannels_Off(sat_packet_t *cmd)
{
	if (NULL == cmd->data || NULL == cmd){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__outputbuschanneloff__to_t params;
	isis_eps__outputbuschanneloff__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	isis_eps__outputbuschanneloff__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return err;
}

int CMD_SetGroup_On(sat_packet_t *cmd){
	if (NULL == cmd->data || NULL == cmd){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__outputbusgroupon__to_t params;
	isis_eps__outputbusgroupon__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	isis_eps__outputbusgroupon__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return err;
}

int CMD_SetGroup_Off(sat_packet_t *cmd){
	if (NULL == cmd->data || NULL == cmd){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__outputbusgroupoff__to_t params;
	isis_eps__outputbusgroupoff__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	isis_eps__outputbusgroupoff__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return err;
}

int CMD_GetEpsParameter(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL)
	{
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	isis_eps__getparameter__to_t params;
	isis_eps__getparameter__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	err = isis_eps__getparameter__tmtc(EPS_I2C_BUS_INDEX,&params,&response);

	return err;
}

int CMD_SetEpsParemeter(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	isis_eps__setparameter__to_t params;
	isis_eps__setparameter__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	err = isis_eps__setparameter__tmtc(EPS_I2C_BUS_INDEX,&params,&response);


	return err;
}

int CMD_ResetParameter(sat_packet_t *cmd){
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__resetparameter__to_t params;
	isis_eps__resetparameter__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	err = isis_eps__resetparameter__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return err;
}

int CMD_ResetConfig(sat_packet_t *cmd){
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__resetall__to_t params;
	isis_eps__resetall__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	err = isis_eps__resetall__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return err;
}

int CMD_LoadConfig(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__loadall__to_t params;
	isis_eps__loadall__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	err = isis_eps__loadall__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return err;
}

int CMD_SaveConfig(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	isis_eps__saveall__to_t params;
	isis_eps__saveall__from_t response;
	if(NULL == memcpy(&params,cmd->data,sizeof(params))){
		return E_MEMCPY;
	}
	err = isis_eps__saveall__tmtc(EPS_I2C_BUS_INDEX,&params,&response);
	return err;
}

int CMD_SolarPanelWake(sat_packet_t *cmd)
{

	IsisSolarPanelv2_State_t state = 0;
	state = IsisSolarPanelv2_wakeup();
	TransmitDataAsSPL_Packet(cmd, &state, sizeof(state));
	return state;
}

int CMD_SolarPanelSleep(sat_packet_t *cmd)
{
	IsisSolarPanelv2_State_t state = 0;
	state = IsisSolarPanelv2_sleep();
	TransmitDataAsSPL_Packet(cmd, &state, sizeof(state));
	return state;
}

int CMD_GetSolarPanelState(sat_packet_t *cmd)
{
	IsisSolarPanelv2_State_t state = 0;
	state = IsisSolarPanelv2_getState();
	TransmitDataAsSPL_Packet(cmd, &state, sizeof(state));
	return state;
}

