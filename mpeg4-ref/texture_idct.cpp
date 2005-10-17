/*
 *  Authors: Kristof Denolf (IMEC) and Paul Schumacher (Xilinx)
 *  Based on MPEG-4 reference code
 *  Date: July 4th 2002
 */

#include "AllInOne.h"

// IDCT constants
#define W1 2841 /* 2048*sqrt(2)*cos(1*pi/16) */
#define W2 2676 /* 2048*sqrt(2)*cos(2*pi/16) */
#define W3 2408 /* 2048*sqrt(2)*cos(3*pi/16) */
#define W5 1609 /* 2048*sqrt(2)*cos(5*pi/16) */
#define W6 1108 /* 2048*sqrt(2)*cos(6*pi/16) */
#define W7 565  /* 2048*sqrt(2)*cos(7*pi/16) */

// Function prototypes
static UChar* DCACreconstruct(SInt *q_block, SInt *localDCstore, SInt *DCstore, unsigned char ftype, const UChar *BufPatPrev, 
															unsigned char QP, unsigned short prevRow, unsigned short prevCol, unsigned short prevRowCol, 
															unsigned short DCpos, int posx, int posy, int ACpred_flag, unsigned char blnum, UInt CBP, 
															char dc_scaler, const SInt SliceNum[4]);
static short inv_zigzag_dequant(short *in, unsigned char QP, char dc_scaler, UChar *zigzag, short *out);
static void BlockIDCT(short *in, char dc_scaler, short *out);
static void idctrow(short *in, short *out);
static void idctcol(short *blk, char dc_scaler);
static char cal_dc_scaler(unsigned char QP, unsigned char btype, unsigned char blnum);

/***********************************************************CommentBegin******
 *
 * -- TextureIDCT --  decodes the texture/DCT 
 *
 * Author :		
 *	P. Schumacher, Xilinx
 *
 * Created :		
 *	2004/01/14
 * 
 * Purpose :		
 *      Decodes the texture/DCT information
 *
 ***********************************************************CommentEnd********/
