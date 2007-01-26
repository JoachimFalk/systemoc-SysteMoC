/* 
 *  Author: Kristof Denolf
 *  Based on MPEG-4 reference code  
 *  Date: October 17th 2003
 *  IMEC 
 */ 
 
#ifndef __updateBuffer_c 
#define __updateBuffer_c 
 
#ifndef __AllInOne_h 
#include "AllInOne.h" 
#endif 

#if !defined( noStimuliGeneration )
#include "stimuliGeneration.h"
#endif

void updateBufferY(unsigned char *recFrame, unsigned char *bufferY, int h, int v, unsigned char MBwidth, int frameMemoryOffset)
{
	int width = MBwidth*16;

	int bufferYposition = ((v*MBwidth+h)%LBwidth)*256;
	unsigned int frameMemoryAddress = frameMemoryOffset + (MBwidth*v + h)*4*64;
	int localPosition;

	int i,j,k;
  
	for (k = 0; k < 4; k++)
	{
		localPosition = bufferYposition + (k&2)*64 + (k&1)*8;
		for (i = 0; i < 8; i++)
			for (j = 0; j < 8; j++){
				bufferY[localPosition + i*16 + j] = recFrame[frameMemoryAddress++];
#if !defined( noStimuliGeneration )
				StimuliGenerationMC_WriteSharedMemData(StimuliFile_MC_Buffer_Y,1,localPosition + i*16 + j,recFrame[frameMemoryAddress-1])
#endif
			}
	}
}

void updateBufferChrom(unsigned char *recFrame, unsigned char *bufferChromU, unsigned char *bufferChromV, int h, int v, unsigned char MBwidth, unsigned char MBheight, int frameMemoryOffset) 
{
	int chromWidth = MBwidth*8;
	int width = MBwidth*16;
	int height = MBheight*16;

	int bufferChromPosition = ((v*MBwidth+h)%LBwidth)*64;
	int localPosition = bufferChromPosition;

	unsigned int frameMemoryAddress = frameMemoryOffset + width*height + (v*MBwidth + h)*64;

	int i,j;
  
	for(i=0;i<8;i++) 
		for(j=0;j<8;j++){
			bufferChromU[localPosition++] = recFrame[frameMemoryAddress++]; 
#if !defined( noStimuliGeneration )
			StimuliGenerationMC_WriteSharedMemData(StimuliFile_MC_Buffer_U,1,localPosition-1,recFrame[frameMemoryAddress-1])
#endif
		}

	frameMemoryAddress = frameMemoryOffset + width*height + width*height/4 + (v*MBwidth + h)*64;
	localPosition = bufferChromPosition;
	
	for(i=0;i<8;i++) 
		for(j=0;j<8;j++){
			bufferChromV[localPosition++] = recFrame[frameMemoryAddress++];
#if !defined( noStimuliGeneration )
			StimuliGenerationMC_WriteSharedMemData(StimuliFile_MC_Buffer_V,1,localPosition-1,recFrame[frameMemoryAddress-1])
#endif
		}
}

/*void updateSearchArea(unsigned char* bufferY, unsigned char* searchArea,int v, int h, int i, int j, int MBwidth, int MBheight) 
{ 
	int m,n,indv,indh,MBv,MBh; 
	unsigned char registerY; 

	int searchAreaPosition = 0;
	int bufferYposition = 0;
  
	if(v+i>=0 && v+i<MBheight && h+j>=0 && h+j<MBwidth) 
	{ 
		searchAreaPosition = (((i+1)*16)*SAwidth+((h+j+1)%SAmbWidth)*16);
		bufferYposition = (((v+i)*MBwidth+h+j)%LBwidth)*256;		 
		
		for(m=0;m<16;m++) 
			for(n=0;n<16;n++) 
				searchArea[searchAreaPosition + m*SAwidth + n] = bufferY[bufferYposition + m*16 + n];
	} 
	else if(v+i>=0 && v+i<MBheight) 
	{ 
		if(h+j<0)  
		{ 
			MBh=0; 
			indh=0; 
		} 
		else 
		{ 
			MBh=MBwidth-1; 
			indh=15; 
		} 
		 
		searchAreaPosition = ((i+1)*16)*SAwidth+((h+j+1)%SAmbWidth)*16; 
		bufferYposition = (((v+i)*MBwidth+MBh)%LBwidth)*256 + indh;
 
		for(m=0;m<16;m++) 
		{ 
			registerY = bufferY[bufferYposition+m*16]; 
			for(n=0;n<16;n++) 
				searchArea[searchAreaPosition + m*SAwidth + n] = registerY;
		} 
	} 
	else if(h+j>=0 && h+j<MBwidth) 
	{ 
		 
		if(v+i<0) 
		{ 
			MBv=0; 
			indv=0; 
		} 
		else 
		{ 
			MBv=MBheight-1; 
			indv=15; 
		} 
 
		searchAreaPosition = ((i+1)*16)*SAwidth+((h+j+1)%SAmbWidth)*16;
		bufferYposition = ((MBv*MBwidth+h+j)%LBwidth)*256 + 16*indv;
 
		for(m=0;m<16;m++) 
			for(n=0;n<16;n++) 
				searchArea[searchAreaPosition + m*SAwidth + n] = bufferY[bufferYposition + n];
	} 
	 
	else 
	{ 
		 
		if(h+j<0)  
		{ 
			MBh=0; 
			indh=0; 
		} 
		else 
		{ 
			MBh=MBwidth-1; 
			indh=15; 
		} 
		 
		if(v+i<0) 
		{ 
			MBv=0; 
			indv=0; 
		} 
		else 
		{ 
			MBv=MBheight-1; 
			indv=15; 
		} 

		registerY = bufferY[((MBv*MBwidth+MBh)%LBwidth)*256 + indv*16+indh]; 
		searchAreaPosition = (((i+1)*16)*SAwidth+((h+j+1)%SAmbWidth)*16);
  
		for(m=0;m<16;m++) 
			for(n=0;n<16;n++) 
				searchArea[searchAreaPosition + m*SAwidth + n] = registerY; 
	} 
  
	return;   
} */

