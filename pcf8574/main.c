#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "pcf8574.h"

/* See the Pioneer 600 Schematic */
static uint8_t LED_ON		= 0xEF;
static uint8_t LED_OFF		= 0xFF;
static uint8_t BUZZER_ON	= 0x7F;
static uint8_t BUZZER_OFF	= 0xFF;

void pioneer600_led2_on(i2c *i2c)
{
	pcf8574_port_write(i2c, LED_ON);
}

void pioneer600_led2_off(i2c *i2c)
{
	pcf8574_port_write(i2c, LED_OFF);
}

void pioneer600_buzzer_on(i2c *i2c)
{
	pcf8574_port_write(i2c, BUZZER_ON);
}

void pioneer600_buzzer_off(i2c *i2c)
{
	pcf8574_port_write(i2c, BUZZER_OFF);
}

void pioneer600_read_joystick(i2c *i2c)
{
	uint8_t port;
	port = pcf8574_port_read(i2c);

	if (!(port & 0x01))
		printf("Key A pressed\n");
	else if (!(port & 0x02))
		printf("Key B pressed\n");
	else if (!(port & 0x04))
		printf("Key C pressed\n");
	else if (!(port & 0x08))
		printf("Key D pressed\n");
	else if (port & 0x0F)
		printf("No keys pressed\n");
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
				pioneer600_led2_on(i2c_pcf8574);
				led_status = LED_ON;
			} else {
				pioneer600_led2_off(i2c_pcf8574);
				led_status = LED_OFF;
			}
			break;
		case 3:
			if (buzzer_status == BUZZER_OFF) {
				pioneer600_buzzer_on(i2c_pcf8574);
				buzzer_status = BUZZER_ON;
			} else {
				pioneer600_buzzer_off(i2c_pcf8574);
				buzzer_status = BUZZER_OFF;
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
	ret = pcf8574_free(i2c_pcf8574);
	if (ret == EXIT_FAILURE)
		perror("Failed to close I2C port");

	return 0;
}
