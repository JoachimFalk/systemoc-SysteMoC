/*
 * Concatenate.hpp
 *
 *  Created on: Jun 22, 2015
 *      Author: gasmi
 */

#ifndef PKGINCLUDE_ACTORLIBRARY_SIMULINK_CONCATENATE_HPP_
#define PKGINCLUDE_ACTORLIBRARY_SIMULINK_CONCATENATE_HPP_


#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_expr.hpp>
#include "SimulinkDataType.hpp"

template<typename DATA_TYPE>
class Concatenate: public smoc_actor {
public:
  smoc_port_out<DATA_TYPE>  out;
  smoc_port_out<DATA_TYPE>  in;
  //smoc_port_out<std::vector<double> > out[];

  Concatenate( sc_module_name name, DATA_TYPE constValue )
    : smoc_actor(name, start)
  {
    SMOC_REGISTER_CPARAM(constValue);

    start = out(1)     >>
      CALL(Concatenate::process) >> start
      ;
  }

protected:
  DATA_TYPE constValue;

  void process() {

   double constValue [] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  // double AssembleSubcarriers [88];
   memcpy(&in[88], &constValue[0], sizeof(real_T) <<
            3U);

   memcpy(&out[0], &in[0], 96U * sizeof(real_T));


  }

  smoc_firing_state start;
};



#endif /* PKGINCLUDE_ACTORLIBRARY_SIMULINK_CONCATENATE_HPP_ */
