

#include "diag_dbg.h"

#ifdef __USBJTAG__
#include "usbjtag.h"
#else
//#include "jtag.h"
#endif

void diag_dbg_init_cp(void)
{
#ifdef DAEMON_BOARD
	int res;
	UNSG32 addr, value;

	addr = 0xbe0900c0;
	value = 0x200000e3;
	res = CCOC_CP_mem_write((void*)addr, (void*)&value, 4);
	printf("CP init %s\n", (res == 0)? "succeed" : "failed");
#endif
}

void diag_dbg_flush_cp(void)
{
#ifdef DAEMON_BOARD
	int res;
	UNSG32 addr, value;

	addr = 0xfe0900c8;
	value = 0x1;
	res = CCOC_CP_mem_write((void*)addr, (void*)&value, 4);
	printf("CP flush %s\n", (res == 0)? "succeed" : "failed");
#endif
}

void diag_dbg_help()
{
	fprintf(stdout, "\n DIAG FPGA commands: \n\n");
	fprintf(stdout, "read:  r addr [len]\n");
	fprintf(stdout, "write: w addr val [len]\n");
	fprintf(stdout, "burstread:  R addr [len]\n");
	fprintf(stdout, "burstwrite: W addr val [len]\n");
	fprintf(stdout, "dump:  d addr len fname\n");
	fprintf(stdout, "help:  h\n");
	fprintf(stdout, "quit:  q\n");
	fprintf(stdout, "initCP:  i\n");
	fprintf(stdout, "\n");
	return;
}

SIGN32 diag_dbg_get_input(char arg[][20], SIGN32 *argc)
{
	char *pstart, *pend; 
	char cmd[100]; 

	SIGN32 i = 0, cmd_l = 0; 

	pstart = pend = cmd; 

	fprintf(stdout, "DIAG FPGA> "); 

	fgets(cmd, sizeof(cmd), stdin);

	for (i = 0; *pend != '\0' && i < 5; pend++)
	{
		if (*pend == ' ' || *pend == '\n')
		{
			cmd_l = pend - pstart; 
			if (cmd_l >= 1)
			{
				strncpy(arg[i], pstart, cmd_l); 
				arg[i][cmd_l] = '\0'; 
				i++; 
			}
			pstart = pend + 1; 
		}
	}

	*argc = i;
	return 0;
}

SIGN32 diag_dbg_get_int(char *s, UNSG32 *pval) 
{
	SIGN32 ret;

	if ( *s == '0' || *(s+1) == 'x')
		ret = sscanf(s, "0x%x", pval);
	else 
		ret = sscanf(s, "%x", pval);

	if(ret <= 0) {
		ret = sscanf(s, "%x", pval);
	}

	return ret-1;
}


SIGN32 diag_dbg_parse_read32(char arg[][20], SIGN32 argc)
{
	UNSG32 addr;
	UNSG32 len = 1;
	UNSG32 i;

	SIGN32 r1, r2;

	r1 = r2 = 0;

	if(argc < 2 || argc > 3) {
		fprintf(stdout, "wrong number of arguments for r command\n");
		fprintf(stdout, "r addr [len]\n");
		return -1;
	}

	r1 = diag_dbg_get_int(arg[1], &addr);

	if (argc == 3)
		r2 = diag_dbg_get_int(arg[2], &len);	

	if(r1 < 0 || r2 < 0) {
		fprintf(stdout, "wrong parameter types, integer number expected!!\n");
		return 0;
	}

	printf("read addr 0x%x, len (int) = 0x%x\n", addr, len);

#ifdef DAEMON_BOARD
	for(i = 0; i < len; i++) {
		UNSG32 val;
		if (CCOC_CP_mem_read((void*)&val, (void*)(addr + i*4), 4) != 0) {
			printf("CCOC_CP_mem_read failed\n");
			break;
		} else {
			if (0==(i&3)) fprintf(stdout, "0x%08x: ", addr + i*4);
			fprintf(stdout, "0x%08x ", val);
			if(0==((i+1)&3)) fprintf(stdout, "\n");
		}
	}
#else
	for(i = 0; i < len; i++) {
		fprintf(stdout, "0x%08x ", single_read32(addr + i*4));
		if(0==((i+1)&3)) fprintf(stdout, "\n");
	}
#endif
	fprintf(stdout, "\n");

	return 0;
}

