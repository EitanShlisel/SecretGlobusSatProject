
#ifndef COMMANDDICTIONARY_H_
#define COMMANDDICTIONARY_H_

typedef enum spl_command_type_t{
	trxvu_cmd_type,
	eps_cmd_type,
	telemetry_cmd_type,
	filesystem_cmd_type,
	obc_cmd_type
}spl_command_type_t;

/*!
 * @brief routes the data into the appropriate trxvu command according to the command sub type
 * @param[in] sub_type sup-type of the command to be executed
 * @param[in] data the data to be passed into the command to be executed
 * @return
 */
int trxvu_command_router(char sub_type, unsigned char *data);

/*!
 * @brief routes the data into the appropriate eps command according to the command sub type
 * @param[in] sub_type sup-type of the command to be executed
 * @param[in] data the data to be passed into the command to be executed
 * @return
 */
int eps_command_router(char sub_type, unsigned char *data);

/*!
 * @brief routes the data into the appropriate telemetry command according to the command sub type
 * @param[in] sub_type sup-type of the command to be executed
 * @param[in] data the data to be passed into the command to be executed
 * @return
 */
int telemetry_command_router(char sub_type, unsigned char *data);

/*!
 * @brief routes the data into the appropriate file-system command according to the command sub type
 * @param[in] sub_type sup-type of the command to be executed
 * @param[in] data the data to be passed into the command to be executed
 * @return
 */
int filesystem_command_router(char sub_type, unsigned char *data);

/*!
 * @brief routes the data into the appropriate obc command according to the command sub type
 * @param[in] sub_type sup-type of the command to be executed
 * @param[in] data the data to be passed into the command to be executed
 * @return
 */
int obc_command_router(char sub_type, unsigned char *data);

#endif /* COMMANDDICTIONARY_H_ */
