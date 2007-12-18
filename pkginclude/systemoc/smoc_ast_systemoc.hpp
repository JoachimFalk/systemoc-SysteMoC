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

#ifndef _INCLUDED_SMOC_AST_SYSTEMOC_HPP
#define _INCLUDED_SMOC_AST_SYSTEMOC_HPP

#include <string>
#include <typeinfo>
#include <sstream>

#include <boost/intrusive_ptr.hpp>
#include <cosupport/refcount_object.hpp>
#include <cosupport/functor.hpp>

namespace smoc_modes {
  class PGWriter;
}

class smoc_port_ast_iface;

template <typename T,
          //template <typename, typename> class R,
          template <typename> class R>
class smoc_port_in_base;

template <typename T, 
          //template <typename, typename> class R, 
          template <typename> class R, 
          template <typename> class STORAGE_TYPE> 
class smoc_port_out_base;

namespace SysteMoC { namespace ActivationPattern {

namespace Detail {

  struct DISABLED { operator bool() const { return false; } };
  struct BLOCKED  {};
  struct ENABLED  { operator bool() const { return true; } };

  template <typename T> struct TypeFilter { typedef T type; };
  template <> struct TypeFilter<DISABLED> { typedef bool type; };
  template <> struct TypeFilter<ENABLED>  { typedef bool type; };

} // namespace SysteMoC::ActivationPattern::Detail

class ValueTypeContainer;

class ValueContainer {
protected:
  std::string value;
public:
  template <typename T >
  explicit
  ValueContainer(const T &v) {
    std::ostringstream o;
    o << v; value = o.str();
  }

  ValueContainer(const ValueContainer &v)
    : value(v.value) {}

  ValueContainer(const ValueTypeContainer &vt);
  
  operator const std::string &() const
    { return value; }
};

std::ostream &operator << (std::ostream &o, const ValueContainer &value);

class TypeIdentifier {
protected:
  std::string type;

  TypeIdentifier(const std::string &type):
    type(type) {}
public:
  operator const std::string &() const
    { return type; }
};

template <typename T>
class Type: public TypeIdentifier {
protected:
  std::string type;
public:
  Type():
    TypeIdentifier(typeid(typename Detail::TypeFilter<T>::type).name()) {}
};

std::ostream &operator << (std::ostream &o, const TypeIdentifier &type);

class ValueTypeContainer
: public ValueContainer
, public TypeIdentifier {
public:
  template <typename T >
  explicit
  ValueTypeContainer(const T &v)
    : ValueContainer(v),
      TypeIdentifier(Type<T>()) {}
};

class PortIdentifier {
protected:
  const smoc_port_ast_iface &port;
public:
  PortIdentifier(const smoc_port_ast_iface &port)
    : port(port) {}

  const smoc_port_ast_iface *getPortPtr() const
    { return &port; }
};

class TypePortIdentifier
: public TypeIdentifier
, public PortIdentifier {
public:
  //template <typename T, template <typename, typename> class R, class PARAM_TYPE>
  template <typename T, template <typename> class R>
  TypePortIdentifier(const smoc_port_in_base<T,R> &port)
    : TypeIdentifier(Type<T>()),
      PortIdentifier(port) {}
};

class SymbolIdentifier {
protected:
  std::string name;
public:
  SymbolIdentifier(const std::string &name)
    : name(name) {}

  operator const std::string &() const
    { return name; }
};

std::ostream &operator << (std::ostream &o, const SymbolIdentifier &symbol);

class TypeSymbolIdentifier
: public TypeIdentifier
, public SymbolIdentifier {
public:
  template<class R, class F>
  explicit
  TypeSymbolIdentifier(const CoSupport::Functor<R,F> &functor)
    : TypeIdentifier(Type<typename CoSupport::Functor<R,F>::return_type>()),
      SymbolIdentifier(functor.name) {}
  template<class R, class F>
  explicit
  TypeSymbolIdentifier(const CoSupport::ConstFunctor<R,F> &functor)
    : TypeIdentifier(Type<typename CoSupport::Functor<R,F>::return_type>()),
      SymbolIdentifier(functor.name) {}
  template<typename T>
  explicit
  TypeSymbolIdentifier(const T &var, const std::string &name)
    : TypeIdentifier(Type<T>()),
      SymbolIdentifier(name) {}

//reinterpret_cast<const dummy *>(f.obj)
//reinterpret_cast<const fun   *>(&f.func)
};

#include "smoc_ast_common.hpp"

} } // namespace SysteMoC::ActivationPattern

#endif // _INCLUDED_SMOC_AST_SYSTEMOC_HPP
