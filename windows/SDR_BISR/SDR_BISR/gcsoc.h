#define DLL_API __declspec(dllexport)
#include <stdio.h>
#include "ctypes.h"

DLL_API int GCSOC_HW_check();
DLL_API int GCSOC_AHBRAM_init(FILE *sfbin);
DLL_API int *GCSOC_MBIST_read(int addr, int len, int *buf);
DLL_API int GCSOC_REG_read(int addr);
DLL_API void GCSOC_REG_write(int addr, int data);
DLL_API int GCSOC_CONFIG_set(FILE *fp);
DLL_API int GCSOC_FLAG_read();
