#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>

#include <stdlib.h>
#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#ifdef TESTING_TRXVU_FRAME_LENGTH
#include <hal/Utility/util.h>
#endif
#define SIZE_RXFRAME	200
#define SIZE_TXFRAME	235


Boolean 		g_mute_flag = MUTE_OFF;				// mute flag - is the mute enabled
time_unix 		g_mute_end_time = 0;				// time at which the mute will end
time_unix 		g_prev_beacon_time = 0;				// the time at which the previous beacon occured
time_unix 		g_beacon_interval_time = 0;			// seconds between each beacon
unsigned char	g_current_beacon_period = 0;		// marks the current beacon cycle(how many were transmitted before change in baud)
unsigned char 	g_beacon_change_baud_period = 0;	// every 'g_beacon_change_baud_period' beacon will be in 1200Bps and not 9600Bps

xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xTaskHandle xDumpHandle = NULL;			 //task handle for dump task
xSemaphoreHandle xIsTransmitting = NULL; // mutex on transmission.

void InitSemaphores()
{
	if(NULL == xIsTransmitting)
		vSemaphoreCreateBinary(xIsTransmitting);
	if(NULL == xDumpLock)
		vSemaphoreCreateBinary(xDumpLock);
	if(NULL == xDumpQueue)
		xDumpQueue = xQueueCreate(1, sizeof(Boolean));
}

int InitTrxvu() {

	ISIStrxvuI2CAddress myTRXVUAddress;
	ISIStrxvuFrameLengths myTRXVUBuffers;


	int retValInt = 0;

	//Buffer definition
#ifdef TESTING_TRXVU_FRAME_LENGTH
	myTRXVUBuffers.maxAX25frameLengthTX = TESTING_TRXVU_FRAME_LENGTH;//SIZE_TXFRAME;
	myTRXVUBuffers.maxAX25frameLengthRX = SIZE_RXFRAME;
#else
	myTRXVUBuffers.maxAX25frameLengthTX = SIZE_TXFRAME;//SIZE_TXFRAME;
	myTRXVUBuffers.maxAX25frameLengthRX = SIZE_RXFRAME;
#endif
	//I2C addresses defined
	myTRXVUAddress.addressVu_rc = I2C_TRXVU_RC_ADDR;
	myTRXVUAddress.addressVu_tc = I2C_TRXVU_TC_ADDR;


	//Bitrate definition
	ISIStrxvuBitrate myTRXVUBitrates;
	myTRXVUBitrates = trxvu_bitrate_9600;
	retValInt = IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUBuffers,
			&myTRXVUBitrates, 1);
	if (retValInt != 0) {
		return retValInt;
	}
	vTaskDelay(100);

	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,myTRXVUBitrates);
	vTaskDelay(100);

	InitSemaphores();
	//TODO: should be somehere else
	//TODO: update global parameter when updating the FRAM
	FRAM_read(&g_beacon_change_baud_period, BEACON_BITRATE_CYCLE_ADDR,
	BEACON_BITRATE_CYCLE_SIZE);

	FRAM_read((unsigned char*)&g_beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);

	return 0;
}

int TRX_Logic() {
	int err = 0;
	int frame_count = GetNumberOfFramesInBuffer();
	sat_packet_t cmd = { 0 };

	if (frame_count > 0) {
		err = GetOnlineCommand(&cmd);
		ResetGroundCommWDT();
		SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0);

	} else if (GetDelayedCommandBufferCount() > 0) {
		err = GetDelayedCommand(&cmd);
	}
	if (command_found == err) {
		err = ActUponCommand(&cmd);
		//TODO: send message to ground when a delayd command was not executed-> add to log
	}
	//TODO: add delayed command stream to the logic
	BeaconLogic();

	if (command_found != err)
		return err;

	return command_succsess;
}

int GetNumberOfFramesInBuffer() {
	unsigned short frameCounter = 0;
	int err = IsisTrxvu_rcGetFrameCount(0, &frameCounter);
	if (0 != err) {
		return -1;
	}
	return frameCounter;
}

Boolean CheckTransmitionAllowed() {
	Boolean low_voltage_flag = TRUE;

	low_voltage_flag = EpsGetLowVoltageFlag();

	if (g_mute_flag == MUTE_OFF && low_voltage_flag == FALSE) {
		return TRUE;
	}

	return FALSE;
}


