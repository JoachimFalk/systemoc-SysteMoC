
#ifndef PLUGS_H
#define PLUGS_H

#include "storages.h"
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
class InPortPlugBase
{
public:
  typedef std::map<size_t, smoc_event *>  eventMapType;

  //
  InPortPlugBase(StorageInIf &storage,
                 CommitReadListener &listener) : // exactly one listener
    mStorage(storage),
    mReadListener(listener)
  {}

  //
  virtual ~InPortPlugBase(void) {}

  //
  size_t numAvailable(void) const { return mStorage.numAvailable(); }

  // create or return event
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("InPortPlugBase::dataAvailableEvent(): n = "
               << n << std::endl);
    eventMapType::iterator iter = mEventMap.find(n);
    if (iter == mEventMap.end()) {
      const eventMapType::value_type value(n, new smoc_event());
      iter = mEventMap.insert(value).first;

      DBG_OUT("InPortPlugBase::dataAvailableEvent(): create new event\n");

      if (numAvailable() >= n)
        iter->second->notify();
    }
    return *(iter->second);
  }
  
  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("InPortPlugBase::commitRead(): consumed " << consume
               << " data. Forward to adapter.\n");

    mReadListener.notifyCommitRead(consume);
  }

  //
  void updateDataAvailableEvents()
  {
    DBG_SC_OUT("InPortPlugBase::updateDataAvailableEvents(): \n");
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
  eventMapType         mEventMap;
  StorageInIf         &mStorage;
  CommitReadListener  &mReadListener;
};


/******************************************************************************
 * date type independent part of smoc_chan_out_if
 *
 */
class OutPortPlugBase
{
public:
  typedef std::map<size_t, smoc_event *>  eventMapType;
  
  //
  OutPortPlugBase(StorageOutIf &storage,
                  CommitWriteListener &listener) :
    mStorage(storage),
    mWriteListener(listener)
  {}

  //
  virtual ~OutPortPlugBase(void) {}

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

  // forward to adapter
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("OutAdapter::commitWrite(): produced " << produce
               << " data.\n");

    // check if every comitted token is valid
    for (size_t i = 0; i < produce; ++i)
      assert(mStorage.tokenIsValid(i));

    // plug forwards this to some adapter which owns some commitPolicy
    //  and bundles all commiting stuff
    mWriteListener.notifyCommitWrite(produce);
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
  eventMapType          mEventMap;
  StorageOutIf         &mStorage;
  CommitWriteListener  &mWriteListener;
};


/******************************************************************************
 * this plug basically wraps smoc_chan_in_if functions to two objects,
 * InPortPlugBase and InPortStorageReadIf<T>.
 *
 */
template<typename T>
class InPortPlug :
  public smoc_chan_in_if<T, smoc_channel_access>,
  public InPortPlugBase
{
public:
  typedef InPortPlug<T>                      thisType;
  typedef typename thisType::access_in_type  accessInType;

  // constructor
  InPortPlug(InPortStorage<T> &storage,
             CommitReadListener &listener) :
    InPortPlugBase(storage, listener),    // only StorageInIf
    mStorage(storage)                     // InPortStorageReadIf<T>
  {}

  /*
   * smoc_chan_in_if
   */
  //
  size_t numAvailable(void) const
  {
    DBG_SC_OUT("InPortPlug::numAvailable():\n");
    return InPortPlugBase::numAvailable();
  }

  //
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("InPortPlug::dataAvailableEvent():\n");
    return InPortPlugBase::dataAvailableEvent(n);
  }

  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("InPortPlug::commitRead(): got " << consume
               << " data.\n");
    InPortPlugBase::commitRead(consume);
  }

  //
  accessInType *getReadChannelAccess(void)
  {
    DBG_SC_OUT("InPortPlug::getReadChannelAccess():\n");
    return mStorage.getReadChannelAccess();
  }
  
private:
  // smoc_chan_in_if
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  InPortStorageReadIf<T>  &mStorage;
};


/******************************************************************************
 * this plug basically wraps smoc_chan_out_if functions to two objects,
 * OutPortPlugBase and OutPortStorageWriteIf<T>.
 *
 */
template<typename T>
class OutPortPlug :
  public smoc_chan_out_if<T, smoc_channel_access>,
  public OutPortPlugBase
{
public:
  typedef OutPortPlug<T>                      thisType;
  typedef typename thisType::access_out_type  accessOutType;

  // constructor
  OutPortPlug(OutPortStorage<T> &storage,
              CommitWriteListener &listener) :
    OutPortPlugBase(storage, listener),    // only StorageOutIf
    mStorage(storage)                      // OutPortStorageWriteIf<T>
  {}

  /*
   * smoc_chan_in_if
   */
  //
  size_t numFree(void) const
  {
    DBG_SC_OUT("smoc_port_out_plug::numFree():\n");
    return OutPortPlugBase::numFree();
  }

  //
  smoc_event &spaceAvailableEvent(size_t n)
  {
    DBG_SC_OUT("smoc_port_out_plug::spaceAvailableEvent():\n");
    return OutPortPlugBase::spaceAvailableEvent(n);
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

    OutPortPlugBase::commitWrite(produce);
  }
  
private:
  // smoc_chan_in_if
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  OutPortStorageWriteIf<T>  &mStorage;
};
 

#endif // PLUGS_H
