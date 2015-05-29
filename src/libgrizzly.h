/*
 * libgrizzly.h
 *
 *  Created on: May 26, 2015
 *      Author: felix
 */
#ifndef LIBGRIZZLY_H_
#define LIBGRIZZLY_H_

#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#define IDVENDOR 0x03eb
#define IDPRODUCT 0x204f
#define COMMAND_GET_ADDR "\x9b\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

#define    ADDR_MODE                            0x01
#define    ADDR_SPEED                           0x04
#define    ADDR_UPDATE							0x08
#define	   ADDR_MODE_RO							0x09
#define    ADDR_SPEED_RO						0x0C
#define    ADDR_MOTORCURRENT                    0x10
#define    ADDR_ENCODERCOUNT                    0x20
#define    ADDR_PCONSTANT                       0x30
#define    ADDR_ICONSTANT                       0x34
#define    ADDR_DCONSTANT                       0x38
#define    ADDR_TIMEOUT                         0x80
#define    ADDR_CURRENTLIMIT                    0x82
#define    ADDR_ACCELLIMIT                      0x90
#define    ADDR_UPTIME                          0x94
#define    ADDR_ENABLEUSB                       0x9A
#define    ADDR_ADDRESSLIST                     0x9B

#define    CMODE_NO_PID                         0x02
#define    CMODE_SPEED_PID                      0x04
#define    CMODE_POSITION_PID                   0x06

#define    DMODE_DRIVE_COAST                    0x00
#define    DMODE_DRIVE_BRAKE                    0x10
#define    DMODE_BRAKE_COAST                    0x20

#define		DEFAULT_TIMEOUT						500

libusb_device_handle* grizzly_init(libusb_context* ctx, unsigned char device_addr);
void print_vendor_product(libusb_device *dev);
int is_grizzly(libusb_device *dev);
int grizzly_send_bytes(libusb_device_handle* dev, unsigned char* cmd);
int grizzly_exchange_bytes(libusb_device_handle* dev, unsigned char* cmd, unsigned char* rtn);
libusb_device_handle* find_grizzly(libusb_context* ctx, unsigned char addr);
ssize_t get_all_grizzlies(libusb_context* ctx, libusb_device_handle** handles);
void grizzly_write_registers(libusb_device_handle* dev, unsigned char addr, unsigned char* data, int num);
void grizzly_write_single_register(libusb_device_handle* dev, unsigned char addr, unsigned char data);
void grizzly_read_registers(libusb_device_handle* dev, unsigned char addr, unsigned char* data, int num);
unsigned char grizzly_read_single_register(libusb_device_handle* dev, unsigned char addr);
int grizzly_read_as_int(libusb_device_handle* dev, unsigned char addr, int num);
void grizzly_write_as_int(libusb_device_handle* dev, unsigned char addr, int val, int num);
void grizzly_set_target(libusb_device_handle* dev, float setpoint);
void grizzly_set_mode(libusb_device_handle* dev, char cmode, char dmode);
float grizzly_read_current(libusb_device_handle* dev);
int grizzly_read_encoder(libusb_device_handle* dev);
void grizzly_write_encoder(libusb_device_handle* dev, int new_val);
void grizzly_limit_acceleration(libusb_device_handle* dev, int new_val);
void grizzly_limit_current(libusb_device_handle* dev, int new_val);
void grizzly_init_pid(libusb_device_handle* dev, float kp, float ki, float kd);
void grizzly_read_pid_constants(libusb_device_handle* dev, float* constants);
unsigned char grizzly_addr_to_id(unsigned char addr);
unsigned char grizzly_id_to_addr(unsigned char id);
int grizzly_enable(libusb_device_handle* dev);
void grizzly_disable(libusb_device_handle* dev);
void grizzly_exit(libusb_device_handle* grizzly);
int grizzly_cleanup_all(libusb_context* ctx, libusb_device_handle** all_handles, int num_devices, int error);


#endif /* LIBGRIZZLY_H_ */
