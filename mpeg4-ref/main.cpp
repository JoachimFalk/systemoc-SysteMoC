/*
 *  Author: Kristof Denolf and ATOMIUM 
 *  Based on MPEG-4 reference code
 *  Date: November 21st 2001
 *  IMEC
 */

#include <time.h>
#include <string.h>

#ifndef __AllInOne_h
#include "AllInOne.h"
#endif

#ifdef USE_DISPLAY
#include "picture.h"
#endif

#include "stimuliGeneration.h"
#include "stimuliGenerationGlobals.h"

// 
// Global variables
//
// for test and analysis only
 int framenum;
#ifdef VERBOSE
 int iframes, pframes; 
 int partitioning_used;
 int vol_header_bits, vop_header_bits;
 int vp_header_bits, mb_header_bits;
 int motion_bits, texture_bits;
 int motion_blocks, texture_blocks;
 int motion_bits_prev, texture_bits_prev;
 int motion_blocks_prev, texture_blocks_prev;
 short mvx_max, mvy_max;
 unsigned long parser_cycles;
#endif
// filenames and file pointers
 char bitfile[MAX_STREAMS][300];
 char outfile[300];
 char outfileU[300];
 char outfileV[300];
 int resync_errors;
#ifdef WRITE_FILE
 File *yuv_file, *u_file, *v_file;
#endif
#ifdef COMPARE_FILE
 File *ref_file;
 char reffile[300];
 int frames_in_error;
#endif
#ifdef WRITE_INST_FILE
 File *dct_file1, *dct_file2;
 File *mot_file1, *mot_file2;
#endif

//
// Register files
//
// NOTE: these are accessible by the ParserVLD HW block
// as well as either the CopyControl, MotionComp or TextureIDCT HW block
 unsigned int dec_regfile[MAX_STREAMS];
 unsigned int mc_regfile[1];
 unsigned int idct_regfile[1];


//
// Function Prototypes
//
Vol *readControlFile(FILE *file, unsigned char VOnumber, unsigned char singleStream);
static void ReadControlFiles(int argc, Char *argv[], Vol *VOlist[], float *framerate, int *dwidth, int *dheight, unsigned short *streams, int *ulx_display, int *uly_display);
void closeOutputFiles(Vol *VOlist[],unsigned char totalNumberOfVOs);

/***********************************************************CommentBegin******
 *
 * -- main -- Decodes an MPEG4 bitstream and composes the decoded VOPs
 *
 * Author :		
 *	Michael Wollborn (TUH)
 *
 * Created :		
 *	08-AUG-1996
 *
 * Purpose :		
 *	Handle the decoding of the MPEG-4 bitstream, and compose the decoded
 *	VOPs into a sequence with a given display rate, width and height.
 * 
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 * Converted to testbench function by P. Schumacher, Xilinx, 2004/01/09
 * Updated for harmonization with encoder: Kristof Denolf (IMEC), April 22nd 2004
 *
 ***********************************************************CommentEnd********/

