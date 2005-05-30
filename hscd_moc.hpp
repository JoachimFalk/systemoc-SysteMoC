// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_MOC_HPP
#define _INCLUDED_HSCD_MOC_HPP

#include <systemc.h>
#include <hscd_graph_type.hpp>

#include <list>

template<typename T>
void dump(std::list<T> &nodes) {
  std::cout << "=== dump ===" << std::endl;
  for ( typename std::list<T>::const_iterator iter = nodes.begin();
        iter != nodes.end();
        ++iter ) {
    const hscd_firing_state &s = (*iter)->currentState();
    std::cout << "actor: " << (*iter)->myModule()->name() << " state: " << &s << std::endl;
    std::cout << s;
  }
}

template <typename T_scheduler, typename T_constraintset>
class hscd_moc
  : public T_constraintset,
    public T_scheduler {
private:
//  void process() { return schedule(
//    static_cast<typename T_scheduler::cset_ty *>(this)); }
protected:
  void finalise() {
    T_constraintset::finalise();
    T_scheduler::finalise();
  }
public:
  typedef hscd_moc<T_scheduler, T_constraintset> this_type;
  
//  SC_HAS_PROCESS(this_type);
//  
  explicit hscd_moc( sc_module_name name )
    : T_constraintset(name), T_scheduler(
        static_cast<typename T_scheduler::cset_ty *>(this)) {
//#ifndef __SCFE__
//    SC_THREAD(process);
//#endif
  }
  hscd_moc()
    : T_constraintset(), T_scheduler(
        static_cast<typename T_scheduler::cset_ty *>(this)) {
//#ifndef __SCFE__
//    SC_THREAD(process);
//#endif
  }

#ifndef __SCFE__
  sc_module *myModule() { return this; }

  void assemble( hscd_modes::PGWriter &pgw ) const {
    return T_constraintset::assemble(pgw); }
//  void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
//    return T_constraintset::pgAssemble(pgw); }
#endif
};

class hscd_scheduler_base {
protected:
  template <typename T>
  hscd_interface_action diverge(
      const hscd_firing_state &(T::*f)() ) {
//    std::cerr << "diverge" << std::endl;
    return hscd_interface_action(hscd_func_diverge(this,f));
  }
};

class hscd_moc_scheduler_sdf
  : public hscd_fixed_transact_node,
    public hscd_scheduler_base {
public:
  typedef hscd_moc_scheduler_sdf  this_type;
  typedef hscd_sdf_constraintset  cset_ty;
protected:
  cset_ty *c;
  
  void schedule() {}
private:
  hscd_interface_transition analyse( cset_ty *c ){
    return hscd_activation_pattern() >> call(&hscd_moc_scheduler_sdf::schedule);
  }
public:
  hscd_moc_scheduler_sdf( cset_ty *c )
    : hscd_fixed_transact_node(analyse(c)), c(c) {}
};

class hscd_moc_scheduler_ndf
  : public hscd_transact_node,
    public hscd_firing_types,
    public hscd_scheduler_base {

public:
  typedef hscd_moc_scheduler_ndf  this_type;
  typedef hscd_ndf_constraintset  cset_ty;
protected:
  cset_ty *c;
  
  typedef std::pair<transition_ty *, hscd_root_node *>  transition_node_ty;
  typedef std::list<transition_node_ty>                 transition_node_list_ty;
  
  const hscd_firing_state &schedule() {
    cset_ty::nodes_ty nodes = c->getNodes();
    transition_node_list_ty tln;
    
    do {
      tln.clear();
      for ( cset_ty::nodes_ty::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter ) {
        hscd_firing_state   &s  = (*iter)->currentState();
        resolved_state_ty   &rs = s.getResolvedState();
        maybe_transition_ty  mt = rs.findEnabledTransition();
        
        if ( mt.first )
          tln.push_front(transition_node_ty(mt.second,*iter));
      }
      // dump(nodes);
      for ( transition_node_list_ty::const_iterator iter = tln.begin();
            iter != tln.end();
            ++iter ) {
        iter->second->currentState().execute(iter->first);
      }
    } while (!tln.empty());
    s = Transact( hscd_activation_pattern() >> diverge(&hscd_moc_scheduler_ndf::schedule) );
    return s;
  }
private:
  hscd_firing_state s;
  
  void analyse() {
    s = Transact( hscd_activation_pattern() >> diverge(&hscd_moc_scheduler_ndf::schedule) );
  }
public:
  hscd_moc_scheduler_ndf( cset_ty *c )
    : hscd_transact_node(s), c(c) { analyse(); }
};

