/*
 *  Author: Kristof Denolf and ATOMIUM 
 *  Based on MPEG-4 reference code
 *  Date: November 21st 2001
 *  IMEC
 */


#ifndef __AllInOne_h
#include "AllInOne.h"
#endif

#ifdef USE_DISPLAY
#include "picture.h"
#endif

static Void BitstreamFillBuffer(Bitstream *stream);

/***********************************************************CommentBegin******
 *
 * -- BitstreamInit -- Initialises a bitstream structure
 *
 * Author :		
 *	Paulo Nunes (IST) - Paulo.Nunes@lx.it.pt
 *
 * Created :		
 *	1-Mar-1996
 *
 * Purpose :		
 *	Initialisation of a bitstream structure.
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

int BitstreamInit(Bitstream *stream, int num)
{
  File *file;
  Int i;

  if ((file = fopen(bitfile[num], "rb")) == NULL)
		return -1;

  stream->fptr = file;
  stream->incnt = 0;
  stream->rdptr = stream->rdbfr + BFR_SIZE;
	stream->rdtop = stream->rdbfr;
	stream->bitcount = 0;

  for (i = 0; i < 3 * INBFR_BYTES; i++)
  {
    stream->inbfr[i] = 0;
  }

  return 0;
}	// BitstreamInit


/***********************************************************CommentBegin******
 *
 * -- BitstreamClose -- Frees a bitstream structure
 *
 * Author :		
 *	Paulo Nunes (IST) - Paulo.Nunes@lx.it.pt
 *
 * Created :		
 *	1-Mar-1996
 *
 * Purpose :		
 *	Freeing a bitstream structure.
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

void BitstreamClose(Bitstream *stream)
{
	if (stream) {
		if (stream->fptr) fclose(stream->fptr);
		free(stream);
	}
}


/***********************************************************CommentBegin******
 *
 * -- BitstreamFillBuffer -- Fills a bitstream structure
 *
 * Author :		
 *	Paulo Nunes (IST) - Paulo.Nunes@lx.it.pt
 *
 * Created :		
 *	1-Mar-1996
 *
 * Purpose :		
 *	To read a piece of the bitstream from the bitstream file to the
 *	bitstream buffer. 
 *	
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

static Void BitstreamFillBuffer(Bitstream *stream)
{
  Int l, i, bytesInBuffer;

  for (i = 0; i < INBFR_BYTES; i++)
  {
    stream->inbfr[i] = stream->inbfr[i + 2 * INBFR_BYTES];
  }

	if (stream->rdptr < stream->rdtop)
		bytesInBuffer = stream->rdtop - stream->rdptr;
	else
		bytesInBuffer = BFR_SIZE - (stream->rdptr - stream->rdtop);

  if (bytesInBuffer <= 2*INBFR_BYTES)
  {
		if (stream->rdptr < stream->rdtop)
		{
			l = fread((Char *) stream->rdtop, sizeof(UChar), BFR_SIZE - (stream->rdtop - stream->rdbfr), stream->fptr);
			l += fread((Char *) stream->rdbfr, sizeof(UChar), stream->rdptr - stream->rdbfr, stream->fptr);
		}
		else
		{
			l = fread((Char *) stream->rdtop, sizeof(UChar), stream->rdptr - stream->rdtop, stream->fptr);
		}

		if (l < BFR_SIZE - bytesInBuffer)
		{
			if ((BFR_SIZE - bytesInBuffer) - l >= 4)
			{
				if ((stream->rdtop + l) > BFR_SIZE + stream->rdbfr)
					l -= BFR_SIZE  + stream->rdbfr - stream->rdtop;
				else
					l += stream->rdtop - stream->rdbfr;

				for (i = 0; i < 3; i++)
				{
					if (l >= BFR_SIZE)
						l = 0;
					stream->rdbfr[l++] = 0;
				}

				if (l >= BFR_SIZE)
					l = 0;

				stream->rdbfr[l++] = 1;
				stream->rdtop = stream->rdbfr+l;
			}
			else
			{
				if ((stream->rdtop + l) > BFR_SIZE + stream->rdbfr)
					l -= BFR_SIZE  + stream->rdbfr - stream->rdtop;
				else
					l += stream->rdtop - stream->rdbfr;

				stream->rdtop = stream->rdbfr+l;
			}
			
		}
		else
			stream->rdtop = stream->rdptr;
  }

  for (l = 0; l < 2 * INBFR_BYTES; l++)
	{
		if (stream->rdptr >= stream->rdbfr+BFR_SIZE)
			stream->rdptr = stream->rdbfr;
    stream->inbfr[l + INBFR_BYTES] = *stream->rdptr++;
	}

  stream->incnt += 2 * INBFR_BYTES * 8;
}				/* BitstreamFillBuffer */


