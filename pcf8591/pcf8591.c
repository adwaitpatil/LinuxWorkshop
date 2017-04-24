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

int main(int argc, char **argv)
{
	uint32_t fd;
	uint32_t ret;
	uint32_t adc_channel = 0;
	uint32_t dac_output = 0;
	uint8_t command[2] = {0};
	uint8_t value[4] = {0};
	useconds_t delay = 5000;

	char *dev = "/dev/i2c-0";

	printf("PCF8591 ADC and DAC test code\n");

	fd = open(dev, O_RDWR);
	if (fd < 0) {
		perror("Opening i2c device node\n");
		return -1;
	}

	ret = ioctl(fd, I2C_SLAVE, PCF8591_ADDRESS);
	if (ret < 0) {
		perror("Selecting i2c device\n");
		close (fd);
		return -1;
	}

	while (true) {
		printf("\n1. ADC 2. DAC 3. Exit\n");
		scanf("%d", &adc_channel);
		switch (adc_channel) {
		case 1:
			printf("\nEnter ADC channel number 0-3:\t");
			scanf("%d", &adc_channel);
			if ((adc_channel >= 0) && (adc_channel <= 3)) {
				/*
				 * Analog output enable | Read Input i
				 * See Page 6 of NXP PCF8591 data sheet
				 */
				command[0] = 0x40 | ((adc_channel) & 0x03);	/* Control Byte */
				command[1] = dac_output;					/* DAC Data Register */

				ret = write(fd, &command, 2);
				if (ret != 2) {
					perror("i2c write failed");
					continue;
				}

				usleep(delay);

				ret = read(fd, &value[0], 1);
				if(ret != 1) {
					perror("i2c read failed");
					continue;
				}

				printf("\nADC Channel %d: value: %d\n", adc_channel, value[0]);
			} else {
				printf("Invalid ADC channel number\n");
			}
			break;
		case 2:
			printf("Enter DAC value:\t");
			scanf("%d", &dac_output);

			command[0] = 0x40 | ((adc_channel) & 0x03);	/* Control Byte */
			command[1] = dac_output & 0xff;				/* DAC Data Register */

			ret = write(fd, &command, 2);
			if (ret != 2) {
				perror("i2c write failed");
				continue;
			}

			usleep(delay);
			break;
		case 3:
			goto exit;
			break;
		default:
			printf("Invalid input\n");
			break;
		}
	}

exit:
	close(fd);

	return(0);
}
