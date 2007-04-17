

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
#include "transactor_storages.h"
#include "debug_on.h"


/******************************************************************************
 * part of smoc_chan_in_if
 *
 */
class PortInTransactor
{
public:
  typedef std::map<size_t, smoc_event *>  eventMapType;

  //
  PortInTransactor(StorageInTransactorIf &storage) :
    mStorage(storage)
  {}

  //
  virtual ~PortInTransactor(void) {}

  //
  size_t numAvailable(void) const { return mStorage.numAvailable(); }

  // create or return event
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("PortInAdapter::dataAvailableEvent(): n = " << n << std::endl);
    eventMapType::iterator iter = mEventMap.find(n);
    if (iter == mEventMap.end()) {
      const eventMapType::value_type value(n, new smoc_event());
      iter = mEventMap.insert(value).first;

      DBG_OUT("PortInAdapter::dataAvailableEvent(): create new event\n");

      if (numAvailable() >= n)
        iter->second->notify();
    }
    return *(iter->second);
  }
  
  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("PortInAdapter::commitRead(): consumed " << consume
               << " data.\n");

    assert(numAvailable() >= consume);

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
  eventMapType            mEventMap;
  StorageInTransactorIf  &mStorage;
};


/******************************************************************************
 * part of smoc_chan_out_if
 *
 */
class PortOutTransactor
{
public:
  typedef std::map<size_t, smoc_event *>  eventMapType;
  
  //
  PortOutTransactor(StorageOutTransactorIf &storage) :
    mStorage(storage)
  {}

  //
  virtual ~PortOutTransactor(void) {}

  //
  size_t numFree(void) const { return mStorage.numFree(); }

  //
  smoc_event &spaceAvailableEvent(size_t n)
  {
    DBG_SC_OUT("PortOutAdapter::spaceAvailableEvent(): n = " << n << std::endl);
    eventMapType::iterator iter = mEventMap.find(n);
    if (iter == mEventMap.end()) {
      const eventMapType::value_type value(n, new smoc_event());
      iter = mEventMap.insert(value).first;

      // FIXME: remove debug code
      DBG_OUT("PortOutAdapter::spaceAvailableEvent(): create new event\n");

      if (numFree() >= n)
        iter->second->notify();
    }
    return *(iter->second);
  }

  //
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("OutAdapter::commitWrite(): produced " << produce
               << " data.\n");

    mStorage.commitRaWrite(produce);

    updateSpaceAvailableEvents();
  }

  //
  void updateSpaceAvailableEvents()
  {
    eventMapType::iterator iter = mEventMap.begin();

    while (iter != mEventMap.end()) {
      if (iter->first > numFree())
        iter->second->reset();
      else
        iter->second->notify();

      ++iter;
    }
  }

private:
  eventMapType             mEventMap;
  StorageOutTransactorIf  &mStorage;
};


/******************************************************************************
 * this plug basically wraps smoc_chan_in_if functions.
 *
 */
template<typename T>
class SmocPortInPlug :
  public smoc_chan_in_if<T, smoc_channel_access>,
  public PortInTransactor
{
public:
  typedef SmocPortInPlug<T>                  thisType;
  typedef typename thisType::access_in_type  accessInType;

  // constructor
  SmocPortInPlug(SmocPortInStorage<T> &storage) :
    PortInTransactor(storage),    // StorageTransactorIf
    mStorage(storage)             // SmocPortInStorageReadIf<T>
  {}

  /*
   * smoc_chan_in_if
   */
  //
  size_t numAvailable(void) const
  {
    DBG_SC_OUT("SmocPortInPlug::numAvailable():\n");
    return PortInTransactor::numAvailable();
  }

  //
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("SmocPortInPlug::dataAvailableEvent():\n");
    return PortInTransactor::dataAvailableEvent(n);
  }

  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("SmocPortInPlug::commitRead(): got " << consume
               << " data.\n");
    PortInTransactor::commitRead(consume);
  }

  //
  accessInType *getReadChannelAccess(void)
  {
    DBG_SC_OUT("SmocPortInPlug::getReadChannelAccess():\n");
    return mStorage.getReadChannelAccess();
  }
  
private:
  // smoc_chan_in_if
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  SmocPortInStorageReadIf<T>  &mStorage;
};


/******************************************************************************
 * this plug basically wraps smoc_chan_out_if functions to two objects.
 *
 */
