
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

#include "debug_on.h"

#include "tlm.h"

#include "tlm_pvt_annotated_fifo.h"

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


/******************************************************************************
 *
 */
template<typename T>
class circular_buffer
{
public:
  // FIXME: T has to be default constructable. Concept check this!
  typedef T                           data_type;
  typedef circular_buffer<data_type>  this_type;

  //
  circular_buffer(const int size = 1) :
    m_buffer(size), // fill buffer with default constructed objects
    m_size(size),
    m_read(0),
    m_write(0),
    m_num_data(0)
  {}

  // buffer size
  size_t get_size(void) const { return m_size; }

  // free slots
  size_t num_free(void) const { return m_size - m_num_data; }

  // available data
  size_t num_available(void) const { return m_num_data; }

  //
  bool is_empty(void) const { return m_num_data == 0; }

  //
  bool is_full(void) const { return ((m_write == m_read) && !is_empty()); }

  //
  void put(const data_type &d)
  {
    DBG_OUT("put(): num_data: " << m_num_data << std::endl);
    assert(!is_full());

    m_buffer[m_write] = d;
    m_write = (m_write + 1) % m_size;
    ++m_num_data;
  }

  //
  data_type get(void)
  {
    assert(!is_empty());
    DBG_OUT("get(): num_data: " << m_num_data << std::endl);

    const size_t ret = m_read;
    m_read = (m_read + 1) % m_size;
    --m_num_data;
    DBG_OUT("get(): num_data: " << m_num_data << std::endl);
    return m_buffer[ret];
  }

  // skip n data without getting it
  void skip(const size_t n)
  {
    //FIXME: remove debug code
    DBG_OUT("skip(): num_data: " << m_num_data << std::endl);
    assert(n <= m_num_data);
    m_read = (m_read + n) % m_size;
    m_num_data -= n;
  }

protected:
  size_t get_read(void) const { return m_read; }
  size_t get_write(void) const { return m_write; }
  
  std::vector<data_type>  m_buffer;

  const size_t            m_size;
  size_t                  m_read;
  size_t                  m_write;
  size_t                  m_num_data;
};


/******************************************************************************
 *
 */
template<typename T>
class circular_buffer_ra_read :
  public circular_buffer<T>
{
  private:
  typedef circular_buffer<T> base_type;

public:
  typedef typename circular_buffer<T>::data_type  data_type;

  //
  circular_buffer_ra_read(const size_t n) :
    circular_buffer<T>(n)
  {}

  // random access data in buffer
  const data_type &operator[](size_t n) const
  {
    assert(n < base_type::num_available());
    const size_t pos = (base_type::get_read() + n) % base_type::get_size();
    return base_type::m_buffer[pos];
  }

private:
  // disable
  data_type get(void) {}

  // disable 
  data_type &operator[](size_t n) {}
};


/*****************************************************************************/


/*
 *
 */
template<typename T>
void set_all_vec_elements(std::vector<T> &vec, const T t)
{
  typedef std::vector<T> vector_type;
  typedef typename vector_type::iterator iter_type;

  for (iter_type iter = vec.begin(); iter != vec.end(); ++iter)
    *iter = t;
}


/*
 *
 */
template<typename T>
bool vec_element_equals(typename std::vector<T>::const_iterator iter,
                        typename std::vector<T>::const_iterator end,
                        const T &t)
{
  for (; iter != end; ++iter)
    if (*iter == t)
      return true;

  return false;
}


/******************************************************************************
 *
 */
template<typename T>
class circular_buffer_ra_write :
  public circular_buffer<T>
{
private:
  typedef circular_buffer<T> base_type;
  typedef std::vector<bool>  bitvector_type;

public:
  typedef circular_buffer_ra_write<T>             this_type;
  typedef typename circular_buffer<T>::data_type  data_type;

  //
  circular_buffer_ra_write(const size_t n) :
    circular_buffer<T>(n),
    m_written_flags(n, false) // TODO: maybe to big
  {}

  // random access data in buffer
  const data_type &operator[](size_t n) const
  {
    assert(n < base_type::num_free());
    const size_t pos = (base_type::m_write + n) % base_type::m_size;
    return base_type::m_buffer[pos];
  }

  // use const version to avoid code duplication
  data_type &operator[](size_t n)
  {
    m_written_flags[n] = true;
    return const_cast<data_type&>(static_cast<const this_type&>(*this)[n]);
  }

  //
  void commit_ra_write(const size_t n)
  {
    bitvector_type::const_iterator iter = m_written_flags.begin();

    assert(n <= base_type::num_free());
    
    // every commited token should be written
    for (size_t i = n; i > 0; --i) {
      assert(iter != m_written_flags.end());
      assert(*iter == true);
      ++iter;
    }

    // no other token should be written
    if (vec_element_equals(iter, m_written_flags.end(), true)) {
      assert(0);
    }

    base_type::m_write = (base_type::m_write + n) % base_type::m_size;
    base_type::m_num_data += n;

    // FIXME: set only needed
    set_all_vec_elements(m_written_flags, false);
  }

private:
  // disable
  void put(const data_type &d) {}
  
  bitvector_type m_written_flags;
};


