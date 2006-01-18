// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _INCLUDED_SMOC_OP_PORT_LIST_HPP
#define _INCLUDED_SMOC_OP_PORT_LIST_HPP

#include <iostream>

#include <vector>

#include <list>
#include <cosupport/stl_output_for_list.hpp>

#include <set>

#include <cosupport/oneof.hpp>

#include <smoc_guard.hpp>
#include <smoc_root_port.hpp>

#include <boost/shared_ptr.hpp>

#include <cosupport/commondefs.h>

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
  R operator()() const = 0;
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

template <typename P, class PN = void>
struct smoc_param {
  typedef P  type;
  typedef PN tail;
  
  P  p;
  PN pn;
  
  smoc_param( const P &_p, const PN &_pn )
    : p(_p), pn(_pn) {}
};

template<>
struct smoc_param<void,void> {};

template <typename M, class MN = void>
struct smoc_missing {
  typedef M  type;
  typedef MN tail;
};

template<>
struct smoc_missing<void,void> {};

template <class K, class ML, class PL = smoc_param<void> >
struct smoc_accumulate_param {
  typedef smoc_accumulate_param<K, typename ML::tail, smoc_param<typename ML::type, PL> > fnt_ty;
  
  K  k;
  PL pl;
  
  smoc_accumulate_param(const K &_k, const PL &_pl = PL())
    : k(_k), pl(_pl) {}
  
  fnt_ty operator()(
      const typename ML::type &p) {
    return fnt_ty(k, smoc_param<typename ML::type, PL>(p, pl));
  }
/*
  typename fnt_ty::fnt_ty operator()(
      const typename ML::type &p1,
      const typename ML::tail::type &p2) {
    return (*this)(p1)(p2);
  }
*/
  
};

template <class K, class PL>
struct smoc_accumulate_param<K, smoc_missing<void>, PL>
: public smoc_member_func_interface<typename K::return_type> {
  
  K  k;
  PL pl;
  
  smoc_accumulate_param(const K &k, const PL &_pl = PL())
    : k(k), pl(_pl) {}
  
  typename K::return_type operator()() const
    { return k.callIt(pl); }
  const char *getFuncName() const 
    { return k.name; }
};

template <typename R, class F>
struct smoc_member_func;

template <typename R, class T>
struct smoc_member_func<R,R (T::*)()> {
  typedef R		return_type;
  typedef smoc_missing<void> missing_type; 
  
  T          *obj;
  R      (T::*f)();
  const char *name;
  
  template <class X>
  smoc_member_func( X *_obj, R (T::*_f)(), const char *_name )
    : obj(dynamic_cast<T *>(_obj)), f(_f), name(_name)
    { assert(obj != NULL &&  f != NULL); }
  
  template <class PL>
  R callIt( const PL &pl ) const {
    return (obj->*f)();
  }
};
template <typename R, class T, typename P1>
struct smoc_member_func<R,R (T::*)(P1)> {
  typedef R		return_type;
  typedef smoc_missing<P1, smoc_missing<void> > missing_type; 
  
  T	     *obj;
  R      (T::*f)(P1);
  const char *name;
  
  template <class X>
  smoc_member_func( X *_obj, R (T::*_f)(P1), const char *_name )
    : obj(dynamic_cast<T *>(_obj)), f(_f), name(_name)
    { assert(obj != NULL &&  f != NULL); }
  
  template <class PL>
  R callIt( const PL &pl ) const {
    return (obj->*f)(pl.p);
  }
};
template <typename R, class T, typename P1, typename P2>
struct smoc_member_func<R,R (T::*)(P1,P2)> {
  typedef R		return_type;
  typedef smoc_missing<P1, smoc_missing<P2, smoc_missing<void> > > missing_type; 
  
  T	     *obj;
  R      (T::*f)(P1, P2);
  const char *name;
  
  template <class X>
  smoc_member_func( X *_obj, R (T::*_f)(P1, P2), const char *_name )
    : obj(dynamic_cast<T *>(_obj)), f(_f), name(_name)
    { assert(obj != NULL &&  f != NULL); }
  
