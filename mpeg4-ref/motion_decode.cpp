/*
 *  Author: Kristof Denolf and ATOMIUM 
 *  Based on MPEG-4 reference code
 *  Date: November 28th, 2001
 *  IMEC
 */

#ifndef __AllInOne_h
#include "AllInOne.h"
#endif

static void DecodeMBVec(SInt *mot_x, SInt *mot_y, Bitstream *stream, unsigned char comp, int xpos, int ypos, unsigned short DCpos, int f_code, unsigned char *error_flag, SInt *mv_x, SInt *mv_y, SInt *slice_nb, int MB_width, unsigned short LB);
static int VlcDecMV(Bitstream *stream, unsigned char *error_flag);

/***********************************************************CommentBegin******
 *
 * -- MotionDecoder --  
 *
 * Author :
 *      Luis Ducla-Soares (IST) - lds@lx.it.pt
 *
 * Created :
 *      10.09.97
 *
 * Purpose :
 *       decodes the motion vectors of a MB
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ************************************************************CommentEnd********/
unsigned
char MotionDecoder(Bitstream *stream, unsigned char ftype, unsigned char coded, unsigned char skipped_flag, 
									 int x_pos, int y_pos, unsigned short DCpos, int MB_width, int f_code, 
									 SInt *mot_x, SInt *mot_y, SInt *mvx, SInt *mvy, unsigned char MBtype, SInt *slice_nb, 
									 unsigned short LB)
{
  unsigned char comp;
	short tmp, sign;
	unsigned char error_flag=0;
	
	// For SW instrumenting only!
#ifdef VERBOSE
	stream->bstart = stream->bitcount;
#endif

	if (ftype && coded)
	{
		if (MBtype <= 2)
		{
			if (MBtype == 2)
			{
				for (comp = 0; comp < 4; comp++)
				{
					DecodeMBVec(mot_x, mot_y, stream, comp, x_pos, y_pos, DCpos, f_code, &error_flag, mvx, mvy, slice_nb, MB_width, LB);
					if (error_flag) return error_flag;
				}
				
				tmp = (mvx[0] + mvx[1] + mvx[2] + mvx[3]);
				
				if (tmp < 0)
				{
					sign = -1;
					tmp = -tmp;
				}
				else
					sign = 1;

				if (tmp % 16 < 3)
					mvx[4] = mvx[5] = sign*(tmp/16*2);
				else if (tmp % 16 > 13)
					mvx[4] = mvx[5] = sign*((tmp/16*2) + 2);
				else
					mvx[4] = mvx[5] = sign*((tmp/16*2) + 1);

				tmp = (mvy[0] + mvy[1] + mvy[2] + mvy[3]);
				if (tmp < 0)
				{
					sign = -1;
					tmp = -tmp;
				}
				else
					sign = 1;

				if (tmp % 16 < 3)
					mvy[4] = mvy[5] = sign*(tmp/16*2);
				else if (tmp % 16 > 13)
					mvy[4] = mvy[5] = sign*((tmp/16*2) + 2);
				else
					mvy[4] = mvy[5] = sign*((tmp/16*2) + 1);
			}
			else
			{
				DecodeMBVec(mot_x, mot_y, stream, 0, x_pos, y_pos, DCpos, f_code, &error_flag, mvx, mvy, slice_nb, MB_width, LB);
				if (error_flag) return error_flag;
				
				tmp = mvx[1] = mvx[2] = mvx[3] = mvx[0];
				mvx[4] = mvx[5] = (tmp % 4 == 0 ? tmp >> 1 : tmp >> 1 | 1);

				tmp = mvy[1] = mvy[2] = mvy[3] = mvy[0];
				mvy[4] = mvy[5] = (tmp % 4 == 0 ? tmp >> 1 : tmp >> 1 | 1);
			}
			
			mot_x[2*DCpos] = mvx[2];
			mot_x[2*DCpos + 1] = mvx[3];
			mot_y[2*DCpos] = mvy[2];
			mot_y[2*DCpos + 1] = mvy[3];
    
		}
		else
		{
			mot_x[2*DCpos] = 0;
			mot_x[2*DCpos + 1] = 0;
			mot_y[2*DCpos] = 0;
			mot_y[2*DCpos + 1] = 0;
			mvx[1] = mvx[3] = 0; //only vectors at positions 1 and 3 needed next time in prediction
			mvy[1] = mvy[3] = 0; //only vectors at positions 1 and 3 needed next time in prediction
		}
	} // if ftype && coded

	// MBtype=0 (P-VOP only)
	//		NOTE: only vectors at positions 1 and 3 needed for next prediction
  if (skipped_flag)
	{
		mvx[1] = mvx[3] = 0;
		mvy[1] = mvy[3] = 0;
		mot_x[2*DCpos] = 0;
		mot_x[2*DCpos+1] = 0;
		mot_y[2*DCpos] = 0;
		mot_y[2*DCpos+1] = 0;

		mvx[0] = mvx[2] = mvx[4] = mvx[5] = 0; //to get old block to the display controller
		mvy[0] = mvy[2] = mvx[4] = mvx[5] = 0; //to get old block to the display controller

  }

#ifdef VERBOSE
	motion_bits += stream->bitcount - stream->bstart;
#endif
	return error_flag;
} // MotionDecoder