SIGN32 diag_dbg_parse_burstread32(char arg[][20], SIGN32 argc)
{
	UNSG32 addr;
	UNSG32 len = 1;

	SIGN32 r1, r2;

	r1 = r2 = 0;

	if(argc < 2 || argc > 3) {
		fprintf(stdout, "wrong number of arguments for r command\n");
		fprintf(stdout, "r addr [len]\n");
		return -1;
	}

	r1 = diag_dbg_get_int(arg[1], &addr);

	if (argc == 3)
		r2 = diag_dbg_get_int(arg[2], &len);	

	if(r1 < 0 || r2 < 0) {
		fprintf(stdout, "wrong parameter types, integer number expected!!\n");
		return 0;
	}

	printf("read addr 0x%x, len (int) = 0x%x\n", addr, len);

#ifdef DAEMON_BOARD
	memset(buf, 0, len*4);
	if (CCOC_CP_mem_read((void*)buf, (void*)(addr), len*4) != 0) {
		printf("CCOC_CP_mem_read failed\n");
	} else {
		for(i = 0; i < len; i++) {
			if (0==(i&3)) fprintf(stdout, "0x%08x: ", addr + 4*i);
			fprintf(stdout, "0x%08x ", buf[i]);
			if(0==((i+1)&3)) fprintf(stdout, "\n");
		}
	}
#else
	printf("not supported\n");
#endif
	fprintf(stdout, "\n");

	return 0;
}

SIGN32 diag_dbg_parse_write32(char arg[][20], SIGN32 argc) 
{
	UNSG32 addr;
	UNSG32 val;
	UNSG32 len = 1;
	
	UNSG32 i;
	SIGN32 r1, r2, r3;

	r1 = r2 = r3 = 0;

	if(argc < 3 || argc > 4) {
		fprintf(stdout, "wrong number of arguments for w command\n");
		fprintf(stdout, "w addr val [len]\n");
		return -1;
	}

	r1 = diag_dbg_get_int(arg[1], &addr);
	r2 = diag_dbg_get_int(arg[2], &val);

	if (argc == 4)
		r3 = diag_dbg_get_int(arg[3], &len);

	if(r1 < 0 || r2 < 0 || r3 < 0) {
		fprintf(stdout, "wrong parameter types, integer number expected!!\n");
		return 0;
	}

	printf("Write 0x%x to addr 0x%x, len (int) = 0x%x\n", val, addr, len);
	
#ifdef DAEMON_BOARD
	for (i = 0; i < len; i++) {
		if (CCOC_CP_mem_write((void*)(addr + i*4), (void*)&val, 4) != 0) {
			printf("CCOC_CP_mem_write failed\n");
			break;
		}
	}
#else
	for(i = 0; i < len; i++)
		single_write32(addr + i*4, val);
#endif

	return 0;
}

SIGN32 diag_dbg_parse_burstwrite32(char arg[][20], SIGN32 argc) 
{
	UNSG32 addr;
	UNSG32 val;
	UNSG32 len = 1;
	
	SIGN32 r1, r2, r3;

	r1 = r2 = r3 = 0;

	if(argc < 3 || argc > 4) {
		fprintf(stdout, "wrong number of arguments for w command\n");
		fprintf(stdout, "w addr val [len]\n");
		return -1;
	}

	r1 = diag_dbg_get_int(arg[1], &addr);
	r2 = diag_dbg_get_int(arg[2], &val);

	if (argc == 4)
		r3 = diag_dbg_get_int(arg[3], &len);

	if(r1 < 0 || r2 < 0 || r3 < 0) {
		fprintf(stdout, "wrong parameter types, integer number expected!!\n");
		return 0;
	}

	printf("Write 0x%x to addr 0x%x, len (int) = 0x%x\n", val, addr, len);
	
#ifdef DAEMON_BOARD
	for (i = 0; i < len; i++) {
		buf[i] = val;
	}
	if (CCOC_CP_mem_write((void*)(addr), (void*)buf, len*4) != 0) {
		printf("CCOC_CP_mem_write failed\n");
	}
#else
	printf("not supported\n");
#endif

	return 0;
}

