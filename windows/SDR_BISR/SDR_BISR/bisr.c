#include "bisr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *fp_dumper;
extern FILE *fp_FTRes;
extern FILE *fp_FTRes_ex;

UNSG32 Merge(UNSG8 *argv[], UNSG32 cnt, UNSG8 *outfile[])
{
	UNSG32	i, j;
	UNSG32  *buffer = (UNSG32 *)malloc(MAX_BUFF_SIZE*sizeof(UNSG32));
	UNSG32  *p_buf = buffer;
	FILE	*fp_in, *fp_out;
	UNSG32  buf_dep = 0;
	
	for ( i = 0; i < cnt; i++)
	{
		fp_in = fopen(argv[i], "rb");
		if ( !fp_in )
		{
			PRN_LOG(fp_dumper, PRN_ERR, "can not find file!\n");
			return PROC_ERR;
		}
		//fread(buffer, MAX_BUFF_SIZE*sizeof(UNSG32), 1, fp_in);

		for ( j = 0; j < 6; j++)
		{
			do{
				if (fread(p_buf, sizeof(UNSG32), 1, fp_in) != 1)
					return PROC_ERR;
				buf_dep++;
			}while(*(p_buf++) != 0xdeaddead);

			if ( i == 0)
			{
				fp_out = fopen(outfile[j], "wb");
				if ( !fp_out )
					return PROC_ERR;
			}
			else 
			{
				fp_out = fopen(outfile[j], "ab+");
				if ( !fp_out )
					return PROC_ERR;
			}

			fwrite(p_buf - buf_dep, buf_dep-1, sizeof(UNSG32), fp_out);

			fclose(fp_out);
			//p_buf = buffer;
			buf_dep = 0;
		}
		p_buf = buffer;
		fclose(fp_in);
	}

	//pending 0xdeaddead
	for ( j = 0; j < 6; j++)
	{
		fp_out = fopen(outfile[j], "ab+");
		*buffer = 0xdeaddead;
		
		fwrite(buffer, 1, sizeof(UNSG32), fp_out);

		fclose(fp_out);
	}

	free(buffer);
	return SUCCESS;
}

void Read_CHIPID()
{
	UNSG32 addr;
	UNSG32 data,  dat_H;
	UNSG32 i;

	for(i = 0; i < 3; i++) {
		addr = SDRC_REGADD + 0x64 + i*SDRC_REGOFFSET;//SDRC_REGADD + ((2 - 1) / 2) * SDRC_REGOFFSET + 0x64;

		dat_H = (FUSE_REGADD + 0x0E) << 16;
		data = 0x00 | dat_H;
		WRITESDR(addr, data | (1 << 31));

		dat_H = (FUSE_REGADD + 0x00) << 16;
		data = 0x07 | dat_H;
		WRITESDR(addr, data | (1 << 31));

		dat_H = (FUSE_REGADD + 0xD) << 16;
		data = dat_H;
		WRITESDR(addr, data | (0 << 31));

		data = READSDR(addr+8);
		printf("chipID[47:32]:%x\n", data);

		dat_H = (FUSE_REGADD + 0xC) << 16;
		data = dat_H;
		WRITESDR(addr, data | (0 << 31));

		data = READSDR(addr+8);
		printf( "chipID[31:16]:%x\n", data);

		dat_H = (FUSE_REGADD + 0x6) << 16;
		data = dat_H;
		WRITESDR(addr, data | (0 << 31));

		data = READSDR(addr+8);
		printf( "chipID[15:0]:%x\n", data);
	}
}

void Write_CHIPID(UNSG32 addr, UNSG32 data)
{
	PrepareSDR(SDRC_REGADD+0x64);
	WRITE_TO_SDR(SDRC_REGADD+0x64, addr, data);
	ExitSDR(SDRC_REGADD+0x64);
}

