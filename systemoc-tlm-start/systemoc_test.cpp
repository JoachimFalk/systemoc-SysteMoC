
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include "debug_on.h"


enum e_colors {
  C_BLUE,
  C_RED
};


#if 0
/******************************************************************************
 *
 *
 */
template<class C>
class ChannelInAccess :
  public smoc_channel_access<const smoc_storage<C>, const C&>
{
public:
  typedef C                                           data_type;
  typedef smoc_channel_access<const smoc_storage<C>,
                              const C&>               this_type;
  typedef typename
    smoc_channel_access<const smoc_storage<C>,
                        const C&>::return_type        return_type;

  // implement obsolete function because interface demands it...
  void setLimit(size_t l) {}

  //
  return_type operator[](size_t n)
  {
    DBG_SC_OUT("ChannelInAccess::operator[]: n = " << n << std::endl);
    assert(n == 0);
    return m_storage;
  }
  
  //
  const return_type operator[](size_t n) const
  {
    assert(0);
  }

  //
  bool isValid(size_t i) { return true; }

  //
  void setData(C c) { m_storage = c; }

  //
  C getData(void) const { return m_storage; }
  

private:
  smoc_storage<C> m_storage;
};


/******************************************************************************
 *
 *
 */
template<class C>
class ChannelOutAccess :
  public smoc_channel_access<smoc_storage<C>, smoc_storage_wom<C> >
{
public:
  typedef C                                                 data_type;
  typedef smoc_channel_access<smoc_storage<C>,
                              smoc_storage_wom<C> >         this_type;
  typedef typename
    smoc_channel_access<smoc_storage<C>,
                        smoc_storage_wom<C> >::return_type  return_type;

  // implement obsolete function because interface demands it...
  void setLimit(size_t l) {}

  //
  return_type operator[](size_t n)
  {
    DBG_SC_OUT("ChannelOutAccess::operator[]: n = " << n << std::endl);
    assert(n == 0);
    return m_storage;
  }
  
  //
  const return_type operator[](size_t n) const
  {
    assert(0);
  }

  //
  bool isValid(size_t i)
  {
    assert(i == 0);
    return m_storage.isValid();
  }

  //
  C getData(void) const { return m_storage; }

private:
  smoc_storage<C> m_storage;
};
#endif


/******************************************************************************
 *
 * Adapter sc_fifo -> smoc_port_in
 */
