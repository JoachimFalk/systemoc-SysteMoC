// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_OP_PORT_LIST_HPP
#define _INCLUDED_HSCD_OP_PORT_LIST_HPP

#include <vector>
#include <list>
#include <set>
#include <map>

#include <iostream>

#include <hscd_root_port.hpp>
#include <oneof.hpp>
#include <commondefs.h>

class hscd_activation_pattern;
class hscd_interface_transition;
class hscd_port2op_if;
class hscd_root_port;
class hscd_root_node;

template <typename T> class hscd_port_in;
template <typename T> class hscd_port_out;

class hscd_op_port {
public:
  typedef hscd_op_port  this_type;

  friend class hscd_activation_pattern;
  friend class hscd_firing_state;
private:
  hscd_root_port *port;
  size_t          commit;
protected:
  template <typename T> friend class hscd_port_in;
  template <typename T> friend class hscd_port_out;
  
  bool stillPossible() const {
    return (commit >= port->committedCount()) &&
           (commit <= port->maxCommittableCount());
  }
  
  hscd_op_port( hscd_root_port *port, size_t commit )
    : port(port), commit(commit) {}
  
  void                  addCommitCount( size_t n ) { commit += n; }
public:
  size_t                commitCount() const { return commit; }
  
  hscd_root_port       *getPort()           { return port; }
  const hscd_root_port *getPort()     const { return port; }
  
  bool knownUnsatisfiable() const
    { return commit  > port->maxAvailableCount() || !stillPossible(); }
  bool knownSatisfiable()  const
    { return commit <= port->availableCount() && stillPossible(); }
  bool satisfied()   const
    { return commit == port->doneCount() && stillPossible(); }
  bool isInput()     const { return port->isInput();  }
  bool isOutput()    const { return port->isOutput(); }
  bool isUplevel()   const { return port->isUplevel(); }
  
  void reset()    { port->reset(); }
  void transfer() { port->setCommittedCount(commit); port->transfer(); }
};

