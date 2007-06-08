
#ifndef TRANSACTORS_PLUGS
#define TRANSACTORS_PLUGS


#include "transactor_storages.h"
#include "debug_on.h"


/******************************************************************************
 *
 *
 */
class CommitReadListener
{
public:
  virtual void notifyCommitRead(size_t consume) = 0;
};


/******************************************************************************
 *
 *
 */
class CommitWriteListener
{
public:
  virtual void notifyCommitWrite(size_t produce) = 0;
};


/******************************************************************************
 * date type independent part of smoc_chan_in_if
 *
 */
class PortInTransactor
{
public:
  typedef std::map<size_t, smoc_event *>  eventMapType;

  //
  PortInTransactor(StorageInIf &storage,
                   CommitReadListener &manager) : // exactly one listener
    mStorage(storage),
    mManager(manager)
  {}

  //
  virtual ~PortInTransactor(void) {}

  //
  size_t numAvailable(void) const { return mStorage.numAvailable(); }

  // create or return event
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("PortInTransactor::dataAvailableEvent(): n = "
               << n << std::endl);
    eventMapType::iterator iter = mEventMap.find(n);
    if (iter == mEventMap.end()) {
      const eventMapType::value_type value(n, new smoc_event());
      iter = mEventMap.insert(value).first;

      DBG_OUT("PortInTransactor::dataAvailableEvent(): create new event\n");

      if (numAvailable() >= n)
        iter->second->notify();
    }
    return *(iter->second);
  }
  
  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("PortInTransactor::commitRead(): consumed " << consume
               << " data. Forward to manager.\n");

    mManager.notifyCommitRead(consume);
  }

  //
  void updateDataAvailableEvents()
  {
    DBG_SC_OUT("PortInTransactor::updateDataAvailableEvents(): \n");
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
  eventMapType                  mEventMap;
  StorageInIf                  &mStorage;
  CommitReadListener  &mManager;
};


/******************************************************************************
 * date type independent part of smoc_chan_out_if
 *
 */
class PortOutTransactor
{
public:
  typedef std::map<size_t, smoc_event *>  eventMapType;
  
  //
  PortOutTransactor(StorageOutIf &storage,
                    CommitWriteListener &manager) :
    mStorage(storage),
    mManager(manager)
  {}

  //
  virtual ~PortOutTransactor(void) {}

  // forward to associated storage
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

  // forward to manager
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("OutAdapter::commitWrite(): produced " << produce
               << " data.\n");

    // check if every comitted token is valid
    for (size_t i = 0; i < produce; ++i)
      assert(mStorage.tokenIsValid(i));

    // transactor forwards this to some manager which has some commitPolicy
    //  and bundles all commiting stuff
    mManager.notifyCommitWrite(produce);
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
  eventMapType                   mEventMap;
  StorageOutIf                  &mStorage;
  CommitWriteListener  &mManager;
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
  SmocPortInPlug(SmocPortInStorage<T> &storage,
                 CommitReadListener &manager) :
    PortInTransactor(storage, manager),    // only StorageInIf
    mStorage(storage)                      // SmocPortInStorageReadIf<T>
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
 * this plug basically wraps smoc_chan_out_if functions to two objects,
 * transactor and storage. Plug should not have full access to storage, so
 * everything is forwarded to transactor.
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
  SmocPortOutPlug(SmocPortOutStorage<T> &storage,
                  CommitWriteListener &manager) :
    PortOutTransactor(storage, manager),    // only StorageOutIf
    mStorage(storage)                       // SmocPortOutStorageWriteIf<T>
  {}

  /*
   * smoc_chan_in_if
   */
  //
  size_t numFree(void) const
  {
    DBG_SC_OUT("smoc_port_out_plug::numFree():\n");
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

    PortOutTransactor::commitWrite(produce);
  }
  
private:
  // smoc_chan_in_if
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  SmocPortOutStorageWriteIf<T>  &mStorage;
};
 

#endif // TRANSACTORS_PLUGS
