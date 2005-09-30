// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_ROOT_NODE_HPP
#define _INCLUDED_SMOC_ROOT_NODE_HPP

#include <smoc_firing_rules.hpp>
#include <smoc_port.hpp>
//#include <smoc_op.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

//#include <oneof.hpp>

#include <systemc.h>

#include <list>

class smoc_opbase_node {
public:
  typedef smoc_opbase_node this_type;
protected:
  template <typename T>
  smoc_func_call call ( void (T::*f)(), const char *func_name ) {
//    std::cerr << "call(f)" << std::endl;
    return smoc_func_call(this,f,func_name);
  }
  template <typename T>
  smoc_func_call call ( void (T::*f)()) {
//    std::cerr << "call(f)" << std::endl;
    return smoc_func_call(this,f);
  }
  template <typename T, class X>
  typename Expr::MemGuard<T,X>::type guard(T (X::*m)() const) const {
    return Expr::guard( dynamic_cast<const X *>(this), m );
  }
  template <typename T>
  static
  typename Expr::Var<T>::type var(T &x)
    { return Expr::var(x); }
  
  virtual ~smoc_opbase_node() {}
};

// smoc_opbase_node must be the first class from which smoc_root_node
// is derived. This requirement comes from the reinterpret_cast in
// smoc_func_xxx classes in smoc_firing_rules.hpp
class smoc_root_node
#ifndef __SCFE__
: public smoc_opbase_node,
  public smoc_modes::smoc_modes_base_structure {
#else
: public smoc_opbase_node {
#endif
private:
  smoc_firing_state        _currentState;
  const smoc_firing_state *_initialState;
  
  const smoc_firing_state &communicate() {
    
#ifdef SYSTEMOC_DEBUG
    std::cout << "  <call actor=" << myModule()->name()
              << " func=smoc_root_node::communicate>" << std::endl;
    std::cout << "    <communication type=\"execute\"/>" << std::endl;
#endif
    
    assert( vpc_event );
    
    for ( smoc_port_list::iterator iter = ports_setup.begin();
          iter != ports_setup.end();
          ++iter ) {
      (*iter)->commExec();
      (*iter)->reset();
    }
    ports_setup.clear();
#ifdef SYSTEMOC_DEBUG
    std::cout << "  </call>"<< std::endl;
#endif
    return nextState;
  }
protected:
  smoc_root_node(const smoc_firing_state &s)
    : _currentState(s), _initialState(NULL), is_v1_actor(false),
      commstate( smoc_activation_pattern(Expr::till(vpc_event), true) >>
                   smoc_interface_action(smoc_func_diverge(
                      this,&smoc_root_node::communicate)) ) {}
  smoc_root_node(smoc_firing_state &s)
    : _initialState(&s), is_v1_actor(false),
      commstate( smoc_activation_pattern(Expr::till(vpc_event), true) >>
                   smoc_interface_action(smoc_func_diverge(
                      this,&smoc_root_node::communicate)) ) {}
public:
  // FIXME: protection
  bool               is_v1_actor;
  smoc_firing_state  commstate;
  smoc_firing_state  nextState;
  smoc_event         vpc_event;
  smoc_port_list     ports_setup;
  
  virtual void finalise() {
//    std::cout << myModule()->name() << ": finalise" << std::endl;
    if ( _initialState != NULL ) {
      _currentState = *_initialState;
      _initialState = NULL;
    }
    _currentState.finalise(this);
//    dumpActor(std::cout);
  }
  //sc_event		_fire;
  //smoc_port_in<void>  fire_port;

#ifndef __SCFE__
  virtual sc_module *myModule() = 0;
  const sc_module *myModule() const {
    return const_cast<smoc_root_node *>(this)->myModule();
  }
  
  virtual void pgAssemble( smoc_modes::PGWriter &, const smoc_root_node * ) const;
  void assemble( smoc_modes::PGWriter &pgw ) const;
#endif
  
  const smoc_port_list getPorts() const;
  
  std::ostream &dumpActor( std::ostream &o );
  
  const smoc_firing_state &currentState() const { return _currentState; }
  smoc_firing_state       &currentState()       { return _currentState; }
};

typedef std::list<smoc_root_node *> smoc_node_list;

#endif // _INCLUDED_SMOC_ROOT_NODE_HPP
