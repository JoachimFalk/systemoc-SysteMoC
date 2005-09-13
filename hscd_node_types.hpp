// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_NODE_TYPES_HPP
#define _INCLUDED_HSCD_NODE_TYPES_HPP

#include <smoc_node_types.hpp>
#include <hscd_op.hpp>

class hscd_choice_node
  : public smoc_root_node {
  private:
    smoc_firing_state start;
  protected:
    void transact( const hscd_op_transact &op ) { op.startOp(); }
    void choice( const hscd_op_choice &op )     { op.startOp(); }
    
    hscd_choice_node()
      : smoc_root_node(start) { is_v1_actor = true; }
};

class hscd_choice_active_node
  : public sc_module,
    public hscd_choice_node {
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_choice_active_node);
    
    explicit hscd_choice_active_node( sc_module_name name )
      : sc_module(name) {
      SC_THREAD(process);
    }
    hscd_choice_active_node()
      : sc_module(sc_gen_unique_name("hscd_choice_active_node")) {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
#endif
};

class hscd_transact_node
  : public hscd_choice_node {
  private:
    /* disable */
    void choice( const hscd_op_choice &op );
  protected:
    hscd_transact_node()
      : hscd_choice_node() {}
};

class hscd_transact_active_node
  : public sc_module,
    public hscd_transact_node {
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_transact_active_node);
    
    explicit hscd_transact_active_node( sc_module_name name )
      : sc_module(name) {
      SC_THREAD(process);
    }
    hscd_transact_active_node()
      : sc_module(sc_gen_unique_name("hscd_transact_active_node")) {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
#endif
};

class hscd_fixed_transact_node
  : public hscd_transact_node {
  private:
    hscd_op_transact op;
    
    /* disable */
    void transact( const hscd_op_transact &op );
  protected:
    void init()
      { hscd_transact_node::transact(op.onlyInputs()); }
    void transact()
      { hscd_transact_node::transact(op); }
    
    hscd_fixed_transact_node( const hscd_op_transact &op )
      : hscd_transact_node(), op(op) {}
};

class hscd_fixed_transact_active_node
  : public sc_module,
    public hscd_fixed_transact_node {
  private:
    void init() {
      hscd_fixed_transact_node::init();
      process();
    }
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_fixed_transact_active_node);
    
    explicit hscd_fixed_transact_active_node( sc_module_name name, hscd_op_transact op )
      : sc_module(name),
        hscd_fixed_transact_node(op) {
      SC_THREAD(init);
    }
    hscd_fixed_transact_active_node( hscd_op_transact op )
      : sc_module(sc_gen_unique_name("hscd_fixed_transact_active_node")),
        hscd_fixed_transact_node(op) {
      SC_THREAD(init);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
#endif
};

#endif // _INCLUDED_HSCD_NODE_TYPES_HPP
