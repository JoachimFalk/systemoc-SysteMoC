/*
 * DiscretePulseGenerator.hpp
 *
 *  Created on: Jun 17, 2015
 *      Author: gasmi
 */

#ifndef PKGINCLUDE_ACTORLIBRARY_SIMULINK_DISCRETEPULSEGENERATOR_HPP_
#define PKGINCLUDE_ACTORLIBRARY_SIMULINK_DISCRETEPULSEGENERATOR_HPP_


#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_expr.hpp>
#include "SimulinkDataType.hpp"

template<typename T>
 class DiscretePulseGenerator: public smoc_actor {
public:
  smoc_port_out<T>  out;

  DiscretePulseGenerator(sc_module_name name, int32_t period, int32_t phase, int32_t pulsewidth, T Amp)
    : smoc_actor(name, start), period(period), phase(phase), pulsewidth(pulsewidth), Amp(Amp) {
    assert(phase == 0 && "FIXME: Implement phase handling in DiscretePulseGenerator!");



    start =
      out(1)                   >>
      CALL(DiscretePulseGenerator::process) >> start
      ;
  }

protected:
  int32_t period;
  int32_t phase;
  int32_t pulsewidth;
  T       Amp;
  int32_t clockTickCounter =0;
  real_T rtb_PulseGenerator =0;

  void process() {



    rtb_PulseGenerator= ( clockTickCounter < pulsewidth)&& ( clockTickCounter  >=0 )?  Amp : 0.0;

    if (clockTickCounter >= period - 1.0)

     clockTickCounter=0;
   else
     clockTickCounter++;


     out[0] =  rtb_PulseGenerator;

  }

  smoc_firing_state start;
};

#endif /* PKGINCLUDE_ACTORLIBRARY_SIMULINK_DISCRETEPULSEGENERATOR_HPP_ */
