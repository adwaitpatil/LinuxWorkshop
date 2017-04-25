#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "libsoc_gpio.h"

/*
 * libsoc GPIO documentation: http://jackmitch.github.io/libsoc/c/gpio/#gpio
 */

volatile bool interrupt_occurred = false;
volatile bool led_status = false;

int gpio_interrupt_callback(void* arg)
{
	interrupt_occurred = true;

	return 0;
}

int main(void)
{
	uint32_t gpio_input;
	uint32_t led_output;
	uint32_t ret = 0;
	gpio *gpio_interrupt = NULL;
	gpio *gpio_led = NULL;

	printf("Enter the GPIO number to use as interrupt:\t");
	scanf("%d", &gpio_input);

	printf("Enter the GPIO number used for LED output:\t");
	scanf("%d", &led_output);

	/*
	 * It is expected that the gpio being requested is pin multiplexed correctly
	 * in the kernel device tree or board file as applicable else gpio request
	 * is bound to fail.
	 */
	gpio_interrupt = libsoc_gpio_request(gpio_input, LS_SHARED);
	if (gpio_interrupt == NULL) {
		perror("gpio request failed");
		goto exit;
	}

	gpio_led = libsoc_gpio_request(led_output, LS_SHARED);
	if (gpio_led == NULL) {
		perror("led gpio request failed");
		goto exit;
	}

	ret = libsoc_gpio_set_direction(gpio_led, OUTPUT);
	if (ret == EXIT_FAILURE) {
		perror("Failed to set gpio led direction");
		goto exit;
	}

	ret = libsoc_gpio_set_direction(gpio_interrupt, INPUT);
	if (ret == EXIT_FAILURE) {
		perror("Failed to set gpio direction");
		goto exit;
	}

	ret = libsoc_gpio_set_edge(gpio_interrupt, RISING);
	if (ret == EXIT_FAILURE) {
		perror("Failed to set gpio edge");
		goto exit;
	}

	ret = libsoc_gpio_callback_interrupt(gpio_interrupt, &gpio_interrupt_callback, NULL);
	if (ret == EXIT_FAILURE) {
		perror("Failed to set gpio callback");
		goto exit;
	}

	printf("Waiting for interrupt\n");

	while (true) {
		if (interrupt_occurred) {

			interrupt_occurred = false;

			if (!led_status) {
				led_status = true;
				libsoc_gpio_set_level(gpio_led, HIGH);
			} else {
				led_status = false;
				libsoc_gpio_set_level(gpio_led, LOW);
			}
			printf("Interrupt occurred\n");
			/* Uncomment this to exit on first interrupt */
			//goto exit;
		}
		usleep(200);
	}

exit:
	if (gpio_interrupt) {
		ret = libsoc_gpio_free(gpio_interrupt);
		if (ret == EXIT_FAILURE)
			perror("Could not free gpio");
	}

	if (gpio_led) {
		ret = libsoc_gpio_free(gpio_led);
		if (ret == EXIT_FAILURE)
			perror("Could not free led gpio");
	}

	return ret;
}
