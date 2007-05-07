
#ifndef MANAGERS_AGGREGATION
#define MANAGERS_AGGREGATION

#include "transactor_storages.h"
#include "transactors_plugs.h"

#include "debug_off.h"

#include "tlm.h"
#include "tlm_pvt_annotated_fifo.h"


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
class ManagerCommitReadListener
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
class ManagerCommitWriteListener
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
class InPortManager :
  public CommitReadListener
{
public:
  //
  InPortManager(PortPlugInfos<ADDRESS> *myInfos,
                PortPlugInfos<ADDRESS> *syblingInfos) :
    mIsInitialized(false),
    mPortPlugInfos(myInfos),
    mSyblingPortPlugInfos(syblingInfos),
    mCommitListener(NULL)
  {}

  // return pair<plug, storage>?
  template<typename T>
  SmocPortInPlug<T> *createPlugAndStorage(const int size = 1,
                                          const T *dummy = NULL)
  {
    assert(!mIsInitialized);

    SmocPortInStorage<T> *storage = new SmocPortInStorage<T>(size);
    mStorage = storage;
    SmocPortInPlug<T> *plug = new SmocPortInPlug<T>(*storage, *this);
    mTransactor = plug;
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
  // SmocPortInPlug<T> *createInPlug(storage) ...

  // This is what Aggregation needs:
  //  updateDataAvailableEvents() if storage changed
  //  getByteAccess() to change storage

  //
  void updateDataAvailableEvents(void)
  {
    mTransactor->updateDataAvailableEvents();
  }

  //
  StorageByteAccessIf &getByteAccess(void)
  {
    return *mStorage;
  }

  //
  void registerManagerCommitReadListener(ManagerCommitReadListener<ADDRESS>
      *listener)
  {
    // allow only one listener for now
    assert(mCommitListener == NULL);
    DBG_OUT("InPortManager::()registerCommitReadListener.\n");
    mCommitListener = listener;
  }

  /*
   * CommitReadListener
   */
  void notifyCommitRead(size_t consume)
  {
    assert(mIsInitialized);
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
  InPortManager(const InPortManager &other) {}

  //
  InPortManager &operator=(InPortManager &other) { return other; }


  bool                        mIsInitialized; // FIXME: isPlugCreated?
  PortInTransactor           *mTransactor;
  StorageInIf                *mStorage;
  PortPlugInfos<ADDRESS>              *mPortPlugInfos;
  const PortPlugInfos<ADDRESS>        *mSyblingPortPlugInfos;
  ManagerCommitReadListener<ADDRESS>  *mCommitListener;
};


/******************************************************************************
 * encapsulates and creates plug and storage. optionally, a storage or proxy 
 * can be given (TODO). Only data independent interface to storage.
 * TODO: give commitPolicy when creating.
 *
 */
template <typename ADDRESS>
class OutPortManager :
  public CommitWriteListener
{
public:
  //
  OutPortManager(PortPlugInfos<ADDRESS> *myInfos,
                 PortPlugInfos<ADDRESS> *syblingInfos) :
    mIsInitialized(false),
    mPortPlugInfos(myInfos),
    mSyblingPortPlugInfos(syblingInfos),
    mCommitWriteListener(NULL)
  {}

  // return pair<plug, storage>?
  template<typename T>
  SmocPortOutPlug<T> *createPlugAndStorage(const int size = 1,
                                           const T *dummy = NULL)
  {
    assert(!mIsInitialized);

    SmocPortOutStorage<T> *storage = new SmocPortOutStorage<T>(size);
    mStorage = storage;
    SmocPortOutPlug<T> *plug = new SmocPortOutPlug<T>(*storage, *this);
    mTransactor = plug;
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
  // SmocPortOutPlug<T> *createOutPlug(storage) ...

  // This is what Aggregation needs:
  //  updateDataAvailableEvents() if storage changed
  //  getByteAccess() to change storage

  //
  void updateSpaceAvailableEvents(void)
  {
    mTransactor->updateSpaceAvailableEvents();
  }

  //
  StorageByteAccessIf &getByteAccess(void)
  {
    return *mStorage;
  }

  //
  void registerManagerCommitWriteListener(ManagerCommitWriteListener<ADDRESS>
      *listener)
  {
    DBG_OUT("OutPortManager::registerCommitWriteListener().\n");
    assert(mCommitWriteListener == NULL);
    mCommitWriteListener = listener;
  }

  /*
   * CommitWriteListener
   */
  void notifyCommitWrite(size_t produce)
  {
    assert(mIsInitialized);
    assert(mStorage->numFree() >= produce);

    mStorage->commitRaWrite(produce);

    updateSpaceAvailableEvents();

    if (mCommitWriteListener)
      mCommitWriteListener->notifyCommitWrite(*mPortPlugInfos, produce);
  }

private:
  // disabled because plug got reference to 'this'
  OutPortManager(const OutPortManager &other) {}

  //
  OutPortManager &operator=(OutPortManager &other) { return other; }


  bool                  mIsInitialized;
  PortOutTransactor    *mTransactor;
  StorageOutIf         *mStorage;
  PortPlugInfos<ADDRESS>        *mPortPlugInfos;
  const PortPlugInfos<ADDRESS>  *mSyblingPortPlugInfos;
  ManagerCommitWriteListener<ADDRESS> *mCommitWriteListener;
};


/******************************************************************************
 *
 *
 */
template <typename ADDRESS>
class PortManagerFactory
{
public:
  typedef std::pair<InPortManager<ADDRESS>*,
                    OutPortManager<ADDRESS>*>  managerPairType;

  template<typename T>
  std::pair<SmocPortInPlug<T>*, SmocPortOutPlug<T>*>
  createManagerPair(const size_t inSize,
                    const size_t outSize,
                    InPortManager<ADDRESS> **inPortManager,
                    OutPortManager<ADDRESS> **outPortManager,
                    const T *dummy = NULL)
  {
    DBG_OUT("PortManagerFactory::createManagerPair().\n");
    // TODO: remember for delete, debug, ...
    PortPlugInfos<ADDRESS> *inPlugInfos = new PortPlugInfos<ADDRESS>();
    PortPlugInfos<ADDRESS> *outPlugInfos = new PortPlugInfos<ADDRESS>();
 
  // FIXME FIXME FIXME
    
//    InPortManager<ADDRESS> x(inPlugInfos, outPlugInfos);

  //    *x = new InPortManager<addressType>(inPlugInfos, outPlugInfos);
    // THIS DOES x.createPlugAndStorage(inSize, new T());
//    SmocPortInPlug<T> *inPlug2 = x.createPlugAndStorage(inSize);
    

    *inPortManager = new InPortManager<ADDRESS>(inPlugInfos, outPlugInfos);
    inPortMangerVec.push_back(*inPortManager);
    SmocPortInPlug<T> *inPlug =
      (**inPortManager).createPlugAndStorage(inSize, (T*)NULL);

    *outPortManager = new OutPortManager<ADDRESS>(outPlugInfos, inPlugInfos);
    outPortMangerVec.push_back(*outPortManager);
    SmocPortOutPlug<T> *outPlug =
      (*outPortManager)->createPlugAndStorage(outSize, (T*)NULL);

    return std::pair<SmocPortInPlug<T>*, SmocPortOutPlug<T>*>(inPlug, outPlug);

  }

private:
  std::vector<InPortManager<ADDRESS>*>  inPortMangerVec;
  std::vector<OutPortManager<ADDRESS>*> outPortMangerVec;
};


/******************************************************************************
 *
 *
 */
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE = tlm::TLM_PASS_BY_POINTER>
class PortManagerAggregation :
  public sc_module,
  public ManagerCommitReadListener<ADDRESS>,//implement commitRead for managers
  public ManagerCommitWriteListener<ADDRESS>
{
public:
  SC_HAS_PROCESS(PortManagerAggregation);

  typedef StorageByteAccessIf::byteType   byteType;

  // TLM typedefs
  typedef tlm::tlm_request<ADDRESS, DATA, DATA_MODE>   request_type;
  typedef tlm::tlm_response<DATA, DATA_MODE>           response_type;

  typedef tlm::tlm_annotated_master_if<request_type,
                                       response_type>  tlm_master_if_type;
  typedef sc_port<tlm_master_if_type>                  tlm_master_port_type;

  typedef tlm::tlm_annotated_slave_if<request_type,
                                      response_type>   tlm_slave_if_type;
  typedef sc_port<tlm_slave_if_type>                   tlm_slave_port_type;


  // TLM ports
  tlm_master_port_type  masterPort;
  tlm_slave_port_type   slavePort;


  //
  PortManagerAggregation(sc_module_name name_,
                         const ADDRESS addrStart,
                         const ADDRESS addrEnd) :
    sc_module(name_),
    mCurrentInPortId(0),
    mCurrentOutPortId(0),
    mTransactionCount(0),
    mAddrStart(addrStart),
    mAddrEnd(addrEnd),
    mNextAddress(addrStart)
  {
    DBG_SCN_OUT("PortPlugAggregation::PortManagerAggregation(): my addresses: "
            << mAddrStart << "-" << mAddrEnd << ".\n");
  }

  //
  //virtual ~PortManagerAggregation(void) {}

  // TODO: is registering protocol dependend? I think so, because sometimes
  // there is need for meta-information or data access, somtimes not. Let
  // Aggregator implement many protocols and write specific registering
  // functions. Set protocol type in PortPlugInfos and assert equal protocl for
  // two transactors (managers) at end of elaboration.

  //
  void registerInPortManager(InPortManager<ADDRESS> *inPortManager)
  {
    DBG_SCN_OUT("PortPlugAggregation::registerInPortManager().\n");
    assert(inPortManager->isInitialized());
    inPortManager->getPortPlugInfos().id = mCurrentInPortId++;
    inPortManager->getPortPlugInfos().protocol = PushDataProtocol;
    // FIXME
    inPortManager->getPortPlugInfos().data = mAddrStart;
    inPortManager->registerManagerCommitReadListener(this);
    mInPlugs.push_back(inPortManager);
  }

  //
  void registerOutPortManager(OutPortManager<ADDRESS> *outPortManager)
  {
    DBG_SCN_OUT("PortPlugAggregation::registerOutPortManager().\n");
    assert(outPortManager->isInitialized());
    outPortManager->getPortPlugInfos().id = mCurrentOutPortId++;
    outPortManager->getPortPlugInfos().protocol = PushDataProtocol;
    outPortManager->registerManagerCommitWriteListener(this);
    mOutPlugs.push_back(outPortManager);
  }

  //
  void putBytes(const size_t pos, const byteType *a, const size_t n)
  {
    DBG_SCN_OUT("PortPlugAggregation::putBytes(): pos: " << pos << "; n = "
            << n << std::endl);
    assert(pos < getNumInPlugs());
    //StorageByteAccessIf &byteAccess = *(mDynamicPlugs[pos]->mStorage);
    StorageByteAccessIf &byteAccess = mInPlugs[pos]->getByteAccess();
    const size_t ret = byteAccess.putBytes(a, n);

    DBG_DOBJ(ret);
    assert(ret == n);

    mInPlugs[pos]->updateDataAvailableEvents();
  }

  //
  void getBytes(const size_t pos, byteType *a, const size_t n)
  {
    DBG_SCN_OUT("PortPlugAggregation::getBytes(): pos: " << pos << std::endl);
    assert(pos < getNumOutPlugs());
    StorageByteAccessIf &byteAccess = mOutPlugs[pos]->getByteAccess();
    const size_t ret = byteAccess.getBytes(a, n);
    assert(ret == n);

    mOutPlugs[pos]->updateSpaceAvailableEvents();
  }

  //
  size_t getNumInPlugs(void) const { return mInPlugs.size(); }

  //
  size_t getNumOutPlugs(void) const { return mOutPlugs.size(); }

  /*
   * ManagerCommitReadListener
   */
  void notifyCommitRead(const PortPlugInfos<ADDRESS> &myInfos,
                        const size_t consume)
  {
    DBG_SCN_OUT("PortPlugAggregation::commitRead(): id " << myInfos.id <<
            " called" << std::endl);
  }

  /*
   * ManagerCommitWriteListener
   */
  void notifyCommitWrite(const PortPlugInfos<ADDRESS> &myInfos,
                         const size_t produce)
  {
    assert(produce == 1);
    DBG_SCN_OUT("PortPlugAggregation::commitWrite(): id " << myInfos.id <<
            " called" << std::endl);
    masterSendRequest();
  }

private:
  // disabled
  PortManagerAggregation(const PortManagerAggregation &other) {}

  //
  void end_of_elaboration(void);

  //
  bool isMyAddress(const ADDRESS &address) const
  {
    return ((address >= mAddrStart) &&
            (address <= mAddrEnd));
  }

  //
  void masterSendRequest(void);
  
  //
  void masterGetResponse(void);

  //
  void slaveGetRequestMethod(void);
  
  std::vector<InPortManager<ADDRESS>*>   mInPlugs;
  std::vector<OutPortManager<ADDRESS>*>  mOutPlugs;
  int                           mCurrentInPortId;
  int                           mCurrentOutPortId;
  request_type                  mRequest;
  response_type                 mResponse;
  int                           mTransactionCount;
  const ADDRESS                 mAddrStart;
  const ADDRESS                 mAddrEnd;
  ADDRESS                       mNextAddress;
  sc_event                      mTrySendEvent;

  // FIXME
  ADDRESS                       mSendAddressHack;
};



//
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE>
void PortManagerAggregation<ADDRESS, DATA, DATA_MODE>::
      end_of_elaboration(void)
{
  SC_METHOD(slaveGetRequestMethod);
  sensitive << slavePort->ok_to_peek();
  dont_initialize();

  SC_METHOD(masterSendRequest);
  sensitive << mTrySendEvent; // << TODO: ok_to_put?
  dont_initialize();

  SC_METHOD(masterGetResponse);
  sensitive << masterPort->ok_to_peek();
  dont_initialize();

  mSendAddressHack = (mOutPlugs.front())->getSyblingPortPlugInfos().data;

  DBG_SCN_OUT("PortPlugAggregation::end_of_elaboration(): I will send to "
             << mSendAddressHack << std::endl);

  // FIXME: walk through all transactors and assert protocl of peer is equal
}



//
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE>
void PortManagerAggregation<ADDRESS, DATA, DATA_MODE>::
      masterSendRequest(void)
{
  if (!masterPort->nb_can_put()) {
    DBG_SCN_OUT("PortPlugAggregation::masterSendRequest(): can't " \
               "nb_put data. I try later.\n");
    //assert(0);
    // FIXME: remind send wish for specific port!
    mTrySendEvent.notify(sc_time(1, SC_NS));
    return;
  }

  mRequest.set_command(tlm::WRITE);
  mRequest.set_address(mSendAddressHack);
  mRequest.set_transaction_id(++mTransactionCount);

  // FIXME
  const size_t id = 0;

  DATA data;
  getBytes(id, (unsigned char *)&data, sizeof(data));
  mRequest.set_data(data);
  // ...

  if (!masterPort->nb_put(mRequest, sc_time(1, SC_NS))) {
    DBG_SCN_OUT("PortPlugAggregation::masterSendRequest(): can't " \
               "nb_put. Fatal!\n");
    assert(0);
  }
  mOutPlugs[id]->updateSpaceAvailableEvents();
}



//
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE>
void PortManagerAggregation<ADDRESS, DATA, DATA_MODE>::
      masterGetResponse(void)
{
  DBG_SCN_OUT("PortPlugAggregation::masterGetResponse()\n");

  if (!masterPort->nb_peek(mResponse)) {
    DBG_SCN_OUT("PortInAdapter::masterGetResponse(): can't nb_peek.\n");
    return;
  }

  // get data from channel
  if (!masterPort->nb_get(mResponse, sc_time(1, SC_NS))) {
    DBG_SCN_OUT("PortPlugAggregation::masterGetResponse(): can't " \
               "nb_get data. Fatal!\n");
    assert(0);
  }
}


//
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE>
void PortManagerAggregation<ADDRESS, DATA, DATA_MODE>::
      slaveGetRequestMethod(void)
{
  DBG_SCN_OUT("PortPlugAggregation::slaveGetRequestMethod()\n");

  // FIXME: don't overwrite request in progress
  if (!slavePort->nb_peek(mRequest)) {
    DBG_SCN_OUT("PortInAdapter::request_method(): can't nb_peek.\n");
    return;
  }

  // check if request is for us
  if (!isMyAddress(mRequest.get_address())) {
    DBG_SCN_OUT("PortInAdapter::request_method(): not my address: "
                << mRequest.get_address() << std::endl);
    return;
  }
  else {
    DBG_SCN_OUT("PortInAdapter::request_method(): is my address: "
                << mRequest.get_address() << std::endl);
  }

  if (!slavePort->nb_can_get()) {
    DBG_SCN_OUT("PortInAdapter::request_method(): ERROR: can't get!\n");
    return;
  }

  // get data from channel
  if (!slavePort->nb_get(mRequest, sc_time(1, SC_NS))) {
    DBG_SCN_OUT("PortPlugAggregation::slaveGetRequestMethod(): can't " \
               "nb_get data. Fatal!\n");
    assert(0);
  }

  // FIXME: look up in map
  const size_t currentManager = 0;
  assert(currentManager < mInPlugs.size());

  // FIXME: deserialize, burst, ... ? Better: store in char buffer, write at
  // one in storage
  DATA data = mRequest.get_data();

  //
  putBytes(currentManager, (unsigned char*)&data, sizeof(data));

  mResponse.set_transaction_id(mRequest.get_transaction_id());
  // ...

  // send response
  if (!slavePort->nb_put(mResponse, sc_time(1, SC_NS))) {
    DBG_SCN_OUT("PortPlugAggregation::slaveGetRequestMethod()(): can't " \
               "nb_put. Fatal!\n");
    assert(0);
  }

  mInPlugs[currentManager]->updateDataAvailableEvents();
}


#endif // MANAGERS_AGGREGATION
