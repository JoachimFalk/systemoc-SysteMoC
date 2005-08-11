// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_OP_HPP
#define _INCLUDED_HSCD_OP_HPP

#include <smoc_root_port.hpp>
#include <hscd_root_port_list.hpp>

#include <systemc.h>

//template <typename T> class hscd_op;

template <typename T>
class hscd_op {
  public:
    friend class hscd_choice_node;
    
    typedef T				running_op_type;
    typedef typename T::op_list_type	op_list_type;
    typedef hscd_op<T>			this_type;
  private:
    hscd_op_port_base_list *pl;
    
    void startOp() const {
      sc_event w;
      
      running_op_type op(*pl,w);
      
      if ( !op.isFinished() ) {
        // wait till finished
        wait(w);
        assert( op.isFinished() );
      }
    }
    
    hscd_op( hscd_op_port_base_list &pl )
      :pl(&pl) {}
  public:
    hscd_op()
      :pl() {}
    hscd_op( op_list_type &pl )
      :pl(&pl) {}
    hscd_op( hscd_op_port p )
      :pl(&static_cast<hscd_op_port_list &>(p)) {}
    
    this_type onlyInputs() {
      return this_type(pl->onlyInputs());
    }
    this_type onlyOutputs() {
      return this_type(pl->onlyOutputs());
    }
};

class hscd_running_op_transact {
  public:
    typedef hscd_running_op_transact   this_type;
    typedef hscd_op_port_and_list   op_list_type;
  private:
    friend class hscd_op<this_type>;
    
    hscd_op_port_base_list &pl;
    sc_event               &e;
    size_t                 outstanding;
    
    bool isFinished() const { return outstanding == 0; }

  /*
    void incommingTransfer( hscd_root_port &port ) {
      if ( ready(port) ) {
	finishTransfer(port);
	--outstanding;
	if ( isFinished() )
	  finished();
      }
    }*/
  protected:
    hscd_running_op_transact
      ( hscd_op_port_base_list &pl, sc_event &e )
      : pl(pl), e(e), outstanding(0) {
/*
      for ( hscd_op_port_base_list::iterator iter = pl.begin();
	    iter != pl.end();
	    ++iter ) {
	requestTransfer(*iter);
	if ( !ready(*iter) )
	  ++outstanding;
      }
*/
    }
  public:
};

class hscd_running_op_choice {
  public:
    typedef hscd_running_op_choice     this_type;
    typedef hscd_op_port_or_list    op_list_type;
  private:
    friend class hscd_op<this_type>;
    
    hscd_op_port_base_list &pl;
    sc_event               &e;
    enum {
      OP_WAIT_ALL,
      OP_WAIT_READY,
      OP_READY }            opstate;
    
    bool isFinished() const { return opstate == OP_READY; }
/* 
    void incommingTransfer( hscd_root_port &port ) {
      //std::cerr << "incommingTransfer " << opstate << std::endl;
      if ( opstate == OP_WAIT_ALL ) {
	assert( started(port) );
	for ( hscd_op_port_base_list::iterator aiter = pl.begin();
	      aiter != pl.end();
	      ++aiter )
	  if ( static_cast<hscd_root_port &>(*aiter) != port )
	    cancelTransfer(*aiter);
	opstate = OP_WAIT_READY;
      }
      assert( opstate == OP_WAIT_READY );
      if (ready(port)) {
	opstate = OP_READY;
	finishTransfer(port);
	finished();
      }
    }
*/
  protected:
    hscd_running_op_choice
      ( hscd_op_port_base_list &pl, sc_event &e )
      : pl(pl), e(e), opstate(OP_WAIT_ALL) {
  /*
      for ( hscd_op_port_base_list::iterator aiter = pl.begin();
	    aiter != pl.end();
	    ++aiter ) {
	requestTransfer(*aiter);
	if ( started(*aiter) ) {
	  hscd_op_port_base_list::iterator biter;
	  
	  for ( biter = pl.begin();
		biter != aiter;
		++biter )
	    cancelTransfer(*biter);
	  for ( ++biter;
		biter != pl.end();
		++biter )
	    clearReady(*biter);
	  opstate = ready(*aiter)
	    ? OP_READY
	    : OP_WAIT_READY;
	  break;
	}
      }
    */
    }
  public:
};

typedef hscd_op<hscd_running_op_transact>	hscd_op_transact;
typedef hscd_op<hscd_running_op_choice>		hscd_op_choice;

#endif // _INCLUDED_HSCD_OP_HPP
