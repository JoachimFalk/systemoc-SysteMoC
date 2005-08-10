// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_NODE_TYPES_HPP
#define _INCLUDED_HSCD_NODE_TYPES_HPP

#include <smoc_node_types.hpp>
#include <hscd_op.hpp>

class hscd_choice_active_node
  : public sc_module,
    public smoc_root_node {
  private:
    smoc_firing_state start;
  protected:
    virtual void process() = 0;
    
    void transact( hscd_op_transact op ) { op.startOp(); }
    void choice( hscd_op_choice op )     { op.startOp(); }
    
    SC_HAS_PROCESS(hscd_choice_active_node);
    
    explicit hscd_choice_active_node( sc_module_name name )
      : sc_module(name),
        smoc_root_node(start) {
      SC_THREAD(process);
    }
    hscd_choice_active_node()
      : sc_module( sc_gen_unique_name("hscd_choice_active_node") ),
        smoc_root_node(start) {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
#endif
};

class hscd_transact_active_node
  : public hscd_choice_active_node {
  private:
    /* disable */
    void choice( hscd_op_choice op );
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_transact_active_node);
    
    explicit hscd_transact_active_node( sc_module_name name )
      : hscd_choice_active_node(name) {
      SC_THREAD(process);
    }
    hscd_transact_active_node()
      : hscd_choice_active_node( sc_gen_unique_name("hscd_transact_active_node") ) {
      SC_THREAD(process);
    }
  public:
};

class hscd_fixed_transact_active_node
  : public hscd_transact_active_node {
  private:
    hscd_op_transact op;
    
    void init() {
      hscd_transact_active_node::transact(op.onlyInputs());
      process();
    }
    
    // disable
    void transact( hscd_op_transact op );
  protected:
    void transact() {
      hscd_transact_active_node::transact(op);
    }
    
    SC_HAS_PROCESS(hscd_fixed_transact_active_node);
    
    explicit hscd_fixed_transact_active_node( sc_module_name name, hscd_op_transact op )
      : hscd_transact_active_node( name ),
        op(op) {
      SC_THREAD(init);
    }
    hscd_fixed_transact_active_node( hscd_op_transact op )
      : hscd_transact_active_node( sc_gen_unique_name("hscd_fixed_transact_active_node") ),
        op(op) {
      SC_THREAD(init);
    }
  public:
};

#endif // _INCLUDED_HSCD_NODE_TYPES_HPP
