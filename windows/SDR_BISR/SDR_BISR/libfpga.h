#ifndef LIBFPGA_H
#define LIBFPGA_H



#include "ctypes.h"
#ifdef __cplusplus
extern "C"
{
#endif
UNSG32 fpga_drv_io_init();
void   fpga_drv_io_free();
void   fpga_drv_jtag_reset();
UNSG32 fpga_drv_io_rd32(UNSG32 addr);
void   fpga_drv_io_wr32(UNSG32 addr, UNSG32 data);
void fpga_drv_io_wr64(UNSG32 addr, UNSG64 data);
UNSG64 fpga_drv_io_rd64(UNSG32 addr);
void fpga_drv_ddr_wrBuf(UNSG32 addr, UNSG32 cnt, UNSG32 *pBuf);
void fpga_drv_ddr_rdBuf(UNSG32 addr, UNSG32 cnt, UNSG32 *pBuf);

//void phy_init();
//void mc_start();
#ifdef __cplusplus
}
#endif
#endif