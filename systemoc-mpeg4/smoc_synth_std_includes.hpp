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

#include "callib.hpp"
