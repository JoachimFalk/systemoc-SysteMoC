

#include <iostream>
#include <string>
#include <map>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

#include "tlm.h"
#include "tlm_pvt_annotated_fifo.h"

#include "circular_buffer.h"
#include "debug_on.h"

// SysteMoC uses different debugging style: if NDEBUG is set, debugging code is
//  disabled. Define macro for NDEBUG undefed code.
#ifndef NDEBUG
  #define DBG_SMOC(e) do {e;} while(0)
#else
  #define DBG_SMOC(e) do {} while(0)
#endif



/******************************************************************************
 * get and put data in bytewise manner. This makes it independent of concrete
 * data type.
 *
 */
class StorageByteAccessIf
{
public:
  typedef unsigned char byteType;
  
  // storage is empty?
  virtual bool isEmpty(void) const = 0;

  // storage has data available?
  virtual bool isFree(void) const = 0;

  // not virtual to prevent bypassing of test
  size_t getBytes(byteType *a, const size_t n)
  {
    if (isEmpty() || (n != dataSize()))
      return 0;

    return getBytesImpl(a, dataSize());
  }

  //
  size_t putBytes(byteType *a, const size_t n)
  {
    // FIXME: remove debug code
    DBG_OUT("StorageByteAccessIf::putBytes()\n");
    if (!isFree() || (n != dataSize()))
      return 0;

    return putBytesImpl(a, dataSize());
  }

private:
  //
  virtual size_t getBytesImpl(byteType *a, const size_t n) = 0;

  //
  virtual size_t putBytesImpl(byteType *a, const size_t n) = 0;

  //
  virtual size_t dataSize(void) const = 0;
};


/******************************************************************************
 * smoc_port_in_plug needs this interface to access storage
 *
 */
template<typename T>
class SmocPortInStorageReadIf
{
public:
  typedef smoc_channel_access<const T&>         readAccessType;
  typedef typename readAccessType::return_type  accessReturnType;//const T&

  //
  virtual readAccessType* getReadChannelAccess(void) = 0;
};


/******************************************************************************
 *
 *
 */
class StorageTransactorIf
{
public:
  //
  virtual size_t numAvailable(void) const = 0;

  //
  virtual void skip(const size_t n) = 0;

