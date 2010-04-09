// vim: set sw=2 ts=8:

#ifndef INCLUDE_WSDF_INPUT_SWITCH_HPP
#define INCLUDE_WSDF_INPUT_SWITCH_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_node_types.hpp>

template <typename T>
class m_wsdf_input_switch: public smoc_actor {

public:
        
  smoc_md_port_in<T,2> orig_in;
  smoc_md_port_in<T,2> buffered_in;       
        
  smoc_md_port_out<T,2> out;

#ifndef PURE_MD_MODE
  smoc_port_in<T>      max_in;
#else
  smoc_md_port_in<T,2> max_in;
#endif
        

private:

  void copy_from_orig(){
    //std::cout << "Copy from orig" << std::endl;
    out[0][0] = orig_in[0][0];
  }

  void copy_from_buffered(){
    //std::cout << "Copy from buffered" << std::endl;
    out[0][0] = buffered_in[0][0];
  }

#ifdef PURE_MD_MODE
  bool read_orig_guard() const {
    if (max_in[0][0] == 0)
      return true;
    else
      return false;
  }
#endif

  smoc_firing_state state1;
#ifndef PURE_MD_MODE
  smoc_firing_state state2;
#endif

public:
  m_wsdf_input_switch( sc_module_name name ,
                       unsigned size_x,
                       unsigned size_y)
    : smoc_actor(name,state1),
      orig_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
              ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
              ns_smoc_vector_init::ul_vector_init[1][1], //c
              ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
              ns_smoc_vector_init::sl_vector_init[0][0], //bs
              ns_smoc_vector_init::sl_vector_init[0][0] //bt
              ),
      buffered_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
                  ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
                  ns_smoc_vector_init::ul_vector_init[1][1], //c
                  ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
                  ns_smoc_vector_init::sl_vector_init[0][0], //bs
                  ns_smoc_vector_init::sl_vector_init[0][0] //bt
                  ),
      out(ns_smoc_vector_init::ul_vector_init[1][1] <<
          ns_smoc_vector_init::ul_vector_init[size_x][size_y]
          )
#ifdef PURE_MD_MODE
    , max_in(ns_smoc_vector_init::ul_vector_init[size_x][size_y], //firing_blocks
             ns_smoc_vector_init::ul_vector_init[size_x][size_y], //u0
             ns_smoc_vector_init::ul_vector_init[1][1], //c
             ns_smoc_vector_init::ul_vector_init[1][1], //delta_c
             ns_smoc_vector_init::sl_vector_init[0][0], //bs
             ns_smoc_vector_init::sl_vector_init[0][0] //bt
             )
#endif

  {
#ifndef PURE_MD_MODE
    state1 = (orig_in(1) && buffered_in(1) && max_in(1) && out(1))
      >> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) == 0))
      >> CALL(m_wsdf_input_switch::copy_from_orig) 
      >> state1
      | (buffered_in(1) && max_in(1) && out(1))
      >> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) != 0))
      >> CALL(m_wsdf_input_switch::copy_from_buffered) 
      >> state2
      | (orig_in(1) && buffered_in(1) && out(1))
      >> ((out.getIteration(0,0) != (size_t)0) || (out.getIteration(0,1) != (size_t)0))
      >> CALL(m_wsdf_input_switch::copy_from_orig) 
      >> state1;

    state2 = (orig_in(1) && buffered_in(1) && max_in(1) && out(1))
      >> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) == 0))
      >> CALL(m_wsdf_input_switch::copy_from_orig) 
      >> state1
      | (buffered_in(1) && max_in(1) && out(1))
      >> ((out.getIteration(0,0) == (size_t)0) && (out.getIteration(0,1) == (size_t)0) && (max_in.getValueAt(0) != 0))
      >> CALL(m_wsdf_input_switch::copy_from_buffered) 
      >> state2
      | (buffered_in(1) && out(1))
      >> ((out.getIteration(0,0) != (size_t)0) || (out.getIteration(0,1) != (size_t)0))
      >> CALL(m_wsdf_input_switch::copy_from_buffered) 
      >> state2;
#else
    state1 = (orig_in(1) && buffered_in(1) && max_in(1) && out(1))
      >> GUARD(m_wsdf_input_switch::read_orig_guard)
      >> CALL(m_wsdf_input_switch::copy_from_orig) 
      >> state1
      | (buffered_in(1) && max_in(1) && out(1))
      >> (!GUARD(m_wsdf_input_switch::read_orig_guard))
      >> CALL(m_wsdf_input_switch::copy_from_buffered) 
      >> state1;
#endif
  }
        
};


#endif //INCLUDE_WSDF_INPUT_SWITCH_HPP
