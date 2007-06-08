
#ifndef ADAPTERS_H
#define ADAPTERS_H

#include "storages.h"
#include "plugs.h"

#include "debug_off.h"


/******************************************************************************
 *
 *
 */
enum ProtocolType
{
  UnknownProtocol,
  PushDataProtocol
};


/******************************************************************************
 *
 *
 */
template <typename ADDRESS, ADDRESS defaultAddress = 0x0>
class PortPlugInfos
{
public:
  PortPlugInfos(const int id = 0, const ProtocolType = UnknownProtocol) :
    id(id),
    protocol(protocol),
    data(defaultAddress),
    size(defaultAddress)
  {}

  int          id;
  ProtocolType protocol;
  ADDRESS      data;
  ADDRESS      size; 
};


/******************************************************************************
 *
 *
 */
template <typename ADDRESS>
class AdapterCommitReadListener
{
public:
  virtual void notifyCommitRead(const PortPlugInfos<ADDRESS> &myInfos,
                                const size_t consume) = 0;
};


/******************************************************************************
 *
 *
 */
template <typename ADDRESS>
class AdapterCommitWriteListener
{
public:
  virtual void notifyCommitWrite(const PortPlugInfos<ADDRESS> &myInfos,
                                 const size_t produce) = 0;
};


/******************************************************************************
 * encapsulates and creates plug and storage. optionally, a storage or proxy 
 * can be given (TODO). Only data independent interface to storage.
 * TODO: give commitPolicy when creating.
 *
 */
template <typename ADDRESS>
class InPortAdapter :
  public CommitReadListener
{
public:
  //
  InPortAdapter(PortPlugInfos<ADDRESS> *myInfos,
                PortPlugInfos<ADDRESS> *syblingInfos) :
    mIsInitialized(false),
    mPortPlugInfos(myInfos),
    mSyblingPortPlugInfos(syblingInfos),
    mCommitListener(NULL)
  {}

  // return pair<plug, storage>?
  template<typename T>
  InPortPlug<T> *createPlugAndStorage(const int size = 1,
                                      const T *dummy = NULL)
  {
    assert(!mIsInitialized);

    InPortStorage<T> *storage = new InPortStorage<T>(size);
    mStorage = storage;
    InPortPlug<T> *plug = new InPortPlug<T>(*storage, *this);
    mPlugBase = plug;
    mIsInitialized = true;

    return plug;
  }

  // full access
  PortPlugInfos<ADDRESS> &getPortPlugInfos(void) { return *mPortPlugInfos; }

  // read only
  const PortPlugInfos<ADDRESS> &getPortPlugInfos(void) const
  {
    return *mPortPlugInfos;
  }

  //
  const PortPlugInfos<ADDRESS> &getSyblingPortPlugInfos(void) const
  {
    return *mSyblingPortPlugInfos;
  }

  //
  bool isInitialized(void) const { return mIsInitialized; }

  // create plug with given storage/storageProxy
  // InPortPlug<T> *createInPlug(storage) ...

  // This is what Aggregation needs:
  //  updateDataAvailableEvents() if storage changed
  //  getByteAccess() to change storage

  //
  void updateDataAvailableEvents(void)
  {
    assert(isInitialized());
    mPlugBase->updateDataAvailableEvents();
  }

  //
  StorageByteAccessIf &getByteAccess(void)
  {
    assert(isInitialized());
    return *mStorage;
  }

  //
  void registerAdapterCommitReadListener(AdapterCommitReadListener<ADDRESS>
      *listener)
  {
    // allow only one listener for now
    assert(mCommitListener == NULL);
    DBG_OUT("InPortAdapter::()registerCommitReadListener.\n");
    mCommitListener = listener;
  }

  /*
   * CommitReadListener
   */
  void notifyCommitRead(size_t consume)
  {
    assert(isInitialized());
    assert(mStorage->numAvailable() >= consume);
    // invalidate data in storage (avoid accidentally reread)
    mStorage->invalidate(consume);

    // skip commited data in storage
    mStorage->skip(consume);

    // update events
    updateDataAvailableEvents();

    if (mCommitListener)
      mCommitListener->notifyCommitRead(*mPortPlugInfos, consume);
  }

private:
  // disabled because plug got reference to 'this'
  InPortAdapter(const InPortAdapter &other) {}

  //
  InPortAdapter &operator=(InPortAdapter &other) { return other; }


  bool                                 mIsInitialized; // FIXME: isPlugCreated?
  InPortPlugBase                      *mPlugBase;
  StorageInIf                         *mStorage;
  PortPlugInfos<ADDRESS>              *mPortPlugInfos;
  const PortPlugInfos<ADDRESS>        *mSyblingPortPlugInfos;
  AdapterCommitReadListener<ADDRESS>  *mCommitListener;
};


/******************************************************************************
 * encapsulates and creates plug and storage. optionally, a storage or proxy 
 * can be given (TODO). Only data independent interface to storage.
 * TODO: give commitPolicy when creating.
 *
 */
