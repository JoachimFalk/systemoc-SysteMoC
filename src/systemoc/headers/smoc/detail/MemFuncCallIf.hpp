// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_MEMFUNCCALLIF_HPP
#define _INCLUDED_SMOC_DETAIL_MEMFUNCCALLIF_HPP

#include <CoSupport/String/convert.hpp>
#include <CoSupport/String/DoubleQuotedString.hpp>

#include <boost/intrusive_ptr.hpp>

#include <vector>
#include <stddef.h> // for size_t

namespace smoc { namespace Detail {

struct ParamInfo {
  std::string name;
  std::string type;
  std::string value;
};
typedef std::vector<ParamInfo> ParamInfoList;

struct ParamInfoVisitor {
  ParamInfoList pil;

  template<class P>
  void operator()(const P& p) {
    ParamInfo pi;
    //pi.name = FIXME;
    pi.type = typeid(P).name();
    pi.value = CoSupport::String::asStr(p);
    pil.push_back(pi);
  }

  template<class P>
  void operator()(const std::string &name, const P &p) {
    ParamInfo pi;
    pi.name = name;
    pi.type = typeid(P).name();
    pi.value = CoSupport::String::asStr(p);
    pil.push_back(pi);
  }

  void operator()(const std::string &p) {
    std::stringstream sstr;
    sstr << "std::string(" << CoSupport::String::DoubleQuotedString(p) << ")";
    ParamInfo pi;
    //pi.name = FIXME;
    pi.type = typeid(std::string).name();
    pi.value = sstr.str();
    pil.push_back(pi);
  }

  void operator()(const std::string &name, const std::string &p) {
    std::stringstream sstr;
    sstr << "std::string(" << CoSupport::String::DoubleQuotedString(p) << ")";
    ParamInfo pi;
    pi.name  = name;
    pi.type  = typeid(std::string).name();
    pi.value = sstr.str();
    pil.push_back(pi);
  }

  void operator()(const char *p) {
    std::stringstream sstr;
    sstr << CoSupport::String::DoubleQuotedString(p);
    ParamInfo pi;
    //pi.name = FIXME;
    pi.type = typeid(std::string).name();
    pi.value = sstr.str();
    pil.push_back(pi);
  }

  void operator()(const std::string &name, const char *p) {
    std::stringstream sstr;
    sstr << CoSupport::String::DoubleQuotedString(p);
    ParamInfo pi;
    pi.name = name;
    pi.type = typeid(std::string).name();
    pi.value = sstr.str();
    pil.push_back(pi);
  }

  void operator()(char *p)
    { (*this)(static_cast<const char *>(p)); }

  void operator()(const std::string &name, char *p)
    { (*this)(name, static_cast<const char *>(p)); }

};

template <typename R>
class MemFuncCallIf;

template <typename R>
static inline
void intrusive_ptr_add_ref(MemFuncCallIf<R> *r);
template <typename R>
static inline
void intrusive_ptr_release(MemFuncCallIf<R> *r);

template <typename R>
class MemFuncCallIf {
  typedef MemFuncCallIf<R> this_type;
  
  friend void intrusive_ptr_add_ref<R>(this_type *);
  friend void intrusive_ptr_release<R>(this_type *);
private:
  size_t refcount;
public:
  MemFuncCallIf()
    : refcount(0) {}
  
  virtual
  R call() const = 0;
  virtual
  const char *getFuncName() const = 0;
  virtual
  const char *getCxxType() const = 0;

  virtual
  ParamInfoList getParams() const = 0;
  
  virtual
  ~MemFuncCallIf() {}
};

template <typename R>
static inline
void intrusive_ptr_add_ref(MemFuncCallIf<R> *r)
  { ++r->refcount; }
template <typename R>
static inline
void intrusive_ptr_release(MemFuncCallIf<R> *r)
  { if ( !--r->refcount ) delete r; }

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_MEMFUNCCALLIF_HPP */
