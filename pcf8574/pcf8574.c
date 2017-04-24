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

static uint8_t LED_ON		= 0xEF;
static uint8_t LED_OFF		= 0xFF;
static uint8_t BUZZER_ON	= 0x7F;
static uint8_t BUZZER_OFF	= 0xFF;

/*
 * libsoc i2c API Documentation: http://jackmitch.github.io/libsoc/c/i2c/#i2c
 */

i2c* pcf8574_init(uint8_t i2c_bus, uint8_t i2c_address)
{
	i2c *i2c;
	uint8_t ret;
	uint8_t buf[1] = {0};

	i2c = libsoc_i2c_init(i2c_bus, i2c_address);
	if (i2c == NULL) {
		perror("libsoc_i2c_init failed");
		return i2c;
	}

	/*
	 * http://cache.nxp.com/documents/data_sheet/PCF8574_PCF8574A.pdf
	 * See Page 7 of the data sheet Quasi-bidirectional I/Os
	 * Default all expander pins as input
	 */
	buf[0] = 0xff;
	ret = libsoc_i2c_write(i2c, buf, 1);
	if (ret == EXIT_FAILURE) {
		perror("PCF8574 initialization failed");
		libsoc_i2c_free(i2c);
		return NULL;
	}

	return i2c;
}

int pcf8574_close(i2c *i2c)
{
	return libsoc_i2c_free(i2c);
}

int pioneer600_led2_on(i2c *i2c)
{
	uint8_t ret;
	uint8_t buf[1] = {0};

	buf[0] = LED_ON;
	ret = libsoc_i2c_write(i2c, buf, 1);
	if (ret == EXIT_FAILURE)
		perror("I2C write failed");

	return ret;
}

int pioneer600_led2_off(i2c *i2c)
{
	uint8_t ret;
	uint8_t buf[1] = {0};

	buf[0] = LED_OFF;
	ret = libsoc_i2c_write(i2c, buf, 1);
	if (ret == EXIT_FAILURE)
		perror("I2C write failed");

	return ret;
}

int pioneer600_buzzer_on(i2c *i2c)
{
	uint8_t ret;
	uint8_t buf[1] = {0};

	buf[0] = BUZZER_ON;
	ret = libsoc_i2c_write(i2c, buf, 1);
	if (ret == EXIT_FAILURE)
		perror("I2C write failed");

	return ret;
}

int pioneer600_buzzer_off(i2c *i2c)
{
	uint8_t ret;
	uint8_t buf[1] = {0};

	buf[0] = BUZZER_OFF;
	ret = libsoc_i2c_write(i2c, buf, 1);
	if (ret == EXIT_FAILURE)
		perror("I2C write failed");

	return ret;
}

void pioneer600_read_joystick(i2c *i2c)
{
	uint8_t ret;
	uint8_t buf[1] = {0};

	ret = libsoc_i2c_read(i2c, buf, 1);
	if (ret == EXIT_FAILURE)
		perror("I2C read failed");
	else {
		if (buf[0] & 0x0F)
			printf("No keys pressed\n");

		if (!(buf[0] & 0x01))
			printf("Key A pressed\n");

		if (!(buf[0] & 0x02))
			printf("Key B pressed\n");

		if (!(buf[0] & 0x04))
			printf("Key C pressed\n");

		if (!(buf[0] & 0x08))
			printf("Key D pressed\n");
	}

	return;
}

int main(void)
{
	i2c *i2c_pcf8574;
	uint8_t led_status = LED_OFF;
	uint8_t buzzer_status = BUZZER_OFF;
	uint32_t input;
	uint8_t ret = 0;

	i2c_pcf8574 = pcf8574_init(0, PCF8574_ADDRESS);
	if (i2c_pcf8574 == NULL)
		return EXIT_FAILURE;

	while (true) {
		printf("\nEnter choice: 1. Joy stick status 2. Led On/Off 3. Buzzer On/Off 4. Exit\n");
		scanf("%d", &input);
		switch (input) {
		case 1:
			pioneer600_read_joystick(i2c_pcf8574);
			break;
		case 2:
			if (led_status == LED_OFF) {
				ret = pioneer600_led2_on(i2c_pcf8574);
				if (ret == EXIT_FAILURE)
					printf("Led turn on failed\n");
				else {
					led_status = LED_ON;
					printf("Led On\n");
				}
			} else {
				ret = pioneer600_led2_off(i2c_pcf8574);
				if (ret == EXIT_FAILURE)
					printf("Led turn off failed\n");
				else {
					led_status = LED_OFF;
					printf("Led Off\n");
				}
			}
			break;
		case 3:
			if (buzzer_status == BUZZER_OFF) {
				ret = pioneer600_buzzer_on(i2c_pcf8574);
				if (ret == EXIT_FAILURE)
					printf("Buzzer turn on failed\n");
				else {
					buzzer_status = BUZZER_ON;
					printf("Buzzer On\n");
				}
			} else {
				ret = pioneer600_buzzer_off(i2c_pcf8574);
				if (ret == EXIT_FAILURE)
					printf("Buzzer turn off failed\n");
				else {
					buzzer_status = BUZZER_OFF;
					printf("Buzzer Off\n");
				}
			}
			break;
		case 4:
			goto exit;
			break;
		default:
			printf("Enter valid input\n");
			break;
		}
	}

exit:
	ret = pcf8574_close(i2c_pcf8574);
	if (ret == EXIT_FAILURE)
		perror("Failed to close I2C port");

	return 0;
}