void copyControl(unsigned char *recFrame,  //shared memory in
unsigned char hIn, unsigned char vIn,  // scalar fifo in
unsigned int *cc_regfile, int frameMemoryOffset, // parameters in 
unsigned char *bufferY, unsigned char *bufferU, unsigned char *bufferV, /*unsigned char *searchArea,*/ // shared memory out
unsigned char *hOut, unsigned char *vOut // scalar fifo out
) 
{
//	int i,j;
	const UChar vopType = (cc_regfile[0] >> 12) & 0x1;
	const UChar MBheight = (cc_regfile[0] >> 6) & 0x3F;
	const UChar MBwidth = (cc_regfile[0] >>  0) & 0x3F;

#ifdef functionTimes 
	extern cycles localUpdateBufferYUVCycles,localUpdateSearchAreaCycles; 
	extern cycles overheadCycles; 
	cycles startProcessing, stopProcessing; 

	getClockTicks(&startProcessing); 		
#endif	
	
	if (vopType)
	{
	
		//update bufferYUV
		if(vIn==0 && hIn==0)
		{
			updateBufferY(recFrame,bufferY,hIn,vIn,MBwidth,frameMemoryOffset);
			updateBufferChrom(recFrame,bufferU,bufferV,hIn,vIn,MBwidth,MBheight,frameMemoryOffset);
		}
		
		if(vIn==0 && hIn+1<MBwidth)
		{
			updateBufferY(recFrame,bufferY,hIn+1,vIn,MBwidth,frameMemoryOffset);
			updateBufferChrom(recFrame,bufferU,bufferV,hIn+1,vIn,MBwidth,MBheight,frameMemoryOffset);
		}
				
		if(hIn==0 && vIn+1<MBheight)
		{
			updateBufferY(recFrame,bufferY,hIn,vIn+1,MBwidth,frameMemoryOffset);
			updateBufferChrom(recFrame,bufferU,bufferV,hIn,vIn+1,MBwidth,MBheight,frameMemoryOffset);
		}
				
		if(hIn+1<MBwidth && vIn+1<MBheight)
		{
			updateBufferY(recFrame,bufferY,hIn+1,vIn+1,MBwidth,frameMemoryOffset);
			updateBufferChrom(recFrame,bufferU,bufferV,hIn+1,vIn+1,MBwidth,MBheight,frameMemoryOffset);
		}

		
#ifdef functionTimes 
		getClockTicks(&stopProcessing); 
		localUpdateBufferYUVCycles += (stopProcessing - startProcessing - overheadCycles);  

		getClockTicks(&startProcessing); 		
#endif

		//update SearchArea
		
		/*if(hIn==0)
		{
			for(i=-1;i<2;i++)
				for(j=-1;j<2;j++)
					updateSearchArea(bufferY,searchArea,vIn,hIn,i,j,MBwidth,MBheight);
		}
		else 
			for(i=-1;i<2;i++)
				updateSearchArea(bufferY,searchArea,vIn,hIn,i,1,MBwidth,MBheight);*/
	}

#if !defined( noStimuliGeneration )
	StimuliGenerationMC_WriteSharedMemData(StimuliFile_MC_Buffer_Y,2,0,0)
	StimuliGenerationMC_WriteSharedMemData(StimuliFile_MC_Buffer_U,2,0,0)
	StimuliGenerationMC_WriteSharedMemData(StimuliFile_MC_Buffer_V,2,0,0)
#endif
	
	*hOut = hIn;
	*vOut = vIn;

#ifdef functionTimes 
	getClockTicks(&stopProcessing); 
	localUpdateSearchAreaCycles += (stopProcessing - startProcessing - overheadCycles);  
#endif

	return;
}	
 
#endif 
