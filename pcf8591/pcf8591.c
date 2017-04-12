#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define PCF8591_ADDRESS		0x48

int main( int argc, char **argv )
{
	uint8_t i;
	uint32_t fd;
	uint32_t ret;
	uint8_t command[2];
	uint8_t value[4];
	useconds_t delay = 2000;

	char *dev = "/dev/i2c-0";

	printf("PCF8591 ADC and DAC test code\n");

	fd = open(dev, O_RDWR);
	if(fd < 0) {
		perror("Opening i2c device node\n");
		return 1;
	}

	ret = ioctl(fd, I2C_SLAVE, PCF8591_ADDRESS);
	if(ret < 0) {
		perror("Selecting i2c device\n");
	}

	while (true) {
		for(i = 0; i < 4; i++) {

			/*
			 * Analog output enable | Read Input i
			 * See Page 6 of NXP PCF8591 data sheet
			 */
			command[0] = 0x40 | ((i + 1) & 0x03);	/* Control Byte */
			command[1]++;							/* DAC Data Register */

			ret = write(fd, &command, 2);
			if (ret != 2)
				perror("writing i2c device");

			usleep(delay);

			/* the read is always one step behind the selected input */
			ret = read(fd, &value[i], 1);
			if(ret != 1)
				perror("reading i2c device");

			usleep(delay);
		}

		printf("0x%02x 0x%02x 0x%02x 0x%02x\n", value[0], value[1], value[2], value[3]);
		sleep(1);
	}

	close(fd);
	return(0);
}
