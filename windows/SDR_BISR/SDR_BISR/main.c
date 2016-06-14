#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bisr.h"

//#define GEN_TEST
FILE			*fp_dumper;
FILE			*fp_FTRes;
FILE			*fp_FTRes_ex;

#ifdef GEN_TEST
//普通资源：不同行，不同列，修补资源正常
static const UNSG32 testpattern1[] = 
{
	0xbeef1000, 0xbeef1100, 0x12312142, 0x12358121,
	0xbeef1200, 0xbeef1300, 0xbeef1400, 0xdeaddead
};

//普通资源：同行同小MAT，不同列，修补资源正常
//(row, col): (0x123, 0x10), (0x2A0, 0x10)
static const UNSG32 testpattern2[] =
{
	0xbeef1000, 0xbeef1100, 0x9190, 0x15010,
	0xbeef1200, 0xbeef1300, 0xbeef1400, 0xdeaddead
};

//普通资源：同列，不同行，修补资源正常
//(row, col): (0x123, 0x10), (0x10, 0x5), (0x200, 0x24), (0x19, 0x11), (0x123, 0x30)
static const UNSG32 testpattern3[] =
{
	0xbeef1000, 0xbeef1100, 0x9190, 0x805,
	0x10024, 0xcf1, 0x91B0,
	0xbeef1200, 0xbeef1300, 0xbeef1400, 0xdeaddead
};

//普通资源：混合，修补资源
//(row, col)
//sMAT0: (0x123, 0x10), (0x10, 0x5), (0xdeadbeef, 0x219, 0xc), (0x200, 0x24), (0x19, 0x71), (0x123, 0x30)
//       (0x2A0, 0x10), 
//sMAT1: (0x2B0, 0x0), (0x54F, 0x49), (0x2B0, 0x17), (0x2B1, 0x17)
//sMAT23:(0x3FFF, 0x73), (0x3DD0, 0x12), (0x3FFF, 0x7F)
//WL Redundancy: (0x10, 0x0), (0x11, 0x60)
//BL Redundancy: (0x123, 0x1), (0x3D50, 0x0), (0x3D50, 0x1), (0x3D51, 0x1), (0x3D52, 0x1)
//Cross: (0x1, 0x1)
static const UNSG32 testpattern4[] =
{
	0xbeef1000, 0xbeef1100, 0x9190, 0x805, 0xdeadbeef, 0x10C8C, 0x10024, 0xCF1, 0x91B0, 0x15010,
	0x15800, 0x2A7C9, 0x15817, 0x15897,
	0x1FFFF3, 0x1EEE92, 0x1FFFFF,
	0xbeef1200, 0x800, 0x8E0,
	0xbeef1300, 0x9181, 0x1EA800, 0x1EA801, 0x1EA881, 0x1EA901, 
	0xbeef1400, 0x81, 0xdeaddead
};
#endif

void main(UNSG32 argc, UNSG8* argv[])
{
	UNSG8 *outfile[6] = {"ram0.bin", "ram1.bin",
		"ram2.bin", "ram3.bin", "ram4.bin", "ram5.bin"};
	UNSG8 *infile[8] = {"test_err_00_0", "test_err_00_1", 
		"test_err_55_0", "test_err_55_1", "test_err_aa_0",
		"test_err_aa_1", "test_err_ff_0", "test_err_ff_1"};
	//UNSG8	*infile[1] = {"test_err_00_0"};
	UNSG32 i, stat;

#ifdef _JTAG_
	//init jtag
	i = fpga_drv_io_init();
	if(i == 1)
	{
		//printf("JTAG initialization panic!\n");
		return -1;
	}
	
	fpga_drv_jtag_reset();

	//Read_CHIPID();
	
	//Write_CHIPID(0x4, 1);

	Init_SDR();

	diag_dbg();
#endif

#ifdef GEN_TEST
	FILE *fp = fopen("ft_res4.bin", "wb");
	fwrite(testpattern4, sizeof(testpattern4), 1, fp);
	fclose(fp);
	return;
#endif

#if 0
	Merge(infile, 1, outfile);

	for (i = 0; i < 6; i++)
 		SDR_BISR_Entry(&outfile[i]); 
#else

	stat = Merge(infile, 8, outfile);  //stat: PROC_ERR, SUCCESS
	//stat = SDR_BISR_DBG(outfile);
	//return;
	stat = SDR_BISR_Entry(outfile);  //stat: SUCCESS, PROC_ERR, INFO_ERR, REPAIR_ERR
	//Write_CHIPID(0x7, 0x2);
	//SDR_BISR_Entry(argv);
#endif

	return ;
}