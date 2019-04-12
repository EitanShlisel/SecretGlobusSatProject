/*
 * GlobalStandards.h
 *
 *  Created on: Feb 19, 2019
 *      Author: Eitan
 */

#ifndef GLOBALSTANDARDS_H_
#define GLOBALSTANDARDS_H_

#include <hal/boolean.h>
#include "FRAM_FlightParameters.h"
#include "SysI2CAddr.h"

#ifndef NULL
	#define NULL ((void*)0)	//!< define 'null'
#endif

#ifndef TRUE
	#define TRUE (0xFF)		//!< define logic 'true' value
#endif

#ifndef FALSE
	#define FALSE (0x00)	//!< define logic 'false' value
#endif

#define CHAR_ARRAY_TO_INT(arr)	(((arr)[3]<<0 & 0xff) + \
								((arr)[2]<<8 & 0xff00) + \
								((arr)[1]<<16 & 0xff0000) + \
								((arr)[0]<<24 & 0xff000000)) \

#define MINUTES_TO_MILLISECONDS(minute) ((minute)*60*1000)
#define SECONDS_TO_TICKS(secs) ((secs * 1000) /portTICK_RATE_MS)
#define UNIX_JAN_D1_Y2000 {0,0,0,1,1,1,0,0} //!< Time structure of the date 1/1/2000 at 00:00
typedef unsigned int time_unix; 	///< in seconds
typedef unsigned char byte;

typedef unsigned short voltage_t; 	///< in mV
typedef unsigned short current_t; 	///< in mA
typedef short temp_t;				///< in celsius
typedef char channel_t;


#endif /* GLOBALSTANDARDS_H_ */
