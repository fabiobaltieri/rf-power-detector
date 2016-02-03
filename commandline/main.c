#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>
#include <math.h>

#include <usb.h>

#include "opendevice.h"

#include "../firmware/requests.h"

#define PRODUCT "rf-power-detector"

static int get_uint16(usb_dev_handle *handle, int req)
{
	int ret;
	uint16_t data;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      req,
			      0, 0, (char *)&data, sizeof(data), 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}

	data = le16toh(data);

	return data;
}

static void send_reset(usb_dev_handle *handle)
{
	int ret;

	ret = usb_control_msg(handle,
			      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
			      USB_ENDPOINT_IN,
			      CUSTOM_RQ_RESET,
			      0, 0, NULL, 0, 1000);

	if (ret < 0) {
		printf("usb_control_msg: %s\n", usb_strerror());
		exit(1);
	}
}

static void usage(char *name)
{
	fprintf(stderr, "Usage: %s -h\n", name);
	fprintf(stderr, "       %s -R\n", name);
	fprintf(stderr, "       %s [options]\n", name);
	fprintf(stderr, "options:\n"
			"  -h         this help\n"
			"  -R         reset device\n"
			"  -p pad     external attenuator value in dB\n"
			"  -d msec    delay time between read in milliseconds\n"
			);
	exit(1);
}

int main(int argc, char **argv)
{
	usb_dev_handle *handle = NULL;
	int opt;
	int reset = 0;
	int raw_data;
	float slope = -22;
	float intercept = 15;
	float vout;
	float dbm_in;
	float dbm;
	float pad = 0;
	float mw;
	int fixed_pad_db;
	int delay = 100;

	usb_init();

	while ((opt = getopt(argc, argv, "hRp:d:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			break;
		case 'R':
			reset = 1;
			break;
		case 'p':
			pad = strtof(optarg, NULL);
			break;
		case 'd':
			delay = strtol(optarg, NULL, 0);
			break;
		default:
			usage(argv[0]);
		}
	}

	if (usbOpenDevice(&handle, 0, NULL, 0, PRODUCT, NULL, NULL, NULL) != 0) {
		fprintf(stderr, "error: could not find USB device \"%s\"\n", PRODUCT);
		exit(1);
	}

	if (reset) {
		send_reset(handle);
		return 0;
	}

	fixed_pad_db = get_uint16(handle, CUSTOM_RQ_GET_PAD_DB);
	printf("fixed_pad: %ddB, pad: %.1fdB\n", fixed_pad_db, pad);
	for (;;) {
		raw_data = get_uint16(handle, CUSTOM_RQ_GET_VALUE);
		vout = 2560.0 / 1024 * (float)raw_data;
		dbm_in = vout / slope + intercept;
		dbm = dbm_in + fixed_pad_db + pad;
		mw = powf(10, dbm/10);
		printf("power: %7.2fdBm, %9.2f mW", dbm, mw);
		if (dbm_in < -50)
			printf(" [no signal]");
		else if (dbm_in > -5)
			printf(" [overloaded]");
		printf("\n");
		usleep(delay * 1000);
	}

	usb_close(handle);

	return 0;
}
