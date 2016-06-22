#include "bisr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include "usbjtag.h"
#include "libfpga.h"
#include "lusb0_usb.h"

extern FILE *fp_dumper;


void colRepair(int** colFuse, int colRed[24][4]){

	int colRedbin[4][8][24] = {0};
	int colFusetemp[4][3][8][8] = {0};
	int colFusebin[4][3][8][8] = {0};

	int i,j,k,l,m,n;

	//colRed to binary expression conversion
	for(i=0;i<24;i++)
		for(j=0;j<4;j++)
			for(k=0;k<8;k++)
				colRedbin[j][k][i] = (colRed[i][j] / (int)pow(2,k)) % 2;	

	//reset unused redundancy
	for(i=0;i<4;i++)
		for(j=0;j<24;j++)
			if (colRedbin[i][7][j] == 0)
				for (k=0;k<7;k++)
					colRedbin[i][k][j] = 0;

	//colRed to colFuse convertion
	m = 0;
	n = 0;
	for(i=0;i<4;i++)
		for(j=0;j<3;j++)
			for(k=0;k<8;k++)
				for(l=0;l<8;l++){
					colFusebin[i][j][l][k] = colRedbin[i][m][n];
					n++;
					if(n==24){
						n = 0;
						m++;
					}
					if(m==8)
						m = 0;
				}

				//colFuse addrress generation
				for(i=0;i<48;i++){
					colFuse[2*i] = 0x88 + i + i/8*8;
				}

				for(i=48;i<96;i++){
					colFuse[2*i] = 0x08 + i - 48 + (i - 48) /8*8;
				}

				//colFuse data generation
				l = 0;
				for(i=0;i<4;i++)
					for(j=0;j<3;j++)
						for(k=0;k<8;k++){
							colFuse[2*l+1] = colFusebin[i][j][k][7]*0x80 + colFusebin[i][j][k][6]*0x40 + colFusebin[i][j][k][5]*0x20 + colFusebin[i][j][k][4]*0x10
								+ colFusebin[i][j][k][3]*0x08 + colFusebin[i][j][k][2]*0x04 + colFusebin[i][j][k][1]*0x02 + colFusebin[i][j][k][0]*0x01;
							l++;
						}
}

void rowRepair(int** rowFuse, int rowRed[128]){

	int rowRedbin[128][16] = {0};
	int rowFusebin[256][8] = {0};

	int i,j;

	//rowRed to binary expression conversion
	for(i=0;i<128;i++)
		for(j=0;j<16;j++)
			rowRedbin[i][j] = (rowRed[i] / (int)pow(2,j)) % 2;

	//reset unused redundancy
	for(i=0;i<128;i++)
		if (rowRedbin[i][14] == 0)
			for (j=0;j<14;j++)
				rowRedbin[i][j] = 1;

	//rowRed to rowFuse convertion
	for(i=0;i<8;i++)
		for(j=0;j<256;j++){
			if((j-14)%16 == 0)
				rowFusebin[j][i] = rowRedbin[i+j/16*8][j%16];
			else if ((j-15)%16 == 0)
				rowFusebin[j][i] = 0;
			else
				rowFusebin[j][i] = 1 - rowRedbin[i+j/16*8][j%16];
		}

		//rowFuse addrress generation
		for(i=0;i<256;i++){
			rowFuse[2*i] = 0x100 + i;
		}

		//rowFuse data generation
		for(i=0;i<256;i++)
			rowFuse[2*i+1] = rowFusebin[i][7]*0x80 + rowFusebin[i][6]*0x40 + rowFusebin[i][5]*0x20 + rowFusebin[i][4]*0x10
			+ rowFusebin[i][3]*0x08 + rowFusebin[i][2]*0x04 + rowFusebin[i][1]*0x02 + rowFusebin[i][0]*0x01;
}

UNSG32 fill_normal_err(ERR_RESOURCE_INFO_t *nlR, UNSG32 rowaddr, UNSG32 coladdr)
{
	UNSG32	sMat, bMat, tmp;

	bMat = rowaddr / BIGMAT_W;
	tmp  = rowaddr % 0x800;
	tmp  = (tmp >= SMALLMAT_W) ? ((tmp >= ((2 * SMALLMAT_W) - 16)) ? 2 : 1) : 0;
	sMat = bMat * 3 + tmp;

	nlR->rowAdr = rowaddr;
	nlR->colAdr = coladdr;
	nlR->bMAT   = bMat;
	nlR->sMAT	= sMat;

#ifdef SDR_DBG
	PRN_LOG(fp_dumper, PRN_DBG, "\t/**********ERROR INFO*************/\n");
	PRN_LOG(fp_dumper, PRN_DBG, "\t rowaddr: %d, 0x%x\n", rowaddr, rowaddr);
	PRN_LOG(fp_dumper, PRN_DBG, "\t coladdr: %d, 0x%x\n", coladdr, coladdr);
	PRN_LOG(fp_dumper, PRN_DBG, "\t bMat   : %d, 0x%x\n", bMat, bMat);
	PRN_LOG(fp_dumper, PRN_DBG, "\t sMat   : %d, 0x%x\n", sMat, sMat);
#endif

}

/****************************************************************
 * Description:
 *				If the WordLine Redundancy is err, fill the 
 *				buffer and a word line redundancy use a buffer
 ****************************************************************/
