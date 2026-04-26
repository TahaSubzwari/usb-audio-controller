#ifndef USB_AUDIO_H
#define USB_AUDIO_H

#include "usbd_hid.h"

uint8_t USBD_HID_SendVolumeReport(USBD_HandleTypeDef *pdev, int8_t delta);

#endif
