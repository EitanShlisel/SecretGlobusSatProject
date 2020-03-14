#ifndef TELEMETRYCOLLECTOR_H_
#define TELEMETRYCOLLECTOR_H_

#include "GlobalStandards.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"

//TODO: finish WOD telemetry according to requirements... TRX TLM...
typedef struct __attribute__ ((__packed__)) WOD_Telemetry_t
{
	time_unix sat_time;				///< current Unix time of the satellites clock [sec]
	power_t charging_power;			///< the charging power [mW]
	voltage_t vbat;					///< the voltage on the battery [mV]

	voltage_t voltage_channel_0;	///< the output voltage on channel 0 [mV]
	voltage_t voltage_channel_1;	///< the output voltage on channel 1 [mV]
	voltage_t voltage_channel_2;	///< the output voltage on channel 2 [mV]
	voltage_t voltage_channel_3;	///< the output voltage on channel 3 [mV]
	voltage_t voltage_channel_4;	///< the output voltage on channel 4 [mV]
	voltage_t voltage_channel_5;	///< the output voltage on channel 5 [mV]
	voltage_t voltage_channel_6;	///< the output voltage on channel 6 [mV]
	voltage_t voltage_channel_7;	///< the output voltage on channel 7 [mV]

	current_t current_channel_0;	///< the output current on channel 0 [mA]
	current_t current_channel_1;	///< the output current on channel 1 [mA]
	current_t current_channel_2;	///< the output current on channel 2 [mA]
	current_t current_channel_3;	///< the output current on channel 3 [mA]
	current_t current_channel_4;	///< the output current on channel 4 [mA]
	current_t current_channel_5;	///< the output current on channel 5 [mA]
	current_t current_channel_6;	///< the output current on channel 6 [mA]
	current_t current_channel_7;	///< the output current on channel 7 [mA]

	unsigned int free_memory;		///< number of bytes free in the satellites SD [byte]
	unsigned int corrupt_bytes;		///< number of currpted bytes in the memory	[bytes]
	unsigned short number_of_resets;///< counts the number of resets the satellite has gone through [#]
} WOD_Telemetry_t;

#define NUMBER_OF_TELEMETRIES 10	///< number of telemetries the satellite saves
#define NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS 5			///<
/*!
 * @brief copies the corresponding filename into a buffer.
 * @return	-1 on NULL input
 * 			-2 on unknown  tlm_type
 */
int GetTelemetryFilenameByType(tlm_type tlm_type,char filename[MAX_F_FILE_NAME_SIZE]);


/*!
 * @brief Creates all telemetry files,
 * @param[out]	tlms_created states whether the files were created successful
 */
void TelemetryCreateFiles(Boolean8bit tlms_created[NUMBER_OF_TELEMETRIES]);

/*!
 * @brief saves all telemetries into the appropriate TLM files
 */
void TelemetryCollectorLogic();

/*!
 *  @brief saves current EPS telemetry into file
 */
void TelemetrySaveEPS();

/*!
 *  @brief saves current TRXVU telemetry into file
 */
void TelemetrySaveTRXVU();

/*!
 *  @brief saves current Antenna telemetry into file
 */
void TelemetrySaveANT();

/*!
 *  @brief saves current solar panel telemetry into file
 */
void TelemetrySaveSolarPanels();

/*!
 *  @brief saves current WOD telemetry into file
 */
void TelemetrySaveWOD();

/*!
 * @brief Gets all necessary telemetry and arranges it into a WOD structure
 * @param[out] output WOD telemetry. If an error occurred while getting TLM the fields will be zero
 */
void GetCurrentWODTelemetry(WOD_Telemetry_t *wod);

#endif /* TELEMETRYCOLLECTOR_H_ */
