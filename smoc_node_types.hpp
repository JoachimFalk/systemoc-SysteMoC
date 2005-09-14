// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_NODE_TYPES_HPP
#define _INCLUDED_SMOC_NODE_TYPES_HPP

#include <smoc_root_node.hpp>

class smoc_actor
  : public smoc_root_node,
    public sc_module {
  protected:
    explicit smoc_actor( sc_module_name name, const smoc_firing_state &s )
      : smoc_root_node(s),
        sc_module(name),
        vpc_event(true){}
    smoc_actor(const smoc_firing_state &s)
      : smoc_root_node(s),
        sc_module( sc_gen_unique_name("smoc_actor") ),
        vpc_event(true) {}
    explicit smoc_actor( sc_module_name name, smoc_firing_state &s )
      : smoc_root_node(s),
        sc_module(name),
        vpc_event(true) {}
    smoc_actor(smoc_firing_state &s)
      : smoc_root_node(s),
        sc_module( sc_gen_unique_name("smoc_actor") ),
        vpc_event(true) {}
  public:
  // FIXME: protection
  smoc_event      vpc_event;
  
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( smoc_modes::PGWriter &pgw ) const {
      return smoc_root_node::assemble(pgw); }
#endif
};

#endif // _INCLUDED_SMOC_NODE_TYPES_HPP
