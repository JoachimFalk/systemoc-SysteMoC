#ifndef _INCLUDED_SMOC_MD_FIFO_HPP
#define _INCLUDED_SMOC_MD_FIFO_HPP

#include <cosupport/commondefs.h>

#ifndef NO_SMOC
#include <smoc_chan_if.hpp>
#endif
#include <smoc_storage.hpp>


//#include <systemc.h>
//#include <vector>
//#include <queue>
//#include <map>

#ifndef NO_SMOC
#include <hscd_tdsim_TraceLog.hpp>
#else
#include <string>
#endif

#include <smoc_md_loop.hpp>
#include <smoc_md_buffer.hpp>
#include <smoc_wsdf_edge.hpp>
#include <smoc_debug_out.hpp>

#include <smoc_md_port.hpp>

/// 101: SysteMoC Interface
/// 102: Memory access error
/// 103: Parameter propagation
#define VERBOSE_LEVEL 101



template <typename T>
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
		const unsigned int token_dimensions;
		const iter_domain_vector_type src_iter_max;
		const iter_domain_vector_type snk_iter_max;
		const buffer_init b;
		
  protected:
    chan_init(const char *name,
							const unsigned int token_dimensions,
							const iter_domain_vector_type& src_iter_max,
							const iter_domain_vector_type& snk_iter_max,
							const buffer_init& b)
      : name(name),
				token_dimensions(token_dimensions),
				src_iter_max(src_iter_max),
				snk_iter_max(snk_iter_max),
				b(b)
		{}
  };


    
public:
  smoc_md_fifo_kind(const chan_init &i)
#ifndef NO_SMOC
		: smoc_nonconflicting_chan(i.name != NULL ? i.name : sc_gen_unique_name( "smoc_md_fifo" ) ),
			BUFFER_CLASS(i.b),
#else
			: BUFFER_CLASS(i.b),
				_name(i.name != NULL ? i.name : "md_fifo"),
#endif
			src_loop_iterator(i.src_iter_max, i.token_dimensions),
			snk_loop_iterator(i.snk_iter_max, i.token_dimensions),
			schedule_period_difference(0)
	{}

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
#ifdef ENABLE_SYSTEMC_VPC
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

private:
  
  //disabled
  smoc_md_fifo_kind( const this_type&);
  this_type& operator = (const this_type &);
  
protected:

#ifdef NO_SMOC
private:
	std::string _name;
protected:
	std::string name() const {return _name;}
#endif
  
  /// Current source and sink iteration vectors.
  /// They specify, which iteration is executed NEXT.	
  smoc_md_static_loop_iterator src_loop_iterator;
  smoc_md_static_loop_iterator snk_loop_iterator;

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




};


template <class BUFFER_CLASS>
size_t smoc_md_fifo_kind<BUFFER_CLASS>::usedStorage() const{

#if (VERBOSE_LEVEL == 101) || (VERBOSE_LEVEL == 102)
	dout << this->name() << ": ";
	dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::usedStorage()" << endl;
	dout << inc_level;
#endif
	size_t return_value = 0;

  // In this function we assume, that the data element
  // belonging to the maximum window iteration is produced
  // by the source at last!

#if (VERBOSE_LEVEL == 101) || (VERBOSE_LEVEL == 102)
	dout << "Next sink invocation ID: " << snk_loop_iterator.iteration_vector();
	dout << endl;
#endif
		

  // Get latest produced data element required by the
  // next sink actor invocation
	smoc_md_loop_snk_data_element_mapper::data_element_id_type 
		src_data_element_id((*this).snk_data_el_mapper.token_dimensions());
	if (!(*this).snk_data_el_mapper.get_req_src_data_element(snk_loop_iterator,
																													 src_data_element_id)){
		/// Source does not need to produce anythouth for 
		/// this sink iteration
#if (VERBOSE_LEVEL == 101) || (VERBOSE_LEVEL == 102)
		dout << "Source actor does not need to produce anything" << endl;
#endif
		return_value = 1;
	}else{		

#if (VERBOSE_LEVEL == 101) || (VERBOSE_LEVEL == 102)
		dout << "Required data element: " << src_data_element_id;
		dout << endl;
#endif

	
		// Required source iteration for production of this data
		// element
		iter_domain_vector_type req_src_iteration(src_loop_iterator.iterator_depth()); 
		smoc_md_loop_src_data_element_mapper::id_type schedule_period_offset;
#if VERBOSE_LEVEL == 102
		dout << "src_loop_iterator.iterator_depth() = " << src_loop_iterator.iterator_depth() << endl;
#endif
		bool temp = 
			(*this).src_data_el_mapper.get_src_loop_iteration(src_data_element_id,
																												req_src_iteration,
																												schedule_period_offset
																												);
		// error checking
		assert(temp);

#if (VERBOSE_LEVEL == 101) || (VERBOSE_LEVEL == 102)
		dout << "Required src iteration: " << req_src_iteration;
		dout << " (schedule_period_offset  = " << schedule_period_offset << ")";
		dout << endl;
#endif
	
		if (schedule_period_difference > schedule_period_offset){
			//Sink actor can fire
			return_value = 1;
#if (VERBOSE_LEVEL == 101) || (VERBOSE_LEVEL == 102)
			dout << "Sink can fire due to schedule period difference" << endl;
			dout << inc_level;
			dout << "schedule_period_difference = " << schedule_period_difference << endl;
			dout << "schedule_period_offset = " << schedule_period_offset << endl;
			dout << dec_level;
#endif
		}else if (req_src_iteration.is_lex_smaller_than(src_loop_iterator.iteration_vector())){
			//Sink actor can fire
			return_value = 1;
		}else{
			//Sink actor is blocked
			return_value = 0;
		}	
	}

#if (VERBOSE_LEVEL == 101) || (VERBOSE_LEVEL == 102)
	dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::usedStorage()" << endl;
	dout << dec_level;
#endif

	return return_value;
		
}

