/*
 *  Authors: Kristof Denolf (IMEC) and Paul Schumacher (Xilinx)  
 *  Based on MPEG-4 reference code 
 *  Date: November 21st 2001 
 */ 
 
/* This software module is an implementation of a part of one or more MPEG-4 
 * Video (ISO/IEC 14496-2) tools as specified by the MPEG-4 Video (ISO/IEC 
 * 14496-2) standard. 
 * 
 * ISO/IEC gives users of the MPEG-4 Video (ISO/IEC 14496-2) standard free 
 * license to this software module or modifications thereof for use in hardware 
 * or software products claiming conformance to the MPEG-4 Video (ISO/IEC 
 * 14496-2) standard. 
 * 
 * Those intending to use this software module in hardware or software products 
 * are advised that its use may infringe existing patents. The original 
 * developer of this software module and his/her company, the subsequent 
 * editors and their companies, and ISO/IEC have no liability for use of this 
 * software module or modifications thereof in an implementation. Copyright is 
 * not released for non MPEG-4 Video (ISO/IEC 14496-2) Standard conforming 
 * products. 
 * 
 * ACTS-MoMuSys partners retain full right to use the code for his/her own 
 * purpose, assign or donate the code to a third party and to inhibit third 
 * parties from using the code for non MPEG-4 Video (ISO/IEC 14496-2) Standard 
 * conforming products. This copyright notice must be included in all copies or 
 * derivative works. 
 * 
 * Copyright (c) 1996 
 * 
 *****************************************************************************/ 

#ifndef __AllInOne_h
#define __AllInOne_h


 
#include <math.h> 
#include <stdarg.h> 
#include <sys/timeb.h>
// #ifdef WIN32 
// #include "picture.h"
// #include <cstdlib> 
// #include <cstdio> 
// #include <cstring> 
// #else 
// #include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
// #endif

// **************
// Directives
// **************
// #ifdef WIN32
#define VERBOSE
#define STOP_ON_ERROR
// #define USE_DISPLAY
#define WRITE_FILE
// #define COMPARE_FILE
// #define WRITE_TB_FILE
// #endif
// #define USE_ORIG_IDCT
#define USE_FAST_IDCT


/////////////////////////////////////////////////////////////////////
// Typedefs, Structures, & Constants
/////////////////////////////////////////////////////////////////////

//
// Constant definitions
//
#define MAX_STREAMS			(    6 )
#define MAX_WIDTH			(  720 )
#define MAX_HEIGHT			(  576 )
#define BITS_DIM			(   10 )
#define BITS_PIXEL			(    8 )
#define BITS_QUANT			(    5 )
#define BITS_DCT			(   12 )
#define BITS_DCT_IQ			(   12 )
#define MB_WIDTH			(   16 )
#define MB_HEIGHT			(   16 ) 
#define B_WIDTH				(    8 )
#define B_HEIGHT			(    8 )
#define B_SIZE				(   64 ) // B_WIDTH * B_HEIGHT
#define BLOCK_CNT			(    6 ) 
#define MAXVAL_PIXEL		(  255 ) // 2^BITS_PIXEL - 1
#define MIDVAL_PIXEL		(  128 ) // 2^(BITS_PIXEL - 1)
#define YDEFAULT			(  128 )
#define UDEFAULT			(  128 )
#define VDEFAULT			(  128 )
#define BFR_SIZE			( 2048 ) 
#define INBFR_BYTES			(    7 ) // mininum value: 5 
// marker codes and lengths
#define VOL_START_CODE								(0x012)
#define VOP_START_CODE								(0x1B6)
#define RESYNC_MARKER									(1)
#define VOL_START_CODE_LENGTH					(28)
#define VOP_START_CODE_LENGTH					(32)
#define MCBPC_LENGTH									( 9)
#define USER_DATA_START_CODE_LENGTH		(32)
#define SHORT_VIDEO_START_CODE_LENGTH (22)
#define START_CODE_PREFIX_LENGTH			(24)
// frame and block types
#define I_VOP (0) 
#define P_VOP (1) 
#define B_VOP (2) 
#define INTRA (0) 
#define INTER (1) 
  
