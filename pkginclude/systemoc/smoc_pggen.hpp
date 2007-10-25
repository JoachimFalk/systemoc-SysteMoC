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

#ifndef _INCLUDED_SMOC_PGGEN_HPP
#define _INCLUDED_SMOC_PGGEN_HPP

#include <iostream>
#include <exception>

#include <cassert>

#include <map>
#include <set>

#include <cosupport/filter_ostream.hpp>
#include <cosupport/container_insert.hpp>

#include <sysc/kernel/sc_object.h>

#include <boost/static_assert.hpp>

class smoc_root_node;

namespace smoc_modes {

  extern bool dumpProblemgraph;

  class NgId {
    template<class> friend class IdPool;
  private:
    size_t id;
  protected:
    NgId(size_t id)
      : id(id) {}
  public:
    operator size_t() const
      { return id; }
    operator std::string() const;

    void dump(std::ostream &) const;
  };

  inline
  std::ostream &operator << (std::ostream &out, const NgId &id)
    { id.dump(out); return out; }

  class eNoChannel : public std::exception {};
  class eNoInterface : public std::exception {};
  class eNoPort : public std::exception {};
 
  // http://isthe.com/chongo/tech/comp/fnv/ 
  class FNV32 : public std::unary_function<const char*, uint32_t> {
    typedef const char* Arg;
    typedef uint32_t    Ret;
  public:
    Ret operator()(Arg n) {
      Ret h = INIT;
      while(*n) {
        h *= PRIME;
        h ^= *n++;
      }
      return h;
    }
  private:
    static const Ret INIT  = 0x811c9dc5u;
    static const Ret PRIME = 0x01000193u;
  };
  
  // http://isthe.com/chongo/tech/comp/fnv/ 
  class FNV16 : public std::unary_function<const char*, uint16_t> {
    typedef const char* Arg;
    typedef uint16_t    Ret;
  public:
    Ret operator()(Arg n)
    { return xorFold(fnv32(n)); }
  private:
    FNV32 fnv32;
    Ret xorFold(FNV32::result_type h)
    { return (h >> 16) ^ (h & 0xFFFF); }
  };

  template<class HashFunc>
  class IdPool {
    typedef typename HashFunc::result_type hash_type;
    BOOST_STATIC_ASSERT(sizeof(NgId) == 2 * sizeof(hash_type));
  public:
    IdPool() :
      unnamedIds(0),
      offset(1 << 8 * sizeof(hash_type))
    {}

    IdPool(const HashFunc& hash) :
      unnamedIds(0),
      offset(1 << 8 * sizeof(hash_type)),
      hash(hash)
    {}

    // generate id for the specified named object
    NgId regObj(sc_core::sc_object* obj, size_t index = 0) {
      ObjToId::iterator i = idByObj(obj);
      if(i == objToId.end())
        i = CoSupport::pac_insert(objToId, obj);
      // determine free id
      hash_type _id = hash(obj->name());
      while(!namedIds.insert(_id).second) {
#ifdef SYSTEMOC_DEBUG
        std::cerr << "hash collision: " << _id << std::endl;
#endif
        _id++;
      }
      NgId id = _id + offset;
      // insert object into maps
      CoSupport::pac_insert(i->second, index, id);
      CoSupport::pac_insert(idToObj, id, obj);
#ifdef SYSTEMOC_DEBUG
      std::cerr << "registered " << obj->name() << ": " << id << std::endl;
#endif
      return id;
    }

    // delete id for the specified named object
    void unregObj(const sc_core::sc_object* obj) {
      ObjToId::iterator i = idByObj(obj);
      if(i == objToId.end()) {
        std::cerr << obj->name() << " is not registered!" << std::endl;
        assert(0);
      }
      // delete references from maps
#ifdef SYSTEMOC_DEBUG
      std::cerr << "unregistered " << obj->name() << ": " << i->second << std::endl;
#endif
      for(IndexMap::iterator idx = i->second.begin();
          idx != i->second.end();
          ++i)
      {
        idToObj.erase(objById(idx->second));
      }
      objToId.erase(i);
    }
    
    // determines if the specified named object has an id
    bool hasId(const sc_core::sc_object* o) const
    { return idByObj(o) != objToId.end(); }

    // lookup id for a named object (FIXME: more than one id
    // for the same object via index parameter)
    NgId getId(const sc_core::sc_object* obj, size_t index = 0) const {
      ObjToId::const_iterator i = idByObj(obj);
      if(i == objToId.end()) {
        std::cerr << obj->name() << " is not registered!" << std::endl;
        assert(0);
      }
      IndexMap::const_iterator idx = i->second.find(index);
      assert(idx != i->second.end());
      return idx->second;
    }

    // determines if the specified id has a named object
    bool hasObj(const NgId& id) const
    { return objById(id) != idToObj.end(); }

    // lookup named object for an id
    sc_core::sc_object* getObj(const NgId& id) const {
      IdToObj::const_iterator i = objById(id);
      if(i == idToObj.end()) {
        std::cerr << id << " has no object!" << std::endl;
        assert(0);
      }
      return i->second;
    }

    // return a new id for an unnamed object
    NgId getId()
    { assert(unnamedIds < offset); return unnamedIds++; }

  private:
    // ids for unnamed objects are reserved in sequence
    hash_type unnamedIds;
    
    // ids for named objects are calculated by the hash function
    std::set<hash_type> namedIds;

    // the resulting id for unnamed objects will be in the
    // interval [0, offset[ and for named objects in [offset, ...]
    NgId offset;

    // object -> id lookup map
    typedef std::map<size_t, NgId> IndexMap;
    typedef std::map<sc_core::sc_object*, IndexMap> ObjToId;
    ObjToId objToId;

    // id -> object lookup map
    typedef std::map<NgId, sc_core::sc_object*> IdToObj;
    IdToObj idToObj;

    // hash function used for generating ids for named objects
    HashFunc hash;

    // lookup id by object pointer (find needs key_type)
    ObjToId::iterator idByObj(const sc_core::sc_object* obj)
    { return objToId.find(const_cast<sc_core::sc_object*>(obj)); }
    
    // lookup id by object pointer (find needs key_type)
    ObjToId::const_iterator idByObj(const sc_core::sc_object* obj) const
    { return objToId.find(const_cast<sc_core::sc_object*>(obj)); }

    // lookup object pointer by id
    IdToObj::iterator objById(const NgId& id)
    { return idToObj.find(id); }
    
    // lookup object pointer by id
    IdToObj::const_iterator objById(const NgId& id) const
    { return idToObj.find(id); }
  };

  extern IdPool<FNV16> idPool;

  class PGWriter {
    friend class Node;
  protected:
    CoSupport::FilterOstream    out;
    CoSupport::IndentStreambuf  indenter;
  public:
    PGWriter(std::ostream &_out)
      : out(_out) { out.insert(indenter); }
    
    void indentUp() 
      { indenter.setDeltaLevel(1); }
    void indentDown()
      { indenter.setDeltaLevel(-1); }

    template <typename T>
    std::ostream &operator << (T t) { return out << t; }

    ~PGWriter() {
      out.flush();
    }
  };

  void dump(std::ostream &out, smoc_root_node &top);
};

#endif // _INCLUDED_SMOC_PGGEN_HPP