// Values							Bits
// ------							----
// Inputs
//		q_block					12 [64 values]
//		frame type			 1
//		block type			 1
//		block # or blnum	 3
//		block pattern		 6
//		quant step			 5
//		X position			10
//		Y position			10
//		DC position			 4
//		AC pred flag		 1
//		buffer pattern	 8 [LB values]
//		slice #	array		 8 [LB values]				 
// Parameters
//		bits/pixel			 4
//		blocks/MB				 4
//		MB width				 5
//		VOP width				10
// Outputs
//		texture block		 9 [64 values]
unsigned char TextureIDCT(SInt *q_block, const unsigned char blnum, const int posx, const int posy, 
													const unsigned char btype, const unsigned char ACpred_flag, const unsigned short DCpos,  
													const int CBP, const unsigned char BufPatPrev[3], SInt *texture_block)
{
	// Register file
	const UChar ftype = (idct_regfile[0] >> 11) & 0x1;
	const UChar quant_step = (idct_regfile[0] >> 6) & 0x1F;
	const UChar MBwidth = (idct_regfile[0] >>  0) & 0x3F;
	/* DBP */ const UChar MBheight = (idct_regfile[0] >>  16) & 0x3F;

	const int width = MBwidth * MB_WIDTH;

	// For now, disable slice numbers
	//const short SliceNum[4] = {idct_regfile[12], idct_regfile[13], idct_regfile[14], idct_regfile[15]};
	const short SliceNum[4] = {0, 0, 0, 0};

	unsigned char error_flag=0;
	int m;
	short DC;
	char dc_scaler;
	unsigned char *zz;
	SInt dequant_block[B_SIZE];
  static int flag_count = 0;

	const int MB_in_width = width / MB_WIDTH;
	const int LB = MB_in_width + 2;
	const unsigned short prevRow = ((posy-1)*MB_in_width + posx)%LB;
	const unsigned short prevCol = (posy*MB_in_width + posx -1)%LB;
  const unsigned short prevRowCol = ((posy-1)*MB_in_width + posx - 1)%LB;
	// const unsigned char ACcoded = CBP & (1 << (BLOCK_CNT - 1 - blnum));
	static UChar zigzag[64] = { 0, 1, 5, 6, 14, 15, 27, 28, 2, 4, 7, 13, 16, 26, 29, 42, 3, 8, 12, 17, 25, 30, 41, 43, 9, 11, 18, 24, 
		31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 
		57, 58, 62, 63 };
	static short DCstore[15*4*LB_MAX], localDCstore[15*4];

  static FILE *fi_param = 0, *fi_data = 0, *fi_flags = 0,  *fo = 0;
  static int fcount = 0;
  #define INAMEtexparam "texture_param.dat"
  #define INAMEtexdata "texture_data.dat"
  #define INAMEtexflags "texture_flags.dat"
  #define ONAMEtex "texture_out.dat"
  #define NCOUNTtex 70000

  // if( fcount < NCOUNTtex )
  { short t;
    int j;
    static int lastframe=-1;
    if( fi_data == 0 )
    { fi_param = fopen( INAMEtexparam, "wb" );
      fi_data  = fopen( INAMEtexdata , "wb" );
      fi_flags = fopen( INAMEtexflags, "wb" );
      fo = fopen( ONAMEtex, "wb" );
    }
    ++fcount;
    if( framenum != lastframe )
    { t = -1      ; fwrite( &t, 2,  1, fi_param );
      t = MBwidth ; fwrite( &t, 2,  1, fi_param );
      t = MBheight; fwrite( &t, 2,  1, fi_param );
      t = -1      ; fwrite( &t, 2,  1, fi_param );
      lastframe = framenum;
    }
    t = btype   ; fwrite( &t, 2,  1, fi_param ); 
    t = posx    ; fwrite( &t, 2,  1, fi_param );
    t = posy    ; fwrite( &t, 2,  1, fi_param );
    t = blnum   ; fwrite( &t, 2,  1, fi_param );

    for( j=0; j<64; j++ )
    { t = q_block[j];
      fwrite( &t , 2, 1, fi_data );
    }

    t = CBP & (1 << (BLOCK_CNT - 1 - blnum)) ? 1 : 0;
    if( ACpred_flag ) t |= 2;
    flag_count++;
    if( flag_count == 4658 )
    {
      j = 0;
    }

    fwrite( &t , 2, 1, fi_flags );
    t = quant_step; fwrite( &t , 2, 1, fi_flags );
    t = cal_dc_scaler(quant_step, btype, blnum); fwrite( &t , 2, 1, fi_flags );

    if( fcount == 586 )
    {
      t = 1;
    }

  }
/*  else if( fi_data && framenum)
  { fclose( fi_param );
    fclose( fi_data );
    fclose( fi_flags);
    fclose( fo );
    fi_param = fi_data = fo = 0;
  } */
 
	dc_scaler = cal_dc_scaler(quant_step, btype, blnum);


	//
	// DC/AC reconstruction (if needed)
	//
	if (btype == INTER)
	{
		zz = zigzag;
		#ifdef DEBUG_IDCT
		printf("zzselect = 0\n");
		#endif
	}
	else
	{
		zz = DCACreconstruct(q_block, localDCstore, DCstore, ftype, BufPatPrev, quant_step, prevRow, prevCol, prevRowCol, 
			DCpos, posx, posy, ACpred_flag, blnum, CBP, dc_scaler, SliceNum);

		if (blnum > 1)
		{
			for (m = 1; m < 8; m++)
      {
				DCstore[(DCpos*4+blnum-2)*15 + m] = q_block[zz[m]];
				DCstore[(DCpos*4+blnum-2)*15 + m + 7] = q_block[zz[m  * 8]];
			}
		}
		else
		{
			for (m = 1; m < 8; m++)
			{
				localDCstore[(2*(posx%2) + blnum)*15 + m] = q_block[zz[m]];
				localDCstore[(2*(posx%2) + blnum)*15 + m + 7] = q_block[zz[m * 8]];
			}
		}
	}

	//
	// Inverse zigzag & dequantization
	//
	DC = inv_zigzag_dequant(q_block, quant_step, dc_scaler, zz, dequant_block);
	
	//
	// Perform IDCT
	//
	BlockIDCT(dequant_block, dc_scaler, texture_block);

	// 
	// Store DC value
	// NOTE: can be done at same time as processing in HW
	//
	if (btype == INTRA)
	{
		if (blnum > 1)
			DCstore[(DCpos*4 + blnum - 2)*15] = DC;
		else
			localDCstore[(2*(posx%2) + blnum)*15] = DC;

	} 

#ifdef DEBUG_IDCT
	printf("DC values: input = %d, final = %d (DC scaler = %d)\n", q_block[0], DC, dc_scaler);
#endif

	// For SW instrumenting only
#ifdef VERBOSE
	texture_blocks++;
#endif
	return error_flag;
} // TextureIDCT