template<typename T>
class SmocPortOutPlug :
  public smoc_chan_out_if<T, smoc_channel_access>,
  public PortOutTransactor
{
public:
  typedef SmocPortOutPlug<T>                  thisType;
  typedef typename thisType::access_out_type  accessOutType;

  // constructor
  SmocPortOutPlug(SmocPortOutStorageWriteIf<T> &storage) :
    mStorage(storage)
  {}

  /*
   * smoc_chan_in_if
   */
  //
  size_t numFree(void) const
  {
    DBG_SC_OUT("smoc_port_out_plug::numFree():\n");
    //return SmocPortOutTransactor::numFree();
    return PortOutTransactor::numFree();
  }

  //
  smoc_event &spaceAvailableEvent(size_t n)
  {
    DBG_SC_OUT("smoc_port_out_plug::spaceAvailableEvent():\n");
    return PortOutTransactor::spaceAvailableEvent(n);
  }

  //
  accessOutType *getWriteChannelAccess(void)
  {
    DBG_SC_OUT("smoc_port_out_plug::getWriteChannelAccess():\n");
    return mStorage.getWriteChannelAccess();
  }

  //
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("smoc_port_out_plug::commitWrite(): got " << produce
               << " data.\n");

    // check if every comitted token is valid
    for (size_t i = 0; i < produce; ++i)
      assert(mStorage.tokenIsValid(i));

    PortOutTransactor::commitWrite(produce);
  }
  
private:
  // smoc_chan_in_if
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  SmocPortOutStorageWriteIf<T>  &mStorage;
};
 

/******************************************************************************
 *
 *
 */
class InPortPlugAggregation
{
public:
  typedef std::pair<StorageByteAccessIf*,
                    PortInTransactor*>    storagePlugPairType;
  typedef StorageByteAccessIf::byteType   byteType;

  //
  InPortPlugAggregation(void) {}

  //
  template<typename T>
  SmocPortInPlug<T> &createInPlug(const size_t size = 1)
  {
    DBG_OUT("InPortPlugAggregation::createInPlug(): create new in plug.\n");
    SmocPortInStorage<T> *storage = new SmocPortInStorage<T>(size);

    SmocPortInPlug<T> *plug = new SmocPortInPlug<T>(*storage);

    mDynamicPlugs.push_back(storagePlugPairType(storage, plug));
    DBG_DOBJ(mDynamicPlugs.size());

    return *plug;
  }

  //
  void writeBytes(const size_t pos, const byteType *a, const size_t n)
  {
    DBG_OUT("InPortPlugAggregation::writeBytes(): pos: " << pos << std::endl);
    assert(pos < getNumPlugs());
    StorageByteAccessIf *byteAccess = mDynamicPlugs[pos].first;
    const size_t ret = byteAccess->putBytes(a, n);
    assert(ret == n);

    PortInTransactor *transactor = mDynamicPlugs[pos].second;
    transactor->updateDataAvailableEvents();
  }

  //
  size_t getNumPlugs(void) const { return mDynamicPlugs.size(); }

private:
  // disabled
  InPortPlugAggregation(const InPortPlugAggregation &other) { assert(0); }

  std::vector<storagePlugPairType> mDynamicPlugs;
};


#if 0
/******************************************************************************
 *
 *
 */
class SmocInPlugContainer
{
public:
  SiblingInfo const *getSiblingInfo(void) const { return mSiblingInfo; }
private:
  Plug*;
  Storage*;
  SiblingInfo const *mSiblingInfo;
  MyInfo;
}

/******************************************************************************
 *
 *
 */
class SmocPlugFactory
{
public:

  //
  void createPlugPair() {}
};

class TransactorAggregation
{
public:

  //
  void registerInTransactor() {}

  //
  void registerOutTransactor() {}
};
#endif


/******************************************************************************
 *
 *
 */
class A1 :
  public smoc_actor
{
public:
  smoc_port_in<int>  in1;
  smoc_port_in<unsigned long long>  in2;

  A1(sc_module_name name) :
    smoc_actor(name, start),
    m_fire_counter(0)
  {
    start =
      in1(1)           >>
      CALL(A1::func1)  >> start
    |
      in2(1)           >>
      CALL(A1::func2)  >> start;
  }

private:
  smoc_firing_state start;
  int               m_fire_counter;

  void func1(void)
  {
    DBG_SC_OUT("A1::func1(): got " << in1[0] << std::endl);
    ++m_fire_counter;
  }

  //
  void func2(void)
  {
    DBG_SC_OUT("A1::func2(): got " << in2[0] << std::endl);
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
  PutModule(sc_module_name name, InPortPlugAggregation &plugAggregation) :
    sc_module(name),
    mPlugAggregation(plugAggregation)
  {}

private:
  typedef InPortPlugAggregation::byteType byteType;
  //
  void end_of_elaboration(void)
  {
    const int i = 42;
    mPlugAggregation.writeBytes(0,
                                reinterpret_cast<const byteType*>(&i),
                                sizeof(int));
    const unsigned long long j = 11;
    mPlugAggregation.writeBytes(1,
                                reinterpret_cast<const byteType*>(&j),
                                sizeof(unsigned long long));
  }

  InPortPlugAggregation &mPlugAggregation;
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
  Tlm_tester(sc_module_name name, InPortPlugAggregation &plugAggregation) :
    smoc_graph(name),
    a1("A1")
  {
    connectChanPort(plugAggregation.createInPlug<int>(), a1.in1);
    connectChanPort(plugAggregation.createInPlug<unsigned long long>(),
                    a1.in2);
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

  InPortPlugAggregation plugAggregation;

  Tlm_tester<address_type, data_type, data_mode>
    tlm_tester("tlm_tester", plugAggregation);
  smoc_top top(&tlm_tester);

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

