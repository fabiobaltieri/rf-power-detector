#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <usb.h>

#include "../firmware/requests.h"

#include "librfpower.h"

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

int rfpower_get_pad(usb_dev_handle *handle)
{
	return get_uint16(handle, CUSTOM_RQ_GET_PAD_DB);
}

float rfpower_get_dbm(usb_dev_handle *handle)
{
        int raw_data;
        float slope = -22;
        float intercept = 15;
        float vout;
        float dbm;

	raw_data = get_uint16(handle, CUSTOM_RQ_GET_VALUE);
	vout = 2560.0 / 1024 * (float)raw_data;
	dbm = vout / slope + intercept;

	return dbm;
}

void rfpower_send_reset(usb_dev_handle *handle)
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