void FinishDump(dump_arguments_t *task_args,unsigned char *buffer, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) {

	SendAckPacket(acktype, task_args->cmd, err, size);
	if (NULL != task_args) {
		free(task_args);
	}
	if (NULL != xDumpLock) {
		xSemaphoreGive(xDumpLock);
	}
	if (xDumpHandle != NULL) {
		vTaskDelete(xDumpHandle);
	}
	if(NULL != buffer){
		free(buffer);
	}
}

void AbortDump()
{
	FinishDump(NULL,NULL,ACK_DUMP_ABORT,NULL,0);
}

void SendDumpAbortRequest() {
	if (eTaskGetState(xDumpHandle) == eDeleted) {
		return;
	}
	Boolean queue_msg = TRUE;
	int err = xQueueSend(xDumpQueue, &queue_msg, SECONDS_TO_TICKS(1));
	if (0 != err) {
		if (NULL != xDumpLock) {
			xSemaphoreGive(xDumpLock);
		}
		if (xDumpHandle != NULL) {
			vTaskDelete(xDumpHandle);
		}
	}
}

Boolean CheckDumpAbort() {
	portBASE_TYPE err;
	Boolean queue_msg;
	err = xQueueReceive(xDumpQueue, &queue_msg, SECONDS_TO_TICKS(1));
	if (err == pdPASS) {
		return queue_msg;
	}
	return FALSE;
}

void DumpTask(void *args) {
	if (NULL == args) {
		FinishDump(NULL, NULL, ACK_DUMP_ABORT, NULL, 0);
		return;
	}
	dump_arguments_t *task_args = (dump_arguments_t *) args;
	sat_packet_t dump_tlm = { 0 };

	int err = 0;
	int availFrames = 0;
	unsigned int num_of_elements = 0;
	unsigned int size_of_element = 0;

	unsigned char *buffer = NULL;
	char filename[MAX_F_FILE_NAME_SIZE] = { 0 };

	err = GetTelemetryFilenameByType((tlm_type_t) task_args->dump_type,
			filename);
	if (0 != err) {
		FinishDump(task_args, buffer, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
		return;
	}

	SendAckPacket(ACK_DUMP_START, task_args->cmd,
			(unsigned char*) &num_of_elements, sizeof(num_of_elements));

	for (unsigned int i = 0; i < num_of_elements; i++) {

		if (CheckDumpAbort() || CheckTransmitionAllowed()) {
			FinishDump(task_args, buffer, ACK_DUMP_ABORT, NULL, 0);
		}
		if (0 == availFrames)
			vTaskDelay(10);

		AssmbleCommand((unsigned char*)buffer, size_of_element,
				(char) DUMP_SUBTYPE, (char) (task_args->dump_type),
				task_args->cmd->ID, &dump_tlm);

		//TODO: finish dump

		TransmitSplPacket(&dump_tlm, &availFrames);
	}
	FinishDump(task_args, buffer, ACK_DUMP_FINISHED, NULL, 0);
}

int DumpTelemetry(sat_packet_t *cmd) {
	if (NULL == cmd) {
		return -1;
	}

	dump_arguments_t *dmp_pckt = malloc(sizeof(*dmp_pckt));
	unsigned int offset = 0;

	dmp_pckt->cmd = cmd;

	memcpy(&dmp_pckt->dump_type, cmd->data, sizeof(dmp_pckt->dump_type));
	offset += sizeof(dmp_pckt->dump_type);

	memcpy(&dmp_pckt->t_start, cmd->data + offset, sizeof(dmp_pckt->t_start));
	offset += sizeof(dmp_pckt->t_start);

	memcpy(&dmp_pckt->t_end, cmd->data + offset, sizeof(dmp_pckt->t_end));

	if (xSemaphoreTake(xDumpLock,SECONDS_TO_TICKS(1)) != pdTRUE) {
		return E_GET_SEMAPHORE_FAILED;
	}
	xTaskCreate(DumpTask, (const signed char* const )"DumpTask", 2000,
			(void* )dmp_pckt, configMAX_PRIORITIES - 2, xDumpHandle);

	return 0;
}

//Sets the bitrate to 1200 every third beacon and to 9600 otherwise
int BeaconSetBitrate() {

	int err = 0;
	if (g_current_beacon_period % g_beacon_change_baud_period == 0) {
		err = IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_1200);
		g_current_beacon_period = 0;
	} else {
		err = IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
	}
	g_current_beacon_period++;
	return err;
}

