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
	err = libusb_control_transfer(dev, 0xa1, 0x01, 0x0301, 0, temp, num_bytes + 1, DEFAULT_TIMEOUT);

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

libusb_device_handle* grizzly_init(libusb_context* ctx, unsigned char device_addr) {
	libusb_device_handle* device = find_grizzly(ctx, device_addr);
	if (device == NULL) {
		printf("Could not find grizzly\n");
		return NULL;
	}
	grizzly_write_as_int(device, ADDR_ENABLEUSB, 1, 1);
	return device;
}

void grizzly_write_registers(libusb_device_handle* dev, unsigned char addr, unsigned char* data, int num) {
	if (num > 14) {
		printf("Cannot write more than 14 bytes at a time.\n");
	} else {
		unsigned char buffer[16];
		for (int i = 2; i < 16; i++) {
			buffer[i] = 0;
		}
		buffer[0] = addr;
		buffer[1] = (unsigned char)num | 0x80;
		for (int i = 0; i < num; i++) {
			buffer[i + 2] = data[i];
		}
		grizzly_send_bytes(dev, buffer);
		//printf("%d bytes written\n", cnt);
	}
}

void grizzly_write_single_register(libusb_device_handle* dev, unsigned char addr, unsigned char data) {
	grizzly_write_registers(dev, addr, &data, 1);
}

void grizzly_read_registers(libusb_device_handle* dev, unsigned char addr, unsigned char* data, int num) {
	if (num > 127) {
		printf("Cannot read more than 127 bytes at a time.\n");
	} else {
		unsigned char buffer[16];
		for (int i = 2; i < 16; i++) {
			buffer[i] = 0;
		}
		buffer[0] = addr;
		buffer[1] = (unsigned char)(num & 0xff);

		for (int i = 0; i < num; i++) {
			buffer[i + 2] = (unsigned char)0;
		}

		grizzly_exchange_bytes(dev, buffer, data);
	}
}

unsigned char grizzly_read_single_register(libusb_device_handle* dev, unsigned char addr) {
	unsigned char rtn;
	grizzly_read_registers(dev, addr, &rtn, 1);
	return rtn;
}

int grizzly_read_as_int(libusb_device_handle* dev, unsigned char addr, int num) {
	if (num > 4) {
		printf ("Cannot read int greater than 4 bytes. Truncating to 4.\n");
		num = 4;
	}
	unsigned char buf[num];
	grizzly_read_registers(dev, addr, buf, num);

	int rtn = 0;
	for (int i = 0; i < num; i++) {
		rtn |= (buf[i] << (8 * i));
	}
	return rtn;
}

void grizzly_write_as_int(libusb_device_handle* dev, unsigned char addr, int val, int num) {
	unsigned char buf[num];
	for (int i = 0; i < num; i++) {
		buf[i] = (val >> (8 * i)) & 0xff;
	}

	grizzly_write_registers(dev, addr, buf, num);
}

void grizzly_set_target(libusb_device_handle* dev, float setpoint) {
	int fixed_setpoint = (int)(setpoint * 65536);
	unsigned char buf[5];
	for (int i = 0; i < 5; i++) {
		buf[i] = (fixed_setpoint >> (8 * i)) & 0xff;
	}
	grizzly_write_registers(dev, ADDR_SPEED, buf, 5);
}

void grizzly_set_mode(libusb_device_handle* dev, char cmode, char dmode) {
	unsigned char mode = grizzly_read_single_register(dev, ADDR_MODE_RO);
	mode &= 0x01; // Get enable bit
	mode |= cmode | dmode;
	grizzly_write_single_register(dev, ADDR_MODE, mode);
	grizzly_write_single_register(dev, ADDR_UPDATE, 0);
}

float grizzly_read_current(libusb_device_handle* dev) {
	int raw_adc = grizzly_read_as_int(dev, ADDR_MOTORCURRENT, 2);
	return (5.0 / 1024.0) * (1000.0 / 66.0) * (raw_adc - 511);
}

int grizzly_read_encoder(libusb_device_handle* dev) {
	return grizzly_read_as_int(dev, ADDR_ENCODERCOUNT, 4);
}

void grizzly_write_encoder(libusb_device_handle* dev, int new_val) {
	grizzly_write_as_int(dev, ADDR_ENCODERCOUNT, new_val, 4);
}

void grizzly_limit_acceleration(libusb_device_handle* dev, int new_val) {
	if (new_val > 142) {
		printf("Acceleration limit cannot exceed 142. Clamping value to 142.\n");
		new_val = 142;
	}
	if (new_val <= 0) {
		printf("Acceleration limit must be positive. Clamping value to 0.");
		new_val = 0;
	}

	grizzly_write_as_int(dev, ADDR_ACCELLIMIT, new_val, 1);
}

void grizzly_limit_current(libusb_device_handle* dev, int new_val) {
	if (new_val <= 0) {
		printf("Current limit must be a positive number. Clamping value to 0.");
		new_val = 0;
	}

	int adc_val = (int)(new_val * (1024.0 / 5.0) * (66.0 / 1000.0));
	grizzly_write_as_int(dev, ADDR_CURRENTLIMIT, adc_val, 2);
}

void grizzly_init_pid(libusb_device_handle* dev, float kp, float ki, float kd) {
	int p = (int)(kp * 65536);
	int i = (int)(ki * 65536);
	int d = (int)(kd * 65536);

	grizzly_write_as_int(dev, ADDR_PCONSTANT, p, 4);
	grizzly_write_as_int(dev, ADDR_ICONSTANT, i, 4);
	grizzly_write_as_int(dev, ADDR_DCONSTANT, d, 4);
}

void grizzly_read_pid_constants(libusb_device_handle* dev, float* constants) {
	int p = grizzly_read_as_int(dev, ADDR_PCONSTANT, 4);
	int i = grizzly_read_as_int(dev, ADDR_ICONSTANT, 4);
	int d = grizzly_read_as_int(dev, ADDR_DCONSTANT, 4);

	constants[0] = (float)(p / 65536.0);
	constants[1] = (float)(i / 65536.0);
	constants[2] = (float)(d / 65536.0);
}

unsigned char grizzly_addr_to_id(unsigned char addr) {
	return 0x0f - addr;
}

unsigned char grizzly_id_to_addr(unsigned char id) {
	return 0x0f - id;
}

int grizzly_enable(libusb_device_handle* dev) {
	unsigned char mode = grizzly_read_single_register(dev, ADDR_MODE_RO);
	mode |= 0x01;
	grizzly_write_single_register(dev, ADDR_MODE, mode);
	grizzly_write_single_register(dev, ADDR_UPDATE, 0x00);
	mode = grizzly_read_single_register(dev, ADDR_MODE_RO);

	return mode & 0x01;
}

void grizzly_disable(libusb_device_handle* dev) {
	unsigned char mode = grizzly_read_single_register(dev, ADDR_MODE_RO);
	mode &= (0xff - 0x01);
	grizzly_write_single_register(dev, ADDR_MODE, mode);
}

void grizzly_exit(libusb_device_handle* grizzly) {
	libusb_attach_kernel_driver(grizzly, 0);
	libusb_close(grizzly);
}

int grizzly_cleanup_all(libusb_context* ctx, libusb_device_handle** all_handles, int num_devices, int error) {
	for (int i = 0; i < num_devices; i++) {
		grizzly_exit(all_handles[i]);
	}
	libusb_exit(ctx);
	return error;
}
