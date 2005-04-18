// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_ROOT_NODE_HPP
#define _INCLUDED_HSCD_ROOT_NODE_HPP

#include <hscd_firing_rules.hpp>
#include <hscd_port.hpp>
//#include <hscd_op.hpp>
#ifndef __SCFE__
# include <hscd_pggen.hpp>
#endif

//#include <oneof.hpp>

#include <systemc.h>

#include <list>

class hscd_opbase_node {
public:
  typedef hscd_opbase_node this_type;
protected:
  template <typename T>
  hscd_interface_action call(
      void (T::*f)(),
      const hscd_firing_state &s ) {
//    std::cerr << "call" << std::endl;
    return hscd_interface_action(s,hscd_func_call(this,f));
  }
  template <typename T>
  hscd_interface_action call(
      void (T::*f)(),
      hscd_firing_state &s ) {
//    std::cerr << "call" << std::endl;
    return hscd_interface_action(s,hscd_func_call(this,f));
  }
  template <typename T>
  hscd_interface_action call(
      void (T::*f)(),
      hscd_firing_state *s ) {
//    std::cerr << "call" << std::endl;
    return hscd_interface_action(s,hscd_func_call(this,f));
  }
  
  template <typename T>
  hscd_interface_action branch(
      const hscd_firing_state &(T::*f)(),
      const hscd_firing_state_list &sl ) {
//    std::cerr << "branch" << std::endl;
    return hscd_interface_action(sl,hscd_func_branch(this,f));
  }
  hscd_firing_state Transact( const hscd_interface_transition &t ) {
//    std::cerr << "Transact" << std::endl;
    return hscd_firing_state(t);
  }
  hscd_firing_state Choice( const hscd_transition_list &tl ) {
//    std::cerr << "Choice" << std::endl;
    return hscd_firing_state(tl);
  }
};

class hscd_root_node
  : public hscd_opbase_node {
private:
  hscd_firing_state        _currentState;
  const hscd_firing_state *_initialState;
  
  bool           ports_valid;
  hscd_port_list ports;
protected:
  hscd_root_node(const hscd_firing_state &s)
    : _currentState(s), _initialState(NULL), ports_valid(false)
    {}
  hscd_root_node(hscd_firing_state &s)
    : _initialState(&s), ports_valid(false)
    {}
  
  virtual ~hscd_root_node() {}
public:
  virtual void finalise() {
//    std::cout << myModule()->name() << ": finalise" << std::endl;
    if ( _initialState != NULL ) {
      _currentState = *_initialState;
      _initialState = NULL;
    }
    _currentState.finalise(this);
  }
  //sc_event		_fire;
  //hscd_port_in<void>  fire_port;

#ifndef __SCFE__
  virtual sc_module *myModule() = 0;
  virtual const sc_module *myModule() const {
    return const_cast<hscd_root_node *>(this)->myModule();
  }
  
  void assemble( hscd_modes::PGWriter &pgw ) const;
#endif

  hscd_port_list &getPorts();

  const hscd_firing_state &currentState() const { return _currentState; }
  hscd_firing_state       &currentState()       { return _currentState; }
};

#endif // _INCLUDED_HSCD_ROOT_NODE_HPP