void BeaconLogic() {
	int err = 0;

	if (!CheckExecutionTime(g_prev_beacon_time, g_beacon_interval_time)) {
		return;
	}

	WOD_Telemetry_t wod = { 0 };
	GetCurrentWODTelemetry(&wod);

	sat_packet_t cmd = { 0 };
	err = AssmbleCommand((unsigned char*) &wod, sizeof(wod), trxvu_cmd_type,
			BEACON_SUBTYPE, 0xFFFFFFFF, &cmd);
	if (0 != err) {
		return;
	}

	Time_getUnixEpoch(&g_prev_beacon_time);

	BeaconSetBitrate();

	TransmitSplPacket(&cmd, NULL);
	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_bitrate_9600);

#ifdef TESTING
	printf("beacon was transmitted, timestamp: %lu\n",g_prev_beacon_time);
#endif

}

int muteTRXVU(time_unix duration) {
	if (duration > MAX_MUTE_TIME) {
		return -2;
	}
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);

	g_mute_end_time = curr_tick_time + duration;
	g_mute_flag = MUTE_ON;

	return 0;
}

void UnMuteTRXVU() {
	g_mute_end_time = 0;
	g_mute_flag = MUTE_OFF;
}

Boolean GetMuteFlag() {
	return g_mute_flag;
}

Boolean CheckForMuteEnd() {
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);
	return (curr_tick_time > g_mute_end_time);
}

int GetTrxvuBitrate(ISIStrxvuBitrateStatus *bitrate) {
	if (NULL == bitrate) {
		return E_NOT_INITIALIZED;
	}
	ISIStrxvuTransmitterState trxvu_state;
	int err = IsisTrxvu_tcGetState(ISIS_TRXVU_I2C_BUS_INDEX, &trxvu_state);

	if (E_NO_SS_ERR == err) {
		*bitrate = trxvu_state.fields.transmitter_bitrate;
	}
	return err;

}

int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data,
		unsigned int length) {
	int err = 0;
	sat_packet_t packet = { 0 };
	if (NULL != cmd) {
		err = AssmbleCommand(data, length, cmd->cmd_type, cmd->cmd_subtype,
				cmd->ID, &packet);
	} else {
		err = AssmbleCommand(data, length, 0xFF, 0xFF, 0xFFFFFFFF, &packet); //TODO: figure out what should be the 'FF'
	}
	if (err != 0) {
		return err;
	}
	err = TransmitSplPacket(&packet, NULL);
	return err;
}

int TransmitSplPacket(sat_packet_t *packet, int *avalFrames) {
	if (!CheckTransmitionAllowed()) {
		return -1;
	}

	if (NULL == packet) {
		return E_NOT_INITIALIZED;
	}

	int err = 0;
	unsigned int data_length = packet->length + sizeof(packet->length)
			+ sizeof(packet->cmd_subtype) + sizeof(packet->cmd_type)
			+ sizeof(packet->ID);

	if (xSemaphoreTake(xIsTransmitting,SECONDS_TO_TICKS(1)) != pdTRUE) {
		return E_GET_SEMAPHORE_FAILED;
	}
	err = IsisTrxvu_tcSendAX25DefClSign(ISIS_TRXVU_I2C_BUS_INDEX,
			(unsigned char*) packet, data_length, (unsigned char*) &avalFrames);

	xSemaphoreGive(xIsTransmitting);

	return err;
}

int UpdateBeaconBaudCycle(unsigned char cycle)
{
	if(cycle < DEFALUT_BEACON_BITRATE_CYCLE)
	{
		return E_PARAM_OUTOFBOUNDS;
	}
	int err = FRAM_write((unsigned char*) &cycle, BEACON_BITRATE_CYCLE_ADDR,
			BEACON_BITRATE_CYCLE_SIZE);
	if (0 != err) {
		return err;
	}
	g_beacon_change_baud_period = cycle;
	return E_NO_SS_ERR;
}

int UpdateBeaconInterval(time_unix intrvl) {
	if (intrvl > MAX_BEACON_INTERVAL || intrvl < MIN_BEACON_INTERVAL) {
		return E_PARAM_OUTOFBOUNDS;
	}
	int err = FRAM_write((unsigned char*) &intrvl, BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);
	if (0 != err) {
		return err;
	}
	g_beacon_interval_time = intrvl;
	return E_NO_SS_ERR;
}
