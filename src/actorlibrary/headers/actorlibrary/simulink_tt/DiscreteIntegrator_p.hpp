/*  Library : Discrete
    Block : Discrete-Time Integrator
    Despcription : Perform discrete-time integration or accumulation of signal
                   You can use the Discrete-Time Integrator block in place of the 
                   Integrator block to create  a purely discrete system.
                   
                   For the Discrete-Time Integrator block, you can:
                   Define initial conditions on the block dialog box or as input to the block.
                   Define an input gain(K) value.
                   Output the block state.
                   Define upper and lower limits on the integral.
                   Reset the state depending on an additional reset input.

    Integration and Accumulation Methods.
    Forward Euler method ( the default ), also known as Forward Rectangular, or left-hand approximation.
    For this method, 1/s is approximated by T/(z-1). The resulting expression for the output of the block at step n is
    y : output
    u : input
    x : state    

    y(n) = y(n-1) + K*T*u(n-1)

    Let x(n+1) = x(n) + K*T*u(n). The block uses the following steps to compute its output:

    Step 0: 	        y(0)   = x(0) = IC (clip if necessary)
			x(1)   = y(0) + K*T*u(0)

    Step 1:		y(1)   = x(1)
			x(2)   = x(1) + K*T*u(1)

    Step n:		y(n)   = x(n)
			x(n+1) = x(n) + K*T*u(n) (clip if necessary)

    With this method, input port 1 does not have direct feedthrough.



    TODO:Backward Euler method. 
    TODO:Trapezoidal method.

 TODO: Zero-Crossing
*/


#ifndef __INCLUDED__DISCRETEINTEGRATOR_P__HPP__
#define __INCLUDED__DISCRETEINTEGRATOR_P__HPP__


#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
//#include <actorlibrary/tt/TT.hpp>

template<typename T>
 class DiscreteIntegrator_p: public smoc_periodic_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  DiscreteIntegrator_p( sc_module_name name, sc_time per, sc_time off, T gain,T sampleTime,   T ic )
    : smoc_periodic_actor(name, start, per, off), gain(gain), sampleTime(sampleTime),  state(ic) {


    start = //Expr::till( this->getEvent() )  >>
      out(1)     >> in (1)     >>
      CALL(DiscreteIntegrator_p::process) >> start
      ;
  }

protected:

  T gain;
  T sampleTime;
  //int step = 0;
  //T ic;
  T state;
  T u;

  void process() {
	 //step++;
	//sampleTime = 0.01;
         //std::cout << "Scope:<" << this->name() << "> " << " @ " << " state:" << state << " gain:" << gain << " sampleTime:" << sampleTime << std::endl;
	//std::cerr << this->name() << " : fired @ " << sc_time_stamp() << std::endl;
	 //this->resetEvent();
         out[0] = state;
         u = in[0];
	 //if( abs((gain*sampleTime)*u) < (T)0.01 )
	//	state = state;
         //else
		//if( (gain*sampleTime)*u > (-0.171622) )
		   state = state + (gain*sampleTime)*u;
		//else
	         //  state = state + (-0.171622);//(-0.063928);
		//state = state + (gain*sampleTime)*u;
        //std::cerr << this->name() << " : Action finished" << std::endl; 
	 //step++;
        
  }

  smoc_firing_state start;
  //smoc_firing_state loop;
};

#endif // __INCLUDED__DISCRETEINTEGRATOR_P__HPP__

