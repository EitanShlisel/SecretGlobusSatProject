#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

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
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Beacon.h"


#define LOGD(f_, ...) printf(("D: " f_ "\n"), ##__VA_ARGS__)
#define LOGE(f_, ...) printf(("E: " f_ "\n"), ##__VA_ARGS__)

xQueueHandle 		xDumpQueue 	= NULL;
xSemaphoreHandle 	xDumpLock 	= NULL;
xTaskHandle 		xDumpHandle = NULL;			 //task handle for dump task
time_unix			g_gs_start_time = 0;		// time since last online command received.
void InitSemaphores()
{
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
	myTRXVUBuffers.maxAX25frameLengthTX = SIZE_TXFRAME;//SIZE_TXFRAME;
	myTRXVUBuffers.maxAX25frameLengthRX = SIZE_RXFRAME;

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

	ISISantsI2Caddress myAntennaAddress;
	myAntennaAddress.addressSideA = ANTS_I2C_SIDE_A_ADDR;
	myAntennaAddress.addressSideB = ANTS_I2C_SIDE_B_ADDR;

	//Initialize the AntS system
	retValInt = IsisAntS_initialize(&myAntennaAddress, 1);
	if (retValInt != 0) {
		return retValInt;
	}
	retValInt = IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_idle_state_off);
	if (retValInt != 0) {
		return retValInt;
	}
	InitTxModule();
	InitBeaconParams();
	InitSemaphores();

	return 0;
}

CommandHandlerErr TRX_Logic() {
	int err = cmd_no_command_found;
	int frame_count = GetNumberOfFramesInBuffer();
	sat_packet_t cmd = { 0 };

	if (frame_count > 0) {
		err = GetOnlineCommand(&cmd);
		ResetGroundCommWDT();
		EnterGS_Mode();
		SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0);

	} else if (GetDelayedCommandBufferCount() > 0) {
		err = GetDelayedCommand(&cmd);
	}
	if (cmd_command_found == err) {
		err = ActUponCommand(&cmd);
		//TODO: log error
		//TODO: send message to ground when a delayed command was not executed-> add to log
	}
	BeaconLogic();

	if (cmd_command_found != err)
		return err;

	return cmd_command_succsess;
}

void FinishDump(dump_arguments_t *task_args,unsigned char *buffer, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) {

	SendAckPacket(acktype, &(task_args->cmd), err, size);
	if (NULL != task_args) {
		LOGD("free args");
		free(task_args);
	}
	if (NULL != xDumpLock) {
		LOGD("release lock");
		xSemaphoreGive(xDumpLock);
	}
	if (xDumpHandle != NULL) {
		LOGD("delete handle");
		vTaskDelete(xDumpHandle);
		xDumpHandle = NULL;
	}

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
	}
}

Boolean CheckDumpAbort() {
	portBASE_TYPE err;
	Boolean queue_msg;
	err = xQueueReceive(xDumpQueue, &queue_msg, 0);
	if (err == pdPASS) {
		return queue_msg;
	}
	return FALSE;
}

