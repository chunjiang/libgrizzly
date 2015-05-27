/*
 ============================================================================
 Name        : libgrizzly.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include "libgrizzly.h"

int is_grizzly(libusb_device *dev) {
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		printf("Failed to get description\n");
		return -1;
	}
	return (desc.idVendor == IDVENDOR) & (desc.idProduct == IDPRODUCT);
}

int grizzly_send_bytes(libusb_device_handle* dev, unsigned char* cmd) {
	return libusb_control_transfer(dev, 0x21, 0x09, 0x0300, 0, cmd, 16, DEFAULT_TIMEOUT);
}

int grizzly_exchange_bytes(libusb_device_handle* dev, unsigned char* cmd, unsigned char* rtn) {
	int err = grizzly_send_bytes(dev, cmd);
	if (err < 0) {
		return err;
	}

	uint16_t num_bytes = cmd[1] & 0x7f;
	unsigned char temp[num_bytes + 1];
	err = libusb_control_transfer(dev, 0xa1, 0x01, 0x0301, 0, &temp, num_bytes + 1, DEFAULT_TIMEOUT);

	for (int i = 0; i < num_bytes; i++) {
		rtn[i] = temp[i + 1];
	}
	if (err < 0) {
		return err;
	}
	return num_bytes;
}

ssize_t get_all_grizzlies(libusb_context* ctx, libusb_device_handle** handles) {
	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(ctx, &list);
	printf("%d Devices\n", (int)cnt);

	ssize_t grizzly_count = 0, i;
	for (i = 0; i < cnt; ++i) {
		if (is_grizzly(list[i])) {
			libusb_device_handle* current_handle;
			int err = libusb_open(list[i], &current_handle);
			if (err) {
				printf("Failed to open grizzly because %d\n", err);
				return -1;
			}

			err = 0;
			err = libusb_detach_kernel_driver(current_handle, 0);
			if (err) {
				printf("Failed to detach kernel driver\n");
			}

			printf("Successfully opened and detached\n");
			handles[grizzly_count++] = current_handle;
		}
	}
	printf("%d Grizzlies detected\n", (int)grizzly_count);
	libusb_free_device_list(list, 1);
	return grizzly_count;
}

libusb_device_handle* find_grizzly(libusb_context* ctx, unsigned char addr) {
	libusb_device_handle* handles[10];
	ssize_t num = get_all_grizzlies(ctx, handles);

	unsigned char rtn_buffer[16];
	for (int i = 0; i < num; i++) {
		grizzly_exchange_bytes(handles[i], (unsigned char*)COMMAND_GET_ADDR, rtn_buffer);
		if (addr == rtn_buffer[0] >> 1) {
			return handles[i];
		} else {
			libusb_attach_kernel_driver(handles[i], 0);
			libusb_close(handles[i]);
		}
	}
	return NULL;
}

void grizzly_init() {

}

void grizzly_set_registers(libusb_device_handle* dev, unsigned char addr, unsigned char* data, int num) {
	if (num > 14) {
		printf("Cannot write more than 14 bytes at a time.\n");
	} else {
		unsigned char buffer[16];
		for (int i = 0; i < 16; i++) {
			buffer[i] = 0;
		}
		buffer[0] = addr;
		buffer[1] = (unsigned char)num | 0x80;
		for (int i = 0; i < num; i++) {
			buffer[i + 2] = data[i];
		}
		int cnt = grizzly_send_bytes(dev, buffer);
		//printf("%d bytes written\n", cnt);
	}
}

void grizzly_read_registers(libusb_device_handle* dev, unsigned char addr, unsigned char* data, int num) {
	if (num > 127) {
		printf("Cannot read more than 127 bytes at a time.\n");
	} else {
		unsigned char buffer[16];
		buffer[0] = addr;
		buffer[1] = (unsigned char)num;

		for (int i = 0; i < num; i++) {
			buffer[i + 2] = (unsigned char)0;
		}

		grizzly_exchange_bytes(dev, buffer, data);
	}
}
