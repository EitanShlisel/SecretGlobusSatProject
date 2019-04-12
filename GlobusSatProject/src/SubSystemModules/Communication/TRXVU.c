#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <satellite-subsystems/IsisTRXVU.h>
#include <hal/errors.h>

#include "TRXVU.h"
#include "SPL.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"

#include "SubSystemModules/Housekepping/TelemetryCollector.h"

#define SIZE_RXFRAME	200
#define SIZE_TXFRAME	235

#define MAX_SEMAPHORE_WAIT (SECONDS_TO_TICKS(10))	// wait for 10 seconds to aquire semaphore
xSemaphoreHandle xIsTransmitting;


Boolean CheckTransmitionAllowed()
{
	unsigned char mute_flag = 0 , low_voltage_flag = 0;
	FRAM_read(&mute_flag,MUTE_FLAG_ADDR,MUTE_FLAG_SIZE);
	FRAM_read(&low_voltage_flag,TRXVU_LOW_VOLTAGE_FLAG_ADDR,TRXVU_LOW_VOLTAGE_FLAG_SIZE);

	if(mute_flag || low_voltage_flag)
	{
		return FALSE;
	}

	return TRUE;
}

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

	retValInt = IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUBuffers,
			&myTRXVUBitrates, 1);
	if (retValInt != 0)
	{
		return retValInt;
	}
	vSemaphoreCreateBinary(xIsTransmitting);
	vTaskDelay(100);
	return 0;
}

int TRX_Logic()
{
	int frame_count = GetNumberOfFramesInBuffer();
	int err = 0;
	sat_packet_t cmd = { 0 };
	if (frame_count > 0)
	{
		err = GetOnlineCommand(&cmd);
	} else if (GetDelayedCommandBufferCount() > 0)
	{
		err = GetDelayedCommand(&cmd);
	}
	if (command_found != err)
	{
		return -1;
	}
	err = ActUponCommand(&cmd);

	BeaconLogic();

	if (0 != err)
		return -1;
	return 0;
}

int TransmitSplPacket(sat_packet_t *packet)
{
	//check if MUTE or LOW_VOLTAGE
	if(!CheckTransmitionAllowed())
	{
		return -1;
	}
	if (NULL == packet)
	{
		return E_NOT_INITIALIZED;
	}
	int err = 0;
	unsigned char avalFrames;
	unsigned int data_length = packet->length + sizeof(packet->cmd_subtype)
			+ sizeof(packet->cmd_type);

	//check if TX is transmitting
	if (xSemaphoreTake(xIsTransmitting, MAX_SEMAPHORE_WAIT))
	{
		err = IsisTrxvu_tcSendAX25DefClSign(0, (unsigned char*) packet, data_length,
				&avalFrames);//TODO: think if we need 'avalFrames'

		xSemaphoreGive(xIsTransmitting);
		return err;
	}
	else{
		return E_GET_SEMAPHORE_FAILED;
	}
}

int DumpTelemetry(int type, time_unix t_start, time_unix t_end)
{
	if (xSemaphoreTake(xIsTransmitting, MAX_SEMAPHORE_WAIT))
	{
		//TODO: finish DumpTelemetry
		xSemaphoreGive(xIsTransmitting);
	}
	return 0;
}

void BeaconLogic()
{
	WOD_Telemetry_t wod = { 0 };

	int err = 0;
	GetCurrentWODTelemetry(&wod);
	if (0 != err)
	{
		return;
	}
	sat_packet_t cmd = { 0 };

	parseCommand(&cmd, (char*) &wod, sizeof(wod));
	cmd.cmd_type = trxvu_cmd_type;
	cmd.cmd_subtype = BEACON_SUBTYPE;
	TransmitSplPacket(&cmd);
}

int muteTRXVU(time_unix duration, char mute_enable)
{
	//TODO: finish muteTRXVU
	return 0;
}

int GetNumberOfFramesInBuffer()
{
	unsigned short frameCounter = 0;
	int err = IsisTrxvu_rcGetFrameCount(0, &frameCounter);
	if (0 != err) {
		return -1;
	}
	return frameCounter;
}