  //
  virtual void invalidate(const size_t n) = 0;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class SmocPortInStorage :
  public SmocPortInStorageReadIf<T>, // interface for smoc port
  public StorageByteAccessIf,        // interface for PlugAggregation
  public StorageTransactorIf         // interface for PortInTransactor
{
public:
  typedef typename SmocPortInStorageReadIf<T>::readAccessType   readAccessType;
  typedef typename SmocPortInStorageReadIf<T>::accessReturnType
    accessReturnType;
  typedef circular_buffer_ra_data<smoc_storage<T> >             storageType;

  //
  SmocPortInStorage(const size_t size = 1) :
    mStorage(size),
    mReadAccessWrapper(mStorage)
  {}

  //
  virtual ~SmocPortInStorage(void) {}

  // wrapper for random_access_circular_buffer::is_full()
  bool isFull(void) const { return mStorage.is_full(); }

  // wrapper for random_access_circular_buffer::put()
  void put(const T& t) { mStorage.put(t); }


  /*
   * StorageTransactorIf
   */
  // wrapper for random_access_circular_buffer:num_available()
  size_t numAvailable(void) const { return mStorage.num_available(); }
  
  // wrapper for random_access_circular_buffer::skip()
  void skip(const size_t n) { mStorage.skip(n); }

  // call invalidate() on smoc_storages
  void invalidate(const size_t n)
  {
    for (size_t i = 0; i < n; ++i)
      mStorage[i].invalidate();
  }


  /*
   * StorageByteAccessIf
   */
  // wrapper for random_access_circular_buffer::is_empty()
  bool isEmpty(void) const { return mStorage.is_empty(); }

  //
  bool isFree(void) const { return mStorage.num_free() > 0; }
 

  /*
   * smoc_port_in_storage_read_if
   */
  //
  readAccessType* getReadChannelAccess(void)
  {
    return &mReadAccessWrapper;
  }

private:
  /*
   * StorageByteAccessIf
   */
  size_t getBytesImpl(byteType *a, const size_t n)
  {
    assert(0);
    return 0;
  }

  //
  size_t putBytesImpl(byteType *a, const size_t n)
  {
    DBG_OUT("SmocPortInStorage::putBytesImpl():\n");
    assert(n == dataSize());
    mStorage.put(*(reinterpret_cast<T*>(a)));
    return dataSize();
  }

  //
  size_t dataSize(void) const { return sizeof(T); }


  /********************************************************
   * wraps our storage so smoc_port_in likes it
   */
  class ReadAccessWrapper :
    public readAccessType
  {
  public:
    ReadAccessWrapper(const storageType &storage) :
      mStorage(storage)
#ifndef NDEBUG
      , mLimit(0)
#endif
    {}

    /*
     * write_access_type
     */
#ifndef NDEBUG
    // set limit and assert in operator[]
    void setLimit(size_t l) { mLimit = l; }
#endif

    const accessReturnType operator[](size_t n) const
    {
      DBG_SMOC(assert(n < mLimit));
      return mStorage[n];
    }

    accessReturnType operator[](size_t n)
    {
      DBG_SMOC(assert(n < mLimit));
      return mStorage[n];
    }
    
    bool tokenIsValid(size_t n) const
    {
      DBG_SMOC(assert(n < mLimit));
      return mStorage[n].isValid();
    }

  private:
    const storageType  &mStorage;
#ifndef NDEBUG
    size_t              mLimit;
#endif
  };
  /*******************************************************/

  // local storage
  storageType        mStorage;  
  // and its wrapper object
  ReadAccessWrapper  mReadAccessWrapper;
};


/******************************************************************************
 * part of smoc_chan_in_if which every transactor has to implement
 *
 */
class PortInTransactorIf
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
 *
 *
 */
// FIXME: interface not needed anymore
class PortInTransactor :
  public PortInTransactorIf
{
public:
  typedef std::map<size_t, smoc_event *>  eventMapType;

  //
  PortInTransactor(StorageTransactorIf &storage) :
    mStorage(storage)
  {}

  //
  virtual ~PortInTransactor(void) {}

  /*
   * port_in_transactor_if
   */
  //
  size_t numAvailable(void) const
  {
    return mStorage.numAvailable();
  }

  // create or return event
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("PortInAdapter::dataAvailableEvent(): n = " << n << std::endl);
    eventMapType::iterator iter = mEventMap.find(n);
    if (iter == mEventMap.end()) {
      const eventMapType::value_type value(n, new smoc_event());
      iter = mEventMap.insert(value).first;

      // FIXME: remove debug code
      DBG_OUT("PortInAdapter::dataAvailableEvent(): create new event\n");

      if (mStorage.numAvailable() >= n)
        iter->second->notify();
    }
    return *(iter->second);
  }
  
  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("PortInAdapter::commitRead(): consumed " << consume
               << " data.\n");

    assert(mStorage.numAvailable() >= consume);

    // invalidate data in storage (avoid accidentally reread)
    mStorage.invalidate(consume);

    // skip commited data in storage
    mStorage.skip(consume);

    updateDataAvailableEvents();
  }

  //
  void updateDataAvailableEvents()
  {
    DBG_SC_OUT("PortInAdapter::updateDataAvailableEvents(): \n");
    eventMapType::iterator iter = mEventMap.begin();

    while (iter != mEventMap.end()) {
      if (iter->first > numAvailable()) {
        iter->second->reset();
        DBG_OUT("   reset\n");
      }
      else {
        iter->second->notify();
        DBG_OUT("   notify\n");
      }

      ++iter;
    }
  }

private:
  eventMapType          mEventMap;
  StorageTransactorIf  &mStorage;
};


#if 0
/******************************************************************************
 *
 *
 */
class SmocPortInPlugBasic
{
public:
  // constructor
  SmocPortInPlugBasic(StorageTransactorIf &storage) :
    mTransactor(storage)
  {}

  //
  size_t numAvailable(void) const
  {
    DBG_SC_OUT("smoc_port_in_plug::numAvailable():\n");
    return mTransactor.numAvailable();
  }

  //
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("smoc_port_in_plug::dataAvailableEvent():\n");
    return mTransactor.dataAvailableEvent(n);
  }

  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("smoc_port_in_plug::commitRead(): got " << consume
               << " data.\n");
    mTransactor.commitRead(consume);
  }

private:
  //
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  PortInTransactor           mTransactor;
};
#endif


/******************************************************************************
 * this plug basically wraps smoc_chan_in_if functions to two objects.
 *
 */
