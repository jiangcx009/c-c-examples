#include "bisr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

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
	PRN_LOG(PRN_DBG, "/**********ERROR INFO*************/\n");
	PRN_LOG(PRN_DBG, "\t rowaddr: %d, 0x%x\n", rowaddr, rowaddr);
	PRN_LOG(PRN_DBG, "\t coladdr: %d, 0x%x\n", coladdr, coladdr);
	PRN_LOG(PRN_DBG, "\t bMat   : %d, 0x%x\n", bMat, bMat);
	PRN_LOG(PRN_DBG, "\t sMat   : %d, 0x%x\n", sMat, sMat);
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
		PRN_LOG(PRN_ERR, "WordLine Redundancy addr is error!\n");
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
		PRN_LOG(PRN_INFO, "Have fill the buffer!\n");
		return CTX_PROC_STAT;
	}

	wlR[index].colAdr = coladdr;
	wlR[index].rowAdr = rowaddr;
	wlR[index].err    = 1;
	wlR[index].bMAT   = rowaddr/8;

#ifdef SDR_DBG
	PRN_LOG(PRN_DBG, "/**********ERROR INFO*************/\n");
	PRN_LOG(PRN_DBG, "\t rowaddr: %d, 0x%x\n", rowaddr, rowaddr);
	PRN_LOG(PRN_DBG, "\t coladdr: %d, 0x%x\n", coladdr, coladdr);
	PRN_LOG(PRN_DBG, "\t bMat   : %d, 0x%x\n", rowaddr/8, rowaddr/8);
	PRN_LOG(PRN_DBG, "\t sMat   : %d, 0x%x\n", 0, 0);
#endif

	return CTX_PROC_STAT;
}

/****************************************************************
 * Description:
 *				If the BitLine Redundancy is err, fill the 
 *				buffer and a bit line of a small Mat use a buffer
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
		PRN_LOG(PRN_ERR, "BitLine Redundancy is error!\n");
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
	PRN_LOG(PRN_DBG, "/**********ERROR INFO*************/\n");
	PRN_LOG(PRN_DBG, "\t rowaddr: %d, 0x%x\n", rowaddr, rowaddr);
	PRN_LOG(PRN_DBG, "\t coladdr: %d, 0x%x\n", coladdr, coladdr);
	PRN_LOG(PRN_DBG, "\t bMat   : %d, 0x%x\n", bMat, bMat);
	PRN_LOG(PRN_DBG, "\t sMat   : %d, 0x%x\n", sMat, sMat);
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
	index = col_index * 24 + sMat;

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
	PRN_LOG(PRN_DBG, "/**********ERROR INFO*************/\n");
	PRN_LOG(PRN_DBG, "\t rowaddr: %d, 0x%x\n", rowaddr, rowaddr);
	PRN_LOG(PRN_DBG, "\t coladdr: %d, 0x%x\n", coladdr, coladdr);
	PRN_LOG(PRN_DBG, "\t bMat   : %d, 0x%x\n", bMat, bMat);
	PRN_LOG(PRN_DBG, "\t sMat   : %d, 0x%x\n", sMat, sMat);
#endif

	return CTX_PROC_STAT;
}

