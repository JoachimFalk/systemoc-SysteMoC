// Filename: parser.c
//
// Author: Paul Schumacher
//
// Description: 
//   All header parsing functions
//			* VOL and VOP headers
//			* Macroblock (MB) headers
// Update
//	- April 22nd Kristof Denolf (IMEC): support correct usage of texture update and motion compensation of the encoder (harmonization)

#ifndef __AllInOne_h
#include "AllInOne.h"
#endif

//
// FSM states
//
enum FSM_STATES
{
	stParseVOL = 0,
	stParseVOP,
	stParseMB,
	stMotionDecode,
	stMotionCompensate,
	stTextureVLD,
	stFinishBlock,
	stFinishMB,
	stFinishVOP,
	stResyncStream,
	stDone,
	NumberFsmStates // must be last
};

// For user viewing/debugging only
char *FsmStates[NumberFsmStates] = 
{"stParseVOL",
	"stParseVOP",
	"stParseMB",
	"stMotionDecode",
	"stMotionCompensate",
	"stTextureVLD",
	"stFinishBlock",
	"stFinishMB",
	"stFinishVOP",
	"stResyncStream",
	"stDone"};
char *FrameType[2] = {"I Frame", "P Frame"};
int FsmCycles[NumberFsmStates] = {24, 10, 6, 20, 0, 70, 1, 1, 1, 1, 0};

static UChar ParseVolHeader(Bitstream *stream, int *width, int *height, unsigned int* frameMemoryOffset, UChar stnum, int *time_inc_res);
static UChar ParseVopHeader(Bitstream *stream, int time_inc_res, UChar *decode_type, UChar *round_type, 
														UChar *vop_quant, UChar *fcode, UChar *error_flag);
static UChar ParseMBheader(Bitstream *stream, int mbnum, UChar *ftype, UChar *btype,
													 UChar *skipped_flag, UChar *coded, int *CBP, UChar *ACpred_flag, 
													 UChar *MBtype);
static int mylog(int x);

/***********************************************************CommentBegin******
 *
 * -- ParserVLD --  Top level parser/VLD function
 *
 * Author :		
 *	Luis Ducla-Soares (IST) - lds@lx.it.pt
 *
 * Created :		
 *	18-Sep-97
 * 
 * Purpose :		
 *      Finite state machine for parsing bitstream and performing
 *			necessary variable length decoding (VLD) functions.
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 * Combined into a single top-level function by P. Schumacher, Xilinx, 2004/1/8
 * Update for move of bufferYUV by Kristof Denolf (IMEC), April 9th 2004
 *
 ***********************************************************CommentEnd********/
 
