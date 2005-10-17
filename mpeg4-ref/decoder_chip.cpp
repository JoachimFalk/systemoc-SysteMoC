// Filename: decoder_chip.c
//
// Author: Paul Schumacher, Kristof Denolf
//
// Description: 
//   Top-level decoder chip
// 

#ifndef __AllInOne_h
#include "AllInOne.h"
#endif

#ifdef USE_DISPLAY
#include "picture.h"
#endif

#include "stimuliGeneration.h"

/***********************************************************CommentBegin******
 *
 * -- DecoderChip -- Top-level for MPEG-4 decoder hardware design
 *
 * Author :		
 *	Paul Schumacher (Xilinx)
 *
 * Created :		
 *	2004/01/09
 *
 * Update
 *	April 9th 2004, Kristof Denolf (IMEC): move bufferYUV and harmonize with encoder (copyControl block added)
 *  April 22nd, 2004, Kristof Denolf (IMEC): displaying to multiple windows for multiple streams
 *  May 6th, 2004, Kristof Denolf (IMEC): add display controller
 *
 * Purpose :		
 *	Top-level function containing the entire MPEG-4 decoder hardware design
 * 
 ***********************************************************CommentEnd********/
// Values						Bits
// ------						----
// Inputs
//		input stream	 8
// Parameters
//		bits/pixel		 4
//		disp. width		10
//		disp. height	10
//		no. streams		 4
//		frame rate		 6 (not used in SW)
// Outputs
//		output stream	 8
void DecoderChip(Vol *VOlist[], int dwidth, int dheight, unsigned short streams, int *ulx_display, int *uly_display)
{ 
	unsigned char stop=0, stnum=0;
	int fwidth[MAX_STREAMS], fheight[MAX_STREAMS];
	unsigned int frameMemoryOffset[MAX_STREAMS];

	// Enable flags
	unsigned char write_frame=0, perform_cc = 0, perform_mc=0, perform_idct=0;

	// MC: input data
	short mvx, mvy;
	unsigned char blnum;

	// MC: output data
	UChar comp_block[B_WIDTH*B_HEIGHT];

	// IDCT: input data
	SInt q_block[B_WIDTH*B_HEIGHT];
	UChar btype, ACpred_flag;
	unsigned short DCpos;
	int CBP;
	UChar bp_prev[3];
	
	// IDCT: output data
	SInt texture_block[B_WIDTH*B_HEIGHT];

	// texutureUpdate : output data
	unsigned char recBlock[B_WIDTH*B_HEIGHT];
	
	// Adder: input data
	unsigned char MCcoded, IDCTcoded;
	
	// Allocate buffer Y/U/V
	static unsigned char bufferY[4*64*LBwidth];
	static unsigned char bufferU[64*LBwidth];
	static unsigned char bufferV[64*LBwidth];

	// Allocate frame memory
	static unsigned char recFrame[MAX_WIDTH*MAX_HEIGHT + MAX_WIDTH*MAX_HEIGHT/4 + MAX_WIDTH*MAX_HEIGHT/4];
	
#ifdef USE_DISPLAY
	CPicture *picSet[MAX_STREAMS];
#endif

	unsigned char hToCC[1]; // to keep parser and CC in sync
	unsigned char vToCC[1];

	unsigned char hToMC[1]; // to keep CC and MC in sync
	unsigned char vToMC[1];

	unsigned char hToTU[1];
	unsigned char vToTU[1];
	
	unsigned char tu_mode[1];

	unsigned char hToDC[1];
	unsigned char vToDC[1];
	unsigned char kToDC[1];
int junk;
short DBPmvx, DBPmvy;


#ifdef USE_DISPLAY
	initDisplays(VOlist,(unsigned char)streams,picSet);
#endif

	// Decide if this frame is dumped or not
	// MC
	UpdateMCSkipCondition((StimuliMC_frame_cnt >= 7) & (StimuliMC_frame_cnt <=8))
	//UpdateMCSkipCondition((StimuliMC_frame_cnt <=3))
	//UpdateSkipCondition(1) // all frames

	///////////////////////
	// Functional blocks //
	///////////////////////
	while (!stop)
	{


		//
		// Block #1: Parser/VLD
		//				
		
		stop = ParserVLD(streams, fwidth, fheight, frameMemoryOffset, &write_frame, &perform_cc, &perform_mc, &perform_idct, 
			&stnum, &mvx, &mvy, &blnum, comp_block, q_block, texture_block, &MCcoded, &IDCTcoded, hToCC,vToCC, tu_mode, hToTU, vToTU, 
			&btype, &ACpred_flag, &DCpos, &CBP, bp_prev);
if( framenum == 189 )
{
   junk = 0;
}

		//
		// Block #2: Copy Controller
		//

		if (perform_cc){
			copyControl(recFrame, hToCC[0], vToCC[0], mc_regfile, dec_regfile[stnum], bufferY, bufferU, bufferV, hToMC, vToMC); 
			
			StimuliGenerationMC_WriteData(StimuliFile_MC_CC2MC_FIFO,((*vToMC & 0x3F) << 6) | (*hToMC & 0x3F));

		}

		//
		// Block #3: Motion Compensation
		//

		if (perform_mc){

			StimuliGenerationMC_WriteData(StimuliFile_MC_VLD2MC_FIFO,((mvx        & 0x3F) << 10)   | 
																	 ((mvy        & 0x3F) << 4)    | 
																	 ((blnum      & 0x07) << 1)    | 
																	   perform_mc & 0x01
										  ); 

			MotionCompensateBlock(bufferY,bufferU,bufferV,blnum,hToMC[0],vToMC[0],mvx,mvy,mc_regfile,comp_block);
	DBPmvx = mvx;
  DBPmvy = mvy;

			StimuliGenerationMC_WriteObjectFifo(StimuliFile_MC_Comp, comp_block, 64, perform_mc);
		}

		if (perform_idct || (perform_mc && !IDCTcoded))
		{
			if (!MCcoded)
			{
				// If the block not coded dump zeros
				StimuliGenerationMC_WriteData(StimuliFile_MC_VLD2MC_FIFO,((0        & 0x3F) << 10)   | 
																	 ((0        & 0x3F) << 4)    | 
																	 ((blnum      & 0x07) << 1)    | 
																	   0 & 0x01
																	);
				StimuliGenerationMC_WriteObjectFifo(StimuliFile_MC_Comp, comp_block, 64, 0);
			}
		}


		//
		// Block #4: Texture/IDCT
		//
		
		if (perform_idct)
			TextureIDCT(q_block, blnum, hToTU[0], vToTU[0], btype, ACpred_flag, DCpos, CBP, bp_prev, texture_block);

		//
		// Block #5: Texture Update (previously called adder)
		//

		if (perform_idct || (perform_mc && !IDCTcoded))
		{
			textureUpdate(comp_block, texture_block, hToTU[0], vToTU[0], blnum, tu_mode[0], mc_regfile, dec_regfile[stnum],
      recBlock, hToDC, vToDC, kToDC, recFrame,
         /* added by DBP */ mvx, mvy, (mc_regfile[0] >> 13) & 0x1 );
if( (tu_mode[0] != 0 && tu_mode[0] != 6 ) &&
  ( mvx != DBPmvx || mvy != DBPmvy || hToTU[0] != hToMC[0] || vToTU[0] != vToMC[0] ) )
{
  junk = 0;
}

#ifdef USE_DISPLAY
			displayControl(recBlock,hToDC[0],vToDC[0],kToDC[0],mc_regfile,frameMemoryOffset[stnum],picSet[stnum]);
#endif
		}

		// Write output YUV
		if (write_frame)
		{
#ifdef WRITE_FILE
			WriteVopRaw(VOlist[stnum]->y_file,VOlist[stnum]->u_file,VOlist[stnum]->v_file,recFrame, fwidth[stnum], fheight[stnum], frameMemoryOffset[stnum]);
#endif
			// Decide if this frame is dumped or not
			// MC
			UpdateMCSkipCondition((StimuliMC_frame_cnt >= 7) & (StimuliMC_frame_cnt <=8))
			//UpdateMCSkipCondition((StimuliMC_frame_cnt <=3))
			//UpdateSkipCondition(1) // all frames

		}
	}

#ifdef USE_DISPLAY
	stopDisplays((unsigned char)streams,picSet);
#endif

	return;
} // DecoderChip