#if 0
/******************************************************************************
 * This circular buffer is fun. You can alter its data.
 * ENABLE_AHEAD lets you choose to enable random-access-ahead-of-read-pointer-
 * code. This is usefull to buffer commited data but continue writing into
 * circular buffer.
 *
 */
template<typename T, bool ENABLE_AHEAD = false>
class random_access_circular_buffer
{
public:
  // FIXME: T has to be default constructable. Concept check this!
  typedef T                                         data_type;
  typedef random_access_circular_buffer<data_type>  this_type;

  //
  random_access_circular_buffer(const int size = 1) :
    m_size(size),
    m_read(0),
    m_write(0),
    m_num_data(0),
    m_ra_ahead(0),
    m_buffer(size) // fill buffer with default constructed objects
  {}

  // buffer size
  size_t get_size(void) const { return m_size; }

  // free slots
  size_t num_free(void) const { return m_size - m_num_data; }

  // available data
  size_t num_available(void) const { return m_num_data; }

  //
  void set_ra_ahead(const size_t n)
  {
    if (ENABLE_AHEAD) {
      assert(n < m_num_data);
      m_ra_ahead = n;
    }
    else {
      assert(!"read ahead code should be enabled!");
    }
  }

  //
  void decr_ra_ahead(const size_t n = 1)
  {
    assert(m_ra_ahead >= n);
    m_ra_ahead -= n;
  }

  //
  void incr_ra_ahead(const size_t n = 1) { m_ra_ahead += n; }

  //
  bool is_empty(void) const { return m_num_data == 0; }

  //
  bool is_full(void) const { return ((m_write == m_read) && !is_empty()); }

  // peek
  const data_type &operator[](size_t n) const
  {
    assert(!is_empty());

    size_t  pos;
 
    if (ENABLE_AHEAD) {
      assert((n + m_ra_ahead) < m_num_data);
      pos = (m_read + n + m_ra_ahead) % m_size;
    }
    else {
      assert(n < m_num_data);
      pos = (m_read + n) % m_size;
    }
    return m_buffer[pos];
  }

  //
  data_type &operator[](size_t n)
  {
    // use const version to avoid code duplication
    return const_cast<data_type&>(static_cast<const this_type&>(*this)[n]);
  }

  //
  void put(const data_type &d)
  {
    assert(!is_full());

    m_buffer[m_write] = d;
    m_write = (m_write + 1) % m_size;
    ++m_num_data;
  }

  //
  data_type get(void)
  {
    assert(!is_empty());

    const size_t ret = m_read;
    m_read = (m_read + 1) % m_size;
    --m_num_data;
    return m_buffer[ret];
  }

  // skip n data without getting it (needed for random access read)
  void skip(const size_t n)
  {
    assert(n <= m_num_data);
    m_read = (m_read + n) % m_size;
    m_num_data -= n;
  }

  // this function is ugly! it only bends write pointer to reserve space for
  //  random access and leaves token unaltered which means you can read old
  //  data values accidently. This must be guarded at some higher level or we
  //  add some kind of valid vector TODO
  //  (problem is to move m_write. better idea is some ra_write_ahead and move
  //  m_write after random access write. put() will be failure und invalid data
  //  will be failure, too.
  void reserve(const size_t n)
  {
    assert(n < m_num_data + m_size);
    m_write = (m_write + n) % m_size;
    m_num_data += n;
  }

private:
  const size_t            m_size;
  size_t                  m_read;
  size_t                  m_write;
  size_t                  m_num_data;
  size_t                  m_ra_ahead;
  std::vector<data_type>  m_buffer;
};
#endif