UNSG32 fill_WL_redundancy_err(ERR_RESOURCE_INFO_t *wlR, UNSG32 rowaddr, UNSG32 coladdr)
{
	UNSG32		index = 0;

	if ( rowaddr < 0 || rowaddr > 0x7F)
	{
		PRN_LOG(fp_dumper, PRN_ERR, "WordLine Redundancy addr is error!\n");
		return ERR_PROC_STAT;
	}

#if (VERSION == 2)
	index = 128*coladdr + rowaddr;
#else 
	index = rowaddr;
#endif

	//fill the buffer
	if ( wlR[index].rowAdr == index)
	{
		PRN_LOG(fp_dumper, PRN_DBG, "Have fill the buffer!\n");
		return CTX_PROC_STAT;
	}

	wlR[index].colAdr = coladdr;
	wlR[index].rowAdr = rowaddr;
	wlR[index].err    = 1;
	wlR[index].bMAT   = rowaddr/8;

#ifdef SDR_DBG
	PRN_LOG(fp_dumper, PRN_DBG, "\t/**********ERROR INFO*************/\n");
	PRN_LOG(fp_dumper, PRN_DBG, "\t rowaddr: %d, 0x%x\n", rowaddr, rowaddr);
	PRN_LOG(fp_dumper, PRN_DBG, "\t coladdr: %d, 0x%x\n", coladdr, coladdr);
	PRN_LOG(fp_dumper, PRN_DBG, "\t bMat   : %d, 0x%x\n", rowaddr/8, rowaddr/8);
	PRN_LOG(fp_dumper, PRN_DBG, "\t sMat   : %d, 0x%x\n", 0, 0);
#endif

	return CTX_PROC_STAT;
}

/****************************************************************
 * Description:
 *				If the BitLine Redundancy is err, fill the 
 *				buffer and a bitline of a small Mat use a buffer
 ****************************************************************/
UNSG32 fill_BL_redundancy_err(ERR_RESOURCE_INFO_t *blR, UNSG32 rowaddr, UNSG32 coladdr)
{
	UNSG32		index = 0;
	UNSG32		col_index;
	UNSG32		sMat, bMat, tmp;
	bMat = rowaddr / BIGMAT_W;
	tmp  = rowaddr % 0x800;
	tmp  = (tmp >= SMALLMAT_W) ? ((tmp >= ((2 * SMALLMAT_W) - 16)) ? 2 : 1) : 0;
	sMat = bMat * 3 + tmp;

	if ( coladdr < 0 || coladdr > 0x3)
	{
		PRN_LOG(fp_dumper, PRN_ERR, "BitLine Redundancy is error!\n");
		return ERR_PROC_STAT;
	}
	col_index  = coladdr;

#if (VERSION == 2)
	index = col_index * BLR_W + rowaddr;
#else 
	index = col_index * 24 + sMat;
#endif

	//fill the buffer
	blR[index].colAdr = coladdr;
	blR[index].rowAdr = rowaddr;
	blR[index].err    = 1;
	blR[index].sMAT   = sMat;

#ifdef SDR_DBG
	PRN_LOG(fp_dumper, PRN_DBG, "\t/**********ERROR INFO*************/\n");
	PRN_LOG(fp_dumper, PRN_DBG, "\t rowaddr: %d, 0x%x\n", rowaddr, rowaddr);
	PRN_LOG(fp_dumper, PRN_DBG, "\t coladdr: %d, 0x%x\n", coladdr, coladdr);
	PRN_LOG(fp_dumper, PRN_DBG, "\t bMat   : %d, 0x%x\n", bMat, bMat);
	PRN_LOG(fp_dumper, PRN_DBG, "\t sMat   : %d, 0x%x\n", sMat, sMat);
#endif

	return CTX_PROC_STAT;
}

//HOW ?
UNSG32 fill_CROSS_redundancy_err(SDR_BISR_t *sdr_info, UNSG32 rowaddr, UNSG32 coladdr)
{
	UNSG32		index = 0;
	UNSG32		col_index;
	UNSG32		sMat, bMat;

	bMat = rowaddr / 128;
	sMat = 3 * bMat + 1;

#if (VERSION == 2)
#else 
	//index = coladdr * 8 + bMat;

	//fill the BL Redundancy buffer
	index = coladdr * 24 + sMat;

	sdr_info->bitLineResource[index].colAdr = coladdr;
	sdr_info->bitLineResource[index].rowAdr = rowaddr;
	sdr_info->bitLineResource[index].err    = 1;
	sdr_info->bitLineResource[index].sMAT   = sMat;
	sdr_info->bitLineResource[index].bMAT   = bMat;

	//fill the WL Redundancy buffer
	index = rowaddr;

	sdr_info->wordLineResource[index].colAdr = coladdr;
	sdr_info->wordLineResource[index].rowAdr = rowaddr;
	sdr_info->wordLineResource[index].err    = 1;
	sdr_info->wordLineResource[index].sMAT   = sMat;
	sdr_info->wordLineResource[index].bMAT   = bMat;
#endif

	

#ifdef SDR_DBG
	PRN_LOG(fp_dumper, PRN_DBG, "\t/**********ERROR INFO*************/\n");
	PRN_LOG(fp_dumper, PRN_DBG, "\t rowaddr: %d, 0x%x\n", rowaddr, rowaddr);
	PRN_LOG(fp_dumper, PRN_DBG, "\t coladdr: %d, 0x%x\n", coladdr, coladdr);
	PRN_LOG(fp_dumper, PRN_DBG, "\t bMat   : %d, 0x%x\n", bMat, bMat);
	PRN_LOG(fp_dumper, PRN_DBG, "\t sMat   : %d, 0x%x\n", sMat, sMat);
#endif

	return CTX_PROC_STAT;
}