/***********************************************************CommentBegin******
 *
 * -- DCACreconstruct -- Does DC/AC reconstruction. Changes qcoeff values as
 *		           appropriate.
 *
 * Author :		
 *	Luis Ducla Soares (IST) - lds@lx.it.pt
 *
 * Created :		
 *	23.10.96
 *
 * Purpose :		
 *	Does DC/AC reconstruction. Changes qcoeff values as appropriate. 
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

static unsigned char* DCACreconstruct(SInt *q_block, SInt *localDCstore, SInt *DCstore, unsigned char ftype, 
																			const UChar BufPatPrev[3], unsigned char QP, unsigned short prevRow, unsigned short prevCol, 
																			unsigned short prevRowCol, unsigned short DCpos, int posx, int posy, int ACpred_flag, 
																			unsigned char blnum, UInt CBP, char dc_scaler, const SInt SliceNum[4])
{
	SInt block_A, block_B, block_C;
  SInt grad_hor, grad_ver, DC_pred;
  SInt tmpval;
	unsigned char direction;
	unsigned char prevRowInter, prevColInter, prevRowColInter;
	int m;
	unsigned char *zz;
	static UChar zigzag[64] = { 0, 1, 5, 6, 14, 15, 27, 28, 2, 4, 7, 13, 16, 26, 29, 42, 3, 8, 12, 17, 25, 30, 41, 43, 9, 11, 
		18, 24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 
		35, 36, 48, 49, 57, 58, 62, 63 };
	static UChar zigzag_v[64] = { 0, 4, 6, 20, 22, 36, 38, 52, 1, 5, 7, 21, 23, 37, 39, 53, 2, 8, 19, 24, 34, 40, 50, 54, 3, 
		9, 18, 25, 35, 41, 51, 55, 10, 17, 26, 30, 42, 46, 56, 60, 11, 16, 27, 31, 43, 47, 57, 61, 12, 15, 28, 32, 44, 48, 58, 
		62, 13, 14, 29, 33, 45, 49, 59, 63 };
	static UChar zigzag_h[64] = { 0, 1, 2, 3, 10, 11, 12, 13, 4, 5, 8, 9, 17, 16, 15, 14, 6, 7, 19, 18, 26, 27, 28, 29, 20, 
		21, 24, 25, 30, 31, 32, 33, 22, 23, 34, 35, 42, 43, 44, 45, 36, 37, 40, 41, 46, 47, 48, 49, 38, 39, 50, 51, 56, 57, 58, 
		59, 52, 53, 54, 55, 60, 61, 62, 63 };

  if (posx == 0 && posy == 0)
	{ 
		prevRowInter = prevColInter = prevRowColInter = 1;
  }
  else if (posx == 0)
  {
		prevColInter = prevRowColInter = 1;
		if (ftype)
		{
			prevRowInter = BufPatPrev[0];
		}
		else
			prevRowInter = 0;
  }
  else if (posy == 0)
  {
		prevRowInter = prevRowColInter = 1;
		if (ftype)
		{
			prevColInter = BufPatPrev[1];
		}
		else
			prevColInter = 0;
  }
	else
	{
		if (ftype)
		{
			prevRowInter = BufPatPrev[0];
			prevColInter = BufPatPrev[1];
			prevRowColInter = BufPatPrev[2];
		}
		else
			prevRowInter = prevColInter = prevRowColInter = 0;
	}

	if (blnum == 0)
	{
		if (prevColInter || (SliceNum[2] != SliceNum[0]))
			block_A = MIDVAL_PIXEL * 8;
		else
			block_A = localDCstore[(2*((posx-1)%2) + 1)*15];

		if (prevRowColInter || (SliceNum[3] != SliceNum[0]))
			block_B = MIDVAL_PIXEL * 8;
		else
			block_B = DCstore[(prevRowCol*4 + 1)*15];

		if (prevRowInter || (SliceNum[1] != SliceNum[0]))
			block_C = MIDVAL_PIXEL * 8;
		else
			block_C = DCstore[prevRow*4*15];
	}
	else if (blnum == 1)
	{
		block_A = localDCstore[2*(posx%2)*15];

		if (prevRowInter || (SliceNum[1] != SliceNum[0]))
			block_B = block_C = MIDVAL_PIXEL * 8;
		else
		{
			block_B = DCstore[(prevRow*4)*15];
			block_C = DCstore[(prevRow*4 + 1)*15];
		}
	}
	else if (blnum == 2)
	{
		if (prevColInter || (SliceNum[2] != SliceNum[0]))
			block_A =  block_B = MIDVAL_PIXEL * 8;
		else
		{
			block_A = DCstore[(prevCol*4 + 1)*15];
			block_B = localDCstore[(2*((posx-1)%2)+1)*15];
		}

		block_C = localDCstore[2*((posx)%2)*15];
	}
	else if (blnum == 3)
	{
		block_A = DCstore[(DCpos*4)*15];
		block_B = localDCstore[2*((posx)%2)*15];
		block_C = localDCstore[(2*((posx)%2) + 1)*15];
	}
	else if (blnum == 4 || blnum == 5)
	{
		if (prevColInter || (SliceNum[2] != SliceNum[0]))
			block_A = MIDVAL_PIXEL * 8;
		else
			block_A = DCstore[(prevCol*4 + blnum - 2)*15];

		if (prevRowColInter || (SliceNum[3] != SliceNum[0]))
			block_B = MIDVAL_PIXEL * 8;
		else
			block_B = DCstore[(prevRowCol*4 + blnum - 2)*15];

		if (prevRowInter || (SliceNum[1] != SliceNum[0]))
			block_C = MIDVAL_PIXEL * 8;
		else
			block_C = DCstore[(prevRow*4 + blnum - 2)*15];
	}

  grad_hor = block_B - block_C;
  grad_ver = block_A - block_B;

  if ((grad_ver < 0 ? -grad_ver : grad_ver) < (grad_hor < 0 ? -grad_hor : grad_hor))
  {
    DC_pred = block_C;
    direction = 1;
  }
  else
  {
    DC_pred = block_A;
    direction = 0;
  }
  
	//tmpval = q_block[0] + (DC_pred + dc_scaler / 2) / dc_scaler;
	//tmpval = (q_block[0] * dc_scaler) + DC_pred;

	tmpval = (q_block[0] * dc_scaler) + ((DC_pred + dc_scaler / 2) / dc_scaler) * dc_scaler;
#ifdef DEBUG_IDCT
printf("DC = %d = (%d * %d) + (((%d + %d) / %d) * %d)\n", tmpval, q_block[0], dc_scaler, DC_pred, dc_scaler / 2, dc_scaler, dc_scaler);
#endif
	q_block[0] = tmpval < -2048 ? -2048 : tmpval > 2047 ? 2047 : tmpval;

	

  if (!(CBP & (1 << (BLOCK_CNT - 1 - blnum))) && !ACpred_flag) //only DC prediction needed
	{
		#ifdef DEBUG_IDCT
		printf("zzselect = 0\n");
		#endif
		return zigzag;
	}
	else //perform AC prediction
	{
	  if (ACpred_flag == 1)
    {
		  if (direction == 0)
		  {
				zz = zigzag_v;
				#ifdef DEBUG_IDCT
				printf("zzselect = 2\n");
				#endif
			}
			else
			{
				zz = zigzag_h;
				#ifdef DEBUG_IDCT
				printf("zzselect = 3\n");
				#endif
			}
    }
		else
		{
			zz = zigzag;
			#ifdef DEBUG_IDCT
			printf("zzselect = 0\n");
			#endif
		}
  
		if (ACpred_flag == 1)
		{
	    if (blnum == 0)
			{
				if (direction) //predict from block C
				{
					if (!prevRowInter && (SliceNum[1] == SliceNum[0])) //general case
						for (m = 1; m < 8; m++)
							q_block[zz[m]] += DCstore[(prevRow*4)*15 + m];
				}
				else // predict from block A
				{
					if (!prevColInter && (SliceNum[2] == SliceNum[0]))
						for (m = 1; m < 8; m++)
							q_block[zz[m*8]] += localDCstore[(2*((posx-1)%2) + 1)*15 + m + 7];
				}
			}
			else if (blnum == 1)
			{
				if (direction) //predict from block C
				{
					if (!prevRowInter && (SliceNum[1] == SliceNum[0]))
						for (m = 1; m < 8; m++)
							q_block[zz[m]] += DCstore[(prevRow*4 + 1)*15 + m];
				}
				else //predict from block A
					for (m = 1; m < 8; m++)
						q_block [zz[m*8]] += localDCstore[(2*(posx%2))*15 + m + 7];

			}
			else if (blnum == 2)
			{
				if (direction) //predict from block C
					for (m = 1; m < 8; m++)
						q_block[zz[m]] += localDCstore[2*(posx%2)*15 + m];
				else //predict from block A
				{
					if (!prevColInter && (SliceNum[2] == SliceNum[0]))
						for (m = 1; m < 8; m++)
							q_block [zz[m*8]] += DCstore[(prevCol*4 + 1)*15 + m + 7];
				}
			}
			else if (blnum == 3)
			{
				if (direction) //predict from block C
					for (m = 1; m < 8; m++)
						q_block[zz[m]] += localDCstore[(2*(posx%2)+1)*15 + m];
				else //predict from block A
					for (m = 1; m < 8; m++)
						q_block [zz[m*8]] += DCstore[(DCpos*4 )*15 + m + 7];
			}
			else if (blnum == 4 || blnum == 5)
			{
				if (direction) //predict from block C
				{
					if (!prevRowInter && (SliceNum[1] == SliceNum[0]))
						for (m = 1; m < 8; m++)
							q_block[zz[m]] += DCstore[(prevRow*4 + blnum - 2)*15 + m];
				}
				else //predict  from block A
				{
					if (!prevColInter && (SliceNum[2] == SliceNum[0]))
						for (m = 1; m < 8; m++)
							q_block [zz[m*8]] += DCstore[(prevCol*4 + blnum - 2)*15 + m + 7];
				}
			}
		}
	}

	return zz;
}


//
// Inverse zigzag and dequantization
//
static short inv_zigzag_dequant(short *in, unsigned char QP, char dc_scaler, UChar *zigzag, short *out)
{
	int n, round;
	short DC=0; //initialized by DBP

	round = !(QP & 1);
	
	for (n=0; n < B_SIZE; n++)
	{
		// Inverse zigzag
		out[n] = in[zigzag[n]];
		//printf("n = %d: in[%d] = %d\n", n, zigzag[n], out[n]);

		// Dequantization
		if (out[n] != 0)
		{		
			// DC value
			if (n == 0)
			{
				if (dc_scaler)
				{						
					DC = out[n] = out[n] < -2048 ? -2048 : out[n] > 2047 ? 2047 : out[n]; /*saturation*/
				}
				else
				{
					if (out[n] < 0)
					{
						out[n] = QP * (((-out[n])<<1) + 1) - round;
						out[n] = out[n] > 2048 ? -2048 : -out[n];
					}
					else
					{
						out[n] = QP * ((out[n]<<1) + 1) - round;
						out[n] = out[n] > 2047 ? 2047 : out[n];
					}
				}
			}
			// AC values
			else 
			{
				 
				if (out[n] < 0)
				{
//printf("out[%d] = %d = -((%d * %d) - %d) [NEGATIVE]\n", n, -(QP * (((-out[n])<<1) + 1) - round), QP, (((-out[n])<<1) + 1), round);
					out[n] = QP * (((-out[n])<<1) + 1) - round;
					out[n] = out[n] > 2048 ? -2048 : -out[n];
				}
				else
				{
//printf("out[%d] = %d = (%d * %d) - %d [NEGATIVE]\n", n, QP * ((out[n]<<1) + 1) - round, QP, (((out[n])<<1) + 1), round);
					out[n] = QP * ((out[n]<<1) + 1) - round;
					out[n] = out[n] > 2047 ? 2047 : out[n];
				}
			}
		}
	} // for n

