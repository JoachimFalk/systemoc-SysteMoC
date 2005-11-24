/*
 *  Author: Kristof Denolf
 *  Date: November 21st 2001
 *  IMEC
 */

#ifndef __AllInOne_h
#include "AllInOne.h"
#endif
 
static void VlcDecTCOEF(Bitstream *stream, unsigned char *error_flag, unsigned char btype, struct Tcoef *run_level);

/***********************************************************CommentBegin******
 *
 * -- TextureVLD --  variable length decoding of texture/DCT 
 *
 * Author :		
 *	P. Schumacher, Xilinx
 *
 * Created :		
 *	2004/01/14
 * 
 * Purpose :		
 *      Performs variable length decoding on texture/DCT information
 *
 ***********************************************************CommentEnd********/
// Values							Bits
// ------							----
// Inputs
//		data stream			 8
//		frame type			 1
//		block type			 1
//		block # or comp	 3
//		block pattern		 6				 
// Parameters
//		blocks/MB				 4
//		block width			 4
// Outputs
//		q_block					12 [64 values]
unsigned char TextureVLD(Bitstream *stream, unsigned char ftype, unsigned char btype, 
							   unsigned char comp, int CBP, SInt *q_block, int mbx, int mby)
{
	SInt i;
	int n, first_bit, last;
	unsigned char error_flag=0;
	/*UChar*/UInt bits, done=0;
	UInt DC_size/*SC-parser.hpp: dcbits .*/, code;
	struct Tcoef run_level = {0,0,0,0};
	const unsigned char ACcoded = CBP & (1 << (BLOCK_CNT - 1 - comp));
		
	// For SW instrumenting only
#ifdef VERBOSE
	stream->bstart = stream->bitcount;
#endif
if( mby == 0 && mbx == 0 && comp == 3 )
{
  code = 0;
}
	// Zero out q_block
	for (n = 0; n < B_WIDTH * B_HEIGHT; n++)
		q_block[n] = 0;

	//////////////////
	// Get DC value //
	//////////////////
	/* Code for number of DC bits
                          Y(comp<4)   UV
     000000000000        err          err
     000000000001        err          12
     00000000001x         12          11
     0000000001xx         11          10
     000000001xxx         10          9
     00000001xxxx          9          8
     0000001xxxxx          8          7
     000001xxxxxx          7          6
     00001xxxxxxx          6          5
     0001xxxxxxxx          5          4
     001xxxxxxxxx          4          3
     010xxxxxxxxx          3            \   2
     011xxxxxxxxx          0            /   2
     10xxxxxxxxxx          2          1
     11xxxxxxxxxx          1          0          */

        if (btype == INTRA)
	{
		//
		// Extract the size of the DC coefficient (in bits)
		//
		// Y blocks
		if (comp < 4)
		{
			bits = 11;
			while (bits >= 4) {	
				code = BitstreamShowBits(stream, bits);
				if (code == 1) {
					BitstreamFlushBits(stream, bits);
					DC_size = bits+1;
					done = 1;
					break;
				}
				else
					bits--;
			} 

			if (!done)
			{
				code = BitstreamShowBits(stream, 3);
				switch (code)
				{
				case 1:
					BitstreamFlushBits(stream, 3);
					DC_size = 4;
					done = 1;
					break;
				case 2:
					BitstreamFlushBits(stream, 3);
					DC_size = 3;
					done = 1;
					break;
				case 3:
					BitstreamFlushBits(stream, 3);
					DC_size = 0;
					done = 1;
					break;
				default:
					done = 0;
				}
			}

			if (!done)
			{
				code = BitstreamShowBits(stream, 2);				
				switch (code)
				{
				case 2:
					BitstreamFlushBits(stream, 2);
					DC_size = 2;
					break;
				case 3:
					BitstreamFlushBits(stream, 2);
					DC_size = 1;
					break;
				default:
					FatalError("cannot extract DC size");
				}
			}
		}
		// U or V blocks
		else
		{
			bits = 12;
			while (bits >= 2) {	
				code = BitstreamShowBits(stream, bits);
				if (code == 1) {
					BitstreamFlushBits(stream, bits);
					DC_size = bits;
					break;
				}
				else
					bits--;
			} 
			if (bits == 1) {
				BitstreamFlushBits(stream, 2);
				DC_size = 3-code;
			}
		}

		// Use the size to decode the DC coefficient
		if (DC_size)
		{ int myval, dummy;

			code = BitstreamReadBits(stream, DC_size);  /*DC coeff*/
			first_bit = code >> (DC_size - 1);

			if (!first_bit)
      { myval = code + 1 - (1 << DC_size);
				q_block[0] = (short) (-1*(code ^ (1 << DC_size) - 1));
        if( q_block[0] != myval )
        {
          dummy = 1;
          printf("dummy value!\n");
        }
      }
			else
				q_block[0] = (short) code;

			if (DC_size > 8)
				BitstreamReadBits(stream, 1); /*Marker bit*/
		}
	} // if btype == INTRA

	///////////////////
	// Get AC values //
	///////////////////
	if (ACcoded)
	{ static int dummy = 0;

		// i = !btype
 		i = (btype == INTRA) ? 1 : 0;
		
		do
		{
			VlcDecTCOEF(stream, &error_flag, btype, &run_level);
			if (error_flag)
				FatalError("cannot decode TCOEF");

			i += run_level.run;
			if (i >= 64)
				FatalError("too much COEF");

			if (run_level.sign == 1)
				q_block[i] = (SInt)-run_level.level;
			else
				q_block[i] = (SInt)run_level.level;

			last = run_level.last;

      if( mbx == 0 && mby == 1 )
      {
        dummy += 1;
      }
      else dummy += 1;
			i++;

		}
		while (!last);
    printf("AC coded sampleblock\n");
	} else { // !ACcoded
    printf("Not AC coded sampleblock\n");
  }

	// For SW instrumenting only
#ifdef VERBOSE
	texture_bits += stream->bitcount - stream->bstart;
#endif
	return error_flag;
} // TextureVLD


