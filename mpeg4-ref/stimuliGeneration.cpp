#include "stimuliGeneration.h"

int StimuliGenerationOpenFiles(){

/* BEGIN ------ Motion Compensation Stimuli Generation ------ */

#ifdef StimuliGenerationMC

	// Open files for Motion compensation stimuli

	if ((StimuliFile_MC_CC2MC_FIFO = fopen(StimuliFileName_MC_CC2MC_FIFO,"w")) == 0){
		printf("Cannot open %s File\n",StimuliFileName_MC_CC2MC_FIFO);
		return 1;
	}

	if ((StimuliFile_MC_VLD2MC_FIFO = fopen(StimuliFileName_MC_VLD2MC_FIFO,"w")) == 0){
		printf("Cannot open %s File\n",StimuliFileName_MC_VLD2MC_FIFO);
		return 1;
	}

	if ((StimuliFile_MC_Buffer_Y = fopen(StimuliFileName_MC_Buffer_Y,"w")) == 0){
		printf("Cannot open %s File\n",StimuliFileName_MC_Buffer_Y);
		return 1;
	}
	if ((StimuliFile_MC_Buffer_U = fopen(StimuliFileName_MC_Buffer_U,"w")) == 0){
		printf("Cannot open %s File\n",StimuliFileName_MC_Buffer_U);
		return 1;
	}
	if ((StimuliFile_MC_Buffer_V = fopen(StimuliFileName_MC_Buffer_V,"w")) == 0){
		printf("Cannot open %s File\n",StimuliFileName_MC_Buffer_V);
		return 1;
	}

	if ((StimuliFile_MC_Comp = fopen(StimuliFileName_MC_Comp,"w")) == 0){
		printf("Cannot open %s File\n",StimuliFileName_MC_Comp);
		return 1;
	}

#endif

/* END ------ Motion Compensation Stimuli Generation ------ */

	// Here add files open procedure for other block

	return 0;
}


int StimuliGenerationCloseFiles(){

/* BEGIN ------ Motion Compensation Stimuli Generation ------ */

#ifdef StimuliGenerationMC
	if (StimuliFile_MC_CC2MC_FIFO)
		fclose(StimuliFile_MC_CC2MC_FIFO);

	if (StimuliFile_MC_VLD2MC_FIFO)
		fclose(StimuliFile_MC_VLD2MC_FIFO);

	if (StimuliFile_MC_Buffer_Y)
		fclose(StimuliFile_MC_Buffer_Y);

	if (StimuliFile_MC_Buffer_U)
		fclose(StimuliFile_MC_Buffer_U);

	if (StimuliFile_MC_Buffer_V)
		fclose(StimuliFile_MC_Buffer_V);

	if (StimuliFile_MC_Comp)
		fclose(StimuliFile_MC_Comp);

#endif

/* END ------ Motion Compensation Stimuli Generation ------ */

  // Here add files close procedure for other block

	return 0;
}
