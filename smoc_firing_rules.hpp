// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_OP_PORT_LIST_HPP
#define _INCLUDED_SMOC_OP_PORT_LIST_HPP

#include <iostream>

#include <vector>

#include <list>
#include <jf-libs/stl_output_for_list.hpp>

#include <set>

#include <jf-libs/oneof.hpp>

#include <smoc_guard.hpp>
#include <smoc_root_port.hpp>

#include <boost/shared_ptr.hpp>

#include <jf-libs/commondefs.h>

class smoc_activation_pattern;
class smoc_transition;
class smoc_port2op_if;
class smoc_root_node;

class smoc_firing_state;
class smoc_firing_rules;
class smoc_transition_list;

#define CALL(func) call(&func,#func)

class smoc_func_call {
private:
  struct dummy;
  typedef void (dummy::*fun)();

  dummy *obj;
  fun    f;
  const char *func_name;
public:
  template <class X, class T>
  smoc_func_call( X *_obj, void (T::*_f)() )
    : obj(reinterpret_cast<dummy *>(
            /*dynamic_cast<T *>*/(_obj))),
      f(*reinterpret_cast<fun *>(&_f)),
      func_name("")
    { assert(f != NULL && obj != NULL); }

  template <class X, class T>
  smoc_func_call( X *_obj, void (T::*_f)(), const char *func_name )
    : obj(reinterpret_cast<dummy *>(
            /*dynamic_cast<T *>*/(_obj))),
      f(*reinterpret_cast<fun *>(&_f)),
      func_name(func_name)
    { assert(f != NULL && obj != NULL); }

  void operator()() const { 
    (obj->*f)(); 
  }
  const char* getFuncName(){
    return func_name;
  }
};

class smoc_func_diverge {
private:
  struct dummy;
  typedef const smoc_firing_state &(dummy::*fun)();

  dummy *obj;
  fun    f;
public:
  template <class X, class T>
  smoc_func_diverge( X *_obj, const smoc_firing_state &(T::*_f)() )
    : obj(reinterpret_cast<dummy *>(
            /*dynamic_cast<T *>*/(_obj))),
      f(*reinterpret_cast<fun *>(&_f))
    { assert(f != NULL && obj != NULL); }

  const smoc_firing_state &operator()() const { return (obj->*f)(); }
};

class smoc_func_branch: public smoc_func_diverge {
public:
  template <class X, class T>
  smoc_func_branch( X *_obj, const smoc_firing_state &(T::*_f)() )
    : smoc_func_diverge(_obj,_f) {}
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
  friend class smoc_scheduler_base;
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