UChar ParserVLD(short streams, int *width, int *height, unsigned int *frameMemoryOffset, 
		unsigned char *write_frame, unsigned char *perform_cc, unsigned char *perform_mc, unsigned char *perform_idct,
		unsigned char *stnum_out, short *mvx_out, short *mvy_out, unsigned char *blnum_out, 
		unsigned char *comp_block, short *q_block, short *texture_block,
		unsigned char *MCcoded_out, unsigned char *IDCTcoded_out,
		unsigned char *hOut1, unsigned char *vOut1,unsigned char *tu_mode, unsigned char *hOut2, unsigned char *vOut2,
		unsigned char *btype_out, unsigned char *ACpred_flag_out, unsigned short *DCpos_out, int *CBP_out, unsigned char *bp_prev)
{
  int i, nextState, resync_bits;
  UChar decode_type, stop=0, error_flag=0;
	
	// Registered values
	static Bitstream *stream[MAX_STREAMS];
	static UChar init_stream=1;
	static int currState = stParseVOL;
	static int returnState = stParseVOP;
	static int mbnum=0;
	static int MB_in_width, MB_in_height, MB_in_VOP;
	static UChar fcode, vop_quant, LB;
	static int resync_marker_length = 17;
	static UChar bufferPattern[LB_MAX];
	static UChar prevRow, prevCol, prevRowCol;
	static short mvx[BLOCK_CNT], mvy[BLOCK_CNT];
	static short slice_counter = 0;	
	static short slice_nb[LB_MAX];
	static UChar ACpred_flag = 0;
	static UChar coded, skipped_flag;
	static UChar ftype, MBtype, btype, ACcoded;
	static int CBP, x_pos, y_pos;
	static UChar stnum=0, blnum=0;
	static unsigned short DCpos=0;
	static short mot_x[2*LB_MAX], mot_y[2*LB_MAX];
	static UChar MCcoded=0, IDCTcoded=0;
	static UChar round_type;
	static int time_inc_res;

	// Verify printf
	//printf("    %20s  (stnum=%d, ftype = %d, mbnum=%d, blnum=%d)\n", FsmStates[currState], stnum, ftype, mbnum, blnum);

	// Initialize streams
	if (init_stream) 
	{
		//printf("init streams...\n");
		// Allocate memory
		for (i=0; i < streams; i++) {
			stream[i] = (Bitstream *) malloc(sizeof(Bitstream));
			stop |= BitstreamInit(stream[i], i);
		}
		if (stop)
			FatalError("cannot initialize bitstream(s)");
		init_stream = 0;
	}

	// Default values
	*write_frame = 0;
	*perform_cc = 0;
	*perform_mc = 0;
	*perform_idct = 0;

	// Estimate hardware clock cycles for parser/VLD block
#ifdef VERBOSE
	parser_cycles += FsmCycles[currState];
#endif

	////////////////////////////////
	// Finite State Machine (FSM) //
	////////////////////////////////
	switch (currState)
	{
	//////////////////////
	// Parse VOL Header //
	//////////////////////
	case stParseVOL:
		stop = ParseVolHeader(stream[stnum], width, height, frameMemoryOffset, stnum, &time_inc_res);

		// Register files (per-stream values)
		dec_regfile[stnum] = frameMemoryOffset[stnum];

		if (stnum < streams-1) 
		{
			stnum++;
			nextState = stParseVOL;
		}
		else
		{
			//stnum = 0;
			nextState = stParseVOP;
		}
		break;

	//////////////////////
	// Parse VOP Header //
	//////////////////////
	case stParseVOP:

		if (stnum < streams-1) 
			stnum++;
		else
			stnum = 0;

		stop = ParseVopHeader(stream[stnum], time_inc_res, &decode_type, &round_type, &vop_quant, &fcode, &error_flag); 
		ftype = decode_type & 0x1;
		
		if (!stop && !error_flag)
		{
			// Printing to stdout
			#ifdef VERBOSE
				if (stnum == 0) printf("FRAME %d:\n",framenum);
				printf("  VOP decoding of stream %d: %d x %d (%s)\n", 
					stnum, width[stnum], height[stnum], FrameType[ftype]);
				if (ftype == I_VOP)
					iframes++;
				else
					pframes++;
			#else
				if (stnum == 0 && (framenum % 10) == 0)
					printf("FRAME %d:\n",framenum);
			#endif
			
			// For SW instrumenting only
				if (stnum == 0) {
					framenum++;	
					// EPFL_SIT new agent
				}
		} // if !stop && !error_flag

		// Per-frame constants
		MB_in_width = width[stnum] / MB_WIDTH;
		MB_in_height = height[stnum] / MB_HEIGHT;
		MB_in_VOP = MB_in_width * MB_in_height;
		LB = MB_in_width + 2;
		resync_marker_length = (ftype == I_VOP) ? 17 : 16 + fcode;
		
		// Register files (per-VOP values)
		mc_regfile[0] = (unsigned int) (round_type << 13) + (ftype << 12) + (MB_in_height << 6) + MB_in_width;
		
		idct_regfile[0] = (unsigned int) (ftype << 11) + (vop_quant << 6) + MB_in_width /* DBP */ + (MB_in_height << 16);

		// Reset values
		for (i = 0; i < LB; i++)
				bufferPattern[i] = 0;
		slice_counter = 0;
		//pattern = 0;
		ACpred_flag = 0;
		DCpos = 0;
		mbnum = 0;
		blnum = 0;

		if (!stop && !error_flag) 
			nextState = stParseMB;
		else
			if (error_flag)
			{
				printf("wrong VOP start code found in stream %d (corrupted bitstream?)\n", stnum);
				returnState = stParseVOP;
				nextState = stResyncStream;
			}
			else
				nextState = stDone;
		break;

	/////////////////////
	// Parse MB Header //
	/////////////////////
	case stParseMB:
		stop = ParseMBheader(stream[stnum], mbnum, &ftype, &btype, &skipped_flag, &coded, &CBP, &ACpred_flag, &MBtype);
#ifdef VERBOSE
                printf("Decoding macroblock %u of stream %u\n",mbnum,stnum);

                if (ftype == P_VOP){
                  printf("  Decoding P_VOP\n");
                }else if(ftype == I_VOP){
                  printf("  Decoding I_VOP\n");
                }else{
                  printf("  Decoding ftype: %u\n",ftype);
                }

                if (btype == INTER){
                  printf("  Decoding INTER block\n");
                }else if(btype == INTRA){
                  printf("  Decoding INTRA block\n");
                }else{
                  printf("  Decoding block type: %u\n",btype);
                }

               printf("  MB-Type: %u\n",MBtype);

               printf("  CBP: %d\n",CBP);
               
               printf("  AC-prediction flag: %u\n",ACpred_flag);

               
               printf("  coded: %u; skipped: %u\n",coded,skipped_flag);
#endif


               
		// Per-MB constants
		x_pos = mbnum % MB_in_width;
		y_pos = mbnum / MB_in_width;
		slice_nb[DCpos] = slice_counter; // not currently used
		if (ftype == I_VOP) DCpos = mbnum % LB;
		prevRow = ((y_pos-1)*MB_in_width + x_pos) % LB;
		prevCol = (y_pos*MB_in_width + x_pos -1) % LB;
		prevRowCol = ((y_pos-1)*MB_in_width + x_pos - 1) % LB;

		// Scalar fifo (per MB values)
//		if (ftype == P_VOP)
//		{
			*hOut1 = x_pos;
			*vOut1 = y_pos;		
			*perform_cc = 1;
//		}
		*hOut2 = x_pos;
		*vOut2 = y_pos;

		// Initialize values
		blnum = 0;
		ACcoded = (UChar) (CBP & (1 << (BLOCK_CNT - 1 - blnum)));
		MCcoded = 0;
		IDCTcoded = coded && (btype == INTRA || (btype == INTER && ACcoded));
		
		if (!stop) 
			if (ftype == I_VOP)
				nextState = stTextureVLD;
			else
				nextState = stMotionDecode;
		else
			nextState = stDone;
		break;

	///////////////////////
	// Decode MC Vectors //
	///////////////////////
	case stMotionDecode:
		stop = MotionDecoder(stream[stnum], ftype, coded, skipped_flag, x_pos, y_pos, DCpos, MB_in_width, fcode, 
			mot_x, mot_y, mvx, mvy, MBtype, slice_nb, LB);

		//MCcoded = coded && (btype == INTER) && (ACcoded || mvx[blnum] != 0 || mvy[blnum] !=0);
		MCcoded = (btype == INTER);

		if (!stop)
		{
			if (MCcoded)
				nextState = stMotionCompensate;
			else if (IDCTcoded)
				nextState = stTextureVLD;
			else
				nextState = stFinishBlock;
		}
		else
			nextState = stDone;
		break;

	/////////////////////////
	// Motion compensation //
	/////////////////////////
	case stMotionCompensate:			

		*perform_mc = 1;
		
		if (!stop)
		{
			if (IDCTcoded)
				nextState = stTextureVLD;
			else
				nextState = stFinishBlock;
		}
		else
			nextState = stDone;
		break;
							
	/////////////////
	// Texture VLD //
	/////////////////
	case stTextureVLD:
		stop = TextureVLD(stream[stnum], ftype, btype, blnum, CBP, q_block, x_pos, y_pos);

		if (!stop)
		{
			*perform_idct = 1;
			nextState = stFinishBlock;
		}
		else
			nextState = stDone;
		break;
		
	//////////////////
	// Finish Block //
	//////////////////
	case stFinishBlock:	
		
		// Looking ahead to coding of next block
		ACcoded = CBP & (1 << (BLOCK_CNT - 1 - (blnum+1)));
		//MCcoded = coded && (btype == INTER) && (ACcoded || mvx[blnum+1] != 0 || mvy[blnum+1] !=0);
		MCcoded = (btype == INTER);
		IDCTcoded = coded && (btype == INTRA || (btype == INTER && ACcoded));

		if (!stop)
		{
			if (blnum < BLOCK_CNT-1)
			{
				blnum++;
				if (MCcoded)
					nextState = stMotionCompensate;
				else if (IDCTcoded)
					nextState = stTextureVLD;
				else
					nextState = stFinishBlock;
			}
			else
				nextState = stFinishMB;
		}
		else
			nextState = stDone;
		break;
						
	///////////////
	// Finish MB //
	///////////////
	case stFinishMB:
		//
		// Set buffer pattern
		// 
		if (btype == INTRA && coded)
			bufferPattern[DCpos] = 0;
		else
			bufferPattern[DCpos] = 1;

		DCpos = (mbnum+1) % LB;
		
		if (!stop)
		{
			if (mbnum < MB_in_VOP-1)
				nextState = stParseMB;
			else
				nextState = stFinishVOP;
			blnum = 0;
			mbnum++;
		}
		else
			nextState = stDone;
		break;
			
	////////////////
	// Finish VOP //
	////////////////
	case stFinishVOP:
			
		// Byte-align bitstream
		BitstreamByteAlign(stream[stnum]);

		if (!stop)
		{
			*write_frame = 1;
			mbnum = 0;
			blnum = 0;
			nextState = stParseVOP;
		}
		else
			nextState = stDone;
		break;

	///////////////////
	// Resync Stream //
	///////////////////
	case stResyncStream:
		// Find next marker
		printf("Finding next marker...\n");
		resync_bits = 0;
		while (!CheckBitStuffing(stream[stnum]) || (((BitstreamShowBitsByteAlign(stream[stnum], resync_marker_length)) != RESYNC_MARKER) &&
			((BitstreamShowBitsByteAlign(stream[stnum], VOP_START_CODE_LENGTH)) != VOP_START_CODE))) 
		{
			BitstreamFlushBits(stream[stnum], 1);
			resync_bits++;
		}
		printf("Found marker after shifting %d bits\n", resync_bits);
		resync_errors++;

		// Now that marker has been found, go back to previous state
		// NOTE: this may have to be selected based upon which marker is found!
		nextState = returnState;
		break;

	//////////
	// Done //
	//////////
	default:
		stop = 1;
		nextState = stDone;
		break;
	} // FSM switch statement

	// Register next state of FSM
	currState = nextState;
	
	//
	// Outputs
	//
	*stnum_out = stnum;
	*mvx_out = mvx[blnum];
	*mvy_out = mvy[blnum];
	*blnum_out = blnum;
	*MCcoded_out = MCcoded;
	*IDCTcoded_out = IDCTcoded;
	*btype_out = btype;
	*ACpred_flag_out = ACpred_flag;
	*DCpos_out = DCpos;
	*CBP_out = CBP;
	bp_prev[0] = bufferPattern[prevRow];
	bp_prev[1] = bufferPattern[prevCol];
	bp_prev[2] = bufferPattern[prevRowCol];
	
	if (btype == INTRA)
			*tu_mode = 6;
	else
	{
		if (!coded || (mvx[blnum] == 0 && mvy[blnum] == 0 && !ACcoded)) // Inter skipped & MVs 0 -> skipped block
			*tu_mode = 0;
		else if (!ACcoded) // Inter skipped & !MVs 0 -> use only compensated block
			*tu_mode = 1;
		else
			*tu_mode = 4;
	}

	// Free memory
	if (stop)
	{
		//printf("free streams...\n");
		for (i=0; i < streams; i++)
			BitstreamClose(stream[i]);
	}

	return stop;
} // ParserVLD()