/******************************************************************************
 * SmocPortInPlug needs this interface to access storage
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
  typedef circular_buffer_ra_read<smoc_storage<T> >  storage_type;

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
    // FIXME
    /*for (size_t i = 0; i < n; ++i)
      m_storage[i].invalidate();*/
  }

  // wrapper for random_access_circular_buffer::skip()
  void skip(const size_t n) { m_storage.skip(n); }

  // wrapper for random_access_circular_buffer::is_empty()
  bool is_empty(void) const { return m_storage.is_empty(); }

  // wrapper for random_access_circular_buffer::is_full()
  bool is_full(void) const { return m_storage.is_full(); }

  // wrapper for random_access_circular_buffer::put()
  void put(const T& t) { m_storage.put(t); }

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
 * part of smoc_chan_in_if which every adapter has to implement
 *
 */
template<typename T>
class PortInAdapterIf
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
class SmocPortInPlug :
  public smoc_chan_in_if<T, smoc_channel_access>
{
public:
  typedef SmocPortInPlug<T>                   this_type;
  typedef typename this_type::access_in_type  access_in_type;

  // constructor
  SmocPortInPlug(smoc_port_in_storage_read_if<T> &storage,
                 PortInAdapterIf<T> &parent_adapter) :
    m_storage(storage),
    m_parent_adapter(parent_adapter)
  {}

  //
  size_t numAvailable(void) const
  {
    DBG_SC_OUT("SmocPortInPlug::numAvailable():\n");
    return m_parent_adapter.numAvailable();
  }

  //
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("SmocPortInPlug::dataAvailableEvent():\n");
    return m_parent_adapter.dataAvailableEvent(n);
  }

  //
  access_in_type *getReadChannelAccess(void)
  {
    DBG_SC_OUT("SmocPortInPlug::getReadChannelAccess():\n");
    return m_storage.getReadChannelAccess();
  }

  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("SmocPortInPlug::commitRead(): got " << consume
               << " data.\n");
    assert(consume == 1);
    m_parent_adapter.commitRead(consume);
  }
  
private:
  //
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  smoc_port_in_storage_read_if<T>  &m_storage;
  PortInAdapterIf<T>               &m_parent_adapter;
};


/******************************************************************************
 *
 *
 */
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE = tlm::TLM_PASS_BY_POINTER>
class PortInToTlmAdapter :
  public PortInAdapterIf<DATA>,
  public sc_module
{
public:
  SC_HAS_PROCESS(PortInToTlmAdapter);

  typedef tlm::tlm_request<ADDRESS, DATA, DATA_MODE>  request_type;
  typedef tlm::tlm_response<DATA, DATA_MODE>          response_type;
  typedef tlm::tlm_annotated_slave_if<request_type,
                                      response_type>  tlm_interface_type;
  typedef sc_port<tlm_interface_type>                 tlm_port_type;

  // tlm slave port
  tlm_port_type tlm_port_in;

  //
  PortInToTlmAdapter(sc_module_name name, size_t n = 1) :
    sc_module(name),
    m_read_new_data(true),
    m_storage(n),
    m_smoc_port_in_plug(m_storage, *this),
    m_num_available(0)
  {}

  //
  SmocPortInPlug<DATA> &get_smoc_port_in_plug() { return m_smoc_port_in_plug; }

  /*
   * PortInAdapterIf
   */
  //
  size_t numAvailable(void) const
  {
    return m_num_available;
  }

  //
  smoc_event &dataAvailableEvent(size_t n)
  {
    // FIXME
    DBG_SC_OUT("PortInAdapter::dataAvailableEvent():\n");
    assert(n == 1);
    return m_event_data_available;
  }
  
  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("PortInAdapter::commitRead(): consumed " << consume
               << " data.\n");

    assert(consume == 1);

    assert(m_num_available >= (int)consume);
    m_num_available -= consume;

    // invalidate data in storage (avoid accidentally reread)
    m_storage.invalidate(consume);

    // get peeked data from channel
    if (!tlm_port_in->nb_get(m_request, sc_time(1, SC_PS))) {
      DBG_SC_OUT("PortInAdapter::commitRead(): can't nb_get data. Fatal!\n");
      assert(0);
    }

    m_response.set_transaction_id(m_request.get_transaction_id());
    // ...

    // send response
    if (!tlm_port_in->nb_put(m_response, sc_time(1, SC_PS))) {
      DBG_SC_OUT("PortInAdapter::commitRead(): can't nb_put. Fatal!\n");
      assert(0);
    }

    // skip commited data in storage
    m_storage.skip(consume);

    if (m_storage.is_empty())
      m_event_data_available.reset();
  }
  
