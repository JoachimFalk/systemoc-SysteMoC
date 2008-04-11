//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_SMOC_MD_FIFO_HPP
#define _INCLUDED_SMOC_MD_FIFO_HPP

#include <cosupport/commondefs.h>
#include <cosupport/smoc_debug_out.hpp>

#include <systemoc/smoc_config.h>

#include <iostream>

#ifndef NO_SMOC
#include "smoc_chan_if.hpp"
#endif
#include "smoc_storage.hpp"

//#include <systemc.h>
//#include <vector>
//#include <queue>
//#include <map>

#ifndef NO_SMOC
#include "hscd_tdsim_TraceLog.hpp"
#else
#include <string>
#endif

#include "smoc_md_loop.hpp"
#include "smoc_md_buffer.hpp"
#include "smoc_md_chan_if.hpp"
#include "smoc_md_port.hpp"
#include "smoc_wsdf_edge.hpp"


//#define ENABLE_SMOC_MD_BUFFER_ANALYSIS
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
// Buffer analysis classes
# include "smoc_md_buffer_analysis.hpp"
# include "smoc_md_ba_linearized_buffer_schedule.hpp"
#endif

/// 101: SysteMoC Interface
/// 102: Memory access error
/// 103: Parameter propagation
#ifndef VERBOSE_LEVEL_SMOC_MD_FIFO
#define VERBOSE_LEVEL_SMOC_MD_FIFO 0
#endif



template <typename T, template <typename> class STORAGE_OUT_TYPE>
class smoc_wsdf_edge;



/// Common base class for all multi-dimensional FIFOs
/// The name of the functions are chosen such, that there is a correspondance with
/// the 1D-Fifo
/// smoc_md_buffer_class provides the functionality of buffer management and must be
/// of type smoc_md_buffer_class or a derivative.
template <class BUFFER_CLASS>
class smoc_md_fifo_kind
#ifndef NO_SMOC
  : public smoc_nonconflicting_chan, public BUFFER_CLASS
#else
  : public BUFFER_CLASS
