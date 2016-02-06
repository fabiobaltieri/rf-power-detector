#define RFPOWER_PRODUCT "rf-power-detector"
#define RFPOWER_DBM_HIGH -5
#define RFPOWER_DBM_LOW -50

void rfpower_send_reset(usb_dev_handle *handle);
float rfpower_get_dbm(usb_dev_handle *handle);
int rfpower_get_pad(usb_dev_handle *handle);
