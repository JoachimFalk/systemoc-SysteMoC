/*
 * TODO: Write Description


 TODO:Backward Euler method.
 TODO:Trapezoidal method.

 TODO: Zero-Crossing
 */

#ifndef __INCLUDED__DISCRETEINTEGRATOR__HPP__
#define __INCLUDED__DISCRETEINTEGRATOR__HPP__

template<typename T>
class DiscreteIntegrator: public smoc_actor {
public:
	smoc_port_in<T> in;
	smoc_port_out<T> out;

	DiscreteIntegrator(sc_module_name name, T gain, T sampleTime, T ic) :
			smoc_actor(name, start), gain(gain), sampleTime(sampleTime), ic(ic), state(0.0) {

		start = in(1) >> out(1) >> CALL(DiscreteIntegrator::process) >> start;
	}

protected:

	T gain;
	T sampleTime;
	//int step = 0;
	T ic;
	T state;

	void process() {
		//step++;
		out[0] = (in[0] - state) * gain / sampleTime;
		state = in[0];
		step++;
	}

	smoc_firing_state start;
	smoc_firing_state loop;
};

#endif // __INCLUDED__DISCRETEINTEGRATOR__HPP__
