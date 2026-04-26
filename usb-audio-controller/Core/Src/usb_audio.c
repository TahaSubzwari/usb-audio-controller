#include "usb_audio.h"
#include "usbd_hid.h"

uint8_t USBD_HID_SendVolumeReport(USBD_HandleTypeDef *pdev, int8_t delta)
{
    uint8_t report[2];
    report[0] = 0x01;	// Report ID

    if (delta > 0)
        report[1] = 0x01;	// Volume Increment bit
    else if (delta < 0)
        report[1] = 0x02;	// Volume Decrement bit
    else
        report[1] = 0x00;	// No change

    return USBD_HID_SendReport(pdev, report, 2);
}

