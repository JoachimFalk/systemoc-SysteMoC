// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <hscd_structure.hpp>
#include <hscd_scheduler.hpp>
#include <hscd_port.hpp>
#include <hscd_rendezvous.hpp>
#include <hscd_node_types.hpp>
#include <hscd_pggen.hpp>

enum dp_forkreq_ty { FORK_TAKE, FORK_DROP };

class dp_fork
  : public hscd_choice_active_node {
public:
  hscd_port_in<dp_forkreq_ty> l_forkreq;
  hscd_port_in<dp_forkreq_ty> r_forkreq;
private:
  void process() {
    while ( 1 ) {
      choice( l_forkreq(1) | r_forkreq(1) );
      assert( (l_forkreq && !r_forkreq && l_forkreq[0] == FORK_TAKE) ||
              (r_forkreq && !l_forkreq && r_forkreq[0] == FORK_TAKE) );
      if ( l_forkreq ) {
        std::cout << "Fork " << name() << " taken by left philosopher !" << std::endl;
        transact( l_forkreq(1) );
        assert( l_forkreq && l_forkreq[0] == FORK_DROP );
        std::cout << "Fork " << name() << " droped by left philosopher !" << std::endl;
      } else {
        std::cout << "Fork " << name() << " taken by right philosopher !" << std::endl;
        transact( r_forkreq(1) );
        assert( r_forkreq && r_forkreq[0] == FORK_DROP );
        std::cout << "Fork " << name() << " droped by right philosopher !" << std::endl;
      }
    }
  }
public:
  dp_fork( sc_module_name name )
    : hscd_choice_active_node(name) {}
};

class dp_footman
  : public hscd_choice_active_node {
public:
  hscd_port_in<void> sitreq_1;
  hscd_port_in<void> sitreq_2;
  hscd_port_in<void> sitreq_3;
  hscd_port_in<void> sitreq_4;
  hscd_port_in<void> sitreq_5;
  hscd_port_in<void> standreq_1;
  hscd_port_in<void> standreq_2;
  hscd_port_in<void> standreq_3;
  hscd_port_in<void> standreq_4;
  hscd_port_in<void> standreq_5;
private:
  void process() {
    int count = 4;
    
    while ( 1 ) {
      if ( count != 0 )
        choice( sitreq_1(1) | standreq_1(1) |
                sitreq_2(1) | standreq_2(1) |
                sitreq_3(1) | standreq_3(1) |
                sitreq_4(1) | standreq_4(1) |
                sitreq_5(1) | standreq_5(1) );
      else
        choice(               standreq_1(1) |
                              standreq_2(1) |
                              standreq_3(1) |
                              standreq_4(1) |
                              standreq_5(1) );
      if ( sitreq_1 || sitreq_2 || sitreq_3 || sitreq_4 || sitreq_5 )
        --count;
      else
        ++count;
    }
  }
public:
  dp_footman( sc_module_name name )
    : hscd_choice_active_node(name) {}
};

class dp_philosopher
  : public hscd_choice_active_node {
public:
  hscd_port_out<dp_forkreq_ty> l_forkreq;
  hscd_port_out<dp_forkreq_ty> r_forkreq;
  hscd_port_out<void>          sitreq;
  hscd_port_out<void>          standreq;
private:
  void process() {
    while ( 1 ) {
      std::cout << "Philosopher " << name() << " want's to eat !" << std::endl;
      transact( sitreq(1) );
      l_forkreq[0] = FORK_TAKE;
      transact( l_forkreq(1) );
      r_forkreq[0] = FORK_TAKE;
      transact( r_forkreq(1) );
      std::cout << "Philosopher " << name() << " eating !" << std::endl;
      wait( sc_time(3,SC_NS) );
      l_forkreq[0] = FORK_DROP;
      transact( l_forkreq(1) );
      r_forkreq[0] = FORK_DROP;
      transact( r_forkreq(1) );
      std::cout << "Philosopher " << name() << " finish eating !" << std::endl;
      transact( standreq(1) );
      wait( sc_time(1,SC_NS) );
    }
  }
public:
  dp_philosopher( sc_module_name name )
    : hscd_choice_active_node(name) {}
};

class m_top
: public hscd_csp_structure {
  private:
    hscd_scheduler_asap *asap;
  public:
    m_top( sc_module_name _name )
      : hscd_csp_structure(_name) {
      dp_fork        &m_fork1        = registerNode(new dp_fork("m_fork1"));
      dp_fork        &m_fork2        = registerNode(new dp_fork("m_fork2"));
      dp_fork        &m_fork3        = registerNode(new dp_fork("m_fork3"));
      dp_fork        &m_fork4        = registerNode(new dp_fork("m_fork4"));
      dp_fork        &m_fork5        = registerNode(new dp_fork("m_fork5"));
      dp_philosopher &m_philosopher1 = registerNode(new dp_philosopher("m_philosopher1"));
      dp_philosopher &m_philosopher2 = registerNode(new dp_philosopher("m_philosopher2"));
      dp_philosopher &m_philosopher3 = registerNode(new dp_philosopher("m_philosopher3"));
      dp_philosopher &m_philosopher4 = registerNode(new dp_philosopher("m_philosopher4"));
      dp_philosopher &m_philosopher5 = registerNode(new dp_philosopher("m_philosopher5"));
      dp_footman     &m_footman      = registerNode(new dp_footman("m_footman"));
      
      connectNodePorts( m_philosopher1.l_forkreq, m_fork1.r_forkreq );
      connectNodePorts( m_philosopher5.r_forkreq, m_fork1.l_forkreq );
      
      connectNodePorts( m_philosopher2.l_forkreq, m_fork2.r_forkreq );
      connectNodePorts( m_philosopher1.r_forkreq, m_fork2.l_forkreq );
      
      connectNodePorts( m_philosopher3.l_forkreq, m_fork3.r_forkreq );
      connectNodePorts( m_philosopher2.r_forkreq, m_fork3.l_forkreq );
      
      connectNodePorts( m_philosopher4.l_forkreq, m_fork4.r_forkreq );
      connectNodePorts( m_philosopher3.r_forkreq, m_fork4.l_forkreq );
      
      connectNodePorts( m_philosopher5.l_forkreq, m_fork5.r_forkreq );
      connectNodePorts( m_philosopher4.r_forkreq, m_fork5.l_forkreq );

      connectNodePorts( m_philosopher1.sitreq,   m_footman.sitreq_1   );
      connectNodePorts( m_philosopher1.standreq, m_footman.standreq_1 );

      connectNodePorts( m_philosopher2.sitreq,   m_footman.sitreq_2   );
      connectNodePorts( m_philosopher2.standreq, m_footman.standreq_2 );

      connectNodePorts( m_philosopher3.sitreq,   m_footman.sitreq_3   );
      connectNodePorts( m_philosopher3.standreq, m_footman.standreq_3 );

      connectNodePorts( m_philosopher4.sitreq,   m_footman.sitreq_4   );
      connectNodePorts( m_philosopher4.standreq, m_footman.standreq_4 );

      connectNodePorts( m_philosopher5.sitreq,   m_footman.sitreq_5   );
      connectNodePorts( m_philosopher5.standreq, m_footman.standreq_5 );
      
      asap = new hscd_scheduler_asap( "asap", getNodes() );
    }
};

int sc_main (int argc, char **argv) {
  try {
    m_top *top;
    hscd_top x( (top = new m_top("top")) );
    
    hscd_modes::dump( std::cout, *top );
    
    sc_start(-1);
  } catch (...) {
    std::cout << "exception !" << std::endl;
    throw;
  }
  return 0;
}
