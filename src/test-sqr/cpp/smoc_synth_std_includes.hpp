#ifndef INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
#define INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP

#include <cmath>
#include <cassert>

// NOTE: Forte cynthesizer does not support includes of stdlib.h and stdio.h due to missing
// overrides in $CYNTH_HOME/include/std !
#include <cstdlib>
#include <cstdio>

#include <iostream>

#if defined(SYSTEMC_VERSION) || defined(SYNTHESIS_FRAMEWORK_VERSION)
# define SQR_LOGGING 1
#endif //defined(SYSTEMC_VERSION) || defined(SYNTHESIS_FRAMEWORK_VERSION)

// Maximum (and default) number of Src iterations. Lower default number via
//  command line parameter.
#ifdef SQR_LOGGING
const int NUM_MAX_ITERATIONS = 1000000;
#else //!defined(SQR_LOGGING)
const int NUM_MAX_ITERATIONS = 250000000;
#endif //!defined(SQR_LOGGING)

#endif // INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