/***********************************************************CommentBegin******
 * 
 * -- DecodeMBVec -- 
 *
 * Author :
 *      
 *
 * Created :
 *      
 *
 * Purpose :
 *      Calculates a component of a block (or MB) vector by decoding 
 *      the magnitude & residual of the diff. vector, making the prediction, 
 *      and combining the decoded diff. and the predicted values
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

static void DecodeMBVec(SInt *mot_x, SInt *mot_y, Bitstream *stream, unsigned char comp, int xpos, int ypos, 
												unsigned short DCpos, int f_code, unsigned char *error_flag, SInt *mvx, SInt *mvy, SInt *slice_nb, 
												int MB_width, unsigned short LB)
{
  SInt pmvx = 0, pmvy = 0; 
	SInt residualx = 0, residualy = 0;
	SInt vlc_code_magx = 0, vlc_code_magy = 0;
	SInt p1x = 0, p2x = 0, p3x = 0;
  SInt p1y = 0, p2y = 0, p3y = 0;
  int rule1, rule2, rule3;
	int tmp = ypos*MB_width+xpos;
	unsigned short tmppos1 = (tmp-1)%LB;
	unsigned short tmppos2 = (tmp - MB_width)%LB;
	unsigned short tmppos3 = (tmp - MB_width + 1)%LB;
  int r_size, scale_factor, low, high, range, diff_vector;
	
  vlc_code_magx = (SInt) VlcDecMV(stream, error_flag);

  if (*error_flag)
    return;

  if (f_code > 1 && vlc_code_magx != 0)
    residualx = (short) BitstreamReadBits(stream, f_code); /*MV residual*/
  else
    residualx = 0;
  vlc_code_magy = (SInt) VlcDecMV(stream, error_flag);

  if (*error_flag)
    return;

  if (f_code > 1 && vlc_code_magy != 0)
    residualy = (short) BitstreamReadBits(stream, f_code - 1); /*MV residual*/
  else
    residualy = 0;

	//
	// Make the motion vectors prediction
	//
	if ((comp == 0 || comp == 2) && (xpos == 0  || (slice_nb[DCpos] != slice_nb[tmppos1])))
    rule1 = 1;
  else
    rule1 = 0;

  if ((comp == 0 || comp == 1) && (ypos == 0  || (slice_nb[DCpos] != slice_nb[tmppos2])))
    rule2 = 1;
  else
    rule2 = 0;

  if ((comp == 0 || comp == 1) && (xpos == MB_width - 1 || ypos == 0  || (slice_nb[DCpos] != slice_nb[tmppos3])))
    rule3 = 1;
  else
    rule3 = 0;

	if (comp == 0)
	{
	  if (!rule1)
		{
			p1x = mvx[1];
			p1y = mvy[1];
	  }

	  if (!rule2)
		{
			p2x = mot_x[2*tmppos2];
			p2y = mot_y[2*tmppos2];
		}

	  if (!rule3)
	  {
			p3x = mot_x[2*tmppos3];
			p3y = mot_y[2*tmppos3];
		}
	}

	if (comp == 1)
	{
	  if (!rule1)
		{
			p1x = mvx[0];
			p1y = mvy[0];
	  }

	  if (!rule2)
		{
			p2x = mot_x[2*tmppos2 + 1];
			p2y = mot_y[2*tmppos2 + 1];
		}

	  if (!rule3)
	  {
			p3x = mot_x[2*tmppos3];
			p3y = mot_y[2*tmppos3];
		}
	}

	if (comp == 2)
	{
	  if (!rule1)
		{
			p1x = mvx[3]; //still value of previous MB
			p1y = mvy[3]; //still value of previous MB
	  }

	  if (!rule2)
		{
			p2x = mvx[0];
			p2y = mvy[0];
		}

	  if (!rule3)
	  {
			p3x = mvx[1];
			p3y = mvy[1];
		}
	}

	if (comp == 3)
	{
	  if (!rule1)
		{
			p1x = mvx[2];
			p1y = mvy[2];
	  }

	  if (!rule2)
		{
			p2x = mvx[0];
			p2y = mvy[0];
		}

	  if (!rule3)
	  {
			p3x = mvx[1];
			p3y = mvy[1];
		}
	}

  if (rule1 && rule2 && rule3)
  {
    mvx[comp] = mvy[comp] = 0;
  }
  else 
  if (rule1 + rule2 + rule3 == 2)
  {
    mvx[comp] = (SInt)  (p1x + p2x + p3x);
    mvy[comp] = (SInt)  (p1y + p2y + p3y);
  }
  else
  {
    mvx[comp] = (SInt) ( (p1x + p2x + p3x - (p1x > (p2x > p3x ? p2x : p3x) ? p1x : p2x > p3x ? p2x : p3x) - (p1x < (p2x < p3x ? p2x : p3x) ? p1x : p2x < p3x ? p2x : p3x)));
    mvy[comp] = (SInt) ( (p1y + p2y + p3y - (p1y > (p2y > p3y ? p2y : p3y) ? p1y : p2y > p3y ? p2y : p3y) - (p1y < (p2y < p3y ? p2y : p3y) ? p1y : p2y < p3y ? p2y : p3y)));
  }
	
	//
	// De-scale MVDE
	//

	// Initialize parameters
	r_size = f_code - 1;
  scale_factor = 1 << r_size;
  range = 32 * scale_factor;
  low = -range;
  high = range - 1;

	// X direction
  if (scale_factor == 1 || vlc_code_magx == 0)
    diff_vector = vlc_code_magx;
  else
  {
    diff_vector = (((vlc_code_magx < 0 ? -vlc_code_magx : vlc_code_magx) - 1) << r_size) + residualx + 1;

    if (vlc_code_magx < 0)
      diff_vector = -diff_vector;
  }
  mvx[comp] += diff_vector;

  if (mvx[comp] < low)
    mvx[comp] += 2 * range;
  else if (mvx[comp] > high)
    mvx[comp] -= 2 * range;

	// Y direction
	if (scale_factor == 1 || vlc_code_magy == 0)
    diff_vector = vlc_code_magy;
  else
  {
    diff_vector = (((vlc_code_magy < 0 ? -vlc_code_magy : vlc_code_magy) - 1) << r_size) + residualy + 1;

    if (vlc_code_magy < 0)
      diff_vector = -diff_vector;
  }
  mvy[comp] += diff_vector;

  if (mvy[comp] < low)
    mvy[comp] += 2 * range;
  else if (mvy[comp] > high)
    mvy[comp] -= 2 * range;

} // DecodeMBVec


