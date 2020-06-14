#ifndef TELEMETRYCOLLECTOR_H_
#define TELEMETRYCOLLECTOR_H_

#include "GlobalStandards.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"

typedef struct __attribute__ ((__packed__)) WOD_Telemetry_t
{
	voltage_t vbat;									///< the current voltage on the battery [mV]
	voltage_t volt_5V;								///< the current voltage on the 5V bus [mV]
	voltage_t volt_3V3;								///< the current voltage on the 3V3 bus [mV]
	power_t charging_power;							///< the current charging power [mW]
	power_t consumed_power;							///< the power consumed by the satellite [mW]
	current_t electric_current;						///< the up-to-date electric current of the battery [mA]
	current_t current_3V3;							///< the up-to-date 3.3 Volt bus current of the battery [mA]
	current_t current_5V;							///< the up-to-date 5 Volt bus current of the battery [mA]
	temp_t mcu_temp; 								/*!< Measured temperature provided by a sensor internal to the MCU in raw form */
	temp_t bat_temp; 								/*!< 2 cell battery pack: not used 4 cell battery pack: Battery pack temperature on the front of the battery pack. */
	temp_t solar_panels[NUMBER_OF_SOLAR_PANELS]; 	// temp of each solar panel
	time_unix sat_time;								///< current Unix time of the satellites clock [sec]
	unsigned int free_memory;						///< number of bytes free in the satellites SD [byte]
	unsigned int corrupt_bytes;						///< number of currpted bytes in the memory	[bytes]
	unsigned int number_of_resets;					///< counts the number of resets the satellite has gone through [#]
	unsigned int num_of_cmd_resets;					///< counts the number of resets the satellite has gone through [#]
} WOD_Telemetry_t;

#define NUMBER_OF_TELEMETRIES 			 6			///< number of telemetries the satellite saves
#define NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS 5			///< number of subsystems in the satellite
/*!
 * @brief copies the corresponding filename into a buffer.
 * @return	-1 on NULL input
 * 			-2 on unknown  tlm_type
 */
int GetTelemetryFilenameByType(tlm_type tlm_type,char filename[MAX_F_FILE_NAME_SIZE]);


/*!
 * @brief Creates all telemetry files as metadata information in the FRAM
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
/*!
 *
 * @brief Init Telemetry collector saving periods
 * @param[out] fram error
 */
int InitTelemetryCollrctor();

#endif /* TELEMETRYCOLLECTOR_H_ */
