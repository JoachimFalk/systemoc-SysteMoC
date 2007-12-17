// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_HSCD_NODE_TYPES_HPP
#define _INCLUDED_HSCD_NODE_TYPES_HPP

#include "hscd_tdsim_TraceLog.hpp"

#include "smoc_node_types.hpp"
#include "hscd_op.hpp"

/*
class hscd_choice_node
  : public smoc_root_node {
  private:
    smoc_firing_state start;
  protected:
    void transact( const hscd_op_transact &op ) { 
#ifdef SYSTEMOC_TRACE
      TraceLog.traceStartTransact(this);
#endif
      op.startOp();
#ifdef SYSTEMOC_TRACE
      TraceLog.traceEndTransact(this);
#endif
     }
    void choice( const hscd_op_choice &op )     {
#ifdef SYSTEMOC_TRACE
      TraceLog.traceStartChoice(this);
#endif
       op.startOp();
#ifdef SYSTEMOC_TRACE
      TraceLog.traceEndChoice(this);
#endif
 }
    
  // overloads finalise in smoc_root_node
  // don't calls finalise on _initialState
  // but "finalising" smoc_v1 ports
  void finalise() {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "hscd_choice_node::finalise(), name == " << this->name() << std::endl;
#endif
    
    // Preallocate ID
    smoc_modes::PGWriter::getId(this);
    
    smoc_port_list ports = getPorts();
    
    for (smoc_port_list::iterator iter = ports.begin();
   iter != ports.end();
   ++iter)
      (*iter)->finalise(this);
  }

    hscd_choice_node()
      : smoc_root_node(start) { }
};
*/

class hscd_choice_active_node
: public smoc_root_node {
private:
  smoc_firing_state start;
protected:
  void transact(const hscd_op_transact &op) { 
#ifdef SYSTEMOC_TRACE
    TraceLog.traceStartTransact(this);
#endif
    op.startOp();
#ifdef SYSTEMOC_TRACE
    TraceLog.traceEndTransact(this);
#endif
  }
  void choice(const hscd_op_choice &op) {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceStartChoice(this);
#endif
    op.startOp();
#ifdef SYSTEMOC_TRACE
    TraceLog.traceEndChoice(this);
#endif
  }

  virtual void init()
    { return process(); }

  virtual void process() = 0;

  SC_HAS_PROCESS(hscd_choice_active_node);

  explicit hscd_choice_active_node(sc_module_name name)
    : smoc_root_node(name, start) {
    SC_THREAD(init);
  }
  hscd_choice_active_node()
    : smoc_root_node(sc_gen_unique_name("hscd_choice_active_node"), start) {
    SC_THREAD(init);
  }
//public:
//#ifndef __SCFE__
//  sc_module *myModule() { return this; }
//#endif
};

/*
class hscd_transact_node
  : public hscd_choice_node {
  private:
    // disable
    void choice( const hscd_op_choice &op );
  protected:
    hscd_transact_node()
      : hscd_choice_node() {}
};
 */

class hscd_transact_active_node
: public hscd_choice_active_node {
protected:
  explicit hscd_transact_active_node(sc_module_name name)
    : hscd_choice_active_node(name) {}
  hscd_transact_active_node()
    : hscd_choice_active_node(sc_gen_unique_name("hscd_transact_active_node")) {}
private:
  // disable
  void choice(const hscd_op_choice &op);
//public:
//#ifndef __SCFE__
//  sc_module *myModule() { return this; }
//#endif
};

/*
class hscd_fixed_transact_node
  : public hscd_transact_node {
  private:
    hscd_op_transact op;
    
    // disable
    void transact( const hscd_op_transact &op );
  protected:
    void init()
      { hscd_transact_node::transact(op.onlyInputs()); }
    void transact()
      { hscd_transact_node::transact(op); }
    
    hscd_fixed_transact_node( const hscd_op_transact &op )
      : hscd_transact_node(), op(op) {}
};
*/

class hscd_fixed_transact_active_node
: public hscd_transact_active_node {
private:
  hscd_op_transact op;

  void init() {
    hscd_transact_active_node::transact(op.onlyInputs());
    process();
  }
protected:
  void transact() {
    hscd_transact_active_node::transact(op);
  }

  virtual void process() = 0;

  explicit hscd_fixed_transact_active_node(sc_module_name name, hscd_op_transact op)
    : hscd_transact_active_node(name),
      op(op) {}
  hscd_fixed_transact_active_node(hscd_op_transact op)
    : hscd_transact_active_node(sc_gen_unique_name("hscd_fixed_transact_active_node")),
      op(op) {}
private:
  // disable
  void transact(const hscd_op_transact &op);
//public:
//#ifndef __SCFE__
//  sc_module *myModule() { return this; }
//#endif
};

#endif // _INCLUDED_HSCD_NODE_TYPES_HPP
