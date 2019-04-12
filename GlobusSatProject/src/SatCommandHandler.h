
#ifndef SATCOMMANDS_H_
#define SATCOMMANDS_H_

#define MAX_COMMAND_DATA_LENGTH 200 ///< maximum AX25 data field available for downlink

typedef enum __attribute__ ((__packed__)) CMD_ERR{
	command_found = 0,
	no_command_found,
	execution_error
}CMD_ERR;

typedef struct __attribute__ ((__packed__)) sat_packet_t
{
	char cmd_type;
	char cmd_subtype;
	unsigned int length;
	unsigned char data[MAX_COMMAND_DATA_LENGTH];

}sat_packet_t;


/*!
 * @brief parses given data stream into 'sat_command_t' structure.
 * @param[out] parsed_cmd pointer to parsed command buffer
 * @param[in] data uplink data to be parsed into a 'sat_command_t'
 * @param[in] data_length length of data in received packet in bytes
 * @return	0 when successful
 * 			-1 when failed
 */
int parseCommand(sat_packet_t* parsed_cmd,char *data, unsigned int data_length);

/*!
 * @brief returns a command to be executed if there is one in the delayed command buffer
 * @param[out] cmd pointer to parsed command
 * @return	0 when command to execute found
 * 			-1 when no command to execute
 * 			-2 when when error occurred
 */
int GetDelayedCommand(sat_packet_t *cmd);

/*!
 * @brief returns number of commands in the delayed command buffer
 * @return	#number number of delayed commands in buffer
 * 			0 when no command to execute
 * 			-1 when when error occurred
 */
int GetDelayedCommandBufferCount();

/*!
 * @brief returns an online(immediate) command to be executed if there is one in the command buffer
 * @note cmd is set
 * @return	0 when command to execute found
 * 			-1 when no command to execute
 * 			-2 when when error occured
 */
int GetOnlineCommand(sat_packet_t *cmd);

#endif /* SATCOMMANDS_H_ */
