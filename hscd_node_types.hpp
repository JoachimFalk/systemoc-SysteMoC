// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_NODE_TYPES_HPP
#define _INCLUDED_HSCD_NODE_TYPES_HPP

#include <hscd_root_node.hpp>

class hscd_choice_node
  : public hscd_root_node {
  protected:
    hscd_choice_node(const hscd_firing_state &s)
      : hscd_root_node(s) {}
    hscd_choice_node(hscd_firing_state &s)
      : hscd_root_node(s) {}
};

class hscd_choice_passive_node
  : public hscd_choice_node,
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public sc_module {
  protected:
    explicit hscd_choice_passive_node( sc_module_name name, const hscd_firing_state &s )
      : hscd_choice_node(s),
        sc_module(name) {}
    hscd_choice_passive_node(const hscd_firing_state &s)
      : hscd_choice_node(s),
        sc_module( sc_gen_unique_name("hscd_choice_active_node") ) {}
    explicit hscd_choice_passive_node( sc_module_name name, hscd_firing_state &s )
      : hscd_choice_node(s),
        sc_module(name) {}
    hscd_choice_passive_node(hscd_firing_state &s)
      : hscd_choice_node(s),
        sc_module( sc_gen_unique_name("hscd_choice_active_node") ) {}
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return hscd_choice_node::assemble(pgw); }
#endif
};

/*
class hscd_choice_active_node
  : public sc_module,
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public hscd_choice_node {
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_choice_active_node);
    
    explicit hscd_choice_active_node( sc_module_name name, const hscd_firing_state &s )
      : sc_module(name),
        hscd_choice_node(s) {
      SC_THREAD(process);
    }
    hscd_choice_active_node(const hscd_firing_state &s)
      : sc_module( sc_gen_unique_name("hscd_choice_active_node") ),
        hscd_choice_node(s) {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return hscd_choice_node::assemble(pgw); }
#endif
};*/

class hscd_transact_node
  : public hscd_choice_node {
  private:
    /* disable */
    //void choice( hscd_op_choice op );
    //void startChoice( hscd_op_choice op );
    hscd_firing_state Choice( const hscd_transition_list &tl );
  protected:
    hscd_transact_node(const hscd_firing_state &s)
      : hscd_choice_node(s) {}
    hscd_transact_node(hscd_firing_state &s)
      : hscd_choice_node(s) {}
};

class hscd_transact_passive_node
  : public hscd_transact_node,
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public sc_module {
  protected:
    explicit hscd_transact_passive_node( sc_module_name name, const hscd_firing_state &s )
      : hscd_transact_node(s),
        sc_module(name) {}
    hscd_transact_passive_node(const hscd_firing_state &s)
      : hscd_transact_node(s),
        sc_module( sc_gen_unique_name("hscd_transact_active_node") ) {}
    explicit hscd_transact_passive_node( sc_module_name name, hscd_firing_state &s )
      : hscd_transact_node(s),
        sc_module(name) {}
    hscd_transact_passive_node(hscd_firing_state &s)
      : hscd_transact_node(s),
        sc_module( sc_gen_unique_name("hscd_transact_active_node") ) {}
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return hscd_transact_node::assemble(pgw); }
#endif
};

/*
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
    
    explicit hscd_transact_active_node( sc_module_name name, const hscd_firing_state &s )
      : sc_module(name),
        hscd_transact_node(s) {
      SC_THREAD(process);
    }
    hscd_transact_active_node(const hscd_firing_state &s)
      : sc_module( sc_gen_unique_name("hscd_transact_active_node") ),
        hscd_transact_node(s) {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return hscd_transact_node::assemble(pgw); }
#endif
};
*/

class hscd_fixed_transact_node
  : public hscd_transact_node {
private:
  hscd_interface_transition _it;
  hscd_firing_state         _startState;
  hscd_firing_state         _writeState;
  
  hscd_firing_state Transact( const hscd_interface_transition &t );
  template <typename T>
  hscd_interface_action branch(
      hscd_firing_state (T::*f)(),
      const hscd_firing_state_list &sl );
  template <typename T>
  hscd_interface_action call(
      void (T::*f)(),
      const hscd_firing_state &s );
  template <typename T>
  hscd_interface_action call(
      void (T::*f)(),
      hscd_firing_state &s );
  template <typename T>
  hscd_interface_action call(
      void (T::*f)(),
      hscd_firing_state *s );
protected:
  //hscd_op_transact op;
  
  hscd_fixed_transact_node(const hscd_interface_transition &it)
    : hscd_transact_node(_startState), _it(it) {}
  
  void finalise() {
    hscd_firing_state loop;
    
    _startState = hscd_transact_node::Transact( _it.onlyInputs() );
    _writeState = hscd_transact_node::Transact( _it.getActivationPattern().onlyOutputs() >> _startState );
    return hscd_transact_node::finalise();
  }
  
  template <typename T>
  hscd_interface_action call( void (T::*f)() )
    { return hscd_transact_node::call(f, &_writeState); }
};

class hscd_fixed_transact_passive_node
  : public hscd_fixed_transact_node,
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public sc_module {
  protected:
    explicit hscd_fixed_transact_passive_node(
        sc_module_name name, const hscd_interface_transition &it )
      : hscd_fixed_transact_node(it),
        sc_module( name ) {}
    hscd_fixed_transact_passive_node(
        const hscd_interface_transition &it )
      : hscd_fixed_transact_node(it),
        sc_module( sc_gen_unique_name("hscd_fixed_transact_passive_node") ) {}
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return hscd_fixed_transact_node::assemble(pgw); }
#endif
};

/*
class hscd_fixed_transact_active_node
  : public sc_module,
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public hscd_fixed_transact_node {
  private:
    void init() {
      //hscd_transact_node::transact(op.onlyInputs());
      process();
    }
    
    // disable
    //void startTransact();
  protected:
    SC_HAS_PROCESS(hscd_fixed_transact_active_node);
    
    explicit hscd_fixed_transact_active_node( sc_module_name name, const hscd_activation_pattern &ap )
      : sc_module( name ),
        hscd_fixed_transact_node(ap) {
      SC_THREAD(init);
    }
    hscd_fixed_transact_active_node(const hscd_activation_pattern &ap)
      : sc_module( sc_gen_unique_name("hscd_fixed_transact_active_node") ),
        hscd_fixed_transact_node(ap) {
      SC_THREAD(init);
    }
  public:
#ifndef __SCFE__
   sc_module *myModule() { return this; }
    
   void assemble( hscd_modes::PGWriter &pgw ) const {
      return hscd_fixed_transact_node::assemble(pgw); }
#endif
};
*/

#endif // _INCLUDED_HSCD_NODE_TYPES_HPP
