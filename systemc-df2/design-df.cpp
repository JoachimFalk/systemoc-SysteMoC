// vim: set sw=2 ts=8:

#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
#include <hscd_node_types.hpp>
#include <hscd_structure.hpp>
#include <hscd_scheduler.hpp>
#include <cstdlib>
#include <iostream>


// queue pair number
typedef int t_qpn;
// transmit token
typedef int t_TT;


class m_h_sched :
	public hscd_transact_active_node {
public:
	hscd_port_out<t_qpn> out_getTT;
	hscd_port_in<t_TT> in_getTT;

        hscd_port_out<void> out_void;

private:
	void process () {
		t_qpn qpn;
		t_TT TT;

		qpn = 42;

		//out_getTT.write(qpn);
		out_getTT[0] = qpn;
		transact(out_getTT(1));
		
		cout << " S> t out\n";

		transact(out_getTT(1) & in_getTT(1));
		cout << " S> t in\n";

		//TT = in_getTT.read();
		TT = in_getTT[0];

		cout << "SCHED> just got TT for qpn " << qpn << ": " << TT << std::endl;
	}

public:
	m_h_sched(sc_module_name name)
	  : hscd_transact_active_node(name) {
//		setTokens(out_getTT, 1);
//		setTokens(in_getTT, 1);
//		SC_THREAD(process);
	}
};


class m_h_transmit_queue :
	public hscd_choice_active_node {
public:
	hscd_port_in<t_qpn> in_getTT;
	hscd_port_in<void> in_void;
	hscd_port_out<t_TT> out_getTT;

private:
	void process () {
		t_qpn qpn;

		while(1) {
		  choice( in_getTT(1) | in_void(1) );
		  
		  if ( in_getTT ) {
		    cout << " TQ> t in\n";
		    
		    //qpn = in_getTT.read();
		    qpn = in_getTT[0];
		    
		    cout << "TQ> TT requested for qpn " << qpn << std::endl;
		    
		    out_getTT[0] = 7;
		    transact(out_getTT(1));
		    cout << " TQ> t out\n";
		  } else {
		    std::cout << "foo" << std::endl;
		  }
		}

	}

public:
	m_h_transmit_queue(sc_module_name name)
	  : hscd_choice_active_node(name) {
//		setTokens(out_getTT, 1);
//		setTokens(in_getTT, 1);
//		SC_THREAD(process);
	}
};

class m_h_foo:
	public hscd_fixed_transact_active_node {
public:
	hscd_port_in<t_qpn> in_getTT;
	hscd_port_out<t_TT> out_getTT;
private:
	void process () {
	  t_qpn qpn;
	  
	  while(1) {
	    out_getTT[0] = in_getTT[0];
	    transact();
	  }
	}
public:
	m_h_foo()
	  :hscd_fixed_transact_active_node( "foo", in_getTT(1) & out_getTT(1) ) {}
};

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

int sc_main (int argc, char **argv) {
  hscd_top top( new m_top("top") );
  
  sc_start(-1);
  return 0;
}