#endif
{

public:
  typedef smoc_md_fifo_kind  this_type;
  typedef size_t size_type;

  /// Specification of iteration domain
  typedef typename smoc_md_loop_iterator_kind::iter_domain_vector_type iter_domain_vector_type;  

  /// Make buffer_init visible
  typedef typename BUFFER_CLASS::buffer_init buffer_init;  

  /// Channel initializer
  /// used to setup a channel
  class chan_init {
    friend class smoc_md_fifo_kind;
  private:
    const char *name;
    const buffer_init b;
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
    //buffer analysis user inter
    smoc_md_ba::smoc_md_ba_user_interface* ba_ui;
#endif
    
  protected:
    chan_init(const char *name,
              const buffer_init& b
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
	      , smoc_md_ba::smoc_md_ba_user_interface* ba_ui
#endif
	      )
      : name(name),
        b(b)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
      , ba_ui(ba_ui)
#endif
    {}
  };


    
public:
  smoc_md_fifo_kind(const chan_init &i)
#ifndef NO_SMOC
    : smoc_nonconflicting_chan(i.name != NULL ? i.name : sc_gen_unique_name( "smoc_md_fifo" ) ),
      BUFFER_CLASS(i.b),
#else
      : BUFFER_CLASS(i.b),
        _name(i.name != NULL ? i.name : "smoc_md_fifo"),
#endif
      schedule_period_difference(0),
      _usedStorage(0),
      _usedStorageValid(false)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
      , buffer_analysis(i.ba_ui != NULL ? 
			i.ba_ui->create_buffer_analysis_object(this->src_loop_iterator,
							       this->snk_loop_iterator) :
			NULL)
#endif
  {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 102
    CoSupport::dout << "Enter smoc_md_fifo_kind::smoc_md_fifo_kind(const chan_init &i)" << std::endl;
    CoSupport::dout << "Leave smoc_md_fifo_kind::smoc_md_fifo_kind(const chan_init &i)" << std::endl;
#endif
  }

  virtual ~smoc_md_fifo_kind(){
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
    if (buffer_analysis != NULL)
      delete buffer_analysis;
#endif
  }

protected:

  /* *****************************************
     Functions treating the number of          
     available windows                         
     ***************************************** */
  /// This function verifies, whever a complete new window
  /// can be read. If yes, the function returns 1, otherwise zero.
  virtual size_t usedStorage() const;  

  /* *****************************************
     Functions treating the number of         
     free effective tokens                     
     ***************************************** */
  /// Check, whether a new effective token can be accepted.
  /// If yes, the function returns 1, otherwise zero.
  virtual size_t unusedStorage() const;


  /* *****************************************
     Read and write operations
     ***************************************** */
  /// Function is called, when the current window
  /// is not required any more
  /// The parameter n specifies the number of windows
  /// which have been consumed. Currently, only n = 1
  /// is supported.
  virtual void rpp(size_t n = 1);

  /// Function is called, when the source actor
  /// has produced the complete effective token
  /// The paramter n specifies the number of effective
  /// tokens which have been generated. Currently,
  /// only n = 1 is supported
#ifdef SYSTEMOC_ENABLE_VPC
  virtual void wpp(size_t n, const smoc_ref_event_p &le);
#else
  virtual void wpp(size_t n = 1);
#endif


  /* *****************************************
     Event Handling
     ***************************************** */

  /// Generate the events originated by a write operation
  void generate_write_events();

  /// Generate the events originated by a read operation
  void generate_read_events();  
  
  /// Returns an event, which is notified when n complete windows
  /// are available in the FIFO.
  /// If n == MAX_TYPE(size_t), then an event is returned
  /// which is notified whenever a write operation has taken place.
  /// Currently, only n = 1 or n = MAX_TYPE(size_t) is supported.
#ifndef NO_SMOC
  smoc_event& getEventAvailable(size_t n = 1);
#endif

  /// Returns an event which is notified when n complete effective
  /// tokens can be written into the FIFO.
  /// If n == MAX_TYPE(size_t), then an event is returned
  /// which is notified whenever a read operation has taken place.
  /// Currently, only n = 1 or n = MAX_TYPE(size_t) is supported.
#ifndef NO_SMOC
  smoc_event& getEventFree(size_t n = 1);  
#endif

  /* Functions for generation of problem graph */
#ifndef NO_SMOC
  void channelAttributes(smoc_modes::PGWriter &pgw) const {};
#endif

#ifndef NO_SMOC
  virtual void channelContents(smoc_modes::PGWriter &pgw) const {};
#endif

  const char *name() const { return smoc_nonconflicting_chan::name(); }

private:
  
  //disabled
  smoc_md_fifo_kind( const this_type&);
  this_type& operator = (const this_type &);
  
#ifdef NO_SMOC
private:
  std::string _name;
protected:
  std::string name() const {return _name;}
#endif

protected:
  /// The source and the sink iterator can be in different schedule
  /// periods. The next variable specifies the difference between
  /// the sink and the source schedule period:
  /// schedule_period_difference = src_period - snk_period;
  long schedule_period_difference;

#ifndef NO_SMOC
  /// Events
  smoc_event eventWrite; // write of effective token occured
  smoc_event eventRead;  // read of effective token occured
  
  smoc_event eventWindowAvailable; // There is a new window available
  smoc_event eventEffTokenFree;    // A new effective token can be written
#endif


private:
  //in order to increase simulation speed, we buffer the value
  //calculated by the function usedStorage.
  mutable size_t _usedStorage;
  mutable bool _usedStorageValid;

  //updates _usedStorage
  virtual void calcUsedStorage() const;

  //called, when one window is consumed
  virtual void decUsedStorage();
  
  //called, when one effective token is produced
  virtual void incUsedStorage();


private:
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  //Object for buffer analysis
  smoc_md_ba::smoc_md_buffer_analysis* buffer_analysis;
#endif

};