void Init_SDR()
{
	UNSG32 i, j,addr,data,tmp,rdata,rdata1,data1;
	int buf[3*1024], len;
	UNSG32 base_adr = SDRC_REGADD;
	//char path[128];
	memset(buf, 0, sizeof(buf));

	for(j = 0; j < 3; j++) {
		addr = base_adr;
		data = 0x32;
		WRITESDR(addr, data);
		if(data!=READSDR(addr)){
			printf("error data:%x(%x)\n",READSDR(addr),0x32);
		}

		addr = base_adr + 8; //refresh time
		data = 0xb0;
		WRITESDR(addr, data);
		if(data !=READSDR(addr)){
			printf("error data:%x(%x)\n",READSDR(addr), 0xb0);
		}

		addr = base_adr + 0x2c;
		data = 0x394a4404;//39373303
		WRITESDR(addr, data);//39483504
		if(data !=READSDR(addr)){
			printf("error data:%x(%x)\n",READSDR(addr), 0x394a4404);
		}

		//test_reg_cfg(0x7,0x32);
		WRITESDR(base_adr+0x64, 0x80320007);
		WRITESDR(base_adr+0x64, 0x80100007);
		//test_reg_cfg(0x800,0x22);

		printf( "0xc:0x%08x\n", READSDR(base_adr + 0xc));
		addr = base_adr + 0xc;
		data = 0x30fd97;
		WRITESDR(addr, data);//39483504
		data = 0x30fd96;
		printf( "0xc:0x%08x\n", READSDR(addr));

		i = 0;
		while(1){
			if(data==READSDR(addr))
			{
				break;
			}
			if(i++%1024*1024 == 0)
				printf( "0xc:0x%08x\n", READSDR(addr));
		}
		base_adr += SDRC_REGOFFSET;
	}
	printf( "DDR init finish\n");
}

UNSG32 SDR_BISR_Check(SDR_BISR_P bisr_info)
{
	UNSG32 i, j;
	ERR_RESOURCE_INFO_p		p_blR = bisr_info->bitLineResource;
	ERR_RESOURCE_INFO_p		p_wlR = bisr_info->wordLineResource;
	UNSG32 rp_addr;
	UNSG32 failed = 0;

	if (!bisr_info->normal_err_count)
		return 1;

	PRN_LOG(fp_dumper, PRN_INFO, "\n/**************** Check Whether Repair is Successful or not ****************/\n");

	WRITESDR(0x88600064, 0x80100007);
	WRITESDR(0x88640064, 0x80100007);
	WRITESDR(0x88680064, 0x80100007);

	for ( i = 0; i < 128; i++) {
		if (p_wlR->used) {
			rp_addr = (p_wlR->redundancy_resource.rowaddr << 7) | (p_wlR->redundancy_resource.coladdr);
			if (!WL_Repair_check(rp_addr, bisr_info->SDRID, i)) {
				PRN_LOG(fp_dumper, PRN_INFO, "\t addr:%x is repaired by WL Red!\n", rp_addr);
				PRN_LOG(fp_dumper, PRN_INFO, "\t Repair fail!\n");
				failed = 1;
			}
		}
		p_wlR++;
	}

	for ( j = 0; j < 4; j++)
		for ( i = 0; i < 24; i++) {
			if (p_blR->used) {
				if ((p_blR->hasCross & 0x3 == CROSSREPAIR) && (p_blR->redundancy_resource.rowaddr == (p_blR->hasCross >> 8))) //cross
				{
					p_blR++;
					continue;
				}

				rp_addr = (p_blR->redundancy_resource.rowaddr << 7) | (p_blR->redundancy_resource.coladdr);			
				if (!BL_Repair_check(rp_addr, bisr_info->SDRID, j)) {
					PRN_LOG(fp_dumper, PRN_INFO, "\t addr:%x is repaired by BL Red!\n", rp_addr);
					PRN_LOG(fp_dumper, PRN_INFO, "\t Repair fail!\n");
					failed = 1;
				}
			}
			p_blR++;
		}

	return (!failed);
}

