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

#ifndef _INCLUDED_HSCD_OP_HPP
#define _INCLUDED_HSCD_OP_HPP

#include "smoc_root_port.hpp"
#include "smoc_event.hpp"
#include "hscd_root_port_list.hpp"

#include <systemc.h>

#include <map>

#include "hscd_tdsim_TraceLog.hpp"

//template <typename T> class hscd_op;

template <typename T>
class hscd_op {
public:
  friend class hscd_choice_active_node;
  
  typedef T        running_op_type;
  typedef typename T::op_list_type  op_list_type;
  typedef hscd_op<T>      this_type;
private:
  hscd_op_port_base_list pl;
  
  void startOp() const {
    // clear all ready flags of ports
    for ( hscd_op_port_base_list::const_iterator iter = pl.begin();
          iter != pl.end();
          ++iter )
      iter->clearReady();
    running_op_type op(pl);
  }
  
  hscd_op( const hscd_op_port_base_list &pl )
    :pl(pl) {}
public:
  hscd_op( const op_list_type &pl )
    :pl(pl) {}
  hscd_op( const hscd_op_port &p )
    :pl(p) {}
  
  this_type onlyInputs()
    { return this_type(pl.onlyInputs()); }
  this_type onlyOutputs()
    { return this_type(pl.onlyOutputs()); }
};

class hscd_running_op_transact
: protected smoc_event_listener {
public:
  typedef hscd_running_op_transact  this_type;
  typedef hscd_op_port_and_list     op_list_type;
  
  friend class hscd_op<this_type>;
protected:
  typedef std::map<smoc_event_waiter *, const hscd_op_port *>  pm_ty;
  
  pm_ty     pm;
  sc_event  e;

  void signaled( smoc_event_waiter *se ) {
    // Reset event to be notified again !
    bool retval          = true;
    pm_ty::iterator iter = pm.find(se);
    
    assert(*se && iter != pm.end());
    if ( iter->second->isReady() ) {
      pm.erase(iter);
      se->delListener(this);
      iter->second->communicate();
      if ( pm.empty() )
        e.notify();
      // Do not need new notification
      retval = false;
    }
//#ifdef SYSTEMOC_DEBUG
//    std::cerr << "hscd_running_op_transact::signaled "
//      "this == " << this << ", "
//      "missing == " << pm.size() <<
//      (retval ? "" : " [HIT]" ) << std::endl;
//#endif
    assert(retval == false);
  }

  void eventDestroyed(smoc_event_waiter *) {}

  hscd_running_op_transact( const hscd_op_port_base_list &pl ) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<hscd_running_op_transact id=\"" << this << "\">" << std::endl;
#endif
    for ( hscd_op_port_base_list::const_iterator iter = pl.begin();
          iter != pl.end();
          ++iter ) {
      if ( !iter->isReady() ) {
        smoc_event &se = iter->blockEvent();
        se.reset();
        se.addListener(this);
        pm[&se] = &*iter;
      } else
        iter->communicate();
    }
    if ( !pm.empty() ) {
#ifdef SYSTEMOC_DEBUG
      std::cerr << "  <blocked id=\"" << this << "\">" << std::endl;
#endif
#ifdef SYSTEMOC_TRACE
      TraceLog.traceBlockingWaitStart();
#endif
      wait(e);
#ifdef SYSTEMOC_TRACE
      TraceLog.traceBlockingWaitEnd();
#endif
#ifdef SYSTEMOC_DEBUG
      std::cerr << "  </blocked id=\"" << this << "\">" << std::endl;
#endif
    }
    assert( pm.empty() );
#ifdef SYSTEMOC_DEBUG
    std::cerr << "</hscd_running_op_transact id=\"" << this << "\">" << std::endl;
#endif
  }

  virtual ~hscd_running_op_transact() {}
};

class hscd_running_op_choice
: protected smoc_event_listener {
public:
  typedef hscd_running_op_choice    this_type;
  typedef hscd_op_port_or_list      op_list_type;
  
  friend class hscd_op<this_type>;
protected:
  typedef std::map<smoc_event_waiter *, const hscd_op_port *>  pm_ty;
  
  pm_ty               pm;
  sc_event            e;
  const hscd_op_port *ready;
  
  void signaled( smoc_event_waiter *se ) {
    // Reset event to be notified again !
    bool retval          = true;
    pm_ty::iterator iter = pm.find(se);
    
    assert(*se && iter != pm.end());
    if (iter->second->isReady() && !ready) {
      ready = iter->second;
      ready->communicate();
      pm.erase(iter);
      se->delListener(this);
      // Do not need new notification
      retval = false;
    }
//#ifdef SYSTEMOC_DEBUG
//    std::cerr << "hscd_running_op_choice::signaled this == " << this << ", missing == " << pm.size() << std::endl;
//#endif
    assert(retval == false);
  }

  void eventDestroyed(smoc_event_waiter *) {}

  hscd_running_op_choice( const hscd_op_port_base_list &pl )
    : ready(NULL) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "<hscd_running_op_choice id=\"" << this << "\">" << std::endl;
#endif
    for ( hscd_op_port_base_list::const_iterator iter = pl.begin();
          iter != pl.end();
          ++iter ) {
      if ( !iter->isReady() ) {
        smoc_event &se = iter->blockEvent();
        se.reset();
        se.addListener(this);
        pm[&se] = &*iter;
      } else {
        ready = &*iter;
        ready->communicate();
      }
    }
    if ( ready == NULL ) {
#ifdef SYSTEMOC_DEBUG
      std::cerr << "  <blocked id=\"" << this << "\">" << std::endl;
#endif
#ifdef SYSTEMOC_TRACE
      TraceLog.traceBlockingWaitStart();
#endif
      wait(e);
#ifdef SYSTEMOC_TRACE
      TraceLog.traceBlockingWaitEnd();
#endif
#ifdef SYSTEMOC_DEBUG
      std::cerr << "  </blocked id=\"" << this << "\">" << std::endl;
#endif
    }
    assert( ready != NULL );
    for ( pm_ty::iterator iter = pm.begin();
          iter != pm.end();
          ++iter )
      iter->first->delListener(this);
#ifdef SYSTEMOC_DEBUG
    std::cerr << "</hscd_running_op_choice id=\"" << this << "\">" << std::endl;
#endif
  }

  virtual ~hscd_running_op_choice() {}
};
 
typedef hscd_op<hscd_running_op_transact>  hscd_op_transact;
typedef hscd_op<hscd_running_op_choice>    hscd_op_choice;

#endif // _INCLUDED_HSCD_OP_HPP
