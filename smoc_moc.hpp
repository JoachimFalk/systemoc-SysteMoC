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
  
  void getLeafNodes(smoc_node_list &nodes, smoc_graph *node);
  
  void schedule(smoc_graph *c);
};

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