#ifdef DEBUG_IDCT
	for (n=0; n < B_SIZE; n++)
		printf("out[%d] = %d\n", n, out[n]);
#endif

	return DC;
} // inv_zigzag_dequant


//
// Block IDCT function
//
static void BlockIDCT(short *in, char dc_scaler, short *out)
{
	unsigned char i;

  static FILE *fi = 0, *fo = 0;
  static int fcount = 0;
  #define INAMEblk "blockidct_in.dat"
  #define ONAMEblk "blockidct_out.dat"
  #define NCOUNTblk 1000

  if( fcount < NCOUNTblk )
  { int dc = dc_scaler;
    static int lastframe=-1;

    if( framenum != lastframe )
    {
      lastframe = framenum;
    }

    if( fi == 0 )
    { fi = fopen( INAMEblk, "wb" );
      fo = fopen( ONAMEblk, "wb" );
    }
    //++fcount;
    fwrite( &dc, 2,  1, fi );
    fwrite( in , 2, 64, fi );
  }
  else if( fi )
  { fclose( fi );
    fclose( fo );
    fi = fo = 0;
  }
  
	// 1-D IDCT on rows
	for (i=0; i<8; i++)
		idctrow(in+(i<<3), out+(i<<3));

	// 1-D IDCT on columns
  for (i=0; i<8; i++)
    idctcol(out+i, dc_scaler);

  if( fo ) fwrite( out, 2, 64, fo );

	return;
} // BlockIDCT


