/*
 *
 * @file	FRAM_FlightParameters.h
 * @brief	ordering all flight parameters(saved on the FRAM)  addresses and sizes
 * @note 	sizes are measured in chars = 1 byte. e.g size = 4, meaning 4 bytes(=int)
 */

#ifndef FRAM_FLIGHTPARAMETERS_H_
#define FRAM_FLIGHTPARAMETERS_H_

#include <hal/Storage/FRAM.h>


// <Satellite Management>
#define FIRST_ACTIVATION_FLAG_ADDR 0x42	//<! is this the first activation after launch flag
#define FIRST_ACTIVATION_FLAG_SIZE 2	//<! length in bytes of FIRST_ACTIVATION_FLAG

#define TIME_ADDR 0X10					//<! most updated time before restart
#define TIME_SIZE 4						//<! length in bytes of TIME
// </Satellite Management>

// <TRXVU>

#define DELAYED_CMD_BUFFER_ADDR			0x100		//<! starting address of the delayed command buffer
#define DELAYED_CMD_BUFFER_ELEMENT_SIZE sizeof(sat_command_t)	//<! size of command element- statically defined structure
#define DELAYED_CMD_BUFFER_LENGTH		50			//<! number of commands the buffer is capable of saving(self defined, can be changed)

#define DELAYED_CMD_TIME_BOUND			60 			//<! if requested execution time has passed by this measure the command will not be executed and deleted from buffer

#define DELAYED_CMD_FRAME_COUNT_ADDR	0X200		//<! Address where amount of frames in delayed buffer is held
#define DELAYED_CMD_FRAME_COUNT_SIZE	1			//<! number of bytes describing the frame count of delayed buffer

#define MUTE_FLAG_ADDR					0x300		//<! flag that states whether the satellite is in mute
#define MUTE_FLAG_SIZE					1			//<! size of mute flag in bytes

#define TRXVU_LOW_VOLTAGE_FLAG_ADDR		0x400		//<! flag that states whether the satellite is in low voltage mode
#define TRXVU_LOW_VOLTAGE_FLAG_SIZE		1			//<! size of low voltage flag in bytes

// </TRXVU>

// <EPS>

// </>


#endif /* FRAM_FLIGHTPARAMETERS_H_ */
