// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_NODE_TYPES_HPP
#define _INCLUDED_HSCD_NODE_TYPES_HPP

#include <hscd_root_node.hpp>

class hscd_choice_node
  : public hscd_root_node {
  protected:
    hscd_choice_node(sc_module_name name)
      : hscd_root_node(name) {}
};

class hscd_transact_node
  : public hscd_choice_node {
  private:
    /* disable */
    void choice( hscd_op_choice op );
    void startChoice( hscd_op_choice op );
  protected:
    hscd_transact_node(sc_module_name name)
      : hscd_choice_node(name) {}
};

class hscd_fixed_transact_node
  : public hscd_transact_node {
private:
  /* disable */
  void transact( hscd_op_transact op );
  void startTransact( hscd_op_transact op );
protected:
  hscd_op_transact op;
  
  hscd_fixed_transact_node( sc_module_name name, hscd_op_transact op )
    : hscd_transact_node(name), op(op) {}
  
  void startTransact() {
    return hscd_transact_node::startTransact(op);
  }
  void transact() {
    hscd_transact_node::transact(op);
  }
  
  virtual void process() = 0;
};

class hscd_fixed_transact_active_node
  : public hscd_fixed_transact_node {
  private:
    void init() {
      hscd_transact_node::transact(op.onlyInputs());
      process();
    }
    
    // disable
    void startTransact();
  protected:
    SC_HAS_PROCESS(hscd_fixed_transact_active_node);
    
    hscd_fixed_transact_active_node( sc_module_name name, hscd_op_transact op )
      : hscd_fixed_transact_node(name,op) {
	SC_THREAD(init);
    }

  public:
};

class hscd_fixed_transact_passive_node
  : public hscd_fixed_transact_node {
  private:
    // disable
    void transact();
  protected:
    hscd_fixed_transact_passive_node( sc_module_name name, hscd_op_transact op )
      : hscd_fixed_transact_node(name,op) {
	hscd_transact_node::startTransact(op.onlyInputs());
    }
  public:
    void fire() {
      waitFinished();
      process();
      startTransact();
    }
};

#endif // _INCLUDED_HSCD_NODE_TYPES_HPP