/* row (horizontal) IDCT
 *
 *           7                       pi         1
 * dst[k] = sum c[l] * src[l] * cos( -- * ( k + - ) * l )
 *          l=0                      8          2
 *
 * where: c[0]    = 128
 *        c[1..7] = 128*sqrt(2)
 */
static void idctrow(short *in, short *out)
{
	int tmpval;
	int x[B_WIDTH];

  static FILE *fi = 0, *fo = 0;
  static int fcount = 0;
  #define INAMErow "idctrow_in.dat"
  #define ONAMErow "idctrow_out.dat"
  #define NCOUNTrow 0

  if( fcount < NCOUNTrow )
  { if( fi == 0 )
    { fi = fopen( INAMErow, "wb" );
      fo = fopen( ONAMErow, "wb" );
    }
    ++fcount;
    fwrite( in, 2, 8, fi );
  }
  else if( fi )
  { fclose( fi );
    fclose( fo );
    fi = fo = 0;
  }
				
	/* first stage */
	x[0] = (in[0]<<11) + 128; /* for proper rounding in the fourth stage */
	x[4] = in[4]<<11;
	tmpval = W7*(in[1]+in[7]);
	x[1] = tmpval + (W1-W7)*in[1];
	x[7] = tmpval - (W1+W7)*in[7];
	tmpval = W3*(in[5]+in[3]);
	x[5] = tmpval - (W3-W5)*in[5];
	x[3]= tmpval - (W3+W5)*in[3];
	x[2] = in[2];
	x[6] = in[6];
			
	/* second stage */
	tmpval = x[0] + x[4];
	x[0] -= x[4];
	x[4] = W6*(x[2]+x[6]);
	x[6] = x[4] - (W2+W6)*x[6];
	x[2] = x[4] + (W2-W6)*x[2];
	x[4] = x[1] + x[5];
	x[1] -= x[5];
	x[5] = x[7] + x[3];
	x[7] -= x[3];
			
	/* third stage */
	x[3] = tmpval + x[2];
	tmpval -= x[2];
	x[2] = x[0] + x[6];
	x[0] -= x[6];
	x[6] = (181*(x[1]+x[7])+128)>>8;
	x[1] = (181*(x[1]-x[7])+128)>>8;
			
	/* fourth stage */
	out[0] = (x[3]+x[4])>>8;
	out[1] = (x[2]+x[6])>>8;
	out[2] = (x[0]+x[1])>>8;
	out[3] = (tmpval+x[5])>>8;
	out[4] = (tmpval-x[5])>>8;
	out[5] = (x[0]-x[1])>>8;
	out[6] = (x[2]-x[6])>>8;
	out[7] = (x[3]-x[4])>>8;

   if( fo ) fwrite( out, 2, 8, fo );
 
	return;
} // idctrow