#define LB_MAX (MAX_WIDTH / MB_WIDTH + 2)

#define PARSER_CLOCK	(  100 ) // clock frequency for parser/VLD (in MHz)

// constants from encoder
#define LBwidth 95 // 2*maxMBwidth + 3 + 2; maximum size BufferY/U/V including pipeline delay

//
// Typedefs
//
typedef unsigned char UChar;
typedef short SInt;
typedef int Int;
typedef unsigned char Byte;
typedef unsigned char BYTE;
typedef short Short;
typedef char Char;
typedef unsigned long UInt;
typedef float Float;
typedef FILE File;
typedef void Void;
typedef double Double;
typedef const int C_Int; 
typedef const UInt C_UInt; 

//
// Structures
//
struct Tcoef 
{ 
	SInt last; 
	SInt run; 
	SInt level; 
	SInt sign; 
};

struct VLCtab
{ 
	Int val; 
	Int len; 
};

typedef struct  
{ 
	File *fptr;
	UChar *rdptr;
	UChar rdbfr[BFR_SIZE+4];  
	UChar inbfr[3*INBFR_BYTES]; 
	unsigned short incnt; 
	UChar *rdtop;
	unsigned long bitcount;
	unsigned long bstart,bfinish;
	unsigned short streamnum;
} Bitstream;


struct vol
{
	int displayWidth;
	int displayHeight;
	float framerate;
	FILE *y_file; FILE *u_file; FILE *v_file;
};
typedef struct vol Vol;

struct vop;
typedef struct vop Vop; 

//
// Macros
//
  
// Comment macro (show only in verbose mode) 
#ifdef VERBOSE
#define COMMENT(x) x
#else
#define COMMENT(x)
#endif

// 
// Global variables
//
// for test and analysis only
extern int framenum;
#ifdef VERBOSE
extern int iframes, pframes; 
extern int partitioning_used;
extern int vol_header_bits, vop_header_bits;
extern int vp_header_bits, mb_header_bits;
extern int motion_bits, texture_bits;
extern int motion_blocks, texture_blocks;
extern int motion_bits_prev, texture_bits_prev;
extern int motion_blocks_prev, texture_blocks_prev;
extern short mvx_max, mvy_max;
extern unsigned long parser_cycles;
#endif
// filenames and file pointers
extern char bitfile[MAX_STREAMS][300];
extern char outfile[300];
extern char outfileU[300];
extern char outfileV[300];
extern int resync_errors;
#ifdef WRITE_FILE
extern File *yuv_file, *u_file, *v_file;
#endif
#ifdef COMPARE_FILE
extern File *ref_file;
extern char reffile[300];
extern int frames_in_error;
#endif
#ifdef WRITE_INST_FILE
extern File *dct_file1, *dct_file2;
extern File *mot_file1, *mot_file2;
#endif

//
// Register files
//
// NOTE: these are accessible by the ParserVLD HW block
// as well as either the CopyControl, MotionComp or TextureIDCT HW block
extern unsigned int dec_regfile[MAX_STREAMS];
extern unsigned int mc_regfile[1];
extern unsigned int idct_regfile[1];

/////////////////////////////////////////////////////////////////////
// Function Prototypes
/////////////////////////////////////////////////////////////////////
// Top level
void DecoderChip(Vol *VOlist[], int dwidth, int dheight, unsigned short streams, int *ulx_display, int *uly_display);

// Utilities
void FatalError(char *string);
char *strrepl(char *Str, unsigned short BufSiz, char *OldStr, char *NewStr);

// Bitstream handling
int CheckBitStuffing(Bitstream *stream);
UInt BitstreamByteAlign(Bitstream *stream);
void BitstreamFlushBits(Bitstream *stream, int nbits);
UInt BitstreamShowBits(Bitstream *stream, int nbits);
unsigned long BitstreamReadBits(Bitstream *stream, int nbits);
int BitstreamInit(Bitstream *stream, int num);
void BitstreamClose(Bitstream *stream);
UInt BitstreamShowBitsByteAlign(Bitstream *stream, int nbits); 