class hscd_activation_pattern
  :public std::map<const hscd_root_port *, hscd_op_port> {
public:
  typedef hscd_activation_pattern this_type;
  
  friend class hscd_firing_state;
protected:
  sc_event *_blocked;
  
  void block( sc_event *e ) { _blocked = e; }
  void unblock() {
    assert( _blocked != NULL );
    _blocked->notify();
    _blocked = NULL;
  }
  
  template <typename T>
  struct filterNot_ty: public T {
    bool operator ()( const hscd_op_port &port ) const
      { return !T::operator ()(port); }
  };
  
  template <typename T>
  this_type grep() const {
    hscd_activation_pattern retval;
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
    bool operator ()( const hscd_op_port &port ) const
      { return port.isInput(); }
  };
  typedef filterNot_ty<filterInput_ty> filterOutput_ty;
  
  struct filterUplevel_ty {
    bool operator ()( const hscd_op_port &port ) const
      { return port.isUplevel(); }
  };
  typedef filterNot_ty<filterUplevel_ty> filterNotUplevel_ty;
  
  struct filterKnownSatisfiable_ty {
    bool operator ()( const hscd_op_port &port ) const
      { return port.knownSatisfiable(); }
  };
  typedef filterNot_ty<filterKnownSatisfiable_ty> filterNotKnownSatisfiable_ty;
  
  struct filterKnownUnsatisfiable_ty {
    bool operator ()( const hscd_op_port &port ) const
      { return port.knownUnsatisfiable(); }
  };
  typedef filterNot_ty<filterKnownUnsatisfiable_ty> filterNotKnownUnsatisfiable_ty;
  
  struct filterSatisfied_ty {
    bool operator ()( const hscd_op_port &port ) const
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
  
  bool isBlocked() const { return _blocked != NULL; }
  
  hscd_activation_pattern() {}
  
  hscd_activation_pattern( hscd_op_port p ) {
    (*this) &= p;
  }
  
  this_type &operator &= ( hscd_op_port p ) {
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
};

#ifndef _COMPILEHEADER_HSCD_ACTIVATION_PATTERN__OPERATOR_AND
GNU89_EXTERN_INLINE
#endif
hscd_activation_pattern operator & (hscd_activation_pattern ap, hscd_op_port p ) {
  return ap &= p;
}

class hscd_firing_state;
class hscd_firing_rules;
class hscd_transition_list;

class hscd_func_call {
private:
  struct dummy;
  typedef void (dummy::*fun)();

  dummy *obj;
  fun    f;
public:
  template <class X, class T>
  hscd_func_call( X *_obj, void (T::*_f)() )
    : obj(reinterpret_cast<dummy *>(
            /*dynamic_cast<T *>*/(_obj))),
      f(*reinterpret_cast<fun *>(&_f))
    { assert(f != NULL && obj != NULL); }

  void operator()() const { (obj->*f)(); }
};

class hscd_func_branch {
private:
  struct dummy;
  typedef const hscd_firing_state &(dummy::*fun)();

  dummy *obj;
  fun    f;
public:
  template <class X, class T>
  hscd_func_branch( X *_obj, const hscd_firing_state &(T::*_f)() )
    : obj(reinterpret_cast<dummy *>(
            /*dynamic_cast<T *>*/(_obj))),
      f(*reinterpret_cast<fun *>(&_f))
    { assert(f != NULL && obj != NULL); }

  const hscd_firing_state &operator()() const { return (obj->*f)(); }
};

struct hscd_firing_types {
  struct                                  transition_ty;
  struct                                  resolved_state_ty;
  typedef oneof<hscd_firing_state *, resolved_state_ty *>
                                          state_ty;
  typedef std::list<state_ty>             statelist_ty;
  typedef std::list<transition_ty>        transitionlist_ty;
  typedef std::pair<bool,transition_ty *> maybe_transition_ty;
//  typedef std::set<hscd_root_port *>  ports_ty;
  typedef oneof<hscd_func_call, hscd_func_branch>
                                          func_ty;
  
  class transition_ty {
  public:
    hscd_activation_pattern ap;
    func_ty                 f;
    statelist_ty            sl;
    
    void initTransition(
        hscd_firing_rules *fr,
        const hscd_interface_transition &t );
    
    resolved_state_ty *execute();
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

class hscd_firing_state
  :public hscd_firing_types {
public:
  friend class hscd_opbase_node;
  friend class hscd_firing_rules;
  friend class transition_ty;
  
  typedef hscd_firing_state this_type;
private:
  resolved_state_ty *rs;
  hscd_firing_rules *fr;
protected:
  hscd_firing_state( const hscd_transition_list &tl );
public:
  hscd_firing_state(): rs(NULL), fr(NULL) {}
  hscd_firing_state( const this_type &x )
    : rs(NULL), fr(NULL) { *this = x; }
  
  this_type &operator = (const this_type &x);

  bool inductionStep();
  bool choiceStep();

  void dump( std::ostream &o ) const;
  
  bool isResolvedState() const { return rs != NULL; }
  resolved_state_ty &getResolvedState() const
    { assert( rs != NULL ); return *rs; }
  
  void finalise( hscd_root_node *actor ) const;
  
  void execute( transition_ty *t ) { rs = t->execute(); }
  
  ~hscd_firing_state();
private:
  hscd_port_list &getPorts() const;
  // disable
};

class hscd_firing_state_ref
  :public hscd_firing_types {
public:
  typedef hscd_firing_state_ref this_type;
private:
  oneof<hscd_firing_state *, hscd_firing_state> s;
public:
  hscd_firing_state_ref( const hscd_firing_state &n )
    : s(n) { assert( n.isResolvedState() ); }
  hscd_firing_state_ref( hscd_firing_state       *n )
    : s(n) { assert( n != NULL /*&& !n->isResolvedState()*/ ); }
  hscd_firing_state_ref( hscd_firing_state       &n ) {
    if ( n.isResolvedState() )
      s = n;
    else
      s = &n;
  }
  
  operator hscd_firing_state *() const
    { return s; }
  operator hscd_firing_state() const
    { return s; }
  
  bool isResolved() const
    { assert( s.type() == 1 || s.type() == 2 ); return s.type() == 2; }
};

static inline
std::ostream &operator <<( std::ostream &out, const hscd_firing_state &s )
  { s.dump(out); return out; }

class hscd_firing_rules
  :public hscd_firing_types {
public:
  typedef hscd_firing_rules this_type;
  
  friend class hscd_firing_state;
  friend class hscd_firing_types::transition_ty;
private:
  typedef  std::set<hscd_firing_state *>
    references_ty;
  typedef std::list<resolved_state_ty *>
    resolved_states_ty;
  typedef std::list<state_ty *>
    unresolved_states_ty;
  
  unresolved_states_ty  unresolved_states;
  resolved_states_ty    resolved_states;
  references_ty         references;
  hscd_root_node       *actor;
  
  void _addRef( hscd_firing_state *s, hscd_firing_rules *p );
protected:
  hscd_firing_rules( hscd_firing_state *s )
    : actor(NULL) {
    addRef(s); resolved_states.push_front(s->rs);
  }
  
  void addRef( hscd_firing_state *s ) { _addRef(s,NULL); }
  
  void delRef( hscd_firing_state *s );
  
  void addUnresolved( state_ty *us ) {
    unresolved_states.push_front(us);
  }
  
  void resolve();
  void unify( hscd_firing_rules *fr );
  
  void finalise( hscd_root_node *actor );
  
  ~hscd_firing_rules() {
    for ( resolved_states_ty::iterator iter = resolved_states.begin();
          iter != resolved_states.end();
          ++iter )
      delete *iter;
  }
};

#ifndef _COMPILEHEADER_HSCD_FIRING_STATE__DESTRUCTOR
GNU89_EXTERN_INLINE
#endif
hscd_firing_state::~hscd_firing_state() {
  if ( fr )
    fr->delRef(this);
}

class hscd_firing_state_list
  :public std::vector<hscd_firing_state_ref> {
public:
  typedef hscd_firing_state_list  this_type;
public:
  hscd_firing_state_list( const value_type        &v ) {
    push_back(v); }
  this_type &operator |=( const value_type        &v ) {
    push_back(v); return *this; }
};

#ifndef _COMPILEHEADER_HSCD_FIRING_STATE_LIST_OPERATOR_OR_1
GNU89_EXTERN_INLINE
#endif
hscd_firing_state_list operator |
  (hscd_firing_state_list sl,
   const hscd_firing_state_list::value_type &v ) {
  return sl |= v;
}
#ifndef _COMPILEHEADER_HSCD_FIRING_STATE_LIST_OPERATOR_OR_2
GNU89_EXTERN_INLINE
#endif
hscd_firing_state_list operator |
  (const hscd_firing_state_list::value_type &v1,
   const hscd_firing_state_list::value_type &v2 ) {
  return hscd_firing_state_list(v1) |= v2;
}

class hscd_interface_action {
public:
  friend class hscd_opbase_node;
  friend class hscd_firing_types::transition_ty;
  friend class hscd_interface_transition operator >>
    (hscd_activation_pattern p, const hscd_firing_state_ref &s);
//  friend class hscd_firing_state;
  
  typedef hscd_interface_action this_type;
private:
  hscd_firing_state_list     sl;
  hscd_firing_types::func_ty f;
protected:
  hscd_interface_action(const hscd_firing_state_ref &s)
    : sl(s), f() {}
  hscd_interface_action(const hscd_firing_state_ref &s,
                        const hscd_func_call &f)
    : sl(s), f(f) {}
  hscd_interface_action(const hscd_firing_state_ref &s,
                        const hscd_func_branch &f)
    : sl(s), f(f) {}
  hscd_interface_action(const hscd_firing_state_list &sl,
                        const hscd_func_branch &f)
    : sl(sl), f(f) {}
};

class hscd_interface_transition {
public:
  friend class hscd_firing_types::transition_ty;
  
  typedef hscd_interface_transition this_type;
private:
  hscd_activation_pattern ap;
  hscd_interface_action   ia;
public:
  hscd_interface_transition(
      const hscd_activation_pattern &ap,
      const hscd_interface_action   &ia )
    : ap(ap), ia(ia) {}
  
  const hscd_activation_pattern &getActivationPattern() { return ap; }
  const hscd_interface_action   &getInterfaceAction() { return ia; }
  this_type onlyInputs() const { return this_type(ap.onlyInputs(), ia); }
  this_type onlyOutputs() const { return this_type(ap.onlyOutputs(), ia); }
};

class hscd_transition_list
  :public std::vector<hscd_interface_transition> {
public:
  typedef hscd_transition_list this_type;
protected:
public:
  hscd_transition_list() {}
  
  hscd_transition_list( hscd_interface_transition t ) {
    push_back(t);
  }
  
  this_type &operator |= ( hscd_interface_transition t ) {
    push_back(t); return *this;
  }
};

#ifndef _COMPILEHEADER_HSCD_TRANSITION_LIST__OPERATOR_OR
GNU89_EXTERN_INLINE
#endif
hscd_transition_list operator | (hscd_transition_list tl, hscd_interface_transition t ) {
  return tl |= t;
}

#ifndef _COMPILEHEADER_HSCD_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_1
GNU89_EXTERN_INLINE
#endif
hscd_interface_transition operator >> (hscd_activation_pattern p, hscd_interface_action a) {
//  std::cerr << ">>" << std::endl;
  return hscd_interface_transition(p,a);
}
#ifndef _COMPILEHEADER_HSCD_INTERFACE_TRANSITION__OPERATOR_SHIFTRR_2
GNU89_EXTERN_INLINE
#endif
hscd_interface_transition operator >> (hscd_activation_pattern p, const hscd_firing_state_ref &s) {
//  std::cerr << ">>" << std::endl;
  return hscd_interface_transition(p,hscd_interface_action(s));
}

#endif // _INCLUDED_HSCD_OP_PORT_LIST_HPP