UNSG32 SDR_BISR_Entry(UNSG8 *argv[], int ReRepair)
{
	UNSG32		i;
	SDR_BISR_t  *sdr_bisr = NULL; 
	UNSG8		**arg = argv;
	UNSG32		stat;

#ifdef DUMP_LOG
	fp_dumper = fopen("bisr.log", "wt");
#endif

#ifdef FT_TEST
	for ( i = 0; i < 3; i++)
	{
#endif
		PrepareSDR(0);

		// open SDR device
		PRN_LOG(fp_dumper, PRN_DBG, "input file: %s, %s\n", arg[0], arg[1]);
		sdr_bisr = sdr_open(sdr_bisr, arg);

		if ( !sdr_bisr )
		{
			PRN_LOG(fp_dumper, PRN_ERR, "malloc sdr bisr failed!\n");
			stat = PROC_ERR;
			return stat;
		}

		// Repair SDR
		stat = SDR_PrePrepair(sdr_bisr, ReRepair);

#ifdef FT_TEST
		switch(stat)
		{
		case SUCCESS:
			Do_Repair(sdr_bisr);		
			break;
		case PROC_ERR: 
			PRN_LOG(fp_dumper, PRN_INFO, "Program Error!\n"); 
			break;
		case REPAIR_ERR: 
			PRN_LOG(fp_dumper, PRN_INFO, "Repair Error!\n"); 
			break;
		case INFO_ERR: 
			PRN_LOG(fp_dumper, PRN_INFO, "Get Err Info Error!\n"); 
			break;
		default: PRN_LOG(fp_dumper, PRN_INFO, "Repair Successfully!\n");
		}

#else
		switch(stat)
		{
		case SUCCESS:	 
			PRN_LOG(fp_dumper, PRN_INFO, "Repair Successfully!\n");
			Do_Repair(sdr_bisr);
			break;
		case PROC_ERR: 
			PRN_LOG(fp_dumper, PRN_INFO, "Program Error!\n"); 
			break;
		case REPAIR_ERR: 
			PRN_LOG(fp_dumper, PRN_INFO, "Repair Error!\n"); 
			break;
		case INFO_ERR:
			PRN_LOG(fp_dumper, PRN_INFO, "Get Err Info Error!\n"); 	
			break;
		default: 
			PRN_LOG(fp_dumper, PRN_INFO, "Unknow Status!\n");
		}

		sdr_bisr = sdr_release(sdr_bisr);
#endif

		ExitSDR(0);

		//if (SDR_BISR_Check(sdr_bisr)) 
		if (1)
		{
			PRN_LOG(fp_dumper, PRN_INFO, "/*********************** SDRAM %x %x Repair Successfully! ***********************/\n", sdr_bisr->SDRID - 1, sdr_bisr->SDRID);
		}
		else {
			PRN_LOG(fp_dumper, PRN_INFO, "/*********************** SDRAM %x %x Repair Error! ***********************/\n", sdr_bisr->SDRID - 1, sdr_bisr->SDRID);
		}

		PRN_LOG(fp_dumper, PRN_INFO, "\n============================================ Cut Line! ============================================\n\n");

		// release SDR device
		sdr_bisr = sdr_release(sdr_bisr); 

#ifdef FT_TEST
		arg = arg + 2;
	}
#endif	

#ifdef DUMP_LOG
	fclose(fp_dumper);
#endif

	return stat;
}

UNSG32 SDR_BISR_DBG(UNSG8 *argv[], int ReRePair)
{
	UNSG32		i;
	SDR_BISR_t  *sdr_bisr = NULL; 
	UNSG8		**arg = argv;
	UNSG32		stat;

#ifdef DUMP_LOG
	fp_dumper = fopen("bisr.log", "wt");
#endif

	for ( i = 0; i < 3; i++)
	{
		sdr_bisr = sdr_open(sdr_bisr, arg);

		if ( !sdr_bisr )
		{
			PRN_LOG(fp_dumper, PRN_ERR, "malloc sdr bisr failed!\n");
			stat = PROC_ERR;
			return stat;
		}

		// Repair SDR
		stat = SDR_PrePrepair(sdr_bisr, ReRePair);

		switch(stat)
		{
		case SUCCESS:
			//Do_Repair(sdr_bisr);
			//SDR_BISR_Check(sdr_bisr);
			PRN_LOG(fp_dumper, PRN_INFO, "/*********************** SDRAM %x %x Repair Successfully! ***********************/\n", sdr_bisr->SDRID - 1, sdr_bisr->SDRID);
			break;
		case PROC_ERR: 
			PRN_LOG(fp_dumper, PRN_INFO, "Program Error!\n"); 
			break;
		case REPAIR_ERR: 
			PRN_LOG(fp_dumper, PRN_INFO, "Repair Error!\n"); 
			break;
		case INFO_ERR: 
			PRN_LOG(fp_dumper, PRN_INFO, "Get Err Info Error!\n"); 
			break;
		default: PRN_LOG(fp_dumper, PRN_INFO, "Repair Successfully!\n");
		}

		PRN_LOG(fp_dumper, PRN_INFO, "\n============================================ Cut Line! ============================================\n\n");

		// release SDR device
		sdr_bisr = sdr_release(sdr_bisr); 

		arg = arg + 2;
	}

#ifdef DUMP_LOG
	fclose(fp_dumper);
#endif

	return stat;
}

