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
  
  void schedule() { assert("FIXME sdf scheduler unfinished !" == NULL); }
private:
  smoc_firing_state s;
  
  void analyse() {
    s = smoc_activation_pattern(Expr::literal(true), true) >>
        call(&smoc_scheduler_sdf::schedule) >> s;
  }
public:
  smoc_scheduler_sdf( cset_ty *c )
    : smoc_root_node(s), c(c) { analyse(); }
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
  
  const smoc_firing_state &schedule() {
    cset_ty::nodes_ty nodes = c->getNodes();
    bool again;
    
#ifdef SYSTEMOC_DEBUG
    std::cout << "<smoc_scheduler_ndf::schedule>" << std::endl;
#endif
    // FIXME: Big hack !!!
    _ctx.hierarchy = myModule();
    do {
      again = false;
      for ( cset_ty::nodes_ty::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter )
        again |= (*iter)->currentState().tryExecute();
//      wait(SC_ZERO_TIME);
    } while (again);
    {
      smoc_transition_list      tl;
      smoc_root_port_bool_list  l;
      
      for ( cset_ty::nodes_ty::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter )
        (*iter)->currentState().findBlocked(l);
#ifdef SYSTEMOC_DEBUG
      std::cout << "CREATE TRANSITIONS: " << l << std::endl;
#endif
      for ( smoc_root_port_bool_list::const_iterator iter = l.begin();
            iter != l.end();
            ++iter ) {
        tl |= smoc_activation_pattern(Expr::vguard(*iter), true) >>
                diverge(&smoc_scheduler_ndf::schedule);
      }
      s = tl;
    }
#ifdef SYSTEMOC_DEBUG
    std::cout << "</smoc_scheduler_ndf::schedule>" << std::endl;
#endif
    return s;
  }
 
  /*
  void finalise() {
    s = smoc_activation_pattern() >> diverge(&smoc_scheduler_ndf::schedule);
    smoc_root_node::finalise();
  }*/
public:
  smoc_scheduler_ndf( cset_ty *c )
    : smoc_root_node( smoc_activation_pattern(Expr::literal(true), true) >>
                      diverge(&smoc_scheduler_ndf::schedule) ),
      c(c) {
#ifdef SYSTEMOC_DEBUG
    std::cout << "smoc_scheduler_ndf" << std::endl;
#endif
      }
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
  
  void assemble( smoc_modes::PGWriter &pgw ) const
    { return smoc_ndf_constraintset::assemble(pgw); }
#endif

};

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

/*
template <typename T_constraintset>
class smoc_csp_moc
  : public smoc_moc<smoc_scheduler_csp, T_constraintset> {
  public:
    explicit smoc_csp_moc( sc_module_name name )
      : smoc_moc<smoc_scheduler_csp, T_constraintset>(name) {}
    smoc_csp_moc()
      : smoc_moc<smoc_scheduler_csp, T_constraintset>() {}
};
*/

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

template <typename T_top>
class smoc_top_moc
  : public T_top {
private:
//  bool notagain;
  
  // called by elaboration_done (does nothing by default)
  void end_of_elaboration() {
 //   if ( !notagain ) {
      finalise();
//      notagain = true;
//    }
  }
  
  void schedule() {
    do {
      bool executed = this->currentState().tryExecute();
      assert( executed == true );
      {
#ifdef SYSTEMOC_DEBUG
        std::cout << "in top scheduler !!!" << std::endl;
#endif
        smoc_root_port_bool_list l;
        
        smoc_event_or_list ol;
        this->currentState().findBlocked(l);
        for ( smoc_root_port_bool_list::iterator iter = l.begin();
              iter != l.end();
              ++iter ) {
          smoc_root_port_bool::reqs_ty &reqs = iter->reqs;
          
         /* 
          smoc_event_and_list al;
          for ( smoc_root_port_bool::reqs_ty::iterator riter = reqs.begin();
                riter != reqs.end();
                ++riter )
            al &= *static_cast<smoc_event *>(*riter);
          ol |= al;*/
#ifdef SYSTEMOC_DEBUG
	  std::cout << reqs << std::endl;
#endif
          assert( reqs.size() <=  1 );
	  if ( !reqs.empty() ) {
	    ol |= *static_cast<smoc_event *>(*reqs.begin());
	  }
        }
        smoc_wait(ol);
#ifdef SYSTEMOC_DEBUG
        for ( smoc_event_or_list::iterator iter = ol.begin();
              iter != ol.end();
              ++iter )
          std::cout << **iter << std::endl;
#endif
      }
    } while ( 1 );
  }
public:
  typedef smoc_top_moc<T_top> this_type;
  
  SC_HAS_PROCESS(this_type);
  
  explicit smoc_top_moc( sc_module_name name )
    : T_top(name)/*, notagain(false)*/ {
#ifndef __SCFE__
    SC_THREAD(schedule);
#endif
  }
  smoc_top_moc()
    : T_top() {
#ifndef __SCFE__
    SC_THREAD(schedule);
#endif
  }
};

#endif // _INCLUDED_SMOC_MOC_HPP
