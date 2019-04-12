#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>

#include <at91/utility/exithandler.h>
#include <string.h>

#include "GlobalStandards.h"
#include "InitSystem.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"

#define I2c_SPEED_Hz 100000
#define I2c_Timeout 10
#define I2c_TimeoutTest portMAX_DELAY

void firstActivationProcedure()
{
	//TODO: finish 'DeploySystem'
	//TODO: finish 'firstActivationProcedure'
}

void StartFRAM()
{
	int error = FRAM_start();
	if (0 != error) {
		printf("error in FRAM_start(); err = %d\n", error);
	}
}

void StartI2C()
{
	int error = I2C_start(I2c_SPEED_Hz, I2c_Timeout);
	if (0 != error) {
		printf("error in I2C_start; err = %d\n", error);
	}
}

void StartSPI()
{
	int error = SPI_start(bus1_spi, slave1_spi);
	if (0 != error) {
		printf("error in SPI_start; err = %d\n", error);
	}
}

void StartTIME()
{
	int error = 0;
	Time initial_time_jan2000 = UNIX_JAN_D1_Y2000;
	error = Time_start(&initial_time_jan2000, 0);
	if (0 != error) {
		printf("error in StartTIME; err = %d\n", error);
	}
}

int DeploySystem()
{
	unsigned char first_activation[FIRST_ACTIVATION_FLAG_SIZE] = { 0 };
	FRAM_read(first_activation, FIRST_ACTIVATION_FLAG_ADDR,
			FIRST_ACTIVATION_FLAG_SIZE);

	char res = 0x00;
	for (int i = 0; i < FIRST_ACTIVATION_FLAG_SIZE; ++i)
	{
		res |= first_activation[i];
	}

	//TODO: increase of fatal error RESTART
	if (!res)
	{
		vTaskDelay(MINUTES_TO_MILLISECONDS(30));// wait 30 minutes before deployment. 30min = 1800 sec = 1800*1000 millisec

		firstActivationProcedure();

		//set 'first_activation' to TRUE
		memset(first_activation, (unsigned char) (0xFF),
				FIRST_ACTIVATION_FLAG_SIZE);
		FRAM_write(first_activation, FIRST_ACTIVATION_FLAG_ADDR,
				FIRST_ACTIVATION_FLAG_SIZE);
	}
	return 0;
}

int InitSubsystems()
{
	StartI2C();

	StartSPI();

	StartFRAM();

	StartTIME();

	// InitializeFS(first_activation);

	EPS_Init();

	init_trxvu();

	DeploySystem();

	return 0;
}

