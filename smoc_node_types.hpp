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

#ifndef _INCLUDED_SMOC_NODE_TYPES_HPP
#define _INCLUDED_SMOC_NODE_TYPES_HPP

#include <smoc_root_node.hpp>

class smoc_actor
  : public smoc_root_node,
    public sc_module {
  protected:
//  explicit smoc_actor(sc_module_name name, const smoc_firing_state &s)
//    : smoc_root_node(s), sc_module(name) {}
//  smoc_actor(const smoc_firing_state &s)
//    : smoc_root_node(s),
//      sc_module(sc_gen_unique_name("smoc_actor")) {}
    explicit smoc_actor(sc_module_name name, smoc_firing_state &s)
      : smoc_root_node(s),
        sc_module(name) {}
    smoc_actor(smoc_firing_state &s)
      : smoc_root_node(s),
        sc_module(sc_gen_unique_name("smoc_actor")) {}

#ifdef SYSTEMOC_DEBUG
    ~smoc_actor() {
      std::cerr << "~smoc_actor() name = \""
                << myModule()->name() << "\"" << std::endl;
    }
#endif
  public:
#ifndef __SCFE__
    sc_module *myModule() { return this; }
    
    void assemble( smoc_modes::PGWriter &pgw ) const {
      return smoc_root_node::assemble(pgw); }
#endif
};

#endif // _INCLUDED_SMOC_NODE_TYPES_HPP
