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

#include <systemoc/smoc_root_port.hpp>
#include <systemoc/smoc_root_node.hpp>
#include <systemoc/smoc_ngx_sync.hpp>

#include <cosupport/oneof.hpp>

using namespace CoSupport;
using namespace SysteMoC::NGXSync;


smoc_port_sysc_iface::smoc_port_sysc_iface(const char *name)
  : sc_port_base(name, 1),
    outer(NULL), inner(NULL) {
  idPool.regObj(this);
  idPool.regObj(this, 1);
}

smoc_port_sysc_iface::~smoc_port_sysc_iface() {
  idPool.unregObj(this);
}

smoc_port_ast_iface::smoc_port_ast_iface() {
}
  
smoc_port_ast_iface::~smoc_port_ast_iface() {
}

void smoc_port_ast_iface::dump(std::ostream &out) const {
  out << "port(" << this
//    <<      ",name=" << static_cast<const sc_object *>(this)->name()
//    <<      ",hierarchy=" << getHierarchy()->name()
      <<      ",available=" << availableCount() << ")";
}

const char* const smoc_port_sysc_iface::kind_string = "smoc_port_sysc_iface";

const sc_module *smoc_port_sysc_iface::owner() const {
  const sc_module *retval =
    dynamic_cast<const sc_module *>(this->get_parent());
  assert(retval != NULL);
  return retval;
}

// Bind interface to this port. This must be here because
// otherwise the bind definition below would hide all other
// bind methods with different type signatures.
void smoc_port_sysc_iface::bind(sc_interface &interface_) {
  sc_port_base::bind(interface_);
}
// Bind parent port to this port and track hierarchy relations.
void smoc_port_sysc_iface::bind(this_type &outer_) {
  assert(outer == NULL && outer_.inner == NULL);
  outer         = &outer_;
  outer_.inner  = this;
  sc_port_base::bind(outer_);
}

/*
smoc_root_node *smoc_port_ast_iface::getActor() const {
  smoc_root_node *retval =
    dynamic_cast<smoc_root_node *>(this->get_parent());
  assert(retval != NULL);
  return retval;
}
 */