/***********************************************************CommentBegin******
 *
 * -- BitstreamShowBits -- Shows the next "nbits" of the bitstream
 *
 * Author :		
 *	Paulo Nunes (IST) - Paulo.Nunes@lx.it.pt
 *
 * Created :		
 *	1-Mar-1996
 *
 * Purpose :		
 *	To see the next "nbits" bitstream bits without advancing the read
 *	pointer. 
 * 
 * Description :	
 *	This function allows to see the next "nbits" after the current
 *	position of the bitstream pointer without advancing the pointer. It
 *	returns "nbits" bitstream bits right aligned in one UInt (see
 *	example)  
 *  			  ______ 
 *  			  |    | 
 *  			  v    v
 *  	bitstream -> 0000001000100111101 
 *  			  ^
 *  			  |
 *  	BitstreamShowBits(bitstream, 6) = 17
 *  	(i.e. 00000000000000000000000000010001) 
 *  					^    ^ 
 *  					|    |
 *  	
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

UInt BitstreamShowBits(Bitstream *stream, Int nbits)
{
  UChar *v;
  UInt b;
  static UInt msk[33] = { 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287, 1048575, 2097151, 4194303, 8388607, 16777215, 33554431, 67108863, 134217727, 268435455, 536870911, 1073741823, 2147483647, -1 };
	b = 0;
  if (stream->incnt < (nbits + 8))
    BitstreamFillBuffer(stream);
  v = stream->inbfr + ((3 * 8 * INBFR_BYTES - stream->incnt) >> 3);
		
	switch((nbits-1)>>3){
	case 0:
		b = ((v[0] <<8 | v[1]) >> (9 - nbits + (stream->incnt - 1 & 7))) & msk[nbits];
		break;
	case 1:
		b = ((v[0] << 16 | v[1] << 8 | v[2]) >> (17 - nbits + (stream->incnt - 1 & 7))) & msk[nbits];
		break;
	case 2:
		b = ((v[0] << 24 | v[1] << 16 | v[2] << 8 | v[3]) >> (25 - nbits + (stream->incnt - 1 & 7))) & msk[nbits];
		break;
	case 3:
		b = ((v[0] << 32 | v[1] << 24 | v[2] << 16 | v[3] << 8 | v[4]) >> (33 - nbits + (stream->incnt - 1 & 7))) & msk[nbits];
		break;
	default:
		fprintf(stderr, "ERROR: number of bits greater than size of UInt.\n");
	}
	
	return b;
}

/***********************************************************CommentBegin******
 *
 * -- BitstreamFlushBits -- Advances the bitstream read pointer
 *
 * Author :		
 *	Paulo Nunes (IST) - Paulo.Nunes@lx.it.pt
 *
 * Created :		
 *	1-Mar-1996
 *
 * Purpose :		
 *	To advance the bitstream read pointer.
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

Void BitstreamFlushBits(Bitstream *stream, Int nbits)
{
  stream->incnt -= nbits;

  if (stream->incnt < 0)
    BitstreamFillBuffer(stream);

#ifdef VERBOSE
	stream->bitcount += nbits;
#endif
}				/* RVLCBitstreamFlushBitsBackward */

