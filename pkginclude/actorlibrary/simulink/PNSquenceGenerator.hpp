/*
 * PNSquenceGenerator.hpp
 *
 *  Created on: Jun 19, 2015
 *      Author: gasmi
 */

#ifndef PKGINCLUDE_ACTORLIBRARY_SIMULINK_PNSQUENCEGENERATOR_HPP_
#define PKGINCLUDE_ACTORLIBRARY_SIMULINK_PNSQUENCEGENERATOR_HPP_
#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>

template<typename DATA_TYPE>
class PNSquenceGenerator: public smoc_actor {
public:
  smoc_port_out<DATA_TYPE>  out;
  //smoc_port_out<std::vector<double> > out[];

  PNSquenceGenerator( sc_module_name name )
    : smoc_actor(name, start)
  {


    start = out(1)     >>
      CALL(PNSquenceGenerator::process) >> start
      ;
  }

protected:

  int32_T idx=0;
  uint8_T tmp=0;
  uint8_T tmp_0=0;
  int8_T rtb_PNSequenceGenerator[88];
  int i=0;
  void process() {

   const uint8_T PNSequenceGenerator_[ ]= { 1U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 1U };
   const uint8_T PNSequenceGenerato_j[] ={ 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U };
   uint8_T shiftReg[15];

     for( idx=0 ; idx< 88 ; idx++){

     tmp= 0U;
     for( int i= 0 ; i< 15; i++){
       tmp=  ( uint8_T ) (PNSequenceGenerator_[ i+1] * shiftReg[i] + tmp);
     }

    tmp&=1 ;
    tmp_0= 0U;


    for( i=0;i<15;15)
      tmp_0=  (uint8_T)((uint32_T)(uint8_T)((uint32_T)shiftReg[i] * PNSequenceGenerato_j[i] )+ tmp_0);



    rtb_PNSequenceGenerator[idx] = (int8_T)(tmp_0 & 1);
           for (i = 13; i >= 0; i += -1) {
            shiftReg[i + 1] = shiftReg[i];

            }
          shiftReg[0U] = tmp;
     }
    out[0] =  rtb_PNSequenceGenerator;
  }

  smoc_firing_state start;
};



#endif /* PKGINCLUDE_ACTORLIBRARY_SIMULINK_PNSQUENCEGENERATOR_HPP_ */
