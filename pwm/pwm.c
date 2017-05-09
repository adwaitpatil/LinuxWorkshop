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

#define SERVO_PERIOD          20000000
#define DMIN                  150000
#define DMAX                  2000000
#define STEP_SIZE             5

int main(void)
{
	pwm* vf_pwm = NULL;
	uint32_t ret = EXIT_SUCCESS;
	uint32_t pwm_number = 0;
	uint32_t pwm_chip_number = 0;
	uint32_t count = 2;
	uint32_t ms;

	printf("Enter PWM chip number:\t");
	scanf("%d", &pwm_chip_number);
	printf("Enter PWM number:\t");
	scanf("%d", &pwm_number);
	vf_pwm = libsoc_pwm_request(pwm_chip_number, pwm_number, LS_GREEDY);
	if (vf_pwm == NULL) {
		perror("PWM request failed");
		return EXIT_FAILURE;
	}

	/* As required for Tower Pro Micro Servo 9g SG90 */
	ret = libsoc_pwm_set_period(vf_pwm, SERVO_PERIOD);
	if (ret == EXIT_FAILURE) {
		perror("PWM set period failed");
		goto exit_failure;
	}

	ret = libsoc_pwm_set_enabled(vf_pwm, ENABLED);
	if (ret == EXIT_FAILURE) {
		perror("PWM enable failed");
		goto exit_failure;
	}

	while (count > 0) {
		for (ms = DMIN; ms <= DMAX;) {
			ret = libsoc_pwm_set_duty_cycle(vf_pwm, ms);
			if (ret == EXIT_FAILURE) {
				perror("PWM set duty cycle failed");
				goto exit_failure;
			}
			ms = ms + STEP_SIZE;
		}
		for (ms = DMAX; ms >= DMIN;) {
			ret = libsoc_pwm_set_duty_cycle(vf_pwm, ms);
			if (ret == EXIT_FAILURE) {
				perror("PWM set duty cycle failed");
				goto exit_failure;
			}
			ms = ms - STEP_SIZE;
		}
		count--;
	}

	printf("Disabling PWM\n");
	ret = libsoc_pwm_set_enabled(vf_pwm, DISABLED);
	if (ret == EXIT_FAILURE)
		perror("PWM disable failed");

exit_failure:
	libsoc_pwm_free(vf_pwm);

	return ret;
}
