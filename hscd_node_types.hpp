// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _INCLUDED_HSCD_NODE_TYPES_HPP
#define _INCLUDED_HSCD_NODE_TYPES_HPP

#include <hscd_tdsim_TraceLog.hpp>

#include <smoc_node_types.hpp>
#include <hscd_op.hpp>

class hscd_choice_node
  : public smoc_root_node {
  private:
    smoc_firing_state start;
  protected:
    void transact( const hscd_op_transact &op ) { 
#ifdef SYSTEMOC_TRACE
      if(dynamic_cast<sc_module*>(this))TraceLog.traceStartTransact( (dynamic_cast<sc_module*>(this))->name() );
      else TraceLog.traceStartTransact( "Noname" );
#endif
       op.startOp();
#ifdef SYSTEMOC_TRACE
      if(dynamic_cast<sc_module*>(this))TraceLog.traceEndTransact( (dynamic_cast<sc_module*>(this))->name() );
      else TraceLog.traceStartTransact( "Noname" );
#endif
     }
    void choice( const hscd_op_choice &op )     {
#ifdef SYSTEMOC_TRACE
      if(dynamic_cast<sc_module*>(this))TraceLog.traceStartChoice( (dynamic_cast<sc_module*>(this))->name() );
      else TraceLog.traceStartTransact( "Noname" );
#endif
       op.startOp();
#ifdef SYSTEMOC_TRACE
      if(dynamic_cast<sc_module*>(this))TraceLog.traceEndChoice( (dynamic_cast<sc_module*>(this))->name() );
      else TraceLog.traceStartTransact( "Noname" );
#endif
 }
    
  // overloads finalise in smoc_root_node
  // don't calls finalise on _initialState
  // but "finalising" smoc_v1 ports
  void finalise() {
#ifdef SYSTEMOC_DEBUG
    std::cerr << myModule()->name() << ": finalise" << std::endl;
#endif

    smoc_port_list ports = getPorts();

    for (smoc_port_list::iterator iter = ports.begin();
	 iter != ports.end();
	 ++iter)
      (*iter)->finalise();
  }

    hscd_choice_node()
      : smoc_root_node(start) { }
};

class hscd_choice_active_node
  : public sc_module,
    public hscd_choice_node {
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_choice_active_node);
    
    explicit hscd_choice_active_node( sc_module_name name )
      : sc_module(name) {
      SC_THREAD(process);
    }
    hscd_choice_active_node()
      : sc_module(sc_gen_unique_name("hscd_choice_active_node")) {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
#endif
};

class hscd_transact_node
  : public hscd_choice_node {
  private:
    /* disable */
    void choice( const hscd_op_choice &op );
  protected:
    hscd_transact_node()
      : hscd_choice_node() {}
};

class hscd_transact_active_node
  : public sc_module,
    public hscd_transact_node {
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_transact_active_node);
    
    explicit hscd_transact_active_node( sc_module_name name )
      : sc_module(name) {
      SC_THREAD(process);
    }
    hscd_transact_active_node()
      : sc_module(sc_gen_unique_name("hscd_transact_active_node")) {
      SC_THREAD(process);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
#endif
};

class hscd_fixed_transact_node
  : public hscd_transact_node {
  private:
    hscd_op_transact op;
    
    /* disable */
    void transact( const hscd_op_transact &op );
  protected:
    void init()
      { hscd_transact_node::transact(op.onlyInputs()); }
    void transact()
      { hscd_transact_node::transact(op); }
    
    hscd_fixed_transact_node( const hscd_op_transact &op )
      : hscd_transact_node(), op(op) {}
};

class hscd_fixed_transact_active_node
  : public sc_module,
    public hscd_fixed_transact_node {
  private:
    void init() {
      hscd_fixed_transact_node::init();
      process();
    }
  protected:
    virtual void process() = 0;
    
    SC_HAS_PROCESS(hscd_fixed_transact_active_node);
    
    explicit hscd_fixed_transact_active_node( sc_module_name name, hscd_op_transact op )
      : sc_module(name),
        hscd_fixed_transact_node(op) {
      SC_THREAD(init);
    }
    hscd_fixed_transact_active_node( hscd_op_transact op )
      : sc_module(sc_gen_unique_name("hscd_fixed_transact_active_node")),
        hscd_fixed_transact_node(op) {
      SC_THREAD(init);
    }
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
#endif
};

#endif // _INCLUDED_HSCD_NODE_TYPES_HPP
