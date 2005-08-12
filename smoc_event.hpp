
#include <systemc.h>

#include <iostream>

#include <vector>
#include <set>

#ifndef _INCLUDED_SMOC_EVENT_HPP
#define _INCLUDED_SMOC_EVENT_HPP

class smoc_event_listener {
public:
  friend class smoc_event;
protected:
  virtual void signaled( smoc_event *e ) = 0;
};

class smoc_event {
public:
  typedef smoc_event                        this_type;
  typedef void (this_type::*unspecified_bool_type)();
protected:
  typedef std::set<smoc_event_listener *>  ell_ty;
  
  size_t  missing;
  ell_ty  ell;
public:
  smoc_event(): missing(1)
    {}
  
  void notify() {
    missing = 0;
    for ( ell_ty::iterator iter = ell.begin();
          iter != ell.end();
          ++iter )
      (*iter)->signaled(this);
    ell.clear();
  }
  
  virtual void addListener(smoc_event_listener *el) {
    bool success = ell.insert(el).second;
    assert( success );
  }
  virtual void delListener(smoc_event_listener *el) {
    ell.erase(el);
  }
  
  virtual void reset()
    { missing = 1; }
  
  operator unspecified_bool_type() const
    { return missing == 0 ? &this_type::notify : NULL; }
  
  class smoc_event_or_list  operator | ( smoc_event &p );
  class smoc_event_and_list operator & ( smoc_event &p );
  
  virtual ~smoc_event() {}
  
  void dump(std::ostream &out) const {
    std::cout << "smoc_event( missing: " << missing << ")";
  }
private:
  // disable
  // smoc_event( const this_type & );
};

static inline
std::ostream &operator << ( std::ostream &out, const smoc_event &se ) {
  se.dump(out); return out;
}

class smoc_event_or_list
: public std::vector<smoc_event *>,
  public smoc_event,
  protected smoc_event_listener {
protected:
  void signaled( smoc_event *e )
    { clearListener(); notify();  }
  
  void clearListener() {
    for ( iterator iter = begin();
	  iter != end();
	  ++iter )
      (*iter)->delListener(this);
  }
public:
  typedef smoc_event_or_list this_type;
  
  smoc_event_or_list()
    {}
  smoc_event_or_list( smoc_event &p )
    { *this |= p; }
  this_type operator | ( smoc_event &p )
    { return this_type(*this) |= p; }
  this_type &operator |= ( smoc_event &p ) {
    if (p)
      missing = 0;
    push_back(&p);
    return *this;
  }
  
  void addListener(smoc_event_listener *el) {
    if ( ell.empty() ) {
      missing = 1;
      for ( iterator iter = begin();
	    iter != end();
	    ++iter ) {
	if ( !**iter ) {
	  (*iter)->addListener(this);
	} else {
	  missing = 0;
	}
      }
    }
    smoc_event::addListener(el);
    if ( !missing )
      notify();
  }
  void reset() {
    missing = 0;
    for ( iterator iter = begin();
	  iter != end();
	  ++iter ) {
      ++missing;
      (*iter)->reset();
    }
  }

};

class smoc_event_and_list
: public std::vector<smoc_event *>,
  public smoc_event,
  protected smoc_event_listener {
protected:
  void signaled( smoc_event *e ) {
    assert( missing > 0 );
    if ( !--missing )
      notify();
  }
public:
  typedef smoc_event_and_list this_type;
  
  smoc_event_and_list()
    { missing = 0; }
  smoc_event_and_list( smoc_event &p )
    { missing = 0; *this &= p; }
  this_type operator & ( smoc_event &p )
    { return this_type(*this) &= p; }
  this_type &operator &= ( smoc_event &p ) {
    if ( !p )
      ++missing;
    push_back(&p);
    return *this;
  }
  
  void addListener(smoc_event_listener *el) {
    if ( ell.empty() ) {
      missing = 0;
      for ( iterator iter = begin();
	    iter != end();
	    ++iter )
	if ( !**iter ) {
	  ++missing;
	  (*iter)->addListener(this);
	}
    }
    smoc_event::addListener(el);
    if ( !missing )
      notify();
  }
  void reset() {
    missing = 0;
    for ( iterator iter = begin();
	  iter != end();
	  ++iter ) {
      ++missing;
      (*iter)->reset();
    }
  }
  
  virtual ~smoc_event_and_list() {}
};

inline
smoc_event_or_list  smoc_event::operator | ( smoc_event &p )
  { return smoc_event_or_list(*this) |= p; }

inline
smoc_event_and_list smoc_event::operator & ( smoc_event &p )
  { return smoc_event_and_list(*this) &= p; }

static inline
void smoc_notify(smoc_event& se)
  { return se.notify(); }
static inline
void smoc_reset(smoc_event& se)
  { return se.reset(); }
static inline
void smoc_wait( smoc_event &se ) {
  if ( !se ) {
    struct _: public smoc_event_listener {
      sc_event e;
      void signaled( smoc_event * )
      { e.notify(); }
      virtual ~_() {}
    } w;
    se.addListener(&w);
    wait(w.e);
  }
}

#endif // _INCLUDED_SMOC_EVENT_HPP

