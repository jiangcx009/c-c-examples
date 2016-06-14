#ifndef __BISR_H__
#define __BISR_H__

#include "ctypes.h"
#include <stdio.h>
#include "libfpga.h"
#include "usbjtag.h"
//#include "lusb0_usb.h"

#define DUMP_LOG
//#define SDR_DBG //output info log for debug
#define FT_TEST
#define VERSION 1
//#define CORNER_CASE  //generate corner case err
//#define WLR_ONLY
//#define ReRepair
#define _JTAG_

#define MAX_BUFF_SIZE 2048*60/4

//#define	NLR_SIZE 1024*16384
#define NLR_SIZE 1024       //Only store the err normal resource
#if (VERSION == 2)
#define WLR_SIZE 128*1024	//Store all WL Redundancy resource
#define BLR_SIZE 4*2048*8	//Store all BL Redundancy resource
#define BLR_W    2048*8
#define WLR_H    1024
#else
/* VERSION == 1
 * If one of WL or BL Redundancy unit is error, 
 * then the hole WL or BL Redundancy Line
 * can not be used for repair
 */
#define WLR_SIZE 128	//Store all WL Redundancy resource
#define BLR_SIZE 96  	//Store all BL Redundancy resource
#define CROR_SIZE 1
#endif

#define BIGMAT_W 2048
#define SMALLMAT_W 688

#define MAX_ERR_FOR_BL 100

//Register define
#define FUSE_REGADD 0x10   
#define SDRC_REGADD 0x88600000
#define SDRC_REGOFFSET 0x40000


#ifdef _JTAG_
#define WRITESDR(a, b)  single_write32((a), (b)) 
#define READSDR(a)  single_read32((a))
#else
#define WRITESDR(a, b)  a/*printf("write sdr addr:%x, dat:%x\n", a, b);*/
#define READSDR(a)  a/*printf("read sdr addr:%x\n", a);*/
#endif


enum PRN_MACRO {
	PRN_BIN = 0,
	PRN_DBG = 1,
	PRN_INFO = 2,
	PRN_WARN,
	PRN_ERR
};

#define PRN_LEVEL  PRN_INFO

#ifdef FT_TEST
#define PRN_ENB 0
#else
#define PRN_ENB 1
#endif

#if (PRN_ENB != 1)
#ifdef DUMP_LOG
#define PRN_LOG(fp, level, format, ...)  if (level >= PRN_LEVEL) \
	fprintf(fp, (format), __VA_ARGS__);
#else
#define PRN_LOG(fp, level, format, ...) if (level >= PRN_LEVEL) \
	fprintf(stdout, (format), __VA_ARGS__);
#endif
#else
#define PRN_LOG //
#endif

enum SDR_ERR_TYPE {
	NORMAL_ERR = 1,
	WORDLINE_ERR,
	BITLINE_ERR,
	CROSS_ERR
};

enum SDR_PROC_STAT {
	START_PROC_STAT = 0,
	HEAD_PROC_STAT,
	CTX_PROC_STAT,
	FINISH_PROC_STAT,
	ERR_PROC_STAT
};

enum REPAIR_STAT {
	START_REPAIR_STAT = 0,
	BL_REPAIR_STAT,
	BL_FINISH_STAT,
	BL_ERR_STAT,
	BL_TO_WL_STAT,
	WL_REPAIR_STAT,
	WL_FINISH_STAT,
	CROSS_REPAIR_STAT,
	FINISH_REPAIR_STAT,
	ERR_REPAIR_STAT
};

enum SORT_TYPE {
	NORMAL_SORT = 0
};

enum ERR_TYPE {
	SUCCESS = 0,
	PROC_ERR = 1,
	INFO_ERR,
	REPAIR_ERR
};

//The information of normal resource which redundancy has repaired
typedef struct RED_REP_INFO
{
	UNSG32		rowaddr;   
	UNSG32		coladdr;
	UNSG32		sMataddr;
}RED_REP_INFO_t;

typedef struct ERR_RESOURCE_INFO
{
	UNSG8		bl_repair_flag; //BL err is more than threshold
	UNSG8		used;  //whether for WL or BL Redundancy is used or not
	UNSG8		last_used; 
	//UNSG8		type;  //WL,BL,Normal error
	UNSG8		err;   //the resource is error or not when type is WL and BL
	UNSG8		sMAT;   //Err belong to which small MAT in big MAT 0~2
	UNSG8		bMAT;	//Err belong to which big MAT 0~7
	UNSG32		rowAdr; //WL address
	UNSG32		colAdr; //BL address
	RED_REP_INFO_t		redundancy_resource; //
}ERR_RESOURCE_INFO_t, *ERR_RESOURCE_INFO_p;



typedef struct SDR_BISR
{
	UNSG8			type;   //WL,BL,Normal error
	UNSG8			bl_num_flag; //BL err is more than threshold
	UNSG32			SDRID;  //which SDRAM id
	UNSG32			normal_err_count;

	ERR_RESOURCE_INFO_p		normalResource;
	ERR_RESOURCE_INFO_p		wordLineResource;
	ERR_RESOURCE_INFO_p		bitLineResource;
	ERR_RESOURCE_INFO_p		crossResource;

	ERR_RESOURCE_INFO_p		p_normalResource;
	ERR_RESOURCE_INFO_p		p_wordLineResource;
	ERR_RESOURCE_INFO_p		p_bitLineResource;
	ERR_RESOURCE_INFO_p		p_crossResource;
}SDR_BISR_t, *SDR_BISR_P;

UNSG32 SDR_BISR_Entry(UNSG8 *argv[]);
UNSG32 SDR_BISR_DBG(UNSG8 *argv[]);
SDR_BISR_t*	sdr_open(SDR_BISR_t* sdr_bisr, UNSG8 *argv[]);
SDR_BISR_P	sdr_release(SDR_BISR_P	sdr_info);
UNSG32 SDR_PrePrepair(SDR_BISR_P bisr_info);
void Do_Repair(SDR_BISR_P bisr_info);
UNSG32 Merge(UNSG8 *argv[], UNSG32 cnt, UNSG8 *outfile[]);
void Read_CHIPID();
void Write_CHIPID(UNSG32 addr, UNSG32 data);
UNSG32 BL_Repair_check(UNSG32 addr, UNSG32 sdr_id, UNSG32 rp_coladdr);
UNSG32 WL_Repair_check(UNSG32 addr, UNSG32 sdr_id, UNSG32 rp_rowaddr);
UNSG32 SDR_BISR_Check(SDR_BISR_P bisr_info);

void Init_SDR();

#endif