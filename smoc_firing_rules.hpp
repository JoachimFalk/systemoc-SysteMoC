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

#ifndef _INCLUDED_SMOC_OP_PORT_LIST_HPP
#define _INCLUDED_SMOC_OP_PORT_LIST_HPP

#include <iostream>

#include <vector>

#include <list>
#include <cosupport/stl_output_for_list.hpp>

#include <set>

#include <cosupport/oneof.hpp>

#include <systemc.h>
#include <smoc_guard.hpp>
#include <smoc_root_port.hpp>

#include <boost/shared_ptr.hpp>

#include <cosupport/commondefs.h>
#include <cosupport/functor.hpp>

class smoc_activation_pattern;
class smoc_transition;
class smoc_port2op_if;
class smoc_root_node;

class smoc_firing_state;
class smoc_firing_rules;
class smoc_transition_list;

#define CALL(func)    call(&func,#func)
#define GUARD(func)   guard(&func,#func)
#define VAR(variable) var(variable,#variable)

template <typename R>
class smoc_member_func_interface;

template <typename R>
static inline
void intrusive_ptr_add_ref( smoc_member_func_interface<R> *r );
template <typename R>
static inline
void intrusive_ptr_release( smoc_member_func_interface<R> *r );

typedef std::vector<smoc_firing_state *> smoc_firing_states;

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

class smoc_func_call {
private:
  typedef void					return_type;
  
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

class smoc_func_diverge {
private:
  typedef const smoc_firing_state	       &return_type;
  
  boost::intrusive_ptr<
    smoc_member_func_interface<return_type> >   k;
public:
  
  template <class F, class PL>
  smoc_func_diverge( const smoc_member_func<F, PL> &_k )
    : k(new smoc_member_func<F, PL>(_k)) {}
  
  template <class T>
  smoc_func_diverge( T *_obj, return_type (T::*_f)() )
    : k(new typename CoSupport::ParamAccumulator<
	      smoc_member_func,
	      CoSupport::Functor<return_type, return_type (T::*)()> >::accumulated_type
	    (CoSupport::Functor<return_type, return_type (T::*)()>(_obj, _f, "")))
    {}
  
  return_type operator()() const {
    return k->call();
  }
};

class smoc_func_branch: public smoc_func_diverge {
public:
  template <class F, class PL>
  smoc_func_branch( const smoc_member_func<F, PL> &_k )
    : smoc_func_diverge(_k) {}
};

class smoc_firing_state_ref;

struct smoc_firing_types {
  struct                                  transition_ty;
  struct                                  resolved_state_ty;
  typedef std::list<resolved_state_ty *>  statelist_ty;
  typedef std::list<transition_ty>        transitionlist_ty;
  typedef std::pair<bool,transition_ty *> maybe_transition_ty;
//  typedef std::set<smoc_root_port *>  ports_ty;
  typedef CoSupport::oneof<smoc_func_call, smoc_func_branch, smoc_func_diverge>
                                          func_ty;
  
  class transition_ty
  : public smoc_activation_pattern {
  public:
//  smoc_activation_pattern ap;
    func_ty                 f;
    statelist_ty            sl;
    
    smoc_root_node         *actor;
  public:
    transition_ty( smoc_firing_state_ref *r, const smoc_transition &t );

#ifdef SYSTEMOC_DEBUG
    status_t getStatus() const {
      std::cerr << "transition_ty::getStatus: " << *this << std::endl;
      return smoc_activation_pattern::getStatus();
    }
#endif

    bool isEnabled() const
      { return getStatus() == ENABLED; }

    smoc_root_node &getActor() {
      assert(actor != NULL);
      return *actor;
    }

    void execute(resolved_state_ty **rs, smoc_root_node *actor);

    void finalise(smoc_root_node *a);

    void dump(std::ostream &out) const;

//#ifdef SYSTEMOC_DEBUG
//    ~transition_ty() {
//      std::cerr << "~transition_ty() this == " << this << std::endl;
//    }
//#endif
  };
  
  class resolved_state_ty {
  public:
    // outgoing transitions from this state
    transitionlist_ty       tl;
  protected:
    // all ports in the outgoing transitions
    // ports_ty             ps;
  public:
    resolved_state_ty() {}
    