int main(int argc, char *argv[])
{ 
  int dwidth, dheight;
	unsigned short streams;
	unsigned char err_flag=0;
  float framerate;
	unsigned long bitcount;
	clock_t tstart, tfinish;
  double duration;
	int ulx_display[MAX_STREAMS];
	int uly_display[MAX_STREAMS];

	Vol *VOlist[MAX_STREAMS];

	// Initialize global variables used
	// in SW instrumenting only
#ifdef VERBOSE
	iframes=0; pframes=0; framenum=0;
	partitioning_used=0;
	vol_header_bits=0; vop_header_bits=0;
	vp_header_bits=0; mb_header_bits=0;
	motion_bits=0; texture_bits=0;
	motion_blocks=0; texture_blocks=0;
	motion_bits_prev=0; texture_bits_prev=0;
	motion_blocks_prev=0; texture_blocks_prev=0;
	mvx_max=0; mvy_max=0; parser_cycles=0;
#endif
	resync_errors=0; bitcount=0;

	//////////////////////////
	// Reading control file //
	//////////////////////////
  ReadControlFiles(argc, argv, VOlist, &framerate, &dwidth, &dheight, &streams, ulx_display, uly_display);

#ifdef COMPARE_FILE
	strcpy(reffile, outfile);
	strrepl(reffile, 300, "data_dec_test", "data_dec_refer");
        strcpy( reffile, "ref_foreman_qcif_30.yuv" ); // DBP
	if ((ref_file = fopen(reffile, "rb")) == NULL)
		printf("Warning: cannot open reference file\n");
	frames_in_error = 0;
#endif

#ifdef WRITE_INST_FILE
	dct_file1 = fopen("..\\instrument\\texture_blocks.dat", "w");
	dct_file2 = fopen("..\\instrument\\texture_bits.dat", "w");
	mot_file1 = fopen("..\\instrument\\motion_blocks.dat", "w");
	mot_file2 = fopen("..\\instrument\\motion_bits.dat", "w");
#endif

	if (StimuliGenerationOpenFiles())
		FatalError("Cannot Open Stimuli Files\n");

	tstart = clock();

	//////////////////
	// Decoder Chip //
	//////////////////
	DecoderChip(VOlist,dwidth, dheight, streams, ulx_display, uly_display);

	//////////////////////
	// Print statistics //
	//////////////////////
	tfinish = clock();
	duration = (double)(tfinish - tstart) / CLOCKS_PER_SEC;
	printf("\nDecoding Performance:\n  %d frames in %.2f seconds (%.2f fps)\n\n",
	        framenum, duration, (double)framenum/duration);
#ifdef VERBOSE
	bitcount = vol_header_bits + vop_header_bits + vp_header_bits + mb_header_bits + motion_bits + texture_bits;
	printf("Statistics:\n-----------\n");
	printf("  Input Bitrate: %.0f bits/sec\n", bitcount*framerate/framenum);
	printf("  I frames = %d   P frames = %d\n",iframes,pframes);
	if (partitioning_used)
		printf("  Data partitioning used\n");
	else
		printf("  No data partitioning used\n");
	printf("  Maximum motion vectors (abs): X = %.1f, Y = %.1f\n",mvx_max/2.0,mvy_max/2.0);
	printf("  8x8 Block allocation: (1 MB = six 8x8 Blocks)\n");
	printf("    Motion Comp:    %8lu\n", motion_blocks);
	printf("    Texture/DCT:    %8lu\n", texture_blocks);
	printf("  Bitstream allocations: (in bits)\n");
	printf("    VOL header:     %8lu\n", vol_header_bits);
	printf("    VOP headers:    %8lu\n", vop_header_bits);
	printf("    Packet headers: %8lu\n", vp_header_bits);
	printf("    MB headers:     %8lu\n", mb_header_bits);
	printf("    Motion vectors: %8lu\n", motion_bits);
	printf("    Texture/DCT:    %8lu\n", texture_bits);
	printf("                    --------\n");
	printf("    Total:          %8lu\n", bitcount);
	printf("    Uncompressed: %10lu\n\n",(unsigned long) (1.5*dwidth*dheight*framenum*BITS_PIXEL));
	printf("  Parser/VLD HW Analysis (estimated):\n");
	printf("    Assumed clock freq.   = %d MHz\n", PARSER_CLOCK);
	printf("    Total HW Clock Cycles = %d cycles\n", parser_cycles);
        printf("    Total processing time = %.1f msec\n", 1.0 * parser_cycles / PARSER_CLOCK);
	printf("    Max. throughput       = %.1f Mbits/sec\n\n", 1.0 * bitcount * PARSER_CLOCK / parser_cycles);
	printf("  Resync errors: %d\n", resync_errors);
#endif

#ifdef WRITE_FILE // close files
	closeOutputFiles(VOlist,(unsigned char)streams);
#endif

#ifdef COMPARE_FILE
	if (ref_file != NULL && streams == 1)
	{
		printf("    Frames in error: %d\n",frames_in_error);
		fclose(ref_file);
	}
#endif

#ifdef WRITE_INST_FILE
	if (dct_file1) fclose(dct_file1); 
	if (dct_file2) fclose(dct_file2);
	if (mot_file1) fclose(mot_file1); 
	if (mot_file2) fclose(mot_file2);
#endif

	if (StimuliGenerationCloseFiles())
		FatalError("Cannot Open Stimuli Files\n");

	exit(0);
} // main


