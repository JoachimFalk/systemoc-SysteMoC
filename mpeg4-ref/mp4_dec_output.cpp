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

#ifdef COMPARE_FILE
#include <math.h>
#endif

/***********************************************************CommentBegin******
 *
 * -- WriteVopRAW --
 *
 * Author:
 *      K. Denolf (IMEC)
 *
 * Created:
 *      April 21st 2004
 *
 * Description:
 *      Write the YUV data from the Vop to the associated files.
 *
 *
 ***********************************************************CommentEnd********/

void WriteVopRaw(FILE *y_file, FILE *u_file, FILE *v_file, unsigned char *recFrame, int width, int height, unsigned int frameMemoryOffset) 
{ 
	int height_uv = height/2; 
	int width_uv = width /2; 
	unsigned char *data; 
  char datum; 
  int i, j;

	int MBwidth = width/16;
	int MBheight = height/16;

#ifdef COMPARE_FILE
	char error_code=0, ref_datum;
#endif
	 
  data = recFrame + frameMemoryOffset; 
 
  if (BITS_PIXEL <= 8) 
    for (j = 0; j < height; j++) 
      for (i = 0; i < width; i++) 
      { 
        datum = (char) data[((j/16)*MBwidth + (i/16))*256 + ((j&15)/8)*128 + ((i&15)/8)*64 + (j&7)*8 + (i&7)];

				#ifdef WRITE_FILE	
        fwrite(&datum, 1, 1, y_file); 
				#endif

				#ifdef COMPARE_FILE
				if (ref_file)
				{
					fread(&ref_datum, 1, 1, ref_file);
					if (datum != ref_datum) error_code |= 4;
				}
				#endif
      } 
     
  data += width*height; 
 
  if (u_file == 0)
	{
		for (j = 0; j < height_uv; j++) 
			for (i = 0; i < width_uv; i++) 
			{ 
				datum = (char) data[((j/8)*MBwidth + (i/8))*64 + (j&7)*8 + (i&7)]; 

				#ifdef WRITE_FILE	
				fwrite(&datum, 1, 1, y_file);
				#endif

				#ifdef COMPARE_FILE
				if (ref_file)
				{
					fread(&ref_datum, 1, 1, ref_file);
					if (datum != ref_datum) error_code |= 2;
				}
				#endif
			}
	}
	else
	{
		#ifdef WRITE_FILE	
		for (j = 0; j < height_uv; j++) 
			for (i = 0; i < width_uv; i++) 
			{ 
				datum = (char) data[((j/8)*MBwidth + (i/8))*64 + (j&7)*8 + (i&7)]; 
				fwrite(&datum, 1, 1, u_file);	
				
			}
		#endif
	}
 
  data += width*height/4; 
 
  if (v_file == 0)
	{
		for (j = 0; j < height_uv; j++) 
			for (i = 0; i < width_uv; i++) 
			{ 
				datum = (char) data[((j/8)*MBwidth + (i/8))*64 + (j&7)*8 + (i&7)]; 

				#ifdef WRITE_FILE	
				fwrite(&datum, 1, 1, y_file);	
				#endif

				#ifdef COMPARE_FILE
				if (ref_file)
				{
					fread(&ref_datum, 1, 1, ref_file);
					if (datum != ref_datum) error_code |= 1;
				}
				#endif
			}
	}
	else
	{
		#ifdef WRITE_FILE	
		for (j = 0; j < height_uv; j++) 
			for (i = 0; i < width_uv; i++) 
			{ 
				datum = (char) data[((j/8)*MBwidth + (i/8))*64 + (j&7)*8 + (i&7)]; 
				fwrite(&datum, 1, 1, v_file);				
			}
		#endif
	} 			

#ifdef COMPARE_FILE
	if (error_code)
	{
		printf("  Error(s) found in frame!! (code: %d)\n", error_code);
		frames_in_error++;
	}
#endif
}

