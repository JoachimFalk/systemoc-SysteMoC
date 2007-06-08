
#ifndef AGGREGATIONS_H
#define AGGREGATIONS_H

#include "adapters.h"
#include "storages.h"

#include "tlm.h"
#include "tlm_pvt_annotated_fifo.h"


// NOTE: for now only one adapter type, further others are possible (e.g., only
//  master or slave port)


/******************************************************************************
 *
 *
 */
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE = tlm::TLM_PASS_BY_POINTER>
class AdapterAggregation :
  public sc_module,
  public AdapterCommitReadListener<ADDRESS>,//implement commitRead for adapters
  public AdapterCommitWriteListener<ADDRESS>
{
public:
  SC_HAS_PROCESS(AdapterAggregation);

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
  AdapterAggregation(sc_module_name name_,
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
    DBG_SCN_OUT("PortPlugAggregation::AdapterAggregation(): my addresses: "
            << mAddrStart << "-" << mAddrEnd << ".\n");
  }

  //
  //virtual ~AdapterAggregation(void) {}

  // TODO: is registering protocol dependend? I think so, because sometimes
  // there is need for meta-information or data access, somtimes not. Let
  // Aggregator implement many protocols and write specific registering
  // functions. Set protocol type in PortPlugInfos and assert equal protocl for
  // two adapters at end of elaboration.

  //
  void registerInPortAdapter(InPortAdapter<ADDRESS> *inPortAdapter)
  {
    DBG_SCN_OUT("PortPlugAggregation::registerInPortAdapter().\n");
    assert(inPortAdapter->isInitialized());
    inPortAdapter->getPortPlugInfos().id = mCurrentInPortId++;
    inPortAdapter->getPortPlugInfos().protocol = PushDataProtocol;
    // FIXME
    inPortAdapter->getPortPlugInfos().data = mAddrStart;
    inPortAdapter->registerAdapterCommitReadListener(this);
    mInPlugs.push_back(inPortAdapter);
  }

  //
  void registerOutPortAdapter(OutPortAdapter<ADDRESS> *outPortAdapter)
  {
    DBG_SCN_OUT("PortPlugAggregation::registerOutPortAdapter().\n");
    assert(outPortAdapter->isInitialized());
    outPortAdapter->getPortPlugInfos().id = mCurrentOutPortId++;
    outPortAdapter->getPortPlugInfos().protocol = PushDataProtocol;
    outPortAdapter->registerAdapterCommitWriteListener(this);
    mOutPlugs.push_back(outPortAdapter);
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
   * AdapterCommitReadListener
   */
  void notifyCommitRead(const PortPlugInfos<ADDRESS> &myInfos,
                        const size_t consume)
  {
    DBG_SCN_OUT("PortPlugAggregation::commitRead(): id " << myInfos.id <<
            " called" << std::endl);
  }

  /*
   * AdapterCommitWriteListener
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
  AdapterAggregation(const AdapterAggregation &other) {}

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
  
  std::vector<InPortAdapter<ADDRESS>*>   mInPlugs;
  std::vector<OutPortAdapter<ADDRESS>*>  mOutPlugs;
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
void AdapterAggregation<ADDRESS, DATA, DATA_MODE>::
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

  // FIXME: walk through all adapters and assert protocl of peer is equal
}



//
template <typename ADDRESS,
          typename DATA,
          tlm::tlm_data_mode DATA_MODE>
void AdapterAggregation<ADDRESS, DATA, DATA_MODE>::
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
void AdapterAggregation<ADDRESS, DATA, DATA_MODE>::
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
void AdapterAggregation<ADDRESS, DATA, DATA_MODE>::
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
  const size_t currentAdapter = 0;
  assert(currentAdapter < mInPlugs.size());

  // FIXME: deserialize, burst, ... ? Better: store in char buffer, write at
  // one in storage
  DATA data = mRequest.get_data();

  //
  putBytes(currentAdapter, (unsigned char*)&data, sizeof(data));

  mResponse.set_transaction_id(mRequest.get_transaction_id());
  // ...

  // send response
  if (!slavePort->nb_put(mResponse, sc_time(1, SC_NS))) {
    DBG_SCN_OUT("PortPlugAggregation::slaveGetRequestMethod()(): can't " \
               "nb_put. Fatal!\n");
    assert(0);
  }

  mInPlugs[currentAdapter]->updateDataAvailableEvents();
}



#endif // AGGREGATIONS_H
