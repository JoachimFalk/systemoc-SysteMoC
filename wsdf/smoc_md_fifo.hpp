//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef _INCLUDED_SMOC_MD_FIFO_HPP
#define _INCLUDED_SMOC_MD_FIFO_HPP

#include <CoSupport/commondefs.h>
#include <CoSupport/Streams/DebugOStream.hpp>

#include <systemoc/smoc_config.h>

#include <iostream>
#include <sstream>

#include "smoc_chan_if.hpp"
#include "smoc_fifo.hpp"
#include "smoc_storage.hpp"

//#include <systemc.h>
//#include <vector>
//#include <queue>
//#include <map>

#include "hscd_tdsim_TraceLog.hpp"

#include "smoc_md_loop.hpp"
#include "smoc_md_buffer.hpp"
#include "smoc_md_chan_if.hpp"
#include "smoc_md_port.hpp"
#include <wsdf/smoc_wsdf_edge.hpp>

//#define ENABLE_SMOC_MD_BUFFER_ANALYSIS
//#define SYSTEMOC_TRACE_BUFFER_SIZE

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
// Buffer analysis classes
# include "smoc_md_buffer_analysis.hpp"
# include "smoc_md_ba_linearized_buffer_size.hpp"
//# include "smoc_md_ba_linearized_buffer_schedule.hpp"
#endif

/// 101: SysteMoC Interface
/// 102: Memory access error
/// 103: Parameter propagation
#ifndef VERBOSE_LEVEL_SMOC_MD_FIFO
#define VERBOSE_LEVEL_SMOC_MD_FIFO 0
#endif



template <typename T, template <typename> class STORAGE_OUT_TYPE>
class smoc_wsdf_edge;
//template <class BUFFER_CLASS>
//class smoc_md_hw_fifo_kind;



/// Common base class for all multi-dimensional FIFOs
/// The name of the functions are chosen such, that there is a correspondance with
/// the 1D-Fifo
/// smoc_md_buffer_class provides the functionality of buffer management and must be
/// of type smoc_md_buffer_class or a derivative.
template <class BUFFER_CLASS>
class smoc_md_fifo_kind
  : public smoc_nonconflicting_chan, 
    public BUFFER_CLASS
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
    std::string name;
    const buffer_init b;
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
    //buffer analysis user inter
    smoc_md_ba::smoc_md_ba_user_interface* ba_ui;
#endif
    
  protected:
    chan_init(const std::string& name,
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
  smoc_md_fifo_kind(const chan_init &i);
  virtual ~smoc_md_fifo_kind();

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


#ifdef SYSTEMOC_ENABLE_VPC
  /// This function will be called if a new source invocation
  /// shall become visible to the sink
  void latencyExpired(size_t n = 1);
#endif


  /* *****************************************
     Event Handling
     ***************************************** */

  /// Generate the events originated by a write operation
  void generate_write_events();
  void generate_write2Src_events();
  void generate_write2Snk_events();

  /// Generate the events originated by a read operation
  void generate_read_events();  
  void generate_read2Snk_events();
  void generate_read2Src_events();
  
  /// Returns an event, which is notified when n complete windows
  /// are available in the FIFO.
  /// If n == MAX_TYPE(size_t), then an event is returned
  /// which is notified whenever a write operation has taken place.
  /// Currently, only n = 1 or n = MAX_TYPE(size_t) is supported.
  smoc_event& getEventAvailable(size_t n = 1);

  /// Returns an event which is notified when n complete effective
  /// tokens can be written into the FIFO.
  /// If n == MAX_TYPE(size_t), then an event is returned
  /// which is notified whenever a read operation has taken place.
  /// Currently, only n = 1 or n = MAX_TYPE(size_t) is supported.
  smoc_event& getEventFree(size_t n = 1);  

  /* Functions for generation of problem graph */

  /// This function returns a string indentifying the channel type
  virtual const char* getChannelTypeString() const;

  void channelAttributes(smoc_modes::PGWriter &pgw) const;

  const char *name() const { return smoc_nonconflicting_chan::name(); }

private:
  
  //disabled
  smoc_md_fifo_kind( const this_type&);
  this_type& operator = (const this_type &);
  
protected:
  /// The source and the sink iterator can be in different schedule
  /// periods. The next variable specifies the difference between
  /// the sink and the source schedule period:
  /// visible_schedule_period_difference = src_period - snk_period;
  long visible_schedule_period_difference;

  /// Events
  smoc_event eventWrite; // write of effective token occured
  smoc_event eventRead;  // read of effective token occured
  
  smoc_event eventWindowAvailable; // There is a new window available
  smoc_event eventEffTokenFree;    // A new effective token can be written

#ifdef SYSTEMOC_ENABLE_VPC
  Detail::LatencyQueue latencyQueue;

  /// If SystemVPC is enabled we have to distinguish between the current
  /// source invocation (given by src_loop_iterator)
  /// and the source invocation which is visible by the sink
  smoc_md_static_loop_iterator src_iterator_visible;

#endif


private:
  //in order to increase simulation speed, we buffer the value
  //calculated by the function usedStorage.
  mutable size_t _usedStorage;
  mutable bool _usedStorageValid;

  //updates _usedStorage
  //virtual void calcUsedStorage() const;
  //DO NOT make virtual
  void calcUsedStorage() const;

  //called, when one window is consumed
  //DO NOT make virtual
  void decUsedStorage();

protected:
  
  //called, when one effective token is produced
  virtual void incUsedStorage();


private:
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  //Object for buffer analysis
  smoc_md_ba::smoc_md_buffer_analysis* buffer_analysis;

# ifdef SYSTEMOC_ENABLE_VPC
#  ifdef SYSTEMOC_TRACE_BUFFER_SIZE
  sc_trace_file *buffer_size_trace_file;

  /// The following function is called by the SystemC-Kernel.
  /// We use it to generate the buffer size trace file as only
  /// here the correct module name is available.
  virtual void start_of_simulation();
#  endif
# endif

#endif

};