template<typename T>
class InPortAdapter :
  public smoc_chan_in_if<T, smoc_channel_access>,
  public smoc_channel_access<const smoc_storage<T>, const T&>,
  public sc_module
{
public:
  typedef InPortAdapter<T>                      this_type;
  typedef typename this_type::access_in_type    access_in_type;
  typedef typename access_in_type::return_type  return_type;

  // connect sc_fifo to this port
  sc_fifo_in<T> fifo_in;

  //
  InPortAdapter(sc_module_name name) :
    sc_module(name),
    m_read_new_data(true)
  {
    SC_HAS_PROCESS(InPortAdapter);
    SC_THREAD(process);
  }

  /*
   * smoc_chan_in_if implementation
   */
  //
  size_t numAvailable() const
  {
    DBG_SC_OUT("committedOutCount():\n");
    if (fifo_in.num_available() > 0)
      return 1;
    else
      return 0;
  }

  //
  smoc_event &dataAvailableEvent(size_t n)
  {
    DBG_SC_OUT("blockEventOut():\n");
    assert(n == 1);
    return event_data_available;
  }

  //
  access_in_type *getWriteChannelAccess(void)
  {
    DBG_SC_OUT("accessSetupIn():\n");
    return this;
  }

  //
  void commitRead(size_t consume)
  {
    DBG_SC_OUT("OutAdapter::commExecIn(): read " << consume << " data.\n");
    DBG_DOBJ(fifo_in.num_available());

    assert(consume == 1);

    // reset data available event if fifo is empty
    if (fifo_in.num_available() == 0)
      event_data_available.reset();

    m_read_new_data = true;
  }

  /*
   * smoc_channel_access implementation
   */
  // implement obsolete function because interface demands it...
  void setLimit(size_t l) {}

  //
  return_type operator[](size_t n)
  {
    DBG_SC_OUT("ChannelInAccess::operator[]: n = " << n << std::endl);
    assert(n == 0);

    // guard read access to allow multiple reading the same value
    if (m_read_new_data) {
      m_storage = fifo_in.read();
      m_read_new_data = false;
    }
    return m_storage;
  }
  
  //
  const return_type operator[](size_t n) const
  {
    assert(0);
  }

  //
  bool tokenIsValid(size_t i)
  {
    DBG_SC_OUT("InPortAdapter::isValid(): i == " << i << std::endl);
    assert(i == 0);
    return true;
  }

  /*
   * module implementation
   */
  //
  void setData(T t) { m_storage = t; }

  //
  T getData(void) const { return m_storage; }

private:
  // smoc_chan_in_if interface
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  //
  void process(void)
  {
    while (1) {
      if (fifo_in.num_available() == 0)
        wait(fifo_in.data_written_event());

      //DBG_SC_OUT("InAdapter::process(): Data: "
      //           << getData() << std::endl);
      event_data_available.notify();
      wait(1, SC_PS);
    }
  }

  smoc_event event_data_available;
  smoc_storage<T> m_storage;
  bool m_read_new_data;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class OutPortAdapter :
  public smoc_chan_out_if<T, smoc_channel_access>,
  public smoc_channel_access<smoc_storage<T>, smoc_storage_wom<T> >,
  public sc_module
{
public:
  typedef OutPortAdapter<T>                      this_type;
  typedef typename this_type::access_out_type    access_out_type;
  typedef typename access_out_type::return_type  return_type;

  // connect sc_fifo to this port
  sc_fifo_out<T> fifo_out;

  //
  OutPortAdapter(sc_module_name name) :
    sc_module(name)
  {
    SC_HAS_PROCESS(OutPortAdapter);
    SC_THREAD(process);
  }

  /*
   * smoc_chan_out_if implementation
   */
  // what does committedInCount mean? FIFO returns unusedStorage.
  //
  size_t numFree() const
  {
    DBG_SC_OUT("committedInCount():\n");
    return fifo_out.num_free();
  }

  //
  smoc_event &spaceAvailableEvent(size_t n)
  {
    DBG_SC_OUT("blockEventIn():\n");
    assert(n == 1);
    return event_space_available;
  }

  //
  access_out_type *getReadChannelAccess(void)
  {
    DBG_SC_OUT("accessSetupOut():\n");
    return this;
  }

  //
  void commitWrite(size_t produce)
  {
    DBG_SC_OUT("OutAdapter::commExecOut(): got " << produce << " data.\n");

    assert(produce == 1);
    assert(fifo_out.num_free() > 0);

    fifo_out.write(m_storage);

    if (fifo_out.num_free() == 0)
      event_space_available.reset();
  }

  /*
   * smoc_channel_access implementation
   */
  // implement obsolete function because interface demands it...
  void setLimit(size_t l) {}

  //
  return_type operator[](size_t n)
  {
    DBG_SC_OUT("ChannelOutAccess::operator[]: n = " << n << std::endl);
    assert(n == 0);

    return m_storage;
  }
  
  //
  const return_type operator[](size_t n) const
  {
    assert(0);
  }

  //
  bool tokenIsValid(size_t i)
  {
    DBG_SC_OUT("OutPortAdapter::isValid(): i == " << i << std::endl);
    assert(i == 0);
    return true;
  }
  
  /*
   * module implementation
   */
  //
  T getData(void) const { return m_storage; }

private:
  //
  const sc_event& default_event() const { return smoc_default_event_abort(); };

  //
  void process(void)
  {
    while (1) {
      /*if (isValid(0))
        DBG_SC_OUT("OutAdapter::process(): Data: "
                   << getData() << std::endl);
      else
        DBG_SC_OUT("OutAdapter::process(): Data not valid.\n");
      */

      if (fifo_out.num_free() == 0)
        wait(fifo_out.data_read_event());

      event_space_available.notify();
      wait(1, SC_PS);
    }
  }

  smoc_event event_space_available;
  smoc_storage<T> m_storage;
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
  smoc_port_out<int> out2;

  A1(sc_module_name name) :
    smoc_actor(name, start)
  {
    start =
        // a1
        (in1(1) && GUARD(A1::check_red))  >>
        (out1(1) && out2(1))              >>
        CALL(A1::func_a1)                 >> start;
  }

private:
  smoc_firing_state start;

  bool check_red(void) const { return true; }
  
  bool check_blue(void) const { return true; }

  void func_a1(void)
  {
    DBG_SC_OUT("A1: func_a1()\n");
    out1[0] = in1[0];
    out2[0] = in1[0];
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
  smoc_port_in<int>  in2;

  A2(sc_module_name name) :
    smoc_actor(name, start)
  {
    start =
        // a1
        (in1(1) && in2(1) && GUARD(A2::check_red))  >>
        out1(1)                                     >>
        CALL(A2::func_a1)                           >> start;
  }

private:
  smoc_firing_state start;

  bool check_red(void) const { return true; }
  
  bool check_blue(void) const { return true; }

  void func_a1(void)
  {
    const int i = in1[0] + in2[0];
    DBG_SC_OUT("A2::func_a1(): write " << i << std::endl);
    out1[0] = i;
  }
};


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
    connectNodePorts(a1.out1, a2.in1, smoc_fifo<int>(8) << 42);
    connectNodePorts(a2.out1, a1.in1, smoc_fifo<int>(8));
    connectChanPort(outAdapter, a1.out2);
    connectChanPort(inAdapter, a2.in2);

    outAdapter.fifo_out.bind(outAdapter_out);
    inAdapter.fifo_in.bind(inAdapter_in);
  }
private:
  A1 a1;
  A2 a2;
  OutPortAdapter<int> outAdapter;
  InPortAdapter<int>  inAdapter;

public:
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
  Dumper dumper("dumper");

  smoc_top_moc<Schedule_tester> schedule_tester("schedule_tester");
  sc_fifo<int> fifo;

  schedule_tester.outAdapter_out(fifo);
  schedule_tester.inAdapter_in(fifo);
  fifo.write(13);

#ifndef KASCPAR_PARSING  
  if (argc > 1 && 0 == strncmp(argv[1],
                               "--generate-problemgraph",
                               sizeof("--generate-problemgraph")))
  {
    smoc_modes::dump(std::cout, schedule_tester);
  }
  else {
    sc_start(10, SC_PS);
  }
#endif  
  return 0;
}