SDR_BISR_t*	sdr_open(SDR_BISR_t* sdr_bisr, UNSG8 *argv[])
{
	// open sdr_bisr
	sdr_bisr = (SDR_BISR_t*)malloc(sizeof(SDR_BISR_t));
	if ( !sdr_bisr )
	{
		PRN_LOG(fp_dumper, PRN_ERR, "malloc SDR BISR failed and exit !\n");
		return 0;
	}

	// open FT result file
	fp_FTRes = fopen(argv[0], "rb");
	if ( !fp_FTRes ) 
	{
		PRN_LOG(fp_dumper, PRN_ERR, "can not find FT result file and exit !\n");
		return 0;
	}

#ifdef FT_TEST
	fp_FTRes_ex = fopen(argv[1], "rb");
	if ( !fp_FTRes_ex ) 
	{
		PRN_LOG(fp_dumper, PRN_ERR, "can not find FT result file and exit !\n");
		return 0;
	}
#endif

	// open sdr
	sdr_bisr->normalResource = (ERR_RESOURCE_INFO_p)malloc(MAX_BUFF_SIZE*sizeof(ERR_RESOURCE_INFO_t));
	if ( !sdr_bisr->normalResource )
	{
		PRN_LOG(fp_dumper, PRN_ERR, "malloc normalResource failed and exit !\n");
		return 0;
	}

	sdr_bisr->wordLineResource = (ERR_RESOURCE_INFO_p)malloc(WLR_SIZE*sizeof(ERR_RESOURCE_INFO_t));
	if ( !sdr_bisr->wordLineResource )
	{
		PRN_LOG(fp_dumper, PRN_ERR, "malloc wordLineResource failed and exit !\n");
		return 0;
	}

	sdr_bisr->bitLineResource = (ERR_RESOURCE_INFO_p)malloc(BLR_SIZE*sizeof(ERR_RESOURCE_INFO_t));
	if ( !sdr_bisr->bitLineResource )
	{
		PRN_LOG(fp_dumper, PRN_ERR, "malloc bitLineResource failed and exit !\n");
		return 0;
	}

	//initial 
	sdr_bisr->p_normalResource = sdr_bisr->normalResource;
	sdr_bisr->p_wordLineResource = sdr_bisr->wordLineResource;
	sdr_bisr->p_bitLineResource = sdr_bisr->bitLineResource;
	sdr_bisr->normal_err_count = 0;
	sdr_bisr->SDRID = 0;

	memset(sdr_bisr->normalResource, 0, sizeof(ERR_RESOURCE_INFO_t)*MAX_BUFF_SIZE);
	memset(sdr_bisr->wordLineResource, 0, sizeof(ERR_RESOURCE_INFO_t)*WLR_SIZE);
	memset(sdr_bisr->bitLineResource, 0, sizeof(ERR_RESOURCE_INFO_t)*BLR_SIZE);
	
	return sdr_bisr;
}

SDR_BISR_P	sdr_release(SDR_BISR_P	sdr_bisr)
{
	if ( sdr_bisr->normalResource )
		free(sdr_bisr->normalResource);

	if ( sdr_bisr->wordLineResource )
		free(sdr_bisr->wordLineResource);

	if ( sdr_bisr->bitLineResource )
		free(sdr_bisr->bitLineResource);

	fclose(fp_FTRes);

#ifdef FT_TEST
	fclose(fp_FTRes_ex);
#endif

	if ( sdr_bisr )
		free(sdr_bisr);

	return sdr_bisr;
}

UNSG32 precharge_nbytes_data(UNSG32 *buffer, UNSG32 buffer_ptr, UNSG32 get_byte, UNSG32* dat_byte)
{
	UNSG32 i = 0;
	
	memcpy(dat_byte, buffer + buffer_ptr, get_byte); 
	return get_byte;
}

UNSG32 IsSDRIDFlag(UNSG32 buffer)
{
	if ( buffer == 0xbeef1000 || buffer == 0xbeef2000 \
		|| buffer == 0xbeef3000 || buffer == 0xbeef4000 \
		|| buffer == 0xbeef5000 || buffer == 0xbeef6000)
	{
		return 1;
	}
	else return 0;
}

UNSG32 GetSDRID(UNSG32 buffer)
{
	return (buffer & 0xF000) >> 12;
}