    void clearTransitions();
    void addTransition(smoc_firing_state_ref *r, const smoc_transition_list &tl );
    
//  bool tryExecute(resolved_state_ty **rs, smoc_root_node *actor);
//  void findBlocked(smoc_event_or_list &l);

    void finalise(smoc_root_node *a);

#ifdef SYSTEMOC_DEBUG
    ~resolved_state_ty() {
      std::cerr << "~resolved_state_ty() this == " << this << std::endl;
    }
#endif
  };
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_firing_types::transition_ty &t )
  { t.dump(out); return out; }

class smoc_firing_state_ref
  :public smoc_firing_types {
public:
  typedef smoc_firing_state_ref this_type;
  
  friend class smoc_firing_rules;
  friend class smoc_firing_types::transition_ty;
  friend class smoc_scheduler_top;
//private:
protected:
  mutable resolved_state_ty *rs;
  mutable smoc_firing_rules *fr;
protected:
  // create new empty state with new firing_rules
  smoc_firing_state_ref()
    : rs(NULL), fr(NULL) {}
public:
  // create alias firing state
  smoc_firing_state_ref(const smoc_firing_state_ref &rhs);
  
  resolved_state_ty &getResolvedState() const;
  
  resolved_state_ty *finalise( smoc_root_node *actor ) const;
//bool tryExecute();
//void findBlocked(smoc_event_or_list &l);
  void dump( std::ostream &o ) const;
  const smoc_firing_rules &getFiringRules() const
    { return *fr; }
  
  ~smoc_firing_state_ref();
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_firing_state_ref &s )
  { s.dump(out); return out; }

class smoc_firing_state
  :public smoc_firing_state_ref,
   public sc_object {
public:
  friend class smoc_opbase_node;
  friend class transition_ty;
  
  typedef smoc_firing_state this_type;
public:
  smoc_firing_state(const smoc_transition_list &tl);
  smoc_firing_state(const smoc_transition &t);
  smoc_firing_state();
  smoc_firing_state(const this_type &x);
  
//  bool isResolvedState() const { return rs != NULL; }
  
  this_type &operator = (const this_type &x);
  this_type &operator = (const smoc_transition_list &tl);
  this_type &operator = (const smoc_transition &t);
  
  void addTransition( const smoc_transition_list &tl );
  void clearTransition();

  ~smoc_firing_state();
private:
  // smoc_port_list &getPorts() const;
  // disable
};

class smoc_firing_state_list
  :public std::vector<smoc_firing_state_ref> {
public:
  typedef smoc_firing_state_list  this_type;
public:
  smoc_firing_state_list() {}
  smoc_firing_state_list( const value_type &v ) { push_back(v); }
  
  this_type &operator |=( const value_type &v )
    { push_back(v); return *this; }
};

#ifndef _COMPILEHEADER_SMOC_FIRING_STATE_LIST_OPERATOR_OR_1
GNU89_EXTERN_INLINE
#endif
smoc_firing_state_list operator |
  (smoc_firing_state_list sl,
   const smoc_firing_state_list::value_type &v ) {
  return sl |= v;
}
#ifndef _COMPILEHEADER_SMOC_FIRING_STATE_LIST_OPERATOR_OR_2
GNU89_EXTERN_INLINE
#endif
smoc_firing_state_list operator |
  (const smoc_firing_state_list::value_type &v1,
   const smoc_firing_state_list::value_type &v2 ) {
  return smoc_firing_state_list(v1) |= v2;
}



class smoc_firing_rules
  :public smoc_firing_types {
public:
  typedef smoc_firing_rules this_type;
  
  friend class smoc_firing_types::transition_ty;
  friend class smoc_firing_state_ref;
  friend class smoc_firing_state;
private:
  typedef std::set<const smoc_firing_state_ref *> references_ty;
  
  statelist_ty            states;
  references_ty           references;
  smoc_root_node         *actor;
  
  void _addRef(const smoc_firing_state_ref *s,
               const smoc_firing_rules     *p);
protected:
  smoc_firing_rules(const smoc_firing_state_ref *s)
    : actor(NULL) { addState(s); }

  smoc_root_node* getActor() { return actor; }
 
  void addRef(const smoc_firing_state_ref *s)
    { _addRef(s,NULL); }
  void delRef(const smoc_firing_state_ref *s);
  void addState(const smoc_firing_state_ref *s);

  void unify(smoc_firing_rules *fr);

  void finalise(smoc_root_node *actor);
public:
  const statelist_ty &getFSMStates() const
    { return states; }
  
  ~smoc_firing_rules();
};

class smoc_interface_action {
public:
  friend class smoc_opbase_node;
  friend class smoc_scheduler_ndf;
  friend class smoc_root_node;
  friend class smoc_firing_types::transition_ty;
  friend class smoc_transition;
//  friend class smoc_firing_state;
  
