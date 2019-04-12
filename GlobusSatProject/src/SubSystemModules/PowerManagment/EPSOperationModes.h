#ifndef EPSOPERATIONMODES_H_
#define EPSOPERATIONMODES_H_

#include <satellite-subsystems/IsisEPS.h>

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Full mode
 */
void EnterFullMode();

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Cruise mode
 */
void EnterCruiseMode();

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Safe mode
 */
void EnterSafeMode();

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Critical mode
 */
void EnterCriticalMode();

#endif /* EPSOPERATIONMODES_H_ */
