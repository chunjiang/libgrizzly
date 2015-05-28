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

	// Assume only 1 grizzly
	libusb_device_handle* device = find_grizzly(ctx, 0x0f);

	if (device == NULL) {
		printf("Could not find grizzly\n");
		return -1;
	}

	unsigned char read_count[4] = {0, 0, 0, 0};
	unsigned char write_count[4] = {0, 0, 0, 0};
	grizzly_write_registers(device, 0x80, (unsigned char*)"\x00\x00", 2);
	grizzly_write_registers(device, 0x01, (unsigned char*)"\x03\x00\x00\x00\x00\x00\x00\x00", 8);
	int s = 0;
	while (1) {
/*
		grizzly_read_registers(device, 0x20, read_count, 4);
		int cycles = 0;
		for (int i = 0; i < 4; i++) {
			cycles |= (int)(read_count[i] << (8 * i));
		}
		printf("Time %d ", cycles);

		int cnt = 0;
		if (cycles > 5000) {
			cnt = grizzly_send_bytes(device, "\x20\x84\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");
			//grizzly_set_registers(device, 0x20, write_count, 4);
		}

		unsigned char mode;
		grizzly_read_registers(device, 0x09, &mode, 1);
		printf("%d\n", mode);
		unsigned char zero_mode = 0;
		unsigned char non_mode = 4;
		if (cycles > 5000) {
			grizzly_set_registers(device, 0x01, &zero_mode, 1);
		} else {
			grizzly_set_registers(device, 0x01, &non_mode, 1);
		}

		grizzly_set_registers(device, 0x08, &zero_mode, 1);
*/
		// Set 0 timeout

		// Set drive mode and 25% pwm
		grizzly_set_target(device, -s);
		unsigned char buf[1];
		grizzly_read_registers(device, 0x09, buf, 1);
		int mode = buf[0];
		int speed = grizzly_read_as_int(device, 0x0c, 4);
		printf("Mode: %d, Speed %d, %d\n", mode, speed / 65536, s);
		s = (s + 1) % 101;
		sleep(1);
	}
	return 0;
}
