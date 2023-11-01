#ifndef _USB_COM_H
#define _USB_COM_H

void send_data_to_host(usbd_device *usbd_dev, const char *data, int len);
usbd_device *usb_setup(void);
void usb_disconnect(usbd_device *usbd_dev);

#endif