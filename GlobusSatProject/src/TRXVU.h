
#include "GlobalStandards.h"
#include "SatCommandHandler.h"

#define MAX_MUTE_TIME 660 	///< max mute duration will be 11 minutes = 60 *11 [sec]
#define MUTE_ON 0xff		///< mute is on flag
#define MUTE_OFF 0			///< mute is off flag

/*!
 * @brief initializes the TRXVU subsystem
 * @return	0 on successful init
 * 			errors according to <hal/errors.h>
 */
int init_trxvu();

/*!
 * @brief The TRXVU logic according to the sub-system flowchart
 * @return	0 on success
 * 			-1 on failure
 */
int TRX_Logic();

/*!
 * @brief 	Transmits a packet according to the SPL protocol
 * @param[in] packet packet to be transmitted
 * @return    Error code according to <hal/errors.h>
 */
int TransmitSplPacket(sat_packet_t *packet);

/*!
 * @brief dump telemetry to the ground station with telemetry in time range
 * @param[in] type telemetry type to be dumped
 * @param[in] t_start time from which to start dumping
 * @param[in] t_end time from which to stop dump
 * @return	0 on success
 * 			-1 on failure
 */
int DumpTelemetry(int type, time_unix t_start, time_unix t_end);

/*!
 * @brief
 * @param
 * @return
 */
void BeaconLogic();

/*!
 * @brief
 * @param[in] duration for how long will the satellite be in mute state
 * @param[in] mute_enable mute flag. according to 'MUTE_ON' and 'MUTE_OFF'
 * @return	0 in successful
 * 			-1 in failure
 */
int muteTRXVU(time_unix duration, char mute_enable);

/*!
 * @brief returns number of online frames are in the TRX frame buffer
 * @return	#number number of packets available
 * 			-1 in case of failure
 */
int GetNumberOfFramesInBuffer();

