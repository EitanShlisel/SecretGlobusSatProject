#ifndef BEACON_H_
#define BEACON_H_

/*!
 *	@brief initializes the relevant parameters for
 *	the beacon module ongoing work
 */
void InitBeaconParams();

/*!
 * @brief transmits beacon according to beacon logic
 */
void BeaconLogic();

//TODO:document
int UpdateBeaconBaudCycle(unsigned char cycle);

//TODO:document
int UpdateBeaconInterval(time_unix intrvl);


#endif /* BEACON_H_*/
