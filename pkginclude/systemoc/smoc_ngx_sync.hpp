//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef _INCLUDED_SMOC_NGX_SYNC_HPP
#define _INCLUDED_SMOC_NGX_SYNC_HPP

#include "smoc_pggen.hpp"
#include "smoc_root_port.hpp"

#include <stdexcept>
#include <string>
#include <map>
#include <set>

#include <cosupport/container_insert.hpp>
#include <cosupport/string_hash.hpp>
#include <cosupport/SMXIdManager.hpp>

#include <sysc/kernel/sc_object.h>

#include <acpgacc/smoc_synth_netgraph_access.hpp>

#define objAs CoSupport::dynamic_pointer_cast

namespace SysteMoC { namespace NGXSync {

  using NGX::NgId;

  typedef CoSupport::SMXIdSer IdAttr;

  typedef sc_core::sc_object SCObj;

  std::ostream& operator<<(std::ostream& out, const IdAttr& id);

  struct AlreadyInitialized : public std::runtime_error {
    AlreadyInitialized();
  };
 
  class IdPool {
  public:
    // generate id for the specified named object and index
    NgId regObj(SCObj* obj, size_t index = 0);

    // register object with specified id and index
    NgId regObj(SCObj* obj, const NgId& id, size_t index);
    
    // delete all ids for the specified named object
    void unregObj(const SCObj* obj);
    
    // lookup id for the specified named object and index
    NgId getId(const SCObj* obj, size_t index = 0) const;
    
    // return a new id for an unnamed object
    NgId getId();

    // convenience method for printing the id of a named object
    IdAttr printId(const SCObj* obj, size_t index = 0) const;
    
    // convenience method for printing the id of an unnamed object
    IdAttr printId();

    // convenience method for printing the magic invalid id
    IdAttr printIdInvalid() const;

    // lookup named object for an id (returns 0 if not found)
    SCObj* getObj(const NgId& id) const;

  };

  extern IdPool idPool;

  class NGXConfig {
  public:
    // returns single NGXConfig instance 
    static NGXConfig& getInstance();
    
    // loads the specified NGX file
    void loadNGX(const std::string& ngx);
    
    // determines if a valid NGX file is loaded
    bool hasNGX() const;

    // returns the currently loaded NGX file
    const NGX::NetworkGraphAccess& getNGX() const;

    // destructor
    ~NGXConfig();
  private:
    // invisible constructor for singleton-pattern
    NGXConfig();

    // disabled copy-constructor
    NGXConfig(const NGXConfig&);

    // disabled assigment operator
    NGXConfig& operator=(const NGXConfig&);

    // the currently loaded NGX file
    NGX::NetworkGraphAccess* _ngx;
  };

  class NGXCache {
  public:
    // returns single NGXConfig instance 
    static NGXCache& getInstance();

    // lookup NGX object by SystemC object (returns 0 if none exists)
    NGX::IdedObj::ConstPtr get(SCObj* obj, size_t index = 0);

    // lookup SystemC object by NGX object (returns 0 if none exists)
    SCObj* get(NGX::IdedObj::ConstPtr obj);
    
    // lookup SystemC object by NGX object (returns 0 if none exists)
    SCObj* get(NGX::IdedObj::ConstRef obj);

    // find / cache a port which is compiled into the model and has the
    // same interface as the specified port (returns 0 if none exists)
    // (this makes only sense if all compiled ports are registered)
    smoc_root_port* getCompiledPort(NGX::Port::ConstPtr p);

  private:
    // NGX -> SystemC lookup map
    typedef std::map<NGX::IdedObj::ConstPtr, SCObj*> NGX2SC;
    NGX2SC ngx2sc;

    // SystemC -> NGX lookup map
    typedef std::map<size_t, NGX::IdedObj::ConstPtr> IndexMap;
    typedef std::map<SCObj*, IndexMap> SC2NGX;
    SC2NGX sc2ngx;

    // invisible constructor for singleton-pattern
    NGXCache();

    // disabled copy-constructor
    NGXCache(const NGXCache&);

    // disabled assigment operator
    NGXCache& operator=(const NGXCache&);
  };

} } // SysteMoC::NGXSync

#endif // _INCLUDED_SMOC_NGX_SYNC_HPP