/***********************************************************CommentBegin******
 *
 * -- BitstreamReadBits -- Reads the next next "nbits" from the bitstream
 *
 * Author :		
 *	Paulo Nunes (IST) - Paulo.Nunes@lx.it.pt
 *
 * Created :		
 *	1-Mar-1996
 *
 * Purpose :		
 *	To read the next "nbits" from the bitstream.
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

UInt BitstreamReadBits(Bitstream *stream, Int nbits)
{
  UInt l;

#ifdef USE_DISPLAY
	ProcessWindowsMessages();
#endif

  l = BitstreamShowBits(stream, nbits);
  BitstreamFlushBits(stream, nbits);
  return l;
}


/***********************************************************CommentBegin******
 *
 * -- BitstreamByteAlign -- Advances the bitstream read pointer until it is
 *                          byte aligned
 *
 * Author :		
 *	Luis Ducla-Soares (IST) - lds@lx.it.pt
 *
 * Created :		
 *	14-Jan-1997
 *
 * Purpose :		
 *	To advance the bitstream read pointer until it is byte aligned.
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

UInt BitstreamByteAlign(Bitstream *stream)
{
  UInt n_stuffed, v, should_bits;

  n_stuffed = stream->incnt % 8;

  if (n_stuffed == 0)
    n_stuffed = 8;
  v = BitstreamShowBits(stream, n_stuffed);
  should_bits = (1 << (n_stuffed - 1)) - 1;

  if (v != should_bits)
  {
#ifdef VERBOSE
    fprintf(stderr, "stuffing bits not correct\n");
#endif
	  BitstreamFlushBits(stream, n_stuffed);
  }
	else
	{
  BitstreamFlushBits(stream, n_stuffed);
	}
	
	return n_stuffed;
}

/* BitstreamByteAlign */


/***********************************************************CommentBegin******
 *
 * -- BitstreamShowBitsByteAlign -- Shows the next "nbits" of the bitstream
 *
 * Author :		
 *	Luis Ducla Soares (IST) - lds@lx.it.pt
 *
 * Created :		
 *	28-April-1997
 *
 * Purpose :		
 *	To see the next "nbits" bitstream bits (byte aligned) without advancing the read
 *	pointer. 
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

UInt BitstreamShowBitsByteAlign(Bitstream *stream, Int nbits)
{
  UChar *v;
  UInt b;
  C_UInt byte = 8;
  static UInt msk[33] = { 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287, 1048575, 2097151, 4194303, 8388607, 16777215, 33554431, 67108863, 134217727, 268435455, 536870911, 1073741823, 2147483647, -1 };
  UInt n_stuffed;

  if (nbits > 8 * sizeof(UInt))
  {
    fprintf(stderr, "ERROR: number of bits greater than size of UInt.\n");
		return 0;
  }

  n_stuffed = stream->incnt % byte;

  if (n_stuffed == 0)
    n_stuffed = byte;

  if ((stream->incnt) < (nbits + n_stuffed + 8))
    BitstreamFillBuffer(stream);
  v = stream->inbfr + ((3 * 8 * INBFR_BYTES - stream->incnt + n_stuffed) >> 3);
  b = v[0] << 24 | v[1] << 16 | v[2] << 8 | v[3];
  return (b >> (32 - nbits)) & msk[nbits];
}				/* BitstreamShowBitsByteAlign */



/***********************************************************CommentBegin******
 *
 * -- CheckBitStuffing -- Shows the next "nbits" of the bitstream
 *
 * Author :		
 *	Luis Ducla Soares (IST) - lds@lx.it.pt
 *
 * Created :		
 *	28-Apr-1997
 *
 * Purpose :		
 *	To see the next "nbits" bitstream bits (byte aligned) without advancing the read
 *	pointer. 
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

Int CheckBitStuffing(Bitstream *stream)
{
  UInt n_stuffed;
  C_UInt byte = 8;
  static UInt bit_stuffing[9] = { 0, 0, 1, 3, 7, 15, 31, 63, 127 };

  n_stuffed = stream->incnt % byte;

  if (n_stuffed == 0)
    n_stuffed = byte;

  if (BitstreamShowBits(stream, n_stuffed) != bit_stuffing[n_stuffed])
    return 0;
  else
    return 1;
}