  template <class PL>
  R callIt( const PL &pl ) const  {
    return (obj->*f)(pl.pn.p, pl.p);
  }
};
template <typename R, class T, typename P1, typename P2, typename P3>
struct smoc_member_func<R,R (T::*)(P1,P2,P3)> {
  typedef R		return_type;
  typedef smoc_missing<P1, smoc_missing<P2, smoc_missing<P3, smoc_missing<void> > > > missing_type; 
  
  T	     *obj;
  R      (T::*f)(P1, P2, P3);
  const char *name;
  
  template <class X>
  smoc_member_func( X *_obj, R (T::*_f)(P1, P2, P3), const char *_name )
    : obj(dynamic_cast<T *>(_obj)), f(_f), name(_name)
    { assert(obj != NULL &&  f != NULL); }
  
  template <class PL>
  R callIt( const PL &pl ) const {
    return (obj->*f)(pl.pn.pn.p, pl.pn.p, pl.p);
  }
};

/*
template <typename R, class F>
struct smoc_member_func;

template <typename R, class T>
struct smoc_member_func<R,R (T::*)()> {
  typedef R		return_type;
  typedef smoc_missing<void> missing_type; 
  
  T     *obj;
  R (T::*f)();
  
  template <class X>
  smoc_member_func( X *_obj, R (T::*_f)() )
    : obj(dynamic_cast<T *>(_obj)), f(_f)
    { assert(obj != NULL &&  f != NULL); }
  
  template <class PL>
  R callIt( const PL &pl ) const {
    return (obj->*f)();
  }
};
template <typename R, class T, typename P1>
struct smoc_member_func<R,R (T::*)(P1)> {
  typedef R		return_type;
  typedef smoc_missing<P1, smoc_missing<void> > missing_type; 
  
  T     *obj;
  R (T::*f)(P1);
  
  template <class X>
  smoc_member_func( X *_obj, R (T::*_f)(P1) )
    : obj(dynamic_cast<T *>(_obj)), f(_f)
    { assert(obj != NULL &&  f != NULL); }
  
  template <class PL>
  R callIt( const PL &pl ) const {
    return (obj->*f)(pl.p);
  }
};
template <typename R, class T, typename P1, typename P2>
struct smoc_member_func<R,R (T::*)(P1,P2)> {
  typedef R		return_type;
  typedef smoc_missing<P1, smoc_missing<P2, smoc_missing<void> > > missing_type; 
  
  T     *obj;
  R (T::*f)(P1, P2);
  
  template <class X>
  smoc_member_func( X *_obj, R (T::*_f)(P1, P2) )
    : obj(dynamic_cast<T *>(_obj)), f(_f)
    { assert(obj != NULL &&  f != NULL); }
  
  template <class PL>
  R callIt( const PL &pl ) const  {
    return (obj->*f)(pl.pn.p, pl.p);
  }
};
template <typename R, class T, typename P1, typename P2, typename P3>
struct smoc_member_func<R,R (T::*)(P1,P2,P3)> {
  typedef R		return_type;
  typedef smoc_missing<P1, smoc_missing<P2, smoc_missing<P3, smoc_missing<void> > > > missing_type; 
  
  T     *obj;
  R (T::*f)(P1, P2, P3);
  
  template <class X>
  smoc_member_func( X *_obj, R (T::*_f)(P1, P2, P3) )
    : obj(dynamic_cast<T *>(_obj)), f(_f)
    { assert(obj != NULL &&  f != NULL); }
  
  template <class PL>
  R callIt( const PL &pl ) const {
    return (obj->*f)(pl.pn.pn.p, pl.pn.p, pl.p);
  }
};



template <typename R, class T, typename P1 = void, typename P2 = void, typename P3 = void>
struct smoc_member_func;

template <typename R, class T>
struct smoc_member_func<R,T,void,void,void>
: public smoc_member_func_interface<R> {
  T     *obj;
  R (T::*f)();
  
  smoc_member_func( T *_obj, R (T::*_f)() )
    : obj(_obj), f(_f)
    { assert(obj != NULL &&  f != NULL); }
  
  R operator()() const { return (obj->*f)(); }
};
template <typename R, class T, typename P1>
struct smoc_member_func<R,T,P1,void,void>
: public smoc_member_func_interface<R> {
  T     *obj;
  R (T::*f)(P1);
  P1     p1;
  
  smoc_member_func( T *_obj, R (T::*_f)(P1), const P1 &_p1 )
    : obj(_obj), f(_f), p1(_p1)
    { assert(obj != NULL &&  f != NULL); }
  
  R operator()() const { return (obj->*f)(p1); }
};
template <typename R, class T, typename P1, typename P2>
struct smoc_member_func<R,T,P1,P2,void>
: public smoc_member_func_interface<R> {
  T     *obj;
  R (T::*f)(P1, P2);
  P1     p1;
  P2     p2;
  
  smoc_member_func( T *_obj, R (T::*_f)(P1, P2), const P1 &_p1, const P2 &_p2 )
    : obj(_obj), f(_f), p1(_p1), p2(_p2)
    { assert(obj != NULL &&  f != NULL); }
  
  R operator()() const { return (obj->*f)(p1, p2); }
};
template <typename R, class T, typename P1, typename P2, typename P3>
struct smoc_member_func
: public smoc_member_func_interface<R> {
  T     *obj;
  R (T::*f)(P1, P2, P3);
  P1     p1;
  P2     p2;
  P3     p3;
  
  smoc_member_func( T *_obj, R (T::*_f)(P1, P2, P3), const P1 &_p1, const P2 &_p2, const P3 &_p3 )
    : obj(_obj), f(_f), p1(_p1), p2(_p2), p3(_p3)
    { assert(obj != NULL &&  f != NULL); }
  
  R operator()() const { return (obj->*f)(p1, p2, p3); }
};
*/

