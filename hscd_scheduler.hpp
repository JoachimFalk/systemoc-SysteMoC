// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_SCHEDULER_HPP
#define _INCLUDED_HSCD_SCHEDULER_HPP

#include <systemc.h>
#include <hscd_root_node.hpp>
#include <hscd_structure.hpp>
#include <hscd_rendezvous.hpp>

#include <list>

class hscd_scheduler_base
  : public hscd_opbase_node {
  public:
    hscd_scheduler_base( sc_module_name name )
      : hscd_opbase_node(name) {}

    void assemble( hscd_modes::PGWriter &pgw ) const {
      assert( 0 );
    }
};

class hscd_scheduler_asap
  : public hscd_scheduler_base {
  private:
    //    nodes_ty nodes;
    hscd_op_port_or_list    fire_list;
    
    void schedule() {
      while ( 1 )
        choice( fire_list );
    }
  public:
    SC_HAS_PROCESS(hscd_scheduler_asap);
    
    template <typename T>
    hscd_scheduler_asap( sc_module_name name, const std::list<T> &nl )
      : hscd_scheduler_base(name) {
      for ( typename std::list<T>::const_iterator iter = nl.begin();
            iter != nl.end(); ++iter ) {
        hscd_rendezvous::chan_type<void> *fire_channel = new hscd_rendezvous::chan_type<void>();
        hscd_port_out<void>              *fire_port    = new hscd_port_out<void>();
        
        (*iter)->fire_port(*fire_channel);
        (*fire_port)(*fire_channel);
        fire_list | (*fire_port)(1);
        // nodes.push_back(*iter);
        // fire.push_back(
      }
      SC_THREAD(schedule);
    }

};

#endif // _INCLUDED_HSCD_SCHEDULER_HPP
