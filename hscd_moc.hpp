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
  public:
    explicit hscd_moc( sc_module_name name )
      : T_constraintset(name), T_scheduler(
          static_cast<typename T_scheduler::cset_ty *>(this)) {}
    hscd_moc()
      : T_constraintset(), T_scheduler(
          static_cast<typename T_scheduler::cset_ty *>(this)) {}

#ifndef __SCFE__
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return T_constraintset::assemble(pgw); }
    void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
      return T_constraintset::pgAssemble(pgw); }
#endif
};

class hscd_moc_scheduler_asap
  : public hscd_fixed_transact_node {
private:
  //    nodes_ty nodes;
  hscd_op_port_or_list    fire_list;
  
  void process() {
    while ( 1 )
      hscd_choice_node::choice( fire_list );
  }
  
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
  typedef hscd_fifocsp_constraintset  cset_ty;
public:
  hscd_moc_scheduler_asap(
      hscd_fifocsp_constraintset *c )
    : hscd_fixed_transact_node( hscd_op_transact() ) {
    analyse(c->getNodes()); }
};

class hscd_moc_scheduler_sdf
  : public hscd_fixed_transact_node {
private:
  //    nodes_ty nodes;
  hscd_op_port_or_list    fire_list;
  
  void process() {
    std::cout << "hscd_moc_scheduler_sdf" << std::endl;
    while ( 1 )
      hscd_choice_node::choice( fire_list );
  }
  
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
  typedef hscd_sdf_constraintset  cset_ty;
public:
  hscd_moc_scheduler_sdf(
      cset_ty *c )
    : hscd_fixed_transact_node( hscd_op_transact() ) {
    analyse(c->getNodes()); }
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

#endif // _INCLUDED_HSCD_MOC_HPP
