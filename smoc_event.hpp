
#include <systemc.h>

#include <vector>
#include <list>

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
  typedef smoc_event this_type;
  typedef void (this_type::*unspecified_bool_type)();
private:
  typedef std::list<smoc_event_listener *>  ell_ty;
  
  bool    notified;
  ell_ty  ell;
public:
  smoc_event(): notified(false) {}
  
  void notify() {
    notified = true;
    for ( ell_ty::iterator iter = ell.begin();
          iter != ell.end();
          ++iter )
      (*iter)->signaled(this);
    ell.clear();
  }
  
  void addListener(smoc_event_listener *el)
    { ell.push_back(el); }
  
  operator unspecified_bool_type() const
    { return notified ? &this_type::notify : NULL; }
  
  class smoc_event_or_list  operator | ( smoc_event &p );
  class smoc_event_and_list operator & ( smoc_event &p );
};

class smoc_event_or_list
: public std::vector<smoc_event *> {
public:
  typedef smoc_event_or_list this_type;
  
  smoc_event_or_list( smoc_event &p )
    { push_back(&p); }
  this_type &operator |= ( smoc_event &p )
    { push_back(&p); return *this; }
  this_type operator | ( smoc_event &p )
    { return this_type(*this) |= p; }
};

class smoc_event_and_list
: public std::vector<smoc_event *> {
public:
  typedef smoc_event_and_list this_type;
  
  smoc_event_and_list( smoc_event &p )
    { push_back(&p); }
  this_type &operator &= ( smoc_event &p )
    { push_back(&p); return *this; }
  this_type operator & ( smoc_event &p )
    { return this_type(*this) &= p; }
};

inline
smoc_event_or_list  smoc_event::operator | ( smoc_event &p )
  { return smoc_event_or_list(*this) |= p; }

inline
smoc_event_and_list smoc_event::operator & ( smoc_event &p )
  { return smoc_event_and_list(*this) &= p; }

static inline
void wait( smoc_event &e ) {
  if ( !e ) {
    struct _: public smoc_event_listener {
      sc_event se;
      void signaled( smoc_event * )
        { se.notify(); }
    } w;
    e.addListener(&w);
    wait(w.se);
  }
}

#endif // _INCLUDED_SMOC_EVENT_HPP
