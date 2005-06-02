// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_OP_PORT_LIST_HPP
#define _INCLUDED_SMOC_OP_PORT_LIST_HPP

#include <vector>
#include <list>
#include <set>
#include <map>

#include <iostream>

#include <smoc_root_port.hpp>
#include <oneof.hpp>
#include <commondefs.h>

class smoc_activation_pattern;
class smoc_transition;
class smoc_port2op_if;
class smoc_root_node;

class smoc_activation_pattern
  :public std::map<const smoc_root_port *, smoc_op_port> {
public:
  typedef smoc_activation_pattern this_type;
  
  friend class smoc_firing_state;
protected:
  template <typename T>
  struct filterNot_ty: public T {
    bool operator ()( const smoc_op_port &port ) const
      { return !T::operator ()(port); }
  };
  
  template <typename T>
  this_type grep() const {
    smoc_activation_pattern retval;
    const T filter = T();
    
    for ( const_iterator iter = begin();
	  iter != end();
	  ++iter )
      if ( filter(iter->second) )
	retval.insert( *iter );
    return retval;
  }
  template <typename T>
  bool isAnd() const {
    const T filter = T();
    
    for ( const_iterator iter = begin();
	  iter != end();
	  ++iter )
      if ( !filter(iter->second) )
        return false;
    return true;
  }
  template <typename T>
  bool isOr() const { return !isAnd<filterNot_ty<T> >(); }
  
  struct filterInput_ty {
    bool operator ()( const smoc_op_port &port ) const
      { return port.isInput(); }
  };
  typedef filterNot_ty<filterInput_ty> filterOutput_ty;
  
  struct filterUplevel_ty {
    bool operator ()( const smoc_op_port &port ) const
      { return port.isUplevel(); }
  };
  typedef filterNot_ty<filterUplevel_ty> filterNotUplevel_ty;
  
  struct filterKnownSatisfiable_ty {
    bool operator ()( const smoc_op_port &port ) const
      { return port.knownSatisfiable(); }
  };
  typedef filterNot_ty<filterKnownSatisfiable_ty> filterNotKnownSatisfiable_ty;
  
  struct filterKnownUnsatisfiable_ty {
    bool operator ()( const smoc_op_port &port ) const
      { return port.knownUnsatisfiable(); }
  };
  typedef filterNot_ty<filterKnownUnsatisfiable_ty> filterNotKnownUnsatisfiable_ty;
  
  struct filterSatisfied_ty {
    bool operator ()( const smoc_op_port &port ) const
      { return port.satisfied(); }
  };
public:
  this_type onlyInputs() const { return grep<filterInput_ty>(); }
  this_type onlyOutputs() const { return grep<filterOutput_ty>(); }
  this_type onlyUplevel() const { return grep<filterUplevel_ty>(); }
  this_type onlyNotUplevel() const { return grep<filterNotUplevel_ty>(); }
  bool      knownSatisfiable() const { return isAnd<filterKnownSatisfiable_ty>(); }
  bool      knownUnsatisfiable() const { return isOr<filterKnownUnsatisfiable_ty>(); }
  bool      satisfied() const { return isAnd<filterSatisfied_ty>(); }
  
  smoc_activation_pattern() {}
  
  smoc_activation_pattern( smoc_op_port p ) {
    (*this) &= p;
  }
  
  this_type &operator &= ( smoc_op_port p ) {
    std::pair<iterator, bool> x = insert( value_type(p.getPort(), p) );
    if ( !x.second ) {
      assert( p.getPort() == x.first->second.getPort() );
      x.first->second.addCommitCount(p.commitCount());
    }
    return *this;
  }

  void execute() {
    for ( iterator iter = begin();
	  iter != end();
	  ++iter )
      iter->second.transfer();
  }

  void reset() {
    for ( iterator iter = begin();
	  iter != end();
	  ++iter )
      iter->second.reset();
  }

  void dump(std::ostream &out) const;
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_activation_pattern &ap)
  { ap.dump(out); return out; }

#ifndef _COMPILEHEADER_SMOC_ACTIVATION_PATTERN__OPERATOR_AND
GNU89_EXTERN_INLINE
#endif
smoc_activation_pattern operator & (smoc_activation_pattern ap, smoc_op_port p ) {
  return ap &= p;
}

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
    smoc_activation_pattern ap_pre_check,  ap_pre_exec;
    smoc_activation_pattern ap_post_check, ap_post_exec;
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

    bool knownSatisfiable() const {
      return ap_pre_check.knownSatisfiable() &&
             ap_post_check.knownSatisfiable();
    }
    bool knownUnsatisfiable() const {
      return ap_pre_check.knownUnsatisfiable() ||
             ap_post_check.knownUnsatisfiable();
    }
    void reset() {
      ap_pre_exec.reset(); 
      ap_post_exec.reset(); 
    }
    
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
  friend class smoc_transition operator >>
    (smoc_activation_pattern p, const smoc_firing_state_ref &s);
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
  smoc_activation_pattern ap_pre;
  smoc_interface_action   ia;
public:
  smoc_transition_part(
      const smoc_activation_pattern &ap,
      const smoc_interface_action   &ia )
    : ap_pre(ap), ia(ia) {}
};

class smoc_transition {
public:
  friend class smoc_firing_types::transition_ty;
  
  typedef smoc_transition this_type;
private:
  smoc_activation_pattern ap_pre;
  smoc_interface_action   ia;
  smoc_activation_pattern ap_post;
public:
  smoc_transition(
      const smoc_transition_part    &tp,
      const smoc_activation_pattern &ap_post = smoc_activation_pattern())
    : ap_pre(tp.ap_pre), ia(tp.ia), ap_post(ap_post) {}
  smoc_transition(
      const smoc_interface_action   &ia,
      const smoc_activation_pattern &ap_post)
    : ia(ia), ap_post(ap_post) {}
  smoc_transition(
      const smoc_activation_pattern &ap_pre,
      const smoc_interface_action   &ia,
      const smoc_activation_pattern &ap_post = smoc_activation_pattern())
    : ap_pre(ap_pre), ia(ia), ap_post(ap_post) {}
  
  const smoc_activation_pattern &getActivationPattern() { return ap_pre; }
  const smoc_interface_action   &getInterfaceAction() { return ia; }
  this_type onlyInputs() const
    { return this_type(ap_pre.onlyInputs(), ia, ap_post.onlyInputs()); }
  this_type onlyOutputs() const
    { return this_type(ap_pre.onlyOutputs(), ia, ap_post.onlyOutputs()); }
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
				  const smoc_interface_action &ia) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition_part(ap,ia);
}
#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_2
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_transition_part    &tp,
			     const smoc_activation_pattern &ap) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(tp,ap);
}
#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_3
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (const smoc_interface_action   &ia,
			     const smoc_activation_pattern &ap) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(ia,ap);
}
#ifndef _COMPILEHEADER_SMOC_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_4
GNU89_EXTERN_INLINE
#endif
smoc_transition operator >> (smoc_activation_pattern p, const smoc_firing_state_ref &s) {
//  std::cerr << ">>" << std::endl;
  return smoc_transition(p,smoc_interface_action(s));
}

#endif // _INCLUDED_SMOC_OP_PORT_LIST_HPP
