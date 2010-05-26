#ifndef _INCLUDED_SMOC_REGISTER_HPP
#define _INCLUDED_SMOC_REGISTER_HPP

#include <systemoc/smoc_config.h>
#include <systemoc/smoc_multicast_sr_signal.hpp>

template <typename T>
class smoc_register
: public smoc_multicast_sr_signal_chan<T>::chan_init,
  public SysteMoC::Detail::ConnectProvider<
    smoc_register<T>,
    smoc_multicast_sr_signal_chan<T> > {
  typedef smoc_register<T> this_type;

  friend class SysteMoC::Detail::ConnectProvider<this_type, typename this_type::chan_type>;
public:
  typedef T                             data_type;
  typedef typename this_type::chan_type chan_type;
  typedef std::map<smoc_port_out_base_if*,sc_port_base*>  EntryMap;

private:
  chan_type *chan;
public:
  smoc_register( )
    : smoc_multicast_sr_signal_chan<T>::chan_init("", 1), chan(NULL)
  {  }

  explicit smoc_register( const std::string& name )
    : smoc_multicast_sr_signal_chan<T>::chan_init(name, 1), chan(NULL)
  {  }

  /// @brief Constructor
  smoc_register(const this_type &x)
    : smoc_register<T>::chan_init(x), chan(NULL)
  {  }  


  //method used for setting up the register-behaviour; has to be called after each connect(smoc_out_port<>)
  void enable_register(){
    chan->setSignalState(defined);
    for(EntryMap::const_iterator iter = getChan()->getEntries().begin();
      iter != getChan()->getEntries().end();
      ++iter)
    {
      smoc_multicast_entry<T> *entry = dynamic_cast<smoc_multicast_entry<T> *>( iter->first );
      assert(entry);
      entry->multipleWriteSameValue(true);
    }
  }


  this_type &connect(smoc_port_out<T> &p) {
    this_type* temp=&(SysteMoC::Detail::ConnectProvider<this_type, typename this_type::chan_type>::connect(p));
    enable_register();
    return *temp;
  }

  //QuickFIX needed to compile correctly.. but should be inhereted from ConnectProvider
  this_type &connect(smoc_port_in<T> &p) {
    return (SysteMoC::Detail::ConnectProvider<this_type, typename this_type::chan_type>::connect(p));
  }

  this_type &operator <<(typename this_type::add_param_ty x)
    { add(x); return *this; }

  /// Backward compatibility cruft
  this_type &operator <<(smoc_port_out<T> &p)
    { return this->connect(p); }
  this_type &operator <<(smoc_port_in<T> &p)
    { return this->connect(p); }
  template<class IFACE>
  this_type &operator <<(sc_port<IFACE> &p)
    { return this->connect(p); }
private:
  chan_type *getChan() {
    if (chan == NULL)
      chan = new chan_type(*this);
    return chan;
  }
};

#endif // _INCLUDED_SMOC_REGISTER_HPP