class smoc_func_call {
private:
  boost::intrusive_ptr<
    smoc_member_func_interface<void> >   k;
public:
  template <class K>
  smoc_func_call( const K &_k )
    : k(new K(_k)) {}

  void operator()() const
    { return k->operator()(); }
  const char* getFuncName() const
    { return k->getFuncName(); }
};

class smoc_func_diverge {
private:
  typedef const smoc_firing_state & return_type;
  
  boost::intrusive_ptr<
    smoc_member_func_interface<
      const smoc_firing_state &> >  k;
public:
  template <class K>
  smoc_func_diverge( const K &_k )
    : k(new K(_k)) {}
  template <class T>
  smoc_func_diverge( T *_obj, return_type (T::*_f)() )
    : k(new smoc_accumulate_param<smoc_member_func<return_type, return_type (T::*)()>,
				  smoc_missing<void> >
	(smoc_member_func<return_type, return_type (T::*)()>(_obj, _f, "")) )
    {}
  
  const smoc_firing_state &operator()() const
    { return k->operator()(); }
};

class smoc_func_branch: public smoc_func_diverge {
public:
  template <class K>
  smoc_func_branch( const K &_k )
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
  typedef jflibs::oneof<smoc_func_call, smoc_func_branch, smoc_func_diverge>
                                          func_ty;
  
  class transition_ty {
  public:
    smoc_activation_pattern ap;
    func_ty                 f;
    statelist_ty            sl;
  public:
    transition_ty( smoc_firing_state_ref *r, const smoc_transition &t );
    
    smoc_root_port_bool knownSatisfiable() const
      { return ap.knownSatisfiable(); }
    
    bool tryExecute(resolved_state_ty **rs, smoc_root_node *actor);
    void findBlocked(smoc_root_port_bool_list &l, smoc_root_node *actor);
    
    void dump(std::ostream &out) const;
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
    
    bool tryExecute(resolved_state_ty **rs, smoc_root_node *actor);
    void findBlocked(smoc_root_port_bool_list &l, smoc_root_node *actor);
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
//private:
protected:
  resolved_state_ty *rs;
  smoc_firing_rules *fr;
protected:
  // create new empty state with new firing_rules
  smoc_firing_state_ref();
  
  // make a copy of the state
  void mkCopy( const this_type &rhs );
public:
  smoc_firing_state_ref( const smoc_firing_state_ref &n );
  
  resolved_state_ty &getResolvedState() const
    { assert( rs != NULL ); return *rs; }
  
  void finalise( smoc_root_node *actor ) const;
  bool tryExecute();
  void findBlocked(smoc_root_port_bool_list &l);
  void dump( std::ostream &o ) const;
  const smoc_firing_rules &getFiringRules() const
    { return *fr; }
  