template <class BUFFER_CLASS>
size_t smoc_md_fifo_kind<BUFFER_CLASS>::usedStorage() const{
  if (!_usedStorageValid)
    calcUsedStorage();
  return _usedStorage;
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::decUsedStorage(){
  //currently, we only support consumption of one window
  _usedStorage = 0;

  //request new calculation
  _usedStorageValid = false;
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::incUsedStorage(){
  //currently, we only support consumption of one window
  if (_usedStorage < 1){
    //request new calculation
    _usedStorageValid = false;
  }
}


template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::calcUsedStorage() const{

#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
  CoSupport::dout << this->name() << ": ";
  CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::calcUsedStorage()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  // In this function we assume, that the data element
  // belonging to the maximum window iteration is produced
  // by the source at last!
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
  CoSupport::dout << "Next sink invocation ID: " << (*this).snk_loop_iterator.iteration_vector();
  CoSupport::dout << std::endl;
#endif
    

  // Get latest produced data element required by the
  // next sink actor invocation
  smoc_snk_md_loop_iterator_kind::data_element_id_type 
    src_data_element_id((*this).snk_loop_iterator.token_dimensions());
  if (!(*this).snk_loop_iterator.get_req_src_data_element(src_data_element_id)){
    /// Source does not need to produce anything for 
    /// this sink iteration
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
    CoSupport::dout << "Source actor does not need to produce anything" << std::endl;
#endif
    _usedStorage = 1;
  }else{    

#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
    CoSupport::dout << "Required data element: " << src_data_element_id;
    CoSupport::dout << std::endl;
#endif

  
    // Required source iteration for production of this data
    // element
    iter_domain_vector_type req_src_iteration((*this).src_loop_iterator.iterator_depth()); 
    smoc_src_md_loop_iterator_kind::id_type schedule_period_offset;
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 102
    CoSupport::dout << "(*this).src_loop_iterator.iterator_depth() = " << (*this).src_loop_iterator.iterator_depth() << std::endl;
#endif
    bool temp = 
      (*this).src_loop_iterator.get_src_loop_iteration(src_data_element_id,
						       req_src_iteration,
						       schedule_period_offset
						       );
    // error checking
    assert(temp);

#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
    CoSupport::dout << "Required src iteration: " << req_src_iteration;
    CoSupport::dout << " (schedule_period_offset  = " << schedule_period_offset << ")";
    CoSupport::dout << std::endl;
#endif

    // If schedule_period_offset == 0, then this means that the data element
    // required by the sink is generated by the same source schedule period
    // than the current sink schedule period.
    // If schedule_period_offset == -1, then this means that the data element
    // required by the sink is generated one source schedule period before
    // the current sink schedule period.
    //
    // Hence for successful firing:
    // schedule_period(source) > schedule_period(sink) + schedule_period_offset
    // or
    // (schedule_period(source) == schedule_period(sink) + schedule_period_offset
    //  and source_vector < required_source_vector

    if (schedule_period_difference <  schedule_period_offset){
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
      CoSupport::dout << "Sink is blocked due to difference of schedule periods." << std::endl;
#endif
      _usedStorage = 0;
    }else if (schedule_period_difference > schedule_period_offset){
      //Sink actor can fire
      _usedStorage = 1;
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
      CoSupport::dout << "Sink can fire due to schedule period difference" << std::endl;
      CoSupport::dout << CoSupport::Indent::Up;
      CoSupport::dout << "schedule_period_difference = " << schedule_period_difference << std::endl;
      CoSupport::dout << "schedule_period_offset = " << schedule_period_offset << std::endl;
      CoSupport::dout << CoSupport::Indent::Down;
#endif
    }else if (req_src_iteration.is_lex_smaller_than((*this).src_loop_iterator.iteration_vector())){
      //Sink actor can fire
      _usedStorage = 1;
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
      CoSupport::dout << "Sink can fire" << std::endl;
#endif
    }else{
      //Sink actor is blocked
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
      CoSupport::dout << "Sink is blocked" << std::endl;
#endif
      _usedStorage = 0;
    }  
  }

  _usedStorageValid = true;

#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
  CoSupport::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::calcUsedStorage()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif
    
}

template <class BUFFER_CLASS>
size_t smoc_md_fifo_kind<BUFFER_CLASS>::unusedStorage() const {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << this->name() << ": ";
  CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::unusedStorage()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
  CoSupport::dout << "src_loop_iterator = " << (*this).src_loop_iterator.iteration_vector();
  CoSupport::dout << std::endl;
#endif

  size_t return_value;

  if (BUFFER_CLASS::hasUnusedStorage()){
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << "Source can fire" << std::endl;
#endif
    return_value = 1;
  }else{
    return_value = 0;
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << "Source is blocked" << std::endl;
#endif
  }

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::unusedStorage()" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

  return return_value;
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::rpp(size_t n){
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << this->name() << ": ";
  CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::rpp" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif
  assert(n == 1);

  //free memory
  (*this).free_buffer();

  decUsedStorage();

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  if (buffer_analysis != NULL)
    buffer_analysis->consumption_update();
#endif
  
  // Move to next loop iteration
  if((*this).snk_loop_iterator.inc()){
    //new schedule period has been started
    schedule_period_difference--;
  }
  
  generate_read_events();

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::rpp" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif
  
}

#ifdef SYSTEMOC_ENABLE_VPC
template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::wpp(size_t n, const smoc_ref_event_p &le){
#else
template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::wpp(size_t n){
#endif

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << this->name() << ": ";
  CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::wpp" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  assert(n == 1);

  //be paranoiac
  //allocate memory, if not already done by user
  this->allocate_buffer();

  incUsedStorage();

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  if (buffer_analysis != NULL)
    buffer_analysis->production_update();
#endif

  // Move to next loop iteration
  if((*this).src_loop_iterator.inc()){
    //new schedule period has been started
    schedule_period_difference++;
  }
  
  /// Memory will not be written anymore
  (*this).release_buffer();
  
  generate_write_events();

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::wpp" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif

    
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::generate_write_events() {
  // check, if a new window has been generated
  if (usedStorage() != 0){
#ifndef NO_SMOC
    eventWindowAvailable.notify();
#endif
  }
  
  //Check, if a new effective token be accepted
  if (unusedStorage() != 0){
#ifndef NO_SMOC
    eventEffTokenFree.notify();
#endif
  }else{
#ifndef NO_SMOC
    eventEffTokenFree.reset();
#endif
  }

  //indicate write operation
#ifndef NO_SMOC
  eventWrite.notify();
#endif
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::generate_read_events() {
  // check, if there is still a window available
  if(usedStorage() != 0){
#ifndef NO_SMOC
    eventWindowAvailable.notify();
#endif
  }else{
#ifndef NO_SMOC
    eventWindowAvailable.reset();
#endif
  }
  
  //Check, if we can accept a new effective token
  if(unusedStorage() != 0){
#ifndef NO_SMOC
    eventEffTokenFree.notify();
#endif
  }
  
  //indicate read operation
#ifndef NO_SMOC
  eventRead.notify();
#endif
}

#ifndef NO_SMOC
template <class BUFFER_CLASS>
smoc_event& smoc_md_fifo_kind<BUFFER_CLASS>::getEventAvailable(size_t n) {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << this->name() << ": ";
  CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getEventAvailable" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif

  assert((n == 1) || (n == MAX_TYPE(size_t)));
  
  if (n == 1){
    if (usedStorage() >= n){
      eventWindowAvailable.notify();
    }
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return eventWindowAvailable;
  }else{
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return eventWrite;
  }

}
#endif

#ifndef NO_SMOC
template <class BUFFER_CLASS>
smoc_event& smoc_md_fifo_kind<BUFFER_CLASS>::getEventFree(size_t n) {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << (*this).name() << ": ";
  CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getEventFree" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif
  assert((n == 1) || (n == MAX_TYPE(size_t)));
  
  if (n == 1){
    if (unusedStorage() >= n){
      eventEffTokenFree.notify();
    }
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return eventEffTokenFree;
  }else{
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return eventRead;
  }
};
#endif








/*
template <typename T_chan_kind, typename T_data_type, 
          template <typename, typename> class R_IN,
          template <typename, typename> class R_OUT
          >
class smoc_dummy_chan_if{
public:
            smoc_dummy_chan_if(){};
};
*/



/// Provide the multi-dimensional FIFO with the SysteMoC Channel Interface
template <typename T_DATA_TYPE, 
          class BUFFER_CLASS, 
          template <typename> class STORAGE_OUT_TYPE
         >
class smoc_md_fifo_storage
#ifndef NO_SMOC
  : public smoc_chan_if</*smoc_md_fifo_kind<BUFFER_CLASS>,*/
                        T_DATA_TYPE,
                        smoc_md_snk_channel_access,
                        smoc_md_src_channel_access,
                        STORAGE_OUT_TYPE
                       >,
      public smoc_md_fifo_kind<BUFFER_CLASS> 
#else
    : public smoc_md_fifo_kind<BUFFER_CLASS>
    //: public smoc_dummy_chan_if<smoc_md_fifo_kind<BUFFER_CLASS>,
    //                            T_DATA_TYPE,
    //                            R_IN,
    //                            R_OUT >   
#endif
{
  
public:

  typedef T_DATA_TYPE                                              data_type;
/*#ifndef NO_SMOC
  typedef smoc_chan_if<smoc_md_fifo_kind<BUFFER_CLASS>,
                       T_DATA_TYPE,
                       smoc_md_snk_channel_access,
                       smoc_md_src_channel_access,
                       STORAGE_OUT_TYPE >  parent_type;
#else*/
  typedef smoc_md_fifo_kind<BUFFER_CLASS> parent_type;
//#endif
  typedef smoc_md_fifo_storage<data_type, 
                               BUFFER_CLASS, 
                               STORAGE_OUT_TYPE>       this_type;

  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  
  typedef typename STORAGE_OUT_TYPE<data_type>::storage_type  storage_out_type;
  typedef typename STORAGE_OUT_TYPE<data_type>::return_type   return_out_type;

#ifndef NO_SMOC
  typedef typename BUFFER_CLASS::template smoc_md_storage_access_src<storage_out_type,return_out_type>  
  ring_out_type;
  typedef typename BUFFER_CLASS::template smoc_md_storage_access_snk<storage_in_type,return_in_type>  
  ring_in_type;
#endif
  typedef smoc_storage<data_type>       storage_type;

  /// Make buffer_init visible
  typedef typename parent_type::buffer_init buffer_init;  

  typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;  

public:

  class chan_init
    : public parent_type::chan_init {
    friend class smoc_md_fifo_storage<T_DATA_TYPE, BUFFER_CLASS, STORAGE_OUT_TYPE>;
  protected:
    chan_init( const char *name, 
               const buffer_init &b 
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
	       , smoc_md_ba::smoc_md_ba_user_interface* ba_ui
#endif
	       )
      : parent_type::chan_init(name, 
                               b
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
			       , ba_ui
#endif
			       ) {}
  };

private:
  typename BUFFER_CLASS::template smoc_md_storage_type<storage_type>::type *storage;

protected:
  smoc_md_fifo_storage( const chan_init &i)
    : parent_type(i)
  {
    createStorage(storage);
  }

  ~smoc_md_fifo_storage()
  {
    destroyStorage(storage);
  }

  const char *name() const
  { return parent_type::name(); }

#ifndef NO_SMOC
  ring_in_type * getReadChannelAccess() {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << this->name() << ": ";
    CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getReadChannelAccess" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif
    ring_in_type *r = new ring_in_type();
    initStorageAccess(*r);
    r->SetBuffer(storage);
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::getReadChannelAccess" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return r;
  }
#endif

#ifndef NO_SMOC
  ring_out_type * getWriteChannelAccess() {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << this->name() << ": ";
  CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getWriteChannelAccess" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif
  ring_out_type *r = new ring_out_type();
    initStorageAccess(*r);
    r->SetBuffer(storage);
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::getWriteChannelAccess" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif
  return r;
  }

  void channelContents(smoc_modes::PGWriter &pgw) const {};  
#endif

};



template <class BUFFER_CLASS, 
          template <typename> class STORAGE_OUT_TYPE>
class smoc_md_fifo_storage<void,BUFFER_CLASS,STORAGE_OUT_TYPE>
#ifndef NO_SMOC
  : public smoc_chan_if</*smoc_md_fifo_kind<BUFFER_CLASS>,*/
                        void,
                        smoc_md_snk_channel_access,
                        smoc_md_src_channel_access,
                        STORAGE_OUT_TYPE
                       >,
      public smoc_md_fifo_kind<BUFFER_CLASS>
#else
    : public smoc_md_fifo_kind<BUFFER_CLASS>
    //: public smoc_dummy_chan_if<smoc_md_fifo_kind<BUFFER_CLASS>,
    //                            T_DATA_TYPE,
    //                            R_IN,
    //                            R_OUT >   
#endif
{
  
public:

  typedef void data_type;
/*#ifndef NO_SMOC
  typedef smoc_chan_if<smoc_md_fifo_kind<BUFFER_CLASS>,
                       void,
                       smoc_md_snk_channel_access,
                       smoc_md_src_channel_access,
                       STORAGE_OUT_TYPE >  parent_type;
#else*/
  typedef smoc_md_fifo_kind<BUFFER_CLASS> parent_type;
//#endif
  typedef smoc_md_fifo_storage<data_type, 
                               void, 
                               STORAGE_OUT_TYPE>       this_type;

  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  
  typedef typename STORAGE_OUT_TYPE<data_type>::storage_type  storage_out_type;
  typedef typename STORAGE_OUT_TYPE<data_type>::return_type   return_out_type;

#ifndef NO_SMOC
  typedef typename BUFFER_CLASS::template smoc_md_storage_access_src<storage_out_type,return_out_type>  
  ring_out_type;
  typedef typename BUFFER_CLASS::template smoc_md_storage_access_snk<storage_in_type,return_in_type>  
  ring_in_type;
#endif
  typedef smoc_storage<data_type>       storage_type;

  /// Make buffer_init visible
  typedef typename parent_type::buffer_init buffer_init;  

  typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;  

public:

  class chan_init
    : public parent_type::chan_init {
    friend class smoc_md_fifo_storage<void, BUFFER_CLASS, STORAGE_OUT_TYPE>;
  protected:
    chan_init( const char *name, 
               const buffer_init &b 
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
	      , smoc_md_ba::smoc_md_ba_user_interface* ba_ui
#endif
	       )
      : parent_type::chan_init(name, 
                               b
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
			       , ba_ui
#endif
			       ) {}
  };

private:

protected:
  smoc_md_fifo_storage( const chan_init &i)
    : parent_type(i)
  { }
  
  const char *name() const
  { return parent_type::name(); }

#ifndef NO_SMOC
  ring_in_type * getReadChannelAccess() {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << this->name() << ": ";
    CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getReadChannelAccess" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif
    ring_in_type *r = new ring_in_type();
    initStorageAccess(*r);
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::getReadChannelAccess" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return r;
  }
#endif

#ifndef NO_SMOC
  ring_out_type * getWriteChannelAccess() {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << this->name() << ": ";
  CoSupport::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getWriteChannelAccess" << std::endl;
  CoSupport::dout << CoSupport::Indent::Up;
#endif
  ring_out_type *r = new ring_out_type();
  initStorageAccess(*r);
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::getWriteChannelAccess" << std::endl;
  CoSupport::dout << CoSupport::Indent::Down;
#endif
  return r;
  }

  void channelContents(smoc_modes::PGWriter &pgw) const {};  
#endif

  ~smoc_md_fifo_storage() { 
  }

};


template <typename T_DATA_TYPE,
          template <typename> class STORAGE_OUT_TYPE = smoc_storage_out
         >
class smoc_md_fifo_type
  : public smoc_md_fifo_storage<T_DATA_TYPE, 
                                smoc_simple_md_buffer_kind, 
                                STORAGE_OUT_TYPE> {
public:
  typedef T_DATA_TYPE                  data_type;
  typedef smoc_md_fifo_storage<T_DATA_TYPE, 
                               smoc_simple_md_buffer_kind,
                               STORAGE_OUT_TYPE
                               > parent_type;
  typedef smoc_md_fifo_type<T_DATA_TYPE, STORAGE_OUT_TYPE> this_type;
  
  typedef typename parent_type::storage_in_type storage_in_type;
  typedef typename parent_type::return_in_type return_in_type;
  
  typedef typename parent_type::storage_out_type storage_out_type;
  typedef typename parent_type::return_out_type return_out_type;

  //Make channel init visible
  typedef typename parent_type::chan_init chan_init;
  typedef typename parent_type::buffer_init buffer_init;

#ifdef NO_SMOC
public:
#else
public:
  //protected:
#endif
  
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t consume, const smoc_ref_event_p &le)
#else
  void commitRead(size_t consume)
#endif
  {
#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::dout << this->name() << ": Enter commitRead" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
    CoSupport::dout << "Iteration : " << (*this).snk_loop_iterator.iteration_vector();
    CoSupport::dout << std::endl;
    CoSupport::dout << "Consume " << consume << " windows" << std::endl;
#endif

    //Currently, we only support the consumption zero or one window.
    assert(consume <= 1);
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(this, consume);
#endif
    if (consume >= 1)
      this->rpp(consume);

#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::dout << "Leave commitRead" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif

  }
  
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t produce, const smoc_ref_event_p &le)
#else
  void commitWrite(size_t produce)
#endif
  {

#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::dout << this->name() << ": Enter commitWrite" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
    CoSupport::dout << "Iteration : " << (*this).src_loop_iterator.iteration_vector();
    CoSupport::dout << std::endl;
    CoSupport::dout << "Write " << produce << " effective tokens" << std::endl;
#endif


    //Currently, we only support the consumption of zero or one
    //effective token
    assert(produce <= 1);
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(this, produce);
#endif
    if (produce >= 1){
#ifdef SYSTEMOC_ENABLE_VPC
      this->wpp(produce, le);
#else
      this->wpp(produce);
#endif
    }

#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::dout << "Leave commitWrite" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif

  }
public:
  // constructors
  smoc_md_fifo_type( const chan_init &i )
    : parent_type(i) {
  }

  template<unsigned N,template<class> class S, class Init>
  void connect(smoc_md_port_out<data_type,N,S>& port, const Init& i) {
    port(*this);
    port.setFiringLevelMap(
        i.wsdf_edge_param.calc_src_iteration_level_table());
  }

  template<unsigned N,template<class> class S, class Init>
  void connect(smoc_md_iport_out<data_type,N,S>& port, const Init& i) {
    port(*this);
    port.setFiringLevelMap(
        i.wsdf_edge_param.calc_src_iteration_level_table());
  }

  template<unsigned N,template<class,class> class B, class Init>
  void connect(smoc_md_port_in<data_type,N,B>& port, const Init& i) {
    port(*this);
    port.setFiringLevelMap(
        i.wsdf_edge_param.calc_snk_iteration_level_table());
  }

  template<unsigned N, class Init>
  void connect(smoc_md_iport_in<data_type,N>& port, const Init& i) {
    port(*this);
    port.setFiringLevelMap(
        i.wsdf_edge_param.calc_snk_iteration_level_table());
  }

  // bounce functions
  size_t numAvailable() const { 
#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::dout << this->name() << ": Enter numAvailable()" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif
    size_t return_value = this->usedStorage();
#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::dout << "Fifo contains at least " << return_value << " windows" << std::endl;
    CoSupport::dout << "Leave numAvailable()" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return return_value;
  }
  size_t numFree() const { 
#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::dout << this->name() << ": Enter numFree()" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif
    size_t return_value = this->unusedStorage(); 

#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::dout << "Fifo accepts at least " << return_value << " effective tokens" << std::endl;
    CoSupport::dout << "Leave numFree()" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif
    return return_value;
  }
#ifndef NO_SMOC
  smoc_event &dataAvailableEvent(size_t n)
    { return this->getEventAvailable(n); }
  smoc_event &spaceAvailableEvent(size_t n)
    { return this->getEventFree(n); }
  size_t inTokenId() const
    { assert(!"FIXME: Not implemented !!!"); return 0; }
  size_t outTokenId() const
    { assert(!"FIXME: Not implemented !!!"); return 0; }
#endif
};

/// Channel initialization class
template <typename T, 
          template <typename> class STORAGE_OUT_TYPE = smoc_storage_out>
class smoc_md_fifo
  : public smoc_md_fifo_type<T, STORAGE_OUT_TYPE>::chan_init {
public:
  typedef T                   data_type;
  typedef smoc_md_fifo<T>        this_type;

  //Identification of corresponding channel class
  typedef smoc_md_fifo_type<T, STORAGE_OUT_TYPE>   chan_type;

  //Make buffer_init visible
  typedef typename smoc_md_fifo_type<T, STORAGE_OUT_TYPE>::buffer_init buffer_init;

public:
  
  smoc_md_fifo( const smoc_wsdf_edge_descr& wsdf_edge_param, 
                size_t n
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
		, smoc_md_ba::smoc_md_ba_user_interface* ba_ui = NULL
#endif
		)
    : smoc_md_fifo_type<T,STORAGE_OUT_TYPE>::chan_init(NULL,
                                                       assemble_buffer_init(wsdf_edge_param, n)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
						       , ba_ui
#endif
                                                       ),
    wsdf_edge_param(wsdf_edge_param)
  {}

  explicit smoc_md_fifo( const char *name, 
                         const smoc_wsdf_edge_descr& wsdf_edge_param, 
                         size_t n)
    : smoc_md_fifo_type<T, STORAGE_OUT_TYPE>::chan_init(name,
                                                        assemble_buffer_init(wsdf_edge_param, n)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
							, NULL
#endif
							
                                                        ),
    wsdf_edge_param(wsdf_edge_param)

  {}

  smoc_md_fifo(const smoc_wsdf_edge<T,STORAGE_OUT_TYPE>& edge_param,
               const smoc_wsdf_src_param& src_param,
               const smoc_wsdf_snk_param& snk_param)
    : smoc_md_fifo_type<T, STORAGE_OUT_TYPE>::chan_init(
                                                        edge_param.name,
                                                        assemble_buffer_init(assemble_wsdf_edge(edge_param, src_param, snk_param), edge_param.n)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
							, NULL
#endif
                                                        ),
    wsdf_edge_param(assemble_wsdf_edge(edge_param, src_param, snk_param))
  {
  }
  

  const smoc_wsdf_edge_descr wsdf_edge_param;

private:
  



  smoc_wsdf_edge_descr assemble_wsdf_edge(const smoc_wsdf_edge<T, STORAGE_OUT_TYPE>& edge_param,
                                          const smoc_wsdf_src_param& src_param,
                                          const smoc_wsdf_snk_param& snk_param) const {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 103
    CoSupport::dout << "Enter smoc_md_fifo::assemble_wsdf_edge" << std::endl;
    CoSupport::dout << CoSupport::Indent::Up;
#endif

    typedef smoc_wsdf_edge_descr::udata_type     udata_type;
    typedef smoc_wsdf_edge_descr::uvector_type   uvector_type;

    unsigned token_dimensions = src_param.src_firing_blocks[0].size();

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 103
    CoSupport::dout << "src_param.src_firing_blocks = " << src_param.src_firing_blocks << std::endl;
    CoSupport::dout << "token_dimensions = " << token_dimensions << std::endl;
#endif
    
    
    uvector_type d;

    if (edge_param.d_valid)
      d = edge_param.d;
    else
      d = uvector_type(token_dimensions,(udata_type)0);

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 103
    CoSupport::dout << "d = " << d;
    CoSupport::dout << std::endl;

    CoSupport::dout << "src_firing_blocks = " << src_param.src_firing_blocks;
    CoSupport::dout << std::endl;

    CoSupport::dout << "snk_firing_blocks = " << snk_param.snk_firing_blocks;
    CoSupport::dout << std::endl;

    CoSupport::dout << "u0 = " << snk_param.u0;
    CoSupport::dout << std::endl;

    CoSupport::dout << "c = " << snk_param.c;
    CoSupport::dout << std::endl;

    CoSupport::dout << "delta_c = " << snk_param.delta_c;
    CoSupport::dout << std::endl;

    CoSupport::dout << "bs = " << snk_param.bs;
    CoSupport::dout << std::endl;

    CoSupport::dout << "bt = " << snk_param.bt;
    CoSupport::dout << std::endl;
#endif


    const smoc_wsdf_edge_descr 
      wsdf_edge_param(token_dimensions,
                      src_param.src_firing_blocks,
                      snk_param.snk_firing_blocks,
                      snk_param.u0,
                      snk_param.c,
                      snk_param.delta_c,
                      d,
                      snk_param.bs,
                      snk_param.bt);

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 103
    CoSupport::dout << "Leave smoc_md_fifo::assemble_wsdf_edge" << std::endl;
    CoSupport::dout << CoSupport::Indent::Down;
#endif

    return wsdf_edge_param;
  }
  

  buffer_init assemble_buffer_init(const smoc_wsdf_edge_descr& wsdf_edge_param, 
                                   size_t n) const{
    buffer_init return_value(wsdf_edge_param.src_iteration_max(),
                             wsdf_edge_param.src_data_element_mapping_matrix(),
                             wsdf_edge_param.src_data_element_mapping_vector(),
                             
                             wsdf_edge_param.snk_iteration_max(),
                             wsdf_edge_param.snk_data_element_mapping_matrix(),
                             wsdf_edge_param.snk_data_element_mapping_vector(),

                             wsdf_edge_param.calc_border_condition_matrix(),
                             wsdf_edge_param.calc_low_border_condition_vector(),
                             wsdf_edge_param.calc_high_border_condition_vector(),
                             
                             n);

    return return_value;
  }
};





/// If we want to annotate the parameters at the ports instead of
/// associating them all with the FIFO, we can use this class
template <typename T, 
          template <typename> class STORAGE_OUT_TYPE = smoc_storage_out >
class smoc_wsdf_edge{
public:

  typedef T                   data_type;

  // associated channel init
  typedef smoc_md_fifo<T, STORAGE_OUT_TYPE> chan_init_type;

  //Make buffer_init visible
  typedef typename chan_init_type::buffer_init buffer_init;  

  typedef smoc_wsdf_edge_descr::sdata_type     sdata_type;
  typedef smoc_wsdf_edge_descr::udata_type     udata_type;
  typedef smoc_wsdf_edge_descr::svector_type   svector_type;
  typedef smoc_wsdf_edge_descr::uvector_type   uvector_type;
  typedef smoc_wsdf_edge_descr::u2vector_type  u2vector_type;
  typedef smoc_wsdf_edge_descr::smatrix_type   smatrix_type;
  typedef smoc_wsdf_edge_descr::umatrix_type   umatrix_type;  

public:
  smoc_wsdf_edge(size_t n,
                 const uvector_type& d
                 )
    : name(NULL),
      n(n), 
      d(d), 
      d_valid(true)
  {}

  smoc_wsdf_edge(size_t n)
    : name(NULL),
      n(n), 
      d_valid(false)
  {}


  explicit smoc_wsdf_edge(const char *name, 
                          size_t n,
                          const uvector_type& d
                          )
    : name(name),
      n(n), 
      d(d), 
      d_valid(true)
  {}

  explicit smoc_wsdf_edge(const char *name, 
                          size_t n)
    : name(name),
      n(n), 
      d_valid(false)
  {}

public:
  const char* name;
  const size_t n;

  const uvector_type d;
  const bool d_valid;
};


#endif // _INCLUDED_SMOC_FIFO_HPP
