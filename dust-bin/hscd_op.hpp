// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_OP_HPP
#define _INCLUDED_HSCD_OP_HPP

#include <systemc.h>
#include <hscd_root_port.hpp>
// #include <oneof.hpp>

#include <commondefs.h>

//template <typename T> class hscd_op;

//class hscd_root_node_op_if;
//class hscd_running_op_base;

//class hscd_do_op_if {
//  private:
//    friend class hscd_root_node_op_if;
//  public:
//    virtual ~hscd_do_op_if() {}
//  private:
//    virtual hscd_running_op_base *_startOp(hscd_root_node_op_if *node) = 0;
//    void startOp(hscd_root_node_op_if *node);
//};

class hscd_root_node_op_if {
  private:
    friend class hscd_running_op_base;
    friend class hscd_do_op_if;
    
    //hscd_running_op_base *_opRunning_;
    //oneof<hscd_running_op_transact,hscd_running_op_choice> runningOp;
    hscd_firing_state _initialState;
    hscd_firing_state _currentState;
    
    //void _opFinished();
    //void _opRunning( hscd_running_op_base *op ) {
    //  assert( _opRunning_ == NULL );
    //  _opRunning_ = op;
    //}
  protected:
    hscd_root_node_op_if(const hscd_firing_state &s)
      : _initialState(s), _currentState(s) {}
    
    //virtual void opFinished() = 0;
    //bool finished() const { return _opRunning_ == NULL; }
    
    //void startOp( hscd_do_op_if &op ) { op.startOp(this); }
  public:
    virtual ~hscd_root_node_op_if() {}

    const hscd_firing_state &currentState() const { return _currentState; }
};

/*
#ifndef _COMPILEHEADER_HSCD_DO_OP_IF__START_OP
GNU89_EXTERN_INLINE
#endif
void hscd_do_op_if::startOp(hscd_root_node_op_if *node) {
  node->_opRunning( _startOp(node) );
}

class hscd_running_op_base {
  private:
    hscd_root_node_op_if *notify;
  protected:
    hscd_running_op_base( hscd_root_node_op_if *notify )
      :notify(notify) {}
    
    void finished() { notify->_opFinished(); }
  public:
    virtual ~hscd_running_op_base() {}
};

#ifndef _COMPILEHEADER_HSCD_DO_OP_IF___OP_FINISHED
GNU89_EXTERN_INLINE
#endif
void hscd_root_node_op_if::_opFinished() {
  assert( _opRunning_ != NULL );
  delete _opRunning_; _opRunning_ = NULL;
  opFinished();
}

template <typename T>
class hscd_op:
  public hscd_do_op_if {
  public:
    friend class hscd_root_node;
    
    typedef T				running_op_type;
    typedef typename T::op_list_type	op_list_type;
    typedef hscd_op<T>			this_type;
  private:
    hscd_op_port_base_list *pl;
    
    hscd_running_op_base *_startOp(hscd_root_node_op_if *node) {
      assert( node != NULL );
      running_op_type *op = new running_op_type(*pl,node);
      if ( op->isFinished() ) {
	delete op; op = NULL;
      }
      return op;
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

class hscd_running_op_transact
  : private hscd_port_op_if,
    public hscd_running_op_base {
  public:
    typedef hscd_running_op_transact   this_type;
    typedef hscd_op_port_and_list   op_list_type;
  private:
    friend class hscd_op<this_type>;
    
    hscd_op_port_base_list &pl;
    size_t                 outstanding;
    
    bool isFinished() const { return outstanding == 0; }
    
    void incommingTransfer( hscd_root_port &port ) {
      if ( ready(port) ) {
	finishTransfer(port);
	--outstanding;
	if ( isFinished() )
	  finished();
      }
    }
  protected:
    hscd_running_op_transact
      ( hscd_op_port_base_list &pl, hscd_root_node_op_if *node )
      : hscd_running_op_base(node), pl(pl), outstanding(0) {
      for ( hscd_op_port_base_list::iterator iter = pl.begin();
	    iter != pl.end();
	    ++iter ) {
	requestTransfer(*iter);
	if ( !ready(*iter) )
	  ++outstanding;
      }
    }
  public:
};

class hscd_running_op_choice
  : private hscd_port_op_if,
    public hscd_running_op_base {
  public:
    typedef hscd_running_op_choice     this_type;
    typedef hscd_op_port_or_list    op_list_type;
  private:
    friend class hscd_op<this_type>;
    
    hscd_op_port_base_list &pl;
    enum {
      OP_WAIT_ALL,
      OP_WAIT_READY,
      OP_READY }            opstate;
    
    bool isFinished() const { return opstate == OP_READY; }
    
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
  protected:
    hscd_running_op_choice
      ( hscd_op_port_base_list &pl, hscd_root_node_op_if *node )
      : hscd_running_op_base(node), pl(pl), opstate(OP_WAIT_ALL) {
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
    }
  public:
};



typedef hscd_op<hscd_running_op_transact>	hscd_op_transact;
typedef hscd_op<hscd_running_op_choice>		hscd_op_choice;
*/

#endif // _INCLUDED_HSCD_OP_HPP
