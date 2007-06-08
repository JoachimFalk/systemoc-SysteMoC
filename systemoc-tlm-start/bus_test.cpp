

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

#include "managers_aggregation.h"

/******************************************************************************
 *
 *
 */
class A1 :
  public smoc_actor
{
public:
  smoc_port_in<long long>  in;
  smoc_port_out<int>  out;

  A1(sc_module_name name) :
    smoc_actor(name, start),
    m_fire_counter(0)
  {
    start =
      in(1)           >>
      out(1)          >>
      CALL(A1::func1) >> start;
  }

private:
  smoc_firing_state start;
  int               m_fire_counter;

  void func1(void)
  {
    DBG_SC_OUT("A1::func1(): got " << in[0] << std::endl);
    out[0] = ++m_fire_counter;
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
  smoc_port_in<int>  in;
  smoc_port_out<long long>  out;

  A2(sc_module_name name) :
    smoc_actor(name, start),
    m_fire_counter(0)
  {
    start =
      in(1)           >>
      out(1)          >>
      CALL(A2::func1) >> start;
  }

private:
  smoc_firing_state start;
  int               m_fire_counter;

  void func1(void)
  {
    DBG_SC_OUT("A2::func1(): got " << in[0] << std::endl);
    out[0] = ++m_fire_counter;
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
  PutModule(sc_module_name name, AdapterAggregation &managerAggregation) :
    sc_module(name),
    mManagerAggregation(managerAggregation)
  {}

private:
  typedef AdapterAggregation::byteType byteType;
  //
  void end_of_elaboration(void)
  {
    const long long j = 11;
    mManagerAggregation.putBytes(0,
                                 reinterpret_cast<const byteType*>(&j),
                                 sizeof(long long));
    const int i = 42;
    mManagerAggregation.putBytes(1,
                                 reinterpret_cast<const byteType*>(&i),
                                 sizeof(int));
  }

  AdapterAggregation &mManagerAggregation;
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
  Tlm_tester(sc_module_name name,
             AdapterAggregation &managerAggregation) :
    smoc_graph(name),
    a1("A1"),
    a2("A2")
  {
    InPortAdapter *inPortManager;
    OutPortAdapter *outPortManager;

    std::pair<InPortPlug<long long>*, OutPortPlug<long long>*> plugsA =
      mPortAdapterFactory.createManagerPair<long long>(2, 2,
                                                       &inPortManager,
                                                       &outPortManager);
    managerAggregation.registerInPortAdapter(inPortManager);
    managerAggregation.registerOutPortAdapter(outPortManager);

    connectChanPort(*(plugsA.first), a1.in);
    connectChanPort(*(plugsA.second), a2.out);

    std::pair<InPortPlug<int>*, OutPortPlug<int>*> plugsB =
      mPortAdapterFactory.createManagerPair<int>(2, 2,
                                                 &inPortManager,
                                                 &outPortManager);
    managerAggregation.registerInPortAdapter(inPortManager);
    managerAggregation.registerOutPortAdapter(outPortManager);

    connectChanPort(*(plugsB.first), a2.in);
    connectChanPort(*(plugsB.second), a1.out);
  }

private:
  A1 a1;
  A2 a2;
  PortAdapterFactory mPortAdapterFactory;
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

  AdapterAggregation plugAggregation;

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

