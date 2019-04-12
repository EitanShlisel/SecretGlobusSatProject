
#ifndef COMMANDDICTIONARY_H_
#define COMMANDDICTIONARY_H_

#include "SPL.h"

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
