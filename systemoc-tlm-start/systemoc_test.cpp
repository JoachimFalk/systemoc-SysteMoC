
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

#include "debug_off.h"

#include "tlm.h"

#include "tlm_pvt_annotated_fifo.h"

#include "circular_buffer.h"

// SysteMoC uses different debugging style: if NDEBUG is set, debugging code is
//  disabled. Define macro for NDEBUG undefed code.
#ifndef NDEBUG
  #define DBG_SMOC(e) do {e;} while(0)
#else
  #define DBG_SMOC(e) do {} while(0)
#endif

enum e_colors {
  C_BLUE,
  C_RED
};

const int MAX_FIRE = 5;

#define USE_TRANSACTOR


/******************************************************************************
 * smoc_port_in_plug needs this interface to access storage
 *
 */
template<typename T>
class smoc_port_in_storage_read_if
{
public:
  typedef smoc_channel_access<const T&>           read_access_type;
  typedef typename read_access_type::return_type  access_return_type;//const T&

  //
  virtual read_access_type* getReadChannelAccess(void) = 0;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class smoc_port_in_storage :
  public smoc_port_in_storage_read_if<T>
{
public:
  typedef typename smoc_port_in_storage_read_if<T>::read_access_type
    read_access_type;
  typedef typename smoc_port_in_storage_read_if<T>::access_return_type
    access_return_type;
  typedef circular_buffer_ra_data<smoc_storage<T> >  storage_type;

  //
  smoc_port_in_storage(const size_t size = 1) :
    m_storage(size),
    m_read_access_wrapper(m_storage)
  {}

  //
  virtual ~smoc_port_in_storage(void) {}

  // call invalidate() on smoc_storages
  void invalidate(const size_t n)
  {
    for (size_t i = 0; i < n; ++i)
      m_storage[i].invalidate();
  }

  // wrapper for random_access_circular_buffer::skip()
  void skip(const size_t n) { m_storage.skip(n); }

  // wrapper for random_access_circular_buffer::is_empty()
  bool is_empty(void) const { return m_storage.is_empty(); }

  // wrapper for random_access_circular_buffer::is_full()
  bool is_full(void) const { return m_storage.is_full(); }

  // wrapper for random_access_circular_buffer::put()
  void put(const T& t) { m_storage.put(t); }

  // wrapper for random_access_circular_buffer:num_available()
  size_t num_available(void) const { return m_storage.num_available(); }

  /*
   * smoc_port_in_storage_read_if
   */
  read_access_type* getReadChannelAccess(void)
  {
    return &m_read_access_wrapper;
  }

private:
  /********************************************************
   * wraps our storage so smoc_port_in likes it
   */
  class read_access_wrapper :
    public read_access_type
  {
  public:
    read_access_wrapper(const storage_type &storage) :
      m_storage(storage)
#ifndef NDEBUG
      , m_limit(0)
#endif
    {}

    /*
     * write_access_type
     */
#ifndef NDEBUG
    // set limit and assert in operator[]
    void setLimit(size_t l) { m_limit = l; }
#endif

    const access_return_type operator[](size_t n) const
    {
      DBG_SMOC(assert(n < m_limit));
      return m_storage[n];
    }

    access_return_type operator[](size_t n)
    {
      DBG_SMOC(assert(n < m_limit));
      return m_storage[n];
    }
    
    bool tokenIsValid(size_t n) const
    {
      DBG_SMOC(assert(n < m_limit));
      return m_storage[n].isValid();
    }

  private:
    const storage_type  &m_storage;
#ifndef NDEBUG
    size_t         m_limit;
#endif
  };
  /*******************************************************/

  // local storage
  storage_type         m_storage;  
  // and its wrapper object
  read_access_wrapper  m_read_access_wrapper;
};


/******************************************************************************
 * part of smoc_chan_in_if which every transactor has to implement
 *
 */
template<typename T>
class port_in_transactor_if
{
public:
  virtual size_t numAvailable() const = 0;
  virtual smoc_event &dataAvailableEvent(size_t n) = 0;
#ifdef ENABLE_SYSTEMC_VPC
  virtual void commitRead(size_t consume, const smoc_ref_event_p &) = 0;
#else
  virtual void commitRead(size_t consume) = 0;
#endif
};


/******************************************************************************
 * this plug basically wraps smoc_chan_in_if functions to two objects.
 *
 */
template<typename T>
class smoc_port_in_plug :
  public smoc_chan_in_if<T, smoc_channel_access>
{
public:
  typedef smoc_port_in_plug<T>                this_type;
  typedef typename this_type::access_in_type  access_in_type;

  // constructor
  smoc_port_in_plug(smoc_port_in_storage_read_if<T> &storage,
                    port_in_transactor_if<T> &parent_transactor) :
    m_storage(storage),
    m_parent_transactor(parent_transactor)
  {}

  //
  size_t numAvailable(void) const
  {
    DBG_SC_OUT("smoc_port_in_plug::numAvailable():\n");
    return m_parent_transactor.numAvailable();
  }

  //
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("smoc_port_in_plug::dataAvailableEvent():\n");
    return m_parent_transactor.dataAvailableEvent(n);
  }

