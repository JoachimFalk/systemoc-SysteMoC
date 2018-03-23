// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_FUNC_CALL_HPP
#define _INCLUDED_SMOC_FUNC_CALL_HPP

#include <list>
#include <vector>

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/Lambda/functor.hpp>

#include <boost/variant.hpp>
#include <boost/blank.hpp>

#include <systemoc/smoc_config.h>

#include <smoc/smoc_expr.hpp>

#include <boost/intrusive_ptr.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>
#endif //SYSTEMOC_ENABLE_VPC

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <Maestro/PolyphoniC/polyphonic_smoc_func_call.h>
#endif //MAESTRO_ENABLE_POLYPHONIC

#include <smoc/detail/SimCTXBase.hpp>

namespace smoc { namespace Detail {

  class RuntimeState;

} } // namespace smoc::Detail


/**
 * TODO: replace smoc_member_func_interface, smoc_member_func,
 *       smoc_func_call with boost::function
 */

/**
 * smoc_member_func_interface
 */

template <typename R>
class smoc_member_func_interface;

template <typename R>
static inline
void intrusive_ptr_add_ref( smoc_member_func_interface<R> *r );
template <typename R>
static inline
void intrusive_ptr_release( smoc_member_func_interface<R> *r );

template <typename R>
class smoc_member_func_interface {
public:
  typedef smoc_member_func_interface<R> this_type;
  
  friend void intrusive_ptr_add_ref<R>(this_type *);
  friend void intrusive_ptr_release<R>(this_type *);
private:
  size_t      refcount;
public:
  smoc_member_func_interface()
    : refcount(0) {}
  
  virtual
  R call() const = 0;
  virtual
  const char *getFuncName() const = 0;
  virtual
  const char *getCxxType() const = 0;

  virtual
  smoc::Detail::ParamInfoList getParams() const = 0;
  
  virtual
  ~smoc_member_func_interface() {}
};

template <typename R>
static inline
void intrusive_ptr_add_ref( smoc_member_func_interface<R> *r )
  { ++r->refcount; }
template <typename R>
static inline
void intrusive_ptr_release( smoc_member_func_interface<R> *r )
  { if ( !--r->refcount ) delete r; }

/**
 * smoc_member_func
 */

template<class F, class PL>
class smoc_member_func
: public smoc_member_func_interface<typename F::return_type> {
  typedef smoc_member_func<F, PL>                             this_type;
  typedef smoc_member_func_interface<typename F::return_type> base_type; 
public:
  typedef this_type type;
protected:
  F  f;
  PL pl;
public:
#ifdef MAESTRO_ENABLE_POLYPHONIC
  bool canRunInParallel;
#endif
  smoc_member_func(const F &_f, const PL &_pl = PL())
    : f(_f), pl(_pl)
#ifdef MAESTRO_ENABLE_POLYPHONIC
    , canRunInParallel(_f.canRunInParallel)
#endif
  {}

  typename F::return_type call() const
    { return f.call(pl); }
  const char *getFuncName() const
    { return f.name; }
  const char *getCxxType() const
    { return typeid(f.func).name(); }

  smoc::Detail::ParamInfoList getParams() const { 
    smoc::Detail::ParamInfoVisitor piv;
    f.paramListVisit(pl, piv);
    return piv.pil;
  }
};

/**
 * smoc_func_call
 */

class smoc_func_call
  : public smoc::Detail::SimCTXBase
#ifdef MAESTRO_ENABLE_POLYPHONIC
  , public MAESTRO::PolyphoniC::polyphonic_smoc_func_call
#endif
{
private:
  typedef void return_type;
  
  boost::intrusive_ptr<
    smoc_member_func_interface<return_type> >   k;

  smoc::Detail::ParamInfoList pil;
public:
  
  template <class F, class PL>
  smoc_func_call( const smoc_member_func<F, PL> &_k )
    : k(new smoc_member_func<F, PL>(_k))
#ifdef MAESTRO_ENABLE_POLYPHONIC
    , polyphonic_smoc_func_call(_k.canRunInParallel)
#endif
  {
    pil = k->getParams();
  }
  
  void operator()() const
    { return k->call(); }
  
  const char *getFuncName() const
    { return k->getFuncName(); }
  const char *getCxxType() const
    { return k->getCxxType(); }
  
  const smoc::Detail::ParamInfoList& getParams() const {
    return pil;
  }
};

class smoc_action
: public std::list<smoc_func_call> {
public:
  typedef smoc_action this_type;
public:
  smoc_action()
    {}

  explicit
  smoc_action(smoc_func_call const &a)
    { push_back(a); }

  template <class V>
  typename V::result_type apply_visitor(V &v)
    { return v(*this); }
  template <class V>
  typename V::result_type apply_visitor(V const &v)
    { return v(*this); }
  template <class V>
  typename V::result_type apply_visitor(V &v) const
    { return v(*this); }
  template <class V>
  typename V::result_type apply_visitor(V const &v) const
    { return v(*this); }
};

smoc_action merge(const smoc_action& a, const smoc_action& b);

/**
 * ActionVisitor
 */

class ActionVisitor : public smoc::Detail::SimCTXBase {
public:
  typedef smoc::Detail::RuntimeState *result_type;

public:
  ActionVisitor(result_type dest);

