// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_MOC_HPP
#define _INCLUDED_HSCD_MOC_HPP

#include <systemc.h>
#include <hscd_graph_type.hpp>

#include <list>

template <typename T_scheduler, typename T_constraintset>
class hscd_moc
  : public T_constraintset,
    public T_scheduler {
private:
  void process() { return schedule(
    static_cast<typename T_scheduler::cset_ty *>(this)); }
public:
  typedef hscd_moc<T_scheduler, T_constraintset> this_type;
  
  SC_HAS_PROCESS(this_type);
  
  explicit hscd_moc( sc_module_name name )
    : T_constraintset(name), T_scheduler(
        static_cast<typename T_scheduler::cset_ty *>(this)) {
#ifndef __SCFE__
    SC_THREAD(process);
#endif
  }
  hscd_moc()
    : T_constraintset(), T_scheduler(
        static_cast<typename T_scheduler::cset_ty *>(this)) {
#ifndef __SCFE__
    SC_THREAD(process);
#endif
  }

#ifndef __SCFE__
  sc_module *myModule() { return this; }

  void assemble( hscd_modes::PGWriter &pgw ) const {
    return T_constraintset::assemble(pgw); }
//  void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
//    return T_constraintset::pgAssemble(pgw); }
#endif
};

class hscd_moc_scheduler_sdf
  : public hscd_fixed_transact_node {
public:
  typedef hscd_moc_scheduler_sdf  this_type;
  typedef hscd_sdf_constraintset  cset_ty;
protected:
  void schedule( cset_ty *c ) {}
private:
  hscd_activation_pattern analyse( cset_ty *c ) const {
    return hscd_activation_pattern();
  }
public:
  hscd_moc_scheduler_sdf( cset_ty *c )
    : hscd_fixed_transact_node(analyse(c)) {}
};

class hscd_moc_scheduler_kpn
  : public hscd_transact_node {
public:
  typedef hscd_moc_scheduler_kpn  this_type;
  typedef hscd_kpn_constraintset  cset_ty;
protected:
  void schedule( cset_ty *c ) {}
private:
  hscd_firing_state analyse( cset_ty *c ) const {
    return hscd_firing_state();
  }
public:
  hscd_moc_scheduler_kpn( cset_ty *c )
    : hscd_transact_node(analyse(c)) {}
};

class hscd_moc_scheduler_ddf
  : public hscd_choice_node {
public:
  typedef hscd_moc_scheduler_ddf  this_type;
  typedef hscd_ddf_constraintset  cset_ty;
protected:
  void schedule( cset_ty *c ) {}
private:
  hscd_firing_state analyse( cset_ty *c ) const {
    return hscd_firing_state();
  }
public:
  hscd_moc_scheduler_ddf( cset_ty *c )
    : hscd_choice_node(analyse(c)) {}
};

class hscd_moc_scheduler_csp
  : public hscd_choice_node {
public:
  typedef hscd_moc_scheduler_csp  this_type;
  typedef hscd_csp_constraintset  cset_ty;
protected:
  void schedule( cset_ty *c ) {
    cset_ty::nodes_ty nodes = c->getNodes();

    for ( cset_ty::nodes_ty::const_iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      std::cout << "foo: " << (*iter)->myModule()->name() << std::endl;
    /*
    while (1) {
      for ( cset_ty::nodes_ty::const_iterator iter = nodes.begin();
            iter != nodes.end();
            ++iter ) {
        const hscd_firing_state &s = (*iter)->currentState();
        
        std::cout << "foo: " << &(*iter)->initialState() << std::endl;
      }
    }*/
  }
private:
/* 
#ifndef __SCFE__
  //    nodes_ty nodes;
  hscd_op_port_or_list    fire_list;
  
  template <typename T>
  void analyse( const std::list<T> &nl ) {
    for ( typename std::list<T>::const_iterator iter = nl.begin();
          iter != nl.end(); ++iter ) {
      typename hscd_rendezvous<void>::chan_type *fire_channel =
        new typename hscd_rendezvous<void>::chan_type(  hscd_rendezvous<void>() );
      hscd_port_out<void>              *fire_port    =
        new hscd_port_out<void>();
      
      (*iter)->fire_port(*fire_channel);
      (*fire_port)(*fire_channel);
      fire_list | (*fire_port)(1);
      // nodes.push_back(*iter);
      // fire.push_back(
    }
  }
protected:
  void schedule() {
    while ( 1 )
      hscd_choice_node::choice( fire_list );
  }
#endif
*/
  hscd_firing_state analyse( cset_ty *c ) const {
    return hscd_firing_state();
  }
public:
  hscd_moc_scheduler_csp( cset_ty *c )
    : hscd_choice_node(analyse(c)) {}
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

#endif // _INCLUDED_HSCD_MOC_HPP