  //
  access_in_type *getReadChannelAccess(void)
  {
    DBG_SC_OUT("smoc_port_in_plug::getReadChannelAccess():\n");
    return m_storage.getReadChannelAccess();
  }

  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("smoc_port_in_plug::commitRead(): got " << consume
               << " data.\n");
    m_parent_transactor.commitRead(consume);
  }
  
private:
  //
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  smoc_port_in_storage_read_if<T>  &m_storage;
  port_in_transactor_if<T>         &m_parent_transactor;
};


/******************************************************************************
 *
 *
 */
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE = tlm::TLM_PASS_BY_POINTER>
class port_in_transactor :
  public port_in_transactor_if<DATA>,
  public sc_module
{
public:
  SC_HAS_PROCESS(port_in_transactor);

  typedef tlm::tlm_request<ADDRESS, DATA, DATA_MODE>  request_type;
  typedef tlm::tlm_response<DATA, DATA_MODE>          response_type;
  typedef tlm::tlm_annotated_slave_if<request_type,
                                      response_type>  tlm_interface_type;
  typedef sc_port<tlm_interface_type>                 tlm_port_type;

  typedef std::map<size_t, smoc_event *>              event_map_type;

  // tlm slave port
  tlm_port_type tlm_port_in;

  //
  port_in_transactor(sc_module_name name, size_t n = 1) :
    sc_module(name),
    m_read_new_data(true),
    m_storage(n),
    m_smoc_port_in_plug(m_storage, *this)
  {}

  //
  smoc_port_in_plug<DATA> &get_smoc_port_in_plug()
  {
    return m_smoc_port_in_plug;
  }

  //
  void update_data_available_events()
  {
    event_map_type::iterator iter = m_event_map.begin();

    while (iter != m_event_map.end()) {
      if (iter->first > numAvailable())
        iter->second->reset();
      else
        iter->second->notify();

      ++iter;
    }
  }

  /*
   * port_in_transactor_if
   */
  //
  size_t numAvailable(void) const
  {
    return m_storage.num_available();
  }

  // create or return event
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("PortInAdapter::dataAvailableEvent(): n = " << n << std::endl);
    event_map_type::iterator iter = m_event_map.find(n);
    if (iter == m_event_map.end()) {
      const event_map_type::value_type value(n, new smoc_event());
      iter = m_event_map.insert(value).first;

      // FIXME: remove debug code
      DBG_OUT("PortInAdapter::dataAvailableEvent(): create new event\n");

      if (m_storage.num_available() >= n)
        iter->second->notify();
    }
    return *(iter->second);
  }
  
  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("PortInAdapter::commitRead(): consumed " << consume
               << " data.\n");

    assert(m_storage.num_available() >= consume);

    // invalidate data in storage (avoid accidentally reread)
    m_storage.invalidate(consume);

    // skip commited data in storage
    m_storage.skip(consume);

    update_data_available_events();

    // FIXME: maybe this calls update_data_available_events too
    get_request_method();
  }
  