template <class BUFFER_CLASS>
smoc_md_fifo_kind<BUFFER_CLASS>::smoc_md_fifo_kind(const chan_init &i)
  : smoc_nonconflicting_chan(i.name),
    BUFFER_CLASS(i.b),
    visible_schedule_period_difference(0),
#ifdef SYSTEMOC_ENABLE_VPC
    latencyQueue(std::bind1st(std::mem_fun(&this_type::latencyExpired), this), this),
    src_iterator_visible(this->src_loop_iterator.iteration_max(),
                         this->src_loop_iterator.token_dimensions()),
#endif
    _usedStorage(0),
    _usedStorageValid(false)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  , buffer_analysis(i.ba_ui != NULL ? 
                    i.ba_ui->create_buffer_analysis_object(this->src_loop_iterator,
							       this->snk_loop_iterator) :
                    smoc_modes::dumpSMXWithSim ?
                    //use buffer analysis by default in order
                    //to get size of FIFO
                    new smoc_md_ba::smoc_mb_ba_lin_buffer_size(this->src_loop_iterator,
                                                               this->snk_loop_iterator,
                                                               this->src_loop_iterator.iteration_max()[0]
                                                               ) :
                    NULL)
# ifdef SYSTEMOC_ENABLE_VPC
#  ifdef SYSTEMOC_TRACE_BUFFER_SIZE
  , buffer_size_trace_file(NULL)
#  endif
# endif
#endif
{
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 102
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind::smoc_md_fifo_kind(const chan_init &i)" << std::endl;
#endif
    
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 102
  CoSupport::Streams::dout << "Leave smoc_md_fifo_kind::smoc_md_fifo_kind(const chan_init &i)" << std::endl;
#endif

}

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
# ifdef SYSTEMOC_ENABLE_VPC
#  ifdef SYSTEMOC_TRACE_BUFFER_SIZE

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::start_of_simulation(){
  const smoc_md_ba::smoc_mb_ba_lin_buffer_size* temp =
    dynamic_cast<const smoc_md_ba::smoc_mb_ba_lin_buffer_size*>(buffer_analysis);
  if (temp != NULL){
    std::stringstream filename;
    filename << "bfs_" << this->name();
    buffer_size_trace_file = 
      sc_create_vcd_trace_file(filename.str().c_str());
    assert(buffer_size_trace_file != NULL);
    static_cast<vcd_trace_file*>(buffer_size_trace_file)->sc_set_vcd_time_unit(-9);
    
    sc_trace(buffer_size_trace_file,
             temp->get_buffer_size(),
             filename.str());

    //Trace iterators
    sc_trace(buffer_size_trace_file,
             this->src_loop_iterator.iteration_vector(),
             filename.str() + "_src");
    sc_trace(buffer_size_trace_file,
             this->snk_loop_iterator.iteration_vector(),
             filename.str() + "_snk");
    
  }  
}

