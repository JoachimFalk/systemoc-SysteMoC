// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <hscd_moc.hpp>
#include <hscd_port.hpp>
#include <hscd_rendezvous.hpp>
#include <hscd_node_types.hpp>
#ifndef __SCFE__
//# include <hscd_scheduler.hpp>
# include <hscd_pggen.hpp>
#endif

enum dp_forkreq_ty { FORK_TAKE, FORK_DROP };

class dp_fork
  : public hscd_choice_passive_node {
public:
  hscd_port_in<dp_forkreq_ty> l_forkreq;
  hscd_port_in<dp_forkreq_ty> r_forkreq;
private:
  void l_forkreq_take() { assert(l_forkreq[0] == FORK_TAKE);
    std::cout << "Fork " << name() << " taken by left philosopher !" << std::endl; }
  void l_forkreq_drop() { assert(l_forkreq[0] == FORK_DROP);
    std::cout << "Fork " << name() << " droped by left philosopher !" << std::endl; }
  void r_forkreq_take() { assert(r_forkreq[0] == FORK_TAKE);
    std::cout << "Fork " << name() << " taken by right philosopher !" << std::endl; }
  void r_forkreq_drop() { assert(r_forkreq[0] == FORK_DROP);
    std::cout << "Fork " << name() << " droped by right philosopher !" << std::endl; }
  
  hscd_firing_state fireRules() {
    hscd_firing_state start =
      Choice( l_forkreq(1) >> call(&dp_fork::l_forkreq_take,
                Choice( l_forkreq(1) >> call(&dp_fork::l_forkreq_drop, &start) ) ) |
              r_forkreq(1) >> call(&dp_fork::r_forkreq_take,
                Choice( r_forkreq(1) >> call(&dp_fork::r_forkreq_drop, &start) ) )
            );
    return start;
  }
public:
  dp_fork( sc_module_name name )
    : hscd_choice_passive_node(name, fireRules() ) {}
};

class dp_footman
  : public hscd_choice_passive_node {
public:
  hscd_port_in<void> sitreq_0;
  hscd_port_in<void> sitreq_1;
  hscd_port_in<void> standreq_0;
  hscd_port_in<void> standreq_1;
private:
  void philosopher0_sitdown() {
    std::cout << "Seat " << name() << " taken by philosopher 0 !" << std::endl; }
  void philosopher0_standup() {
    std::cout << "Seat " << name() << " vacated by philosopher 0 !" << std::endl; }
  void philosopher1_sitdown() {
    std::cout << "Seat " << name() << " taken by philosopher 1 !" << std::endl; }
  void philosopher1_standup() {
    std::cout << "Seat " << name() << " vacated by philosopher 1 !" << std::endl; }
  
  hscd_firing_state fireRules() {
    hscd_firing_state start =
      Choice( sitreq_0(1) >> call(&dp_footman::philosopher0_sitdown,
                Choice( standreq_0(1) >> call(&dp_footman::philosopher0_standup, &start ) ) ) |
              sitreq_1(1) >> call(&dp_footman::philosopher1_sitdown,
                Choice( standreq_1(1) >> call(&dp_footman::philosopher1_standup, &start ) ) )
            );
    return start;
  }
public:
  dp_footman( sc_module_name name )
    : hscd_choice_passive_node(name, fireRules() ) {}
};

class dp_philosopher
  : public hscd_choice_passive_node {
public:
  hscd_port_out<dp_forkreq_ty> l_forkreq;
  hscd_port_out<dp_forkreq_ty> r_forkreq;
  hscd_port_out<void>          sitreq;
  hscd_port_out<void>          standreq;
private:
  void nothing() {}
  
  void eat() {
    std::cout << "Philosopher " << name() << " eating !" << std::endl;
    wait( sc_time(3,SC_NS) );
    l_forkreq[0] = r_forkreq[0] = FORK_DROP;
  }
  
  void think() {
    std::cout << "Philosopher " << name() << " thinking !" << std::endl;
    wait( sc_time(100,SC_NS) );
    l_forkreq[0] = r_forkreq[0] = FORK_TAKE; 
  }
  
  hscd_firing_state fireRules() {
    hscd_firing_state start =
      Choice( sitreq(1) >> call(&dp_philosopher::nothing,
        Choice( l_forkreq(1) >> call(&dp_philosopher::nothing,
          Choice( r_forkreq(1) >> call(&dp_philosopher::eat,
            Choice( r_forkreq(1) >> call(&dp_philosopher::nothing,
              Choice( l_forkreq(1) >> call(&dp_philosopher::nothing,
                Choice( standreq(1) >> call(&dp_philosopher::think, &start ) )
              ) )
            ) )
          ) )
        ) )
      ) );
    return start;
  }
public:
  dp_philosopher( sc_module_name name )
    : hscd_choice_passive_node(name, fireRules() ) {
    l_forkreq[0] = r_forkreq[0] = FORK_TAKE; 
  }
};

class m_top
: public hscd_csp_constraintset {
  public:
    m_top( sc_module_name name )
      : hscd_csp_constraintset(name) {
      std::cout << "Instantiating " << name << " !" << std::endl;
      std::cout << "Instantiating fork0 !" << std::endl;
      dp_fork        &m_fork0        = registerNode(new dp_fork("m_fork0"));
      std::cout << "Instantiating fork1 !" << std::endl;
      dp_fork        &m_fork1        = registerNode(new dp_fork("m_fork1"));
      std::cout << "Instantiating philosopher0 !" << std::endl;
      dp_philosopher &m_philosopher0 = registerNode(new dp_philosopher("m_philosopher0"));
      std::cout << "Instantiating philosopher1 !" << std::endl;
      dp_philosopher &m_philosopher1 = registerNode(new dp_philosopher("m_philosopher1"));
      std::cout << "Instantiating footman !" << std::endl;
      dp_footman     &m_footman      = registerNode(new dp_footman("m_footman"));
      
      connectNodePorts( m_philosopher0.l_forkreq, m_fork0.r_forkreq );
      connectNodePorts( m_philosopher1.r_forkreq, m_fork0.l_forkreq );
      
      connectNodePorts( m_philosopher1.l_forkreq, m_fork1.r_forkreq );
      connectNodePorts( m_philosopher0.r_forkreq, m_fork1.l_forkreq );
      
      connectNodePorts( m_philosopher0.sitreq,   m_footman.sitreq_0   );
      connectNodePorts( m_philosopher0.standreq, m_footman.standreq_0 );

      connectNodePorts( m_philosopher1.sitreq,   m_footman.sitreq_1   );
      connectNodePorts( m_philosopher1.standreq, m_footman.standreq_1 );
      std::cout << "Instantiating " << name << " finished !" << std::endl;
    }
};

int sc_main (int argc, char **argv) {
#ifndef __SCFE__
  try {
#endif
    hscd_csp_moc<m_top> *top = new hscd_csp_moc<m_top>("top");
#ifndef __SCFE__
//    hscd_top x(top);
    
    std::cout << "Was here !" << std::endl;
    hscd_modes::dump( std::cout, *top );
    
    sc_start(-1);
  } catch (...) {
    std::cout << "exception !" << std::endl;
    throw;
  }
#endif
  return 0;
}
