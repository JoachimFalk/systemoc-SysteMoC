/*
 
 * Block : Comapre To Zero
 * 
 * Determine how signal compares to zero
 * Library : Logic and Bit Operations

 * The Compare To Zero block compares an input signal to zero. 
 * Specify how the input is compared to zero with the Operator parameter.
 * The Operator parameter can have the following values:
 * ==   whether the input is equal to zero
 * ~=   not equal to zero
 * <    less than zero
 * <=   less than or equal to zero
 * >    greater than zero
 * >=   greater than or equal to zero
 * The output is 0 if the comparsion is false, and 1 if ti is true. 

 */

#ifndef __INCLUDED__COMPARETOX__HPP__
#define __INCLUDED__COMPARETOX__HPP__


#include <cstdlib>
#include <cassert>
#include <iostream>
#include <systemoc/smoc_moc.hpp>


/* 
 * Integer logicOperator = 0 means operator = "=="
 * Integer logicOperator = 1 means operator = "~="
 * Integer logicOperator = 2 means operator = "<"
 * Integer logicOperator = 3 means operator = "<="
 * Integer logicOperator = 4 means operator = ">"
 * Integer logicOperator = 5 means operator = ">="
 */
template<typename T>
class CompareToX: public smoc_actor {
public:
  smoc_port_in<T>   in;
  smoc_port_out<T>  out;	
  
  CompareToX( sc_module_name name, int logicOperator, T x)
    : smoc_actor(name, start), logicOperator(logicOperator), x(x)
  {
    SMOC_REGISTER_CPARAM(logicOperator);
    SMOC_REGISTER_CPARAM(x);

    start =
        out(1) >> in (1) >> CALL(CompareToX::process) >> start
      ;
  }
protected:
  int logicOperator;
  T   x;

  void process() {   
    T data = in[0];
    T outData = 0;
    	    
    //std::cout << "CompareToX> get: " << data << " @ " << sc_time_stamp() << std::endl;

    switch (logicOperator) {
      case 0: /* == */
        if (data == x)
          outData = 1;
        break;
      case 1: /* ~= */
        if (data != x)
          outData = 1;
        break;
      case 2: /* < */
        if (data < x)
          outData = 1;
        break;
      case 3: /* <= */
        if (data <= x)
          outData = 1;
        break;
      case 4: /* > */
        if (data > x)
          outData = 1;
        break;
      case 5: /* >= */
        if (data >= x)
          outData = 1;
        break;
      default:
        assert(!"WTF?!");
    }
    out[0] = (T)outData;
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__COMPARETOX__HPP__