UNSG32 fill_bisr_info(UNSG32 buffer, SDR_BISR_t *bisr_info)
{
	UNSG32	valid_err_dat = buffer && ((1<<21) - 1);
	UNSG32	rowaddr, coladdr;
	UNSG32  stat;

#ifdef CORNER_CASE
	static UNSG32  first_time = 1;

	if ( !first_time) 
		return CTX_PROC_STAT;

	if (first_time) {
		first_time = 0;
		buffer = 0x8FF << 7;
	}
#endif
	
	/*************************Description************************
	 * rowaddr take up 14bits and indicate the count of WL
	 * coladdr take up 7bits and indicate the count of BL
	 ************************************************************/
	rowaddr = (buffer >> 7) & ((1 << 14) - 1);
	coladdr = buffer & ((1 << 7) - 1);

	if ( buffer == 0x00FFFFFF || buffer > 0x1FFFFF)
		return CTX_PROC_STAT;

	if ( rowaddr < 0 || rowaddr > 0x3FFF || coladdr < 0 || coladdr > 0x7F)
	{
		PRN_LOG(fp_dumper, PRN_ERR, "======= The address is out of range! =======\n");
		return ERR_PROC_STAT;
	}


	switch(bisr_info->type) {
	case 1: PRN_LOG(fp_dumper, PRN_DBG, "\t>>>>>>>>> It is Normal Error! >>>>>>>>>\n"); break;
	case 2: PRN_LOG(fp_dumper, PRN_INFO, "\t>>>>>>>>> It is WORD Line Error! >>>>>>>>>\n"); break;
	case 3: PRN_LOG(fp_dumper, PRN_INFO, "\t>>>>>>>>> It is BIT Line Error! >>>>>>>>>\n"); break; 
	case 4: PRN_LOG(fp_dumper, PRN_INFO, "\t>>>>>>>>> It is Cross Error! >>>>>>>>>\n"); 
	}

	if ( bisr_info->type > 1)
		PRN_LOG(fp_dumper, PRN_INFO, "\t======= get SDRAM Err info : %x =======\n", buffer);

	if ( NORMAL_ERR == bisr_info->type ) 
	{
		fill_normal_err(bisr_info->p_normalResource, rowaddr, coladdr);
		bisr_info->normal_err_count++;
		bisr_info->p_normalResource++;
	}
	else if ( WORDLINE_ERR == bisr_info->type )
	{
		fill_WL_redundancy_err(bisr_info->p_wordLineResource, rowaddr, coladdr);
		//bisr_info->p_wordLineResource++;
	}
	else if ( BITLINE_ERR == bisr_info->type )
	{
		fill_BL_redundancy_err(bisr_info->p_bitLineResource, rowaddr, coladdr);
		//bisr_info->p_bitLineResource++;
	}
	else if ( CROSS_ERR == bisr_info->type)
	{
		fill_CROSS_redundancy_err(bisr_info, rowaddr, coladdr);
	}

	return CTX_PROC_STAT;
}

/*********************************************************************
 * @Description:
 *		Step1: Jump to BL Redundancy access mode
 *		Step2: Write values to the first WL of four BL Redundancy
 *		Step3: Jump to normal access mode
 *		Step4: Traverse the first WL of the sMat
 *		Step5: Jump to Step1
 *********************************************************************/
UNSG32 BLRed_used_chk(SDR_BISR_t *bisr_info, UNSG32 rowaddr)
{
	ERR_RESOURCE_INFO_p		blR = bisr_info->bitLineResource;
	ERR_RESOURCE_INFO_p		nlR = bisr_info->normalResource;
	UNSG32					addrC, data, addrD, addr;
	UNSG32					i, j;
	UNSG32					blR_addr;
	
	addrC = SDRC_REGADD + ((bisr_info->SDRID - 1) / 2) * SDRC_REGOFFSET + 0x64;

	for (j = 0; j < 24; j++) 
	{
#if 0
		addrD = 0x15;
		data = 0x718;
		WRITESDR(addrC, data | (addrD << 16) | (1 << 31));
#else
		addrD = 0x15;
		data = 0x670;
		WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

		addrD = 0x10;
		data = 0x7;
		WRITESDR(addrC, data | (addrD << 16) | (1 << 31));
#endif
		
		for (i = 0; i < 4; i++)
		{
			addr = (((j * 0x2B0) << 7) | i) + 0x200000 * ((bisr_info->SDRID - 1)/2);
			WRITESDR(addr<<4, 0x76543210+i);
		}

		addrD = (FUSE_REGADD + 0x5);
		data = 0x658;
		WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

		for (i = 0; i < 128; i++)
		{
			addr = (((j * 0x2B0) << 7) | i) + 0x200000 * ((bisr_info->SDRID - 1)/2);
			data = READSDR(addr<<4);

			if ( 0x7654321 == (data >> 4))
			{
				blR_addr = j + (data & 0xF) * 24;
				blR[blR_addr].last_used = 1;
				blR[blR_addr].redundancy_resource.coladdr = i;
				blR[blR_addr].redundancy_resource.rowaddr = (j * 0x2B0) << 7;

				PRN_LOG(fp_dumper, PRN_INFO, "BLR(sMat:%d,  %d th) repair nlr:%x (row:%x, col:%x)\n", j, data & 0xF, \
					addr, j * 0x2B0, i);
			}
		}
	}
}

/*********************************************************************
 * @Description:
 *		Step1: Jump to WL Redundancy access mode
 *		Step2: Write values to the first BL of four WL Redundancy
 *		Step3: Jump to normal access mode
 *		Step4: Traverse the first WL of the sMat
 *		Step5: Jump to Step1
 *********************************************************************/
UNSG32 WLRed_used_chk(SDR_BISR_t *bisr_info, UNSG32 rowaddr)
{
	ERR_RESOURCE_INFO_p		wlR = bisr_info->wordLineResource;
	ERR_RESOURCE_INFO_p		nlR = bisr_info->normalResource;
	UNSG32					addrC, data, addrD, addr;
	UNSG32					i, j;
	UNSG32					wlR_addr;
	
	addrC = SDRC_REGADD + ((bisr_info->SDRID - 1) / 2) * SDRC_REGOFFSET + 0x64;
	addrD = (FUSE_REGADD + 0x5);
	data = 0x698;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

	for (j = 0; j < 8; j++)  //
	{
		for (i = 0; i < 16; i++) //WL in bMat
		{
			addr = ((i | (j << 11)) << 7) + 0x200000 * ((bisr_info->SDRID - 1)/2);
			WRITESDR(addr<<4, 0x12345600 + i + j*16);
		}
	}

	addrD = (FUSE_REGADD + 0x5);
	data = 0x658;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

	for (i = 0; i < 0x3FFF; i++)
	{
		addr = (i << 7) + 0x200000 * ((bisr_info->SDRID - 1)/2);
		data = READSDR(addr<<4);

		if ( 0x123456 == (data >> 8))
		{
			wlR_addr = (data & 0xFF);
			wlR[wlR_addr].last_used = 1;
			wlR[wlR_addr].redundancy_resource.coladdr = 0;
			wlR[wlR_addr].redundancy_resource.rowaddr = i;
			PRN_LOG(fp_dumper, PRN_INFO, "WLR(bMat:%d,  %d th) repair nlr:%x (row:%x, col:%x)\n", wlR_addr/16, wlR_addr%16,\
				addr, i, 0);
		}
	}
}