private:
  //
  void end_of_elaboration(void)
  {
    SC_METHOD(get_request_method);
    sensitive << tlm_port_in->ok_to_peek();
    dont_initialize();
  }

  //
  void get_request_method(void)
  {
    DBG_SC_OUT("PortInAdapter::request_method()\n");

    // for now we just peek the data, copy it to storage, and get in in
    //  commitRead().
    if (!tlm_port_in->nb_peek(m_request)) {
      DBG_SC_OUT("PortInAdapter::request_method(): can't nb_peek. Fatal!\n");
      assert(0);
    }

    // FIXME: remove debug code
    DBG_DOBJ(m_request.get_transaction_id());

    m_num_available += 1;

    // copy token from channel into local storage
    m_storage.put(m_request.get_address());
    m_event_data_available.notify();
  }

  bool                        m_read_new_data;
  smoc_event                  m_event_data_available;
  smoc_port_in_storage<DATA>  m_storage;
  SmocPortInPlug<DATA>        m_smoc_port_in_plug;
  request_type                m_request;
  response_type               m_response;
  int                         m_num_available;
};


/******************************************************************************
 *
 * Adapter sc_fifo -> smoc_port_in
 */
template<typename T>
class PortInToFifoAdapter :
  public PortInAdapterIf<T>,
  public sc_module
{
public:
  SC_HAS_PROCESS(PortInToFifoAdapter);

  sc_fifo_in<T> fifo_in;

  //
  PortInToFifoAdapter(sc_module_name name, size_t n = 1) :
    sc_module(name),
    m_read_new_data(true),
    m_storage(n),
    m_smoc_port_in_plug(m_storage, *this)
  {}

  //
  SmocPortInPlug<T> &get_smoc_port_in_plug() { return m_smoc_port_in_plug; }

  /*
   * PortInAdapterIf
   */
  //
  size_t numAvailable(void) const
  {
    // FIXME
    return fifo_in.num_available() + 1;
  }

  //
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("PortInAdapter::dataAvailableEvent():\n");
    assert(n == 1);
    return m_event_data_available;
  }
  
  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("PortInAdapter::commitRead(): consumed " << consume
               << " data.\n");

    assert(consume == 1);

    // TODO: invalidate data

    if (fifo_in.num_available() == 0)
      m_event_data_available.reset();

    m_read_new_data = true;
    check_fifo_data_method();
  }
  
private:
  //
  void end_of_elaboration(void)
  {
    SC_METHOD(check_fifo_data_method);
    sensitive << fifo_in.data_written_event();
  }

  //
  void check_fifo_data_method(void)
  {
    if ((fifo_in.num_available() > 0) && m_read_new_data) {
      m_storage.setPos(0, fifo_in.read());
      m_read_new_data = false;
      m_event_data_available.notify();
    }
  }

  bool                     m_read_new_data;
  smoc_event               m_event_data_available;
  smoc_port_in_storage<T>  m_storage;
  SmocPortInPlug<T>        m_smoc_port_in_plug;
};


/******************************************************************************
 * SmocPortOutPlug needs this interface to access storage
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
  typedef T                                           data_type;
  typedef typename smoc_port_out_storage_write_if<T>::write_access_type
    write_access_type;
  typedef typename smoc_port_out_storage_write_if<T>::access_return_type
    access_return_type;
  typedef circular_buffer_ra_write<smoc_storage<T> >  storage_type;

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
    /*for (size_t i = 0; i < n; ++i)
      m_storage[i].invalidate();
      */
    //FIXME
  }

  // wrapper for circular_buffer_ra_write::skip()
  void skip(const size_t n) { m_storage.skip(n); }

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
 * part of smoc_chan_out_if which every adapter has to implement
 *
 */
