#ifndef __StimuliGeneration_h
#define __StimuliGeneration_h

// Here Includes
#include <stdlib.h>
#include <stdio.h>

// Here include your defines (for every block)
// Naming convention <StimuliGeneration><block>
//#define StimuliGenerationMC 1


// Here specify your extern global variables (e.g., Files Declaration)(for every block, use ifdef)
// Naming convention files :<StimuliFile>_<block>_<fileName>
// Naming convention others:<Stimuli><block>_<purpose>


/* BEGIN ------ Motion Compensation Stimuli Generation ------ */

#ifdef StimuliGenerationMC

extern FILE * StimuliFile_MC_CC2MC_FIFO;
extern char * StimuliFileName_MC_CC2MC_FIFO;

extern FILE * StimuliFile_MC_VLD2MC_FIFO;
extern char * StimuliFileName_MC_VLD2MC_FIFO;

extern FILE * StimuliFile_MC_Buffer_Y;
extern char * StimuliFileName_MC_Buffer_Y;

extern FILE * StimuliFile_MC_Buffer_U;
extern char * StimuliFileName_MC_Buffer_U;

extern FILE * StimuliFile_MC_Buffer_V;
extern char * StimuliFileName_MC_Buffer_V;

extern FILE * StimuliFile_MC_Comp;
extern char * StimuliFileName_MC_Comp;

extern int StimuliMC_cnt;
extern int StimuliMC_frame_cnt;
extern int StimuliMC_skip_condition;

#endif


// Here include your macros (read/write monitoring)(for every block, use ifdef)
#ifdef StimuliGenerationMC
  #define StimuliGenerationMC_WriteData(file,data) if (StimuliMC_skip_condition) fprintf(file,"%d\n",data);
  #define StimuliGenerationMC_WriteSharedMemData(file,opp,addr,data) if (StimuliMC_skip_condition) fprintf(file,"%d %d %d\n",opp,addr,data);
  #define StimuliGenerationMC_WriteObjectFifo(stmfile,buffer,objsize,mode) if (StimuliMC_skip_condition) for(StimuliMC_cnt = 0; StimuliMC_cnt < objsize; StimuliMC_cnt++) if (mode == 0) fprintf(stmfile,"0\n"); else fprintf(stmfile,"%d\n",buffer[StimuliMC_cnt]&0x1ff);
  #define UpdateMCSkipCondition(condition) StimuliMC_frame_cnt++; if (condition) StimuliMC_skip_condition = 1; else StimuliMC_skip_condition = 0;
#else
  #define StimuliGenerationMC_WriteData(file,data)
  #define StimuliGenerationMC_WriteSharedMemData(file,opp,addr,data)
  #define StimuliGenerationMC_WriteObjectFifo(stmfile,buffer,objsize,mode)
  #define UpdateMCSkipCondition(condition)
#endif

/* END ------ Motion Compensation Stimuli Generation ------ */

int StimuliGenerationOpenFiles();
int StimuliGenerationCloseFiles();


#endif //__StimuliGenerationExtern_h