// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_NODE_TYPES_HPP
#define _INCLUDED_SMOC_NODE_TYPES_HPP

#include <smoc_root_node.hpp>

class smoc_actor
  : public smoc_root_node,
#ifndef __SCFE__
    public smoc_modes::smoc_modes_base_structure,
#endif
    public sc_module {
  protected:
    explicit smoc_actor( sc_module_name name, const smoc_firing_state &s )
      : smoc_root_node(s),
        sc_module(name) {}
    smoc_actor(const smoc_firing_state &s)
      : smoc_root_node(s),
        sc_module( sc_gen_unique_name("smoc_choice_active_node") ) {}
    explicit smoc_actor( sc_module_name name, smoc_firing_state &s )
      : smoc_root_node(s),
        sc_module(name) {}
    smoc_actor(smoc_firing_state &s)
      : smoc_root_node(s),
        sc_module( sc_gen_unique_name("smoc_choice_active_node") ) {}
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( smoc_modes::PGWriter &pgw ) const {
      return smoc_root_node::assemble(pgw); }
#endif
};

typedef smoc_root_node smoc_choice_node;
typedef smoc_root_node smoc_transact_node;
typedef smoc_root_node smoc_fixed_transact_node;

/*
class smoc_choice_node
  : public smoc_root_node {
  protected:
    smoc_choice_node(const smoc_firing_state &s)
      : smoc_root_node(s) {}
    smoc_choice_node(smoc_firing_state &s)
      : smoc_root_node(s) {}
};

class smoc_choice_active_node
  : public sc_module,
#ifndef __SCFE__
    public smoc_modes::smoc_modes_base_structure,
#endif
    public smoc_choice_node {
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(smoc_choice_active_node);
    
    explicit smoc_choice_active_node( sc_module_name name, const smoc_firing_state &s )
      : sc_module(name),
        smoc_choice_node(s) {
      SC_THREAD(process);
    }
    smoc_choice_active_node(const smoc_firing_state &s)
      : sc_module( sc_gen_unique_name("smoc_choice_active_node") ),
        smoc_choice_node(s) {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( smoc_modes::PGWriter &pgw ) const {
      return smoc_choice_node::assemble(pgw); }
#endif
};

class smoc_transact_node
  : public smoc_choice_node {
  private:
    // disable
    //void choice( smoc_op_choice op );
    //void startChoice( smoc_op_choice op );
    smoc_firing_state Choice( const smoc_transition_list &tl );
  protected:
    smoc_transact_node(const smoc_firing_state &s)
      : smoc_choice_node(s) {}
    smoc_transact_node(smoc_firing_state &s)
      : smoc_choice_node(s) {}
};

class smoc_transact_passive_node
  : public smoc_transact_node,
#ifndef __SCFE__
    public smoc_modes::smoc_modes_base_structure,
#endif
    public sc_module {
  protected:
    explicit smoc_transact_passive_node( sc_module_name name, const smoc_firing_state &s )
      : smoc_transact_node(s),
        sc_module(name) {}
    smoc_transact_passive_node(const smoc_firing_state &s)
      : smoc_transact_node(s),
        sc_module( sc_gen_unique_name("smoc_transact_active_node") ) {}
    explicit smoc_transact_passive_node( sc_module_name name, smoc_firing_state &s )
      : smoc_transact_node(s),
        sc_module(name) {}
    smoc_transact_passive_node(smoc_firing_state &s)
      : smoc_transact_node(s),
        sc_module( sc_gen_unique_name("smoc_transact_active_node") ) {}
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( smoc_modes::PGWriter &pgw ) const {
      return smoc_transact_node::assemble(pgw); }
#endif
};

class smoc_transact_active_node
  : public sc_module,
#ifndef __SCFE__
    public smoc_modes::smoc_modes_base_structure,
#endif
    public smoc_transact_node {
  private:
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(smoc_transact_active_node);
    
    explicit smoc_transact_active_node( sc_module_name name, const smoc_firing_state &s )
      : sc_module(name),
        smoc_transact_node(s) {
      SC_THREAD(process);
    }
    smoc_transact_active_node(const smoc_firing_state &s)
      : sc_module( sc_gen_unique_name("smoc_transact_active_node") ),
        smoc_transact_node(s) {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( smoc_modes::PGWriter &pgw ) const {
      return smoc_transact_node::assemble(pgw); }
#endif
};

class smoc_fixed_transact_node
  : public smoc_transact_node {
private:
  smoc_transition _it;
  smoc_firing_state         _startState;
  smoc_firing_state         _writeState;
  
  smoc_firing_state Transact( const smoc_transition &t );
  template <typename T>
  smoc_interface_action branch(
      smoc_firing_state (T::*f)(),
      const smoc_firing_state_list &sl );
  template <typename T>
  smoc_interface_action call(
      void (T::*f)(),
      const smoc_firing_state &s );
  template <typename T>
  smoc_interface_action call(
      void (T::*f)(),
      smoc_firing_state &s );
  template <typename T>
  smoc_interface_action call(
      void (T::*f)(),
      smoc_firing_state *s );
protected:
  //smoc_op_transact op;
  
  smoc_fixed_transact_node(const smoc_transition &it)
    : smoc_transact_node(_startState), _it(it) {}
  
  void finalise() {
    smoc_firing_state loop;
    
    _startState = smoc_transact_node::Transact( _it.onlyInputs() );
    _writeState = smoc_transact_node::Transact( _it.getActivationPattern().onlyOutputs() >> _startState );
    return smoc_transact_node::finalise();
  }
  
  template <typename T>
  smoc_interface_action call( void (T::*f)() )
    { return smoc_transact_node::call(f, &_writeState); }
};

class smoc_fixed_transact_passive_node
  : public smoc_fixed_transact_node,
#ifndef __SCFE__
    public smoc_modes::smoc_modes_base_structure,
#endif
    public sc_module {
  protected:
    explicit smoc_fixed_transact_passive_node(
        sc_module_name name, const smoc_transition &it )
      : smoc_fixed_transact_node(it),
        sc_module( name ) {}
    smoc_fixed_transact_passive_node(
        const smoc_transition &it )
      : smoc_fixed_transact_node(it),
        sc_module( sc_gen_unique_name("smoc_fixed_transact_passive_node") ) {}
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( smoc_modes::PGWriter &pgw ) const {
      return smoc_fixed_transact_node::assemble(pgw); }
#endif
};

class smoc_fixed_transact_active_node
  : public sc_module,
#ifndef __SCFE__
    public smoc_modes::smoc_modes_base_structure,
#endif
    public smoc_fixed_transact_node {
  private:
    void init() {
      //smoc_transact_node::transact(op.onlyInputs());
      process();
    }
    
    // disable
    //void startTransact();
  protected:
    SC_HAS_PROCESS(smoc_fixed_transact_active_node);
    
    explicit smoc_fixed_transact_active_node( sc_module_name name, const smoc_activation_pattern &ap )
      : sc_module( name ),
        smoc_fixed_transact_node(ap) {
      SC_THREAD(init);
    }
    smoc_fixed_transact_active_node(const smoc_activation_pattern &ap)
      : sc_module( sc_gen_unique_name("smoc_fixed_transact_active_node") ),
        smoc_fixed_transact_node(ap) {
      SC_THREAD(init);
    }
  public:
#ifndef __SCFE__
   sc_module *myModule() { return this; }
    
   void assemble( smoc_modes::PGWriter &pgw ) const {
      return smoc_fixed_transact_node::assemble(pgw); }
#endif
};
*/

#endif // _INCLUDED_SMOC_NODE_TYPES_HPP
