/*  Library : Math Operations
 *  TODO: write description
 *
 *  TODO: Zero-Crossing
 */

#ifndef __INCLUDED__SWITCH__HPP__
#define __INCLUDED__SWITCH__HPP__

#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>

template<typename T1, typename T2>
  class Switch: public smoc_actor {
  public:
    smoc_port_in<T1>  in[2];
    smoc_port_out<T1> out;
    smoc_port_in<T2>  inCtrl;

    Switch(sc_module_name name, T2 threshold, int criteria)
        : smoc_actor(name, start), _threshold(threshold), _criteria(criteria) {
      start = in[0](1) >> in[1](1) >> inCtrl(1) >> out(1) >> CALL(Switch::process) >> start;
    }

  protected:
    smoc_firing_state start;

    T2  _threshold;
    int _criteria;

    void process() {
#ifdef _DEBUG	  
      cout << "Switch " << name() << endl;
#endif	  
      switch (_criteria) {
        case 1:
          if (inCtrl[0] >= _threshold) {
            out[0] = in[0][0];
          } else {
            out[0] = in[1][0];
          }
          break;
        case 2:
          if (inCtrl[0] > _threshold) {
            out[0] = in[0][0];
          } else {
            out[0] = in[1][0];
          }
          break;
        case 3:
          if (inCtrl[0] != 0) {
            out[0] = in[0][0];
          } else {
            out[0] = in[1][0];
          }
          break;
        default:
          assert(!"Oops! Undefined _criteria value!");
          break;
      }
    }
  };

#endif // __INCLUDED__SWITCH__HPP__

