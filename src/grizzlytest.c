/*
 * grizzlytest.c
 *
 *  Created on: May 26, 2015
 *      Author: felix
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "libgrizzly.h"

int main(void) {
	libusb_context* ctx = NULL;
	int result = libusb_init(&ctx);
	if (0 > result)
	{
		fprintf(stderr, "libusb_init() failed with %d.\n", result);
		return -1;
	}

	libusb_device_handle* device = grizzly_init(ctx, 0x0f);
	if (device == NULL) {
		return -1;
	}

	if (!grizzly_enable(device)) {
		printf("Failed to enable.\n");
	}
	grizzly_set_mode(device, CMODE_NO_PID, DMODE_DRIVE_COAST);
	grizzly_set_target(device, 0.);

	grizzly_limit_acceleration(device, 20);
	grizzly_limit_current(device, 1);

	grizzly_init_pid(device, 5, 3, 1);
	float pidconstants[3];
	grizzly_read_pid_constants(device, pidconstants);
	printf("kP: %d, kI: %d, kD: %d\n", (int)pidconstants[0], (int)pidconstants[1], (int)pidconstants[2]);

	int s = 0;
	while (1) {
		// Set drive mode and 25% pwm
		grizzly_set_target(device, -s);

		unsigned char mode = grizzly_read_single_register(device, 0x09);
		float current = grizzly_read_current(device);
		int ticks = grizzly_read_encoder(device);
		int speed = grizzly_read_as_int(device, 0x0c, 4) / 65536;

		printf("Mode: %d, Speed: %d, Current: %f, Count: %d\n", mode, speed, current, ticks);
		s = (s + 1) % 101;
		sleep(1);
	}
	return 0;
}
