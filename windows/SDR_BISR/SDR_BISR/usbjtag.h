//////////////////////////////////////////////////////////////////////

#if !defined(__USBJTAG_H__)
#define __USBJTAG_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>
#include "ctypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define USBDEV_SHARED_VENDOR    0x04B4
#define USBDEV_SHARED_PRODUCT   0x01F1

#define RST_2_IDLE 				0xFE
#define READ_ID 				0xFD
#define	BURSTW_CMD_DATA			0xFC
#define	BURSTW_DATA				0xFB
#define BURSTW_DATA_RSP			0xFA
#define BURSTW_CMD_DATA_RSP		0xF9
#define BURSTR_CMD_DATA			0xF8
#define BURSTR_DATA				0xF7

#define TD_POLL_CMD				0x01
#define TD_POLL_DATA			0x06
#define TD_POLL_READ			0x02

UNSG32 OpenUsbJtag();
UNSG32 CloseUsbJtag();
UNSG32 SetUsbJtagMode(UNSG8 mode);
UNSG32 usbjtag_get_status(UNSG8 mode);
UNSG32 usbjtag_read_id();
SIGN32 usbjtag_write(UNSG32 addr, UNSG32 *data, UNSG32 count);
SIGN32 usbjtag_read(UNSG32 addr, UNSG32 * data, UNSG32 count);
UNSG32 single_write32(UNSG32 addr, UNSG32 wrData);
UNSG32 single_read32(UNSG32 addr);
UNSG32 burst_write(UNSG32 addr, UNSG32 *data, UNSG32 count);
UNSG32 burst_read(UNSG32 addr, UNSG32 *data, UNSG32 count);
UNSG32 UsbJtagTest();
SIGN32 usb_error_handle();

UNSG32 usbjtag_write4dvp(UNSG8 *data, UNSG32 width, UNSG32 height);
#ifdef __cplusplus
}
#endif

#endif // !defined(AFX_USBOP_H__1160A1C7_6AF6_4B15_A091_316DB6090F4E__INCLUDED_)