template<typename T>
class PortOutAdapterIf
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
class SmocPortOutPlug :
  public smoc_chan_out_if<T, smoc_channel_access>
{
public:
  typedef SmocPortOutPlug<T>                   this_type;
  typedef typename this_type::access_out_type  access_out_type;

  // constructor
  SmocPortOutPlug(smoc_port_out_storage_write_if<T> &storage,
                  PortOutAdapterIf<T> &parent_adapter) :
    m_storage(storage),
    m_parent_adapter(parent_adapter)
  {}

  //
  size_t numFree(void) const
  {
    DBG_SC_OUT("SmocPortOutPlug::numFree():\n");
    return m_parent_adapter.numFree();
  }

  //
  smoc_event &spaceAvailableEvent(size_t n)
  {
    DBG_SC_OUT("SmocPortOutPlug::spaceAvailableEvent():\n");
    return m_parent_adapter.spaceAvailableEvent(n);
  }

  //
  access_out_type *getWriteChannelAccess(void)
  {
    DBG_SC_OUT("SmocPortOutPlug::getWriteChannelAccess():\n");
    return m_storage.getWriteChannelAccess();
  }

  //
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("SmocPortOutPlug::commitWrite(): got " << produce
               << " data.\n");
    assert(produce == 1);

    // check if every comitted token is valid
    for (size_t i = 0; i < produce; ++i)
      assert(m_storage.token_is_valid(i));

    m_parent_adapter.commitWrite(produce);
  }
  
private:
  //
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  smoc_port_out_storage_write_if<T>  &m_storage;
  PortOutAdapterIf<T>                &m_parent_adapter;
};
 

/******************************************************************************
 *
 *
 */
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE = tlm::TLM_PASS_BY_POINTER>
class PortOutToTlmAdapter :
  public PortOutAdapterIf<DATA>,
  public sc_module
{
public:
  SC_HAS_PROCESS(PortOutToTlmAdapter);

  typedef tlm::tlm_request<ADDRESS, DATA, DATA_MODE>   request_type;
  typedef tlm::tlm_response<DATA, DATA_MODE>           response_type;
  typedef tlm::tlm_annotated_master_if<request_type,
                                       response_type>  tlm_interface_type;
  typedef sc_port<tlm_interface_type>                  tlm_port_type;

  // tlm master port
  tlm_port_type tlm_port_out;

  //
  PortOutToTlmAdapter(sc_module_name name, size_t n = 1) :
    sc_module(name),
    m_storage(n),
    m_smoc_port_out_plug(m_storage, *this),
    m_commited_token(0),
    m_transaction_count(0)
  {}

  //
  SmocPortOutPlug<DATA> &get_smoc_port_out_plug()
  {
    return m_smoc_port_out_plug;
  }

  /*
   * PortOutAdapterIf
   */
  //
  size_t numFree() const
  {
    return m_storage.num_free();
  }

  //
  smoc_event &spaceAvailableEvent(size_t n)
  {
    // FIXME
    DBG_SC_OUT("PortOutAdapter::spaceAvailableEvent():\n");
    assert(n == 1);
    return m_event_space_available;
  }

  //
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("OutAdapter::commitWrite(): produced " << produce
               << " data.\n");

    assert(produce == 1);

    m_storage.commit_ra_write(produce);

    if (m_storage.is_full())
      m_event_space_available.reset();

    // trigger send_request_method
    send_request_method();
  }

private:
  //
  void end_of_elaboration(void)
  {
    // signal space availability to smoc_port
    if (!m_storage.is_full())
      m_event_space_available.notify();

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

    // let client write to buffer
    if (!m_storage.is_full()) {
      // FIXME
      m_event_space_available.notify();
    }
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

  smoc_event                   m_event_space_available;
  smoc_port_out_storage<DATA>  m_storage;
  SmocPortOutPlug<DATA>        m_smoc_port_out_plug;
  request_type                 m_request;
  response_type                m_response;
  int                          m_commited_token;
  int                          m_transaction_count;
};
 

/******************************************************************************
 *
 *
 */
template<typename T>
class PortOutToFifoAdapter :
  public PortOutAdapterIf<T>,
  public sc_module
{
public:
  SC_HAS_PROCESS(PortOutToFifoAdapter);

  sc_fifo_out<T> fifo_out;

  //
  PortOutToFifoAdapter(sc_module_name name, size_t n = 1) :
    sc_module(name),
    m_storage(n),
    m_smoc_port_out_plug(m_storage, *this)
  {}

  //
  SmocPortOutPlug<T> &get_smoc_port_out_plug() { return m_smoc_port_out_plug; }

  /*
   * PortOutAdapterIf
   */
  //
  size_t numFree() const { return fifo_out.num_free();; }

  //
  smoc_event &spaceAvailableEvent(size_t n)
  {
    DBG_SC_OUT("PortOutAdapter::spaceAvailableEvent():\n");
    assert(n == 1);
    return m_event_space_available;
  }

  //
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("OutAdapter::commitWrite(): got " << produce << " data.\n");

    assert(produce == 1);
    assert(fifo_out.num_free() >= (int)produce);

    fifo_out.write(m_storage[0]);
    wait(1, SC_PS);

    if (fifo_out.num_free() == 0)
      m_event_space_available.reset();
  }

