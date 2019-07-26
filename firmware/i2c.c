#include "i2c.h"
#include <generated/csr.h>

static void wait(unsigned int s)
{
	timer0_en_write(0);
	timer0_reload_write(0);
	timer0_load_write(SYSTEM_CLOCK_FREQUENCY*s);
	timer0_en_write(1);
	timer0_update_value_write(1);
	while(timer0_value_read()) timer0_update_value_write(1);
}

void i2c_init(void){

	uint32_t div_write= SYSTEM_CLOCK_FREQUENCY/(5*FREQ_SCL)-1;
    	//i2c_prescale_write(div_write);
	i2c_rst_1_write(0);
	i2c_I2C_rst_write(1);	
	#1
	
	i2c_rst_1_write(1);
	i2c_I2C_rst_write(0);
	i2c_prescale_write(19999);
	i2c_control_write(0x80);
	i2c_command_write(0x0); //REcién agregado
}

/*
I2C Sequence:
1) generate start command 2) write slave address + write bit
3) receive acknowledge from slave 4) write data
5) receive acknowledge from slave 6) generate stop command
*/

uint8_t _send(uint8_t dat, uint8_t dcom){
	i2c_transmit_write(dat);
	i2c_command_write(dcom);
	printf("a%d\n", i2c_status_read());
	while (STIP & i2c_status_read());
	printf("b%d\n", i2c_status_read());
	if (SRxACK & i2c_status_read())
		return 0;
	else
		return 1;

}
uint8_t i2c_write(uint8_t addr, uint8_t dato){

	if (_send(addr<<1 + 0, CSTA+CTX)){
		printf("c%d\n", i2c_status_read());
		if (_send(dato, CTX+CSTA)){
			printf("d%d \n", i2c_status_read());
			return 1;
		}
	}
	return 0;
}