/*
UNSG32 WLRed_used_chk(SDR_BISR_t *bisr_info, UNSG32 rowaddr)
{
	ERR_RESOURCE_INFO_p		wlR = bisr_info->wordLineResource;
	UNSG32					wlR_num;
	UNSG32					wlR_reg, wlR_dat;
	UNSG32					addrC, data, addrD;

	addrC = SDRC_REGADD + ((bisr_info->SDRID - 1) / 2) * SDRC_REGOFFSET + 0x64;
	wlR_num = rowaddr; //0~127
	wlR_reg = 0x10F + wlR_num/8*0x10;
	wlR_dat = 0x700 | (1 << wlR_num);

	PRN_LOG(fp_dumper, PRN_DBG, "Word Line Redundancy num:%d, reg:%x, dat:%x\n", wlR_num ,wlR_reg, wlR_dat);

	//test mode entry
	addrD = (FUSE_REGADD + 0xe) << 16;
	data = addrD | 0x30;
	WRITESDR(addrC, data | (1 << 31));

	//access the BL hit readout address
	addrD = (FUSE_REGADD + 0x1) << 16;
	data = addrD | wlR_reg;
	WRITESDR(addrC, data | (1 << 31));

	//
	addrD = (FUSE_REGADD + 0x2) << 16;
	data = addrD | wlR_dat;
	WRITESDR(addrC, data | (1 << 31));

	addrD = (FUSE_REGADD + 0xe) << 16;
	data = addrD;
	WRITESDR(addrC, data | (0 << 31));

	data = READSDR(addrC+8);
	wlR[rowaddr].last_used = !((data & (1 << 12)) >> 12);
	//printf("data:%x, used:%d\n", data, wlR[rowaddr].last_used);

	//exit test mode
	addrD = (FUSE_REGADD + 0xe) << 16;
	data = addrD | 0x0;
	WRITESDR(addrC, data | (1 << 31));	

	return wlR[rowaddr].used;
}
*/

/*********************************************************************
 * @Description:
 *		addr: the normal error address from BIST
 *		sdr_id: the Dram chip id
 *		rp_coladdr: the col addr of BL Red which repair the normal error
 *********************************************************************/
UNSG32 BL_Repair_check(UNSG32 addr, UNSG32 sdr_id, UNSG32 rp_coladdr)
{
	UNSG32 wdata = addr;
	UNSG32 rdata;
	UNSG32 addrR;
	UNSG32 addrD, data, addrC;
	UNSG32 addrAct;

	return 1;

	addrAct = addr + 0x200000 * ((sdr_id - 1)/2);
	WRITESDR(addrAct<<4, wdata);

	//Active BL Redundancy access
	addrC = SDRC_REGADD + ((sdr_id - 1) / 2) * SDRC_REGOFFSET + 0x64;
	PRN_LOG(fp_dumper, PRN_DBG, "\t SDRAM Controller addr:%x\n", addrC);

#if 0
	addrD = 0x15;
	data = 0x718;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));
#else
	addrD = 0x15;
	data = 0x670;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

	addrD = 0x10;
	data = 0x7;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));
#endif

	WRITESDR(addrC, data | (addrD << 16) | (0 << 31));
	rdata = READSDR(addrC + 0x8);
	rdata = READSDR(addrC + 0x8);
	rdata = READSDR(addrC + 0x8);

	Sleep(10);
		
	addrR = (addrAct & 0xFFFFFF80) | (rp_coladdr & 0x3); 
	PRN_LOG(fp_dumper, PRN_INFO, "\t calc redundancy map addr:%x\n", addrR);
	rdata = READSDR(addrR<<4);
	rdata = READSDR(addrR<<4);
	PRN_LOG(fp_dumper, PRN_INFO, "\t read data:%x, write data:%x\n", rdata, wdata);	

	//Exit
	addrD = 0x15;
	data = 0x658;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

	if (rdata == wdata)
		return 1;
	else return 0;
}

UNSG32 WL_Repair_check(UNSG32 addr, UNSG32 sdr_id, UNSG32 rp_rowaddr)
{
	UNSG32 wdata = addr;
	UNSG32 rdata;
	UNSG32 addrR;
	UNSG32 addrD, data, addrC;
	UNSG32 rowH, rowL;

	UNSG32 addrAct;

	return 1;
	
	addrAct = addr + 0x200000 * ((sdr_id - 1)/2);
	WRITESDR(addrAct<<4, wdata);

	//Active BL Redundancy access
	addrC = SDRC_REGADD + ((sdr_id - 1) / 2) * SDRC_REGOFFSET + 0x64;
	PRN_LOG(fp_dumper, PRN_DBG, "\tSDRAM Controller addr:%x\n", addrC);
	addrD = 0x15;
	data = 0x698;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

	WRITESDR(addrC, data | (addrD << 16) | (0 << 31));
	rdata = READSDR(addrC + 0x8);

	Sleep(10);

	//normal error row addr high 3bits and low 4bits
	rowL = rp_rowaddr & 0xF;
	rowH = rp_rowaddr >> 4;
	addrR = (addrAct & 0x7F) | (((rowH << 11) | rowL) << 7);
	PRN_LOG(fp_dumper, PRN_INFO, "\tread Red addr:%x\n", addrR);
	rdata = READSDR(addrR<<4);
	PRN_LOG(fp_dumper, PRN_INFO, "\tread data:%x, write data:%x\n", rdata, wdata);

	//Exit
	addrD = 0x15;
	data = 0x658;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

	if (rdata == wdata)
		return 1;
	else return 0;
}

