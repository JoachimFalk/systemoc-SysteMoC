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

#ifndef _INCLUDED_SMOC_ROOT_PORT_HPP
#define _INCLUDED_SMOC_ROOT_PORT_HPP

#include <cosupport/commondefs.h>

#include <iostream>
#include <cassert>

#include <list>
#include <utility>

#include <cosupport/stl_output_for_list.hpp>
#include <cosupport/stl_output_for_pair.hpp>
#include <cosupport/oneof.hpp>

#include <systemc.h>

#include <systemoc/smoc_config.h>

#include "smoc_expr.hpp"
#include "smoc_event.hpp"

class smoc_root_node;

// forward declaration
namespace Expr { template <class E> class Value; }

class smoc_port_hixhax
: public sc_port_base {
  typedef smoc_port_hixhax this_type;

  // this is needed for the finalise call
  friend class smoc_root_node;
private:
  // port one hierarchy level up
  smoc_port_hixhax *outer;
  // port one hierarchy level down
  smoc_port_hixhax *inner;

  static const char* const kind_string;
public:
  smoc_port_hixhax(const char *name);

  virtual bool isInput()  const = 0;
  virtual bool isOutput() const = 0;

  const smoc_port_hixhax *outerConnectedPort() const
    { return outer; }
  const smoc_port_hixhax *innerConnectedPort() const
    { return inner; }
  smoc_port_hixhax *outerConnectedPort()
    { return outer; }
  smoc_port_hixhax *innerConnectedPort()
    { return inner; }

  const sc_module *owner() const;
//const smoc_root_node *getActor() const;

  // Bind interface to this port. This must be here because
  // otherwise the bind definition below would hide all other
  // bind methods with different type signatures.
  void bind(sc_interface &interface_);
  // Bind parent port to this port and track hierarchy relations.
  void bind(this_type &outer_);

  virtual void dump(std::ostream &out) const = 0;

  virtual const char* kind() const
    { return kind_string; }

  virtual ~smoc_port_hixhax();
protected:
  /// Finalise port called by smoc_root_node::finalise
  virtual void finalise(smoc_root_node *node) = 0;
private:
  // disable => non-copyable non-assignable
  smoc_port_hixhax(const this_type &);
  this_type& operator = (const this_type &);
};

static inline
std::ostream &operator <<(std::ostream &out, const smoc_port_hixhax &p)
  { p.dump(out); return out; }

typedef std::list<smoc_port_hixhax *> smoc_port_hixhax_list;

/*
class smoc_port_hixhax_in
: public smoc_port_hixhax {
public:
  smoc_port_hixhax_in(const char *name)
    : smoc_port_hixhax(name) {}
};

class smoc_port_hixhax_out
: public smoc_port_hixhax {
public:
  smoc_port_hixhax_out(const char *name)
    : smoc_port_hixhax(name) {}
};
 */

class smoc_root_port {
  typedef smoc_root_port this_type;
  
  template <class E> friend class Expr::Value;
  friend class smoc_root_node;
  friend class hscd_choice_active_node;
protected:
  smoc_root_port();
public:
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void commExec(size_t, const smoc_ref_event_p &) = 0;
#else
  virtual void commExec(size_t)                           = 0;
#endif
public:
  virtual void        setLimit(size_t) = 0;
  virtual size_t      availableCount() const = 0;
  virtual smoc_event &blockEvent(size_t n = MAX_TYPE(size_t)) = 0;
  
  virtual void clearReady()
    { assert( !"SHOULD NEVER BE CALLED !!!" ); }
  virtual void communicate( size_t n )
    { assert( !"SHOULD NEVER BE CALLED !!!" ); }

  virtual bool isInput()  const = 0;
  virtual bool isOutput() const = 0;

/*
  operator sc_object *() {
    sc_object *retval = dynamic_cast<sc_object *>(this);
    assert(retval != NULL);
    return retval;
  }

  operator const sc_object *() const {
    const sc_object *retval = dynamic_cast<const sc_object *>(this);
    assert(retval != NULL);
    return retval;
  }
 */

  void dump(std::ostream &out) const;
  virtual ~smoc_root_port();
};

/*
static inline
std::ostream &operator <<(std::ostream &out, const smoc_root_port &p)
  { p.dump(out); return out; }
*/

typedef std::list<smoc_root_port *> smoc_port_list;

class smoc_root_port_in
: public smoc_root_port,
  public smoc_port_hixhax {
public:
  smoc_root_port_in(const char* name_)
    : smoc_port_hixhax(name_) {}

  bool isInput()  const { return true; }
  bool isOutput() const { return false; }

  // overload pure virtual dump from smoc_port_hixhax
  virtual void dump(std::ostream &out) const
    { return smoc_root_port::dump(out); }
};

class smoc_root_port_out
: public smoc_root_port,
  public smoc_port_hixhax {
public:
  smoc_root_port_out(const char* name_)
    : smoc_port_hixhax(name_) {}

  bool isInput()  const { return false; }
  bool isOutput() const { return true; }

  // overload pure virtual dump from smoc_port_hixhax
  virtual void dump(std::ostream &out) const
    { return smoc_root_port::dump(out); }
};
  
#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
