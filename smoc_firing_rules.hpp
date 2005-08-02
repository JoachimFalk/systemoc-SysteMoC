// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_OP_PORT_LIST_HPP
#define _INCLUDED_SMOC_OP_PORT_LIST_HPP

#include <vector>
#include <list>
#include <set>
#include <map>

#include <iostream>

#include <oneof.hpp>
#include <commondefs.h>

#include <smoc_guard.hpp>
#include <smoc_root_port.hpp>

class smoc_activation_pattern;
class smoc_transition;
class smoc_port2op_if;
class smoc_root_node;

class smoc_firing_state;
class smoc_firing_rules;
class smoc_transition_list;

class smoc_func_call {
private:
  struct dummy;
  typedef void (dummy::*fun)();

  dummy *obj;
  fun    f;
public:
  template <class X, class T>
  smoc_func_call( X *_obj, void (T::*_f)() )
    : obj(reinterpret_cast<dummy *>(
            /*dynamic_cast<T *>*/(_obj))),
      f(*reinterpret_cast<fun *>(&_f))
    { assert(f != NULL && obj != NULL); }

  void operator()() const { (obj->*f)(); }
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

struct smoc_firing_types {
  struct                                  transition_ty;
  struct                                  resolved_state_ty;
  typedef oneof<smoc_firing_state *, resolved_state_ty *>
                                          state_ty;
  typedef std::list<state_ty>             statelist_ty;
  typedef std::list<transition_ty>        transitionlist_ty;
  typedef std::pair<bool,transition_ty *> maybe_transition_ty;
//  typedef std::set<smoc_root_port *>  ports_ty;
  typedef oneof<smoc_func_call, smoc_func_branch, smoc_func_diverge>
                                          func_ty;
  
  class transition_ty {
  public:
    smoc_activation_pattern ap;
    func_ty                 f;
    statelist_ty            sl;
    
    sc_event *_blocked;
    
    transition_ty()
      : _blocked(NULL) {}
    
    void block( sc_event *e ) { _blocked = e; }
    void unblock() {
      assert( _blocked != NULL );
      _blocked->notify();
      _blocked = NULL;
    }
    
    void initTransition(
        smoc_firing_rules *fr,
        const smoc_transition &t );

    bool knownSatisfiable() const
      { return ap.knownSatisfiable(); }
    bool knownUnsatisfiable() const
      { return ap.knownUnsatisfiable(); }
/*    void reset() {
      ap_pre_exec.reset(); 
      ap_post_exec.reset(); 
    }*/
    
    bool isBlocked() const { return _blocked != NULL; }
    
    resolved_state_ty *execute();

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
    
    transition_ty &addTransition() {
      tl.push_back(transition_ty());
      return tl.back();
    }
    