/***********************************************************CommentBegin******
 *
 * -- ParseVolHeader -- Parse the header of a VOL
 *
 * Author :		
 *      Michael Wollborn (TUH)
 *
 * Created :		
 *	08-AUG-1996
 *
 * Purpose :
 *	Parse the header of a VOL
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

static UChar ParseVolHeader(Bitstream *stream, int *width, int *height, unsigned int* frameMemoryOffset, UChar stnum, int *time_inc_res)
{
  int tmpvar, bits;

	static unsigned int localFrameMemoryOffset = 0;
  
#ifdef VERBOSE
	stream->bstart = stream->bitcount;
#endif
  
	tmpvar = (Int) BitstreamShowBits(stream, 27);
	
  if (tmpvar == 8) /*no short video header*/
  {
    tmpvar = (Int) BitstreamReadBits(stream, 27); /*vo_start_code*/
    tmpvar = (Int) BitstreamReadBits(stream, 5); /*vo_id*/
		
    tmpvar = (Int) BitstreamReadBits(stream, VOL_START_CODE_LENGTH); /*vol_start_code*/
		
    if (tmpvar != VOL_START_CODE) /* vol_start_code*/
    {
      printf("Bitstream does not start with VOL_START_CODE\n");
			return 1;
    }
		
    tmpvar = (Int) BitstreamReadBits(stream, 4); /*vol_id*/
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*random_accessible_vol*/
    BitstreamReadBits(stream, 8); /*video_object_type_indication*/
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*is_object_layer_identifier*/
		
    if (tmpvar)
    {
      tmpvar = (Int) BitstreamReadBits(stream, 4); /*visual_object_layer_verid*/
			tmpvar = (Int) BitstreamReadBits(stream, 3); /*visual_object_layer_priority*/
    }
		
    tmpvar = (Int) BitstreamReadBits(stream, 4); /*aspect_ratio_info*/
		
    if (tmpvar == 15)
    {
      tmpvar = (Int) BitstreamReadBits(stream, 8); /*par_width*/
      tmpvar = (Int) BitstreamReadBits( stream, 8); /*par_height*/
    }
		
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*vol_control_parameters*/
		
    if (tmpvar)
    {
      tmpvar = (Int) BitstreamReadBits(stream, 2); /*chroma_format*/
      tmpvar = (Int) BitstreamReadBits( stream, 1); /*low_delay*/
			tmpvar = (Int) BitstreamReadBits( stream, 1); /*vbv_parameters*/
			
			if (tmpvar)
			{
				tmpvar = (Int) BitstreamReadBits( stream, 15); /*first_half_bit_rate*/
				BitstreamReadBits( stream, 1); /*marker_bit*/
				tmpvar = (Int) BitstreamReadBits( stream, 15); /*latter_half_bit_rate*/
				BitstreamReadBits( stream,  1); /*marker_bit*/
				tmpvar = (Int) BitstreamReadBits( stream, 15); /*first_half_vbv_buffer_size*/
				BitstreamReadBits( stream,  1); /*marker_bit*/ 
				tmpvar = (Int) BitstreamReadBits( stream,  3); /*last_half_vbv_buffer_size*/
				tmpvar = (Int) BitstreamReadBits( stream, 11); /*first_half_vbv_occupancy*/
				BitstreamReadBits( stream,  1); /*marker_bit*/
				tmpvar = (Int) BitstreamReadBits( stream, 15); /*last_half_vbv_occupancy*/
				BitstreamReadBits( stream,  1); /*marker_bit*/  
      }
    }
    
    tmpvar = (Int) BitstreamReadBits(stream, 2); /*vol_shape*/
    BitstreamReadBits(stream, 1); /*marker_bit*/
    *time_inc_res = (Int) BitstreamReadBits(stream, 16); /*time_increment_resolution*/
    BitstreamReadBits(stream, 1); /*marker_bit*/
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*fixed_vop_rate*/
		
    if (tmpvar)
    {
      bits = mylog(*time_inc_res);
			
      if (bits < 1)
        bits = 1;
      BitstreamReadBits(stream, bits); /*fixed_vop_time_increment*/
    }
		
    BitstreamReadBits(stream, 1); /*marker_bit*/
    width[stnum] = (Int) BitstreamReadBits(stream, 13); /*vol_width*/
    BitstreamReadBits(stream, 1); /*marker_bit*/
    height[stnum] = (Int) BitstreamReadBits(stream, 13); /*vol_height*/
    BitstreamReadBits(stream, 1); /*marker_bit*/

		// calculate frame memory offset for multiple stream support. 
		frameMemoryOffset[stnum] = localFrameMemoryOffset;
		localFrameMemoryOffset += width[stnum] * height[stnum] + (width[stnum]*height[stnum])/2;
		
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*interlaced*/
		
	        if (tmpvar)
		{
			fprintf(stderr,"interlaced mode not supported\n");
		}
			
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*OBMC_disable*/
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*vol_sprite_usage*/
			
		if (tmpvar)
		{
			fprintf(stderr,"No sprites supported !\n");
			return 1;
		}
			
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*not_8_bit*/	
    // only 8 bits/pixel supported
		if (tmpvar == 1) return 1;
			
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*vol_quant_type*/
		if (tmpvar)
		{
			fprintf(stderr,"only quantisation method 2 allowed\n");
			return 1;
		} 
			
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*complexity_estimation_disable*/
      
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*resync_marker_disable*/
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*data_partitioning_enable*/
    // data partitioning not supported
		if (tmpvar == 1) return 1;

    if (tmpvar)
    {
      tmpvar = (Int) BitstreamReadBits(stream, 1); /*reversible_vlc_enable*/
    }
			
    tmpvar = (Int) BitstreamReadBits(stream, 1); /*scalability*/
		if (tmpvar)
		{
			fprintf(stderr,"scalability not supported\n");
			return 1;
		}
			
    BitstreamByteAlign(stream);
    tmpvar = (Int) BitstreamShowBits(stream, USER_DATA_START_CODE_LENGTH);
			
		// User data not supported!
    if (tmpvar == 434) 
      FatalError("user data not supported");
			
  
  
  
  }
  else /*short video header or corrupted bitstream*/
  {
    tmpvar = (Int) BitstreamShowBits(stream, SHORT_VIDEO_START_CODE_LENGTH);
		
    if (tmpvar == 32)
    { 
			fprintf(stderr,"Short video header unsupported\n");
			return 1;
    }
		else
		{
			fprintf(stderr,"Corrupted bitstream header\n");
			return 1;
		}
  }



	