// Parser/VLD FSM
UChar ParserVLD(short streams, int *width, int *height, unsigned int *frameMemoryOffset, 
							  unsigned char *write_frame, unsigned char *perform_cc, unsigned char *perform_mc, unsigned char *perform_idct, 
							  unsigned char *stnum_out, short *mvx_out, short *mvy_out, unsigned char *blnum_out, 
							  unsigned char *comp_block, short *q_block, short *texture_block,
							  unsigned char *MCcoded_out, unsigned char *IDCTcoded_out,
								unsigned char *hOut1, unsigned char *vOut1,unsigned char *tu_mode, unsigned char *hOut2, unsigned char *vOut2,
								unsigned char *btype_out, unsigned char *ACpred_flag_out, unsigned short *DCpos_out, int *CBP_out, unsigned char *bp_prev);

// Copy Controller
void copyControl(unsigned char *recFrame,  //shared memory in
unsigned char hIn, unsigned char vIn,  // scalar fifo in
unsigned int *cc_regfile, int frameMemoryOffset, // parameters in 
unsigned char *bufferY, unsigned char *bufferU, unsigned char *bufferV, /*unsigned char *searchArea,*/ // shared memory out
unsigned char *hOut, unsigned char *vOut // scalar fifo out
);

// Motion decoder
unsigned char MotionDecoder(Bitstream *stream, unsigned char ftype, unsigned char coded, 
									 unsigned char skipped_flag, int x_pos, int y_pos, unsigned short DCpos, int MB_width, int f_code, SInt *mot_x, 
									 SInt *mot_y, SInt *mvx, SInt *mvy, unsigned char MBtype, SInt *slice_nb, 
									 unsigned short LB);
// Motion compensation
void MotionCompensateBlock(unsigned char *bufferY, unsigned char *bufferU, unsigned char *bufferV, //shared memory in
/*unsigned char *currentMB,*/ // object fifo in
unsigned char kIn, unsigned char hIn, unsigned char vIn, short mv_xIn, short mv_yIn,/* unsigned char modeIn,*/ // scalar fifo in
unsigned int *mc_regfile, // parameters
unsigned char *compBlock/*, short *errorBlock,*/ //object fifo out
/*unsigned char *kOut, unsigned char *modeOut, short *mv_xOut, short *mv_yOut, int *SADblock,unsigned char *hOut1, unsigned char *vOut1, unsigned char *hOut2, unsigned char *vOut2*/ // scalar fifo out
);

// Texture decoder
unsigned char TextureVLD(Bitstream *stream, unsigned char ftype, unsigned char btype, 
								 unsigned char comp, int CBP, SInt *q_block, int mbx, int mby);
// Texture/IDCT
unsigned char TextureIDCT(SInt *q_block, const unsigned char blnum, const int x_pos, const int y_pos, 
													const unsigned char btype, const unsigned char ACpred_flag, const unsigned short DCpos,  
													const int CBP, const unsigned char BufPatPrev[3], SInt *texture_block);

void textureUpdate(unsigned char *compBlock, short *textBlock, // object fifo in
unsigned char h, unsigned char v, unsigned char k, unsigned char tu_mode, // scalar fifo in
unsigned int *tu_regfile, int frameMemoryOffset, // parameters
unsigned char *recBlock, // object fifo out
unsigned char *hOut, unsigned char *vOut, unsigned char *kOut, // scalar fifo out
unsigned char *recFrame, // shared memory out
short DBPmvx, short DBPmvy, short round_type
);

// #ifdef WIN32

// Display Control 
// void displayControl(unsigned char *recBlock, //objectFifo
// unsigned char hIn, unsigned char vIn, unsigned blockNumber, //scalar fifo in
// unsigned int *dc_regfile, unsigned int frameMemoryOffset, // parameters in				
// CPicture *pic
// );

// void initDisplays(Vol *VOlist[],unsigned char totalNumberOfVOs,CPicture *picSet[]);
// void stopDisplays(unsigned char totalNumberOfVOs,CPicture *picSet[]);
// #endif //WIN32

// Write to output file
void WriteVopRaw(FILE *y_file, FILE *u_file, FILE *v_file, unsigned char *recFrame, int width, int height, unsigned int frameMemoryOffset) ;


#endif 