#  endif
# endif
#endif



template <class BUFFER_CLASS>
smoc_md_fifo_kind<BUFFER_CLASS>::~smoc_md_fifo_kind(){
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  if (buffer_analysis != NULL)
    delete buffer_analysis;
  
# ifdef SYSTEMOC_ENABLE_VPC
#  ifdef SYSTEMOC_TRACE_BUFFER_SIZE
  if (buffer_size_trace_file != NULL)
    sc_close_vcd_trace_file(buffer_size_trace_file);
#  endif
# endif
#endif
}


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
  CoSupport::Streams::dout << this->name() << ": ";
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::calcUsedStorage()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif

  // In this function we assume, that the data element
  // belonging to the maximum window iteration is produced
  // by the source at last!
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
  CoSupport::Streams::dout << "Next sink invocation ID: " << (*this).snk_loop_iterator.iteration_vector();
  CoSupport::Streams::dout << std::endl;
#endif
    

  // Get latest produced data element required by the
  // next sink actor invocation
  smoc_snk_md_loop_iterator_kind::data_element_id_type 
    src_data_element_id((*this).snk_loop_iterator.token_dimensions());
  if (!(*this).snk_loop_iterator.get_req_src_data_element(src_data_element_id)){
    /// Source does not need to produce anything for 
    /// this sink iteration
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
    CoSupport::Streams::dout << "Source actor does not need to produce anything" << std::endl;
#endif
    _usedStorage = 1;
  }else{    

#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
    CoSupport::Streams::dout << "Required data element: " << src_data_element_id;
    CoSupport::Streams::dout << std::endl;
#endif

  
    // Required source iteration for production of this data
    // element
    iter_domain_vector_type 
      req_src_iteration((*this).src_loop_iterator.iterator_depth()); 
    smoc_src_md_loop_iterator_kind::id_type schedule_period_offset;
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 102
    CoSupport::Streams::dout << "(*this).src_loop_iterator.iterator_depth() = " << (*this).src_loop_iterator.iterator_depth() << std::endl;
#endif
    bool temp = 
      (*this).src_loop_iterator.get_src_loop_iteration(src_data_element_id,
						       req_src_iteration,
						       schedule_period_offset
						       );
    // error checking
    assert(temp);

#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
    CoSupport::Streams::dout << "Required src iteration: " << req_src_iteration;
    CoSupport::Streams::dout << " (schedule_period_offset  = " << schedule_period_offset << ")";
    CoSupport::Streams::dout << std::endl;
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

    if (visible_schedule_period_difference <  schedule_period_offset){
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
      CoSupport::Streams::dout << "Sink is blocked due to difference of schedule periods." << std::endl;
#endif
      _usedStorage = 0;
    }else if (visible_schedule_period_difference > schedule_period_offset){
      //Sink actor can fire
      _usedStorage = 1;
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
      CoSupport::Streams::dout << "Sink can fire due to schedule period difference" << std::endl;
      CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
      CoSupport::Streams::dout << "visible_schedule_period_difference = " << visible_schedule_period_difference << std::endl;
      CoSupport::Streams::dout << "schedule_period_offset = " << schedule_period_offset << std::endl;
      CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
#ifdef SYSTEMOC_ENABLE_VPC
    }else if (req_src_iteration.is_lex_smaller_than((*this).src_iterator_visible.iteration_vector())){
#else
    }else if (req_src_iteration.is_lex_smaller_than((*this).src_loop_iterator.iteration_vector())){
#endif
      //Sink actor can fire
      _usedStorage = 1;
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
      CoSupport::Streams::dout << "Sink can fire" << std::endl;
#endif
    }else{
      //Sink actor is blocked
#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
      CoSupport::Streams::dout << "Sink is blocked" << std::endl;
#endif
      _usedStorage = 0;
    }  
  }

  _usedStorageValid = true;

#if (VERBOSE_LEVEL_SMOC_MD_FIFO == 101) || (VERBOSE_LEVEL_SMOC_MD_FIFO == 102)
  CoSupport::Streams::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::calcUsedStorage()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    
}