/***********************************************************CommentBegin******
 *
 * -- VlcDecTCOEF -- Decodes a VLC coded DCT transform coefficient
 *
 * Author :		
 *	Paulo Nunes (IST) - Paulo.Nunes@lx.it.pt
 *
 * Created :		
 *	1-Mar-96
 *
 * Purpose :		
 *	To decode a VLC coded DCT transform coefficient.
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

static void VlcDecTCOEF(Bitstream *stream, unsigned char *error_flag, unsigned char btype, struct Tcoef *run_level)
{
  UInt code;
  struct VLCtab *tab;
	struct VLCtab DCT3Dtab0[112] = { { 4225, 7 }, { 4209, 7 }, { 4193, 7 }, { 4177, 7 }, { 193, 7 }, { 177, 7 }, { 161, 7 }, { 4, 7 }, 
	{ 4161, 6 }, { 4161, 6 }, { 4145, 6 }, { 4145, 6 }, { 4129, 6 }, { 4129, 6 }, { 4113, 6 }, { 4113, 6 }, { 145, 6 }, { 145, 6 }, 
	{ 129, 6 }, { 129, 6 }, { 113, 6 }, { 113, 6 }, { 97, 6 }, { 97, 6 }, { 18, 6 }, { 18, 6 }, { 3, 6 }, { 3, 6 }, { 81, 5 }, { 81, 5 }, 
	{ 81, 5 }, { 81, 5 }, { 65, 5 }, { 65, 5 }, { 65, 5 }, { 65, 5 }, { 49, 5 }, { 49, 5 }, { 49, 5 }, { 49, 5 }, { 4097, 4 }, { 4097, 4 }, 
	{ 4097, 4 }, { 4097, 4 }, { 4097, 4 }, { 4097, 4 }, { 4097, 4 }, { 4097, 4 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, 
	{ 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, 
	{ 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, 
	{ 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, 
	{ 17, 3 }, { 17, 3 }, { 17, 3 }, { 17, 3 }, { 33, 4 }, { 33, 4 }, { 33, 4 }, { 33, 4 }, { 33, 4 }, { 33, 4 }, { 33, 4 }, { 33, 4 }, 
	{ 2, 4 }, { 2, 4 }, { 2, 4 }, { 2, 4 }, { 2, 4 }, { 2, 4 }, { 2, 4 }, { 2, 4 } };
	struct VLCtab DCT3Dtab2[120] = { { 4114, 11 }, { 4114, 11 }, { 4099, 11 }, { 4099, 11 }, { 11, 11 }, { 11, 11 }, { 10, 11 }, { 10, 11 }, { 4545, 10 }, { 4545, 10 }, { 4545, 10 }, { 4545, 10 }, { 4529, 10 }, { 4529, 10 }, { 4529, 10 }, { 4529, 10 }, { 4513, 10 }, { 4513, 10 }, { 4513, 10 }, { 4513, 10 }, { 4497, 10 }, { 4497, 10 }, { 4497, 10 }, { 4497, 10 }, { 146, 10 }, { 146, 10 }, { 146, 10 }, { 146, 10 }, { 130, 10 }, { 130, 10 }, { 130, 10 }, { 130, 10 }, { 114, 10 }, { 114, 10 }, { 114, 10 }, { 114, 10 }, { 98, 10 }, { 98, 10 }, { 98, 10 }, { 98, 10 }, { 82, 10 }, { 82, 10 }, { 82, 10 }, { 82, 10 }, { 51, 10 }, { 51, 10 }, { 51, 10 }, { 51, 10 }, { 35, 10 }, { 35, 10 }, { 35, 10 }, { 35, 10 }, { 20, 10 }, { 20, 10 }, { 20, 10 }, { 20, 10 }, { 12, 11 }, { 12, 11 }, { 21, 11 }, { 21, 11 }, { 369, 11 }, { 369, 11 }, { 385, 11 }, { 385, 11 }, { 4561, 11 }, { 4561, 11 }, { 4577, 11 }, { 4577, 11 }, { 4593, 11 }, { 4593, 11 }, { 4609, 11 }, { 4609, 11 }, { 22, 12 }, { 36, 12 }, { 67, 12 }, { 83, 12 }, { 99, 12 }, { 162, 12 }, { 401, 12 }, { 417, 12 }, { 4625, 12 }, { 4641, 12 }, { 4657, 12 }, { 4673, 12 }, { 4689, 12 }, { 4705, 12 }, { 4721, 12 }, { 4737, 12 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 } };
	struct VLCtab DCT3Dtab4[96] = { { 18, 10 }, { 17, 10 }, { 69121, 9 }, { 69121, 9 }, { 68865, 9 }, { 68865, 9 }, { 68609, 9 }, { 68609, 9 }, { 68353, 9 }, { 68353, 9 }, { 68097, 9 }, { 68097, 9 }, { 65794, 9 }, { 65794, 9 }, { 65540, 9 }, { 65540, 9 }, { 3073, 9 }, { 3073, 9 }, { 2817, 9 }, { 2817, 9 }, { 1794, 9 }, { 1794, 9 }, { 1538, 9 }, { 1538, 9 }, { 1282, 9 }, { 1282, 9 }, { 771, 9 }, { 771, 9 }, { 515, 9 }, { 515, 9 }, { 262, 9 }, { 262, 9 }, { 261, 9 }, { 261, 9 }, { 16, 9 }, { 16, 9 }, { 1026, 9 }, { 1026, 9 }, { 15, 9 }, { 15, 9 }, { 14, 9 }, { 14, 9 }, { 13, 9 }, { 13, 9 }, { 67585, 8 }, { 67585, 8 }, { 67585, 8 }, { 67585, 8 }, { 67329, 8 }, { 67329, 8 }, { 67329, 8 }, { 67329, 8 }, { 67073, 8 }, { 67073, 8 }, { 67073, 8 }, { 67073, 8 }, { 65539, 8 }, { 65539, 8 }, { 65539, 8 }, { 65539, 8 }, { 2561, 8 }, { 2561, 8 }, { 2561, 8 }, { 2561, 8 }, { 2305, 8 }, { 2305, 8 }, { 2305, 8 }, { 2305, 8 }, { 2049, 8 }, { 2049, 8 }, { 2049, 8 }, { 2049, 8 }, { 67841, 8 }, { 67841, 8 }, { 67841, 8 }, { 67841, 8 }, { 770, 8 }, { 770, 8 }, { 770, 8 }, { 770, 8 }, { 260, 8 }, { 260, 8 }, { 260, 8 }, { 260, 8 }, { 12, 8 }, { 12, 8 }, { 12, 8 }, { 12, 8 }, { 11, 8 }, { 11, 8 }, { 11, 8 }, { 11, 8 }, { 10, 8 }, { 10, 8 }, { 10, 8 }, { 10, 8 } };
	struct VLCtab DCT3Dtab5[120] = { { 65543, 11 }, { 65543, 11 }, { 65542, 11 }, { 65542, 11 }, { 22, 11 }, { 22, 11 }, { 21, 11 }, { 21, 11 }, { 66050, 10 }, { 66050, 10 }, { 66050, 10 }, { 66050, 10 }, { 65795, 10 }, { 65795, 10 }, { 65795, 10 }, { 65795, 10 }, { 65541, 10 }, { 65541, 10 }, { 65541, 10 }, { 65541, 10 }, { 3329, 10 }, { 3329, 10 }, { 3329, 10 }, { 3329, 10 }, { 1283, 10 }, { 1283, 10 }, { 1283, 10 }, { 1283, 10 }, { 2050, 10 }, { 2050, 10 }, { 2050, 10 }, { 2050, 10 }, { 1027, 10 }, { 1027, 10 }, { 1027, 10 }, { 1027, 10 }, { 772, 10 }, { 772, 10 }, { 772, 10 }, { 772, 10 }, { 516, 10 }, { 516, 10 }, { 516, 10 }, { 516, 10 }, { 263, 10 }, { 263, 10 }, { 263, 10 }, { 263, 10 }, { 20, 10 }, { 20, 10 }, { 20, 10 }, { 20, 10 }, { 19, 10 }, { 19, 10 }, { 19, 10 }, { 19, 10 }, { 23, 11 }, { 23, 11 }, { 24, 11 }, { 24, 11 }, { 264, 11 }, { 264, 11 }, { 2306, 11 }, { 2306, 11 }, { 66306, 11 }, { 66306, 11 }, { 66562, 11 }, { 66562, 11 }, { 69377, 11 }, { 69377, 11 }, { 69633, 11 }, { 69633, 11 }, { 25, 12 }, { 26, 12 }, { 27, 12 }, { 265, 12 }, { 1539, 12 }, { 266, 12 }, { 517, 12 }, { 1795, 12 }, { 3585, 12 }, { 65544, 12 }, { 66818, 12 }, { 67074, 12 }, { 69889, 12 }, { 70145, 12 }, { 70401, 12 }, { 70657, 12 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 }, { 7167, 7 } };
	struct VLCtab DCT3Dtab3[112] = { { 66561, 7 }, { 66305, 7 }, { 1537, 7 }, { 66817, 7 }, { 1793, 7 }, { 514, 7 }, { 259, 7 }, { 9, 7 }, { 65538, 6 }, { 65538, 6 }, { 1281, 6 }, { 1281, 6 }, { 66049, 6 }, { 66049, 6 }, { 65793, 6 }, { 65793, 6 }, { 1025, 6 }, { 1025, 6 }, { 769, 6 }, { 769, 6 }, { 8, 6 }, { 8, 6 }, { 7, 6 }, { 7, 6 }, { 258, 6 }, { 258, 6 }, { 6, 6 }, { 6, 6 }, { 513, 5 }, { 513, 5 }, { 513, 5 }, { 513, 5 }, { 5, 5 }, { 5, 5 }, { 5, 5 }, { 5, 5 }, { 4, 5 }, { 4, 5 }, { 4, 5 }, { 4, 5 }, { 65537, 4 }, { 65537, 4 }, { 65537, 4 }, { 65537, 4 }, { 65537, 4 }, { 65537, 4 }, { 65537, 4 }, { 65537, 4 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 1, 2 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 2, 3 }, { 257, 4 }, { 257, 4 }, { 257, 4 }, { 257, 4 }, { 257, 4 }, { 257, 4 }, { 257, 4 }, { 257, 4 }, { 3, 4 }, { 3, 4 }, { 3, 4 }, { 3, 4 }, { 3, 4 }, { 3, 4 }, { 3, 4 }, { 3, 4 } };
	struct VLCtab DCT3Dtab1[96] = { { 9, 10 }, { 8, 10 }, { 4481, 9 }, { 4481, 9 }, { 4465, 9 }, { 4465, 9 }, { 4449, 9 }, { 4449, 9 }, { 4433, 9 }, { 4433, 9 }, { 4417, 9 }, { 4417, 9 }, { 4401, 9 }, { 4401, 9 }, { 4385, 9 }, { 4385, 9 }, { 4369, 9 }, { 4369, 9 }, { 4098, 9 }, { 4098, 9 }, { 353, 9 }, { 353, 9 }, { 337, 9 }, { 337, 9 }, { 321, 9 }, { 321, 9 }, { 305, 9 }, { 305, 9 }, { 289, 9 }, { 289, 9 }, { 273, 9 }, { 273, 9 }, { 257, 9 }, { 257, 9 }, { 241, 9 }, { 241, 9 }, { 66, 9 }, { 66, 9 }, { 50, 9 }, { 50, 9 }, { 7, 9 }, { 7, 9 }, { 6, 9 }, { 6, 9 }, { 4353, 8 }, { 4353, 8 }, { 4353, 8 }, { 4353, 8 }, { 4337, 8 }, { 4337, 8 }, { 4337, 8 }, { 4337, 8 }, { 4321, 8 }, { 4321, 8 }, { 4321, 8 }, { 4321, 8 }, { 4305, 8 }, { 4305, 8 }, { 4305, 8 }, { 4305, 8 }, { 4289, 8 }, { 4289, 8 }, { 4289, 8 }, { 4289, 8 }, { 4273, 8 }, { 4273, 8 }, { 4273, 8 }, { 4273, 8 }, { 4257, 8 }, { 4257, 8 }, { 4257, 8 }, { 4257, 8 }, { 4241, 8 }, { 4241, 8 }, { 4241, 8 }, { 4241, 8 }, { 225, 8 }, { 225, 8 }, { 225, 8 }, { 225, 8 }, { 209, 8 }, { 209, 8 }, { 209, 8 }, { 209, 8 }, { 34, 8 }, { 34, 8 }, { 34, 8 }, { 34, 8 }, { 19, 8 }, { 19, 8 }, { 19, 8 }, { 19, 8 }, { 5, 8 }, { 5, 8 }, { 5, 8 }, { 5, 8 } };

	const int intra_max_level[2][64] = { { 27, 10, 5, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 8, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0 } };
	const int intra_max_run0[28] = { 999, 14, 9, 7, 3, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	const int inter_max_run0[13] = { 999, 26, 10, 6, 2, 1, 1, 0, 0, 0, 0, 0, 0 };
	const int inter_max_run1[4] = { 999, 40, 1, 0 };
	const int intra_max_run1[9] = { 999, 20, 6, 1, 0, 0, 0, 0, 0 };
	const int inter_max_level[2][64] = { { 12, 6, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0 } };
	const int ESCAPE = 7167; 
  int dummy;

        run_level->last = 0;
	run_level->run = 0;
	run_level->level = 0;
	
  code = BitstreamShowBits(stream, 12);

  if (code >= 512)
    if (btype == INTER)
      tab = &DCT3Dtab0[(code >> 5) - 16];
    else
      tab = &DCT3Dtab3[(code >> 5) - 16];
  else if (code >= 128)
    if (btype == INTER)
      tab = &DCT3Dtab1[(code >> 2) - 32];
    else
      tab = &DCT3Dtab4[(code >> 2) - 32];
  else if (code >= 8)
    if (btype == INTER)
      tab = &DCT3Dtab2[(code >> 0) - 8];
    else
      tab = &DCT3Dtab5[(code >> 0) - 8];
  else
  {
    fprintf(stderr, "\nCHECK POINT : Error Position BB-1\n");
    *error_flag = 1;
		return;
  }

  BitstreamFlushBits(stream, tab->len);

  if (btype == INTER)
  {
    run_level->run = tab->val >> 4 & 255;
    run_level->level = tab->val & 15;
    run_level->last = tab->val >> 12 & 1;
  }
  else
  {
    run_level->run = tab->val >> 8 & 255;
    run_level->level = tab->val & 255;
    run_level->last = tab->val >> 16 & 1;
  }

  if (tab->val == ESCAPE)
  {
    // assume not a short header
      int level_offset;

      level_offset = BitstreamReadBits(stream, 1); /*ESC level offset*/

      if (!level_offset)
      {
        code = BitstreamShowBits(stream, 12);

        if (code >= 512)
          if (btype == INTER)
            tab = &DCT3Dtab0[(code >> 5) - 16];
          else
            tab = &DCT3Dtab3[(code >> 5) - 16];
        else if (code >= 128)
          if (btype == INTER)
            tab = &DCT3Dtab1[(code >> 2) - 32];
          else
            tab = &DCT3Dtab4[(code >> 2) - 32];
        else if (code >= 8)
          if (btype == INTER)
            tab = &DCT3Dtab2[(code >> 0) - 8];
          else
            tab = &DCT3Dtab5[(code >> 0) - 8];
        else
        {
          fprintf(stderr, "\nCHECK POINT : Error Position BB-2\n");
          *error_flag = 1;
					return;
        }
if( tab->val == ESCAPE )
{
  dummy = 0;
}

        BitstreamFlushBits(stream, tab->len);

        if (btype == INTER)
        {
          run_level->run = tab->val >> 4 & 255;
          run_level->level = tab->val & 15;
          run_level->last = tab->val >> 12 & 1;
        }
        else
        {
          run_level->run = tab->val >> 8 & 255;
          run_level->level = tab->val & 255;
          run_level->last = tab->val >> 16 & 1;
        }

        if (btype == INTER)
					run_level->level = run_level->level + inter_max_level[run_level->last][run_level->run];    
        else
          run_level->level = run_level->level + intra_max_level[run_level->last][run_level->run];
        run_level->sign = (short) BitstreamReadBits(stream, 1); /*SIGN*/
      }
      else
      {
        int run_offset;

        run_offset = BitstreamReadBits(stream, 1); /*ESC run offset*/

        if (!run_offset)
        {
          code = BitstreamShowBits(stream, 12);

          if (code >= 512)
            if (btype == INTER)
              tab = &DCT3Dtab0[(code >> 5) - 16];
            else
              tab = &DCT3Dtab3[(code >> 5) - 16];
          else if (code >= 128)
            if (btype == INTER)
              tab = &DCT3Dtab1[(code >> 2) - 32];
            else
              tab = &DCT3Dtab4[(code >> 2) - 32];
          else if (code >= 8)
            if (btype == INTER)
              tab = &DCT3Dtab2[(code >> 0) - 8];
            else
              tab = &DCT3Dtab5[(code >> 0) - 8];
          else
          {
            fprintf(stderr, "\nCHECK POINT : Error Position BB-3\n");
            *error_flag = 1;
						return;
          }
if( tab->val == ESCAPE )
{
  dummy = 0;
}

          BitstreamFlushBits(stream, tab->len);

          if (btype == INTER)
          {
            run_level->run = tab->val >> 4 & 255;
            run_level->level = tab->val & 15;
            run_level->last = tab->val >> 12 & 1;
          }
          else
          {
            run_level->run = tab->val >> 8 & 255;
            run_level->level = tab->val & 255;
            run_level->last = tab->val >> 16 & 1;
          }

          if (btype == INTER)
          {
						if (run_level->last)
              run_level->run = run_level->run + inter_max_run1[run_level->level] + 1;
            else
              run_level->run = run_level->run + inter_max_run0[run_level->level] + 1;   
          }
          else
          {
            if (run_level->last)
              run_level->run = run_level->run + intra_max_run1[run_level->level] + 1;
            else
              run_level->run = run_level->run + intra_max_run0[run_level->level] + 1;
          }

          run_level->sign = (short) BitstreamReadBits(stream, 1); /*SIGN*/

        }
        else
        {
          run_level->last = (short) BitstreamReadBits(stream, 1); /*LAST*/
          run_level->run = (short) BitstreamReadBits(stream, 6); /*RUN*/
          BitstreamReadBits(stream, 1); /*marker_bit*/
          run_level->level = (short) BitstreamReadBits(stream, 12); /*LEVEL*/
          BitstreamReadBits(stream, 1); /*marker_bit*/

          if (run_level->level >= 2048)
          {
            run_level->sign = 1;
            run_level->level = 4096 - run_level->level;
          }
          else
          {
            run_level->sign = 0;
          }
        }
      }
  }
  else
  {
    run_level->sign = (short) BitstreamReadBits(stream, 1); /*SIGN*/
  }
} // VlcDecTCOEF
