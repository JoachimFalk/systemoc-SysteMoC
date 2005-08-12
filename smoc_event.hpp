
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
  void reset()
  {notified=0;}
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
: public std::vector<smoc_event *>,
  public smoc_event {
public:
  typedef smoc_event_or_list this_type;
  
  smoc_event_or_list( smoc_event &p )
    { *this |= p; }
  this_type operator | ( smoc_event &p )
    { return this_type(*this) |= p; }
  this_type &operator |= ( smoc_event &p )
    { push_back(&p); return *this; }
};

class smoc_event_and_list
: public std::vector<smoc_event *>,
  public smoc_event,
  protected smoc_event_listener {
private:
  size_t missing;
public:
  typedef smoc_event_and_list this_type;
  
  smoc_event_and_list( smoc_event &p ): missing(0)
    { *this &= p; }
  this_type operator & ( smoc_event &p )
    { return this_type(*this) &= p; }
  this_type &operator &= ( smoc_event &p ) {
    if ( !p ) {
      p.addListener(this); 
      ++missing;
    }
    push_back(&p);
    return *this;
  }
  void signaled( smoc_event *e ) {
    assert( missing > 0 );
    if ( !--missing )
      notify();
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