/***********************************************************CommentBegin******
 *
 * -- VlcDecMV -- Decodes the value of a VLC coded motion vector.
 *
 * Author :		
 *	Paulo Nunes (IST) - Paulo.Nunes@lx.it.pt
 *
 * Created :		
 *	1-Mar-96
 *
 * Purpose :		
 *	To decode the value of a VLC coded motion vector.
 *
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

static int VlcDecMV(Bitstream *stream, unsigned char *error_flag)
{
  UInt code;
  int temp;
	struct VLCtab TMNMVtab0[14] = { { 3, 4 }, { -3, 4 }, { 2, 3 }, { 2, 3 }, { -2, 3 }, { -2, 3 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, 
	{ -1, 2 }, { -1, 2 }, { -1, 2 }, { -1, 2 } };
	struct VLCtab TMNMVtab1[96] = { { 12, 10 }, { -12, 10 }, { 11, 10 }, { -11, 10 }, { 10, 9 }, { 10, 9 }, { -10, 9 }, { -10, 9 }, { 9, 9 }, 
	{ 9, 9 }, { -9, 9 }, { -9, 9 }, { 8, 9 }, { 8, 9 }, { -8, 9 }, { -8, 9 }, { 7, 7 }, { 7, 7 }, { 7, 7 }, { 7, 7 }, { 7, 7 }, { 7, 7 }, 
	{ 7, 7 }, { 7, 7 }, { -7, 7 }, { -7, 7 }, { -7, 7 }, { -7, 7 }, { -7, 7 }, { -7, 7 }, { -7, 7 }, { -7, 7 }, { 6, 7 }, { 6, 7 }, { 6, 7 }, 
	{ 6, 7 }, { 6, 7 }, { 6, 7 }, { 6, 7 }, { 6, 7 }, { -6, 7 }, { -6, 7 }, { -6, 7 }, { -6, 7 }, { -6, 7 }, { -6, 7 }, { -6, 7 }, { -6, 7 }, 
	{ 5, 7 }, { 5, 7 }, { 5, 7 }, { 5, 7 }, { 5, 7 }, { 5, 7 }, { 5, 7 }, { 5, 7 }, { -5, 7 }, { -5, 7 }, { -5, 7 }, { -5, 7 }, { -5, 7 }, 
	{ -5, 7 }, { -5, 7 }, { -5, 7 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, 
	{ 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, 
	{ -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 }, { -4, 6 } };
	struct VLCtab TMNMVtab2[124] = { { 32, 12 }, { -32, 12 }, { 31, 12 }, { -31, 12 }, { 30, 11 }, { 30, 11 }, { -30, 11 }, { -30, 11 }, 
	{ 29, 11 }, { 29, 11 }, { -29, 11 }, { -29, 11 }, { 28, 11 }, { 28, 11 }, { -28, 11 }, { -28, 11 }, { 27, 11 }, { 27, 11 }, { -27, 11 }, 
	{ -27, 11 }, { 26, 11 }, { 26, 11 }, { -26, 11 }, { -26, 11 }, { 25, 11 }, { 25, 11 }, { -25, 11 }, { -25, 11 }, { 24, 10 }, { 24, 10 }, 
	{ 24, 10 }, { 24, 10 }, { -24, 10 }, { -24, 10 }, { -24, 10 }, { -24, 10 }, { 23, 10 }, { 23, 10 }, { 23, 10 }, { 23, 10 }, { -23, 10 }, 
	{ -23, 10 }, { -23, 10 }, { -23, 10 }, { 22, 10 }, { 22, 10 }, { 22, 10 }, { 22, 10 }, { -22, 10 }, { -22, 10 }, { -22, 10 }, { -22, 10 }, 
	{ 21, 10 }, { 21, 10 }, { 21, 10 }, { 21, 10 }, { -21, 10 }, { -21, 10 }, { -21, 10 }, { -21, 10 }, { 20, 10 }, { 20, 10 }, { 20, 10 }, 
	{ 20, 10 }, { -20, 10 }, { -20, 10 }, { -20, 10 }, { -20, 10 }, { 19, 10 }, { 19, 10 }, { 19, 10 }, { 19, 10 }, { -19, 10 }, { -19, 10 }, 
	{ -19, 10 }, { -19, 10 }, { 18, 10 }, { 18, 10 }, { 18, 10 }, { 18, 10 }, { -18, 10 }, { -18, 10 }, { -18, 10 }, { -18, 10 }, { 17, 10 }, 
	{ 17, 10 }, { 17, 10 }, { 17, 10 }, { -17, 10 }, { -17, 10 }, { -17, 10 }, { -17, 10 }, { 16, 10 }, { 16, 10 }, { 16, 10 }, { 16, 10 }, 
	{ -16, 10 }, { -16, 10 }, { -16, 10 }, { -16, 10 }, { 15, 10 }, { 15, 10 }, { 15, 10 }, { 15, 10 }, { -15, 10 }, { -15, 10 }, { -15, 10 }, 
	{ -15, 10 }, { 14, 10 }, { 14, 10 }, { 14, 10 }, { 14, 10 }, { -14, 10 }, { -14, 10 }, { -14, 10 }, { -14, 10 }, { 13, 10 }, { 13, 10 }, 
	{ 13, 10 }, { 13, 10 }, { -13, 10 }, { -13, 10 }, { -13, 10 }, { -13, 10 } };

  if (BitstreamReadBits(stream, 1)) /*motion_code*/
  {
    return 0;
  }

  if ((code = BitstreamShowBits(stream, 12)) >= 512)
  {
    code = (code >> 8) - 2;
    BitstreamFlushBits(stream, (TMNMVtab0[code]).len);
    return (TMNMVtab0[code]).val;
  }

  if (code >= 128)
  {
    code = (code >> 2) - 32;
    BitstreamFlushBits(stream, (TMNMVtab1[code]).len);
    return (TMNMVtab1[code]).val;
  }

  temp = code - 4;

  if (temp < 0)
  {
    *error_flag = 1;
    return -1;
  }

  code -= 4;
  BitstreamFlushBits(stream, (TMNMVtab2[code]).len);
  return (TMNMVtab2[code]).val;
} // VlcDecMV

