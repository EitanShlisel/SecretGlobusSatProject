/*
 * @file	EPS.h
 * @brief	EPS- Energy Powering System.This system is incharge of the energy consumtion
 * 			the satellite and switching on and off power switches(5V, 3V3)
 * @see		inspect logic flowcharts thoroughly in order to write the code in a clean manner.
 */

#include <satellite-subsystems/IsisEPS.h>
#include "GlobalStandards.h"

//TODO: change 'SYSTEMX' to the actual system
#define CHANNELS_OFF 0x00 	//!< channel state when all systems are off
#define SYSTEM0		 0x01	//!< channel state when 'SYSTEM0' is on
#define SYSTEM1		 0x02	//!< channel state when 'SYSTEM1' is on
#define SYSTEM2 	 0x04	//!< channel state when 'SYSTEM2' is on
#define SYSTEM3		 0x08	//!< channel state when 'SYSTEM3' is on
#define SYSTEM4		 0x10	//!< channel state when 'SYSTEM4' is on
#define SYSTEM5		 0x20	//!< channel state when 'SYSTEM5' is on
#define SYSTEM6 	 0x40	//!< channel state when 'SYSTEM6' is on
#define SYSTEM7 	 0x80	//!< channel state when 'SYSTEM7' is on
/*!
 * @brief initializes the EPS subsystem.
 * @return	0 on success
 * 			-1 on failure of init
 */
int EPS_Init();

/*!
 * @brief Sets the channel state according to the bitwise 'logic on'
 * if the 2'nd bit is '1' the second channel will turn on (channel = 0b01000000)
 * @note please use the defines defined in this header to turn on/off channels
 * @return	0 on success
 * 			-1 on failure setting channel
 */
int SetChannels(channel_t channel);

/*!
 * @brief EPS logic. controls the state machine of which subsystem
 * is on or off, as a function of only the battery voltage
 * @note this function updates the global data structure
 * @return	0 on success
 * 			-1 on failure setting state of channels
 */
int EPS_Conditioning();