template <class BUFFER_CLASS>
size_t smoc_md_fifo_kind<BUFFER_CLASS>::unusedStorage() const {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << this->name() << ": ";
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::unusedStorage()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
  CoSupport::Streams::dout << "src_loop_iterator = " << (*this).src_loop_iterator.iteration_vector();
  CoSupport::Streams::dout << std::endl;
#endif

  size_t return_value;

  if (BUFFER_CLASS::hasUnusedStorage()){
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << "Source can fire" << std::endl;
#endif
    return_value = 1;
  }else{
    return_value = 0;
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << "Source is blocked" << std::endl;
#endif
  }

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::unusedStorage()" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif

  return return_value;
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::rpp(size_t n){
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << this->name() << ": ";
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::rpp" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
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
    visible_schedule_period_difference--;
  }
  
  generate_read_events();

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::rpp" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
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
  CoSupport::Streams::dout << this->name() << ": ";
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::wpp" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif

  assert(n == 1);

  //be paranoiac
  //allocate memory, if not already done by user
  this->allocate_buffer();

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  if (buffer_analysis != NULL)
    buffer_analysis->production_update();
#endif

  // Move to next loop iteration
  if((*this).src_loop_iterator.inc()){
    //new schedule period has been started
#ifndef SYSTEMOC_ENABLE_VPC
    visible_schedule_period_difference++;
#endif
  }

#ifdef SYSTEMOC_ENABLE_VPC
  latencyQueue.addEntry(n, le);
#else
  //make directly visible
  incUsedStorage();
#endif
  
  /// Memory will not be written anymore
  (*this).release_buffer();
  
  generate_write_events();

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::wpp" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif

    
}