template <class BUFFER_CLASS>
size_t smoc_md_fifo_kind<BUFFER_CLASS>::unusedStorage() const {
#if VERBOSE_LEVEL == 101
	dout << this->name() << ": ";
	dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::unusedStorage()" << endl;
	dout << inc_level;
	dout << "src_loop_iterator = " << src_loop_iterator.iteration_vector();
	dout << endl;
#endif

	size_t return_value;

	if (BUFFER_CLASS::unusedStorage(src_loop_iterator)){
		return_value = 1;
	}else{
		return_value = 0;
	}

#if VERBOSE_LEVEL == 101
	dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::unusedStorage()" << endl;
	dout << dec_level;
#endif

	return return_value;
}

template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::rpp(size_t n){
#if VERBOSE_LEVEL == 101
	dout << this->name() << ": ";
	dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::rpp" << endl;
	dout << inc_level;
#endif
  assert(n == 1);

	//free memory
	(*this).free_buffer(snk_loop_iterator);
	
  // Move to next loop iteration
  if(snk_loop_iterator.inc()){
		//new schedule period has been started
		schedule_period_difference--;
	}
	
  generate_read_events();

#if VERBOSE_LEVEL == 101
	dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::rpp" << endl;
	dout << dec_level;
#endif
	
}

#ifdef ENABLE_SYSTEMC_VPC
template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::wpp(size_t n, const smoc_ref_event_p &le){
#else
template <class BUFFER_CLASS>
void smoc_md_fifo_kind<BUFFER_CLASS>::wpp(size_t n){
#endif

#if VERBOSE_LEVEL == 101
	dout << this->name() << ": ";
	dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::wpp" << endl;
	dout << inc_level;
#endif

  assert(n == 1);

	//be paranoiac
	//allocate memory, if not already done by user
	this->allocate_buffer(src_loop_iterator);
	

  // Move to next loop iteration
	if(src_loop_iterator.inc()){
		//new schedule period has been started
		schedule_period_difference++;
	}
	/// Memory has been occupied
	(*this).src_allocated = false;
	
  generate_write_events();

#if VERBOSE_LEVEL == 101
	dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::wpp" << endl;
	dout << dec_level;
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
#if VERBOSE_LEVEL == 101
	dout << this->name() << ": ";
	dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getEventAvailable" << endl;
	dout << inc_level;
#endif

  assert((n == 1) || (n == MAX_TYPE(size_t)));
	
  if (n == 1){
		if (usedStorage() >= n){
			eventWindowAvailable.notify();
		}
#if VERBOSE_LEVEL == 101
		dout << dec_level;
#endif
    return eventWindowAvailable;
	}else{
#if VERBOSE_LEVEL == 101
		dout << dec_level;
#endif
    return eventWrite;
	}

}
#endif

#ifndef NO_SMOC
template <class BUFFER_CLASS>
smoc_event& smoc_md_fifo_kind<BUFFER_CLASS>::getEventFree(size_t n) {
#if VERBOSE_LEVEL == 101
	dout << (*this).name() << ": ";
	dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::getEventFree" << endl;
	dout << inc_level;
#endif
  assert((n == 1) || (n == MAX_TYPE(size_t)));
	
  if (n == 1){
		if (unusedStorage() >= n){
			eventEffTokenFree.notify();
		}
#if VERBOSE_LEVEL == 101
		dout << dec_level;
#endif
    return eventEffTokenFree;
	}else{
#if VERBOSE_LEVEL == 101
		dout << dec_level;
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
					template <typename, typename> class R_IN, 
					template <typename, typename> class R_OUT >
class smoc_md_fifo_storage
#ifndef NO_SMOC
	: public smoc_chan_if<smoc_md_fifo_kind<BUFFER_CLASS>,
	                      T_DATA_TYPE,
												R_IN,
	                      R_OUT > 
#else
		: public smoc_md_fifo_kind<BUFFER_CLASS>
		//: public smoc_dummy_chan_if<smoc_md_fifo_kind<BUFFER_CLASS>,
		//														T_DATA_TYPE,
		//														R_IN,
		//														R_OUT > 	
#endif
{
	
public:

  typedef T_DATA_TYPE                                              data_type;
#ifndef NO_SMOC
	typedef smoc_chan_if<smoc_md_fifo_kind<BUFFER_CLASS>,
											 T_DATA_TYPE,
											 R_IN,
											 R_OUT >  parent_type;
#else
	typedef smoc_md_fifo_kind<BUFFER_CLASS> parent_type;
#endif
  typedef smoc_md_fifo_storage<data_type, BUFFER_CLASS, R_IN, R_OUT>       this_type;
#ifndef NO_SMOC
  typedef typename this_type::access_out_type  ring_out_type;
  typedef typename this_type::access_in_type   ring_in_type;
#endif
  typedef smoc_storage<data_type>	     storage_type;

	/// Make buffer_init visible
	typedef typename parent_type::buffer_init buffer_init;	

	typedef typename parent_type::iter_domain_vector_type iter_domain_vector_type;	

public:

	class chan_init
    : public parent_type::chan_init {
    friend class smoc_md_fifo_storage<T_DATA_TYPE, BUFFER_CLASS, R_IN, R_OUT>;
  protected:
    chan_init( const char *name, 
							 const unsigned int token_dimensions,
							 const iter_domain_vector_type& src_iter_max,
							 const iter_domain_vector_type& snk_iter_max,
							 const buffer_init &b )
      : parent_type::chan_init(name, 
															 token_dimensions,
															 src_iter_max, 
															 snk_iter_max, 
															 b) {}
  };

private:
  typename BUFFER_CLASS::template smoc_md_storage_type<storage_type>::type *storage;

protected:
	smoc_md_fifo_storage( const chan_init &i)
		: parent_type(i)
	{
		createStorage(storage);
	}

#ifndef NO_SMOC
	void accessSetupIn(ring_in_type &r) {
#if VERBOSE_LEVEL == 101
		dout << this->name() << ": ";
		dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::accessSetupIn" << endl;
		dout << inc_level;
#endif
		initStorageAccess(r);
		r.SetBuffer(storage);
		r.SetIterator((*this).snk_loop_iterator);
#if VERBOSE_LEVEL == 101
		dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::accessSetupIn" << endl;
		dout << dec_level;
#endif
  }
#endif

#ifndef NO_SMOC
  void accessSetupOut(ring_out_type &r) {
#if VERBOSE_LEVEL == 101
  dout << this->name() << ": ";
	dout << "Enter smoc_md_fifo_kind<BUFFER_CLASS>::accessSetupOut" << endl;
	dout << inc_level;
#endif
		initStorageAccess(r);
		r.SetBuffer(storage);
		r.SetIterator((*this).src_loop_iterator);
#if VERBOSE_LEVEL == 101
	dout << "Leave smoc_md_fifo_kind<BUFFER_CLASS>::accessSetupOut" << endl;
	dout << dec_level;
#endif
  }

	void channelContents(smoc_modes::PGWriter &pgw) const {};	
#endif

	~smoc_md_fifo_storage() { 
		delete storage; 
	}

};


template <typename T_DATA_TYPE>
class smoc_md_fifo_type
  : public smoc_md_fifo_storage<T_DATA_TYPE, 
																smoc_simple_md_buffer_kind, 
																smoc_simple_md_buffer_kind::smoc_md_storage_access_snk, 
																smoc_simple_md_buffer_kind::smoc_md_storage_access_src> {
public:
  typedef T_DATA_TYPE						      data_type;
  typedef smoc_md_fifo_storage<T_DATA_TYPE, 
															 smoc_simple_md_buffer_kind,
															 smoc_simple_md_buffer_kind::smoc_md_storage_access_snk, 
															 smoc_simple_md_buffer_kind::smoc_md_storage_access_src
															 > parent_type;
	typedef smoc_md_fifo_type<T_DATA_TYPE> this_type;
  
  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  
  typedef typename smoc_storage_out<data_type>::storage_type  storage_out_type;
  typedef typename smoc_storage_out<data_type>::return_type   return_out_type;

	//Make channel init visible
	typedef typename parent_type::chan_init chan_init;
	typedef typename parent_type::buffer_init buffer_init;

#ifdef NO_SMOC
public:
#else
protected:
#endif
  
#ifdef ENABLE_SYSTEMC_VPC
  void commExecIn(size_t consume, const smoc_ref_event_p &le)
#else
  void commExecIn(size_t consume)
#endif
  {
#if VERBOSE_LEVEL >= 2
		dout << this->name() << ": Enter commExecIn" << endl;
		dout << inc_level;
		dout << "Iteration : " << (*this).snk_loop_iterator.iteration_vector();
		dout << endl;
		dout << "Consume " << consume << " windows" << endl;
#endif

		//Currently, we only support the consumption of exactly one
		//effective token respectively window
		assert(consume == 1);
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(consume, this->name());
#endif
    this->rpp(consume);

#if VERBOSE_LEVEL >= 2
		dout << "Leave commExecIn" << endl;
		dout << dec_level;
#endif

  }
  
#ifdef ENABLE_SYSTEMC_VPC
  void commExecOut(size_t produce, const smoc_ref_event_p &le)
#else
  void commExecOut(size_t produce)
#endif
  {

#if VERBOSE_LEVEL >= 2
		dout << this->name() << ": Enter commExecOut" << endl;
		dout << inc_level;
		dout << "Iteration : " << (*this).src_loop_iterator.iteration_vector();
		dout << endl;
		dout << "Write " << produce << " effective tokens" << endl;
#endif


		//Currently, we only support the consumption of exactly one
		//effective token respectively window
		assert(produce == 1);
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(produce, this->name());
#endif
#ifdef ENABLE_SYSTEMC_VPC
    this->wpp(produce, le);
#else
    this->wpp(produce);
#endif

#if VERBOSE_LEVEL >= 2
		dout << "Leave commExecOut" << endl;
		dout << dec_level;
#endif

  }
public:
  // constructors
  smoc_md_fifo_type( const chan_init &i )
    : parent_type(i) {
  }

  // bounce functions
  size_t committedOutCount() const { 
#if VERBOSE_LEVEL >= 2
		dout << this->name() << ": Enter committedOutCount()" << endl;
		dout << inc_level;
#endif
		size_t return_value = this->usedStorage();
#if VERBOSE_LEVEL >= 2
		dout << "Fifo contains at least " << return_value << " windows" << endl;
		dout << "Leave committedOutCount()" << endl;
		dout << dec_level;
#endif
		return return_value;
	}
  size_t committedInCount() const { 
#if VERBOSE_LEVEL >= 2
		dout << this->name() << ": Enter committedInCount()" << endl;
		dout << inc_level;
#endif
		size_t return_value = this->unusedStorage(); 

#if VERBOSE_LEVEL >= 2
		dout << "Fifo accepts at least " << return_value << " effective tokens" << endl;
		dout << "Leave committedInCount()" << endl;
		dout << dec_level;
#endif
		return return_value;
	}
#ifndef NO_SMOC
  smoc_event &blockEventOut(size_t n)
    { return this->getEventAvailable(n); }
  smoc_event &blockEventIn(size_t n)
    { return this->getEventFree(n); }
#endif
};

/// Channel initialization class
template <typename T>
class smoc_md_fifo
  : public smoc_md_fifo_type<T>::chan_init {
public:
  typedef T                   data_type;
  typedef smoc_md_fifo<T>        this_type;

	//Identification of corresponding channel class
  typedef smoc_md_fifo_type<T>   chan_type;

	//Make buffer_init visible
	typedef typename smoc_md_fifo_type<T>::buffer_init buffer_init;

public:
  
  smoc_md_fifo( const smoc_wsdf_edge_descr& wsdf_edge_param, 
								size_t n)
    : smoc_md_fifo_type<T>::chan_init(NULL,
																			wsdf_edge_param.token_dimensions,
																			wsdf_edge_param.src_iteration_max(),
																			wsdf_edge_param.snk_iteration_max(),
																			assemble_buffer_init(wsdf_edge_param, n))
	{}

  explicit smoc_md_fifo( const char *name, 
												 const smoc_wsdf_edge_descr& wsdf_edge_param, 
												 size_t n)
    : smoc_md_fifo_type<T>::chan_init(name,
																			wsdf_edge_param.token_dimensions,
																			wsdf_edge_param.src_iteration_max(),
																			wsdf_edge_param.snk_iteration_max(),
																			assemble_buffer_init(wsdf_edge_param, n)) 
	{}

	smoc_md_fifo(const smoc_wsdf_edge<T>& edge_param,
							 const smoc_wsdf_src_param& src_param,
							 const smoc_wsdf_snk_param& snk_param)
		: smoc_md_fifo_type<T>::chan_init(
					 edge_param.name,
					 assemble_wsdf_edge(edge_param, src_param, snk_param).token_dimensions,
					 assemble_wsdf_edge(edge_param, src_param, snk_param).src_iteration_max(),
					 assemble_wsdf_edge(edge_param, src_param, snk_param).snk_iteration_max(),
					 assemble_buffer_init(assemble_wsdf_edge(edge_param, src_param, snk_param), edge_param.n)
					 )
	{
	}
	

private:



	smoc_wsdf_edge_descr assemble_wsdf_edge(const smoc_wsdf_edge<T>& edge_param,
																					const smoc_wsdf_src_param& src_param,
																					const smoc_wsdf_snk_param& snk_param) const {
#if VERBOSE_LEVEL == 103
		dout << "Enter smoc_md_fifo::assemble_wsdf_edge" << endl;
		dout << inc_level;
#endif

		typedef smoc_wsdf_edge_descr::udata_type     udata_type;
		typedef smoc_wsdf_edge_descr::uvector_type   uvector_type;

		unsigned token_dimensions = src_param.src_firing_blocks[0].size();

#if VERBOSE_LEVEL == 103
		dout << "token_dimensions = " << token_dimensions << endl;
#endif
		
		
		uvector_type d;

		if (edge_param.d_valid)
			d = edge_param.d;
		else
			d = uvector_type(token_dimensions,(udata_type)0);

#if VERBOSE_LEVEL == 103
		dout << "d = " << d;
		dout << endl;

		dout << "src_firing_blocks = " << src_param.src_firing_blocks;
		dout << endl;

		dout << "snk_firing_blocks = " << snk_param.snk_firing_blocks;
		dout << endl;

		dout << "u0 = " << snk_param.u0;
		dout << endl;

		dout << "c = " << snk_param.c;
		dout << endl;

		dout << "delta_c = " << snk_param.delta_c;
		dout << endl;

		dout << "bs = " << snk_param.bs;
		dout << endl;

		dout << "bt = " << snk_param.bt;
		dout << endl;
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

#if VERBOSE_LEVEL == 103
		dout << "Leave smoc_md_fifo::assemble_wsdf_edge" << endl;
		dout << dec_level;
#endif

		return wsdf_edge_param;
	}
	

	buffer_init assemble_buffer_init(const smoc_wsdf_edge_descr& wsdf_edge_param, 
																	 size_t n) const{
		smoc_md_loop_src_data_element_mapper 
			src_data_el_mapper(wsdf_edge_param.src_data_element_mapping_matrix(),
												 wsdf_edge_param.src_data_element_mapping_vector(),
												 wsdf_edge_param.max_data_element_id()
												 );		
		smoc_md_loop_snk_data_element_mapper
			snk_data_el_mapper(wsdf_edge_param.snk_data_element_mapping_matrix(),
												 wsdf_edge_param.snk_data_element_mapping_vector(),
												 wsdf_edge_param.calc_border_condition_matrix(),
												 wsdf_edge_param.calc_low_border_condition_vector(),
												 wsdf_edge_param.calc_high_border_condition_vector()
												 );
		buffer_init return_value(src_data_el_mapper,
														 snk_data_el_mapper,
														 n);

		return return_value;
	}
};





/// If we want to annotate the parameters at the ports instead of
/// associating them all with the FIFO, we can use this class
template <typename T>
class smoc_wsdf_edge{
public:

	typedef T                   data_type;

	// associated channel init
	typedef smoc_md_fifo<T> chan_init_type;

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