UNSG32 CL_Repair_check(UNSG32 addr, UNSG32 sdr_id, UNSG32 rp_rowaddr)
{
	UNSG32 wdata = addr;
	UNSG32 rdata;
	UNSG32 addrR;
	UNSG32 addrD, data, addrC;
	UNSG32 rowH, rowL;

	UNSG32 addrAct;

	addrAct = addr + 0x200000 * ((sdr_id - 1)/2);
	WRITESDR(addrAct<<4, wdata);

	//Active BL Redundancy access
	addrC = SDRC_REGADD + ((sdr_id - 1) / 2) * SDRC_REGOFFSET + 0x64;
	PRN_LOG(fp_dumper, PRN_DBG, "\tSDRAM Controller addr:%x\n", addrC);
	addrD = 0x15;
	data = 0x698;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

	WRITESDR(addrC, data | (addrD << 16) | (0 << 31));
	rdata = READSDR(addrC + 0x8);

	Sleep(10);

	//normal error row addr high 3bits and low 4bits
	rowL = rp_rowaddr & 0xF;
	rowH = rp_rowaddr >> 4;
	addrR = (addrAct & 0x7F) | (((rowH << 11) | rowL) << 7);
	PRN_LOG(fp_dumper, PRN_INFO, "\tread Red addr:%x\n", addrR);
	rdata = READSDR(addrR<<4);
	PRN_LOG(fp_dumper, PRN_INFO, "\tread data:%x, write data:%x\n", rdata, wdata);

	//Exit
	addrD = 0x15;
	data = 0x658;
	WRITESDR(addrC, data | (addrD << 16) | (1 << 31));

	if (rdata == wdata)
		return 1;
	else return 0;
}


UNSG32 BitLine_Repair(SDR_BISR_t *bisr_info)
{
	//static UNSG8 sMAT_stat[4*24] = {0};  //store the Redundancy status 
	UNSG32	i, j;
	ERR_RESOURCE_INFO_p		nlR = bisr_info->p_normalResource;
	ERR_RESOURCE_INFO_p		blR = bisr_info->bitLineResource;
	UNSG32	sMat_addr;
	UNSG32  index;
	UNSG32  stat;
	UNSG32  blR_used = 0;
	UNSG32  coladdr_sdr, rowaddr_sdr, smataddr_sdr;

	sMat_addr = nlR->sMAT;
	index     = sMat_addr; 

	PRN_LOG(fp_dumper, PRN_INFO, "\n/*********************** Repair with BitLine ***********************/ \n");
	PRN_LOG(fp_dumper, PRN_INFO, ">>>>>> SDRAM ChipID: %x %x >>>>>>\n", bisr_info->SDRID - 1, bisr_info->SDRID);
	PRN_LOG(fp_dumper, PRN_INFO, ">>>>>> It is Normal Error: 0x%x >>>>>>\n", nlR->colAdr | (nlR->rowAdr << 7));

	/***********************************************************************************************************
	 * Description:
	 *				1. find the small Mat of normal error
	 *				2. find the next BL Redundancy of that small Mat
	 *				3. whether the BL Redundancy in the small Mat is err. Step 4 if no, else step 2.
	 *				4. whether the BL Redundancy of the small Mat is used. Step 5 if used, else step 6.
	 *				5. 当前BL Redundancy是否用来修复同一个小MAT且同一行的其他普通错误资源
	 *     			(whether the BL Redundancy is used for previous normal error unit in the same BL and sMAT). 
	 *				如果是，意味着之前修补的普通错误资源与当前需要修补的普通错误资源在同一个小MAT，并且在同一行，
	 *				所以之前的修补顺带修补了当前错误资源。所以可以退出循环，返回BL_FINISH_STAT。如果不是,step 2
	 *				6. 用当前BL Redundancy修补普通错误资源，并将该BL Redundancy信息更新。
	 **********************************************************************************************************/
	for ( i = 0; i < 4; i++) //loop of Redundancy in a small Mat
	{
#ifdef WLR_ONLY
		blR[index].err = 1;
#endif
		if ( blR[index].err)
		{
			PRN_LOG(fp_dumper, PRN_INFO, "======  This BL Redundancy @%d of sMat @%d is Err!\n", i, sMat_addr);
			index = index + 24;
			stat = BL_ERR_STAT;
		}
		else if ( blR[index].last_used)
		{
			PRN_LOG(fp_dumper, PRN_INFO, "======  This BL Redundancy @%d of sMat @%d is last used!\n", i, sMat_addr);
			stat = BL_ERR_STAT;
			index = index + 24;
		}
		else if ( blR[index].used)
		{
			//Whether the unit of BL is Repaired by previous unit in the same BL and sMat
			coladdr_sdr = blR[index].redundancy_resource.coladdr;
			rowaddr_sdr = blR[index].redundancy_resource.rowaddr;
			smataddr_sdr = blR[index].redundancy_resource.sMataddr;
			blR_used = 1;

			blR[index].hasCross |= BLREPAIR;

			if ( coladdr_sdr == nlR->colAdr && smataddr_sdr == nlR->sMAT)
			{
				PRN_LOG(fp_dumper, PRN_INFO, "======  This BL Redundancy has repaired the col @%d of this sMat @%d!\n", coladdr_sdr, smataddr_sdr);
				PRN_LOG(fp_dumper, PRN_DBG, "/**************Repair Info*****************/\n");
				PRN_LOG(fp_dumper, PRN_DBG, "===== err rowaddr: %d, 0x%x\n", nlR->rowAdr, nlR->rowAdr);
				PRN_LOG(fp_dumper, PRN_DBG, "===== err coladdr: %d, 0x%x\n", nlR->colAdr, nlR->colAdr);
				PRN_LOG(fp_dumper, PRN_DBG, "===== err sMat   : %d, 0x%x\n", nlR->sMAT, nlR->sMAT);
				PRN_LOG(fp_dumper, PRN_DBG, "===== redundancy: BitLine sMat :%d, %d th BL\n", nlR->sMAT, i);
				stat = BL_FINISH_STAT; 
				break;
			}
			else 
			{
				PRN_LOG(fp_dumper, PRN_INFO, "======  This BL Redundancy @%d of sMat @%d is used!\n", i, sMat_addr);
				index = index + 24;
				stat = BL_ERR_STAT;
			}
		}
		else
		{
			PRN_LOG(fp_dumper, PRN_INFO, "======  This BL Redundancy @%d of sMat @%d is OK!\n", i, sMat_addr);
			PRN_LOG(fp_dumper, PRN_INFO, "/**************Repair Info*****************/\n");
			PRN_LOG(fp_dumper, PRN_INFO, "===== err rowaddr: %d, 0x%x\n", nlR->rowAdr, nlR->rowAdr);
			PRN_LOG(fp_dumper, PRN_INFO, "===== err coladdr: %d, 0x%x\n", nlR->colAdr, nlR->colAdr);
			PRN_LOG(fp_dumper, PRN_INFO, "===== err sMat   : %d, 0x%x\n", nlR->sMAT, nlR->sMAT);
			PRN_LOG(fp_dumper, PRN_INFO, "===== redundancy: BitLine sMat :%d, %d th BL\n", nlR->sMAT, i);

			blR[index].redundancy_resource.coladdr = nlR->colAdr;
			blR[index].redundancy_resource.rowaddr = nlR->rowAdr;
			blR[index].redundancy_resource.sMataddr = nlR->sMAT;
			blR[index].used = 1;
			blR[index].hasCross |= BLREPAIR;
			//sMAT_stat[index] = 1;
			stat = BL_FINISH_STAT;

			break;
		}
	}


	return stat;	
}