class hscd_moc_scheduler_ddf
  : public hscd_transact_node,
    public hscd_firing_types,
    public hscd_scheduler_base {

public:
  typedef hscd_moc_scheduler_ddf  this_type;
  typedef hscd_ddf_constraintset  cset_ty;
protected:
  cset_ty *c;
  
  typedef std::pair<transition_ty *, hscd_root_node *>  transition_node_ty;
  typedef std::list<transition_node_ty>                 transition_node_list_ty;
  
  const hscd_firing_state &schedule() {
    cset_ty::nodes_ty nodes = c->getNodes();
    transition_node_list_ty tln;
    
    do {
      tln.clear();
      for ( cset_ty::nodes_ty::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter ) {
        hscd_firing_state   &s  = (*iter)->currentState();
        resolved_state_ty   &rs = s.getResolvedState();
        maybe_transition_ty  mt = rs.findEnabledTransition();
        
        if ( mt.first )
          tln.push_front(transition_node_ty(mt.second,*iter));
      }
      // dump(nodes);
      for ( transition_node_list_ty::const_iterator iter = tln.begin();
            iter != tln.end();
            ++iter ) {
        iter->second->currentState().execute(iter->first);
      }
    } while (!tln.empty());
    s = Transact( hscd_activation_pattern() >> diverge(&hscd_moc_scheduler_ddf::schedule) );
    return s;
  }
private:
  hscd_firing_state s;
  
  void analyse() {
    s = Transact( hscd_activation_pattern() >> diverge(&hscd_moc_scheduler_ddf::schedule) );
  }
public:
  hscd_moc_scheduler_ddf( cset_ty *c )
    : hscd_transact_node(s), c(c) { analyse(); }
};

class hscd_moc_scheduler_csp
  : public hscd_choice_node,
    public hscd_firing_types,
    public hscd_scheduler_base {

public:
  typedef hscd_moc_scheduler_csp  this_type;
  typedef hscd_csp_constraintset  cset_ty;
protected:
  cset_ty *c;
  
  void schedule() {
    cset_ty::nodes_ty nodes = c->getNodes();
    
    bool again;
    do {
      do {
        again = false;
        dump(nodes);
        for ( cset_ty::nodes_ty::iterator iter = nodes.begin();
              iter != nodes.end();
              ++iter )
          again |= (*iter)->currentState().inductionStep();
      } while ( again );
      for ( cset_ty::nodes_ty::iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter )
        again |= (*iter)->currentState().choiceStep();
    } while ( again );
  }
private:
  hscd_firing_state analyse( cset_ty *c ) const {
    return hscd_firing_state();
  }
public:
  hscd_moc_scheduler_csp( cset_ty *c )
    : hscd_choice_node(analyse(c)), c(c) {}
};

template <typename T_constraintset>
class hscd_sdf_moc
  : public hscd_moc<hscd_moc_scheduler_sdf, T_constraintset> {
  public:
    explicit hscd_sdf_moc( sc_module_name name )
      : hscd_moc<hscd_moc_scheduler_sdf, T_constraintset>(name) {}
    hscd_sdf_moc()
      : hscd_moc<hscd_moc_scheduler_sdf, T_constraintset>() {}
};

template <typename T_constraintset>
class hscd_csp_moc
  : public hscd_moc<hscd_moc_scheduler_csp, T_constraintset> {
  public:
    explicit hscd_csp_moc( sc_module_name name )
      : hscd_moc<hscd_moc_scheduler_csp, T_constraintset>(name) {}
    hscd_csp_moc()
      : hscd_moc<hscd_moc_scheduler_csp, T_constraintset>() {}
};

template <typename T_constraintset>
class hscd_ddf_moc
  : public hscd_moc<hscd_moc_scheduler_ddf, T_constraintset> {
  public:
    explicit hscd_ddf_moc( sc_module_name name )
      : hscd_moc<hscd_moc_scheduler_ddf, T_constraintset>(name) {}
    hscd_ddf_moc()
      : hscd_moc<hscd_moc_scheduler_ddf, T_constraintset>() {}
};

template <typename T_constraintset>
class hscd_ndf_moc
  : public hscd_moc<hscd_moc_scheduler_ndf, T_constraintset> {
  public:
    explicit hscd_ndf_moc( sc_module_name name )
      : hscd_moc<hscd_moc_scheduler_ndf, T_constraintset>(name) {}
    hscd_ndf_moc()
      : hscd_moc<hscd_moc_scheduler_ndf, T_constraintset>() {}
};

template <typename T_top>
class hscd_top_moc
  : public T_top {
private:
  // called by elaboration_done (does nothing by default)
  void end_of_elaboration() {
    finalise();
    
    hscd_port_list ps = getPorts();
    
    std::cout << "ports:" << std::endl;
    for ( hscd_port_list::const_iterator iter = ps.begin();
          iter != ps.end();
          ++iter ) {
      std::cout << *iter << std::endl;
    }
  }
  
  void process() {
    schedule();
  }
public:
  typedef hscd_top_moc<T_top> this_type;
  
  SC_HAS_PROCESS(this_type);
  
  explicit hscd_top_moc( sc_module_name name )
    : T_top(name) {
#ifndef __SCFE__
    SC_THREAD(process);
#endif
  }
  hscd_top_moc()
    : T_top() {
#ifndef __SCFE__
    SC_THREAD(process);
#endif
  }
};

#endif // _INCLUDED_HSCD_MOC_HPP