  typedef smoc_interface_action this_type;
private:
  smoc_firing_state_list     sl;
  smoc_firing_types::func_ty f;
protected:
  explicit smoc_interface_action(const smoc_firing_state_ref &s)
    : sl(s), f() {}
  explicit smoc_interface_action(const smoc_func_diverge &f)
    : sl(), f(f) {}
  smoc_interface_action(const smoc_firing_state_ref &s,
                        const smoc_func_call &f)
    : sl(s), f(f) {}
  smoc_interface_action(const smoc_firing_state_ref &s,
                        const smoc_func_branch &f)
    : sl(s), f(f) {}
  smoc_interface_action(const smoc_firing_state_list &sl,
                        const smoc_func_branch &f)
    : sl(sl), f(f) {}
};

class smoc_transition_part {
public:
  friend class smoc_transition;
  
  typedef smoc_transition_part this_type;
private:
  smoc_activation_pattern    ap;
  smoc_func_call             f;
public:
  smoc_transition_part(
      const smoc_activation_pattern &ap,
      const smoc_func_call          &f)
    : ap(ap),
      f(f) {}
};

class smoc_transition {
public:
  friend class smoc_firing_types::transition_ty;
  
  typedef smoc_transition this_type;
private:
  smoc_activation_pattern ap;
  smoc_interface_action   ia;
public:
  smoc_transition(
      const smoc_activation_pattern &ap,
      const smoc_func_diverge       &f)
    : ap(ap), ia(f) {}
  smoc_transition(
      const smoc_func_call          &f,
      const smoc_firing_state_ref   &s)
    : ap(Expr::literal(true)), ia(s,f) {}
  smoc_transition(
      const smoc_activation_pattern &ap,
      const smoc_firing_state_ref   &s)
    : ap(ap), ia(s) {}
  smoc_transition(
      const smoc_transition_part    &tp,
      const smoc_firing_state_ref   &s)
    : ap(tp.ap),
      ia(s,tp.f) {}
  
  const smoc_activation_pattern &getActivationPattern() const { return ap; }
  const smoc_interface_action   &getInterfaceAction() const { return ia; }
};

class smoc_transition_list
  :public std::vector<smoc_transition> {
public:
  typedef smoc_transition_list this_type;
protected:
public:
  smoc_transition_list() {}
  
  smoc_transition_list( smoc_transition t ) {
    push_back(t);
  }
  
  this_type &operator |= ( smoc_transition t ) {
    push_back(t); return *this;
  }
};

#ifndef _COMPILEHEADER_SMOC_TRANSITION_LIST__OPERATOR_OR_1
GNU89_EXTERN_INLINE
#endif
smoc_transition_list operator | (const smoc_transition_list &tl,
				 const smoc_transition &t )
  { return smoc_transition_list(tl) |= t; }
#ifndef _COMPILEHEADER_SMOC_TRANSITION_LIST__OPERATOR_OR_2
GNU89_EXTERN_INLINE
#endif
smoc_transition_list operator | (const smoc_transition &tx,
				 const smoc_transition &t )
  { return smoc_transition_list(tx) |= t; }

#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_1
GNU89_EXTERN_INLINE
#endif
smoc_transition_part operator >> (const smoc_activation_pattern &ap,
                                  const smoc_func_call          &f) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition_part(ap,f);
}

#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_2
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_func_call        &f,
			     const smoc_firing_state_ref &s) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(f,s);
}

#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_3
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_transition_part  &tp,
			     const smoc_firing_state_ref &s) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(tp,s);
}

#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_4
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_activation_pattern &ap,
                             const smoc_firing_state_ref   &s) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(ap,s);
}

#endif // _INCLUDED_SMOC_OP_PORT_LIST_HPP