UNSG32 WordLine_Repair(SDR_BISR_t *bisr_info)
{
	UNSG32	i, j;
	ERR_RESOURCE_INFO_p		nlR = bisr_info->p_normalResource;
	ERR_RESOURCE_INFO_p		wlR = bisr_info->wordLineResource;
	ERR_RESOURCE_INFO_p		blR = bisr_info->bitLineResource;
	UNSG32	sMat_addr;
	UNSG32  bMat_addr;
	UNSG32  index, index_wl_redundancy;
	UNSG32  stat = WL_REPAIR_STAT;
	UNSG32  coladdr_sdr, rowaddr_sdr;

	bMat_addr = nlR->bMAT;
	sMat_addr = nlR->sMAT;
	index     = bMat_addr + 1; 
	index     = (index == 8) ? 0 : index;

	PRN_LOG(fp_dumper, PRN_INFO, "\n/*********************** Repair with WordLine ***********************/ \n");
	PRN_LOG(fp_dumper, PRN_INFO, ">>>>>> SDRAM ChipID: %x %x >>>>>>\n", bisr_info->SDRID - 1, bisr_info->SDRID);
	PRN_LOG(fp_dumper, PRN_INFO, ">>>>>> It is Normal Error: 0x%x >>>>>>\n", nlR->colAdr | (nlR->rowAdr << 7));

	/***********************************************************************************************************
	 * Description： 只有BL Redundancy修补不了，才需要用WL Redundancy修补
	 *				1. find the big Mat of normal error
	 *				2. find the next WL Redundancy of other big Mat
	 *				3. whether the WL Redundancy in the big Mat is err. Step 4 if no, else step 2.
	 *				4. whether the WL Redundancy of the big Mat is used. Step 5 if used, else step 6.
	 *				5. 当前WL Redundancy是否用来修复同一个同一列的其他普通错误资源
	 *     			(whether the WL Redundancy is used for previous normal error unit in the same col). 
	 *				如果是，意味着之前修补的普通错误资源与当前需要修补的普通错误资源在同一列，
	 *				所以之前的修补顺带修补了当前错误资源。所以可以退出循环，返回WL_FINISH_STAT。如果不是,step 2
	 *				6. 用当前WL Redundancy修补普通错误资源，并将该WL Redundancy信息更新
	 ***********************************************************************************************************/
	for ( i = index; i != bMat_addr; i = (++i)%8) //loop of WL Redundancy in a big MAT, index..7, 0~index-1
	{
		if ( stat == WL_FINISH_STAT)
			break;

		for ( j = 0; j < 16; j++)
		{
			index_wl_redundancy = index * 16 + j;
#ifdef ReRepair
			//WLRed_used_chk(bisr_info, index_wl_redundancy);
#endif
			if ( wlR[index_wl_redundancy].err)
			{
				PRN_LOG(fp_dumper, PRN_ERR, "======  This WL Redundancy @%d is Err!\n", index_wl_redundancy);
				stat = ERR_REPAIR_STAT;
			}
			else if ( wlR[index_wl_redundancy].last_used)
			{
				PRN_LOG(fp_dumper, PRN_INFO, "======  This WL Redundancy @%d is last used!\n", index_wl_redundancy);
				stat = ERR_REPAIR_STAT;
			}
			else if ( wlR[index_wl_redundancy].used)
			{
				//Whether the unit of WL is Repaired by previous unit in the same WL
				coladdr_sdr = wlR[index_wl_redundancy].redundancy_resource.coladdr;
				rowaddr_sdr = wlR[index_wl_redundancy].redundancy_resource.rowaddr;

				if ( blR[sMat_addr].hasCross && (blR[sMat_addr].used || blR[sMat_addr + 24].used \
					|| blR[sMat_addr + 48].used || blR[sMat_addr + 72].used)) 
				{
					blR[sMat_addr].hasCross |= WLREPAIR;
					blR[sMat_addr].hasCross |= (rowaddr_sdr & 0xFFFF) << 8;

					blR[sMat_addr+24].hasCross |= WLREPAIR;
					blR[sMat_addr+24].hasCross |= (rowaddr_sdr & 0xFFFF) << 8;

					blR[sMat_addr+48].hasCross |= WLREPAIR;
					blR[sMat_addr+48].hasCross |= (rowaddr_sdr & 0xFFFF) << 8;

					blR[sMat_addr+72].hasCross |= WLREPAIR;
					blR[sMat_addr+72].hasCross |= (rowaddr_sdr & 0xFFFF) << 8;
				}

				if ( rowaddr_sdr == nlR->rowAdr)
				{
					stat = WL_FINISH_STAT; 
					PRN_LOG(fp_dumper, PRN_INFO, "======  This WL Redundancy has repaired the WL @%d!\n", rowaddr_sdr);
					PRN_LOG(fp_dumper, PRN_INFO, "/**************Repair Info*****************/\n");
					PRN_LOG(fp_dumper, PRN_INFO, "===== err rowaddr: %d, 0x%x\n", nlR->rowAdr, nlR->rowAdr);
					PRN_LOG(fp_dumper, PRN_INFO, "===== err coladdr: %d, 0x%x\n", nlR->colAdr, nlR->colAdr);
					PRN_LOG(fp_dumper, PRN_INFO, "===== err bMat   : %d, 0x%x\n", nlR->bMAT, nlR->bMAT);
					PRN_LOG(fp_dumper, PRN_INFO, "===== redundancy: WordLine bMat :%d, %d th WL\n", i, j);
					break;
				}
				else 
				{
					PRN_LOG(fp_dumper, PRN_INFO, "======  This WL Redundancy @%d is used!\n", index_wl_redundancy);
					stat = ERR_REPAIR_STAT;
				}
			}
			else
			{
				PRN_LOG(fp_dumper, PRN_INFO, "======  This WL Redundancy @%d is OK!\n", index_wl_redundancy);
				PRN_LOG(fp_dumper, PRN_INFO, "/**************Repair Info*****************/\n");
				PRN_LOG(fp_dumper, PRN_INFO, "===== err rowaddr: %d, 0x%x\n", nlR->rowAdr, nlR->rowAdr);
				PRN_LOG(fp_dumper, PRN_INFO, "===== err coladdr: %d, 0x%x\n", nlR->colAdr, nlR->colAdr);
				PRN_LOG(fp_dumper, PRN_INFO, "===== err bMat   : %d, 0x%x\n", nlR->bMAT, nlR->bMAT);
				PRN_LOG(fp_dumper, PRN_INFO, "===== redundancy: WordLine bMat :%d, %d th WL\n", i, j);

				wlR[index_wl_redundancy].redundancy_resource.coladdr = nlR->colAdr;
				wlR[index_wl_redundancy].redundancy_resource.rowaddr = nlR->rowAdr;
				wlR[index_wl_redundancy].redundancy_resource.sMataddr = nlR->bMAT;
				wlR[index_wl_redundancy].used = 1;

				if ( blR[sMat_addr].hasCross && (blR[sMat_addr].used || blR[sMat_addr + 24].used \
					|| blR[sMat_addr + 48].used || blR[sMat_addr + 72].used)) 
				{
					blR[sMat_addr].hasCross |= WLREPAIR;
					blR[sMat_addr].hasCross |= (nlR->rowAdr & 0xFFFF) << 8;

					blR[sMat_addr+24].hasCross |= WLREPAIR;
					blR[sMat_addr+24].hasCross |= (nlR->rowAdr & 0xFFFF) << 8;

					blR[sMat_addr+48].hasCross |= WLREPAIR;
					blR[sMat_addr+48].hasCross |= (nlR->rowAdr & 0xFFFF) << 8;

					blR[sMat_addr+72].hasCross |= WLREPAIR;
					blR[sMat_addr+72].hasCross |= (nlR->rowAdr & 0xFFFF) << 8;
				}
				//sMAT_stat[index] = 1;
				stat = WL_FINISH_STAT;

				break;
			}
		}
	}
		
	return stat;	
}

