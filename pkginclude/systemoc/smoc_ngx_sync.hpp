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

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <istream>

#include <systemc.h>
#include <sysc/kernel/sc_object.h>

#include <CoSupport/DataTypes/container_insert.hpp>
#include <CoSupport/Math/string_hash.hpp>
#include <CoSupport/SMXIdManager.hpp>

#include <sgx.hpp>

#include <systemoc/smoc_config.h>

#include "smoc_pggen.hpp"
#include "detail/smoc_sysc_port.hpp"

#define objAs CoSupport::DataTypes::dynamic_pointer_cast

extern SystemCoDesigner::SGX::NetworkGraphAccess ngx;

namespace SysteMoC {

namespace SGX = SystemCoDesigner::SGX;
  
namespace NGXSync {

  using SystemCoDesigner::SGX::NgId;

  typedef CoSupport::SMXIdSer IdAttr;

  typedef sc_core::sc_object SCObj;

//  std::ostream& operator<<(std::ostream& out, const IdAttr& id);

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
    
    // loads the specified SGX file
    void loadNGX(const std::string& ngx);
    
    // loads the specified SGX file
    void loadNGX(std::istream& ngx);
    
    // determines if a valid SGX file is loaded
    bool hasNGX() const;

    // returns the currently loaded SGX file
    const SGX::NetworkGraphAccess& getNGX() const;

    // destructor
    ~NGXConfig();
  private:
    // invisible constructor for singleton-pattern
    NGXConfig();

    // disabled copy-constructor
    NGXConfig(const NGXConfig&);

    // disabled assigment operator
    NGXConfig& operator=(const NGXConfig&);

    // the currently loaded SGX file
    SGX::NetworkGraphAccess* _ngx;
  };

  class NGXCache {
  public:
    // returns single NGXConfig instance 
    static NGXCache& getInstance();

    // lookup SGX object by SystemC object (returns 0 if none exists)
    SGX::IdedObj::ConstPtr get(SCObj* obj, size_t index = 0);

    // lookup SystemC object by SGX object (returns 0 if none exists)
    SCObj* get(SGX::IdedObj::ConstPtr obj);
    
    // lookup SystemC object by SGX object (returns 0 if none exists)
    SCObj* get(SGX::IdedObj::ConstRef obj);

    // find / cache a port which is compiled into the model and has the
    // same interface as the specified port (returns 0 if none exists)
    // (this makes only sense if all compiled ports are registered)
    smoc_sysc_port *getCompiledPort(SGX::Port::ConstPtr p);

  private:
    // SGX -> SystemC lookup map
    typedef std::map<SGX::IdedObj::ConstPtr, SCObj*> NGX2SC;
    NGX2SC ngx2sc;

    // SystemC -> SGX lookup map
    typedef std::map<size_t, SGX::IdedObj::ConstPtr> IndexMap;
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
