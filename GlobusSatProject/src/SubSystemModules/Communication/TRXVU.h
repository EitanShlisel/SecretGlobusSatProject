#ifndef TRXVU_H_
#define TRXVU_H_


#include <satellite-subsystems/IsisTRXVU.h>
#include "GlobalStandards.h"
#include "AckHandler.h"
#include "SatCommandHandler.h"

#define SIZE_RXFRAME	200		///< max size of data field in uplink
#define SIZE_TXFRAME	235		///< max size of data field in downlink

#define SIZE_DUMP_BUFFER (4096) ///< buffer size for dump operations

#define DEFAULT_TRANS_RSSI	1500

typedef struct __attribute__ ((__packed__))
{
	unsigned char dump_type;
	time_unix t_start;
	time_unix t_end;
	sat_packet_t cmd;
} dump_arguments_t;


/*!
 * @brief initializes the TRXVU subsystem
 * @return	0 on successful init
 * 			errors according to <hal/errors.h>
 */
int InitTrxvu();

/*!
 * @brief The TRXVU logic according to the sub-system flowchart
 * @return	command_succsess on success
 * 			errors according to CommandHandlerErr enumeration
 * @see "SatCommandHandler.h"
 */
CommandHandlerErr TRX_Logic();


/*!
 * @brief sends an abort message via a freeRTOS queue.
 */
void SendDumpAbortRequest();


/*!
 * @brief dump telemetry to the ground station with telemetry in time range specified in 'cmd'
 * @param[in] cmd the dump command
 * @note this function starts a new dump task
 * @return	0 on success
 * 			-1 on failure
 */
int DumpTelemetry(sat_packet_t *cmd);

/*!
 * @breif checks if GS mode has finished.
 * @return	TRUE if GS mode has ended.
 * 			FALSE if GS mode has yet to end
 */
Boolean CheckForGsModeEnd();

/*!
 *	@breif Starts the GS mode
 *	@return errors according to <hal/errors.h>
 */
int EnterGS_Mode();

/*!
 *	@breif Stops the GS mode.
 *	@return errors according to <hal/errors.h>
 */
int ExitGS_Mode();


#endif
