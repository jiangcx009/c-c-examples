#include "bisr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DUMP_LOG
FILE *fp_dumper;
#endif

void Merge(UNSG8 *argv[], UNSG32 cnt)
{
	UNSG8	outfile[100];
	UNSG32	i, j;
	UNSG32  *buffer = (UNSG32 *)malloc(MAX_BUFF_SIZE*sizeof(UNSG32));
	UNSG32  *p_buf = buffer;
	FILE	*fp_in, *fp_out;
	UNSG32  buf_dep = 0;
	
	for ( i = 0; i < cnt; i++)
	{
		fp_in = fopen(argv[i+1], "rb");
		if ( !fp_in )
		{
			PRN_LOG(PRN_ERR, "can not find file!\n");
			return ;
		}
		//fread(buffer, MAX_BUFF_SIZE*sizeof(UNSG32), 1, fp_in);

		for ( j = 0; j < 6; j++)
		{
			do{
				fread(p_buf, sizeof(UNSG32), 1, fp_in);
				buf_dep++;
			}while(*(p_buf++) != 0xdeaddead);

			sprintf(outfile, "test_ft_res%d.bin", j);
			fp_out = fopen(outfile, "ab+");

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
		sprintf(outfile, "test_ft_res%d.bin", j);
		fp_out = fopen(outfile, "ab+");
		*buffer = 0xdeaddead;
		
		fwrite(buffer, 1, sizeof(UNSG32), fp_out);

		fclose(fp_out);
	}

	free(buffer);

}


UNSG32 SDR_BISR_Entry(const UNSG8 *argv[])
{
	UNSG32 i;
	SDR_BISR_t *sdr_bisr = NULL; 
	UNSG8			**arg = argv;
	UNSG32		stat;

#ifdef DUMP_LOG
	fp_dumper = fopen("bisr.log", "wt");
#endif

#ifdef FT_TEST
	for ( i = 0; i < 3; i++)
	{
#endif
		// open SDR device
		sdr_bisr = sdr_open(sdr_bisr, arg);

		if ( !sdr_bisr )
		{
			PRN_LOG(PRN_ERR, "malloc sdr bisr failed!\n");
			return PROC_ERR;
		}

		// Repair SDR
		stat = SDR_PrePrepair(sdr_bisr);

		switch(stat)
		{
		case PROC_ERR: break;
		case REPAIR_ERR: break;
		case INFO_ERR: break;
		default: PRN_LOG(PRN_INFO, "Unknow Err!\n");
		}

		Do_Repair(sdr_bisr);
		
		// test

		// release SDR device
		sdr_bisr = sdr_release(sdr_bisr);

#ifdef FT_TEST
		arg = arg + 2;
	}
#endif

#ifdef DUMP_LOG
	fclose(fp_dumper);
#endif

	return ;
}

SDR_BISR_t*	sdr_open(SDR_BISR_t* sdr_bisr, UNSG8 *argv[])
{
	// open sdr_bisr
	sdr_bisr = (SDR_BISR_t*)malloc(sizeof(SDR_BISR_t));
	if ( !sdr_bisr )
	{
		PRN_LOG(PRN_ERR, "malloc SDR BISR failed and exit !\n");
		exit(-1);
	}

	// open FT result file
	sdr_bisr->fp_FTRes = fopen(argv[1], "rb");
	if ( !sdr_bisr->fp_FTRes ) 
	{
		printf(PRN_ERR, "can not find FT result file and exit !\n");
		return ;
	}

#ifdef FT_TEST
	sdr_bisr->fp_FTRes_ex = fopen(argv[2], "rb");
	if ( !sdr_bisr->fp_FTRes_ex ) 
	{
		printf(PRN_ERR, "can not find FT result file and exit !\n");
		return ;
	}
#endif

	// open sdr
	sdr_bisr->normalResource = (ERR_RESOURCE_INFO_p)malloc(MAX_BUFF_SIZE*sizeof(ERR_RESOURCE_INFO_t));
	if ( !sdr_bisr->normalResource )
	{
		PRN_LOG(PRN_ERR, "malloc normalResource failed and exit !\n");
		exit(-1);
	}

	sdr_bisr->wordLineResource = (ERR_RESOURCE_INFO_p)malloc(WLR_SIZE*sizeof(ERR_RESOURCE_INFO_t));
	if ( !sdr_bisr->wordLineResource )
	{
		PRN_LOG(PRN_ERR, "malloc wordLineResource failed and exit !\n");
		exit(-1);
	}

	sdr_bisr->bitLineResource = (ERR_RESOURCE_INFO_p)malloc(BLR_SIZE*sizeof(ERR_RESOURCE_INFO_t));
	if ( !sdr_bisr->bitLineResource )
	{
		PRN_LOG(PRN_ERR, "malloc bitLineResource failed and exit !\n");
		exit(-1);
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

	fclose(sdr_bisr->fp_FTRes);

#ifdef FT_TEST
	fclose(sdr_bisr->fp_FTRes_ex);
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
		|| buffer == 0xbeef4000 || buffer == 0xbeef8000 \
		|| buffer == 0xbeefa000 || buffer == 0xbbefc000)
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
		PRN_LOG(PRN_ERR, "SDRAM ID is not match, the FT Info is Wrong!\n");
		return 0;
	}
}

