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
#include <systemoc/detail/smoc_root_node.hpp>

#include <sgx.hpp>

using namespace CoSupport;
using namespace SysteMoC::Detail;

smoc_sysc_port::smoc_sysc_port(const char* name_)
  : sc_port_base(name_, 1),
    interfacePtr(NULL),
    portAccess(NULL),
    parent(NULL), child(NULL) {
}

smoc_sysc_port::~smoc_sysc_port() {
}
  
void smoc_sysc_port::bind(this_type &parent_) {
  assert(parent == NULL && parent_.child == NULL);
  parent        = &parent_;
  parent->child = this;
  sc_port_base::bind(parent_);
}

void smoc_sysc_port::finalise() {
#ifdef SYSTEMOC_NEED_IDS  
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS  
}

#ifdef SYSTEMOC_ENABLE_SGX
SystemCoDesigner::SGX::Port::Ptr smoc_sysc_port::getNGXObj() const
  { return port; }
#endif