private:
  //
  void end_of_elaboration(void)
  {
    SC_METHOD(get_request_method);
    sensitive << tlm_port_in->ok_to_get();
    dont_initialize();
  }

  //
  void get_request_method(void)
  {
    DBG_SC_OUT("PortInAdapter::request_method()\n");

    if (m_storage.is_full() || !tlm_port_in->nb_can_get())
      return;

    //
    /*if (!tlm_port_in->nb_peek(m_request)) {
      DBG_SC_OUT("PortInAdapter::request_method(): can't nb_peek. Fatal!\n");
      assert(0);
    }*/
    // get data from channel
    if (!tlm_port_in->nb_get(m_request, sc_time(1, SC_PS))) {
      DBG_SC_OUT("PortInAdapter::get_request_method(): can't nb_get data. " \
                 "Fatal!\n");
      assert(0);
    }

    // copy token from channel into local storage
    m_storage.put(m_request.get_address());

    m_response.set_transaction_id(m_request.get_transaction_id());
    // ...

    // send response
    if (!tlm_port_in->nb_put(m_response, sc_time(1, SC_PS))) {
      DBG_SC_OUT("PortInAdapter::commitRead(): can't nb_put. Fatal!\n");
      assert(0);
    }

    update_data_available_events();
  }

  bool                        m_read_new_data;
  smoc_port_in_storage<DATA>  m_storage;
  smoc_port_in_plug<DATA>        m_smoc_port_in_plug;
  request_type                m_request;
  response_type               m_response;
  event_map_type              m_event_map;
};


/******************************************************************************
 * smoc_port_out_plug needs this interface to access storage
 *
 */
template<typename T>
class smoc_port_out_storage_write_if
{
public:
  typedef smoc_channel_access<smoc_storage_wom<T> >  write_access_type;
  typedef typename write_access_type::return_type    access_return_type;