#ifdef VERBOSE
	vol_header_bits += stream->bitcount - stream->bstart;
#endif
  return 0;
} // ParseVolHeader


/***********************************************************CommentBegin******
 *
 * -- ParseVopHeader -- Parses the VOP header
 *
 * Author :		
 *      Michael Wollborn (TUH)
 *
 * Created :		
 *	08-AUG-1996
 *
 * Purpose :
 *	Parses the VOP header
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

static UChar ParseVopHeader(Bitstream *stream, int time_inc_res,
														UChar *decode_type, UChar *round_type, UChar *vop_quant,
														UChar *fcode, UChar *error_flag)
{
  int tmpvar, bits, vop_coded, intra_dc_vlc_thr;
	UChar prediction_type; 
	// NOTE: are these per-stream values?
	static int time_base=0, mod_time_base=0;
	static int time_inc=0;

	// SW instrumenting only
#ifdef VERBOSE
	stream->bstart = stream->bitcount;
#endif

	*error_flag = 0;

  tmpvar = (Int) BitstreamShowBits(stream, VOP_START_CODE_LENGTH);

	switch (tmpvar)
	{
	// End of VOL
  case 1:
		printf("End of VOL found\n");
		return 1;
	// VOP start code
	case VOP_START_CODE:
		//
		// Parse VOP header
		//
		tmpvar = (Int) BitstreamReadBits(stream, VOP_START_CODE_LENGTH); /*vop_start_code*/
		prediction_type = (UChar) BitstreamReadBits(stream, 2); /*vop_prediction_type*/
  
		if (prediction_type == B_VOP)
			FatalError("no B_VOPs supported");
  
		tmpvar = (Int) BitstreamReadBits(stream, 1); /*modulo_time_base, stream->bitcount = 179*/

		while (tmpvar == 1)
		{
			tmpvar = (Int) BitstreamReadBits(stream, 1); /*modulo_time_base*/
			time_base++;
		}

		tmpvar = (Int) BitstreamReadBits(stream, 1); /*marker_bit*/
  
		bits = mylog(time_inc_res);
		if (bits < 1)
			bits = 1;

		time_inc = (Int) BitstreamReadBits(stream, bits); /*vop_time_increment*/

		// is this right?  maybe it's time_inc
		mod_time_base += time_base;
  
		tmpvar = (Int) BitstreamReadBits(stream, 1); /*marker_bit*/;
		vop_coded = (Int) BitstreamReadBits(stream, 1); /*vop_coded*/

		if (!vop_coded)
		{
			BitstreamByteAlign(stream);
			printf("Empty Vop decoded, continuing with next vop\n");
		}
		else
		{
			if (prediction_type == P_VOP) 
			{
				*round_type = (UChar) BitstreamReadBits(stream, 1); /*vop_rounding_type*/
			}
			else
				*round_type = 0;

			intra_dc_vlc_thr = (Int) BitstreamReadBits(stream, 3); /*intra_dc_vlc_thr*/
  
			*vop_quant = (Int) BitstreamReadBits(stream, BITS_QUANT); /*vop_quant*/

			if (prediction_type != I_VOP)
				*fcode = (UChar) BitstreamReadBits(stream, 3); /*vop_fcode_for*/
		}
		
		// Create 2-bit 'type' flag
		// NOTE: for now, data partitioning is not supported
		// and only I and P frames
		if (prediction_type == I_VOP)
			*decode_type = 0;
		else
			*decode_type = 1;
		break;
	default:
		//FatalError("wrong VOP start code (possible causes: group of VOPs, corrupted bitstream)");
		*error_flag = 1;
		break;
	}