/***********************************************************CommentBegin******
 *
 * -- ReadControlFile -- Gets from the user needed arguments for 
 *			     decoding & display  
 *
 * Author :		
 *	Sylvie Jeannin (LEP)
 *
 * Created :		
 *	11-Mar-96
 *
 * Purpose :		
 *	Get from the user needed arguments for decoding & display.
 *
 * PI Optimised and cleaned: Kristof Denolf (IMEC), November 28th, 2001
 *
 ***********************************************************CommentEnd********/

Vol *readControlFile(FILE *file, unsigned char VOnumber, unsigned char singleStream)
{
	char string[300], garbage[80];
	
	Vol *currVol;
	FILE *tmpFile = 0;
	int i;
	float f;
	
	currVol = (Vol *) malloc(sizeof(Vol));

	if (!singleStream)
	{
		fscanf(file, "%d", &i);
		fgets(garbage, 80, file);
		if (i != 1)
			FatalError("only 1 VO supported");
	}

	fscanf(file, "%d", &i);
	fgets(garbage, 80, file);
	if (i != 1)
		FatalError("only 1 VOL supported");

	fscanf(file, "%s", bitfile[VOnumber]);
	fgets(garbage, 80, file);

	// volfile & govfile
	fscanf(file, "%s", string);
	fgets(garbage, 80, file);
	fscanf(file, "%s", string);
	fgets(garbage, 80, file);
 
	// error resilience flag (not used)
	fscanf(file, "%d", &i); 
	fgets(garbage, 80, file);

	// filenames, framerate, and width/height (ignored)
#ifdef WRITE_FILE
	fscanf(file, "%s", string); // Y file
	fgets(garbage, 80, file); 

	if ((tmpFile = fopen(string,"wb")) == 0) 
		FatalError("could not open output file");
	else
		currVol->y_file = tmpFile;

	if (string[strlen(string)-1] == 'y') // separate Y, U, V files
	{
		fscanf(file, "%s", string); // U file
		fgets(garbage, 80, file);

		if ((tmpFile = fopen(string,"wb")) == 0) 
			FatalError("could not open output file");
		else
			currVol->u_file = tmpFile;

		fscanf(file, "%s", string); // V file
		fgets(garbage, 80, file);

		if ((tmpFile = fopen(string,"wb")) == 0) 
			FatalError("could not open output file");
		else
			currVol->v_file = tmpFile;
	}
	else
	{
		fscanf(file, "%s", string);
		fgets(garbage, 80, file); // U file
		fscanf(file, "%s", string);
		fgets(garbage, 80, file); // V file
		currVol->u_file = 0;
		currVol->v_file = 0;
	}
			
	fscanf(file, "%s", string);
	fgets(garbage, 80, file); // A file (ignored)
#else // ignore all
	fscanf(file, "%s", string);
	fgets(garbage, 80, file); // Y file
	fscanf(file, "%s", string);
	fgets(garbage, 80, file); // U file
	fscanf(file, "%s", string);
	fgets(garbage, 80, file); // V file
	fscanf(file, "%s", string);
	fgets(garbage, 80, file); // A file 
#endif	
	fscanf(file, "%f", &f); // framerate
	fgets(garbage, 80, file); 
	currVol->framerate = f;
			
	fscanf(file, "%d", &i); // display width
	fgets(garbage, 80, file);
	currVol->displayWidth = i;

	fscanf(file, "%d", &i); // display height
	fgets(garbage, 80, file);
	currVol->displayHeight = i;
			
	// filter type (not used)
	fgets(garbage, 80, file);
			
	// bits per pixel (ignored)
	fgets(garbage, 80, file);
			
	// random access start time (not used)
	fgets(garbage, 80, file);

	// VTCs (not supported)
	fscanf(file, "%d", &i);
	fgets(garbage, 80, file);
	if (i != 0)
		FatalError("no vtc support");

	return currVol;
}

