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

#ifndef _INCLUDED_HSCD_MOC_HPP
#define _INCLUDED_HSCD_MOC_HPP

#include <systemc.h>
#include <hscd_graph_type.hpp>

#include <list>

template <typename T_scheduler, typename T_constraintset>
class hscd_moc
  : public T_constraintset,
    public T_scheduler {
private:
/*
#ifndef __SCFE__
  void process() { return this->schedule(); }
#else */
  void process() {}
/*
#endif
*/
public:
  typedef hscd_moc<T_scheduler, T_constraintset> this_type;
  
  SC_HAS_PROCESS(this_type);
  
  explicit hscd_moc( sc_module_name name )
    : T_constraintset(name), T_scheduler(
        static_cast<typename T_scheduler::cset_ty *>(this)) {
#ifndef __SCFE__
    SC_THREAD(process);
#endif
  }
  hscd_moc()
    : T_constraintset(), T_scheduler(
        static_cast<typename T_scheduler::cset_ty *>(this)) {
#ifndef __SCFE__
    SC_THREAD(process);
#endif
  }

  const sc_module *my_module(void) const { return this; }

#ifndef __SCFE__
  void assemble(smoc_modes::PGWriter &pgw) const {
    return T_constraintset::assemble(pgw); }
  void pgAssemble(sc_module *m, smoc_modes::PGWriter &pgw) const {
    return T_constraintset::pgAssemble(pgw); }
#endif
};

/*
#ifndef __SCFE__
class hscd_moc_scheduler_asap
  : public hscd_fixed_transact_node {
private:
  //    nodes_ty nodes;
  hscd_op_port_or_list    fire_list;
  
  void process() {
    while ( 1 )
      hscd_choice_node::choice( fire_list );
  }
  
  template <typename T>
  void analyse( const std::list<T> &nl ) {
    for ( typename std::list<T>::const_iterator iter = nl.begin();
          iter != nl.end(); ++iter ) {
      typename hscd_rendezvous<void>::chan_type *fire_channel =
        new typename hscd_rendezvous<void>::chan_type(  hscd_rendezvous<void>() );
      hscd_port_out<void>              *fire_port    =
        new hscd_port_out<void>();
      
      (*iter)->fire_port(*fire_channel);
      (*fire_port)(*fire_channel);
      fire_list | (*fire_port)(1);
      // nodes.push_back(*iter);
      // fire.push_back(
    }
  }
protected:
  typedef hscd_fifocsp_constraintset  cset_ty;
public:
  hscd_moc_scheduler_asap(
      hscd_fifocsp_constraintset *c )
    : hscd_fixed_transact_node( hscd_op_transact() ) {
    analyse(c->getNodes()); }
};
#endif
*/

class hscd_moc_scheduler_sdf
/*: public hscd_fixed_transact_node*/ {
protected:
  typedef hscd_sdf_constraintset  cset_ty;
  
#ifndef __SCFE__
private:/*
  //    nodes_ty nodes;
  hscd_op_port_or_list    fire_list;
  
  template <typename T>
  void analyse( const std::list<T> &nl ) {
    for ( typename std::list<T>::const_iterator iter = nl.begin();
          iter != nl.end(); ++iter ) {
      typename hscd_rendezvous<void>::chan_type *fire_channel =
        new typename hscd_rendezvous<void>::chan_type(  hscd_rendezvous<void>() );
      hscd_port_out<void>              *fire_port    =
        new hscd_port_out<void>();
      
      (*iter)->fire_port(*fire_channel);
      (*fire_port)(*fire_channel);
      fire_list | (*fire_port)(1);
      // nodes.push_back(*iter);
      // fire.push_back(
    }
  } */
protected:
  void schedule() {
/*  while ( 1 )
      hscd_choice_node::choice( fire_list ); */
  }
#endif
public:
  template <typename T>
  hscd_moc_scheduler_sdf(T *)
  /*: hscd_fixed_transact_node( hscd_op_transact() )*/ {
#ifndef __SCFE__
//  analyse(c->getNodes());
#endif
  }
};

/*
class hscd_moc_scheduler_csp
  : public hscd_choice_node {
protected:
  typedef hscd_csp_constraintset  cset_ty;
  
#ifndef __SCFE__
private:
  //    nodes_ty nodes;
  hscd_op_port_or_list    fire_list;
  
  template <typename T>
  void analyse( const std::list<T> &nl ) {
    for ( typename std::list<T>::const_iterator iter = nl.begin();
          iter != nl.end(); ++iter ) {
      typename hscd_rendezvous<void>::chan_type *fire_channel =
        new typename hscd_rendezvous<void>::chan_type(  hscd_rendezvous<void>() );
      hscd_port_out<void>              *fire_port    =
        new hscd_port_out<void>();
      
      (*iter)->fire_port(*fire_channel);
      (*fire_port)(*fire_channel);
      fire_list | (*fire_port)(1);
      // nodes.push_back(*iter);
      // fire.push_back(
    }
  }
protected:
  void schedule() {
    while ( 1 )
      hscd_choice_node::choice( fire_list );
  }
#endif
public:
  hscd_moc_scheduler_csp(
      cset_ty *c )
    : hscd_choice_node() {
#ifndef __SCFE__
    analyse(c->getNodes());
#endif
  }
};
*/

template <typename T_constraintset>
class hscd_sdf_moc
  : public hscd_moc<hscd_moc_scheduler_sdf, T_constraintset> {
  public:
    explicit hscd_sdf_moc( sc_module_name name )
      : hscd_moc<hscd_moc_scheduler_sdf, T_constraintset>(name) {}
    hscd_sdf_moc()
      : hscd_moc<hscd_moc_scheduler_sdf, T_constraintset>() {}
};

/*
template <typename T_constraintset>
class hscd_csp_moc
  : public hscd_moc<hscd_moc_scheduler_csp, T_constraintset> {
  public:
    explicit hscd_csp_moc( sc_module_name name )
      : hscd_moc<hscd_moc_scheduler_csp, T_constraintset>(name) {}
    hscd_csp_moc()
      : hscd_moc<hscd_moc_scheduler_csp, T_constraintset>() {}
};
*/

#endif // _INCLUDED_HSCD_MOC_HPP
