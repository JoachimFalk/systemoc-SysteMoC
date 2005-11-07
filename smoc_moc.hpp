// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_MOC_HPP
#define _INCLUDED_SMOC_MOC_HPP

#include <systemc.h>
#include <smoc_graph_type.hpp>

#include <list>

#include <hscd_tdsim_TraceLog.hpp>

class smoc_scheduler_base {
protected:
  template <typename T>
  smoc_interface_action diverge(
      const smoc_firing_state &(T::*f)() ) {
//    std::cerr << "diverge" << std::endl;
    return smoc_interface_action(smoc_func_diverge(this,f));
  }
};

class smoc_scheduler_sdf
  : public smoc_root_node,
    public smoc_scheduler_base {
public:
  typedef smoc_scheduler_sdf  this_type;
  typedef smoc_sdf_constraintset  cset_ty;
protected:
  cset_ty *c;
  
  void schedule();
private:
  smoc_firing_state s;
  
  void analyse();
public:
  smoc_scheduler_sdf( cset_ty *c );
};

typedef class smoc_scheduler_ndf smoc_scheduler_ddf;

class smoc_scheduler_ndf
  : public smoc_root_node,
    public smoc_firing_types,
    public smoc_scheduler_base {
public:
  typedef smoc_scheduler_ndf      this_type;
  typedef smoc_ndf_constraintset  cset_ty;
private:
  smoc_firing_state  s;
  cset_ty           *c;
protected:
  typedef std::pair<transition_ty *, smoc_root_node *>  transition_node_ty;
  typedef std::list<transition_node_ty>                 transition_node_list_ty;
  
  const smoc_firing_state &schedule();
 
  /*
  void finalise() {
    s = smoc_activation_pattern() >> diverge(&smoc_scheduler_ndf::schedule);
    smoc_root_node::finalise();
  }*/
public:
  smoc_scheduler_ndf( cset_ty *c );
};

/*
class smoc_scheduler_csp
  : public smoc_choice_node,
    public smoc_firing_types,
    public smoc_scheduler_base {

public:
  typedef smoc_scheduler_csp  this_type;
  typedef smoc_csp_constraintset  cset_ty;
protected:
  cset_ty *c;
  
  void schedule() {
    smoc_node_list nodes = c->getNodes();
    
    bool again;
    do {
      do {
        again = false;
        dump(nodes);
        for ( smoc_node_list::iterator iter = nodes.begin();
              iter != nodes.end();
              ++iter )
          again |= (*iter)->currentState().inductionStep();
      } while ( again );
      for ( smoc_node_list::iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter )
        again |= (*iter)->currentState().choiceStep();
    } while ( again );
  }
private:
  smoc_firing_state analyse( cset_ty *c ) const {
    return smoc_firing_state();
  }
public:
  smoc_scheduler_csp( cset_ty *c )
    : smoc_choice_node(analyse(c)), c(c) {}
};
*/

class smoc_graph
  : public smoc_ndf_constraintset,
    public smoc_scheduler_ndf {
public:
  typedef smoc_graph this_type;
private:
protected:
  void finalise() {
    smoc_ndf_constraintset::finalise();
    smoc_scheduler_ndf::finalise();
  }
public:
  explicit smoc_graph( sc_module_name name )
    : smoc_ndf_constraintset(name),
      smoc_scheduler_ndf(this) {}
  smoc_graph()
    : smoc_ndf_constraintset(
        sc_gen_unique_name("smoc_graph") ),
      smoc_scheduler_ndf(this) {}

#ifndef __SCFE__
  sc_module *myModule() { return this; }
  
  void pgAssemble( smoc_modes::PGWriter &pgw, const smoc_root_node *n ) const
    { return smoc_ndf_constraintset::pgAssemble(pgw,n); }
#endif

};

class smoc_scheduler_top
  : public smoc_firing_types,
    public smoc_scheduler_base {
public:
  typedef smoc_scheduler_top      this_type;
protected:
  typedef std::pair<transition_ty *, smoc_root_node *>  transition_node_ty;
  typedef std::list<transition_node_ty>                 transition_node_list_ty;
  
  void  schedule(smoc_graph *c);
};