/*************************Description*********************************************
 * Stat Machine description:
 * --------------     -----------     ----------     -----------	 --------------
 * |START REPAIR| --> |BL REPAIR| --> |BL ERROR| --> |WL REPAIR| --> |FINISH/ERROR|
 * --------------	  -----------	  ----------     -----------	 --------------
 *		^				   |							   |
 *	    |   -----------	   |							   |
 *	    --- |BL FINISH| <--|							   |
 *	    |   -----------					----------- 	   |				   
 *		------------------------------- |WL FINISH|<--------
 *										-----------
 *********************************************************************************/
UNSG32 SDR_Redundancy(SDR_BISR_t *bisr_info)
{
	UNSG8	BLR_stat[24][4] = {0};
	UNSG32  repair_stat = START_REPAIR_STAT;
	UNSG32	i, j;
	UNSG32	err_cnt = 0;

	bisr_info->p_normalResource = bisr_info->normalResource;

	do {
		if ( err_cnt == bisr_info->normal_err_count)
		{
			PRN_LOG(fp_dumper, PRN_DBG, "\n/*********************Repair Finish and Successfully!***********************/\n");
			repair_stat = FINISH_REPAIR_STAT;
		}

		if ( FINISH_REPAIR_STAT == repair_stat || ERR_REPAIR_STAT == repair_stat)
			return repair_stat;

		if ( START_REPAIR_STAT == repair_stat)
			repair_stat = BL_REPAIR_STAT;
		else if ( BL_FINISH_STAT == repair_stat)
			repair_stat = START_REPAIR_STAT;
		else if ( BL_ERR_STAT == repair_stat)
			repair_stat = WL_REPAIR_STAT;
		else if ( WL_FINISH_STAT == repair_stat)
			repair_stat = START_REPAIR_STAT;

		if ( BL_REPAIR_STAT == repair_stat)
		{
			repair_stat = BitLine_Repair(bisr_info);
			
		}
		else if ( WL_REPAIR_STAT == repair_stat)
		{
			repair_stat = WordLine_Repair(bisr_info);
		}

		if ( repair_stat == WL_FINISH_STAT || repair_stat == BL_FINISH_STAT)
		{
			bisr_info->p_normalResource++;
			err_cnt++;
		}
		
	}while(1);
}

void PrepareSDR(UNSG32 addr_c)
{
	UNSG32	data,  dat_H;
	UNSG32  i;
	UNSG32  addr = SDRC_REGADD + 0x64;

	for(i = 0; i < 3; i++)
	{
		dat_H = (FUSE_REGADD + 0x0E) << 16;
		data = 0x27 | dat_H;
		WRITESDR(addr, data | (1 << 31));

#ifdef FT_TEST
		//revised by Sheldon, do not need to open pump in CP (still needed in FT)
		dat_H = (FUSE_REGADD + 0x03) << 16;
		data = 0x26 | dat_H;
		WRITESDR(addr, data | (1 << 31));
#endif
		Sleep(10); //wait 10ms

		dat_H = (FUSE_REGADD + 0x02) << 16;
		data = 0x700 | dat_H;
		//data = 0x500 | dat_H;
		WRITESDR(addr, data | (1 << 31));

		Sleep(10);
		addr = addr + SDRC_REGOFFSET;
	}
	
}

