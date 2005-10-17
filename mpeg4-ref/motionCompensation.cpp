/* 
 *  Author: Kristof Denolf (IMEC)
 *  Based on MPEG-4 reference code  
 *  Date: March 24th 2004  
 *  Update: 
 *		April 8th 2004, Kristof Denolf (IMEC): integrate translateAddress function to be
 *										closer to a HW description with a simple control.
 *		April 21st 2004, Kristof Denolf: copied motion compensation from encoder and stripped out unneeded functionality
 *										 using comments
 */ 

#ifndef __motionCompensation_c
#define __motionCompensation_c

#ifndef __AllInOne_h
#include "AllInOne.h"
#endif

#include "stimuliGeneration.h"

unsigned int translateAddress(unsigned char k, short i, short j, unsigned char MBwidth, unsigned char MBheight);
void MotionCompensate(unsigned char *buffer,/*unsigned char *currMB,*/unsigned char h, unsigned char v, unsigned char k, short mot_x, short mot_y, /*unsigned char mode,*/unsigned char MBwidth, unsigned char MBheight, int rounding_control, unsigned char *comp/*, short *error_comp, int *ptr_SADblock*/);

//*********************************************************************************************************************
//  Called by FrameProcesing.c
//*********************************************************************************************************************

// difference with encoder: k (blockNumber) is passed to the MC

void MotionCompensateBlock(unsigned char *bufferY, unsigned char *bufferU, unsigned char *bufferV, //shared memory in
/*unsigned char *currentMB,*/ // object fifo in
unsigned char kIn, unsigned char hIn, unsigned char vIn, short mv_xIn, short mv_yIn,/* unsigned char modeIn,*/ // scalar fifo in
unsigned int *mc_regfile, // parameters
unsigned char *compBlock/*, short *errorBlock,*/ //object fifo out
/*unsigned char *kOut, unsigned char *modeOut, short *mv_xOut, short *mv_yOut, int *SADblock,unsigned char *hOut1, unsigned char *vOut1, unsigned char *hOut2, unsigned char *vOut2*/ // scalar fifo out
)
{
const UChar roundingType = (mc_regfile[0] >> 13) & 0x1;
	const UChar MBheight = (mc_regfile[0] >> 6) & 0x3F;
	const UChar MBwidth = (mc_regfile[0] >>  0) & 0x3F;

	/* static unsigned char k = 0;*/ 
	short local_mvx, local_mvy;
	int i;

if( framenum == 189  )
{
  i = 0;
}

	if (kIn < 4)
		MotionCompensate(bufferY,/*currentMB,*/hIn,vIn,kIn,mv_xIn,mv_yIn,/*modeIn,*/MBwidth,MBheight,roundingType,compBlock/*,errorBlock,SADblock*/);
	else 
	{	
		local_mvx = mv_xIn/*%4 == 0 ? mv_xIn >> 1 : mv_xIn >> 1 | 1*/; 
		local_mvy = mv_yIn/*%4 == 0 ? mv_yIn >> 1 : mv_yIn >> 1 | 1*/;

		if (kIn == 4)
			MotionCompensate(bufferU,/*currentMB,*/hIn,vIn,kIn,local_mvx,local_mvy,/*modeIn,*/MBwidth,MBheight,roundingType,compBlock/*,errorBlock,SADblock*/);
		else
			MotionCompensate(bufferV,/*currentMB,*/hIn,vIn,kIn,local_mvx,local_mvy,/*modeIn,*/MBwidth,MBheight,roundingType,compBlock/*,errorBlock,SADblock*/);
	}
	
/*	*kOut = k;
	*modeOut = modeIn;
	*mv_xOut = mv_xIn;
	*mv_yOut = mv_yIn;
	*hOut1 = hIn;
	*vOut1 = vIn;*/

	/* if (++k > 5)
	{
		k = 0;
		
		*hOut2 = hIn;
		*vOut2 = vIn; 
	} */

	// For SW instrumenting only!
#ifdef VERBOSE
	mv_xIn = (mv_xIn < 0) ? -mv_xIn : mv_xIn;
	mv_yIn = (mv_yIn < 0) ? -mv_yIn : mv_yIn;
	mvx_max = (mv_xIn > mvx_max) ? mv_xIn : mvx_max;
	mvy_max = (mv_yIn > mvy_max) ? mv_yIn : mvy_max;
	motion_blocks++;
#endif
	return;
}