#ifdef SYSTEMOC_ENABLE_VPC
template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::latencyExpired(size_t n){

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << this->name() << ": ";
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::latencyExpired(size_t n)" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
  assert(n == 1);

  // Move to next loop iteration
  if((*this).src_iterator_visible.inc()){
    //new schedule period has been started
    visible_schedule_period_difference++;
  }

  //Make next token visible
  incUsedStorage();
  
  generate_write2Snk_events();

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << this->name() << ": ";
  CoSupport::Streams::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::latencyExpired(size_t n)" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif

}
#endif

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::generate_write_events() {
  generate_write2Src_events();
#ifndef SYSTEMOC_ENABLE_VPC
  generate_write2Snk_events();  
#endif
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::generate_write2Src_events(){
  //Check, if a new effective token be accepted
  if (unusedStorage() != 0){
    eventEffTokenFree.notify();
  }else{
    eventEffTokenFree.reset();
  }
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::generate_write2Snk_events(){
  // check, if a new window has been generated
  if (usedStorage() != 0){
    eventWindowAvailable.notify();
  }

  //indicate write operation
  eventWrite.notify();
}


template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::generate_read_events() {
  generate_read2Snk_events();
  generate_read2Src_events();  
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::generate_read2Snk_events(){
  // check, if there is still a window available
  if(usedStorage() != 0){
    eventWindowAvailable.notify();
  }else{
    eventWindowAvailable.reset();
  }
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::generate_read2Src_events(){
  //Check, if we can accept a new effective token
  if(unusedStorage() != 0){
    eventEffTokenFree.notify();
  }

  //indicate read operation
  eventRead.notify();
}

template <class BUFFER_CLASS>
smoc_event& smoc_md_fifo_kind<BUFFER_CLASS>::getEventAvailable(size_t n) {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << this->name() << ": ";
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getEventAvailable" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif

  //std::cerr << "My name: " << this->name() << std::endl;
  assert((n == 1) || (n == MAX_TYPE(size_t)));
  
  if (n == 1){
    if (usedStorage() >= n){
      eventWindowAvailable.notify();
    }
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    return eventWindowAvailable;
  }else{
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    return eventWrite;
  }

}

template <class BUFFER_CLASS>
smoc_event& smoc_md_fifo_kind<BUFFER_CLASS>::getEventFree(size_t n) {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << (*this).name() << ": ";
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getEventFree" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
  assert((n == 1) || (n == MAX_TYPE(size_t)));
  
  if (n == 1){
    if (unusedStorage() >= n){
      eventEffTokenFree.notify();
    }
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    return eventEffTokenFree;
  }else{
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    return eventRead;
  }
};

template <class BUFFER_CLASS>
const char* 
smoc_md_fifo_kind<BUFFER_CLASS>::getChannelTypeString() const {
  const char* my_type = "md_fifo";
  return my_type;
}


template <class BUFFER_CLASS>
void 
smoc_md_fifo_kind<BUFFER_CLASS>::channelAttributes(smoc_modes::PGWriter &pgw) const {
  
  BUFFER_CLASS::channelAttributes(pgw);

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  /* Simulated buffer size */
  if (buffer_analysis != NULL){
    std::stringstream temp;    
    buffer_analysis->dump_results(temp);
    pgw << "<attribute type=\"sim_size\" value=\""
        << temp.str()
        << "\"/>"
        << std::endl;  
  }
#endif    
};








/// Provide the multi-dimensional FIFO with the SysteMoC Channel Interface
template <typename T_DATA_TYPE, 
          class BUFFER_CLASS, 
          template <typename> class STORAGE_OUT_TYPE
         >
class smoc_md_fifo_storage
  : public smoc_chan_if</*smoc_md_fifo_kind<BUFFER_CLASS>,*/
                        T_DATA_TYPE,
                        smoc_md_snk_port_access_if,
                        smoc_md_src_port_access_if,
                        STORAGE_OUT_TYPE
                       >,
      public smoc_md_fifo_kind<BUFFER_CLASS> 
{
  
public:

  typedef T_DATA_TYPE                                              data_type;
  typedef smoc_md_fifo_kind<BUFFER_CLASS> parent_type;
  typedef smoc_md_fifo_storage<data_type, 
                               BUFFER_CLASS, 
                               STORAGE_OUT_TYPE>       this_type;

  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  
  typedef typename STORAGE_OUT_TYPE<data_type>::storage_type  storage_out_type;
  typedef typename STORAGE_OUT_TYPE<data_type>::return_type   return_out_type;

  typedef typename BUFFER_CLASS::template smoc_md_storage_access_src<storage_out_type,return_out_type>  
  ring_out_type;
  typedef typename BUFFER_CLASS::template smoc_md_storage_access_snk<storage_in_type,return_in_type>  
  ring_in_type;

  typedef smoc_storage<data_type>       storage_type;

  /// Make buffer_init visible
  typedef typename parent_type::buffer_init buffer_init;  

  typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;  

#if 0
  //The HW-FIFO requires access to _usedStorage;
  //and to _usedStorageValid.
  //As these elements are private, declare it as friend.
  friend class smoc_md_hw_fifo_kind<BUFFER_CLASS>;
#endif

public:

  class chan_init
    : public parent_type::chan_init {
    friend class smoc_md_fifo_storage<T_DATA_TYPE, BUFFER_CLASS, STORAGE_OUT_TYPE>;
  protected:
    chan_init( const std::string& name, 
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

  ring_in_type * getReadPortAccess() {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << this->name() << ": ";
    CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getReadPortAccess" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
    ring_in_type *r = new ring_in_type();
    initStorageAccess(*r);
    r->SetBuffer(storage);
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::getReadPortAccess" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    return r;
  }


  ring_out_type * getWritePortAccess() {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << this->name() << ": ";
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getWritePortAccess" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
  ring_out_type *r = new ring_out_type();
    initStorageAccess(*r);
    r->SetBuffer(storage);
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::getWritePortAccess" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
  return r;
  }

  virtual void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<fifo tokenType=\"" << typeid(data_type).name() << "\"/>" << std::endl;
  };  

};



template <class BUFFER_CLASS, 
          template <typename> class STORAGE_OUT_TYPE>
class smoc_md_fifo_storage<void,BUFFER_CLASS,STORAGE_OUT_TYPE>
  : public smoc_chan_if</*smoc_md_fifo_kind<BUFFER_CLASS>,*/
                        void,
                        smoc_md_snk_port_access_if,
                        smoc_md_src_port_access_if,
                        STORAGE_OUT_TYPE
                       >,
      public smoc_md_fifo_kind<BUFFER_CLASS>
{
  
public:

  typedef void data_type;
  typedef smoc_md_fifo_kind<BUFFER_CLASS> parent_type;
  typedef smoc_md_fifo_storage<data_type, 
                               void, 
                               STORAGE_OUT_TYPE>       this_type;

  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  
  typedef typename STORAGE_OUT_TYPE<data_type>::storage_type  storage_out_type;
  typedef typename STORAGE_OUT_TYPE<data_type>::return_type   return_out_type;

  typedef typename BUFFER_CLASS::template smoc_md_storage_access_src<storage_out_type,return_out_type>  
  ring_out_type;
  typedef typename BUFFER_CLASS::template smoc_md_storage_access_snk<storage_in_type,return_in_type>  
  ring_in_type;

  typedef smoc_storage<data_type>       storage_type;

  /// Make buffer_init visible
  typedef typename parent_type::buffer_init buffer_init;  

  typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;  

public:

  class chan_init
    : public parent_type::chan_init {
    friend class smoc_md_fifo_storage<void, BUFFER_CLASS, STORAGE_OUT_TYPE>;
  protected:
    chan_init( const std::string& name, 
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

  ring_in_type * getReadPortAccess() {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << this->name() << ": ";
    CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getReadPortAccess" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
    ring_in_type *r = new ring_in_type();
    initStorageAccess(*r);
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
    CoSupport::Streams::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::getReadPortAccess" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    return r;
  }


  ring_out_type * getWritePortAccess() {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << this->name() << ": ";
  CoSupport::Streams::dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getWritePortAccess" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
  ring_out_type *r = new ring_out_type();
  initStorageAccess(*r);
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 101
  CoSupport::Streams::dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::getWritePortAccess" << std::endl;
  CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
  return r;
  }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    pgw << "<fifo tokenType=\"" << typeid(data_type).name() << "\"/>" << std::endl;
  };  


  ~smoc_md_fifo_storage() { 
  }

};

template <typename T_DATA_TYPE,
          template <typename> class STORAGE_OUT_TYPE
         >
class smoc_md_fifo;

template <typename T_DATA_TYPE,
          template <typename> class STORAGE_OUT_TYPE = smoc_storage_out
         >
class smoc_md_fifo_type
  : public smoc_md_fifo_storage<T_DATA_TYPE, 
                                smoc_simple_md_buffer_kind, 
                                STORAGE_OUT_TYPE> {
  friend class smoc_md_fifo<T_DATA_TYPE, STORAGE_OUT_TYPE>;
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

public:
  
#ifdef SYSTEMOC_ENABLE_VPC
  void commitRead(size_t consume, const smoc_ref_event_p &le)
#else
  void commitRead(size_t consume)
#endif
  {
#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::Streams::dout << this->name() << ": Enter commitRead" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
    CoSupport::Streams::dout << "Iteration : " << (*this).snk_loop_iterator.iteration_vector();
    CoSupport::Streams::dout << std::endl;
    CoSupport::Streams::dout << "Consume " << consume << " windows" << std::endl;
#endif

    //Currently, we only support the consumption zero or one window.
    assert(consume <= 1);
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(this, consume);
#endif
    if (consume >= 1)
      this->rpp(consume);

#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::Streams::dout << "Leave commitRead" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif

  }
  
#ifdef SYSTEMOC_ENABLE_VPC
  void commitWrite(size_t produce, const smoc_ref_event_p &le)
#else
  void commitWrite(size_t produce)
#endif
  {

#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::Streams::dout << this->name() << ": Enter commitWrite" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
    CoSupport::Streams::dout << "Iteration : " << (*this).src_loop_iterator.iteration_vector();
    CoSupport::Streams::dout << std::endl;
    CoSupport::Streams::dout << "Write " << produce << " effective tokens" << std::endl;
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
    CoSupport::Streams::dout << "Leave commitWrite" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif

  }
public:
  // constructors
  smoc_md_fifo_type( const chan_init &i )
    : parent_type(i) {
  }

  /// @brief See smoc_port_registry
  smoc_chan_out_base_if* createEntry()
    { /*FIXME*/return this; }

  /// @brief See smoc_port_registry
  smoc_port_in_base_if* createOutlet()
    { /*FIXME*/return this; }
  
  // bounce functions
  size_t numAvailable() const { 
#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::Streams::dout << this->name() << ": Enter numAvailable()" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
    size_t return_value = this->usedStorage();
#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::Streams::dout << "Fifo contains at least " << return_value << " windows" << std::endl;
    CoSupport::Streams::dout << "Leave numAvailable()" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    return return_value;
  }
  size_t numFree() const { 
#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::Streams::dout << this->name() << ": Enter numFree()" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif
    size_t return_value = this->unusedStorage(); 

#if VERBOSE_LEVEL_SMOC_MD_FIFO >= 2
    CoSupport::Streams::dout << "Fifo accepts at least " << return_value << " effective tokens" << std::endl;
    CoSupport::Streams::dout << "Leave numFree()" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif
    return return_value;
  }

  smoc_event &dataAvailableEvent(size_t n)
    { return this->getEventAvailable(n); }
  smoc_event &spaceAvailableEvent(size_t n)
    { return this->getEventFree(n); }
  size_t inTokenId() const
    { assert(!"FIXME: Not implemented !!!"); return 0; }
  size_t outTokenId() const
    { assert(!"FIXME: Not implemented !!!"); return 0; }

};

/// Channel initialization class
template <typename T, 
          template <typename> class STORAGE_OUT_TYPE = smoc_storage_out>
class smoc_md_fifo
: public smoc_md_fifo_type<T, STORAGE_OUT_TYPE>::chan_init {
  typedef smoc_md_fifo<T, STORAGE_OUT_TYPE> this_type;
public:
  typedef T                                       data_type;
  //Identification of corresponding channel class
  typedef smoc_md_fifo_type<T, STORAGE_OUT_TYPE>  chan_type;
  //Make buffer_init visible
  typedef typename smoc_md_fifo_type<T, STORAGE_OUT_TYPE>::buffer_init buffer_init;
public:
  smoc_md_fifo( const smoc_wsdf_edge_descr& wsdf_edge_param, 
                size_t n
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
		, smoc_md_ba::smoc_md_ba_user_interface* ba_ui = NULL
#endif
		)
    : smoc_md_fifo_type<T,STORAGE_OUT_TYPE>::chan_init("",
                                                       assemble_buffer_init(wsdf_edge_param, n)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
						       , ba_ui
#endif
                                                       ),
    wsdf_edge_param(wsdf_edge_param),
    chan(NULL)
  {}

  explicit smoc_md_fifo( const std::string& name, 
                         const smoc_wsdf_edge_descr& wsdf_edge_param, 
                         size_t n)
    : smoc_md_fifo_type<T, STORAGE_OUT_TYPE>::chan_init(name,
                                                        assemble_buffer_init(wsdf_edge_param, n)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
							, NULL
#endif
							
                                                        ),
      wsdf_edge_param(wsdf_edge_param),
      chan(NULL)
  {}

  smoc_md_fifo(const smoc_wsdf_edge<T,STORAGE_OUT_TYPE>& edge_param,
               const smoc_wsdf_src_param& src_param,
               const smoc_wsdf_snk_param& snk_param)
    : smoc_md_fifo_type<T, STORAGE_OUT_TYPE>::chan_init(
                                                        edge_param.name,
                                                        assemble_buffer_init(assemble_wsdf_edge(edge_param, src_param, snk_param), edge_param.n)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
							, edge_param.ba_ui
#endif
                                                        ),
      wsdf_edge_param(assemble_wsdf_edge(edge_param, src_param, snk_param)),
      chan(NULL)
  {}

  /// @brief Copy constructor
  smoc_md_fifo(const this_type &x)
    : smoc_md_fifo_type<T, STORAGE_OUT_TYPE>::chan_init(x),
      wsdf_edge_param(x.wsdf_edge_param),
      chan(NULL)
  {}

  template<unsigned N,template<class> class S>
  this_type &connect(smoc_md_port_out<data_type,N,S>& p) {
    p(*dynamic_cast<chan_type *>(getChan()->getEntry(&p)));
    p.setFiringLevelMap(
      wsdf_edge_param.calc_src_iteration_level_table());
    return *this;
  }

  template<unsigned N,template<class> class S>
  this_type &connect(smoc_md_iport_out<data_type,N,S>& p) {
    p(*dynamic_cast<chan_type *>(getChan()->getEntry(&p)));
    p.setFiringLevelMap(
      wsdf_edge_param.calc_src_iteration_level_table());
    return *this;
  }

  template<unsigned N,template<class,class> class B>
  this_type &connect(smoc_md_port_in<data_type,N,B>& p) {
    p(*dynamic_cast<chan_type *>(getChan()->getOutlet(&p)));
    p.setFiringLevelMap(
      wsdf_edge_param.calc_snk_iteration_level_table());
    return *this;
  }

  template<unsigned N>
  this_type &connect(smoc_md_iport_in<data_type,N>& p) {
    p(*dynamic_cast<chan_type *>(getChan()->getOutlet(&p)));
    p.setFiringLevelMap(
      wsdf_edge_param.calc_snk_iteration_level_table());
    return *this;
  }

  const smoc_wsdf_edge_descr wsdf_edge_param;

private:
  chan_type *chan;
private:

  smoc_wsdf_edge_descr assemble_wsdf_edge(const smoc_wsdf_edge<T, STORAGE_OUT_TYPE>& edge_param,
                                          const smoc_wsdf_src_param& src_param,
                                          const smoc_wsdf_snk_param& snk_param) const {
#if VERBOSE_LEVEL_SMOC_MD_FIFO == 103
    CoSupport::Streams::dout << "Enter smoc_md_fifo::assemble_wsdf_edge" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Up;
#endif

    typedef smoc_wsdf_edge_descr::udata_type     udata_type;
    typedef smoc_wsdf_edge_descr::uvector_type   uvector_type;

    unsigned token_dimensions = src_param.src_firing_blocks[0].size();

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 103
    CoSupport::Streams::dout << "src_param.src_firing_blocks = " << src_param.src_firing_blocks << std::endl;
    CoSupport::Streams::dout << "token_dimensions = " << token_dimensions << std::endl;
#endif
    
    
    uvector_type d;

    if (edge_param.d_valid)
      d = edge_param.d;
    else
      d = uvector_type(token_dimensions,(udata_type)0);

#if VERBOSE_LEVEL_SMOC_MD_FIFO == 103
    CoSupport::Streams::dout << "d = " << d;
    CoSupport::Streams::dout << std::endl;

    CoSupport::Streams::dout << "src_firing_blocks = " << src_param.src_firing_blocks;
    CoSupport::Streams::dout << std::endl;

    CoSupport::Streams::dout << "snk_firing_blocks = " << snk_param.snk_firing_blocks;
    CoSupport::Streams::dout << std::endl;

    CoSupport::Streams::dout << "u0 = " << snk_param.u0;
    CoSupport::Streams::dout << std::endl;

    CoSupport::Streams::dout << "c = " << snk_param.c;
    CoSupport::Streams::dout << std::endl;

    CoSupport::Streams::dout << "delta_c = " << snk_param.delta_c;
    CoSupport::Streams::dout << std::endl;

    CoSupport::Streams::dout << "bs = " << snk_param.bs;
    CoSupport::Streams::dout << std::endl;

    CoSupport::Streams::dout << "bt = " << snk_param.bt;
    CoSupport::Streams::dout << std::endl;
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
    CoSupport::Streams::dout << "Leave smoc_md_fifo::assemble_wsdf_edge" << std::endl;
    CoSupport::Streams::dout << CoSupport::Streams::Indent::Down;
#endif

    return wsdf_edge_param;
  }

  buffer_init assemble_buffer_init(const smoc_wsdf_edge_descr& wsdf_edge_param, 
                                   size_t n) const{
    buffer_init return_value(wsdf_edge_param,                             
                             n);

    return return_value;
  }

  chan_type *getChan() {
    if (chan == NULL)
      chan = new chan_type(*this);
    return chan;
  }

  // disable
  this_type &operator =(const this_type &);
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
    : name(""),
      n(n), 
      d(d), 
      d_valid(true)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
    , ba_ui(NULL)
#endif
  {}

  smoc_wsdf_edge(size_t n
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
		, smoc_md_ba::smoc_md_ba_user_interface* ba_ui = NULL
#endif
                 )
    : name(""),
      n(n), 
      d_valid(false)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
    , ba_ui(ba_ui)
#endif
  {}


  explicit smoc_wsdf_edge(const std::string& name, 
                          size_t n,
                          const uvector_type& d
                          )
    : name(name),
      n(n), 
      d(d), 
      d_valid(true)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
    , ba_ui(NULL)
#endif
  {}

  explicit smoc_wsdf_edge(const std::string& name, 
                          size_t n)
    : name(name),
      n(n), 
      d_valid(false)
#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
    , ba_ui(NULL)
#endif
  {}

public:
  std::string name;
  const size_t n;

  const uvector_type d;
  const bool d_valid;

#ifdef ENABLE_SMOC_MD_BUFFER_ANALYSIS
  smoc_md_ba::smoc_md_ba_user_interface* ba_ui;
#endif
};


#endif // _INCLUDED_SMOC_FIFO_HPP

