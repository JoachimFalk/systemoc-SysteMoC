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

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/detail/smoc_ngx_sync.hpp>
#include <systemoc/smoc_root_node.hpp>

#include <sgx.hpp>

using namespace CoSupport;
using namespace SysteMoC::Detail;

smoc_sysc_port::smoc_sysc_port(const char* name_)
  : sc_port_base(name_, 1),
    interfacePtr(NULL),
    portAccess(NULL),
    parent(NULL), child(NULL)
{
//idPool.regObj(this);
//idPool.regObj(this, 1);
}

smoc_sysc_port::~smoc_sysc_port() {
//idPool.unregObj(this);
}
  
void smoc_sysc_port::bind(sc_interface &interface_ ) {
  sc_port_base::bind(interface_);
}

void smoc_sysc_port::bind(this_type &parent_) {
  assert(parent == NULL && parent_.child == NULL);
  parent        = &parent_;
  parent->child = this;
  sc_port_base::bind(parent_);
}

void smoc_sysc_port::finalise() {
#ifndef __SCFE__
  assembleXML();
#endif
}

#ifndef __SCFE__
void smoc_sysc_port::assembleXML() {
  using namespace SystemCoDesigner::SGX;

  assert(!port);

  Port _p(name());
  port = &_p;

  // set some attributes
  port->direction().set(isInput() ? Port::IN : Port::OUT);

  smoc_root_node* pn =
    dynamic_cast<smoc_root_node*>(get_parent_object());

  if(pn)
    pn->addPort(_p);
  else
    assert(!"Port has no parent node!");

  if(child) {
    // ports are finalised from bottom to top
    assert(child->port);
    child->port->outerConnectedPort() = port;
  }
}
#endif