void ExitSDR(UNSG32 addr_c)
{
	UNSG32	data,  dat_H, i;
	UNSG32  addr = SDRC_REGADD + 0x64;

	for(i = 0; i < 3; i++)
	{
		dat_H = (FUSE_REGADD + 0x0E) << 16;
		data = 0 | dat_H;
		WRITESDR(addr, data | (1 << 31));

		addr = addr + SDRC_REGOFFSET;
	}
}

void WRITE_TO_SDR(UNSG32 addr, UNSG32 rpadr, UNSG32 rpdat)
{
	UNSG32	data,  dat_H, dat_bit;
	UNSG32  i = 0;

	if (rpdat == 0)
		return;
	//PRN_LOG(fp_dumper, "repair addr:%x, dat:%x\n", rpadr, rpdat);
	
	//addr = SDRC_REGADD + 0x64;

	dat_H = (FUSE_REGADD + 0x01) << 16;
	data = rpadr | dat_H;
	WRITESDR(addr, data | (1 << 31));
	printf("fuse addr:0x%x, dat:0x%x\n", rpadr, rpdat);

#ifdef FT_TEST
	dat_H = (FUSE_REGADD + 0x02) << 16;
	//rpdat = 0x36;
	do {
		dat_H = (FUSE_REGADD + 0x02) << 16;
		dat_bit = rpdat & (1 << i);
		data = dat_bit | dat_H | (0x7 << 8);
		//data = dat_bit | dat_H | (0x5 << 8);
		WRITESDR(addr, data | (1 << 31));
		printf("fuse dat bit shift:0x%x\n", dat_bit);

		dat_H = (FUSE_REGADD + 0x0E) << 16;
		data = 0x7 | dat_H;
		WRITESDR(addr, data | (1 << 31));

		Sleep(10);

		dat_H = (FUSE_REGADD + 0x0E) << 16;
		data = 0x27 | dat_H;
		WRITESDR(addr, data | (1 << 31));
	}while((i++) < 7);
#else
	dat_H = (FUSE_REGADD + 0x02) << 16;
	data = rpdat | dat_H | (0x7 << 8);
	WRITESDR(addr, data | (1 << 31));

	dat_H = (FUSE_REGADD + 0x0E) << 16;
	data = 0x7 | dat_H;
	WRITESDR(addr, data | (1 << 31));

	Sleep(10);

	dat_H = (FUSE_REGADD + 0x0E) << 16;
	data = 0x27 | dat_H;
	WRITESDR(addr, data | (1 << 31));
#endif
}

static UNSG32 colFuse[96][2] = {0};
static UNSG32 rowFuse[256][2] = {0};
void Do_Map(SDR_BISR_P bisr_info)
{
	ERR_RESOURCE_INFO_p		p_blR = bisr_info->bitLineResource;
	ERR_RESOURCE_INFO_p		p_wlR = bisr_info->wordLineResource;
	UNSG32 colRed[24][4] = {0};
	UNSG32 rowRed[128] = {0};
	
	UNSG32	i, j, k;

	//restore the redundancy information
	for ( j = 0; j < 4; j++)
		for ( i = 0; i < 24; i++)
			colRed[i][j] = (p_blR->used << 7) | (p_blR++)->redundancy_resource.coladdr;
	

	for ( i = 0; i < 128; i++)
		rowRed[i] = (p_wlR->used << 14) | (p_wlR++)->redundancy_resource.rowaddr;

	colRepair(colFuse, colRed);
	rowRepair(rowFuse, rowRed);


	PRN_LOG(fp_dumper, PRN_INFO, "colRed:  C0 C1 C2 C3\n");
	for(i=0;i<24;i++){
		PRN_LOG(fp_dumper, PRN_INFO, "MAT%2d:   ",i);
		for(j=0;j<4;j++){
			PRN_LOG(fp_dumper, PRN_INFO, "%+2X ",colRed[i][j]);
		}
		PRN_LOG(fp_dumper, PRN_INFO, "\n");
	}
	PRN_LOG(fp_dumper, PRN_INFO, "\n");
	k = 0;
	PRN_LOG(fp_dumper, PRN_INFO, "colFuse:\nADDR: DATA: \n");
	for(i=0;i<12;i++){
		for(j=0;j<8;j++){
			PRN_LOG(fp_dumper, PRN_INFO, "%+2X ",colFuse[k][0]);
			PRN_LOG(fp_dumper, PRN_INFO, "%+2X | ",colFuse[k][1]);
			k++;
		}
		PRN_LOG(fp_dumper, PRN_INFO, "\n");
	}
	PRN_LOG(fp_dumper, PRN_INFO, "\n");

	//print row results
	k = 0;
	PRN_LOG(fp_dumper, PRN_INFO, "rowRed:\n");
	for(i=0;i<16;i++){
		for(j=0;j<8;j++){
			PRN_LOG(fp_dumper, PRN_INFO, "%+4X ",rowRed[k]);
			k++;
		}
		PRN_LOG(fp_dumper, PRN_INFO, "\n");
	}
	PRN_LOG(fp_dumper, PRN_INFO, "\n");
	k = 0;
	PRN_LOG(fp_dumper, PRN_INFO, "rowFuse:\nADDR: DATA: \n");
	for(i=0;i<32;i++){
		for(j=0;j<8;j++){
			PRN_LOG(fp_dumper, PRN_INFO, "%+3X ",rowFuse[k][0]);
			PRN_LOG(fp_dumper, PRN_INFO, "%+2X | ",rowFuse[k][1]);
			k++;
		}
		PRN_LOG(fp_dumper, PRN_INFO, "\n");
	}
}

void Do_Repair(SDR_BISR_P bisr_info)
{
	UNSG32 i;
	UNSG32 addr;

	addr = SDRC_REGADD + ((bisr_info->SDRID - 1) / 2) * SDRC_REGOFFSET + 0x64;

	//PrepareSDR(addr);

	//to update
	for ( i = 0; i < 0x5F; i++)
		WRITE_TO_SDR(addr, colFuse[i][0], colFuse[i][1]);

	for ( i = 0; i < 0xFF; i++)
		WRITE_TO_SDR(addr, rowFuse[i][0], rowFuse[i][1]);

}