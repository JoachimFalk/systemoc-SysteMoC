// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_SCHEDULER_HPP
#define _INCLUDED_HSCD_SCHEDULER_HPP

#include <systemc.h>
#include <hscd_node_types.hpp>
#include <hscd_structure.hpp>
// #include <hscd_rendezvous.hpp>

#include <list>

class hscd_scheduler_asap
: public sc_module{
public:
  template <typename T>
  hscd_scheduler_asap( sc_module_name name, const std::list<T> &nl )
    : sc_module(name) {}
};

class hscd_top {
private:
  std::list<hscd_choice_active_node *>   nl;
  hscd_scheduler_asap             sched;
  
  std::list<hscd_choice_active_node *> &setTop( hscd_choice_active_node *top ) {
    nl.push_front(top); return nl;
  }
public:
  hscd_top(hscd_choice_active_node *top)
    : sched("xxxx", setTop(top)) {}
};



#endif // _INCLUDED_HSCD_SCHEDULER_HPP