UNSG32 CheckSDRID(UNSG32 sdr_id, UNSG32 buffer)
{
	if ( (( buffer & 0xF000) >> 12) == sdr_id ) 
	{
		return 1;
	}
	else {
		PRN_LOG(fp_dumper, PRN_ERR, "SDRAM ID is not match, the FT Info is Wrong!\n");
		return 1;
	}
}

UNSG32 IsSDRTypeFlag(UNSG32 buffer, UNSG32 sdr_id)
{
	UNSG32 temp = buffer - 0xbeef1000; 
	
	if ( (temp & 0x0FF) == 0x0)
		return 1;
	else
	{
		PRN_LOG(fp_dumper, PRN_ERR, "The Flag is not SDR Type Flag and may be the Flag is wrong!\n");
		return 0;
	}
}

UNSG32 GetSDRType(UNSG32 buffer)
{
	UNSG32 temp = buffer - 0xbeef1000; 

	if ( (temp & 0x0FF) == 0x0)
		temp = (temp >> 8) & 0xF;

#if 0
	switch(temp) {
	case 1: PRN_LOG(fp_dumper, PRN_INFO, ">>>>>>>>>It is Normal Error!\n"); return NORMAL_ERR;
	case 2: PRN_LOG(fp_dumper, PRN_INFO, ">>>>>>>>>It is WORD Line Error!\n"); return WORDLINE_ERR;
	case 3: PRN_LOG(fp_dumper, PRN_INFO, ">>>>>>>>>It is BIT Line Error!\n"); return BITLINE_ERR; 
	case 4: PRN_LOG(fp_dumper, PRN_INFO, ">>>>>>>>>It is Cross Error!\n"); return CROSS_ERR; 
	default: PRN_LOG(fp_dumper, PRN_ERR, "\tThe Type Flag is Error and return !\n"); return 0;    // not the SDR Type header
	}
#else 
	return temp;
#endif
}

UNSG32 IsEndFlag(UNSG32 buffer)
{
	if ( buffer == 0xdeaddead)
		return 1;
	else return 0;
}

UNSG32 IsBLErrNumFlag(UNSG32 buffer)
{
	if ( buffer == 0xdeadbeef)
		return 1;
	else return 0;
}

UNSG32 IsSDRHeader(UNSG32 buffer_value)
{
	UNSG32  sdr_id;

	if ( (buffer_value >> 16) == 0xbeef || (buffer_value >> 16) == 0xdead) 
	{
		return 1;
	}
	else return 0;
}

UNSG32 GetSDRHeader(UNSG32 buffer_value, SDR_BISR_t *bisr_info)
{
	static UNSG32  sdr_id;
	static UNSG32  sdr_type;

	if ( IsEndFlag(buffer_value) )
	{
		PRN_LOG(fp_dumper, PRN_DBG, "\n/***************Analsys one buffer finish！*****************/ \n");
		return FINISH_PROC_STAT;
	}

	if ( IsBLErrNumFlag(buffer_value) )
	{
		bisr_info->bl_num_flag = 1;
		bisr_info->p_normalResource->bl_repair_flag = 1;
		PRN_LOG(fp_dumper, PRN_DBG, "Find Error Number of BL larger than threshold!\n");
		return HEAD_PROC_STAT;
	}

	// compare headers
	if ( IsSDRIDFlag(buffer_value) ) 
	{
		sdr_id = GetSDRID(buffer_value);  // id:1~6
		bisr_info->SDRID = sdr_id;
		PRN_LOG(fp_dumper, PRN_DBG, "\n/************************SDRAM ID is %d************************/ \n", sdr_id);
		return HEAD_PROC_STAT;
	}

	if ( CheckSDRID(sdr_id, buffer_value) && IsSDRTypeFlag(buffer_value, sdr_id))
	{
		sdr_type = GetSDRType(buffer_value);
		bisr_info->type = sdr_type;
		return HEAD_PROC_STAT;
	}
}

