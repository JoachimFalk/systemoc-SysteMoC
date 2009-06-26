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

#ifndef _INCLUDED_DETAIL_SMOC_EVENT_EXPR_HPP
#define _INCLUDED_DETAIL_SMOC_EVENT_EXPR_HPP

#include <systemoc/smoc_config.h>

#include "smoc_event_decls.hpp"
#include "../smoc_expr.hpp"
#include "../smoc_ast_systemoc.hpp"

namespace Expr {

/****************************************************************************
 * DSMOCEvent represents a smoc_event guard which turns true if the event is
 * signaled
 */

class DSMOCEvent {
public:
  typedef bool       value_type;
  typedef DSMOCEvent this_type;

  friend class Value<this_type>;
  friend class AST<this_type>;
  friend class Sensitivity<this_type>;
private:
  smoc_event_waiter &v;
public:
  explicit DSMOCEvent(smoc_event_waiter &v): v(v) {}
};

template <>
struct Value<DSMOCEvent> {
  typedef Expr::Detail::ENABLED result_type;

  static inline
  result_type apply(const DSMOCEvent &e) {
#if defined(SYSTEMOC_ENABLE_DEBUG)
    assert(e.v);
#endif
    return result_type();
  }
};

template <>
struct Sensitivity<DSMOCEvent> {
  typedef Detail::Process      match_type;

  typedef void                 result_type;
  typedef smoc_event_and_list &param1_type;

  static inline
  void apply(const DSMOCEvent &e, smoc_event_and_list &al) {
    al &= e.v;
//#ifdef SYSTEMOC_DEBUG
//    std::cerr << "Sensitivity<DSMOCEvent>::apply(...) al == " << al << std::endl;
//#endif
  }
};

template <>
struct AST<DSMOCEvent> {
  typedef PASTNode result_type;

  static inline
  result_type apply(const DSMOCEvent &e)
    { return PASTNode(new ASTNodeSMOCEvent()); }
};

template <>
struct D<DSMOCEvent>: public DBase<DSMOCEvent> {
  D(smoc_event_waiter &v): DBase<DSMOCEvent>(DSMOCEvent(v)) {}
};

// Make a convenient typedef for the placeholder type.
struct SMOCEvent { typedef D<DSMOCEvent> type; };

// smoc_event_waiter may be an event or a event list
// till-waiting for events allows for hierarchical graph scheduling
static inline
SMOCEvent::type till(smoc_event_waiter &e)
  { return SMOCEvent::type(e); }

} // namespace Expr

#endif // _INCLUDED_DETAIL_SMOC_EVENT_EXPR_HPP