template <typename ADDRESS>
class OutPortAdapter :
  public CommitWriteListener
{
public:
  //
  OutPortAdapter(PortPlugInfos<ADDRESS> *myInfos,
                 PortPlugInfos<ADDRESS> *syblingInfos) :
    mIsInitialized(false),
    mPortPlugInfos(myInfos),
    mSyblingPortPlugInfos(syblingInfos),
    mCommitWriteListener(NULL)
  {}

  // return pair<plug, storage>?
  template<typename T>
  OutPortPlug<T> *createPlugAndStorage(const int size = 1,
                                           const T *dummy = NULL)
  {
    assert(!mIsInitialized);

    OutPortStorage<T> *storage = new OutPortStorage<T>(size);
    mStorage = storage;
    OutPortPlug<T> *plug = new OutPortPlug<T>(*storage, *this);
    mPlugBase = plug;
    mIsInitialized = true;

    return plug;
  }

  // full access
  PortPlugInfos<ADDRESS> &getPortPlugInfos(void) { return *mPortPlugInfos; }
  
  // read only
  const PortPlugInfos<ADDRESS> &getPortPlugInfos(void) const
  {
    return *mPortPlugInfos;
  }

  //
  const PortPlugInfos<ADDRESS> &getSyblingPortPlugInfos(void) const
  {
    return *mSyblingPortPlugInfos;
  }

  //
  bool isInitialized(void) const { return mIsInitialized; }

  // create plug with given storage/storageProxy
  // OutPortPlug<T> *createOutPlug(storage) ...

  // This is what Aggregation needs:
  //  updateDataAvailableEvents() if storage changed
  //  getByteAccess() to change storage

  //
  void updateSpaceAvailableEvents(void)
  {
    assert(isInitialized());
    mPlugBase->updateSpaceAvailableEvents();
  }

  //
  StorageByteAccessIf &getByteAccess(void)
  {
    assert(isInitialized());
    return *mStorage;
  }

  //
  void registerAdapterCommitWriteListener(AdapterCommitWriteListener<ADDRESS>
      *listener)
  {
    DBG_OUT("OutPortAdapter::registerCommitWriteListener().\n");
    assert(mCommitWriteListener == NULL);
    mCommitWriteListener = listener;
  }

  /*
   * CommitWriteListener
   */
  void notifyCommitWrite(size_t produce)
  {
    assert(isInitialized());
    assert(mStorage->numFree() >= produce);

    mStorage->commitRaWrite(produce);

    updateSpaceAvailableEvents();

    if (mCommitWriteListener)
      mCommitWriteListener->notifyCommitWrite(*mPortPlugInfos, produce);
  }

private:
  // disabled because plug got reference to 'this'
  OutPortAdapter(const OutPortAdapter &other) {}

  //
  OutPortAdapter &operator=(OutPortAdapter &other) { return other; }


  bool                                  mIsInitialized;
  OutPortPlugBase                      *mPlugBase;
  StorageOutIf                         *mStorage;
  PortPlugInfos<ADDRESS>               *mPortPlugInfos;
  const PortPlugInfos<ADDRESS>         *mSyblingPortPlugInfos;
  AdapterCommitWriteListener<ADDRESS>  *mCommitWriteListener;
};


/******************************************************************************
 *
 *
 */
template <typename ADDRESS>
class PortAdapterFactory
{
public:
  typedef std::pair<InPortAdapter<ADDRESS>*,
                    OutPortAdapter<ADDRESS>*>  adapterPairType;

  template<typename T>
  std::pair<InPortPlug<T>*, OutPortPlug<T>*>
  createAdapterPair(const size_t inSize,
                    const size_t outSize,
                    InPortAdapter<ADDRESS> **inPortAdapter,
                    OutPortAdapter<ADDRESS> **outPortAdapter,
                    const T *dummy = NULL)
  {
    DBG_OUT("PortAdapterFactory::createAdapterPair().\n");
    // TODO: remember for delete, debug, ...
    PortPlugInfos<ADDRESS> *inPlugInfos = new PortPlugInfos<ADDRESS>();
    PortPlugInfos<ADDRESS> *outPlugInfos = new PortPlugInfos<ADDRESS>();
 
  // FIXME FIXME FIXME
    
//    InPortAdapter<ADDRESS> x(inPlugInfos, outPlugInfos);

  //    *x = new InPortAdapter<addressType>(inPlugInfos, outPlugInfos);
    // THIS DOES x.createPlugAndStorage(inSize, new T());
//    InPortPlug<T> *inPlug2 = x.createPlugAndStorage(inSize);
    

    *inPortAdapter = new InPortAdapter<ADDRESS>(inPlugInfos, outPlugInfos);
    inPortAdapterVec.push_back(*inPortAdapter);
    InPortPlug<T> *inPlug =
      (**inPortAdapter).createPlugAndStorage(inSize, (T*)NULL);

    *outPortAdapter = new OutPortAdapter<ADDRESS>(outPlugInfos, inPlugInfos);
    outPortAdapterVec.push_back(*outPortAdapter);
    OutPortPlug<T> *outPlug =
      (*outPortAdapter)->createPlugAndStorage(outSize, (T*)NULL);

    return std::pair<InPortPlug<T>*, OutPortPlug<T>*>(inPlug, outPlug);

  }

private:
  std::vector<InPortAdapter<ADDRESS>*>  inPortAdapterVec;
  std::vector<OutPortAdapter<ADDRESS>*> outPortAdapterVec;
};



#endif // ADAPTERS_H
