#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <libsoc_pwm.h>

/*
 * libsoc PWM API Documentation: https://jackmitch.github.io/libsoc/c/pwm/
 */

int main(void)
{
	pwm* vf_pwm = NULL;
	uint32_t ret = EXIT_SUCCESS;
	uint32_t pwm_period = 0;
	uint32_t pwm_number = 0;
	uint32_t pwm_chip_number = 0;
	uint32_t pwm_duty_cycle = 0;

	printf("Enter PWM chip number:\t");
	scanf("%d", &pwm_chip_number);
	printf("Enter PWM number:\t");
	scanf("%d", &pwm_number);
	vf_pwm = libsoc_pwm_request(pwm_chip_number, pwm_number, LS_GREEDY);
	if (vf_pwm == NULL) {
		perror("PWM request failed");
		return EXIT_FAILURE;
	}

	printf("Enter PWM Period:\t");
	scanf("%d", &pwm_period);
	ret = libsoc_pwm_set_period(vf_pwm, pwm_period);
	if (ret == EXIT_FAILURE) {
		perror("PWM set period failed");
		goto exit_failure;
	}

	printf("Enter PWM Duty Cycle:\t");
	scanf("%d", &pwm_duty_cycle);
	ret = libsoc_pwm_set_duty_cycle(vf_pwm, pwm_duty_cycle);
	if (ret == EXIT_FAILURE) {
		perror("PWM set duty cycle failed");
		goto exit_failure;
	}

	ret = libsoc_pwm_set_enabled(vf_pwm, ENABLED);
	if (ret == EXIT_FAILURE) {
		perror("PWM enable failed");
		goto exit_failure;
	}

	printf("PWM will be running for 10 seconds\n");
	sleep(10);

	printf("Disabling PWM\n");
	ret = libsoc_pwm_set_enabled(vf_pwm, DISABLED);
	if (ret == EXIT_FAILURE)
		perror("PWM disable failed");

exit_failure:
	libsoc_pwm_free(vf_pwm);

	return ret;
}
