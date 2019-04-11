
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <satellite-subsystems/IsisTRXVU.h>


#include "TRXVU.h"
#include "ActUponCommand.h"

#define SIZE_RXFRAME	200
#define SIZE_TXFRAME	235

int init_trxvu()
{
	int retValInt = 0;

	// Definition of I2C and TRXUV
	ISIStrxvuI2CAddress myTRXVUAddress;
	ISIStrxvuFrameLengths myTRXVUBuffers;
	ISIStrxvuBitrate myTRXVUBitrates;

	//I2C addresses defined
	myTRXVUAddress.addressVu_rc = I2C_TRXVU_RC_ADDR;
	myTRXVUAddress.addressVu_tc = I2C_TRXVU_TC_ADDR;

	//Buffer definition
	myTRXVUBuffers.maxAX25frameLengthTX = SIZE_TXFRAME;
	myTRXVUBuffers.maxAX25frameLengthRX = SIZE_RXFRAME;

	//Bitrate definition
	myTRXVUBitrates = trxvu_bitrate_9600;

	retValInt = IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUBuffers, &myTRXVUBitrates, 1);
	if(retValInt != 0)
	{
		return retValInt;
	}

	if(retValInt != 0)
	{
		return retValInt;
	}
	vTaskDelay(1000);

	return 0;
}

int TRX_Logic()
{
	int frame_count = GetNumberOfFramesInBuffer();
	int err = 0;
	sat_command_t cmd = {0};
	if(frame_count > 0)
	{
		err = GetOnlineCommand(&cmd);
	}
	else if(GetDelayedCommandBufferCount() > 0)
	{
		err = GetDelayedCommand(&cmd);
	}
	if(command_found != err )
	{
		return -1;
	}
	err = ActUponCommand(&cmd);

	if(0 != err)
		return -1;
	return 0;
}

int DumpTelemetry(int type,time_unix t_start, time_unix t_end)
{
	//TODO: finish DumpTelemetry
	return 0;
}

void BeaconLogic()
{
	//TODO: finish BeaconLogic
}

int executeCommands()
{
	//TODO: finish executeCommands
	return 0;
}

int muteTRXVU(time_unix duration, char mute_enable)
{
	//TODO: finish muteTRXVU
	return 0;
}

int GetNumberOfFramesInBuffer()
{
	unsigned short frameCounter = 0;
	int err = IsisTrxvu_rcGetFrameCount(0,&frameCounter);
	if(0 != err)
	{
		return -1;
	}
	return frameCounter;
}

