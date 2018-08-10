/*  Library : Math Operations
 Block : Signum
 Despcription : Indicate sign of input

 Description : The Sign block indicates the sign of the input
 The output is 1 when the input is greater than zero
 The output is 0 when the input is equal to zero
 Then output is -1 when the input is less than zero

 TODO:Zero-Crossing
 One:to detect when the input crosses through zero.
 */

#ifndef __INCLUDED__MATH__HPP__
#define __INCLUDED__MATH__HPP__

#include <cmath>

template<typename T>
class Math: public smoc_actor {
public:
	smoc_port_in<T> in;
	smoc_port_out<T> out;

	Math(sc_module_name name, int mathOperator) :
			smoc_actor(name, start), _mathOperator(mathOperator) {

		start = in(1) >> out(1) >> CALL(Math::process) >> start;
	}

protected:

	int _mathOperator;

	void process() {
#ifdef _DEBUG	  
		cout << name() ;
#endif
	  
		T inV = in[0];
		T outV = 0.0;
	  
		switch (_mathOperator) {
		case 1: // "sqrt"
			outV = std::sqrt(inV);
			break;
		case 2: // "exp"
			outV = std::exp(inV);
			break;
		default:
			// FIXME			
			break;
		}
		out[0] = outV;
#ifdef _DEBUG		
		cout << " inV " << inV << " outV " << outV << " ->" << endl;
#endif		
	}

	smoc_firing_state start;
};

#endif // __INCLUDED__MATH__HPP__
