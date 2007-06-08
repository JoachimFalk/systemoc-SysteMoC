// ============================================================================
//
//     Project: OSCI TLM WG, PVT Examples
//
//     Authors: Tim Kogel, CoWare Inc.
//		Trevor Wieman, Intel Corp.
//
// Description: annotated tlm fifo with modified get policy
//
// ============================================================================

#ifndef TLM_PVT_ANNOTATED_FIFO_HEADER
#define TLM_PVT_ANNOTATED_FIFO_HEADER

#include "tlm.h"
#include "systemc.h"

//
// This modified version of the tlm_annotated_fifo in the tlm-kit 
// has a slightly modified get policy. The notification of the 
// ok_to_get event is suppressed for the duration of the timing 
// annotation from consumer side. This way the consumer does not need 
// to keep track of the busy-cycles.
// 

template< typename T >
class tlm_pvt_annotated_fifo :
  public tlm::tlm_annotated_fifo<T> 
{
  // these 'using's are because gcc 3.4 & 4.0 fail to find the names in the correct places
  using tlm::tlm_annotated_fifo<T>::sensitive;
  using tlm::tlm_annotated_fifo<T>::sensitive_pos;
  using tlm::tlm_annotated_fifo<T>::sensitive_neg;
  using tlm::tlm_annotated_fifo<T>::dont_initialize;
  using tlm::tlm_annotated_fifo<T>::name;

  public:

  SC_HAS_PROCESS(tlm_pvt_annotated_fifo);

  tlm_pvt_annotated_fifo( sc_module_name nm , int size = 1 ) :
     tlm::tlm_annotated_fifo<T>( nm ,size ) ,
     m_busy_until(SC_ZERO_TIME)
  {
    SC_METHOD(ok_to_get_method);
    sensitive << tlm::tlm_annotated_fifo<T>::ok_to_get();
    dont_initialize();
  }

   tlm_pvt_annotated_fifo( int size = 1 ) :
     tlm::tlm_annotated_fifo<T>( size ) ,
     m_busy_until(SC_ZERO_TIME)
  {
    SC_METHOD(ok_to_get_method);
    sensitive << tlm::tlm_annotated_fifo<T>::ok_to_get();
    dont_initialize();
  }
  
  ~tlm_pvt_annotated_fifo() 
  {
  }
  
  const sc_event &ok_to_get(  tlm::tlm_tag<T> *t = 0 ) const {
    return m_guarded_ok_to_get_event;
  }
  const sc_event &ok_to_peek(  tlm::tlm_tag<T> *t = 0 ) const {
    return m_guarded_ok_to_get_event;
  }

  virtual bool nb_can_get( const sc_time &time , tlm::tlm_tag<T> *t = 0 ) const {
    sc_time now = sc_time_stamp();
    if (m_busy_until > now) {
      cerr << now << ", I am actually busy until " << m_busy_until 
	   << ", try again then\n";
      return false;
    }
    return tlm::tlm_annotated_fifo<T>::nb_can_get();
  }

  virtual bool nb_get( T &transaction , const sc_time &time ) {
    sc_time now = sc_time_stamp();
    if (m_busy_until > now) {
      cerr << now << ", I am actually busy until " << m_busy_until 
	   << ", try again then\n";
      return false;
    }
    m_busy_until = now + time;
    return  tlm::tlm_annotated_fifo<T>::nb_get(transaction,time);
  }
 

 protected: 

  void ok_to_get_method( )
  {
    sc_time now = sc_time_stamp();
    if (m_busy_until > now) {
#if (defined DEBUG_TLM_PVT_ANNOTATED_FIFO || defined DEBUG_ALL) 
      cout << now << " tlm_pvt_annotated_fifo (" << name() 
	   << "), ok_to_get_method, the consumer will be busy until " 
	   << m_busy_until << ", so I'll try then\n";
#endif
      m_guarded_ok_to_get_event.cancel();
      m_guarded_ok_to_get_event.notify(m_busy_until - now);
    } else {
      m_guarded_ok_to_get_event.cancel();
      m_guarded_ok_to_get_event.notify();
    }
  }

  sc_time m_busy_until;
  sc_event m_guarded_ok_to_get_event;
};
#endif