/* column (vertical) IDCT
 *
 *             7                         pi         1
 * dst[8*k] = sum c[l] * src[8*l] * cos( -- * ( k + - ) * l )
 *            l=0                        8          2
 *
 * where: c[0]    = 1/1024
 *        c[1..7] = (1/1024)*sqrt(2)
 */
static void idctcol(short *blk, char dc_scaler)
{
	int v;
  int x[9];
	short negMaxval;
  short short_dc_scaler = dc_scaler;

  static FILE *fi = 0, *fo = 0;
  static int fcount = 0;
  #define INAMEcol "idctcol_in.dat"
  #define ONAMEcol "idctcol_out.dat"
  #define NCOUNTcol 0

  if( fcount < NCOUNTcol )
  { if( fi == 0 )
    { fi = fopen( INAMEcol, "wb" );
      fo = fopen( ONAMEcol, "wb" );
    }
    ++fcount;
    fwrite( &short_dc_scaler, 2, 1, fi );
    fwrite( blk +  0, 2, 1, fi );
    fwrite( blk + 32, 2, 1, fi );
    fwrite( blk + 48, 2, 1, fi );
    fwrite( blk + 16, 2, 1, fi );
    fwrite( blk +  8, 2, 1, fi );
    fwrite( blk + 56, 2, 1, fi );
    fwrite( blk + 40, 2, 1, fi );
    fwrite( blk + 24, 2, 1, fi );
  }
  else if( fi )
  { fclose( fi );
    fclose( fo );
    fi = fo = 0;
  }

	if (dc_scaler)
		negMaxval = 0;
	else
		negMaxval = -MAXVAL_PIXEL - 1;
	
  /* shortcut */
  if (!((x[1] = (blk[8*4]<<8)) | (x[2] = blk[8*6]) | (x[3] = blk[8*2]) |
		(x[4] = blk[8*1]) | (x[5] = blk[8*7]) | (x[6] = blk[8*5]) | (x[7] = blk[8*3])))
  {
		v = (blk[8*0]+32)>>6;
		blk[8*0]=blk[8*1]=blk[8*2]=blk[8*3]=blk[8*4]=blk[8*5]=blk[8*6]=blk[8*7] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;

    if( fo )
    { fwrite( blk +  0  , 2, 1, fo );
      fwrite( blk +  8  , 2, 1, fo );
      fwrite( blk + 16  , 2, 1, fo );
      fwrite( blk + 24  , 2, 1, fo );
      fwrite( blk + 32  , 2, 1, fo );
      fwrite( blk + 40  , 2, 1, fo );
      fwrite( blk + 48  , 2, 1, fo );
      fwrite( blk + 56  , 2, 1, fo );
    } 
   return;
  }
	
  x[0] = (blk[8*0]<<8) + 8192;
	
  /* first stage */
  x[8] = W7*(x[4]+x[5]) + 4;
  x[4] = (x[8]+(W1-W7)*x[4])>>3;
  x[5] = (x[8]-(W1+W7)*x[5])>>3;
  x[8] = W3*(x[6]+x[7]) + 4;
  x[6] = (x[8]-(W3-W5)*x[6])>>3;
  x[7] = (x[8]-(W3+W5)*x[7])>>3;
  
  /* second stage */
  x[8] = x[0] + x[1];
  x[0] -= x[1];
  x[1] = W6*(x[3]+x[2]) + 4;
  x[2] = (x[1]-(W2+W6)*x[2])>>3;
  x[3] = (x[1]+(W2-W6)*x[3])>>3;
  x[1] = x[4] + x[6];
  x[4] -= x[6];
  x[6] = x[5] + x[7];
  x[5] -= x[7];
  
  /* third stage */
  x[7] = x[8] + x[3];
  x[8] -= x[3];
  x[3] = x[0] + x[2];
  x[0] -= x[2];
  x[2] = (181*(x[4]+x[5])+128)>>8;
  x[4] = (181*(x[4]-x[5])+128)>>8;
  
  /* fourth stage */
	v = (x[7]+x[1])>>14;
	blk[8*0] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[3]+x[2])>>14;
  blk[8*1] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v =  (x[0]+x[4])>>14;
  blk[8*2] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[8]+x[6])>>14;
  blk[8*3] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[8]-x[6])>>14;
  blk[8*4] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[0]-x[4])>>14;
  blk[8*5] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[3]-x[2])>>14;
  blk[8*6] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;
	v = (x[7]-x[1])>>14;
  blk[8*7] = v < negMaxval ? negMaxval : v > MAXVAL_PIXEL ? MAXVAL_PIXEL : v;

    if( fo )
    { fwrite( blk +  0  , 2, 1, fo );
      fwrite( blk +  8  , 2, 1, fo );
      fwrite( blk + 16  , 2, 1, fo );
      fwrite( blk + 24  , 2, 1, fo );
      fwrite( blk + 32  , 2, 1, fo );
      fwrite( blk + 40  , 2, 1, fo );
      fwrite( blk + 48  , 2, 1, fo );
      fwrite( blk + 56  , 2, 1, fo );
    } 

	return;
} // idctcol


/***********************************************************CommentBegin******
 *
 * -- cal_dc_scaler -- calculation of DC quantization scale according to the incoming Q and type; 
 *
 * Author : Minhua Zhou 		
 *	
 *
 * Created : 04.11.97		
 *	
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

static char cal_dc_scaler(unsigned char QP, unsigned char btype, unsigned char blnum)
{
  char dc_scaler;
	
	if (btype == INTER) 
		dc_scaler = 0;
	else
	{
		if (blnum < 4)
		{
			if (QP > 0 && QP < 5)
				dc_scaler = 8;
			else if (QP > 4 && QP < 9)
				dc_scaler = 2 * QP;
			else if (QP > 8 && QP < 25)
				dc_scaler = QP + 8;
			else
				dc_scaler = 2 * QP - 16;
		}
		else
		{
			if (QP > 0 && QP < 5)
				dc_scaler = 8;
			else if (QP > 4 && QP < 25)
				dc_scaler = (QP + 13) / 2;
			else
				dc_scaler = QP - 6;
		}
	}
	
  return dc_scaler;
} // cal_dc_scaler