  result_type operator()(const smoc_action &f) const;

private:
  result_type dest;
};

namespace smoc { namespace Detail {

  template<class F, class PL>
  struct ActionBuilder {

    typedef smoc_action result_type;

    static
    result_type build(const F &f, const PL &pl)
      { return result_type(smoc_func_call(smoc_member_func<F,PL>(f,pl))); }
  };

#if defined(SYSTEMOC_ENABLE_VPC) || defined(SYSTEMOC_ENABLE_TRANSITION_TRACE)
typedef std::vector<std::string> FunctionNames;

class ActionNameVisitor {
public:
  typedef void result_type;

public:
  ActionNameVisitor(FunctionNames & names);

  result_type operator()(const smoc_action& f) const;

private:
  FunctionNames &functionNames;
};

class GuardNameVisitor: public ExprVisitor<FunctionNames> {
public:
  typedef ExprVisitor<FunctionNames>            base_type;
  typedef GuardNameVisitor                      this_type;

public:
  GuardNameVisitor(FunctionNames & names) :
      functionNames(names)
    , complexity(0) {}

  int getComplexity(){
    return complexity;
  }
  result_type visitVar(const std::string &name, const std::string &type){
    return nullptr;
  }
  result_type visitLiteral(const std::string &type,
      const std::string &value){
    if (type == "m") {
      val.push_back(value);
    }
    complexity++;
    return nullptr;
  }
  result_type visitMemGuard(
      const std::string &name, const std::string& cxxType,
      const std::string &reType, const ParamInfoList &params){
    functionNames.push_back(name);
    complexity++;
    return nullptr;
  }
  result_type visitEvent(const std::string &name){
    return nullptr;
  }
  result_type visitPortTokens(PortBase &p){
    return nullptr;
  }
  result_type visitToken(PortBase &p, size_t n){
    return nullptr;
  }
  result_type visitComm(PortBase &p,
      boost::function<result_type (base_type &)> e){
    return nullptr;
  }
  result_type visitUnOp(OpUnT op,
      boost::function<result_type (base_type &)> e){
    e(*this);
    return nullptr;
  }
  result_type visitBinOp(OpBinT op,
      boost::function<result_type (base_type &)> a,
      boost::function<result_type (base_type &)> b){
    a(*this);
    b(*this);

    return nullptr;
  }
private:
  FunctionNames        &functionNames;
  int                   complexity;
  std::vector<std::string>   val;
};
#endif // defined(SYSTEMOC_ENABLE_VPC) || defined(SYSTEMOC_ENABLE_TRANSITION_TRACE)

} } // namespace smoc::Detail

#ifdef SYSTEMOC_ENABLE_MAESTRO
//////////////TODO: REVIEW THIS SECTION CODE (Visitor's)

using namespace std;

using namespace smoc::Detail;

namespace MetaMap {
  class Transition;
}


namespace smoc { namespace dMM {

class TransitionOnThreadVisitor : public smoc::Detail::SimCTXBase {
public:
  typedef smoc::Detail::RuntimeState *result_type;

public:
  TransitionOnThreadVisitor(result_type dest, MetaMap::Transition* transition);

  result_type operator()(const smoc_action& f) const;

private:
  result_type dest;

  MetaMap::Transition* transition;

  void executeTransition(const smoc_action& f) const;

};

class MMActionNameVisitor {
public:
  typedef void result_type;

public:
  MMActionNameVisitor(list<string> & names):
  functionNames(names) {}

  result_type operator()(const smoc_action& f) const {
    for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
      functionNames.push_back(i->getFuncName());
    }
  }

private:
  list<string> &functionNames;
};

class MMGuardNameVisitor: public ExprVisitor<list<string> > {
public:
  typedef ExprVisitor<list<string> >            base_type;
  typedef MMGuardNameVisitor                    this_type;

public:
  MMGuardNameVisitor(list<string> & names) :
    functionNames(names){}

  result_type visitVar(const std::string &name, const std::string &type){
    return nullptr;
  }
  result_type visitLiteral(const std::string &type,
      const std::string &value){
    return nullptr;
  }
  result_type visitMemGuard(
      const std::string &name, const std::string& cxxType,
      const std::string &reType, const ParamInfoList &params){
    functionNames.push_back(name);
    return nullptr;
  }
  result_type visitEvent(const std::string &name){
    return nullptr;
  }
  result_type visitPortTokens(PortBase &p){
    return nullptr;
  }
  result_type visitToken(PortBase &p, size_t n){
    return nullptr;
  }
  result_type visitComm(PortBase &p,
      boost::function<result_type (base_type &)> e){
    return nullptr;
  }
  result_type visitUnOp(OpUnT op,
      boost::function<result_type (base_type &)> e){
    e(*this);
    return nullptr;
  }
  result_type visitBinOp(OpBinT op,
      boost::function<result_type (base_type &)> a,
      boost::function<result_type (base_type &)> b){
    a(*this);
    b(*this);

    return nullptr;
  }
private:
  list<string> &functionNames;
};
} } // namespace smoc::Detail
#endif // SYSTEMOC_ENABLE_MAESTRO

#endif // _INCLUDED_SMOC_FUNC_CALL_HPP
