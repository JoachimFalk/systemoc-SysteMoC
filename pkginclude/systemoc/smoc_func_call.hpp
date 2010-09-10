// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <CoSupport/Lambda/functor.hpp>

#include <boost/variant.hpp>
#include <boost/blank.hpp>
#include <boost/intrusive_ptr.hpp>

#include <systemoc/smoc_config.h>

#include "smoc_expr.hpp"

#ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>
#endif //SYSTEMOC_ENABLE_VPC

/**
 * TODO: deprecate smoc_func_diverge and smoc_func_branch
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
class smoc_member_func_interface
#ifdef SYSTEMOC_NEED_IDS
  :  public SysteMoC::Detail::NamedIdedObj
#endif // SYSTEMOC_NEED_IDS
{
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
  
  const char* name() const {
    return this->getFuncName();
  }

  virtual
  SysteMoC::Detail::ParamInfoList getParams() const = 0;
  
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
public:
  typedef smoc_member_func<F, PL> type;
protected:
  F  f;
  PL pl;
public:
  smoc_member_func(const F &_f, const PL &_pl = PL() )
    : f(_f), pl(_pl) {}
  
  typename F::return_type call() const
    { return f.call(pl); }
  const char *getFuncName() const
    { return f.name; }
  SysteMoC::Detail::ParamInfoList getParams() const
  { 
    SysteMoC::Detail::ParamInfoVisitor piv;
    f.paramListVisit(pl, piv);
    return piv.pil;
  }
};

/**
 * smoc_func_call
 */

class smoc_func_call : public SysteMoC::Detail::SimCTXBase{
private:
  typedef void return_type;
  
  boost::intrusive_ptr<
    smoc_member_func_interface<return_type> >   k;

  SysteMoC::Detail::ParamInfoList pil;

//friend bool operator <(const smoc_func_call &lhs, const smoc_func_call &rhs);
public:
  
  template <class F, class PL>
  smoc_func_call( const smoc_member_func<F, PL> &_k )
    : k(new smoc_member_func<F, PL>(_k))
  {
#ifdef SYSTEMOC_NEED_IDS
    this->getSimCTX()->createId( k.get() );
#endif // SYSTEMOC_NEED_IDS

    pil = k->getParams();
  }
  
  void operator()() const {
    return k->call();
  }
  
  const char* getFuncName() const {
    return k->getFuncName();
  }
  
#ifdef SYSTEMOC_NEED_IDS
  size_t getId() const {
    return k->getId();
  }
#endif // SYSTEMOC_NEED_IDS
  
  const SysteMoC::Detail::ParamInfoList& getParams() const {
    return pil;
  }
};

//bool operator <(const smoc_func_call &lhs, const smoc_func_call &rhs)
//  { return lhs.k < rhs.k; }

class RuntimeState;

/**
 * smoc_func_diverge
 */

class smoc_func_diverge {
private:
  typedef RuntimeState* return_type;

  boost::intrusive_ptr<
    smoc_member_func_interface<return_type> > k;

//friend bool operator <(const smoc_func_diverge &lhs, const smoc_func_diverge &rhs);
public:
  template <class F, class PL>
  smoc_func_diverge( const smoc_member_func<F, PL> &_k )
    : k(new smoc_member_func<F, PL>(_k)) {}
  
  template <class T>
  smoc_func_diverge( T *_obj, return_type (T::*_f)() )
    : k(new typename CoSupport::Lambda::ParamAccumulator<
        smoc_member_func,
        CoSupport::Lambda::Functor<return_type, return_type (T::*)()> >::accumulated_type
      (CoSupport::Lambda::Functor<return_type, return_type (T::*)()>(_obj, _f, "")))
    {}
  
  return_type operator()() const {
    return k->call();
  }
};

//bool operator <(const smoc_func_diverge &lhs, const smoc_func_diverge &rhs)
//  { return lhs.k < rhs.k; }

/**
 * smoc_sr_func_pair
 */

class smoc_sr_func_pair {
public:
  friend class smoc_transition;
  friend class smoc_transition_part;
  //private:
  smoc_func_call             go;
  smoc_func_call             tick;

#ifdef SYSTEMOC_ENABLE_VPC
  SystemC_VPC::FastLink* tickLink;
#endif // SYSTEMOC_ENABLE_VPC

public:
  smoc_sr_func_pair(
      const smoc_func_call &go,
      const smoc_func_call &tick)
    : go(go), tick(tick) {}
};

class smoc_func_call_list
: public std::list<smoc_func_call> {
public:
  typedef smoc_func_call_list this_type;
public:
  smoc_func_call_list() {}

  smoc_func_call_list(const smoc_func_call& a)
    { push_back(a); }
};

typedef boost::variant<
  smoc_func_call_list,
  smoc_func_diverge,
  smoc_sr_func_pair> smoc_action;

smoc_action merge(const smoc_action& a, const smoc_action& b);

/**
 * ActionVisitor
 */

class ActionVisitor : public SysteMoC::Detail::SimCTXBase {
public:
  typedef RuntimeState* result_type;

public:
  ActionVisitor(RuntimeState* dest, int mode);

  result_type operator()(const smoc_func_call_list& f) const;
  result_type operator()(const smoc_func_diverge& f) const;
  result_type operator()(const smoc_sr_func_pair& f) const;

private:
  RuntimeState* dest;
  int mode;
};

#ifdef SYSTEMOC_ENABLE_VPC
namespace SysteMoC { namespace Detail {

  using SystemC_VPC::FunctionNames;

  class ActionNameVisitor {
public:
  typedef void result_type;

public:
  ActionNameVisitor(FunctionNames & names);

  result_type operator()(const smoc_func_call_list& f) const;
  result_type operator()(const smoc_sr_func_pair& f) const;
  result_type operator()(const smoc_func_diverge& f) const;

private:
  FunctionNames &functionNames;
};

class GuardNameVisitor: public ExprVisitor<FunctionNames> {
public:
  typedef ExprVisitor<FunctionNames>            base_type;
  typedef GuardNameVisitor                      this_type;

public:
  GuardNameVisitor(FunctionNames & names) :
    functionNames(names){}

  result_type visitVar(const std::string &name, const std::string &type){
    return NULL;
  }
  result_type visitLiteral(const std::string &type,
      const std::string &value){
    return NULL;
  }
  result_type visitMemGuard(const std::string &name,
      const std::string &reType, const ParamInfoList &params){
    functionNames.push_back(name);
    return NULL;
  }
  result_type visitEvent(const std::string &name){
    return NULL;
  }
  result_type visitPortTokens(smoc_sysc_port &p){
    return NULL;
  }
  result_type visitToken(smoc_sysc_port &p, size_t n){
    return NULL;
  }
  result_type visitComm(smoc_sysc_port &p,
      boost::function<result_type (base_type &)> e){
    return NULL;
  }
  result_type visitUnOp(OpUnT op,
      boost::function<result_type (base_type &)> e){
    e(*this);
    return NULL;
  }
  result_type visitBinOp(OpBinT op,
      boost::function<result_type (base_type &)> a,
      boost::function<result_type (base_type &)> b){
    a(*this);
    b(*this);

    return NULL;
  }
private:
  FunctionNames &functionNames;
};
} } // namespace SysteMoC::Detail
#endif // SYSTEMOC_ENABLE_VPC

#endif // _INCLUDED_SMOC_FUNC_CALL_HPP