/*
template<typename T>
void dump(std::list<T> &nodes) {
  std::cout << "=== dump ===" << std::endl;
  for ( typename std::list<T>::const_iterator iter = nodes.begin();
        iter != nodes.end();
        ++iter ) {
    const smoc_firing_state &s = (*iter)->currentState();
    std::cout << "actor: " << (*iter)->myModule()->name() << " state: " << &s << std::endl;
    std::cout << s;
  }
}
*/

/*
template <typename T_scheduler, typename T_constraintset>
class smoc_moc
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
  typedef smoc_moc<T_scheduler, T_constraintset> this_type;
  
//  SC_HAS_PROCESS(this_type);
//  
  explicit smoc_moc( sc_module_name name )
    : T_constraintset(name), T_scheduler(
        static_cast<typename T_scheduler::cset_ty *>(this)) {
//#ifndef __SCFE__
//    SC_THREAD(process);
//#endif
  }
  smoc_moc()
    : T_constraintset(), T_scheduler(
        static_cast<typename T_scheduler::cset_ty *>(this)) {
//#ifndef __SCFE__
//    SC_THREAD(process);
//#endif
  }

#ifndef __SCFE__
  sc_module *myModule() { return this; }

  void assemble( smoc_modes::PGWriter &pgw ) const {
    return T_constraintset::assemble(pgw); }
//  void pgAssemble( sc_module *m, smoc_modes::PGWriter &pgw ) const {
//    return T_constraintset::pgAssemble(pgw); }
#endif
};

template <typename T_constraintset>
class smoc_sdf_moc
  : public smoc_moc<smoc_scheduler_sdf, T_constraintset> {
  public:
    explicit smoc_sdf_moc( sc_module_name name )
      : smoc_moc<smoc_scheduler_sdf, T_constraintset>(name) {}
    smoc_sdf_moc()
      : smoc_moc<smoc_scheduler_sdf, T_constraintset>() {}
};

template <typename T_constraintset>
class smoc_csp_moc
  : public smoc_moc<smoc_scheduler_csp, T_constraintset> {
  public:
    explicit smoc_csp_moc( sc_module_name name )
      : smoc_moc<smoc_scheduler_csp, T_constraintset>(name) {}
    smoc_csp_moc()
      : smoc_moc<smoc_scheduler_csp, T_constraintset>() {}
};

template <typename T_constraintset>
class smoc_ddf_moc
  : public smoc_moc<smoc_scheduler_ddf, T_constraintset> {
  public:
    explicit smoc_ddf_moc( sc_module_name name )
      : smoc_moc<smoc_scheduler_ddf, T_constraintset>(name) {}
    smoc_ddf_moc()
      : smoc_moc<smoc_scheduler_ddf, T_constraintset>() {}
};

template <typename T_constraintset>
class smoc_ndf_moc
  : public smoc_moc<smoc_scheduler_ndf, T_constraintset> {
  public:
    explicit smoc_ndf_moc( sc_module_name name )
      : smoc_moc<smoc_scheduler_ndf, T_constraintset>(name) {}
    smoc_ndf_moc()
      : smoc_moc<smoc_scheduler_ndf, T_constraintset>() {}
};
*/

template <typename T_top>
class smoc_top_moc
  : public T_top,
    public smoc_scheduler_top {
private:
  // called by elaboration_done (does nothing by default)
  void end_of_elaboration()
    { finalise(); }
  
  void scheduleTop()
    { return smoc_scheduler_top::schedule(this); }
public:
  typedef smoc_top_moc<T_top> this_type;
  
  SC_HAS_PROCESS(this_type);
  
  explicit smoc_top_moc( sc_module_name name )
    : T_top(name) {
#ifndef __SCFE__
    SC_THREAD(scheduleTop);
#endif
  }
  smoc_top_moc()
    : T_top() {
#ifndef __SCFE__
    SC_THREAD(scheduleTop);
#endif
  }
};

#endif // _INCLUDED_SMOC_MOC_HPP
