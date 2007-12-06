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

#include <sysc/kernel/sc_object.h>

#include <acpgacc/smoc_synth_netgraph_access.hpp>

#define objAs CoSupport::dynamic_pointer_cast

namespace SysteMoC { namespace NGXSync {

  using NGX::NgId;

  struct IdAttr {
    explicit IdAttr(const NGX::NgId& id);
    NGX::NgId id;
  };

  std::ostream& operator<<(std::ostream& out, const IdAttr& id);

  struct AlreadyInitialized : public std::runtime_error {
    AlreadyInitialized();
  };
 
  // calculates smallest x = 2^k; x > N  (e.g. 29 -> 32, 32 -> 64)
  template<uint64_t N, uint64_t S = 1>
  struct NextPower2
  { static const size_t value = NextPower2<N >> 1, S << 1>::value; };

  template<uint64_t S>
  struct NextPower2<0, S>
  { static const uint64_t value = S; };

  // http://isthe.com/chongo/tech/comp/fnv/ 
  // (BITS <= 64)
  template<size_t BITS>
  class FNV {
  public:
    typedef const char* argument_type;
    typedef uint64_t    result_type;
    
    result_type operator()(argument_type n)
    { return xorFold(fnv(n)); }

  private:
    static const result_type MASK = (1ull << BITS) - 1ull;
    
    FNV<NextPower2<BITS>::value> fnv;

    result_type xorFold(result_type h)
    { return (h >> BITS) ^ (h & MASK); }
  };
  
  // http://isthe.com/chongo/tech/comp/fnv/ 
  template<class T, T INIT, T PRIME>
  class FNVBase {
  public:
    typedef const char* argument_type;
    typedef T           result_type;

    result_type operator()(argument_type n) {
      result_type h = INIT;
      while(*n) {
        h *= PRIME;
        h ^= *n++;
      }
      return h;
    }
  };

  // http://isthe.com/chongo/tech/comp/fnv/ 
  template<>
  class FNV<32> : public FNVBase<
    uint32_t,
    0x811C9DC5ul,
    0x01000193ul>
  {};
  
  // http://isthe.com/chongo/tech/comp/fnv/ 
  template<>
  class FNV<64> : public FNVBase<
    uint64_t,
    0xCBF29CE484222325ull,
    0x00000100000001B3ull>
  {};

  class IdPool {
    typedef uint32_t Id;
    static const size_t BITS = std::numeric_limits<Id>::digits;
  public:
    IdPool();

    // generate id for the specified named object and index
    NgId regObj(sc_core::sc_object* obj, size_t index = 0);

    // register object with specified id and index
    NgId regObj(sc_core::sc_object* obj, const NgId& id, size_t index);
    
    // delete all ids for the specified named object
    void unregObj(const sc_core::sc_object* obj);
    
    // lookup id for the specified named object and index
    NgId getId(const sc_core::sc_object* obj, size_t index = 0) const;
    
    // return a new id for an unnamed object
    NgId getId();

    // convenience method for printing the id of a named object
    IdAttr printId(const sc_core::sc_object* obj, size_t index = 0) const;
    
    // convenience method for printing the id of an unnamed object
    IdAttr printId();
    
    // lookup named object for an id (returns 0 if not found)
    sc_core::sc_object* getObj(const NgId& id) const;

    void dump(std::ostream& out) const;

  private:
    // NgId: 2 bits region; (BITS-2) bits id
    //       00...: generated
    //       01...: named
    //       10...: unnamed
    //       11:..: reserved 
    static const Id GENERATED = 0 << (BITS-2);
    static const Id NAMED     = 1 << (BITS-2);
    static const Id UNNAMED   = 2 << (BITS-2);
    static const Id RESERVED  = 3 << (BITS-2);

    // ids for unnamed objects are reserved in sequence
    Id unnamedIds;

    // ids for named objects are calculated by the hash function
    std::set<Id> namedIds;

    // object -> id lookup map
    typedef std::map<size_t, NgId> IndexMap;
    typedef std::map<sc_core::sc_object*, IndexMap> ObjToId;
    ObjToId objToId;
    
    // id -> object lookup map
    typedef std::map<NgId, sc_core::sc_object*> IdToObj;
    IdToObj idToObj;

    // hash function used for generating ids for named objects
    FNV<BITS-2> hash;

    // lookup id by object pointer (find needs key_type)
    ObjToId::iterator idByObj(const sc_core::sc_object* obj);
    
    // lookup id by object pointer (find needs key_type)
    ObjToId::const_iterator idByObj(const sc_core::sc_object* obj) const;

    // lookup object pointer by id
    IdToObj::iterator objById(const NgId& id);
    
    // lookup object pointer by id
    IdToObj::const_iterator objById(const NgId& id) const;
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
    NGX::IdedObj::ConstPtr get(sc_core::sc_object* obj, size_t index = 0);

    // lookup SystemC object by NGX object (returns 0 if none exists)
    sc_core::sc_object* get(NGX::IdedObj::ConstPtr obj);
    
    // lookup SystemC object by NGX object (returns 0 if none exists)
    sc_core::sc_object* get(NGX::IdedObj::ConstRef obj);

    // find / cache a port which is compiled into the model and has the
    // same interface as the specified port (returns 0 if none exists)
    // (this makes only sense if all compiled ports are registered)
    smoc_root_port* getCompiledPort(NGX::Port::ConstPtr p);

  private:
    // NGX -> SystemC lookup map
    typedef std::map<NGX::IdedObj::ConstPtr, sc_core::sc_object*> NGX2SC;
    NGX2SC ngx2sc;

    // SystemC -> NGX lookup map
    typedef std::map<size_t, NGX::IdedObj::ConstPtr> IndexMap;
    typedef std::map<sc_core::sc_object*, IndexMap> SC2NGX;
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