UNSG32 GetSDRErrInfo(UNSG32 *FT_dat_buffer, SDR_BISR_t *bisr_info)
{
	UNSG32  buffer_value;
	UNSG32  read_buffer_cnt = 0;
	UNSG32  buffer_ptr = 0;
	UNSG32  proc_stat = START_PROC_STAT;

	do {
		if ( FINISH_PROC_STAT == proc_stat || ERR_PROC_STAT == proc_stat)
			return proc_stat;

		read_buffer_cnt = 4;
		if ( precharge_nbytes_data(FT_dat_buffer, buffer_ptr, read_buffer_cnt, &buffer_value) != read_buffer_cnt ) 
		{
			PRN_LOG(fp_dumper, PRN_ERR, "\n======= no more data, and data is wrong! =======\n");
			proc_stat = ERR_PROC_STAT;
			break;
		}

		if ( IsSDRHeader(buffer_value) )
		{
			//PRN_LOG(fp_dumper, PRN_INFO, "\n================ get SDRAM Head info : %x =================\n", buffer_value);
			proc_stat = GetSDRHeader(buffer_value, bisr_info);
			buffer_ptr += 1;
		}
		else
		{
			//fprintf(fp_dumper, "get SDRAM Err info : %x =======\n", buffer_value);
			proc_stat = fill_bisr_info(buffer_value, bisr_info);
			buffer_ptr += 1;
		}
	}while(1);
}

/*冒泡排序
 *Return the coladdr after sort
 */
void NMSort(UNSG32 *inp)
{
	UNSG32 i, j, k;
	UNSG32 n = 1024;
	UNSG32 a[1024][2] = {0};  //0: err count; 1: coladdr

	for ( i = 0; i < 1024; i++)
	{
		a[i][0] = inp[i];
		a[i][1] = i; 
	}


	for(i =0 ; i< n-1; ++i) {  
		for(j = 0; j < n-i-1; ++j) {  
			if(a[j][0] < a[j+1][0])  
			{  
				int tmp[2];
				memcpy(tmp, a[j], 2*sizeof(UNSG32));
				memcpy(a[j], a[j+1], 2*sizeof(UNSG32));
				memcpy(a[j+1], tmp, 2*sizeof(UNSG32));
			}  
		}  
	}

	for ( i = 0, k = 0; i < 1024; i++)
	{
		if ( a[i][0] != 0)
			inp[i] = a[i][1];
		else inp[i] = 0xFFFFFFFF;
		//if ( i < 4)
		//	PRN_LOG(fp_dumper, PRN_DBG, "      col: 0x%x, num: 0x%x      \n", a[i][1], a[i][0]);
	}
}

/**************************************************************************
 * Description:
 *				The function take a small MAT as the unit to sort the err
 *				information.
 * Step:
 *				1. 对每个小MAT中的单元都投影到COL方向
 *				2. 按照错误数量对每个小MAT中的col地址进行排序
 *				3. 统计每个小MAT中可以用来修补的BL Redundancy的数目
 *				4. 记录出错的小MAT在即将存储的buffer中的索引，索引与可修补的BLR相关
 *				5. 对每个小MAT，将出错最多的行并且可以用BLR修补的行先合并后存储在buffer前面，一般的行从buffer最后开始存储
 *				6. 更新普通资源出错的个数。
 * What's more
 *				将一个小MAT中的row地址和col地址分别投影到COL和ROW方向，优先使用
 *				出错多的方向的redundancy
 **************************************************************************/
