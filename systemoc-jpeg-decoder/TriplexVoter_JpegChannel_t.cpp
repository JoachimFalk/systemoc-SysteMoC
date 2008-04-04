// vim: set sw=2 ts=8:

#include "channels.hpp"
#include "TriplexVoter_JpegChannel_t.hpp"


void TriplexVoter::forward_in1() {
    out[0] = in1[0];
    outTimeOut[0] = 1;
}

void TriplexVoter::forward_in2() {
    out[0] = in2[0];
    outTimeOut[0] = 1;
}

void TriplexVoter::failed_action() {
  int i = inTimeOut[0];
  outTimeOut[0] = 1;
}

TriplexVoter::TriplexVoter( sc_module_name name )
     : smoc_actor(name,start)
  {
    start = (in1(1) && in2(1) && in3(1) && (in1.getValueAt(0) == in2.getValueAt(0))) 
         >> (out(1) && outTimeOut(1)) 
         >> CALL(TriplexVoter::forward_in1) 
         >> start
       |    (in1(1) && in2(1) && in3(1) && (in1.getValueAt(0) == in3.getValueAt(0)))
         >> (out(1) && outTimeOut(1)) 
         >> CALL(TriplexVoter::forward_in1) 
         >> start
       |    (in1(1) && in2(1) && in3(1) && (in2.getValueAt(0) == in3.getValueAt(0)))
         >> (out(1) && outTimeOut(1)) 
         >> CALL(TriplexVoter::forward_in2) 
         >> start
       |    (in1(1) && in2(1) && in3(1) && (in1.getValueAt(0) != in2.getValueAt(0)) && 
            (in1.getValueAt(0) != in3.getValueAt(0)) && (in2.getValueAt(0) != in3.getValueAt(0)))
         >> failed
       |    inTimeOut(1)
         >> outTimeOut(1)
         >> CALL(TriplexVoter::failed_action)
         >> timeOut;
     timeOut = (in1(1) && in2(1) && (in1.getValueAt(0) == in2.getValueAt(0)))
            >> (out(1) && outTimeOut(1))
            >> CALL(TriplexVoter::forward_in1)
            >> start
          |    (in1(1) && in2(1) && (in1.getValueAt(0) != in2.getValueAt(0))) 
            >> failed
          |    (in1(1) && in3(1) && (in1.getValueAt(0) == in3.getValueAt(0)))
            >> (out(1) && outTimeOut(1))
            >> CALL(TriplexVoter::forward_in1)
            >> start
          |    (in1(1) && in3(1) && (in1.getValueAt(0) != in3.getValueAt(0))) 
            >> failed
          |    (in2(1) && in3(1) && (in2.getValueAt(0) == in3.getValueAt(0)))
            >> (out(1) && outTimeOut(1))
            >> CALL(TriplexVoter::forward_in2)
            >> start
          |    (in2(1) && in3(1) && (in2.getValueAt(0) != in3.getValueAt(0))) 
            >> failed
          |    inTimeOut(1)
            >> 	failed;

  }

