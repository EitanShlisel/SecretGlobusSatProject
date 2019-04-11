/*
 * main.h
 *
 *  Created on: Feb 18, 2019
 *      Author: Eitan
 */

#ifndef MAIN_H_
#define MAIN_H_



/*!
 * @brief initialization of the satellites subsystems
 * @note the function raises a flag stating whether
 *  satellite already hard-deployed(antennas, 30min' delay, etc')
 * @return	0 successful initialization
 * 			-1 failure at initialization
 *
 */
int InitSubSystems();

/*!
 * 	@brief The main operation function. This function implements the 'Loop'
 *  side of the logic and handles all of the satellite macro-logics(TRX,EPS etc')
 *  @note the function must use an infinite loop at its base;
 *  according to the logic and FreeRTOS thread requirements
 */
void taskmain();

/*!
* This is the main function of the satellite operation
* The main function contains a 'Boot' call which is operated once,
* and the taskMain thread which is activated repeatedly.
*/
int main();

#endif /* MAIN_H_ */