  virtual write_access_type* getWriteChannelAccess(void) = 0;
  virtual bool token_is_valid(size_t n) const = 0;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class smoc_port_out_storage :
  public smoc_port_out_storage_write_if<T>
{
public:
  typedef T  data_type;
  typedef typename smoc_port_out_storage_write_if<T>::write_access_type
    write_access_type;
  typedef typename smoc_port_out_storage_write_if<T>::access_return_type
    access_return_type;
  typedef circular_buffer_ra_free_space<smoc_storage<T> >  storage_type;

  //
  smoc_port_out_storage(const size_t size = 1) :
    m_storage(size),
    m_write_access_wrapper(m_storage)
  {}

  //
  virtual ~smoc_port_out_storage(void) {}

  //
  void invalidate(const size_t n)
  {
    for (size_t i = 0; i < n; ++i)
      m_storage[i].invalidate();
  }

  // wrapper for circular_buffer_ra_write::is_empty()
  bool is_empty(void) const { return m_storage.is_empty(); }

  // wrapper for random_access_circular_buffer::is_full()
  bool is_full(void) const { return m_storage.is_full(); }

  // wrapper for circular_buffer_ra_write::get()
  data_type get(void) { return m_storage.get(); }

  // wrapper for circular_buffer_ra_write::num_free()
  size_t num_free(void) const { return m_storage.num_free(); }

  // wrapper for circular_buffer_ra_write::num_available()
  size_t num_available(void) const { return m_storage.num_available(); }

  // wrapper for circular_buffer_ra_write::commit_ra_write()
  void commit_ra_write(size_t n) { return m_storage.commit_ra_write(n); }

  // wrapper for circular_buffer_ra_write::operator[]
  const data_type& operator[](size_t n) const
  {
    assert(m_storage[n].isValid());
    return m_storage[n];
  }

  /*
   * smoc_port_out_storage_write_if
   */
  write_access_type* getWriteChannelAccess(void)
  {
    return &m_write_access_wrapper;
  }

  //
  bool token_is_valid(size_t n) const { return m_storage[n].isValid(); }

private:
  /********************************************************
   * wraps our storage for smoc_port_out
   */
  class write_access_wrapper :
    public write_access_type
  {
  public:
    write_access_wrapper(storage_type &storage) :
      m_storage(storage)
#ifndef NDEBUG
      , m_limit(0)
#endif
    {}

    /*
     * write_access_type
     */
#ifndef NDEBUG
    // set limit and assert in operator[]
    void setLimit(size_t l) { m_limit = l; }
#endif

    //
    const access_return_type operator[](size_t n) const
    {
      DBG_SMOC(assert(n < m_limit));
      return m_storage[n];
    }

    //
    access_return_type operator[](size_t n)
    {
      DBG_SMOC(assert(n < m_limit));
      return m_storage[n];
    }
    
    //
    bool tokenIsValid(size_t n) const
    {
      DBG_SMOC(assert(n < m_limit));
      return m_storage[n].isValid();
    }

  private:
    storage_type  &m_storage;
#ifndef NDEBUG
    size_t         m_limit;
#endif
  };
  /*******************************************************/

  // local storage
  storage_type          m_storage;
  // and its wrapper object
  write_access_wrapper  m_write_access_wrapper;
};


/******************************************************************************
 * part of smoc_chan_out_if which every transactor has to implement
 *
 */
template<typename T>
class port_out_transactor_if
{
public:
  virtual size_t numFree(void) const = 0;
  virtual smoc_event &spaceAvailableEvent(size_t n) = 0;
#ifdef ENABLE_SYSTEMC_VPC
  virtual void commitWrite(size_t produce, const smoc_ref_event_p &) = 0;
#else
  virtual void commitWrite(size_t produce) = 0;
#endif
};


/******************************************************************************
 * this plug basically wraps smoc_chan_out_if functions to two objects.
 *
 */
template<typename T>
class smoc_port_out_plug :
  public smoc_chan_out_if<T, smoc_channel_access>
{
public:
  typedef smoc_port_out_plug<T>                this_type;
  typedef typename this_type::access_out_type  access_out_type;

  // constructor
  smoc_port_out_plug(smoc_port_out_storage_write_if<T> &storage,
                     port_out_transactor_if<T> &parent_transactor) :
    m_storage(storage),
    m_parent_transactor(parent_transactor)
  {}

  //
  size_t numFree(void) const
  {
    DBG_SC_OUT("smoc_port_out_plug::numFree():\n");
    return m_parent_transactor.numFree();
  }

  //
  smoc_event &spaceAvailableEvent(size_t n)
  {
    DBG_SC_OUT("smoc_port_out_plug::spaceAvailableEvent():\n");
    return m_parent_transactor.spaceAvailableEvent(n);
  }

  //
  access_out_type *getWriteChannelAccess(void)
  {
    DBG_SC_OUT("smoc_port_out_plug::getWriteChannelAccess():\n");
    return m_storage.getWriteChannelAccess();
  }

  //
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("smoc_port_out_plug::commitWrite(): got " << produce
               << " data.\n");

    // check if every comitted token is valid
    for (size_t i = 0; i < produce; ++i)
      assert(m_storage.token_is_valid(i));

    m_parent_transactor.commitWrite(produce);
  }
  
private:
  //
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  smoc_port_out_storage_write_if<T>  &m_storage;
  port_out_transactor_if<T>          &m_parent_transactor;
};
 