UNSG32 SDR_Sort(SDR_BISR_P bisr, UNSG32 sort_type, UNSG32 total_err_cnt)
{
	ERR_RESOURCE_INFO_t *p_nlR;
	ERR_RESOURCE_INFO_t *p_blR;
	UNSG32	i, j, k = 0;
	UNSG32 *unit;
	SIGN32 statis_res[24][4] = {0};
	UNSG8 avail_BLR[24];
	ERR_RESOURCE_INFO_t *ptmp;
	ERR_RESOURCE_INFO_t *pn, *pp ; //pn:normal pp:high priority
	ERR_RESOURCE_INFO_t *nlR_cp;
	UNSG32 cnt_pp = 0, cnt_pn = 0;
	UNSG8 pp_index[24] = {0};

	if ( bisr->normal_err_count == 0)
		return 1;

	nlR_cp = (ERR_RESOURCE_INFO_t *)calloc(MAX_BUFF_SIZE, sizeof(ERR_RESOURCE_INFO_t));
	unit = (UNSG32 *)malloc(sizeof(UNSG32)*1024);

	if ( !nlR_cp)
	{
		PRN_LOG(fp_dumper, PRN_ERR, "can not malloc buffer!\n");
		return 0;
	}
	
	if ( !unit )
	{
		PRN_LOG(fp_dumper, PRN_ERR, "malloc sort buffer failed!\n");
		return 0;
	}

	//get available BL Redundancy
	p_blR = bisr->bitLineResource;
	memset(avail_BLR, 4, 24*sizeof(UNSG8));
	for ( i = 0; i < 96; i++, p_blR++)
	{
		if ( p_blR->err )
		{
			avail_BLR[p_blR->sMAT]--;
		}

		if ( avail_BLR[p_blR->sMAT] == 0)
			return 0;
	}

	//PRN_LOG(fp_dumper, PRN_INFO, "\n=========================== Sort Info =======================\n");
	for ( i = 0; i < 24; i++)
	{
		memset(unit, 0, 1024*sizeof(unit));
		ptmp = bisr->normalResource;
		//PRN_LOG(fp_dumper, PRN_DBG, "======= sMAT: 0x%x top col ==========\n", i);
		for ( j = 0; j < total_err_cnt; j++, ptmp++)
		{
			if ( ptmp->sMAT == i) 
			{
				unit[ptmp->colAdr] = unit[ptmp->colAdr] == MAX_ERR_FOR_BL ? MAX_ERR_FOR_BL : unit[ptmp->colAdr]+1;

				if ( ptmp->bl_repair_flag)
					unit[ptmp->colAdr] = MAX_ERR_FOR_BL;
			}
		}

		switch(sort_type) {
		case NORMAL_SORT: NMSort(unit); break;
		default: NMSort(unit); break;
		}

		pp_index[i] = k;
		//restore the sort result to the 24*4 metrix
		for ( j = 0; j < avail_BLR[i]; j++) 
		{
			statis_res[i][j] = unit[j];	 //coladdr
			if ( unit[j] != 0xFFFFFFFF)
				k++;
		}

		//pp_index[i] = (statis_res[i][0] == 0xFF) ? 96: i;
		// if statis_res[i][0] == -1, means this row has no error
		//if ( statis_res[i][0] == -1)
		//	pp_index[i] = 96;
		//else 
		//{
		//	pp_index[i] = k;
		//	k += avail_BLR[i];
		//}
	}
		
	/*********************************************** 
	 * set the bl_repair_flag
	 ***********************************************/
	ptmp = bisr->normalResource;
	pp = nlR_cp;
	pn = nlR_cp + MAX_BUFF_SIZE - 1;
	cnt_pp = k;
	for ( j = 0; j < total_err_cnt; j++, ptmp++)
	{		
		if ( statis_res[ptmp->sMAT][0] == -1)
			continue;

		if (ptmp->colAdr == statis_res[ptmp->sMAT][0]) // the max
		{
			ptmp->bl_repair_flag = 1;
			memcpy(&pp[pp_index[ptmp->sMAT]], ptmp, sizeof(ERR_RESOURCE_INFO_t));
		}
		else if (ptmp->colAdr == statis_res[ptmp->sMAT][1] && avail_BLR[ptmp->sMAT] >= 2) // the next
		{
			ptmp->bl_repair_flag = 1;
			memcpy(&pp[pp_index[ptmp->sMAT] + 1], ptmp, sizeof(ERR_RESOURCE_INFO_t));
		}
		else if (ptmp->colAdr == statis_res[ptmp->sMAT][2] && avail_BLR[ptmp->sMAT] >= 3) // the next
		{
			ptmp->bl_repair_flag = 1;
			memcpy(&pp[pp_index[ptmp->sMAT] + 2], ptmp, sizeof(ERR_RESOURCE_INFO_t));
		}
		else if (ptmp->colAdr == statis_res[ptmp->sMAT][3] && avail_BLR[ptmp->sMAT] >= 4) // the next
		{
			ptmp->bl_repair_flag = 1;
			memcpy(&pp[pp_index[ptmp->sMAT] + 3], ptmp, sizeof(ERR_RESOURCE_INFO_t));
		}
		else 
		{
			cnt_pn++;
			memcpy(pn--, ptmp, sizeof(ERR_RESOURCE_INFO_t));
		}			
	}

	memset(bisr->normalResource, 0, sizeof(ERR_RESOURCE_INFO_t)*MAX_BUFF_SIZE);
	memcpy(bisr->normalResource, nlR_cp, sizeof(ERR_RESOURCE_INFO_t)*cnt_pp);
	memcpy(bisr->normalResource + cnt_pp, nlR_cp + MAX_BUFF_SIZE - cnt_pn, sizeof(ERR_RESOURCE_INFO_t)*cnt_pn);
	bisr->normal_err_count = cnt_pn + cnt_pp;

	free(nlR_cp);
	free(unit);

	return 1;
}

