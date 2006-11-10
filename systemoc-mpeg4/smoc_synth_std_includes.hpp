using namespace std;

#define USE_COUNTER_INPUT

#ifdef XILINX_EDK_RUNTIME
# define USE_COUNTER_INPUT
#endif

#ifndef USE_COUNTER_INPUT
# include <fstream>
# define INAMEblk "test_in.dat"
# define ONAMEblk "test_out.dat"
#endif

#define VERBOSE_TRANSPOSE
#define VERBOSE_IDCT_UPSAMPLE
#define VERBOSE_MIN_DUPLEX
#define VERBOSE_IDCT_SCALE
#define VERBOSE_IDCT_FLY
#define VERBOSE_IDCT_CLIP
#define VERBOSE_IDCT_ADDSUB
#define VERBOSE_IDCT_COL2BLOCK
#define VERBOSE_IDCT_BLOCK2ROW


#include "callib.hpp"
