// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_NODE_TYPES_HPP
#define _INCLUDED_HSCD_NODE_TYPES_HPP

#include <hscd_root_node.hpp>

class hscd_choice_node
  : public hscd_root_node {
  protected:
    hscd_choice_node()
      : hscd_root_node() {}
};

class hscd_choice_active_node
  : public sc_module,
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public hscd_choice_node {
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_choice_active_node);
    
    explicit hscd_choice_active_node( sc_module_name name )
      : sc_module(name),
        hscd_choice_node() {
      SC_THREAD(process);
    }
    hscd_choice_active_node()
      : sc_module( sc_gen_unique_name("hscd_choice_active_node") ),
        hscd_choice_node() {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return leafAssemble(this,pgw); }
#endif
};

class hscd_transact_node
  : public hscd_choice_node {
  private:
    /* disable */
    void choice( hscd_op_choice op );
    void startChoice( hscd_op_choice op );
  protected:
    hscd_transact_node()
      : hscd_choice_node() {}
};

class hscd_transact_active_node
  : public sc_module,
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public hscd_transact_node {
  private:
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_transact_active_node);
    
    explicit hscd_transact_active_node( sc_module_name name )
      : sc_module(name),
        hscd_transact_node() {
      SC_THREAD(process);
    }
    hscd_transact_active_node()
      : sc_module( sc_gen_unique_name("hscd_transact_active_node") ),
        hscd_transact_node() {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return leafAssemble(this,pgw); }
#endif
};

class hscd_fixed_transact_node
  : public hscd_transact_node {
private:
  /* disable */
  void transact( hscd_op_transact op );
  void startTransact( hscd_op_transact op );
protected:
  hscd_op_transact op;
  
  hscd_fixed_transact_node( hscd_op_transact op )
    : hscd_transact_node(), op(op) {}
  
  void startTransact() {
    return hscd_transact_node::startTransact(op);
  }
  void transact() {
    hscd_transact_node::transact(op);
  }
  
  virtual void process() = 0;
};

class hscd_fixed_transact_active_node
  : public sc_module,
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public hscd_fixed_transact_node {
  private:
    void init() {
      hscd_transact_node::transact(op.onlyInputs());
      process();
    }
    
    // disable
    void startTransact();
  protected:
    SC_HAS_PROCESS(hscd_fixed_transact_active_node);
    
    explicit hscd_fixed_transact_active_node( sc_module_name name, hscd_op_transact op )
      : sc_module( name ),
        hscd_fixed_transact_node(op) {
      SC_THREAD(init);
    }
    hscd_fixed_transact_active_node( hscd_op_transact op )
      : sc_module( sc_gen_unique_name("hscd_fixed_transact_active_node") ),
        hscd_fixed_transact_node(op) {
      SC_THREAD(init);
    }

  public:
#ifndef __SCFE__
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return leafAssemble(this,pgw); }
#endif
};

class hscd_fixed_transact_passive_node
  : public sc_module,
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public hscd_fixed_transact_node {
  private:
    // disable
    void transact();
  protected:
    explicit hscd_fixed_transact_passive_node( sc_module_name name, hscd_op_transact op )
      : sc_module( name ),
        hscd_fixed_transact_node(op) {
      hscd_transact_node::startTransact(op.onlyInputs());
    }
    hscd_fixed_transact_passive_node( hscd_op_transact op )
      : sc_module( sc_gen_unique_name("hscd_fixed_transact_passive_node") ),
        hscd_fixed_transact_node(op) {
      hscd_transact_node::startTransact(op.onlyInputs());
    }
  public:
    void opFinished() {
      assert( finished() );
      process();
      startTransact();
    }
    
#ifndef __SCFE__
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return leafAssemble(this,pgw); }
#endif
};

#endif // _INCLUDED_HSCD_NODE_TYPES_HPP
