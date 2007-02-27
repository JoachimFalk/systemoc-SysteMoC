
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

#include "tlm.h"


enum e_colors {
  C_BLUE,
  C_RED
};


/******************************************************************************
 *
 *
 */
template<typename T>
class smoc_port_in_storage_write_if
{
public:
  virtual void setPos(size_t n, const T& t) = 0;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class smoc_port_in_storage_read_if
{
public:
  typedef smoc_channel_access<const smoc_storage<T>,
                              const T&>               read_access_type;
  typedef typename read_access_type::return_type      access_return_type;

  virtual read_access_type* getReadChannelAccess(void) = 0;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class smoc_port_in_storage :
  public smoc_port_in_storage_read_if<T>,
  public smoc_port_in_storage_write_if<T>
{
public:
  typedef typename smoc_port_in_storage_read_if<T>::read_access_type
    read_access_type;
  typedef typename smoc_port_in_storage_read_if<T>::access_return_type
    access_return_type;

  //
  smoc_port_in_storage(const size_t size = 1) :
    m_storage_vec(size),
    m_read_access_wrapper(m_storage_vec)
  {}

  // smoc_port_in_storage_read_if
  read_access_type* getReadChannelAccess(void)
  {
    return &m_read_access_wrapper;
  }

  // smoc_port_in_storage_write_if
  void setPos(size_t n, const T& t)
  {
    // FIXME
    m_storage_vec[n] = t;
  }

private:
  /*
   * wraps our storage
   */
  class read_access_wrapper :
    public read_access_type
  {
  public:
    read_access_wrapper(std::vector<smoc_storage<T> > &storage) :
      m_storage_vec(storage)
    {}

    // FIXME: set limit and assert in operator[]
    void setLimit(size_t l) {} 

    const access_return_type operator[](size_t n) const
    {
      assert(n < m_storage_vec.size());
      return m_storage_vec[n];
    }

    access_return_type operator[](size_t n)
    {
      assert(n < m_storage_vec.size());
      return m_storage_vec[n];
    }
    
    bool tokenIsValid(size_t n)
    {
      assert(n < m_storage_vec.size());
      return m_storage_vec[n].isValid();
    }

  private:
    std::vector<smoc_storage<T> > &m_storage_vec;
  };

  // local storage
  std::vector<smoc_storage<T> >  m_storage_vec;
  // and its wrapper object
  read_access_wrapper            m_read_access_wrapper;
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
    DBG_SC_OUT("SmocPortInPlug::commtRead(): got " << consume
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
 *
 *
 */
template<typename T>
class smoc_port_out_storage_read_if
{
public:
  virtual const T& operator[](size_t n) const = 0;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class smoc_port_out_storage_write_if
{
public:
  typedef smoc_channel_access<smoc_storage<T>,
                              smoc_storage_wom<T> >  write_access_type;
  typedef typename write_access_type::return_type    access_return_type;

  virtual write_access_type* getWriteChannelAccess(void) = 0;
};


/******************************************************************************
 *
 *
 */
template<typename T>
class smoc_port_out_storage :
  public smoc_port_out_storage_read_if<T>,
  public smoc_port_out_storage_write_if<T>
{
public:
  typedef typename smoc_port_out_storage_write_if<T>::write_access_type
    write_access_type;
  typedef typename smoc_port_out_storage_write_if<T>::access_return_type
    access_return_type;

  //
  smoc_port_out_storage(const size_t size = 1) :
    m_storage_vec(size),
    m_write_access_wrapper(m_storage_vec)
  {}

  // smoc_port_out_storage_write_if
  write_access_type* getWriteChannelAccess(void)
  {
    return &m_write_access_wrapper;
  }

  // smoc_port_out_storage_read_if
  const T& operator[](size_t n) const
  {
    assert(m_storage_vec[n].isValid());
    return m_storage_vec[n];
  }

private:
  /*
   * wraps our storage
   */
  class write_access_wrapper :
    public write_access_type
  {
  public:
    write_access_wrapper(std::vector<smoc_storage<T> > &storage) :
      m_storage_vec(storage)
    {}

    // FIXME: set limit and assert in operator[]
    void setLimit(size_t l) {} 

    const access_return_type operator[](size_t n) const
    {
      assert(n < m_storage_vec.size());
      return m_storage_vec[n];
    }

    access_return_type operator[](size_t n)
    {
      assert(n < m_storage_vec.size());
      return m_storage_vec[n];
    }
    
    bool tokenIsValid(size_t n)
    {
      assert(n < m_storage_vec.size());
      return m_storage_vec[n].isValid();
    }

  private:
    std::vector<smoc_storage<T> > &m_storage_vec;
  };

  // local storage
  std::vector<smoc_storage<T> >  m_storage_vec;
  // and its wrapper object
  write_access_wrapper           m_write_access_wrapper;
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
    DBG_SC_OUT("A1::func_a1(): read " << in1[0] << std::endl);
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
    DBG_SC_OUT("A2::func_a1(): read " << in1[0] << " " << in2[0] << std::endl);
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


