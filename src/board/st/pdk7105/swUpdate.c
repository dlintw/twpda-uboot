/*******************************************************************************
Copyright (c) 2010 Shenzhen State Micro Technology Co., Ltd.
All right reserved.

Author: jiagong
Date  : 2010-06-14
Maintiainer: Daniel YC Lin <dlin.tw at gmail>

2013/08/25 Daniel YC Lin. update_process logic changed to more useful.
*********************************************************************************/

/*
* Description:
* internal usb mass storage  partitions:
*  ________________________________________...............
* |          |           |                 |             :
* | 1:Backup | 2:Running |     3: User     |   4: Game   :
* |__________|___________|_________________|..............
*
*/

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/stx7105reg.h>
#include <asm/io.h>
#include <asm/pio.h>
#include <asm/st40reg.h>
#include "dbg.h"
#include "swUpdate.h"
#include "load_uboot_env.h"
#include "sha.h"
#include "algorithm_error.h"

/* extern defines */
extern int do_usb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_ext2load (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_fat_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int  GetStorageDevicesNum(void);
extern unsigned char ReadSPIFlashDataToBuffer(unsigned int AddressOfFlag, unsigned char *buffer, unsigned int length);
extern int do_auto_update_uboot_to_spi (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern void WriteSPIFlashDataFromBuffer(unsigned int AddressOfFlag, unsigned char *buffer, unsigned int length);
extern void WriteSPIFlashDataByChar(unsigned int AddressOfFlag, unsigned char Data);
extern int check_ext2_usb(int part);
extern int run_command (const char *cmd, int flag);

unsigned long cfg_7105_usb_ohci_regs = 0L;
extern int dev_index;

// 32-bit integer manipulation macros (big endian)
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
{                                                                          \
	(n) = ( (unsigned long) (b)[(i) + 3]       )          \
	| ( (unsigned long) (b)[(i) + 2] <<  8 )             \
	| ( (unsigned long) (b)[(i) + 1] << 16 )            \
	| ( (unsigned long) (b)[(i) + 0] << 24 );           \
}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                              \
{                                                                          \
	(b)[(i) + 3] = (unsigned char) ( (n)       );         \
	(b)[(i) + 2] = (unsigned char) ( (n) >>  8 );      \
	(b)[(i) + 1] = (unsigned char) ( (n) >> 16 );     \
	(b)[(i) + 0] = (unsigned char) ( (n) >> 24 );     \
}
#endif
// 32-bit integer manipulation macros (little endian)
#ifndef GET_ULONG_LE
#define GET_ULONG_LE(n,b,i)                              \
{                                                                          \
	(n) = ( (unsigned long) (b)[(i) + 0]       )          \
	| ( (unsigned long) (b)[(i) + 1] <<  8 )             \
	| ( (unsigned long) (b)[(i) + 2] << 16 )            \
	| ( (unsigned long) (b)[(i) + 3] << 24 );           \
}
#endif

#ifndef PUT_ULONG_LE
#define PUT_ULONG_LE(n,b,i)                             \
{                                                                         \
	(b)[(i) + 0] = (unsigned char) ( (n)       );        \
	(b)[(i) + 1] = (unsigned char) ( (n) >>  8 );     \
	(b)[(i) + 2] = (unsigned char) ( (n) >> 16 );    \
	(b)[(i) + 3] = (unsigned char) ( (n) >> 24 );    \
}
#endif


#ifndef XYSSL_SHA1_H
#define XYSSL_SHA1_H
/////////////////////////////////////////////////////
//////////////////////inner//////////////////////////
/////////////////////////////////////////////////////
// sha1 context structure


typedef struct
{
	unsigned long total[2];     /*!< number of bytes processed  */
	unsigned long state[5];     /*!< intermediate digest state  */
	unsigned char buffer[64];   /*!< data block being processed */
}sha1_context;
static	sha1_context sha1_ctx;

// SHA-1 context setup
static void sha1_starts( sha1_context *ctx )
{
	ctx->total[0] = 0;
	ctx->total[1] = 0;

	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
	ctx->state[4] = 0xC3D2E1F0;
}

//sha1
static void sha1_process( sha1_context *ctx, unsigned char data[64] )
{
	unsigned long temp, W[16], A, B, C, D, E;

	GET_ULONG_BE( W[ 0], data,  0 );
	GET_ULONG_BE( W[ 1], data,  4 );
	GET_ULONG_BE( W[ 2], data,  8 );
	GET_ULONG_BE( W[ 3], data, 12 );
	GET_ULONG_BE( W[ 4], data, 16 );
	GET_ULONG_BE( W[ 5], data, 20 );
	GET_ULONG_BE( W[ 6], data, 24 );
	GET_ULONG_BE( W[ 7], data, 28 );
	GET_ULONG_BE( W[ 8], data, 32 );
	GET_ULONG_BE( W[ 9], data, 36 );
	GET_ULONG_BE( W[10], data, 40 );
	GET_ULONG_BE( W[11], data, 44 );
	GET_ULONG_BE( W[12], data, 48 );
	GET_ULONG_BE( W[13], data, 52 );
	GET_ULONG_BE( W[14], data, 56 );
	GET_ULONG_BE( W[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                                                    \
	(                                                                             \
	temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^        \
	W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],                       \
	( W[t & 0x0F] = S(temp,1) )                                      \
	)

#define P(a,b,c,d,e,x)                                                      \
	{                                                                             \
	e += S(a,5) + F(b,c,d) + K + x; b = S(b,30);             \
	}

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];
	E = ctx->state[4];

#define F(x,y,z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

	P( A, B, C, D, E, W[0]  );
	P( E, A, B, C, D, W[1]  );
	P( D, E, A, B, C, W[2]  );
	P( C, D, E, A, B, W[3]  );
	P( B, C, D, E, A, W[4]  );
	P( A, B, C, D, E, W[5]  );
	P( E, A, B, C, D, W[6]  );
	P( D, E, A, B, C, W[7]  );
	P( C, D, E, A, B, W[8]  );
	P( B, C, D, E, A, W[9]  );
	P( A, B, C, D, E, W[10] );
	P( E, A, B, C, D, W[11] );
	P( D, E, A, B, C, W[12] );
	P( C, D, E, A, B, W[13] );
	P( B, C, D, E, A, W[14] );
	P( A, B, C, D, E, W[15] );
	P( E, A, B, C, D, R(16) );
	P( D, E, A, B, C, R(17) );
	P( C, D, E, A, B, R(18) );
	P( B, C, D, E, A, R(19) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

	P( A, B, C, D, E, R(20) );
	P( E, A, B, C, D, R(21) );
	P( D, E, A, B, C, R(22) );
	P( C, D, E, A, B, R(23) );
	P( B, C, D, E, A, R(24) );
	P( A, B, C, D, E, R(25) );
	P( E, A, B, C, D, R(26) );
	P( D, E, A, B, C, R(27) );
	P( C, D, E, A, B, R(28) );
	P( B, C, D, E, A, R(29) );
	P( A, B, C, D, E, R(30) );
	P( E, A, B, C, D, R(31) );
	P( D, E, A, B, C, R(32) );
	P( C, D, E, A, B, R(33) );
	P( B, C, D, E, A, R(34) );
	P( A, B, C, D, E, R(35) );
	P( E, A, B, C, D, R(36) );
	P( D, E, A, B, C, R(37) );
	P( C, D, E, A, B, R(38) );
	P( B, C, D, E, A, R(39) );

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

	P( A, B, C, D, E, R(40) );
	P( E, A, B, C, D, R(41) );
	P( D, E, A, B, C, R(42) );
	P( C, D, E, A, B, R(43) );
	P( B, C, D, E, A, R(44) );
	P( A, B, C, D, E, R(45) );
	P( E, A, B, C, D, R(46) );
	P( D, E, A, B, C, R(47) );
	P( C, D, E, A, B, R(48) );
	P( B, C, D, E, A, R(49) );
	P( A, B, C, D, E, R(50) );
	P( E, A, B, C, D, R(51) );
	P( D, E, A, B, C, R(52) );
	P( C, D, E, A, B, R(53) );
	P( B, C, D, E, A, R(54) );
	P( A, B, C, D, E, R(55) );
	P( E, A, B, C, D, R(56) );
	P( D, E, A, B, C, R(57) );
	P( C, D, E, A, B, R(58) );
	P( B, C, D, E, A, R(59) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

	P( A, B, C, D, E, R(60) );
	P( E, A, B, C, D, R(61) );
	P( D, E, A, B, C, R(62) );
	P( C, D, E, A, B, R(63) );
	P( B, C, D, E, A, R(64) );
	P( A, B, C, D, E, R(65) );
	P( E, A, B, C, D, R(66) );
	P( D, E, A, B, C, R(67) );
	P( C, D, E, A, B, R(68) );
	P( B, C, D, E, A, R(69) );
	P( A, B, C, D, E, R(70) );
	P( E, A, B, C, D, R(71) );
	P( D, E, A, B, C, R(72) );
	P( C, D, E, A, B, R(73) );
	P( B, C, D, E, A, R(74) );
	P( A, B, C, D, E, R(75) );
	P( E, A, B, C, D, R(76) );
	P( D, E, A, B, C, R(77) );
	P( C, D, E, A, B, R(78) );
	P( B, C, D, E, A, R(79) );

#undef K
#undef F

#undef P
#undef R
#undef S

	ctx->state[0] += A;
	ctx->state[1] += B;
	ctx->state[2] += C;
	ctx->state[3] += D;
	ctx->state[4] += E;
}

/*
* SHA-1 process buffer
*/
static void sha1_update( sha1_context *ctx, unsigned char *input,  unsigned long ilen )
{
	unsigned long fill;
	unsigned long left;

	if( ilen <= 0 )
		return;

	left = ctx->total[0] & 0x3F;
	fill = 64 - left;

	ctx->total[0] += ilen;
	ctx->total[0] &= 0xFFFFFFFF;

	if( ctx->total[0] < (unsigned long) ilen )
		ctx->total[1]++;

	if( left && (ilen >= fill) )
	{
		memcpy( (void *) (ctx->buffer + left),
			(void *) input, fill );
		sha1_process( ctx, ctx->buffer );
		input += fill;
		ilen  -= fill;
		left = 0;
	}

	while( ilen >= 64 )
	{
		sha1_process( ctx, input );
		input += 64;
		ilen  -= 64;
	}

	if( ilen > 0 )
	{
		memcpy( (void *) (ctx->buffer + left),
			(void *) input, ilen );
	}
}


// SHA-1 final digest
static unsigned long  sha1_finish( sha1_context *ctx,unsigned char *output )
{
	unsigned long last, padn;
	unsigned long high, low;
	unsigned char msglen[8];
	unsigned char sha_padding[64] ={0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	high = ( ctx->total[0] >> 29 )
		| ( ctx->total[1] <<  3 );
	low  = ( ctx->total[0] <<  3 );

	PUT_ULONG_BE( high, msglen, 0 );
	PUT_ULONG_BE( low,  msglen, 4 );

	last = ctx->total[0] & 0x3F;
	padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

	sha1_update( ctx, (unsigned char *) sha_padding, padn );
	sha1_update( ctx, msglen, 8 );

	PUT_ULONG_BE( ctx->state[0], output,  0 );
	PUT_ULONG_BE( ctx->state[1], output,  4 );
	PUT_ULONG_BE( ctx->state[2], output,  8 );
	PUT_ULONG_BE( ctx->state[3], output, 12 );
	PUT_ULONG_BE( ctx->state[4], output, 16 );

	memset(&sha1_ctx, 0, sizeof( sha1_ctx ) );
	return 0;
}

// output = SHA-1( input buffer )
static void sha1( unsigned char *input, unsigned long  ilen, unsigned char output[20] )
{
	sha1_context ctx;

	sha1_starts( &ctx );
	sha1_update( &ctx, input, ilen );
	sha1_finish( &ctx, output);

	memset( &ctx, 0, sizeof( sha1_context ) );
}

/////////////////////////////////////////////////////
//////////////////////external///////////////////////
/////////////////////////////////////////////////////
unsigned long soft_sha_1(unsigned char *i_pdata, unsigned long  i_udatalen,
						 unsigned char *o_pdata)
{
	sha1(i_pdata, i_udatalen, o_pdata );

	return 0;
}

#endif


#define COMMAND_SIZE	64
#define MAX_NAME_LEN	256
#define MAX_IMAGE_SECTION  0x100000
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define IPTV_FLAG_ADDR              0x90000
#define SPI_UBOOT_UPDATE_INFO       0x91000
#define SPI_LOADER_INFO             0x95000

#define UPDATEUBOOTFLAG   0xabcd6789

#define PIO3_BASSADDRESS  0xFD023000
#define PIO3_CLEAR_C0  *(volatile unsigned char *)(PIO3_BASSADDRESS+0x28)
#define PIO3_CLEAR_C1 *(volatile unsigned char *)(PIO3_BASSADDRESS+0x38)
#define PIO3_CLEAR_C2   *(volatile unsigned char *)(PIO3_BASSADDRESS+0x48)
#define PIO3_SET_C0  *(volatile unsigned char *)(PIO3_BASSADDRESS+0x24)
#define PIO3_SET_C1 *(volatile unsigned char *)(PIO3_BASSADDRESS+0x34)
#define PIO3_SET_C2   *(volatile unsigned char *)(PIO3_BASSADDRESS+0x44)

/*********************** ST ***** HDMI&CVBS ******************************/
#define D_NTSC 1
#define D_PAL 0
#define DISPLAY_SD D_NTSC

#define ARGB8888 1
#define COLORTYPE ARGB8888
/* DEVICE */

unsigned long STSYS_ReadRegDev32BE(void *Address_p) {
	unsigned long RetVal = ((unsigned long) (((*(((volatile unsigned char *) (Address_p))    )) << 24) |
		((*(((volatile unsigned char *) (Address_p)) + 1)) << 16) |
		((*(((volatile unsigned char *) (Address_p)) + 2)) <<  8) |
		((*(((volatile unsigned char *) (Address_p)) + 3))     )));
	return RetVal;
}


unsigned long STSYS_ReadRegDev32LE(void *Address_p)
{
	unsigned long RetVal =  (*((volatile unsigned long *) (Address_p)));
	return RetVal;
}

void STSYS_WriteRegDev32BE(void *Address_p, unsigned long Value)
{
	*((volatile unsigned long *) (Address_p)) = (unsigned long) ((((Value) & 0xFF000000) >> 24) |
		(((Value) & 0x00FF0000) >> 8 ) |
		(((Value) & 0x0000FF00) << 8 ) |
		(((Value) & 0x000000FF) << 24));
}



void STSYS_WriteRegDev32LE(void *Address_p, unsigned long Value)
{
	*((volatile unsigned long *) (Address_p)) = (unsigned long) (Value);
}

#ifdef ULONG
#undef ULONG
#endif
#define ULONG unsigned long

#ifdef UCHAR
#undef UCHAR
#endif
#define UCHAR unsigned char

void WriteRegister(volatile ULONG *reg,ULONG val)
{
	*((volatile ULONG *) (reg)) = (ULONG) (val);
}

void WriteByteRegister(volatile UCHAR *reg,UCHAR val)
{
	*((volatile UCHAR *) (reg)) = (UCHAR) (val);
}

typedef struct PIXER_COLOR
{
	unsigned char uR;
	unsigned char uG;
	unsigned char uB;
}PIXER_COLOR_t;


#define  stsys_WRITEREGDEV32LE(a,v) STSYS_WriteRegDev32LE((void*)a,v)
#define  stsys_READREGDEV32LE(a,v) STSYS_ReadRegDev32LE((void*)a)
#define  stsys_WRITEREGMEM32LE(a,v) STSYS_WriteRegDev32LE((void*)a,v) //was mem
#define  stsys_READREGMEM32LE(a,v) STSYS_ReadRegDev32LE((void*)a) //was mem


#define ST7105_CKG_BASE_ADDRESS           0xfe000000
#define ST7105_CKG_B_BASE_ADDRESS         0xfe000000
#define ST7105_DENC_BASE_ADDRESS          0xfe030000
#define ST7105_HD_TVOUT_BASE_ADDRESS      0xfe030000
#define ST7105_HDMI_BASE_ADDRESS          0xfd104000
#define ST7105_HD_TVOUT_VTGB_BASE_ADDRESS 0xfe030200
#define ST7105_HD_TVOUT_VTGA_BASE_ADDRESS 0xfe030300
#define ST7105_HD_TVOUT_HDF_BASE_ADDRESS  0xfe030800
#define ST7105_AWG_BASE_ADDRESS           0xfe030b00 //sure?

#define ST7105_GDP1_LAYER_BASE_ADDRESS    0xfe20a100
#define ST7105_GDP3_LAYER_BASE_ADDRESS    0xfe20a300

#define ST7105_VMIX1_BASE_ADDRESS         0xfe20ac00
#define ST7105_VMIX2_BASE_ADDRESS         0xfe20ad00

#define ST7105_CFG_BASE_ADDRESS           0xfe001000

#define ST7105_HD_TVOUT_MAIN_GLUE_BASE_ADDRESS 0xfe030400

/*Added by LQ */
#define GDP1_NODE_TOP_ADDRESS           0x98200100
#define GDP1_NODE_TOP_PHY_ADDRESS       0x48200100

#define SD_PIC_LAOD_ADDRESS		        0x980012a0
#define SD_PIC_LAOD_PHY_ADDRESS		    0x480012a0

#define HD_PIC_LAOD_ADDRESS		        0x986012a0
#define HD_PIC_LAOD_PHY_ADDRESS		    0x486012a0

#define GDP3_NODE_TOP_ADDRESS		    0x98300100
#define GDP3_NODE_TOP_PHY_ADDRESS	    0x48300100

#define GDP3_NODE_BOTTOM_ADDRESS	    0x98310100
#define GDP3_NODE_BOTTOM_PHY_ADDRESS	0x48310100

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;
typedef unsigned int    LONG;


/****************************************************************/

static unsigned short smitGetCrc16(unsigned char *data_blk_ptr,unsigned int data_blk_size)
{
	unsigned short		crc_return;
	unsigned char		CRC16Lo=INI_VECTORLo;
	unsigned char		CRC16Hi=INI_VECTORHi;
	unsigned char		SaveHi,SaveLo;
	register unsigned int	i, j;
	for (i = 0; i < data_blk_size; i++)
	{
		CRC16Lo = CRC16Lo ^ *data_blk_ptr++;
		for ( j = 0;  j < 8;  j++ )
		{
			SaveHi = CRC16Hi;
			SaveLo = CRC16Lo;
			CRC16Hi = CRC16Hi >> 1;
			CRC16Lo = CRC16Lo >> 1;
			if ((SaveHi & 0x1) == 0x1)
			{
				CRC16Lo = CRC16Lo | 0x80;
			}
			if ((SaveLo & 0x1) == 0x1)
			{
				CRC16Hi = CRC16Hi ^ POLYNOMIALHi;
				CRC16Lo = CRC16Lo ^ POLYNOMIALLo;
			}
		}
	}
	crc_return=CRC16Hi * 0xff + CRC16Lo;
	return crc_return;
}

/*
* do some operation concerning usb.
* like start, stop usb device, or look usb
* information.
* $ FAILURE_RET, have some error when copy
* $ SUCCESS_RET, success return
*/
int usb_operate(usb_operation_et oper_type)
{
	char *usb_command[3];
	int i;
	int ret = -1;

	for (i = 0; i < ARRAY_SIZE(usb_command); i++) {
		usb_command[i] = malloc(COMMAND_SIZE);
		if (NULL == usb_command[i]) {
			goto fail_return;
		}
		memset(usb_command[i], '\0', COMMAND_SIZE);
	}
	strncpy(usb_command[0], "usb", COMMAND_SIZE);
	switch (oper_type) {
	case USB_START:
		strncpy(usb_command[1], "start", COMMAND_SIZE);
		break;
	case USB_STOP:
		strncpy(usb_command[1], "stop", COMMAND_SIZE);
		break;
	default:
		goto fail_return;
		break;
	}
	ret = do_usb(NULL, 0, 2, usb_command);
	if (0 != ret) {
		goto fail_return;
	}
	for (i = 0; i < ARRAY_SIZE(usb_command); i++) {
		if (usb_command[i]) {
			free(usb_command[i]);
			usb_command[i] = NULL;
		}
	}
	return SUCCESS_RET;
fail_return:
	for (i = 0; i < ARRAY_SIZE(usb_command); i++) {
		if (usb_command[i]) {
			free(usb_command[i]);
			usb_command[i] = NULL;
		}
	}
	return FAILURE_RET;
}

/*
* copy data from USB device to ram.
* @ char *file_name, appoint to the file name which will be copy
* $ FAILURE_RET, have some error when copy
* $ SUCCESS_RET, success return
*/
int usb_ext2load(char *file_name,int partition)
{
	char *ext2load_command[7];
	int i;
	char *buf = (char *)SWLOADADDR;
	int ret = FAILURE_RET;

	for (i = 0; i < ARRAY_SIZE(ext2load_command); i++) {
		ext2load_command[i] = malloc(COMMAND_SIZE);
		if (NULL == ext2load_command[i]) {
			goto fail_return;
		}
		memset(ext2load_command[i], '\0', COMMAND_SIZE);
	}

	strncpy(ext2load_command[0], "ext2load", COMMAND_SIZE);
	strncpy(ext2load_command[1], "usb", COMMAND_SIZE);
	if(partition==INT_USB_PARTITION_BACKUP)
	{
		strncpy(ext2load_command[2], "0:1", COMMAND_SIZE);
	}
	else if(partition==INT_USB_PARTITION_RUNNING)
	{
		strncpy(ext2load_command[2], "0:2", COMMAND_SIZE);
	}
	strncpy(ext2load_command[3], "80000000", COMMAND_SIZE);
	strncpy(ext2load_command[4], file_name, COMMAND_SIZE);

	ret = do_ext2load(NULL, 0, 6, ext2load_command);
	if (1 == ret)
	{
		goto fail_return;
	}
	buf[ret]='\0'; // setting EOF

	for (i = 0; i < ARRAY_SIZE(ext2load_command); i++) {
		if (ext2load_command[i]) {
			free(ext2load_command[i]);
			ext2load_command[i] = NULL;
		}
	}
	return SUCCESS_RET;
fail_return:
	for (i = 0; i < ARRAY_SIZE(ext2load_command); i++) {
		if (ext2load_command[i]) {
			free(ext2load_command[i]);
			ext2load_command[i] = NULL;
		}
	}
	return FAILURE_RET;
}
/*
* copy data from USB device to ram.
* @ char *file_name, appoint to the file name which will be copy
* $ FAILURE_RET, have some error when copy
* $ SUCCESS_RET, success return content on 0x80000000
*/
int usb_fatload(char *file_name,unsigned int LoadSize)
{
	char *fatload_command[7];
	int i;
	int ret = FAILURE_RET;

	for (i = 0; i < ARRAY_SIZE(fatload_command); i++) {
		fatload_command[i] = malloc(COMMAND_SIZE);
		if (NULL == fatload_command[i]) {
			goto fail_return;
		}
		memset(fatload_command[i], '\0', COMMAND_SIZE);
	}

	strncpy(fatload_command[0], "fatload", COMMAND_SIZE);
	strncpy(fatload_command[1], "usb", COMMAND_SIZE);
	strncpy(fatload_command[2], "0", COMMAND_SIZE);
	strncpy(fatload_command[3], "80000000", COMMAND_SIZE);
	strncpy(fatload_command[4], file_name, COMMAND_SIZE);

	if(LoadSize != 0)
	{
		sprintf(fatload_command[5], "%x", LoadSize);
		ret = do_fat_fsload(NULL, 0, 6, fatload_command);
	}
	else
	{
		ret = do_fat_fsload(NULL, 0, 5, fatload_command);
	}

	if (0 != ret) {
		goto fail_return;
	}
	for (i = 0; i < ARRAY_SIZE(fatload_command); i++) {
		if (fatload_command[i]) {
			free(fatload_command[i]);
			fatload_command[i] = NULL;
		}
	}
	return SUCCESS_RET;
fail_return:
	for (i = 0; i < ARRAY_SIZE(fatload_command); i++) {
		if (fatload_command[i]) {
			free(fatload_command[i]);
			fatload_command[i] = NULL;
		}
	}
	return FAILURE_RET;
}

/*
return value:
0 ----equal
-1 ---not equal
*/
int CompareSHA1Value(unsigned char *buffer1, unsigned char *buffer2)
{
	int i;

	for(i=0;i<20;i++)
	{
		if(buffer1[i] != buffer2[i])
		{
			break;
		}
	}

	if(i==20)
	{
		return 0;
	}
	else
	{
		printf("CompareSHA1Value error!!!\n");
		return -1;
	}
}

void SHA1ValueAdd(unsigned char *Sum, unsigned char *AddValue)
{
	int i;

	for(i=0;i<20;i++)
	{
		Sum[i]=(Sum[i]+AddValue[i])%256;
	}

}

/**********************************************************************
Function   :check_update_sw_header
Descriptor :
SUCCESS_RET ----need update
FAILURE_RET ---Don't need update
Notice	   :
**********************************************************************/
static int check_update_sw_header(ImageFileHeader_T *pFileHeader, SPISoftWareInfo_T *pSPISWInfo)
{
	int ret;

	ret =FAILURE_RET;
	DBG2("UBOOT_Ver: File:%d Flash:%d",
		pFileHeader->UBOOT_Ver, pSPISWInfo->UBOOT_Ver);
	if( ((pFileHeader->FactoryID == pSPISWInfo->FactoryID) || (0xffffffff == pSPISWInfo->FactoryID))
		&&((pFileHeader->DeviceID == pSPISWInfo->DeviceID) || (0xffffffff == pSPISWInfo->DeviceID))
		&&(pFileHeader->UpdateUbootFlag == 0xabcd6789)
		&&(pFileHeader->UBOOT_Ver != pSPISWInfo->UBOOT_Ver) )
	{
		ret =  SUCCESS_RET;
	}
	return  ret;
}


static void smit_restartST40(void)
{
	volatile unsigned short *ST40_CPG1_WTCNT   = (unsigned short *)(0xFFC00000 + 0x08);
	volatile unsigned short *ST40_CPG1_WTCSR   = (unsigned short *)(0xFFC00000 + 0x0C);
	volatile unsigned short *ST40_CPG1_WTCSR2 = (unsigned short *)(0xFFC00000 + 0x1C);

	*ST40_CPG1_WTCNT = 0x5AF0;
	*ST40_CPG1_WTCSR = 0xA547;
	*ST40_CPG1_WTCSR2 = 0xAA00;
	*ST40_CPG1_WTCSR  = 0xA5C7;

	for(;;)
		printf("waiting for restar\n");
}

/* init PIO of led and key config */
static void init_pio(void)
{
	/*init led&key*/
	PIO0_CLEAR_C0 = 0x32;
	PIO0_CLEAR_C1 = 0x32;
	PIO0_CLEAR_C2 = 0x32;
	PIO0_SET_C0 = 0x02;
	PIO0_SET_C1 = 0x30;
	PIO0_SET_C2 = 0x02;

	udelay(1000);
}

static void set_led_blue(int v)
{
	STPIO_SET_PIN(PIO0_BASSADDRESS,4,v);
}

static void set_led_red(int v)
{
	STPIO_SET_PIN(PIO0_BASSADDRESS,5,v);
}

/**********************************************************************
Function   :set_led_flash
Descriptor :the board have two led,red and blue;
Input      :
Output     :
return     :
Notice	   :
**********************************************************************/
static void set_led_flash(led_gb620_et led,unsigned int nsec,led_flash_et ntype)
{
	int nswitch,ncnt,ntime;

	nswitch = 1;
	ntime = nsec;

	do
	{
		for(ncnt=0;ncnt<5;ncnt++)
		{
			udelay(200000);
			if(led == GB620_RED_LED)
			{
				set_led_red(nswitch);
			}
			if(led == GB620_BLUE_LED)
			{
				set_led_blue(nswitch);
			}
			nswitch = !nswitch;
		}
		ntime --;
	}while( (GB620_LED_FLASH_ALL == ntype) || (ntime > 0) );

}

/**********************************************************************
Function   :usb_config_start
Descriptor :*USB config and start*;
Input      :
Output     :
return     : SUCCESS_RET:   success
FAILURE_RET:   failed
Notice	   :
**********************************************************************/
static int usb_config_start(unsigned int cfg)
{
	int ncnt = 0;
	int nret = FAILURE_RET;
	int nDev = 0;
	int rc;

	cfg_7105_usb_ohci_regs = cfg;

	//rc = run_command("usb start", 0);
	//DBG1("usb start, return %d", rc);
	//rc = run_command("usb tree", 0);
	//DBG1("usb tree, return %d", rc);

	nret = usb_operate(USB_START);

	/*waiting, until usb device found, timeout is 2s */
	while((dev_index != 2) && (ncnt < 2) && ((nDev = GetStorageDevicesNum()) == 0) )
	{
		//DBG3("dev_index=%d ncnt=%d nDev=%d", dev_index, ncnt, nDev);
		udelay(1000000);
		nret = usb_operate(USB_START);
		ncnt++;
	}
	return nret;
}

/*----------------------------------------------------------------------------
 * FuncName   :  smit_logo_init
 * Description:  display logo, default 480p@NTSC 3.58, default bmp size 190*220
 				 support HD/SD photo
 * Parameters :  @
                 @
 * FuncReturn :  NULL
 * Information:
              1¡¢10-12,2010    jgong    Write/Modify
------------------------------------------------------------------------------*/


/**********************************************************************
Function   :uboot_update
Descriptor :
Input      :none
Output     :none
return     : SUCCESS_RET:   success
 FAILURE_RET:   same version,GetStorageDevicesNum() < 1 or CompareSHA1Value fail
 ABNORMITY_RET: CRC, Header or do_auto_update_uboot_to_spi fail
Notice	   :
**********************************************************************/
int uboot_update(unsigned char flag)
{
	ImageFileHeader_T *pImgFileHeader=NULL;
	unsigned char     *pUbootHeader=NULL;
	unsigned short    CRCValue;
	SPISoftWareInfo_T SPISoftWareInfo={0};
	unsigned char     SHA1TempResult[20]={0};

	if(GetStorageDevicesNum() < 1)
	{
		return FAILURE_RET;
	}
	/* load uboot sw to ddr */
	if(SUCCESS_RET  == usb_fatload("iptvubootupdate.bin", sizeof(ImageFileHeader_T)+MAX_IMAGE_SECTION/2)	)
	{
		/* read spi from addr 0x91000, get sw version info */
		ReadSPIFlashDataToBuffer(SPI_UBOOT_UPDATE_INFO, (unsigned char *)(&SPISoftWareInfo), sizeof(SPISoftWareInfo_T));
		pImgFileHeader = (ImageFileHeader_T *)(SWLOADADDR);
		pUbootHeader = (unsigned char *)(SWLOADADDR+sizeof(ImageFileHeader_T));
		CRCValue = smitGetCrc16((void *)pImgFileHeader, sizeof(ImageFileHeader_T)-2);

		if(CRCValue != pImgFileHeader->CRC_Header)
		{
			set_led_flash(GB620_RED_LED,UBOOT_UPDATE_TIME_SEC,GB620_LED_FLASH_SEC);
			return ABNORMITY_RET;
		}

		/* check sw update info */
		if(0 == check_update_sw_header(pImgFileHeader, &SPISoftWareInfo))
		{
			/* count uboot sw data sha1 */
			soft_sha_1(pUbootHeader, MAX_IMAGE_SECTION/2, SHA1TempResult);

			/* check uboot data valid */
			if(0 == CompareSHA1Value(pImgFileHeader->SHA1_Uboot, SHA1TempResult))
			{
				/* write uboot to spi flash */
				if( SUCCESS_RET == do_auto_update_uboot_to_spi(NULL, pImgFileHeader->UbootSize, \
					sizeof(ImageFileHeader_T), NULL))
				{
					SPISoftWareInfo.UBOOT_Ver = pImgFileHeader->UBOOT_Ver;
					SPISoftWareInfo.DeviceID = pImgFileHeader->DeviceID;
					SPISoftWareInfo.FactoryID = pImgFileHeader->FactoryID;
					strncpy(SPISoftWareInfo.Date, pImgFileHeader->Date,10);

					WriteSPIFlashDataFromBuffer(SPI_UBOOT_UPDATE_INFO, (unsigned char *)(&SPISoftWareInfo), \
						sizeof(SPISoftWareInfo_T));

					if(flag != NORMAL_START)
					{
						WriteSPIFlashDataByChar(IPTV_FLAG_ADDR,NORMAL_START);
					}
					set_led_flash(GB620_BLUE_LED,UBOOT_UPDATE_TIME_SEC,GB620_LED_FLASH_SEC);
					smit_restartST40();
				}
				else
				{
					set_led_flash(GB620_RED_LED,UBOOT_UPDATE_TIME_SEC,GB620_LED_FLASH_SEC);
					return ABNORMITY_RET;
				}
			}
		}
		else
		{
			DBG0("skip upgrade same version iptvubootupdate.bin");
			return FAILURE_RET;
		}
	}
	return FAILURE_RET;
}
/* @brief update_bootenv
 * @param isExternalUSB
 *
 * if first partition exist uboot.sh then excute it.
 * else {
 * 	check uboot upgrade program
 * 	if 2nd parition exist uboot.sh then executed it.
 * }
 */
int update_from_usb(int isExternalUSB)
{
	unsigned int cfg = USB_INT_REGS;
	int rc;

	//DBG1("begin of update_from_usb(%d)", isExternalUSB);
	if (isExternalUSB) {
		cfg = USB_EXT_REGS;
	}
	if (FAILURE_RET == usb_config_start(cfg)) {
		return FAILURE_RET;
	}
	if (ABNORMITY_RET == (rc=load_uboot_env(1))) { // no uboot.sh in part1
		if (NOT_EXT2_FS == check_ext2_usb(1)) {
			if (ABNORMITY_RET == (rc = uboot_update(NORMAL_START)))
				smit_restartST40();
		}
		rc=load_uboot_env(2); // check partition 2
	}
	//DBG1("end of update_from_usb(%d)", isExternalUSB);
	return rc;
}
/* @brief update firmware if required
*/
int update_process(void)
{
	init_pio();

	if (KEY_DOWN == STPIO_GET_PIN(PIO0_BASSADDRESS, 1)) {
		DBG0("RESET pressed, trying external USB at first");
		set_led_blue(0);
		set_led_red(1);
		if (SUCCESS_RET != update_from_usb(1)) {
			DBG0("RESET pressed, trying internal USB");
			return update_from_usb(0);
		}
	} else {
		DBG0("Trying internal USB at first");
		set_led_blue(1);
		set_led_red(0);
		if (SUCCESS_RET != update_from_usb(0)) {
			DBG0("Trying external USB");
			return update_from_usb(1);
		}
	}
	return SUCCESS_RET;
}
int do_usbcfg(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int cfg = USB_INT_REGS;
	// argv[0]='usbcfg', argv[1] first parameter, argc==2
	//DBG1("argc=%d", argc);
	switch (argc) {
		case 2:
			//DBG3("argv=%s,%s,%s", argv[0], argv[1], argv[2]);
			if (argv[1][0] != '0') {
				printf("  set usbcfg=USB_EXT_REGS\n");
				cfg = USB_EXT_REGS;
			} else {
				printf("  set usbcfg=USB_INT_REGS\n");
			}
			cfg_7105_usb_ohci_regs = cfg;
			break;
		default:
			//DBG3("argv=%s,%s,%s", argv[0], argv[1], argv[2]);
			printf ("Usage:\n%s\n", cmdtp->usage);
			if (cfg_7105_usb_ohci_regs == USB_INT_REGS) {
				printf("  usbcfg=USB_INT_REGS\n");
			} else {
				printf("  usbcfg=USB_EXT_REGS\n");
			}
			return(1);
	}

	return 0;
}

U_BOOT_CMD(
	usbcfg,    2,    0,    do_usbcfg,
	"usbcfg\t- switch internal/external usb config address\n",
	"<0/1>  # 0:interal 1:external USB\n"
	"    - switch internal/external usb config address\n"
	);
