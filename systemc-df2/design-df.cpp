// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <hscd_moc.hpp>
#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
#include <hscd_node_types.hpp>
#ifndef __SCFE__
//# include <hscd_scheduler.hpp>
# include <hscd_pggen.hpp>
#endif

#define DEBUG 1

// queue pair number
typedef int t_qpn;
// transmit token
typedef int t_TT;

struct markers {
  hscd_firing_state start;
  hscd_firing_state other;
};


class m_h_sched :
        public markers,
	public hscd_choice_passive_node {
public:
	hscd_port_out<t_qpn> out_getTT;
	hscd_port_in<t_TT> in_getTT;

        hscd_port_out<void> out_void;

private:
        
	hscd_firing_state foo() {
          if ( in_getTT[0] == 1 )
            return start;
          else {
            out_getTT[0] = in_getTT[0];
            return other;
          }
	}

        void process(void) {}

public:
	m_h_sched(sc_module_name name)
	  : hscd_choice_passive_node(name,
              start = Choice(
                (in_getTT(1) & out_getTT(1)) >> call(&m_h_sched::process, &start) |
                in_getTT(1) >> branch(&m_h_sched::foo,
                  &start |
                  (other = Transact( out_getTT(1) >> call(&m_h_sched::process, &start) ))
                )
              ) ) {}
};


class m_h_transmit_queue :
        public markers,
	public hscd_transact_passive_node {
public:
	hscd_port_in<t_qpn> in_getTT;
	hscd_port_in<void> in_void;
	hscd_port_out<t_TT> out_getTT;
private:
	hscd_firing_state process () {
          if ( in_getTT[0] == 1 )
            return start;
          else {
            out_getTT[0] = in_getTT[0];
            return other;
          }
	}
        
	void nothing() {}
public:
	m_h_transmit_queue(sc_module_name name)
	  : hscd_transact_passive_node( name,
              start = Transact(
                in_getTT(1) >> branch(&m_h_transmit_queue::process,
                  &start |
                  (other = Transact( out_getTT(1) >> call(&m_h_transmit_queue::nothing, &start) ))
                )
              )
            ) {}
};

class m_h_foo:
	public hscd_fixed_transact_passive_node {
public:
	hscd_port_in<t_qpn> in_getTT;
	hscd_port_out<t_TT> out_getTT;
private:
	void process () {
          out_getTT[0] = in_getTT[0];
	}
public:
	m_h_foo(sc_module_name name)
	  :hscd_fixed_transact_passive_node( name,
              in_getTT(1) & out_getTT(1) ) {
        }
};

/*
class m_top
: public hscd_fifocsp_structure {
  private:
    hscd_scheduler_asap *asap;
  public:
    m_top( sc_module_name _name ): hscd_fifocsp_structure(_name) {
      m_h_sched           &h_sched =
        registerNode(new m_h_sched("h_sched"));
      m_h_transmit_queue  &h_transmit_queue =
        registerNode(new m_h_transmit_queue("h_tranmit_queue"));
      
      connectNodePorts( h_sched.out_getTT, h_transmit_queue.in_getTT );
      connectNodePorts( h_transmit_queue.out_getTT, h_sched.in_getTT );
      connectNodePorts( h_sched.out_void, h_transmit_queue.in_void );
      
      asap = new hscd_scheduler_asap( "asap", getNodes() );
    }
};
*/

int sc_main (int argc, char **argv) {
//  hscd_top top( new m_top("top") );
  m_h_sched sched("sched");
//  sc_start(-1);
  return 0;
}