template<typename T>
class SmocPortInPlug :
  public smoc_chan_in_if<T, smoc_channel_access>,
  public PortInTransactor
  //public SmocPortInPlugBasic
{
public:
  typedef SmocPortInPlug<T>                  thisType;
  typedef typename thisType::access_in_type  accessInType;

  // constructor
  //SmocPortInPlug(SmocPortInStorageReadIf<T> &storage) :
  SmocPortInPlug(SmocPortInStorage<T> &storage) :
    //SmocPortInPlugBasic(storage), // StorageTransactorIf
    PortInTransactor(storage),    // StorageTransactorIf
    mStorage(storage)             // SmocPortInStorageReadIf<T>
  {}

  /*
   * smoc_chan_in_if
   */
  //
  size_t numAvailable(void) const
  {
    DBG_SC_OUT("smoc_port_in_plug::numAvailable():\n");
    return PortInTransactor::numAvailable();
  }

  //
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("smoc_port_in_plug::dataAvailableEvent():\n");
    return PortInTransactor::dataAvailableEvent(n);
  }

  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("smoc_port_in_plug::commitRead(): got " << consume
               << " data.\n");
    PortInTransactor::commitRead(consume);
  }


  //
  accessInType *getReadChannelAccess(void)
  {
    DBG_SC_OUT("smoc_port_in_plug::getReadChannelAccess():\n");
    return mStorage.getReadChannelAccess();
  }
  
private:
  // smoc_chan_in_if
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  SmocPortInStorageReadIf<T>  &mStorage;
};


/******************************************************************************
 *
 *
 */
class PlugAggregation
{
public:
  typedef std::pair<StorageByteAccessIf*,
                    PortInTransactor*>    storagePlugPairType;
  typedef StorageByteAccessIf::byteType   byteType;

  //
  PlugAggregation(void)
  {
    DBG_OUT("PlugAggregation::PlugAggregation()\n");
  }

  //
  template<typename T>
  SmocPortInPlug<T> &createInPlug(const size_t size = 1)
  {
    DBG_OUT("PlugAggregation::createInPlug(): create new in plug.\n");
    SmocPortInStorage<T> *storage = new SmocPortInStorage<T>(size);

    SmocPortInPlug<T> *plug = new SmocPortInPlug<T>(*storage);

    mDynamicPlugs.push_back(storagePlugPairType(storage, plug));
    DBG_DOBJ(mDynamicPlugs.size());

    return *plug;
  }

  //
  void writeBytes(const size_t pos, byteType *a, const size_t n)
  {
    DBG_OUT("writeBytes()\n");
    assert(pos < mDynamicPlugs.size());
    StorageByteAccessIf *byteAccess = mDynamicPlugs[pos].first;
    const size_t ret = byteAccess->putBytes(a, n);
    assert(ret == n);

    PortInTransactor *transactor = mDynamicPlugs[pos].second;
    transactor->updateDataAvailableEvents();
  }

  //
  size_t getNumPlugs(void) const { return mDynamicPlugs.size(); }

private:
  //
  PlugAggregation(const PlugAggregation &other) { assert(0); }

  std::vector<storagePlugPairType> mDynamicPlugs;
};


/******************************************************************************
 *
 *
 */
class A1 :
  public smoc_actor
{
public:
  smoc_port_in<int>  in1;

  A1(sc_module_name name) :
    smoc_actor(name, start),
    m_fire_counter(0)
  {
    start =
      in1(1)           >>
      CALL(A1::func1)  >> start;
  }

private:
  smoc_firing_state start;
  int               m_fire_counter;

  void func1(void)
  {
    DBG_SC_OUT("A1::func1(): got " << in1[0] << std::endl);
    ++m_fire_counter;
  }
};


/******************************************************************************
 *
 *
 */
class PutModule :
  public sc_module
{
public:
  PutModule(sc_module_name name, PlugAggregation &plugAggregation) :
    sc_module(name),
    mPlugAggregation(plugAggregation)
  {}

private:
  //
  void end_of_elaboration(void)
  {
    int i = 42;
    mPlugAggregation.writeBytes(0,
                                reinterpret_cast<PlugAggregation::byteType*>(&i),
                                sizeof(int));
  }

  PlugAggregation &mPlugAggregation;
};


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
  //
  // FIXME: systemoc copies parameter! we must use pointer instead of reference
  Tlm_tester(sc_module_name name, PlugAggregation *plugAggregation) :
    smoc_graph(name),
    a1("A1")
  {
    SmocPortInPlug<int> &port = plugAggregation->createInPlug<int>();
    connectChanPort(port, a1.in1);
  }

private:
  A1 a1;
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

  PlugAggregation plugAggregation;

  smoc_top_moc<Tlm_tester<address_type, data_type, data_mode> >
    tlm_tester("tlm_tester", &plugAggregation);

  PutModule putModule("putModule", plugAggregation);

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