    maybe_transition_ty findEnabledTransition();
  };
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_firing_types::transition_ty &t )
  { t.dump(out); return out; }

class smoc_firing_state
  :public smoc_firing_types {
public:
  friend class smoc_opbase_node;
  friend class smoc_firing_rules;
  friend class transition_ty;
  
  typedef smoc_firing_state this_type;
private:
  resolved_state_ty *rs;
  smoc_firing_rules *fr;
protected:
  smoc_firing_state( const smoc_transition_list &tl );
public:
  smoc_firing_state(): rs(NULL), fr(NULL) {}
  smoc_firing_state( const this_type &x )
    : rs(NULL), fr(NULL) { *this = x; }
  

  bool inductionStep();
  bool choiceStep();

  void dump( std::ostream &o ) const;
  
  bool isResolvedState() const { return rs != NULL; }
  resolved_state_ty &getResolvedState() const
    { assert( rs != NULL ); return *rs; }
  
  void finalise( smoc_root_node *actor ) const;
  
  void execute( transition_ty *t ) { rs = t->execute(); }
  
  this_type &operator = (const this_type &x);
  this_type &operator = (const smoc_transition_list &tl);
  this_type &operator = (const smoc_transition &t);
  
  ~smoc_firing_state();
private:
  smoc_port_list &getPorts() const;
  // disable
};

class smoc_firing_state_ref
  :public smoc_firing_types {
public:
  typedef smoc_firing_state_ref this_type;
private:
  oneof<smoc_firing_state *, smoc_firing_state> s;
public:
  smoc_firing_state_ref( const smoc_firing_state &n )
    : s(n) { assert( n.isResolvedState() ); }
  smoc_firing_state_ref( smoc_firing_state       *n )
    : s(n) { assert( n != NULL /*&& !n->isResolvedState()*/ ); }
  smoc_firing_state_ref( smoc_firing_state       &n ) {
    if ( n.isResolvedState() )
      s = n;
    else
      s = &n;
  }
  
  operator smoc_firing_state *() const
    { return s; }
  operator smoc_firing_state() const
    { return s; }
  
  bool isResolved() const
    { assert( s.type() == 1 || s.type() == 2 ); return s.type() == 2; }
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_firing_state &s )
  { s.dump(out); return out; }

class smoc_firing_rules
  :public smoc_firing_types {
public:
  typedef smoc_firing_rules this_type;
  
  friend class smoc_firing_state;
  friend class smoc_firing_types::transition_ty;
private:
  typedef  std::set<smoc_firing_state *>
    references_ty;
  typedef std::list<resolved_state_ty *>
    resolved_states_ty;
  typedef std::list<state_ty *>
    unresolved_states_ty;
  
  unresolved_states_ty  unresolved_states;
  resolved_states_ty    resolved_states;
  references_ty         references;
  smoc_root_node       *actor;
  
  void _addRef( smoc_firing_state *s, smoc_firing_rules *p );
protected:
  smoc_firing_rules( smoc_firing_state *s )
    : actor(NULL) {
    assert( s != NULL && s->rs != NULL );
    addRef(s); resolved_states.push_front(s->rs);
  }
  
  void addRef( smoc_firing_state *s ) { _addRef(s,NULL); }
  
  void delRef( smoc_firing_state *s );
  
  void addUnresolved( state_ty *us ) {
    unresolved_states.push_front(us);
  }
  
  void resolve();
  void unify( smoc_firing_rules *fr );
  
  void finalise( smoc_root_node *actor );
  
  ~smoc_firing_rules() {
    for ( resolved_states_ty::iterator iter = resolved_states.begin();
          iter != resolved_states.end();
          ++iter )
      delete *iter;
  }
};

#ifndef _COMPILEHEADER_SMOC_FIRING_STATE__DESTRUCTOR
GNU89_EXTERN_INLINE
#endif
smoc_firing_state::~smoc_firing_state() {
  if ( fr )
    fr->delRef(this);
}

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

class smoc_interface_action {
public:
  friend class smoc_opbase_node;
  friend class smoc_scheduler_base;
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

class smoc_transition_part1 {
public:
  friend class smoc_transition_part2;
  
  typedef smoc_transition_part1 this_type;
private:
  smoc_activation_pattern    ap1, ap2;
public:
  smoc_transition_part1(
      const smoc_activation_pattern &ap1,
      const smoc_activation_pattern &ap2 = smoc_activation_pattern())
    : ap1(ap1), ap2(ap2) {}
};

class smoc_transition_part2 {
public:
  friend class smoc_transition;
  
  typedef smoc_transition_part2 this_type;
private:
  smoc_activation_pattern    ap1, ap2;
  smoc_func_call             f;
public:
  smoc_transition_part2(
      const smoc_transition_part1 &tp1,
      const smoc_func_call        &f)
    : ap1(tp1.ap1), ap2(tp1.ap2), f(f) {}
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
      const smoc_transition_part2   &tp2,
      const smoc_firing_state_ref   &s)
    : ap(tp2.ap1.concat(tp2.ap2)),
      ia(smoc_interface_action(s,tp2.f)) {}
  
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
smoc_transition_part1 operator >> (const smoc_activation_pattern &ap1,
			           const smoc_activation_pattern &ap2) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition_part1(ap1,ap2);
}
#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_2
GNU89_EXTERN_INLINE
#endif
smoc_transition_part2 operator >> (const smoc_transition_part1   &tp1,
                                   const smoc_func_call          &f) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition_part2(tp1,f);
}
#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_3
GNU89_EXTERN_INLINE
#endif
smoc_transition_part2 operator >> (const smoc_activation_pattern &ap,
                                   const smoc_func_call          &f) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition_part2(smoc_transition_part1(ap),f);
}
#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_4
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_transition_part2 &tp2,
			     const smoc_firing_state_ref &s) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(tp2,s);
}

#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_5
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_activation_pattern ap,
                             const smoc_firing_state_ref &s) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(ap,s);
}

/// Legacy stuff
#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_6
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_activation_pattern &ap,
			     const smoc_interface_action &ia) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(ap,ia);
}

#endif // _INCLUDED_SMOC_OP_PORT_LIST_HPP