/******************************************************************************
 *
 *
 */
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE = tlm::TLM_PASS_BY_POINTER>
class port_out_transactor :
  public port_out_transactor_if<DATA>,
  public sc_module
{
public:
  SC_HAS_PROCESS(port_out_transactor);

  typedef tlm::tlm_request<ADDRESS, DATA, DATA_MODE>   request_type;
  typedef tlm::tlm_response<DATA, DATA_MODE>           response_type;
  typedef tlm::tlm_annotated_master_if<request_type,
                                       response_type>  tlm_interface_type;
  typedef sc_port<tlm_interface_type>                  tlm_port_type;

  typedef std::map<size_t, smoc_event *>               event_map_type;

  // tlm master port
  tlm_port_type tlm_port_out;

  //
  port_out_transactor(sc_module_name name, size_t n = 1) :
    sc_module(name),
    m_storage(n),
    m_smoc_port_out_plug(m_storage, *this),
    m_commited_token(0),
    m_transaction_count(0)
  {}

  //
  smoc_port_out_plug<DATA> &get_smoc_port_out_plug()
  {
    return m_smoc_port_out_plug;
  }

  //
  void update_space_available_events()
  {
    event_map_type::iterator iter = m_event_map.begin();

    while (iter != m_event_map.end()) {
      if (iter->first > numFree())
        iter->second->reset();
      else
        iter->second->notify();

      ++iter;
    }
  }

  /*
   * port_out_transactor_if
   */
  //
  size_t numFree() const
  {
    return m_storage.num_free();
  }

  //
  smoc_event &spaceAvailableEvent(size_t n)
  {
    /* FIXME
    DBG_SC_OUT("PortOutAdapter::spaceAvailableEvent():\n");
    assert(n == 1);
    return m_event_space_available;*/
    DBG_SC_OUT("PortOutAdapter::spaceAvailableEvent(): n = " << n << std::endl);
    event_map_type::iterator iter = m_event_map.find(n);
    if (iter == m_event_map.end()) {
      const event_map_type::value_type value(n, new smoc_event());
      iter = m_event_map.insert(value).first;

      // FIXME: remove debug code
      DBG_OUT("PortOutAdapter::spaceAvailableEvent(): create new event\n");

      if (m_storage.num_free() >= n)
        iter->second->notify();
    }
    return *(iter->second);
  }

  //
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("OutAdapter::commitWrite(): produced " << produce
               << " data.\n");

    m_storage.commit_ra_write(produce);

    update_space_available_events();

    // FIXME: maybe this calls update_space_available_events too
    // trigger send_request_method
    send_request_method();
  }

private:
  //
  void end_of_elaboration(void)
  {
    SC_METHOD(send_request_method);
    sensitive << tlm_port_out->ok_to_put();
    dont_initialize();

    SC_METHOD(get_response_method);
    sensitive << tlm_port_out->ok_to_peek();
    dont_initialize();
  }

  // this method is either triggered by ok_to_put or called by commitWrite()
  void send_request_method(void)
  {
    DBG_SC_OUT("OutAdapter::send_request_method()\n");

    if (!m_storage.is_empty()) {
      if (!tlm_port_out->nb_can_put()) {
        DBG_SC_OUT("OutAdapter::send_request_method(): not ok to put.\n");
        return;
      }

      // FIXME: remove debug code
      DBG_DOBJ(m_transaction_count);
      DBG_DOBJ(m_storage.num_available());

      m_request.set_command(tlm::WRITE);
      m_request.set_address(m_storage.get());
      m_request.set_transaction_id(++m_transaction_count);
      // ...

      // FIXME: remove debug code
      DBG_DOBJ(m_transaction_count);
      DBG_DOBJ(m_storage.num_available());

      if (!tlm_port_out->nb_put(m_request, sc_time(1, SC_PS))) {
        DBG_SC_OUT("OutAdapter::send_request_method(): nb_put failed!\n");
        assert(0);
      }
    }
    else {
      DBG_SC_OUT("OutAdapter::send_request_method(): nothing to send.\n");
    }

    update_space_available_events();
  }

  //
  void get_response_method(void)
  {
    DBG_SC_OUT("OutAdapter::get_response_method()\n");

    // we can peek

    if (!tlm_port_out->nb_get(m_response, sc_time(1, SC_PS))) {
      DBG_SC_OUT("OutAdapter::get_response_method(): can't get. Fatal!\n");
      assert(0);
    }
    DBG_DOBJ(m_response.get_transaction_id());
  }

  smoc_port_out_storage<DATA>  m_storage;
  smoc_port_out_plug<DATA>        m_smoc_port_out_plug;
  request_type                 m_request;
  response_type                m_response;
  int                          m_commited_token;
  int                          m_transaction_count;
  event_map_type               m_event_map;
};
 

