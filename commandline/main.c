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

#include "librfpower.h"

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

	if (usbOpenDevice(&handle, 0, NULL, 0, RFPOWER_PRODUCT, NULL, NULL, NULL) != 0) {
		fprintf(stderr, "error: could not find USB device \"%s\"\n", RFPOWER_PRODUCT);
		exit(1);
	}

	if (reset) {
		rfpower_send_reset(handle);
		return 0;
	}

	fixed_pad_db = rfpower_get_pad(handle);
	printf("fixed_pad: %ddB, pad: %.1fdB\n", fixed_pad_db, pad);
	for (;;) {
		dbm_in = rfpower_get_dbm(handle);
		dbm = dbm_in + fixed_pad_db + pad;
		mw = powf(10, dbm/10);
		printf("power: %7.2fdBm, %9.2f mW", dbm, mw);
		if (dbm_in < RFPOWER_DBM_LOW)
			printf(" [no signal]");
		else if (dbm_in > RFPOWER_DBM_HIGH)
			printf(" [overloaded]");
		printf("\n");
		usleep(delay * 1000);
	}

	usb_close(handle);

	return 0;
}