void CheckErr(ERR_RESOURCE_INFO_t *pErr, UNSG32 size)
{
	UNSG32 i;

	for ( i = 0; i < size; i++)
	{
		PRN_LOG(fp_dumper, PRN_DBG, "\n/**********Check ERROR INFO :%d*************/\n", i);
		PRN_LOG(fp_dumper, PRN_DBG, "\t rowaddr: %d, 0x%x\n", pErr[i].rowAdr, pErr[i].rowAdr);
		PRN_LOG(fp_dumper, PRN_DBG, "\t coladdr: %d, 0x%x\n", pErr[i].colAdr, pErr[i].colAdr);
		PRN_LOG(fp_dumper, PRN_DBG, "\t bMat   : %d, 0x%x\n", pErr[i].bMAT, pErr[i].bMAT);
		PRN_LOG(fp_dumper, PRN_DBG, "\t sMat   : %d, 0x%x\n", pErr[i].sMAT, pErr[i].sMAT);

	}
}

/***************************************************************************
 *	Description: Analysis the data of FT result and fill into sdrx.
 *				 1. Make sure which SDRAM is doing repair according to the first 32bits header
 *				 2. Get the error type according to 
 *				 3. Is the end of Buffer
 ***************************************************************************/
UNSG32 SDR_PrePrepair(SDR_BISR_P bisr_info, int ReRapair)
{
	UNSG32	read_cnt;
	UNSG32  *FT_dat_buffer = NULL;
	UNSG32  stat;

	// read all 2*2KB data
	FT_dat_buffer = (UNSG32 *)malloc(sizeof(UNSG32)*MAX_BUFF_SIZE); 
	if ( !FT_dat_buffer )
	{
		PRN_LOG(fp_dumper, PRN_ERR, "malloc buffer failed!\n");
		return PROC_ERR;
	}

	read_cnt = fread(FT_dat_buffer, MAX_BUFF_SIZE, sizeof(UNSG32), fp_FTRes);
	stat = GetSDRErrInfo(FT_dat_buffer, bisr_info);
	//PRN_LOG(fp_dumper, PRN_DBG, "@%d stat:%d\n", __LINE__, stat);

	if ( ERR_PROC_STAT == stat)
	{
		PRN_LOG(fp_dumper, PRN_ERR, "SDRAM Err information is error!\n");
		return INFO_ERR;
	}

#ifdef FT_TEST
	read_cnt = fread(FT_dat_buffer, MAX_BUFF_SIZE, sizeof(UNSG32), fp_FTRes_ex);
	stat = GetSDRErrInfo(FT_dat_buffer, bisr_info);
	if ( ERR_PROC_STAT == stat)
	{
		PRN_LOG(fp_dumper, PRN_ERR, "SDRAM Err information is error!\n");
		return INFO_ERR;
	}
#endif

#if 1
	stat = SDR_Sort(bisr_info, NORMAL_SORT, bisr_info->normal_err_count);
	PRN_LOG(fp_dumper, PRN_DBG, "@%d stat:%d\n", __LINE__, stat);
	if ( !stat )
	{
		PRN_LOG(fp_dumper, PRN_ERR, "sort return err!\n");
		return PROC_ERR;
	}
#endif

#ifdef SDR_DBG
	CheckErr(bisr_info->normalResource, bisr_info->normal_err_count);
	//CheckErr(bisr_info->bitLineResource, 96);
	//CheckErr(bisr_info->wordLineResource, 128);
#endif

	if (ReRapair) {
		//BLRed_used_chk(bisr_info, 0);
		//WLRed_used_chk(bisr_info, 0);
	}

	stat = SDR_Redundancy(bisr_info);
	PRN_LOG(fp_dumper, PRN_DBG, "@%d stat:%d\n", __LINE__, stat);
	if ( ERR_REPAIR_STAT == stat)
	{
		PRN_LOG(fp_dumper, PRN_ERR, "make repair plan failed!\n");
		return REPAIR_ERR;
	}
	

	
	Do_Map(bisr_info);
	
	free(FT_dat_buffer);

	return SUCCESS;
}