UNSG32 IsSDRTypeFlag(UNSG32 buffer, UNSG32 sdr_id)
{
	UNSG32 temp = buffer - 0xbeef1000; 
	
	if ( (temp & 0x0FF) == 0x0)
		return 1;
	else
	{
		PRN_LOG(PRN_ERR, "The Flag is not SDR Type Flag and may be the Flag is wrong!\n");
		return 0;
	}
}

UNSG32 GetSDRType(UNSG32 buffer)
{
	UNSG32 temp = buffer - 0xbeef1000; 

	if ( (temp & 0x0FF) == 0x0)
		temp = temp >> 8;

	switch(temp) {
	case 1: PRN_LOG(PRN_INFO, ">>>>>>>>>It is Normal Error!\n"); return NORMAL_ERR;
	case 2: PRN_LOG(PRN_INFO, ">>>>>>>>>It is WORD Line Error!\n"); return WORDLINE_ERR;
	case 3: PRN_LOG(PRN_INFO, ">>>>>>>>>It is BIT Line Error!\n"); return BITLINE_ERR; 
	case 4: PRN_LOG(PRN_INFO, ">>>>>>>>>It is Cross Error!\n"); return CROSS_ERR; 
	default: PRN_LOG(PRN_ERR, "\tThe Type Flag is Error and return !\n"); return 0;    // not the SDR Type header
	}
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
		PRN_LOG(PRN_INFO, "/***************Analsys one buffer finish！*****************/\n");
		return FINISH_PROC_STAT;
	}

	if ( IsBLErrNumFlag(buffer_value) )
	{
		bisr_info->bl_num_flag = 1;
		bisr_info->p_normalResource->bl_repair_flag = 1;
		PRN_LOG(PRN_INFO, "Find Error Number of BL larger than threshold!\n");
		return HEAD_PROC_STAT;
	}

	// compare headers
	if ( IsSDRIDFlag(buffer_value) ) 
	{
		sdr_id = GetSDRID(buffer_value);  // id:1~6
		bisr_info->SDRID = sdr_id;
		PRN_LOG(PRN_INFO, "SDRAM ID is %d\n", sdr_id);
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
			PRN_LOG(PRN_ERR, "\n======= no more data, and data is wrong! =======\n");
			proc_stat = ERR_PROC_STAT;
			break;
		}

		if ( IsSDRHeader(buffer_value) )
		{
			PRN_LOG(PRN_INFO, "\n================ get SDRAM Head info : %x =================\n", buffer_value);
			proc_stat = GetSDRHeader(buffer_value, bisr_info);
			buffer_ptr += 1;
		}
		else
		{
			PRN_LOG(PRN_INFO, "\n======= get SDRAM Err info : %x =======\n", buffer_value);
			//fprintf(fp_dumper, "get SDRAM Err info : %x =======\n", buffer_value);
			proc_stat = fill_bisr_info(buffer_value, bisr_info);
			buffer_ptr += 1;
		}
	}while(1);
}

//冒泡排序
void NMSort(UNSG8 *inp)
{
	UNSG32 i, j;
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

	for ( i = 0; i < 1024; i++)
	{
		inp[i] = a[i][1];
	}
}

/**************************************************************************
 * Description:
 *				The function take a small MAT as the unit to sort the err
 *				information.
 *				1. 对每个小MAT中的单元都投影到COL方向
 *				2. 按照错误数量进行排序
 **************************************************************************/
