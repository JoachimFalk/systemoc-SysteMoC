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

#ifndef _INCLUDED_SMOC_FUNC_CALL_HPP
#define _INCLUDED_SMOC_FUNC_CALL_HPP

#include <CoSupport/Lambda/functor.hpp>
#include <boost/variant.hpp>
#include <boost/blank.hpp>
#include <boost/intrusive_ptr.hpp>

#define CALL(func)    call(&func, #func)
#define GUARD(func)   guard(&func, #func)
#define VAR(variable) var(variable, #variable)
#define SR_TICK(func) call(&func, #func)
#define SR_GO(func)   call(&func, #func)

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
};



/**
 * smoc_func_call
 */

class smoc_func_call {
private:
  typedef void return_type;
  
  boost::intrusive_ptr<
    smoc_member_func_interface<return_type> >   k;
public:
  
  template <class F, class PL>
  smoc_func_call( const smoc_member_func<F, PL> &_k )
    : k(new smoc_member_func<F, PL>(_k)) {}
  
  void operator()() const {
    return k->call();
  }
  
  const char* getFuncName() const {
    return k->getFuncName();
  }
};

class FiringStateImpl;

/**
 * smoc_func_diverge
 */

class smoc_func_diverge {
private:
  typedef FiringStateImpl* return_type;

  boost::intrusive_ptr<
    smoc_member_func_interface<return_type> > k;
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



/**
 * smoc_func_branch
 */

class smoc_func_branch: public smoc_func_diverge {
public:
  template <class F, class PL>
  smoc_func_branch( const smoc_member_func<F, PL> &_k )
    : smoc_func_diverge(_k) {}
};



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

class smoc_connector_action_pair;

typedef boost::variant<
  boost::blank,
  smoc_func_call,
  smoc_func_diverge,
  smoc_sr_func_pair,
  smoc_connector_action_pair> smoc_action;
  

/**
 * smoc_connector_action_pair
 */

class smoc_connector_action_pair {
public:
  /*smoc_action a;
  smoc_action b;

  smoc_connector_action_pair(
      const smoc_action& a,
      const smoc_action& b)
    : a(a), b(b)
  {}*/
  
  smoc_func_call a;
  smoc_func_call b;

  smoc_connector_action_pair(
      const smoc_func_call& a,
      const smoc_func_call& b)
    : a(a), b(b)
  {}
};


/**
 * ActionVisitor
 */

class ActionVisitor {
public:
  typedef FiringStateImpl* result_type;

public:
  ActionVisitor(FiringStateImpl* dest, int mode);

  result_type operator()(smoc_func_call& f) const;
  result_type operator()(smoc_func_diverge& f) const;
  result_type operator()(smoc_sr_func_pair& f) const;
  result_type operator()(smoc_connector_action_pair& f) const;
  result_type operator()(boost::blank& f) const;

private:
  FiringStateImpl* dest;
  int mode;
};

#endif // _INCLUDED_SMOC_FUNC_CALL_HPP
