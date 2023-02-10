/*  Library : Math Operations
 *  TODO: write description
 *
 *  TODO: Zero-Crossing
 */

#ifndef __INCLUDED__SWITCH__HPP__
#define __INCLUDED__SWITCH__HPP__

template<typename T>
class Switch : public smoc_actor {
public:
	smoc_port_in<T> in[3];
	smoc_port_out<T> out;

	Switch(sc_module_name name, T threshold, int criteria) :
			smoc_actor(name, start), _threshold(threshold), _criteria(criteria), val(0.0) {

		start = in[0](1) >> in[1](1) >> in[2](1) >> CALL(Switch::read) >> run;
		run   = out(1) >> CALL(Switch::write) >> start;
	}

protected:
	T _threshold;
	int _criteria;
	T val;

	void write(){
	  out[0] = val;
	}
	
	void read() {

#ifdef _DEBUG	  
	  cout << "Switch " << name()<<endl;
#endif	  
          
	  T in1    = in[0][0];
          T thresh = in[1][0];
          T in2    = in[2][0];

		switch (_criteria) {

		case 1:
			if ( thresh >= _threshold) {
				val = in1;
			} else {
				val = in2;
			}
			break;
		case 2:
			if (thresh > _threshold) {
			        val = in1;
			} else {
				val = in2;
			}
			break;
		case 3:
			if (thresh != 0) {
				val = in1;
			} else {
				val = in2;
			}
			break;
		default:
                        // FIXME
                        val = in1;
			break;
		}
	}

	smoc_firing_state start, run;
};

#endif // __INCLUDED__SWITCH__HPP__

