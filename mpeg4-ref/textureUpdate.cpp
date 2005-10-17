/* 
 *  Author: Kristof Denolf
 *  Based on MPEG-4 reference code  
 *  Date: October 23rd 2003  
 *  IMEC
 *  Update: May 6th, Kristof Denolf (IMEC), add output to display control
 */ 

#ifndef __textureUpdate_c
#define __textureUpdate_c


#ifndef __AllInOne_h
#include "AllInOne.h"
#endif

void textureUpdate(unsigned char *compBlock, short *textBlock, // object fifo in
unsigned char h, unsigned char v, unsigned char k, unsigned char tu_mode, // scalar fifo in
unsigned int *tu_regfile, int frameMemoryOffset, // parameters
unsigned char *recBlock, // object fifo out
unsigned char *hOut, unsigned char *vOut, unsigned char *kOut, // scalar fifo out
unsigned char *recFrame, // shared memory out
  short DBPmvx, short DBPmvy, short round_type
)
{
	const UChar MBheight = (tu_regfile[0] >> 6) & 0x3F;
	const UChar MBwidth = (tu_regfile[0] >>  0) & 0x3F;

	int width = (k < 4) ? MBwidth*16 : MBwidth*8;
	int height = (k < 4) ? MBheight*16 : MBheight*8;
	int i,j;
	short tmpS;
	unsigned char tmpUChar;
	short maxval = (1 << BITS_PIXEL) - 1;
	
	unsigned int frameMemoryAddress;
  static FILE *fid;
  static int fcount = 0;
	
#ifdef functionTimes 
	extern cycles localTextureUpdateCycles; 
	extern cycles overheadCycles; 
	cycles startProcessing, stopProcessing; 

	getClockTicks(&startProcessing); 
#endif 

  if( framenum == 190 )
  {
    i = 0;
  }

	if (k < 4)
		frameMemoryAddress = frameMemoryOffset + ((MBwidth*v + h)*4 + k)*64;
	else if (k == 4)
		frameMemoryAddress = frameMemoryOffset + width*height*4 + (MBwidth*v + h)*64;
	else
		frameMemoryAddress = frameMemoryOffset + width*height*5 + (MBwidth*v + h)*64;



  #define NAMEmv "mv.dat"
  #define NCOUNTmv 70000

  // if( fcount < NCOUNTmv )
  { short t;
    static int lastframe=-1;

    if( fid == 0 )
    { fid = fopen( NAMEmv, "wb" );
    }
    ++fcount;
    if( framenum != lastframe )
    { t = -1      ; fwrite( &t, 2,  1, fid );
      t = MBwidth ; fwrite( &t, 2,  1, fid );
      t = MBheight; fwrite( &t, 2,  1, fid );
      t = round_type; fwrite( &t, 2,  1, fid );
      t = 0       ; fwrite( &t, 2,  1, fid );
      t = 0       ; fwrite( &t, 2,  1, fid );
      lastframe = framenum;
    }
    t = tu_mode ; fwrite( &t, 2,  1, fid );
    t = h       ; fwrite( &t, 2,  1, fid );
    t = v       ; fwrite( &t, 2,  1, fid );
    t = k       ; fwrite( &t, 2,  1, fid );
    t = DBPmvx  ; fwrite( &t, 2,  1, fid );
    t = DBPmvy  ; fwrite( &t, 2,  1, fid );
  }
//  else
//  {
//    fclose(fid);
//    fid = 0;
//  }




	switch (tu_mode)
	{
		case 0: // inter skipped & MVs 0, send compensated block to display
			for (j = 0; j < 8; j++)
				for (i = 0; i < 8; i++)
				{
					recBlock[j*8 + i] = compBlock[j*8 + i]; // decoder specific
				}

			break;
		
		case 1: // inter skipped & !MVs 0 -> use only compensated block
			for (j = 0; j < 8; j++)
				for (i = 0; i < 8; i++)
				{
					tmpUChar = compBlock[j*8 + i];
					recFrame[frameMemoryAddress++] = tmpUChar;
					recBlock[j*8 + i] = tmpUChar; 
				}

			break;
#if 0
		case 2: // inter 1st row only; Dir == 2; vertical structures
			for (j = 0; j < 8; j++)
				for (i = 0; i< 8; i++)
				{
					tmpS = textBlock[i] + compBlock[j*8 + i];
					tmpUChar = tmpS < 0 ? 0 : tmpS > maxval ? maxval : tmpS;
					recFrame[frameMemoryAddress++] = tmpUChar;
					recBlock[j*8 + i] = tmpUChar; 
				}
			break;

		case 3: // inter 1st column only; Dir == 1; horizontal structure
			for(j = 0; j < 8; j++)
				for(i = 0; i < 8;i++)
				{
					tmpS = textBlock[j*8] + compBlock[j*8 + i];
					tmpUChar = tmpS < 0 ? 0 : tmpS > maxval ? maxval : tmpS;
					recFrame[frameMemoryAddress++] = tmpUChar;
					recBlock[j*8 + i] = tmpUChar; 
				}
			break;
#endif
		case 4: // inter all coeffs
			for(j = 0; j < 8; j++)
				for(i = 0; i < 8;i++)
				{
					tmpS = textBlock[j*8 + i] + compBlock[j*8 + i];
					tmpUChar = tmpS < 0 ? 0 : tmpS > maxval ? maxval : tmpS;
					recFrame[frameMemoryAddress++] = tmpUChar;
					recBlock[j*8 + i] = tmpUChar; 
				}
			break;

#if 0
		case 5: // intra DC coeffs only -> all values identical (IDCTed DC value passed on position 0)
			tmpUChar = textBlock[0] < 0 ? 0 : textBlock[0] > maxval ? maxval : textBlock[0];
			for(j = 0; j < 8; j++)
				for(i = 0; i < 8;i++)
					recFrame[frameMemoryAddress++] = tmpUChar;
					recBlock[j*8 + i] = tmpUChar;
			break;
#endif
		
		case 6: // intra AC & DC coeffs
			for(j = 0; j < 8; j++)
				for(i = 0; i < 8;i++)
				{
					tmpS = textBlock[j*8 + i];
					tmpUChar = tmpS < 0 ? 0 : tmpS > maxval ? maxval : tmpS;
					recFrame[frameMemoryAddress++] = tmpUChar;
					recBlock[j*8 + i] = tmpUChar;				
				}
			break;

		default:
			fprintf(stderr,"wrong mode in textureUpdate process\n");
			break;
	}

	*hOut = h;
	*vOut = v;
	*kOut = k;

	return;


#ifdef functionTimes 
	getClockTicks(&stopProcessing); 
	localTextureUpdateCycles += (stopProcessing - startProcessing - overheadCycles);  
#endif	

}

#endif