  ~smoc_firing_state_ref();
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_firing_state_ref &s )
  { s.dump(out); return out; }

class smoc_firing_state
  :public smoc_firing_state_ref {
public:
  friend class smoc_opbase_node;
  friend class transition_ty;
  
  typedef smoc_firing_state this_type;
public:
  smoc_firing_state( const smoc_transition_list &tl )
    { this->operator = (tl); }
  smoc_firing_state( const smoc_transition &t )
    { this->operator = (t); }
  smoc_firing_state()
    {}
  smoc_firing_state( const this_type &x )
    { *this = x; }
  
//  bool isResolvedState() const { return rs != NULL; }
  
  this_type &operator = (const this_type &x);
  this_type &operator = (const smoc_transition_list &tl);
  this_type &operator = (const smoc_transition &t);
  
  void addTransition( const smoc_transition_list &tl );
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
  
//  friend class smoc_firing_state;
  friend class smoc_firing_state_ref;
  friend class smoc_firing_types::transition_ty;
private:
  typedef std::set<smoc_firing_state_ref *> references_ty;
  
  statelist_ty            states;
  references_ty           references;
  smoc_root_node         *actor;
  
  void _addRef( smoc_firing_state_ref *s, smoc_firing_rules *p );
protected:
  smoc_firing_rules( smoc_firing_state_ref *s )
    : actor(NULL) {
    assert( s != NULL && s->rs != NULL );
    addRef(s);
    states.push_back(s->rs);
  }
  
  smoc_root_node* getActor() { return actor; }
   
  void addRef( smoc_firing_state_ref *s ) { _addRef(s,NULL); }
  
  void delRef( smoc_firing_state_ref *s );
  
  void unify( smoc_firing_rules *fr );
  
  void finalise( smoc_root_node *actor );
 

public:
  const statelist_ty &getFSMStates() const
    { return states; }
  
  ~smoc_firing_rules() {
    for ( statelist_ty::iterator iter = states.begin();
          iter != states.end();
          ++iter )
      delete *iter;
  }
};

#ifndef _COMPILEHEADER_SMOC_FIRING_STATE_REF__DESTRUCTOR
GNU89_EXTERN_INLINE
#endif
smoc_firing_state_ref::~smoc_firing_state_ref() {
  if ( fr )
    fr->delRef(this);
}

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
  smoc_interface_action(const smoc_firing_state_ref &s)
    : sl(s), f() {}
  smoc_interface_action(const smoc_firing_state_ref &s,
                        const smoc_func_call &f)
    : sl(s), f(f) {}
  smoc_interface_action(const smoc_firing_state_ref &s,
                        const smoc_func_branch &f)
    : sl(s), f(f) {}
  smoc_interface_action(const smoc_firing_state_list &sl,
                        const smoc_func_branch &f)
    : sl(sl), f(f) {}
  smoc_interface_action(const smoc_func_diverge &f)
    : sl(), f(f) {}
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
//    const smoc_transition_part1   &tp1,
      const smoc_activation_pattern &ap,
      const smoc_func_call          &f)
    : ap(ap), f(f) {}
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
      const smoc_interface_action   &ia)
    : ap(ap), ia(ia) {}
  smoc_transition(
      const smoc_activation_pattern &ap,
      const smoc_firing_state_ref   &s)
    : ap(ap), ia(smoc_interface_action(s)) {}
  smoc_transition(
      const smoc_transition_part   &tp,
      const smoc_firing_state_ref   &s)
    : ap(tp.ap),
      ia(smoc_interface_action(s,tp.f)) {}
  
  const smoc_activation_pattern &getActivationPattern() const { return ap; }
  const smoc_interface_action   &getInterfaceAction() const { return ia; }
  this_type onlyInputs() const
    { return this_type(ap.onlyInputs(), ia); }
  this_type onlyOutputs() const
    { return this_type(ap.onlyOutputs(), ia); }
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
smoc_transition operator >> (const smoc_transition_part &tp,
			     const smoc_firing_state_ref &s) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(tp,s);
}

#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_3
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_activation_pattern ap,
                             const smoc_firing_state_ref &s) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(ap,s);
}

/// Legacy stuff
#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_4
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_activation_pattern &ap,
			     const smoc_interface_action &ia) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(ap,ia);
}

#endif // _INCLUDED_SMOC_OP_PORT_LIST_HPP
