// vim: set sw=2 ts=8:

#include "channels.hpp"
#include "DuplexVoter_IDCTCoeff_t.hpp"


void DuplexVoter::forward_data() {
    out[0] = in1[0];
    outTimeOut[0] = 1;
  }

void DuplexVoter::failed_action() {
  int i = inTimeOut[0];
  }

DuplexVoter::DuplexVoter( sc_module_name name )
     : smoc_actor(name,start)
  {
    start = (in1(1) && in2(1) && (in1.getValueAt(0) == in2.getValueAt(0))) 
         >> (out(1) && outTimeOut(1)) 
         >> CALL(DuplexVoter::forward_data) 
         >> start
       | inTimeOut(1) >> CALL(DuplexVoter::failed_action) 
         >> failed
       | (in1(1) && in2(1) && (in1.getValueAt(0) != in2.getValueAt(0)))
         >> failed;
  }