SIGN32 diag_dbg_parse_dump(char arg[][20], SIGN32 argc) 
{
	UNSG32 addr, len;
	UNSG32 i;
	SIGN32 r1, r2, r3;
	char fname[20];
	FILE *fout;
	UNSG32 *buf, *d;

	r1 = r2 = r3 = 0;

	if(argc != 4) {
		fprintf(stdout, "wrong number of arguments for d command\n");
		fprintf(stdout, "d addr len fname\n");
		return -1;
	}

	r1 = diag_dbg_get_int(arg[1], &addr);
	r2 = diag_dbg_get_int(arg[2], &len);
	r3 = sscanf(arg[3], "%s", fname);

	r3 -= 1;

	if(r1 < 0 || r2 < 0 || r3 < 0) {
		fprintf(stdout, "wrong parameter types, addr/len are integers and fname is a string!!\n");
		return -1;
	}

	fout = fopen(fname, "wb");
	if(fout == NULL) {
		printf("cannot open output file %s\n", fname);
		return -1;
	}

	buf = (UNSG32 *)malloc(len*sizeof(UNSG32));

#ifdef DAEMON_BOARD
	for(i = 0; i < len; i++) {
		if (CCOC_CP_mem_read((void*)&buf[i], (void*)(addr + i*4), 4) != 0) {
			printf("CCOC_CP_mem_read failed\n");
			break;
		}
	}
#else
	
	for(i = 0; i < len; i++) {
		buf[i] = single_read32(addr + i*4);
	}
	
	i = len;
#define BURST_LEN 1024
	d = buf;
	while(i >= BURST_LEN) {
		burst_read(addr, d, BURST_LEN); 
		i    -= BURST_LEN;
		d    += BURST_LEN;
		addr += BURST_LEN << 2;
		printf("dump rest blocks = %d ...\n",i/1024);
	}
	if(i)
		burst_read(addr, d, i); 
#endif

	fwrite(buf, sizeof(UNSG32), len, fout);

	fclose(fout);
	free(buf);
	return 0;
}

SIGN32 diag_dbg_parse_cmd(char arg[][20], SIGN32 argc)
{
	char *pchar = arg[0];
	char ch;
	SIGN32 ret = 0;
	UNSG8 bvalid_ins = 1; 
	SIGN32 len = strlen(pchar); 

	switch(len)
	{
	case 1:
		ch = pchar[0]; 

		if (ch == 'q')
		{
			ret = 1; 
		}
		else if (ch == 'i')
		{
			diag_dbg_init_cp();
		}
		else if (ch == 'f')
		{
			diag_dbg_flush_cp();
		}
		else if (ch == 'r')
		{
			diag_dbg_parse_read32(arg, argc);
		}
		else if (ch == 'R')
		{
			diag_dbg_parse_burstread32(arg, argc);
		}
		else if (ch == 'w')
		{
			diag_dbg_parse_write32(arg, argc); 
		}
		else if (ch == 'W')
		{
			diag_dbg_parse_burstwrite32(arg, argc); 
		}
		else if (ch == 'd')
		{
			diag_dbg_parse_dump(arg, argc); 
		}
		else if (ch == 'h') {
			diag_dbg_help();
		}
		else
		{
			bvalid_ins = 0; 
		}
		break;

	default:
		bvalid_ins = 0; 
		break; 
	}

	if (!bvalid_ins)
	{
		fprintf(stdout, "Unkown command\n"); 
	}

	return ret;
}


SIGN32 diag_dbg() 
{
	char arg[16][20];
	SIGN32 argc; 

	SIGN32 ret = 0;

	//UNSG32 base_addr;
	//UNSG32 i;

	do {
		diag_dbg_get_input(arg, &argc);
		if(argc > 0)
			ret = diag_dbg_parse_cmd(arg, argc);

#ifdef DAEMON_BOARD
#else

	single_read32(0x0);
#endif

	}while(!ret);

	return 0;
}