#ifdef VERBOSE
	vop_header_bits += stream->bitcount - stream->bstart;
#endif
  return 0;
} // ParseVopHeader


/***********************************************************CommentBegin******
 *
 * -- ParseMBheader --  
 *
 * Author :
 *      Luis Ducla-Soares (IST) - lds@lx.it.pt
 *
 * Created :
 *      8-Sep-97
 *
 * Purpose :
 *       decodes the header (COD and MCBPC) of a MB
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 * Converted to a single function by P. Schumacher, Xilinx, 2004/1/8
 *
 ***********************************************************CommentEnd********/
static UChar ParseMBheader(Bitstream *stream, int mbnum, UChar *ftype, UChar *btype,
									 UChar *skipped_flag, UChar *coded, int *CBP, 
									 UChar *ACpred_flag, UChar *MBtype)
{
	UInt code, CBPY, CBPC;
  Int MCBPC;
	const int VLC_ERROR = -1;
	struct VLCtab MCBPC_Itable[32] = { { VLC_ERROR, 0 }, { 20, 6 }, { 36, 6 }, { 52, 6 }, { 4, 4 }, { 4, 4 }, { 4, 4 }, { 4, 4 }, { 19, 3 }, 
	{ 19, 3 }, { 19, 3 }, { 19, 3 }, { 19, 3 }, { 19, 3 }, { 19, 3 }, { 19, 3 }, { 35, 3 }, { 35, 3 }, { 35, 3 }, { 35, 3 }, { 35, 3 }, 
	{ 35, 3 }, { 35, 3 }, { 35, 3 }, { 51, 3 }, { 51, 3 }, { 51, 3 }, { 51, 3 }, { 51, 3 }, { 51, 3 }, { 51, 3 }, { 51, 3 } };
	struct VLCtab MCBPC_Ptable[256] = { { VLC_ERROR, 0 }, { 255, 9 }, { 52, 9 }, { 36, 9 }, { 20, 9 }, { 49, 9 }, { 35, 8 }, { 35, 8 }, 
	{ 19, 8 }, { 19, 8 }, { 50, 8 }, { 50, 8 }, { 51, 7 }, { 51, 7 }, { 51, 7 }, { 51, 7 }, { 34, 7 }, { 34, 7 }, { 34, 7 }, { 34, 7 }, 
	{ 18, 7 }, { 18, 7 }, { 18, 7 }, { 18, 7 }, { 33, 7 }, { 33, 7 }, { 33, 7 }, { 33, 7 }, { 17, 7 }, { 17, 7 }, { 17, 7 }, { 17, 7 }, 
	{ 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 4, 6 }, { 48, 6 }, { 48, 6 }, { 48, 6 }, { 48, 6 }, { 48, 6 }, 
	{ 48, 6 }, { 48, 6 }, { 48, 6 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, 
	{ 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 3, 5 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, 
	{ 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, 
	{ 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, { 32, 4 }, 
	{ 32, 4 }, { 32, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, 
	{ 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, 
	{ 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 16, 4 }, { 2, 3 }, { 2, 3 }, 
	{ 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, 
	{ 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, 
	{ 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, 
	{ 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, 
	{ 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 1, 3 }, { 1, 3 }, 
	{ 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, 
	{ 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, 
	{ 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, 
	{ 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, 
	{ 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 }, { 1, 3 } };
	struct VLCtab CBPYtab[48] = { { VLC_ERROR, 0 }, { VLC_ERROR, 0 }, { 6, 6 }, { 9, 6 }, { 8, 5 }, { 8, 5 }, { 4, 5 }, { 4, 5 }, { 2, 5 }, 
	{ 2, 5 }, { 1, 5 }, { 1, 5 }, { 0, 4 }, { 0, 4 }, { 0, 4 }, { 0, 4 }, { 12, 4 }, { 12, 4 }, { 12, 4 }, { 12, 4 }, { 10, 4 }, { 10, 4 }, 
	{ 10, 4 }, { 10, 4 }, { 14, 4 }, { 14, 4 }, { 14, 4 }, { 14, 4 }, { 5, 4 }, { 5, 4 }, { 5, 4 }, { 5, 4 }, { 13, 4 }, { 13, 4 }, { 13, 4 }, 
	{ 13, 4 }, { 3, 4 }, { 3, 4 }, { 3, 4 }, { 3, 4 }, { 11, 4 }, { 11, 4 }, { 11, 4 }, { 11, 4 }, { 7, 4 }, { 7, 4 }, { 7, 4 }, { 7, 4 } };
	
	// For SW instrumenting only
#ifdef VERBOSE
	stream->bstart = stream->bitcount;
#endif

	// If P-VOP and no DCT, then mark as un-coded 
	if ((*ftype == P_VOP) && BitstreamReadBits(stream, 1))
	{
		*skipped_flag = 1;
		*coded = 0;
		*MBtype = 0;
		*btype = INTER;
		// For SW instrumenting only
#ifdef VERBOSE
		mb_header_bits += stream->bitcount - stream->bstart;
#endif
		return 0;
	}
	else
	{
		*coded = 1;
		*skipped_flag = 0;
	}

	/////////////////////
	// MB type and CBP //
	/////////////////////
  code = BitstreamShowBits(stream, MCBPC_LENGTH);

  if (code == 1)
  {
    BitstreamFlushBits(stream, MCBPC_LENGTH);
    printf("Error decoding MCBPC of macroblock %d\n", (int) mbnum);
    return 1;
  }
	else
	{
		if ((!*ftype && code < 8) || (*ftype && code == 0))
		{
			printf("Error decoding MCBPC of macroblock %d\n", (int) mbnum);
			return 1;
		}
		else
		{
			if (!*ftype)
			{
				code >>= 3;
				if (code >= 32)
				{
					BitstreamFlushBits(stream, 1);
					MCBPC = 3;
				}	
				else 
				{
					BitstreamFlushBits(stream, (MCBPC_Itable[code]).len);
					MCBPC = (MCBPC_Itable[code]).val;
				}
			}
			else
			{
				if (code >= 256)
				{
					BitstreamFlushBits(stream, 1);
					MCBPC = 0;
				}
				else
				{
					BitstreamFlushBits(stream, (MCBPC_Ptable[code]).len);
					MCBPC = (MCBPC_Ptable[code]).val;
				}
			}
		}
	} // if-else code=1

  *MBtype = MCBPC & 7;
	CBPC = MCBPC >> 4 & 3;
		
  if (*MBtype == 3 || *MBtype == 4)
		*ACpred_flag = (UChar) BitstreamReadBits(stream, 1); /*ACpred_flag*/
  else *ACpred_flag = 0; // DBP added, to make mgmt of this flag localized

	// block type
	*btype = (*MBtype <= 2) ? INTER : INTRA;

	// Inter MB not possible in I-VOP
	if (*ftype == I_VOP && *btype == INTER) 
		return 1;

	/////////////////////////
	// CBPY: CBP luminance //
	/////////////////////////
	code = BitstreamShowBits(stream, 6);

  if (code < 2)
    return 1;
	else
	{
		if (code >= 48)
		{
			BitstreamFlushBits(stream, 2);
			CBPY = 15;
		}
		else
		{
			BitstreamFlushBits(stream, (CBPYtab[code]).len);
			CBPY = (CBPYtab[code]).val;
		}

		if (*btype == INTER)
			CBPY = 15 - CBPY;
	}

	// final code block pattern (CBP)
	*CBP = CBPY << 2 | CBPC;

	// For SW instrumenting only
#ifdef VERBOSE
	mb_header_bits += stream->bitcount - stream->bstart;
#endif
	return 0;
} // ParseMBheader


/***********************************************************CommentBegin******
 *
 * -- mylog -- ceil(2log(x)) function
 *
 * Author :		
 *	Andy Dewilde
 *
 * Created :		
 *	29-05-2002
 *
 * Purpose :		
 *	Replaces the ceil(log/log)-function located in the MPEG4 decoder
 *
 * Basics :		
 *      It replaces functions where the ceil(2log(x)) is needed. This was done
 *      by ceil(log(x)/log(2)).
 *          x    |     ceil(2log(x))
 *      ----------------------------
 *          1    |           0       1 bit needed for x
 *          2    |           1       2 bits needed for x
 *          3    |           2       
 *          4    |           2       3 bits needed for x
 *          5    |           3
 *         ...   |          ... 
 *
 *      Functions counts bits needed to halt current value (x)
 *      If there is more than one '1' in the current value, indicated
 *      by ones, it means that x was not a power of two, so the number
 *      of bits is return needed to repesent the first next power of two, 
 *      minus one. (see above)
 ***********************************************************CommentEnd********/

static int mylog(int x)
{
  int cnt=0, ones=0;

  do
  {
    // count the number of '1'
    if(x & 1)
			ones ++;

    x>>=1;

    // count the number of bits
    cnt++;
  }
  while(x);

  return (ones > 1 ? cnt : cnt-1);
}