int getTelemetryMetaData(tlm_type type, char* filename, int* size_of_element) {
	int err = 0;
	err = GetTelemetryFilenameByType(type, filename);
	if (0 != err) {
		return err;
	}
	if(c_fileGetSizeOfElement(filename,size_of_element) != FS_SUCCSESS) {
		return -1;
	}
	return err;
}
char dump_arr[SIZE_DUMP_BUFFER];
void DumpTask(void *args) {
	if (NULL == args) {
		FinishDump(NULL, NULL, ACK_DUMP_ABORT, NULL, 0);
		return;
	}
	dump_arguments_t *task_args = (dump_arguments_t *) args;
	sat_packet_t dump_tlm = { 0 };

	int err = 0;
	int ack_return_code = ACK_DUMP_FINISHED;
	Boolean is_last_read = FALSE;
	FileSystemResult result = 0;
	int availFrames = 1;
	int num_packets_read = 0; //number of packets read from buffer (single_time)
	int total_packets_read = 0; //total number of packets read from buffer
	unsigned int num_of_elements = 0;
	int size_of_element = 0;
	int size_of_element_with_timestamp;
	time_unix last_read_time; // this is the last time we have on the buffer
	time_unix last_sent_time = task_args->t_start; // this is the last we actually sent(where we want to search next)


	unsigned char *buffer = NULL;
	char filename[MAX_F_FILE_NAME_SIZE] = { 0 };

	buffer = dump_arr;

	err = getTelemetryMetaData(task_args->dump_type, filename, &size_of_element);
	if(0 != err) {
		// TODO: see if this can fit into our goto
		LOGE("problem during dump init with err %d", err);
		FinishDump(task_args, buffer, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
		return;
	}

	size_of_element_with_timestamp = size_of_element + sizeof(time_unix);
	f_managed_enterFS();
	LOGD("\nfilename: %s, size of element: %d t_start: %d t_end: %d", filename, size_of_element, task_args->t_start, task_args->t_end);

	// TODO: consider if we actually want to know the number of packets that will be sent,
	// as it won't be exactly easy.
	SendAckPacket(ACK_DUMP_START, &(task_args->cmd),
			(unsigned char*) &num_of_elements, sizeof(num_of_elements));

	time_unix curr = 0;
	Time_getUnixEpoch(&curr);
	LOGD("starting dump loop at time: %u", curr);
	while(!is_last_read) {
		// read
		num_packets_read = 0;

		// TODO: consider different resolution
		result = c_fileRead(filename, buffer, SIZE_DUMP_BUFFER, 
		last_sent_time, task_args->t_end, &num_packets_read, &last_read_time,1);
		if(result != FS_BUFFER_OVERFLOW) {
			LOGD("c_fileRead returned not buffer overflow but: %d", result);
			is_last_read = TRUE;
		}
		LOGD("read from buffer, num_packets_read: %d", num_packets_read);
		last_sent_time = last_read_time;
		total_packets_read += num_packets_read;
		// send packets
		for(int i = 0; i < num_packets_read; i++) {
			if (CheckDumpAbort() || !CheckTransmitionAllowed()) {
				LOGE("got dump abort");
				ack_return_code = ACK_DUMP_ABORT;
				goto cleanup;
			}
			if (0 == availFrames) {
				vTaskDelay(10);
			}
			AssembleCommand((unsigned char*)buffer + size_of_element_with_timestamp * i, 
				size_of_element_with_timestamp,
				(char) DUMP_SUBTYPE, (char) (task_args->dump_type),
				task_args->cmd.ID, &dump_tlm);
			err = TransmitSplPacket(&dump_tlm, &availFrames);
			if(err != 0) {
				LOGD("transmitsplpacket error: %d", err);
			}
			// availFrames isn't set!! we lose frames without this delay
			if((i+1)%10 == 0) vTaskDelay(100);
		}
	}
	LOGD("finish dump gracefully %d transmitted", total_packets_read);
cleanup:
	f_managed_releaseFS();
	FinishDump(task_args, buffer, ack_return_code, NULL, 0);
	while(1) {
		LOGD("at end of dump task");
		vTaskDelay(5000);
	};
}

int DumpTelemetry(sat_packet_t *cmd) {
	if (NULL == cmd) {
		return -1;
	}

	dump_arguments_t *dmp_pckt = malloc(sizeof(*dmp_pckt));
	int index = 0;
	memcpy(&(dmp_pckt->dump_type),&cmd->data[index],sizeof(dmp_pckt->dump_type));
	index+=sizeof(dmp_pckt->dump_type);
	memcpy(&(dmp_pckt->t_start),&cmd->data[index],sizeof(dmp_pckt->t_start));
	index+=sizeof(dmp_pckt->t_start);
	memcpy(&(dmp_pckt->t_end),&cmd->data[index],sizeof(dmp_pckt->t_end));
	index+=sizeof(dmp_pckt->t_end);
	memcpy(&(dmp_pckt->cmd),cmd,sizeof(*cmd));


	if (xSemaphoreTake(xDumpLock,SECONDS_TO_TICKS(1)) != pdTRUE) {
		return E_GET_SEMAPHORE_FAILED;
	}
	xTaskCreate(DumpTask, (const signed char* const )"DumpTask", 2000,
			(void* )dmp_pckt, configMAX_PRIORITIES - 2, &xDumpHandle);

	return 0;
}

Boolean CheckForGsModeEnd(){
	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);
	time_unix gs_duaration =  curr_time - g_gs_start_time;
	if(gs_duaration >= GS_MODE_TIME){
		return TRUE;
	}
	return FALSE;
}

int EnterGS_Mode(){
	if(MUTE_ON == GetMuteFlag()){
		return 0;
	}
	Time_getUnixEpoch(&g_gs_start_time);
	int err = 0;
//	err = IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_idle_state_on);
	vTaskDelay(100);
	return err;
}

int ExitGS_Mode(){
	int err = 0;
	err = IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_idle_state_off);
	vTaskDelay(100);
	return err;
}