private:
  //
  void end_of_elaboration(void)
  {
    if (fifo_out.num_free() > 0)
      m_event_space_available.notify();

    SC_METHOD(check_fifo_space_method);
    sensitive << fifo_out.data_read_event();
  }

  //
  void check_fifo_space_method(void)
  {
    assert(fifo_out.num_free() > 0); 
    m_event_space_available.notify();
  }

  smoc_event                m_event_space_available;
  smoc_port_out_storage<T>  m_storage;
  SmocPortOutPlug<T>        m_smoc_port_out_plug;
};


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
    smoc_actor(name, start)
  {
    start =
        // a1
        (in1(1) && GUARD(A1::check_red))  >>
        out1(1)                           >>
        //(out1(1) && out2(1))              >>
        CALL(A1::func_a1)                 >> start;
  }

private:
  smoc_firing_state start;

  bool check_red(void) const { return true; }
  
  bool check_blue(void) const { return true; }

  void func_a1(void)
  {
    DBG_SC_OUT("A1::func_a1(): bounce " << in1[0] << std::endl);
    out1[0] = in1[0];
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
        out1(1)                                     >>
        CALL(A2::func_a1)                           >> start;
  }

private:
  smoc_firing_state start;

  bool check_red(void) const { return true; }
  
  bool check_blue(void) const { return true; }

  void func_a1(void)
  {
    DBG_SC_OUT("A2::func_a1(): read " << in1[0] << std::endl);
    DBG_SC_OUT("A2::func_a1(): write " << in1[0] + 2 << std::endl);
    out1[0] = in1[0] + 2;
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
  // bind every adapter to port from Schedule_tester
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

    // connect adapter to SysteMoC ports still unbound
    connectChanPort(outAdapter.get_smoc_port_out_plug(), a1.out2);
    connectChanPort(inAdapter.get_smoc_port_in_plug(), a2.in2);

    // bind adapter to this module's ports
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
  typedef PortOutToTlmAdapter<ADDRESS, DATA, DATA_MODE>  out_adapter_type;
  typedef typename out_adapter_type::tlm_port_type       port_out_type;
  typedef PortInToTlmAdapter<ADDRESS, DATA, DATA_MODE>   in_adapter_type;
  typedef typename in_adapter_type::tlm_port_type        port_in_type;

  port_out_type outAdapter_out;
  port_in_type inAdapter_in;

  //
  Tlm_tester(sc_module_name name) :
    smoc_graph(name),
    a1("A1"),
    a2("A2"),
    outAdapter("outAdapter"),
    inAdapter("inAdapter")
  {
    // connect intern SysteMoC channels
    //connectNodePorts(a1.out1, a2.in1, smoc_fifo<int>(8) << 42);
    connectNodePorts(a2.out1, a1.in1, smoc_fifo<int>(8) << 42);

    // connect adapter to SysteMoC ports still unbound
    connectChanPort(outAdapter.get_smoc_port_out_plug(), a1.out1);
    connectChanPort(inAdapter.get_smoc_port_in_plug(), a2.in1);

    // bind adapter to this module's ports
    outAdapter.tlm_port_out.bind(outAdapter_out);
    inAdapter.tlm_port_in.bind(inAdapter_in);
  }
private:
  A1 a1;
  A2 a2;
  out_adapter_type outAdapter;
  in_adapter_type  inAdapter;
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

  channel_type channel("channel",1,-1); // unbounded response fifo required

  //Dumper dumper("dumper");

  smoc_top_moc<Tlm_tester<address_type, data_type, data_mode> >
    tlm_tester("tlm_tester");

  tlm_tester.outAdapter_out(channel.master_export);
  tlm_tester.inAdapter_in(channel.slave_export);
  //fifo.write(13);

#ifndef KASCPAR_PARSING  
  if (argc > 1 && 0 == strncmp(argv[1],
                               "--generate-problemgraph",
                               sizeof("--generate-problemgraph")))
  {
    smoc_modes::dump(std::cout, tlm_tester);
  }
  else {
    sc_start(10, SC_PS);
  }
#endif  
  return 0;
}


