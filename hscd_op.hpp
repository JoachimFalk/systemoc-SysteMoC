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

#ifndef _INCLUDED_HSCD_OP_HPP
#define _INCLUDED_HSCD_OP_HPP

#include <smoc_root_port.hpp>
#include <smoc_event.hpp>
#include <hscd_root_port_list.hpp>

#include <systemc.h>

#include <map>

#include <hscd_tdsim_TraceLog.hpp>

//template <typename T> class hscd_op;

template <typename T>
class hscd_op {
public:
  friend class hscd_choice_node;
  
  typedef T				running_op_type;
  typedef typename T::op_list_type	op_list_type;
  typedef hscd_op<T>			this_type;
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
  typedef std::map<smoc_event *, const hscd_op_port *>  pm_ty;
  
  pm_ty     pm;
  sc_event  e;
  
  void signaled( smoc_event *se ) {
    pm_ty::iterator iter = pm.find(se);
    
    assert( iter != pm.end() );
    if ( iter->second->isReady() ) {
      pm.erase(iter);
      if ( pm.empty() )
        e.notify();
    } else
      se->addListener(this);
  }
  
  hscd_running_op_transact( const hscd_op_port_base_list &pl ) {
#ifdef SYSTEMOC_DEBUG
    std::cout << "<hscd_running_op_transact id=\"" << this << "\">" << std::endl;
#endif
    for ( hscd_op_port_base_list::const_iterator iter = pl.begin();
          iter != pl.end();
          ++iter ) {
      if ( !iter->isReady() ) {
        smoc_event &se = iter->blockEvent();
        se.addListener(this);
        pm[&se] = &*iter;
      }
    }
    if ( !pm.empty() ) {
#ifdef SYSTEMOC_DEBUG
      std::cout << "  <blocked id=\"" << this << "\">" << std::endl;
#endif
#ifdef SYSTEMOC_TRACE
      TraceLog.traceBlockingWaitStart();
#endif
      wait(e);
#ifdef SYSTEMOC_TRACE
      TraceLog.traceBlockingWaitEnd();
#endif
#ifdef SYSTEMOC_DEBUG
      std::cout << "  </blocked id=\"" << this << "\">" << std::endl;
#endif
    }
    assert( pm.empty() );
    for ( hscd_op_port_base_list::const_iterator iter = pl.begin();
          iter != pl.end();
          ++iter )
      iter->communicate();
#ifdef SYSTEMOC_DEBUG
    std::cout << "</hscd_running_op_transact id=\"" << this << "\">" << std::endl;
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
  typedef std::map<smoc_event *, const hscd_op_port *>  pm_ty;
  
  pm_ty               pm;
  sc_event            e;
  const hscd_op_port *ready;
  
  void signaled( smoc_event *se ) {
    pm_ty::iterator iter = pm.find(se);
    
    assert( iter != pm.end() );
    if ( iter->second->isReady() ) {
      ready = iter->second;
      pm.erase(iter);
      e.notify();
    } else
      se->addListener(this);
  }
  
  hscd_running_op_choice( const hscd_op_port_base_list &pl )
    : ready(NULL) {
#ifdef SYSTEMOC_DEBUG
    std::cout << "<hscd_running_op_choice id=\"" << this << "\">" << std::endl;
#endif
    for ( hscd_op_port_base_list::const_iterator iter = pl.begin();
          iter != pl.end();
          ++iter ) {
      if ( !iter->isReady() ) {
        smoc_event &se = iter->blockEvent();
        se.addListener(this);
        pm[&se] = &*iter;
      } else {
        ready = &*iter;
      }
    }
    if ( ready == NULL ) {
#ifdef SYSTEMOC_DEBUG
      std::cout << "  <blocked id=\"" << this << "\">" << std::endl;
#endif
#ifdef SYSTEMOC_TRACE
      TraceLog.traceBlockingWaitStart();
#endif
      wait(e);
#ifdef SYSTEMOC_TRACE
      TraceLog.traceBlockingWaitEnd();
#endif
#ifdef SYSTEMOC_DEBUG
      std::cout << "  </blocked id=\"" << this << "\">" << std::endl;
#endif
    }
    assert( ready != NULL );
    for ( pm_ty::iterator iter = pm.begin();
          iter != pm.end();
          ++iter )
      iter->first->delListener(this);
    assert( ready->isReady() );
    ready->communicate();
#ifdef SYSTEMOC_DEBUG
    std::cout << "</hscd_running_op_choice id=\"" << this << "\">" << std::endl;
#endif
  }

  virtual ~hscd_running_op_choice() {}
};
 
typedef hscd_op<hscd_running_op_transact>	hscd_op_transact;
typedef hscd_op<hscd_running_op_choice>		hscd_op_choice;

#endif // _INCLUDED_HSCD_OP_HPP