UNSG32 SDR_Sort(ERR_RESOURCE_INFO_t *nlR, UNSG32 sort_type, UNSG32 total_err_cnt)
{
	ERR_RESOURCE_INFO_t *p_nlR;
	UNSG32	i, j;
	UNSG8 *unit = (UNSG8 *)malloc(sizeof(UNSG8)*1024);
	UNSG8 statis_res[24][4] = {0};
	ERR_RESOURCE_INFO_t *ptmp = nlR;
	ERR_RESOURCE_INFO_t *pn, *pp ;
	ERR_RESOURCE_INFO_t *nlR_cp;

	nlR_cp = (ERR_RESOURCE_INFO_t *)malloc(sizeof(ERR_RESOURCE_INFO_t)*MAX_BUFF_SIZE);

	if ( nlR_cp)
	{
		PRN_LOG(PRN_ERR, "can not malloc buffer!\n");
		return 0;
	}
	
	if ( !unit )
	{
		PRN_LOG(PRN_ERR, "malloc sort buffer failed!\n");
		return 0;
	}

	for ( i = 0; i < 24; i++)
	{
		memset(unit, 0, 1024);
		for ( j = 0; j < total_err_cnt; j++, ptmp++)
		{
			if ( ptmp->sMAT == i) 
			{
				unit[ptmp->colAdr] = unit[ptmp->colAdr] == MAX_ERR_FOR_BL ? MAX_ERR_FOR_BL : unit[ptmp->colAdr]+1;

				if ( ptmp->bl_repair_flag)
					unit[ptmp->colAdr] = MAX_ERR_FOR_BL;
			}

			//ptmp++;
		}

		switch(sort_type) {
		case NORMAL_SORT: NMSort(unit); break;
		default: NMSort(unit); break;
		}

		//restore the sort result to the 24*4 metrix
		for ( j = 0; j < 3; j++)
			statis_res[i][j] = unit[j];
	}
		
	/*********************************************** 
	 * set the bl_repair_flag
	 ***********************************************/
	ptmp = nlR;
	pp = nlR_cp;
	pn = nlR_cp + 96;
	for ( j = 0; j < total_err_cnt; j++)
	{
		if (ptmp->colAdr == statis_res[ptmp->sMAT][0] || \
			ptmp->colAdr == statis_res[ptmp->sMAT][1] || \
			ptmp->colAdr == statis_res[ptmp->sMAT][2] || \
			ptmp->colAdr == statis_res[ptmp->sMAT][3] )
		{
				ptmp->bl_repair_flag = 1;
				memcpy(&(pp[ptmp->colAdr]), ptmp, sizeof(ERR_RESOURCE_INFO_t));
		}
		else 
			memcpy(pn++, ptmp, sizeof(ERR_RESOURCE_INFO_t));
	}

	memcpy(nlR, nlR_cp, sizeof(ERR_RESOURCE_INFO_t)*MAX_BUFF_SIZE);

	free(nlR_cp);
	free(unit);

	return 1;
}

void CheckErr(ERR_RESOURCE_INFO_t *pErr, UNSG32 size)
{
	UNSG32 i;

	for ( i = 0; i < size; i++)
	{
		PRN_LOG(PRN_DBG, "\n/**********Check ERROR INFO :%d*************/\n", i);
		PRN_LOG(PRN_DBG, "\t rowaddr: %d, 0x%x\n", pErr[i].rowAdr, pErr[i].rowAdr);
		PRN_LOG(PRN_DBG, "\t coladdr: %d, 0x%x\n", pErr[i].colAdr, pErr[i].colAdr);
		PRN_LOG(PRN_DBG, "\t bMat   : %d, 0x%x\n", pErr[i].bMAT, pErr[i].bMAT);
		PRN_LOG(PRN_DBG, "\t sMat   : %d, 0x%x\n", pErr[i].sMAT, pErr[i].sMAT);

	}
}

/***************************************************************************
 *	Description: Analysis the data of FT result and fill into sdrx.
 *				 1. Make sure which SDRAM is doing repair according to the first 32bits header
 *				 2. Get the error type according to 
 *				 3. Is the end of Buffer
 ***************************************************************************/
UNSG32 SDR_PrePrepair(SDR_BISR_P bisr_info)
{
	UNSG32	read_cnt;
	UNSG32  *FT_dat_buffer = NULL;
	UNSG32  stat;

	// read all 2*2KB data
	FT_dat_buffer = (UNSG32 *)malloc(sizeof(UNSG32)*MAX_BUFF_SIZE); 
	if ( !FT_dat_buffer )
	{
		PRN_LOG(PRN_ERR, "malloc buffer failed!\n");
		return PROC_ERR;
	}

	read_cnt = fread(FT_dat_buffer, MAX_BUFF_SIZE, sizeof(UNSG32), bisr_info->fp_FTRes);
	stat = GetSDRErrInfo(FT_dat_buffer, bisr_info);

	if ( ERR_PROC_STAT == stat)
	{
		PRN_LOG(PRN_ERR, "SDRAM Err information is error!\n");
		return INFO_ERR;
	}

#ifdef FT_TEST
	read_cnt = fread(FT_dat_buffer, MAX_BUFF_SIZE, sizeof(UNSG32), bisr_info->fp_FTRes_ex);
	stat = GetSDRErrInfo(FT_dat_buffer, bisr_info);
	if ( ERR_PROC_STAT == stat)
	{
		PRN_LOG(PRN_ERR, "SDRAM Err information is error!\n");
		return INFO_ERR;
	}
#endif

#if 0
	stat = SDR_Sort(bisr_info->normalResource, NORMAL_SORT, bisr_info->normal_err_count);
	if ( 0 == stat )
	{
		PRN_LOG(PRN_ERR, "sort return err!\n");
		return PROC_ERR;
	}
#endif

	stat = SDR_Redundancy(bisr_info);
	if ( ERR_REPAIR_STAT == stat)
	{
		PRN_LOG(PRN_ERR, "make repair plan failed!\n");
		return REPAIR_ERR;
	}

#ifdef SDR_DBG
	//CheckErr(bisr_info->normalResource, bisr_info->normal_err_count);
	CheckErr(bisr_info->bitLineResource, 96);
	CheckErr(bisr_info->wordLineResource, 128);
#endif
	Do_Map(bisr_info);
	
	free(FT_dat_buffer);
}