#include "debug_on.h"


/******************************************************************************
 *
 *
 */
class A1 :
  public smoc_actor
{
public:
  smoc_port_out<int> out1;
  smoc_port_in<int>  in1;
  //smoc_port_out<int> out2;

  A1(sc_module_name name) :
    smoc_actor(name, start),
    m_fire_counter(0)
  {
    start =
        // a1
        (in1(1) && GUARD(A1::check_red))  >>
        out1(1)                           >>
        CALL(A1::func_1)                  >> start  |
        (in1(2) && GUARD(A1::check_red))  >>
        out1(2)                           >>
        CALL(A1::func_2)                  >> start;
  }

private:
  smoc_firing_state start;
  int               m_fire_counter;

  bool check_red(void) const
  {
    if (m_fire_counter < MAX_FIRE)
      return true;
    else
      return false;
  }
  
  bool check_blue(void) const { return true; }

  void func_1(void)
  {
    DBG_SC_OUT("A1::func_1(): bounce " << in1[0] << std::endl);
    out1[0] = in1[0];
    ++m_fire_counter;
  }

  void func_2(void)
  {
    DBG_SC_OUT("A1::func_2(): bounce x 2 " << in1[0] << std::endl);
    out1[0] = in1[0];
    out1[1] = in1[0];
  }
};


/******************************************************************************
 *
 *
 */
class A2 :
  public smoc_actor
{
public:
  smoc_port_out<int> out1;
  smoc_port_in<int>  in1;
  //smoc_port_in<int>  in2;

  A2(sc_module_name name) :
    smoc_actor(name, start)
  {
    start =
        // a1
        //(in1(1) && in2(1) && GUARD(A2::check_red))  >>
        (in1(1) && GUARD(A2::check_red))  >>
        out1(2)                                     >>
        CALL(A2::func_1)                           >> two;
    two =
        // a1
        //(in1(1) && in2(1) && GUARD(A2::check_red))  >>
        (in1(2) && GUARD(A2::check_red))  >>
        out1(1)                                     >>
        CALL(A2::func_2)                           >> start;
  }

private:
  smoc_firing_state start;
  smoc_firing_state two;

  bool check_red(void) const { return true; }
  
  bool check_blue(void) const { return true; }

  void func_1(void)
  {
    DBG_SC_OUT("A2::func_1(): read " << in1[0] << std::endl);
    DBG_SC_OUT("A2::func_1(): write x 2 " << in1[0] + 2 << std::endl);
    out1[0] = in1[0] + 2;
    out1[1] = in1[0] + 2;
  }

  void func_2(void)
  {
    DBG_SC_OUT("A2::func_2(): read " << in1[0] << std::endl);
    DBG_SC_OUT("A2::func_2(): write " << in1[0] + in1[1] << std::endl);
    out1[0] = in1[0] + in1[1];
  }
};


#if 0
/******************************************************************************
 *
 *
 */
class Schedule_tester :
  public smoc_graph
{
public:
  // bind every transactor to port from Schedule_tester
  sc_fifo_out<int> outAdapter_out;
  sc_fifo_in<int> inAdapter_in;

  Schedule_tester(sc_module_name name) :
    smoc_graph(name),
    a1("A1"),
    a2("A2"),
    outAdapter("outAdapter"),
    inAdapter("inAdapter")
  {
    // connect intern SysteMoC channels
    connectNodePorts(a1.out1, a2.in1, smoc_fifo<int>(8) << 42);
    connectNodePorts(a2.out1, a1.in1, smoc_fifo<int>(8));

    // connect transactor to SysteMoC ports still unbound
    connectChanPort(outAdapter.get_smoc_port_out_plug(), a1.out2);
    connectChanPort(inAdapter.get_smoc_port_in_plug(), a2.in2);

    // bind transactor to this module's ports
    outAdapter.fifo_out.bind(outAdapter_out);
    inAdapter.fifo_in.bind(inAdapter_in);
  }
private:
  A1 a1;
  A2 a2;
  PortOutToFifoAdapter<int> outAdapter;
  PortInToFifoAdapter<int>  inAdapter;
};
#endif


