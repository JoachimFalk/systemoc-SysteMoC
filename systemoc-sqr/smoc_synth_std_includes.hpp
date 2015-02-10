#ifndef INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
#define INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP

#include <cstdlib>
#include <cmath>
#include <cassert>

#include <iostream>

// Maximum (and default) number of Src iterations. Lower default number via
//  command line parameter.
#if defined(SYSTEMC_VERSION) || defined(SQR_LOGGING)
const int NUM_MAX_ITERATIONS = 1000000;
#else
const int NUM_MAX_ITERATIONS = 250000000;
#endif //defined(SYSTEMC_VERSION) || defined(SQR_LOGGING)

#endif // INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
