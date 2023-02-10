/*  Library : Math Operations
    Block : Abs
    Despcription : Output absolute value of input

 TODO: Zero-Crossing
*/

#ifndef __INCLUDED__ABS__HPP__
#define __INCLUDED__ABS__HPP__

#include <cstdlib>
#include <cmath>

template<typename T>
class Abs: public smoc_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T> out;

  Abs(sc_module_name name): smoc_actor(name, start) {
    start = in(1)        >>
      out(1)             >>
      CALL(Abs::process) >> start
      ;
  }

protected:
  void process() {
    out[0] = abs(in[0]);
  }

  smoc_firing_state start;
};

template<>
class Abs<float>: public smoc_actor {
public:
  smoc_port_in<float>  in;
  smoc_port_out<float> out;

  Abs(sc_module_name name): smoc_actor(name, start) {
    start = in(1)        >>
      out(1)             >>
      CALL(Abs::process) >> start
      ;
  }

protected:
  void process() {
    out[0] = fabsf(in[0]);
  }

  smoc_firing_state start;
};

template<>
class Abs<double>: public smoc_actor {
public:
  smoc_port_in<double>  in;
  smoc_port_out<double> out;

  Abs(sc_module_name name): smoc_actor(name, start) {
    start = in(1)        >>
      out(1)             >>
      CALL(Abs::process) >> start
      ;
  }

protected:
  void process() {
    out[0] = fabs(in[0]);
  }

  smoc_firing_state start;
};

template<>
class Abs<long double>: public smoc_actor {
public:
  smoc_port_in<long double>  in;
  smoc_port_out<long double> out;

  Abs(sc_module_name name): smoc_actor(name, start) {
    start = in(1)        >>
      out(1)             >>
      CALL(Abs::process) >> start
      ;
  }

protected:
  void process() {
    out[0] = fabsl(in[0]);
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__ABS__HPP__