/******************************************************************************
 *
 *
 */
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE = tlm::TLM_PASS_BY_POINTER>
class Tlm_tester :
  public smoc_graph
{
public:
  typedef port_out_transactor<ADDRESS, DATA, DATA_MODE>  out_transactor_type;
  typedef typename out_transactor_type::tlm_port_type    port_out_type;
  typedef port_in_transactor<ADDRESS, DATA, DATA_MODE>   in_transactor_type;
  typedef typename in_transactor_type::tlm_port_type     port_in_type;

#ifdef USE_TRANSACTOR
  port_out_type outTransactor_out;
  port_in_type inTransactor_in;
#endif

  //
  Tlm_tester(sc_module_name name) :
    smoc_graph(name),
    a1("A1"),
    a2("A2")
#ifdef USE_TRANSACTOR
    ,
    outTransactor("outTransactor", 2),
    inTransactor("inTransactor", 2)
#endif
  {
    // connect intern SysteMoC channels
    //connectNodePorts(a1.out1, a2.in1, smoc_fifo<int>(8) << 42);
    connectNodePorts(a2.out1, a1.in1, smoc_fifo<int>(8) << 42);

#ifdef USE_TRANSACTOR
    // connect transactor to SysteMoC ports still unbound
    connectChanPort(outTransactor.get_smoc_port_out_plug(), a1.out1);
    connectChanPort(inTransactor.get_smoc_port_in_plug(), a2.in1);

    // bind transactor to this module's ports
    outTransactor.tlm_port_out.bind(outTransactor_out);
    inTransactor.tlm_port_in.bind(inTransactor_in);
#else
    // use normal smoc_fifo
    connectNodePorts(a1.out1, a2.in1, smoc_fifo<int>(4));
#endif
  }

private:
  A1 a1;
  A2 a2;
#ifdef USE_TRANSACTOR
  out_transactor_type outTransactor;
  in_transactor_type  inTransactor;
#endif
};


/******************************************************************************
 *
 *
 */
SC_MODULE(Dumper)
{
  void dump(void)
  {
    while (1) {
      DBG_SC_OUT("dump:dump()\n");
      wait(1, SC_PS);
    }
  }

  SC_CTOR(Dumper)
  {
    SC_THREAD(dump);
  }
};


/*****************************************************************************/


int sc_main (int argc, char **argv)
{
  typedef unsigned int address_type;
  typedef int data_type;

#ifdef PASS_BY_COPY
  const tlm::tlm_data_mode data_mode = tlm::TLM_PASS_BY_COPY ;
#else
  const tlm::tlm_data_mode data_mode = tlm::TLM_PASS_BY_POINTER ;
#endif

  typedef tlm::tlm_request<address_type,
                           data_type,
                           data_mode>             request_type;
  typedef tlm::tlm_response<data_type,data_mode>  response_type;

  typedef tlm_pvt_annotated_fifo<request_type>    request_fifo_type;
  typedef tlm_pvt_annotated_fifo<response_type>   response_fifo_type;
  typedef tlm::tlm_annotated_req_rsp_channel<
    request_type,
    response_type,
    request_fifo_type,
    response_fifo_type>                           channel_type;

  //Dumper dumper("dumper");

  smoc_top_moc<Tlm_tester<address_type, data_type, data_mode> >
    tlm_tester("tlm_tester");

#ifdef USE_TRANSACTOR
  channel_type channel("channel",1,-1); // unbounded response fifo required

  tlm_tester.outTransactor_out(channel.master_export);
  tlm_tester.inTransactor_in(channel.slave_export);
#endif

#ifndef KASCPAR_PARSING  
  if (argc > 1 && 0 == strncmp(argv[1],
                               "--generate-problemgraph",
                               sizeof("--generate-problemgraph")))
  {
    smoc_modes::dump(std::cout, tlm_tester);
  }
  else {
    sc_start(50, SC_PS);
  }
#endif  
  return 0;
}


