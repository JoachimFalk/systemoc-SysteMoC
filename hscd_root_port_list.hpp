// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_OP_PORT_LIST_HPP
#define _INCLUDED_HSCD_OP_PORT_LIST_HPP

#include <vector>
#include <list>
#include <set>
#include <iostream>

#include <oneof.hpp>
#include <commondefs.h>

class hscd_activation_pattern;
class hscd_port2op_if;
class hscd_root_port;

class hscd_op_port {
public:
  friend class hscd_op2port_if;
  friend class hscd_root_port;
  
  typedef hscd_op_port	this_type;
private:
  hscd_root_port               *port;
  size_t                        tokens;
  
  void requestTransfer( hscd_port2op_if &op );
protected:
  hscd_op_port( hscd_root_port &port, size_t tokens )
    : port(&port), tokens(tokens) {}
public:
  operator hscd_root_port &() { return *port; }
  
  hscd_root_port       *getPort() { return port; }
  const hscd_root_port *getPort() const { return port; }
  
  bool isInput() const;
  bool isOutput() const { return !isInput(); }
};

class hscd_activation_pattern
  :public std::vector<hscd_op_port> {
public:
  typedef hscd_activation_pattern this_type;
  
protected:
  template <typename T>
  this_type onlyXXX(const T filter) const {
    hscd_activation_pattern retval;
    
    for ( const_iterator iter = begin();
	  iter != end();
	  ++iter ) {
      if ( filter(*iter) )
	retval.push_back( *iter );
    }
    return retval;
  }
  struct filterInput_ty {
    bool operator ()( const hscd_op_port &port ) const
      { return port.isInput(); }
  };
  struct filterOutput_ty {
    bool operator ()( const hscd_op_port &port ) const
      { return port.isOutput(); }
  };
public:
  this_type onlyInputs() const { return onlyXXX(filterInput_ty()); }
  this_type onlyOutputs() const { return onlyXXX(filterOutput_ty()); }
  
  hscd_activation_pattern() {}
  
  hscd_activation_pattern( hscd_op_port p ) {
    push_back(p);
  }
  
  this_type &operator &= ( hscd_op_port p ) {
    push_back(p); return *this;
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

struct hscd_firing_types {
  struct                              transition_ty;
  typedef std::list<transition_ty>    resolved_state_ty;
  typedef oneof<hscd_firing_state *, resolved_state_ty *>
                                      state_ty;
  typedef std::list<state_ty>         statelist_ty;
  
  struct transition_ty {
    hscd_activation_pattern ap;
    statelist_ty            sl;
  };
};

class hscd_firing_state
  :public hscd_firing_types {
public:
  friend class hscd_opbase_node;
  friend class hscd_firing_rules;
  
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
  
  const resolved_state_ty &getResolvedState() const
    { assert( rs != NULL ); return *rs; }
  
  ~hscd_firing_state();
private:
  // disable
};

class hscd_firing_rules
  :public hscd_firing_types {
public:
  typedef hscd_firing_rules this_type;
  
  friend class hscd_firing_state;
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
  
  void _addRef( hscd_firing_state *s, hscd_firing_rules *p );
protected:
  hscd_firing_rules( hscd_firing_state *s ) {
    addRef(s); resolved_states.push_front(s->rs);
  }
  
  void addRef( hscd_firing_state *s ) { _addRef(s,NULL); }
  
  void delRef( hscd_firing_state *s );
  
  void addUnresolved( state_ty *us ) {
    unresolved_states.push_front(us);
  }
  
  void resolve();
  void unify( hscd_firing_rules *fr );
  
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
  :public std::vector<oneof<hscd_firing_state *, hscd_firing_state> > {
public:
  typedef hscd_firing_state_list  this_type;
public:
  hscd_firing_state_list( const value_type        &v ) {
    push_back(v); }
  hscd_firing_state_list( const hscd_firing_state &n ) {
    push_back(n); }
  hscd_firing_state_list( hscd_firing_state       *n ) {
    push_back(n); }
  this_type &operator |=( const value_type        &v ) {
    push_back(v); return *this; }
  this_type &operator |=( const hscd_firing_state &n ) {
    push_back(n); return *this; }
  this_type &operator |=( hscd_firing_state       *n ) {
    push_back(n); return *this; }
};

#ifndef _COMPILEHEADER_HSCD_FIRING_STATE_LIST_OPERATOR_OR_1
GNU89_EXTERN_INLINE
#endif
hscd_firing_state_list operator |
  (hscd_firing_state_list sl,
   const hscd_firing_state_list::value_type &v ) {
  return sl |= v;
}
#ifndef _COMPILEHEADER_HSCD_FIRING_STATE_LIST_OPERATOR_OR_3
GNU89_EXTERN_INLINE
#endif
hscd_firing_state_list operator |
  (hscd_firing_state_list sl,
   const hscd_firing_state &n ) {
  return sl |= n;
}
#ifndef _COMPILEHEADER_HSCD_FIRING_STATE_LIST_OPERATOR_OR_2
GNU89_EXTERN_INLINE
#endif
hscd_firing_state_list operator |
  (hscd_firing_state_list sl,
   hscd_firing_state *n ) {
  return sl |= n;
}

class hscd_interface_action {
public:
  friend class hscd_opbase_node;
  friend class hscd_firing_state;
  
  typedef hscd_interface_action this_type;
private:
  hscd_firing_state_list  sl;
protected:
  hscd_interface_action(const hscd_firing_state_list &sl)
    : sl(sl) {}
};

struct hscd_node_funcref {
  virtual void operator () = 0;
};

template <typename T>
class hscd_node_funcobj: public hscd_node_funcref {
private:
  void (T::*f)();
public:
  hscd_node_funcobj(void (T::*f)()): f(f) {}
  
  void operator () { return f(); }
};

class hscd_interface_transition {
public:
  friend class hscd_firing_state;
  
  typedef hscd_interface_transition this_type;
private:
  hscd_activation_pattern ap;
  hscd_interface_action   ia;
public:
  hscd_interface_transition(
      const hscd_activation_pattern &ap,
      const hscd_interface_action   &ia )
    : ap(ap), ia(ia) {}
  
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

#ifndef _COMPILEHEADER_HSCD_INTERFACE_TRANSITION__OPERATOR_SHIFTRR
GNU89_EXTERN_INLINE
#endif
hscd_interface_transition operator >> (hscd_activation_pattern p, hscd_interface_action a) {
  std::cerr << ">>" << std::endl;
  return hscd_interface_transition(p,a);
}

#endif // _INCLUDED_HSCD_OP_PORT_LIST_HPP
