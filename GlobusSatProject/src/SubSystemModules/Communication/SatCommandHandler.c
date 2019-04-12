
#include <satellite-subsystems/IsisTRXVU.h>
#include <hal/Timing/Time.h>
#include <string.h>
#include <stdlib.h>

#include "GlobalStandards.h"
#include "SatCommandHandler.h"

typedef struct __attribute__ ((__packed__)) delayed_cmd_t
{
	time_unix exec_time;	///< the execution time of the cmd in unix time
	sat_packet_t cmd;		///< command data
}delayed_cmd_t;


// @param[in] cmd_time command execution time to check
Boolean isDelayedCommandDue(time_unix cmd_time, Boolean *expired)
{
	int err = 0;
	time_unix present_time = 0;
	err = Time_getUnixEpoch((time_unix*)&present_time);
	if(0 != err){
		return FALSE;
	}
	if(NULL != expired)
	{
		*expired = (present_time - cmd_time) > DELAYED_CMD_TIME_BOUND;
	}

	if(cmd_time < present_time)
	{
		return TRUE;
	}
	return FALSE;
}

//TODO: update this function to be more 'delayed cmd' specific
int clearFRAM_Block(unsigned int start_addr,unsigned int end_addr)
{
	if(end_addr <= start_addr || end_addr >= FRAM_MAX_ADDRESS){
		return -1;
	}
	unsigned int region_size =(end_addr-start_addr) * sizeof(unsigned char);

	unsigned char *temp_arr = (unsigned char*)malloc(region_size);
	memset(temp_arr,0, region_size);
	FRAM_write(temp_arr,start_addr,end_addr);

	free(temp_arr);

	unsigned char frame_count=0;
	FRAM_read(&frame_count,DELAYED_CMD_FRAME_COUNT_ADDR,DELAYED_CMD_FRAME_COUNT_SIZE);

	frame_count++;
	FRAM_write(&frame_count,DELAYED_CMD_FRAME_COUNT_ADDR,DELAYED_CMD_FRAME_COUNT_SIZE);

	return 0;
}

int parseCommand(sat_packet_t* parsed_cmd,char *data, unsigned int data_length)
{
	if(NULL == data || NULL == parsed_cmd)
	{
		return execution_error;
	}
	parsed_cmd->cmd_type = data[0];
	parsed_cmd->cmd_subtype = data[1];
	parsed_cmd->length = CHAR_ARRAY_TO_INT(data	+ sizeof(parsed_cmd->cmd_type)
			 	 	 	 	 	 	 	 	 	+ sizeof(parsed_cmd->cmd_subtype));

	memset(parsed_cmd->data,0,MAX_COMMAND_DATA_LENGTH); //set array to zero
	unsigned int offset = sizeof(parsed_cmd->cmd_type)
						 + sizeof(parsed_cmd->cmd_subtype)
						 + sizeof(parsed_cmd->length);
	memcpy(parsed_cmd->data,data + offset,data_length * sizeof(data[0]));
	return command_found;
}

int GetDelayedCommand(sat_packet_t *cmd)
{
	if(NULL == cmd)
	{
		return execution_error;
	}

	unsigned int current_fram_addr = DELAYED_CMD_BUFFER_ADDR;
	time_unix exec_time = 0;
	Boolean cmd_expired = FALSE;
	for (int i = 0; i < DELAYED_CMD_BUFFER_LENGTH; i++)
	{
		FRAM_read((unsigned char*)&exec_time,current_fram_addr,sizeof(time_unix));

		if(isDelayedCommandDue(exec_time,&cmd_expired))
		{
			FRAM_read((unsigned char*)cmd,current_fram_addr + sizeof(time_unix),sizeof(delayed_cmd_t));
			clearFRAM_Block(current_fram_addr,current_fram_addr + sizeof(delayed_cmd_t));
			break;
		}
		else if(cmd_expired)
		{
			clearFRAM_Block(current_fram_addr,current_fram_addr + sizeof(delayed_cmd_t));
		}
		current_fram_addr += sizeof(delayed_cmd_t);
	}

	return command_found;
}

int GetDelayedCommandBufferCount()
{
	unsigned char frame_count = 0;
	int err = FRAM_read(&frame_count,DELAYED_CMD_FRAME_COUNT_ADDR,DELAYED_CMD_FRAME_COUNT_SIZE);
	return err ? -1 : frame_count;

}

int GetOnlineCommand(sat_packet_t *cmd)
{
	if(NULL == cmd)
	{
		return execution_error;
	}
	int err = 0;
	unsigned int length = 0;
	unsigned short frame_count = 0;
	char received_frame_data[MAX_COMMAND_DATA_LENGTH];

	err = IsisTrxvu_rcGetFrameCount(0, &frame_count);
	if(0 != err){
		return execution_error;
	}
	if (0 == frame_count ){
		return no_command_found;
	}

	ISIStrxvuRxFrame rxFrameCmd = { 0, 0, 0, (unsigned char*)received_frame_data }; // for getting raw data from Rx, nullify values

	err = IsisTrxvu_rcGetCommandFrame(0, &rxFrameCmd);//get the frame from the Rx buffer
	if(0 != err){
		return execution_error;
	}

	length = (unsigned int)rxFrameCmd.rx_length;
	err = parseCommand(cmd,received_frame_data,length);
	if(0 != err){
		return execution_error;
	}
	return command_found;
}