UNSG32 fill_bisr_info(UNSG32 buffer, SDR_BISR_t *bisr_info)
{
	UNSG32	valid_err_dat = buffer && ((1<<21) - 1);
	UNSG32	rowaddr, coladdr;
	UNSG32  stat;
	
	/*************************Description************************
	 * rowaddr take up 14bits and indicate the count of WL
	 * coladdr take up 7bits and indicate the count of BL
	 ************************************************************/
	rowaddr = (buffer >> 7) & ((1 << 14) - 1);
	coladdr = buffer & ((1 << 7) - 1);

	if ( rowaddr < 0 || rowaddr > 0x3FFF || coladdr < 0 || coladdr > 0x7F)
	{
		PRN_LOG(PRN_ERR, "======= The address is out of range! =======\n");
		return ERR_PROC_STAT;
	}

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

UNSG32 BitLine_Repair(SDR_BISR_t *bisr_info)
{
	//static UNSG8 sMAT_stat[4*24] = {0};  //store the Redundancy status 
	UNSG32	i, j;
	ERR_RESOURCE_INFO_p		nlR = bisr_info->p_normalResource;
	ERR_RESOURCE_INFO_p		blR = bisr_info->bitLineResource;
	UNSG32	sMat_addr;
	UNSG32  index;
	UNSG32  stat;
	UNSG32  coladdr_sdr, rowaddr_sdr, smataddr_sdr;

	sMat_addr = nlR->sMAT;
	index     = sMat_addr; 

	PRN_LOG(PRN_INFO, "\n/************Repair with BitLine**************/\n");

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
		if ( blR[index].err)
		{
			PRN_LOG(PRN_INFO, "======  This BL Redundancy @%d of sMat @%d is Err!\n", i, sMat_addr);
			index = index + 24;
			stat = BL_ERR_STAT;
		}
		else if ( blR[index].used)
		{
			//Whether the unit of BL is Repaired by previous unit in the same BL and sMat
			coladdr_sdr = blR[index].redundancy_resource.coladdr;
			rowaddr_sdr = blR[index].redundancy_resource.rowaddr;
			smataddr_sdr = blR[index].redundancy_resource.sMataddr;

			if ( coladdr_sdr == nlR->colAdr && smataddr_sdr == nlR->sMAT)
			{
				PRN_LOG(PRN_INFO, "======  This BL Redundancy has repaired the col @%d of this sMat @%d!\n", coladdr_sdr, smataddr_sdr);
				PRN_LOG(PRN_DBG, "/**************Repair Info*****************/\n");
				PRN_LOG(PRN_DBG, "===== err rowaddr: %d, 0x%x\n", nlR->rowAdr, nlR->rowAdr);
				PRN_LOG(PRN_DBG, "===== err coladdr: %d, 0x%x\n", nlR->colAdr, nlR->colAdr);
				PRN_LOG(PRN_DBG, "===== err sMat   : %d, 0x%x\n", nlR->sMAT, nlR->sMAT);
				PRN_LOG(PRN_DBG, "===== redundancy: BitLine sMat :%d, %d th BL\n", nlR->sMAT, i);
				stat = BL_FINISH_STAT; 
				break;
			}
			else 
			{
				PRN_LOG(PRN_INFO, "======  This BL Redundancy @%d of sMat @%d is used!\n", i, sMat_addr);
				index = index + 24;
				stat = BL_ERR_STAT;
			}
		}
		else
		{
			PRN_LOG(PRN_DBG, "======  This BL Redundancy @%d of sMat @%d is OK!\n", i, sMat_addr);
			PRN_LOG(PRN_DBG, "/**************Repair Info*****************/\n");
			PRN_LOG(PRN_DBG, "===== err rowaddr: %d, 0x%x\n", nlR->rowAdr, nlR->rowAdr);
			PRN_LOG(PRN_DBG, "===== err coladdr: %d, 0x%x\n", nlR->colAdr, nlR->colAdr);
			PRN_LOG(PRN_DBG, "===== err sMat   : %d, 0x%x\n", nlR->sMAT, nlR->sMAT);
			PRN_LOG(PRN_DBG, "===== redundancy: BitLine sMat :%d, %d th BL\n", nlR->sMAT, i);

			blR[index].redundancy_resource.coladdr = nlR->colAdr;
			blR[index].redundancy_resource.rowaddr = nlR->rowAdr;
			blR[index].redundancy_resource.sMataddr = nlR->sMAT;
			blR[index].used = 1;
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
	ERR_RESOURCE_INFO_p     clR = bisr_info->p_crossResource;
	UNSG32	sMat_addr;
	UNSG32  bMat_addr;
	UNSG32  index, index_wl_redundancy;
	UNSG32  stat = WL_REPAIR_STAT;
	UNSG32  coladdr_sdr, rowaddr_sdr;

	bMat_addr = nlR->bMAT;
	index     = bMat_addr + 1; 
	index     = (index == 8) ? 0 : index;

	PRN_LOG(PRN_INFO, "\n/************Repair with WordLine**************/\n");

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
			if ( wlR[index_wl_redundancy].err)
			{
				PRN_LOG(PRN_ERR, "======  This WL Redundancy @%d is Err!\n", index_wl_redundancy);
				stat = ERR_REPAIR_STAT;
			}
			else if ( wlR[index_wl_redundancy].used)
			{
				//Whether the unit of WL is Repaired by previous unit in the same WL
				coladdr_sdr = wlR[index_wl_redundancy].redundancy_resource.coladdr;
				rowaddr_sdr = wlR[index_wl_redundancy].redundancy_resource.rowaddr;

				if ( rowaddr_sdr == nlR->rowAdr)
				{
					stat = WL_FINISH_STAT; 
					PRN_LOG(PRN_DBG, "======  This WL Redundancy has repaired the WL @%d!\n", rowaddr_sdr);
					PRN_LOG(PRN_DBG, "/**************Repair Info*****************/\n");
					PRN_LOG(PRN_DBG, "===== err rowaddr: %d, 0x%x\n", nlR->rowAdr, nlR->rowAdr);
					PRN_LOG(PRN_DBG, "===== err coladdr: %d, 0x%x\n", nlR->colAdr, nlR->colAdr);
					PRN_LOG(PRN_DBG, "===== err bMat   : %d, 0x%x\n", nlR->bMAT, nlR->bMAT);
					PRN_LOG(PRN_DBG, "===== redundancy: WordLine bMat :%d, %d th WL\n", i, j);
					break;
				}
				else 
				{
					PRN_LOG(PRN_INFO, "======  This WL Redundancy @%d is used!\n", index_wl_redundancy);
					stat = ERR_REPAIR_STAT;
				}
			}
			else
			{
				PRN_LOG(PRN_DBG, "======  This WL Redundancy @%d is OK!\n", index_wl_redundancy);
				PRN_LOG(PRN_DBG, "/**************Repair Info*****************/\n");
				PRN_LOG(PRN_DBG, "===== err rowaddr: %d, 0x%x\n", nlR->rowAdr, nlR->rowAdr);
				PRN_LOG(PRN_DBG, "===== err coladdr: %d, 0x%x\n", nlR->colAdr, nlR->colAdr);
				PRN_LOG(PRN_DBG, "===== err bMat   : %d, 0x%x\n", nlR->bMAT, nlR->bMAT);
				PRN_LOG(PRN_DBG, "===== redundancy: WordLine bMat :%d, %d th WL\n", i, j);

				wlR[index_wl_redundancy].redundancy_resource.coladdr = nlR->colAdr;
				wlR[index_wl_redundancy].redundancy_resource.rowaddr = nlR->rowAdr;
				wlR[index_wl_redundancy].redundancy_resource.sMataddr = nlR->bMAT;
				wlR[index_wl_redundancy].used = 1;
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
			PRN_LOG(PRN_INFO, "\n/*********************Repair Finish and Successfully!***********************/\n");
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

void PrepareSDR(UNSG32 addr)
{
	UNSG32	data,  dat_H;

	dat_H = (FUSE_REGADD + 0x0E) << 16;
	data = 0x27 | dat_H;
	WRITESDR(addr, data | (1 << 31));

	dat_H = (FUSE_REGADD + 0x03) << 16;
	data = 0x26 | dat_H;
	WRITESDR(addr, data | (1 << 31));

	Sleep(10); //wait 10ms

	dat_H = (FUSE_REGADD + 0x02) << 16;
	data = 0x700 | dat_H;
	WRITESDR(addr, data | (1 << 31));

	Sleep(1);
}

void ExitSDR(UNSG32 addr)
{
	UNSG32	data,  dat_H;

	addr = SDRC_REGADD + 0x64;

	dat_H = (FUSE_REGADD + 0x0E) << 16;
	data = 0 | dat_H;
	WRITESDR(addr, data | (1 << 31));
}

void WRITE_TO_SDR(UNSG32 addr, UNSG32 rpadr, UNSG32 rpdat)
{
	UNSG32	data,  dat_H;
	//PRN_LOG("repair addr:%x, dat:%x\n", rpadr, rpdat);
	
	addr = SDRC_REGADD + 0x64;

	dat_H = (FUSE_REGADD + 0x01) << 16;
	data = rpadr | dat_H;
	WRITESDR(addr, data | (1 << 31));

	dat_H = (FUSE_REGADD + 0x02) << 16;
	data = rpdat | dat_H;
	WRITESDR(addr, data | (1 << 31));

	dat_H = (FUSE_REGADD + 0x0E) << 16;
	data = 0x7 | dat_H;
	WRITESDR(addr, data | (1 << 31));

	Sleep(10);

	dat_H = (FUSE_REGADD + 0x0E) << 16;
	data = 0x27 | dat_H;
	WRITESDR(addr, data | (1 << 31));
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


	PRN_LOG(PRN_DBG, "colRed:  C0 C1 C2 C3\n");
	for(i=0;i<24;i++){
		PRN_LOG(PRN_DBG, "MAT%2d:   ",i);
		for(j=0;j<4;j++){
			PRN_LOG(PRN_DBG, "%+2X ",colRed[i][j]);
		}
		PRN_LOG(PRN_DBG, "\n");
	}
	PRN_LOG(PRN_DBG, "\n");
	k = 0;
	PRN_LOG(PRN_DBG, "colFuse:\nADDR: DATA: \n");
	for(i=0;i<12;i++){
		for(j=0;j<8;j++){
			PRN_LOG(PRN_DBG, "%+2X ",colFuse[k][0]);
			PRN_LOG(PRN_DBG, "%+2X | ",colFuse[k][1]);
			k++;
		}
		PRN_LOG(PRN_DBG, "\n");
	}
	PRN_LOG(PRN_DBG, "\n");

	//print row results
	k = 0;
	PRN_LOG(PRN_DBG, "rowRed:\n");
	for(i=0;i<16;i++){
		for(j=0;j<8;j++){
			PRN_LOG(PRN_DBG, "%+4X ",rowRed[k]);
			k++;
		}
		PRN_LOG(PRN_DBG, "\n");
	}
	PRN_LOG(PRN_DBG, "\n");
	k = 0;
	PRN_LOG(PRN_DBG, "rowFuse:\nADDR: DATA: \n");
	for(i=0;i<32;i++){
		for(j=0;j<8;j++){
			PRN_LOG(PRN_DBG, "%+3X ",rowFuse[k][0]);
			PRN_LOG(PRN_DBG, "%+2X | ",rowFuse[k][1]);
			k++;
		}
		PRN_LOG(PRN_DBG, "\n");
	}
}

void Do_Repair(SDR_BISR_P bisr_info)
{
	UNSG32 i;
	UNSG32 addr;

	addr = SDRC_REGADD + ((bisr_info->SDRID - 1) / 2) * SDRC_REGOFFSET + 0x64;

	PrepareSDR(addr);

	//to update
	for ( i = 0; i < 96; i++)
		WRITE_TO_SDR(addr, colFuse[i][0], colFuse[i][1]);

	for ( i = 0; i < 128; i++)
		WRITE_TO_SDR(addr, rowFuse[i][0], rowFuse[i][1]);

	ExitSDR(addr);
}