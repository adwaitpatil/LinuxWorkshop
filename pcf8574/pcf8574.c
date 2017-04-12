#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <libsoc_i2c.h>

/* PCF8574 I2C Address on Pioneer 600 */
#define PCF8574_ADDRESS		0x20

static uint8_t RS = 0x01;
static uint8_t EN = 0x04;
static uint8_t BL = 0x08;

/*
 * libsoc i2c API Documentation: http://jackmitch.github.io/libsoc/c/i2c/#i2c
 */

int pulse_enable(i2c *i2c, uint8_t data)
{
	uint32_t ret;
	uint8_t buf[4] = {0};

	buf[0] = data | EN;
	ret = libsoc_i2c_write(i2c, buf, 1);
	if (ret == EXIT_FAILURE)
		return ret;

	usleep(1);

	buf[0] = data & ~EN;
	ret = libsoc_i2c_write(i2c, buf, 1);

	return ret;
}

int pcf8574_write_cmd4(i2c *i2c, uint8_t command)
{
	uint32_t ret;
	uint8_t buf[4] = {0};

	buf[0] = command | BL;
	ret = libsoc_i2c_write(i2c, buf, 1);
	if (ret == EXIT_FAILURE)
		return ret;

	ret = pulse_enable(i2c, command | BL);
	if (ret == EXIT_FAILURE)
		perror("Pulse enable failed\n");

	return ret;
}

int pcf8574_write_cmd8(i2c *i2c, uint8_t command)
{
	uint32_t ret;
	uint8_t command_H = command & 0xf0;
	uint8_t command_L = (command << 4) & 0xf0;

	ret = pcf8574_write_cmd4(i2c, command_H);
	if (ret == EXIT_FAILURE) {
		perror("Failed to write command high 4 bits");
		return ret;
	}

	ret = pcf8574_write_cmd4(i2c, command_L);
	if (ret == EXIT_FAILURE) {
		perror("Failed to write command low 4 bits");
		return ret;
	}

	return EXIT_SUCCESS;
}

int pcf8574_write_data4(i2c *i2c, uint8_t data)
{
	uint32_t ret;
	uint8_t buf[4] = {0};

	buf[0] = data | RS | BL;
	ret = libsoc_i2c_write(i2c, buf, 1);
	if (ret == EXIT_FAILURE)
		return ret;

	ret = pulse_enable(i2c, data | RS | BL);
	if (ret == EXIT_FAILURE)
		perror("Pulse enable failed\n");

	return ret;
}

int pcf8574_write_data8(i2c *i2c, uint8_t data)
{
	uint8_t ret;
	uint8_t data_H = data & 0xf0;
	uint8_t data_L = (data << 4) & 0xf0;

	ret = pcf8574_write_data4(i2c, data_H);
	if (ret == EXIT_FAILURE) {
		perror("Failed to write data high 4 bits");
		return ret;
	}

	ret = pcf8574_write_data4(i2c, data_L);
	if (ret == EXIT_FAILURE) {
		perror("Failed to write data low 4 bits");
		return ret;
	}

	return ret;
}

i2c* pcf8574_init(uint8_t i2c_bus, uint8_t i2c_address)
{
	return libsoc_i2c_init(i2c_bus, i2c_address);
}

int pcf8574_close(i2c *i2c)
{
	return libsoc_i2c_free(i2c);
}

int main(void)
{
	i2c *i2c_pcf8574;

	i2c_pcf8574 = pcf8574_init(0, PCF8574_ADDRESS);
	pcf8574_write_data8(i2c_pcf8574, 0xff);

	pcf8574_close(i2c_pcf8574);

	return 0;
}
