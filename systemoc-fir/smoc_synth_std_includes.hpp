#ifndef INCLUDED_SMOC_SYNTH_STD_INCLUDES
#define INCLUDED_SMOC_SYNTH_STD_INCLUDES

#include <vector>

using namespace std;



namespace initializer{

  vector<double> get_fir_params() {
    vector<double> retval;
    // vector [0,0,1]
    retval.push_back(0);
    retval.push_back(0);
    retval.push_back(1);
    return retval;
  }

  vector<double> get_fir_data() {
    vector<double> retval;
    // vector [0,0,0]
    retval.push_back(0);
    retval.push_back(0);
    retval.push_back(0);
    return retval;
  }
                

};

#endif //INCLUDED_SMOC_SYNTH_STD_INCLUDES
