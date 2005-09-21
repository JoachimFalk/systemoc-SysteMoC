#ifndef __StimuliGenerationGlobals_h
#define __StimuliGenerationGlobals_h

// Here include your global variables (e.g., Files Declaration)(for every block, use ifdef)

/* BEGIN ------ Motion Compensation Stimuli Generation ------ */

#ifdef StimuliGenerationMC

FILE * StimuliFile_MC_CC2MC_FIFO = NULL;
char * StimuliFileName_MC_CC2MC_FIFO = "MC_CC2MC_FIFO.txt";

FILE * StimuliFile_MC_VLD2MC_FIFO = NULL;
char * StimuliFileName_MC_VLD2MC_FIFO = "MC_VLD2MC_FIFO.txt";

FILE * StimuliFile_MC_Buffer_Y = NULL;
char * StimuliFileName_MC_Buffer_Y = "MC_Buffer_Y.txt";

FILE * StimuliFile_MC_Buffer_U = NULL;
char * StimuliFileName_MC_Buffer_U = "MC_Buffer_U.txt";

FILE * StimuliFile_MC_Buffer_V = NULL;
char * StimuliFileName_MC_Buffer_V = "MC_Buffer_V.txt";

FILE * StimuliFile_MC_Comp = NULL;
char * StimuliFileName_MC_Comp = "MC_Compensated.txt";

int StimuliMC_cnt = 0;
int StimuliMC_skip_condition = 0;
int StimuliMC_frame_cnt = 0;

#endif

/* END ------ Motion Compensation Stimuli Generation ------ */

#endif //__StimuliGeneration_h