
#include <satellite-subsystems/IsisEPS.h>
#include <hal/errors.h>
#include "GlobalStandards.h"
#include "EPS.h"

int EPS_Init()
{
	unsigned char i2c_address = EPS_I2C_ADDR;
	int rv;
	rv = IsisEPS_initialize(&i2c_address, 1);
	if(rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED)
	{
		return -1;
	}
	return 0;
}


int SetChannels(channel_t channel)
{

	return 0;
}


int EPS_Conditioning()
{
	return 0;
}
