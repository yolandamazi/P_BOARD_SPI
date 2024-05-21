#include "riscv_types.h"
#include "riscv_uart.h"
#include "clinc.h"
#include "dispatch.h"
#include "log.h"

#include "math.h"
#include "nav_spi.h"
#include "gpio_drv.h"

#include "riscv_monotonic_clock.h"

int main()
{
	uint8_t value, status;

	int16_t AclX, AclY, AclZ;

	//config spi device
	config_spi_nav();

	value=read_nav( ACCELEROMETER_SLAVE, WHO_AM_I );
	printf("Accelerometer Device  ID:      %02X\n", value);  // Should be 0x68

	value=read_nav( MAGNETOMETER_SLAVE, WHO_AM_I );
	printf("Magnetomer Device ID:      %02X\n", value);      // Should be 0x3D

	value=read_nav( BAROMETER_SLAVE, WHO_AM_I );
	printf("BAROMETER_SLAVE Device ID:      %02X\n", value); // Should be 0xBD

	config_nav_measurement();

	while(1)
	{
		// Leo datos cada medio segundo
		delay(CLINT_CLOCK/2);

		do {
			status = read_nav(ACCELEROMETER_SLAVE, STATUS_REG);
		} while( !(status & (1<<XLDA_BIT)) ); // XLDA Accelerometer new data available bit

		AclX =getACEL_X();
		AclY =getACEL_Y();
		AclZ =getACEL_Z();

		// Normalizar los datos
		double AclX_norm = (double)AclX / 16384.0;
		double AclY_norm = (double)AclY / 16384.0;
		double AclZ_norm = (double)AclZ / 16384.0;

		printf("AclX: %f ", AclX_norm);
		printf("AclY: %f ", AclY_norm);
		printf("AclZ: %f ", AclZ_norm);

		//float angulo = atan( AclX_norm / AclZ_norm ) * (180 / M_PI); // Atan retorna radianes
		//printf("Angulo X/Z: %f\n", angulo  );

		float pitch = atan2(AclX_norm, sqrt(AclY_norm*AclY_norm + AclZ_norm*AclZ_norm)) * (180 / M_PI);
		float roll =  atan2(AclY_norm, sqrt(AclX_norm*AclX_norm + AclZ_norm*AclZ_norm)) * (180 / M_PI);
		printf("Pitch: %f Roll: %f\n", pitch, roll );
	}

	return 0;
}