//**************************************************************************************************************

unsigned int translateAddress(unsigned char k, short i, short j, unsigned char MBwidth, unsigned char MBheight)
{
	unsigned short width;
	unsigned short height;

	unsigned int address = 0;
	
	if (k < 4) // Y
	{
		width = MBwidth*16;
		height = MBheight*16;
		
		// wrap
		i = i < 0 ? 0 : i >= width ? (width - 1) : i;
		j = j < 0 ? 0 : j >= height ? (height - 1) : j;

		// translate into bufferYUV
		address = (((j/16)*MBwidth + (i/16))%LBwidth)*256 + (j&15)*16 + (i&15);
	}
	else // U or V
	{
		width = MBwidth*8;
		height = MBheight*8;
		
		// wrap
		i = i < 0 ? 0 : i >= width ? (width - 1) : i;
		j = j < 0 ? 0 : j >= height ? (height - 1) : j;

		// translate into bufferYUV
		
		address = (((j/8)*MBwidth + (i/8))%LBwidth)*64 + (j&7)*8 + (i&7);

	}

	return address;
}


void MotionCompensate(unsigned char *buffer,/*unsigned char *currMB,*/unsigned char h, unsigned char v, unsigned char k, short mot_x, short mot_y, /*unsigned char mode,*/unsigned char MBwidth, unsigned char MBheight,
 int rounding_control, unsigned char *comp/*, short *error_comp, int *ptr_SADblock*/)
{
	short SADblock=0;
	
	unsigned char i,j, n;
		
	short xl;
	short yu;

	short buffer_2[2];
	unsigned char buffer_16[16];
	unsigned char tmpUchar;
	//short tmpShort;

	unsigned short currMBposition = k*64;
	unsigned short blockPosition = 0;

#ifdef functionTimes 
	extern cycles localMotionCompensationCycles; 
	extern cycles overheadCycles; 
	cycles startProcessing, stopProcessing; 

	getClockTicks(&startProcessing); 
#endif 
	
 
//	if (StimuliMC_skip_condition) printf("h=%d;v=%d;k=%d;mv_x=%d;mv_y=%d\n",h,v,k,mot_x,mot_y);

	/*if (mode == 0) // intra
	{
		for (i = 0; i < 64; i++)
			error_comp[i] = currMB[currMBposition + i];
	}
	else // inter */
	{
		if (k < 4) // Y
		{
			xl = h*16 + (mot_x >> 1) + (k & 1)*8;
			yu = v*16 + (mot_y >> 1) + (k >> 1)*8;
		}
		else // UV
		{
			//mot_x = mot_x%4 ==0 ? mot_x >> 1 : mot_x >> 1 | 1; // done in motion decoder 
			//mot_y = mot_y%4 ==0 ? mot_y >> 1 : mot_y >> 1 | 1; // done in motion decoder
			xl = h*8 + (mot_x >> 1);
			yu = v*8 + (mot_y >> 1);
		}
		
  if (!(mot_x & 1) && !(mot_y & 1)) // no interpolation needed
  { 
  	for(j = 0; j < 8; j++)
		{
			for(i = 0; i < 8; i++)
	 		{
				tmpUchar = buffer[translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight)];
				comp[blockPosition] = tmpUchar;
				//tmpShort = ((short)(currMB[currMBposition++])-(short)tmpUchar);
				//error_comp[blockPosition++] = tmpShort;
				//SADblock+=abs(tmpShort);
				blockPosition++;
//				if (StimuliMC_skip_condition) printf("addr = %d; data = %d; interp = %d\n",translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight), buffer[translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight)],(short)tmpUchar);
			}
		}
  }
  else if ((mot_x & 1) && !(mot_y & 1)) // interpolate horizontally
	{  
  	n = 0;
  	
		for(j = 0; j < 8; j++)
		{
			buffer_2[1] = buffer[translateAddress(k,xl,(short)(yu+j),MBwidth,MBheight)];
//			if (StimuliMC_skip_condition) printf("addr = %d; data = %d;crt = -; interp = -; diff = -\n", translateAddress(k,xl,(short)(yu+j),MBwidth,MBheight), buffer[translateAddress(k,xl,(short)(yu+j),MBwidth,MBheight)]);

	 		for(i = 1; i < 9; i++)
		 	{
				buffer_2[(n++)&1] = buffer[translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight)];
				tmpUchar = (buffer_2[0] + buffer_2[1] + 1 - rounding_control) >> 1;
				comp[blockPosition] = tmpUchar;
				//tmpShort = ((short)(currMB[currMBposition++])-(short)tmpUchar);
				//error_comp[blockPosition++] = tmpShort;
				//SADblock+=abs(tmpShort);
				blockPosition++;
//				if (StimuliMC_skip_condition) printf("addr = %d; data = %d; interp = %d\n",translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight), buffer[translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight)], (short)tmpUchar);
			} 
		} 
	}
	else if (!(mot_x & 1) && (mot_y & 1)) // interpolate vertically
 	{	
		for (n = 0; n < 8; n++){
			buffer_16[n] = buffer[translateAddress(k,(short)(xl+n),yu,MBwidth,MBheight)];
//			if (StimuliMC_skip_condition) printf("addr = %d; data = %d;crt = -; interp = -; diff = -\n", translateAddress(k,(short)(xl+n),yu,MBwidth,MBheight), buffer[translateAddress(k,(short)(xl+n),yu,MBwidth,MBheight)]);
		}

  	//n = 0;
  	for(j = 1; j < 9; j++)
		{
			
			for(i = 0; i < 8;i++)
		 	{
				buffer_16[n&15] = buffer[translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight)];
				tmpUchar = (buffer_16[(n-8)&15] + buffer_16[n&15] + 1 - rounding_control) >> 1;
				comp[blockPosition] = tmpUchar;
				//tmpShort = ((short)(currMB[currMBposition++])-(short)tmpUchar);
				//error_comp[blockPosition++] = tmpShort;
				//SADblock+=abs(tmpShort);
				blockPosition++;
//				if (StimuliMC_skip_condition) printf("addr = %d; data = %d;interp = %d\n",translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight), buffer[translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight)], (short)tmpUchar);
				n++;
			}
		}     
  }
  else // 2D interpolation required
  {			
	    for (n = 0; n < 9; n++){
			buffer_16[n] = buffer[translateAddress(k,(short)(xl+n),yu,MBwidth,MBheight)];
//			if (StimuliMC_skip_condition) printf("addr = %d; data = %d;crt = -; interp = -; diff = -\n", translateAddress(k,(short)(xl+n),yu,MBwidth,MBheight), buffer[translateAddress(k,(short)(xl+n),yu,MBwidth,MBheight)]);
		}

		//n = 0;			
		for(j = 1; j < 9; j++)
		{
			buffer_16[n&15] = buffer[translateAddress(k,xl,(short)(yu+j),MBwidth,MBheight)];
//			if (StimuliMC_skip_condition) printf("addr = %d; data = %d;crt = -; interp = -; diff = -\n",translateAddress(k,xl,(short)(yu+j),MBwidth,MBheight), buffer[translateAddress(k,xl,(short)(yu+j),MBwidth,MBheight)]);
			buffer_2[n&1] = buffer_16[(n - 9)&15] + buffer_16[n&15];
			n++;

			for (i = 1; i < 9; i++)
			{
				buffer_16[n&15] = buffer[translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight)];;
				buffer_2[n&1] = buffer_16[(n - 9)&15] + buffer_16[n&15]; 
				tmpUchar = (buffer_2[0] + buffer_2[1] + 2 - rounding_control) >> 2;
				comp[blockPosition] = tmpUchar;
				//tmpShort = ((short)(currMB[currMBposition++])-(short)tmpUchar);
				//error_comp[blockPosition++] = tmpShort;
				//SADblock+=abs(tmpShort);
				blockPosition++;
//				if (StimuliMC_skip_condition) printf("addr = %d; data = %d; interp = %d; \n",translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight), buffer[translateAddress(k,(short)(xl+i),(short)(yu+j),MBwidth,MBheight)], (short)tmpUchar);
				n++;
			}
		}
  }

		//*ptr_SADblock=(int)SADblock;
	}

#ifdef functionTimes 
	getClockTicks(&stopProcessing); 
	localMotionCompensationCycles += (stopProcessing - startProcessing - overheadCycles);  
#endif	

	return;
	
}

#endif /* __motionCompensation_c */
