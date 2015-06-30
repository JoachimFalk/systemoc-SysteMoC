/*
 * Ground.hpp
 *
 *  Created on: Jun 18, 2015
 *      Author: gasmi
 */

#ifndef PKGINCLUDE_ACTORLIBRARY_SIMULINK_GROUND_HPP_
#define PKGINCLUDE_ACTORLIBRARY_SIMULINK_GROUND_HPP_

#include <systemoc/smoc_moc.hpp>
//#include <systemoc/smoc_tt.hpp>

template<typename T>
 class Ground: public smoc_actor {
public:
  smoc_port_out<T>  out;

  Ground(sc_module_name name, size_t samplesPerPeriod = 1)
    : smoc_actor(name, start), samplesPerPeriod(samplesPerPeriod)
  {
    start =
        out(1) >> CALL(Ground::process) >> start
      ;
  }

protected:

  size_t samplesPerPeriod;

  template <typename TT>
  struct MyOutput;

  void process() {
     out[0] = MyOutput<T>::computeOutput(samplesPerPeriod);
  }

  smoc_firing_state start;
};

template<typename T>
template<typename TT>
struct Ground<T>::MyOutput {

  static
  TT computeOutput(size_t samplesPerPeriod) {
    assert(samplesPerPeriod == 1);
    return TT(0);
  }
};

template<typename T>
template<typename TT>
struct Ground<T>::MyOutput<std::vector<TT> > {

  static
  std::vector<TT> computeOutput(size_t samplesPerPeriod) {
    return std::vector<TT>(samplesPerPeriod, TT(0));
  }
};

#endif /* PKGINCLUDE_ACTORLIBRARY_SIMULINK_GROUND_HPP_ */
