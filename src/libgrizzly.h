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

void print_vendor_product(libusb_device *dev);
int is_grizzly(libusb_device *dev);
int grizzly_send_bytes(libusb_device_handle* dev, unsigned char* cmd);
int grizzly_exchange_bytes(libusb_device_handle* dev, unsigned char* cmd, unsigned char* rtn);
libusb_device_handle* find_grizzly(libusb_context* ctx, unsigned char addr);
ssize_t get_all_grizzlies(libusb_context* ctx, libusb_device_handle** handles);
void grizzly_set_registers(libusb_device_handle* dev, unsigned char addr, unsigned char* data, int num);
void grizzly_read_registers(libusb_device_handle* dev, unsigned char addr, unsigned char* data, int num);


#endif /* LIBGRIZZLY_H_ */
