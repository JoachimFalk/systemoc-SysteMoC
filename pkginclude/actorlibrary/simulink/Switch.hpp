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
			smoc_actor(name, start), _threshold(threshold), _criteria(criteria) {

		start = in[0](1) >> in[1](1) >> in[2](1) >> out(1) >> CALL(Switch::process) >> start;
	}

protected:
	T _threshold;
	int _criteria;

	void process() {
		switch (_criteria) {
		case 1:
			if (in[1][0] >= _threshold) {
				out[0] = in[0][0];
			} else {
				out[0] = in[2][0];
			}
			break;
		case 2:
			if (in[1][0] > _threshold) {
				out[0] = in[0][0];
			} else {
				out[0] = in[2][0];
			}
			break;
		case 3:
			if (in[1][0] != 0) {
				out[0] = in[0][0];
			} else {
				out[0] = in[2][0];
			}
			break;
		default:
			// ERROR?
			break;
		}
	}

	smoc_firing_state start;
};

#endif // __INCLUDED__SWITCH__HPP__