static void ReadControlFiles(int argc, Char *argv[], Vol *VOlist[], float *framerate, int *dwidth, int *dheight, 
														unsigned short *streams, int *ulx_display, int *uly_display)
{
  char cfgfile[300], string[300], garbage[80];
  File *fd, *bd;
	char ctlfile[300];
  int code;
	static unsigned char VOnumber = 0;
	unsigned char j;

  if (argc == 2)
    strcpy(cfgfile, argv[1]);
  else
    FatalError("Usage: mpeg4_decoder.exe control_file");

  if ((fd = fopen(cfgfile, "r")) == NULL)
    FatalError("Control file not found");
  
	// Check first line
  fscanf(fd, "%d", &code);
  fgets(garbage, 80, fd);

	if (code != 1 && code != 77)
	{
		fclose(fd);
		FatalError("incorrect control file format");
	}

	if (code == 77) // multiple streams
	{
		// # of streams
		fscanf(fd, "%d", streams);
		fgets(garbage, 80, fd);

		if (*streams > MAX_STREAMS)
			FatalError("Too many streams");

		// individual control filenames
		for (j=0; j < *streams; j++)
		{			
			fscanf(fd, "%s", ctlfile);
			fgets(garbage, 80, fd);

			if ((bd = fopen(ctlfile, "r")) == NULL)
				FatalError("Control file for stream not found");
			
			VOlist[j] = readControlFile(bd,j,0);

			fclose(bd);
		}

		// output YUV filename
		fscanf(fd, "%s", string);
		fgets(garbage, 80, fd);

		// framerate
		fscanf(fd, "%f", framerate);
		fgets(garbage, 80, fd);

		// Display width and height
		fscanf(fd, "%d", dwidth);
		fgets(garbage, 80, fd);
		fscanf(fd, "%d", dheight);
		fgets(garbage, 80, fd);

		// Upper-left corner of displays
		//for (i=0; i < *streams; i++) {
		//	fscanf(fd, "%d %d", ulx_display+i, uly_display+i);
			//printf("(%d, %d)\n", ulx_display[i], uly_display[i]);
			//fgets(garbage, 80, fd);
		//}
	}
	else
	{ 
		*streams = 1;
		*ulx_display = 0;
		*uly_display = 0;
		
		VOlist[0] = readControlFile(fd,0,1);
		*dwidth = VOlist[0]->displayWidth;
		*dheight = VOlist[0]->displayHeight;
		*framerate = VOlist[0]->framerate;
	}
} // ReadControlFile


void closeOutputFiles(Vol *VOlist[],unsigned char totalNumberOfVOs)
{
	unsigned char voNumber;
	Vol *vol;

	for (voNumber = 0; voNumber < totalNumberOfVOs; voNumber++)
	{
		vol = VOlist[voNumber];
		//fclose(GetVolBitstream(vol));
		
		fclose(vol->y_file);
		if(vol->u_file)
			fclose(vol->u_file); 
		if (vol->v_file)
			fclose(vol->v_file); 
	}
}

///////////////
// Utilities //
///////////////

//
// Fatal Error
//
void FatalError(char *string)
{
#ifdef VERBOSE
	fprintf(stderr,"ERROR: %s\n",string);
#endif
#ifdef STOP_ON_ERROR
	exit(1);
#endif
	return;
}


// 
// strrepl: Replace OldStr by NewStr in string Str contained in buffer
// of size BufSiz.
//
char *strrepl(char *Str, unsigned short BufSiz, char *OldStr, char *NewStr)
{
	int OldLen, NewLen;
	char *p, *q;

    if(NULL == (p = strstr(Str, OldStr)))
      return Str;
    OldLen = strlen(OldStr);
    NewLen = strlen(NewStr);
    if ((strlen(Str) + NewLen - OldLen + 1) > BufSiz)
      return NULL;
    memmove(q = p+NewLen, p+OldLen, strlen(p+OldLen)+1);
    memcpy(p, NewStr, NewLen);
    return q;
}
