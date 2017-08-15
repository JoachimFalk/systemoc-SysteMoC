//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SMOC_SMOC_REGISTER_HPP
#define _INCLUDED_SMOC_SMOC_REGISTER_HPP

#include "detail/ConnectProvider.hpp"
#include "detail/Chan.hpp"

#include <boost/noncopyable.hpp>

namespace smoc {

/// Base class for the smoc_register channel implementation.
class smoc_register_chan_base
  : public smoc::Detail::Chan
  , private boost::noncopyable
{
  typedef smoc_register_chan_base  this_type;
public:
  friend class smoc_register_outlet_base;
  friend class smoc_register_entry_base;

  /// @brief Channel initializer
  class chan_init {
  public:
    friend class smoc_register_chan_base;
  protected:
    chan_init(const std::string &name, size_t n);
  private:
    std::string name;
    size_t      n;
  };

protected:
  // constructors
  smoc_register_chan_base(chan_init const &i);

  size_t inTokenId() const
    { return tokenId; }

  size_t outTokenId() const
    { return tokenId; }
private:
  /// @brief The tokenId of the next commit token
  size_t tokenId;
};

template <typename T> class smoc_register_chan;

class smoc_register_outlet_base {
protected:
  smoc_register_outlet_base(smoc_register_chan_base *chan);

#ifdef SYSTEMOC_ENABLE_VPC
  /// @brief See PortInBaseIf
  void commitRead(size_t consume, smoc::smoc_vpc_event_p const &readConsumeEvent);
#else
  /// @brief See PortInBaseIf
  void commitRead(size_t consume);
#endif

  smoc_register_chan_base *chan;
  smoc_event               trueEvent;
};

template <typename T>
class smoc_register_outlet
  : public smoc_register_outlet_base
  , public smoc_port_in_if<T,::smoc_1d_port_access_if>
  , public smoc_1d_port_access_if<typename smoc::Detail::StorageTraitsIn<T>::return_type>
{
  typedef smoc_register_outlet<T>                      this_type;
public:
  typedef T                                            data_type;
  typedef smoc::Detail::Storage<data_type>             storage_type;
  typedef typename this_type::access_in_type           ring_in_type;
  typedef typename this_type::return_type              return_type;
  typedef smoc_port_in_if<T,::smoc_1d_port_access_if>  iface_type;

  /// @brief Constructor
  smoc_register_outlet(smoc_register_chan<T> *chan)
    : smoc_register_outlet_base(chan) {}

  // Interface independent of T but needed here in order to override pure virtual methods.
#ifdef SYSTEMOC_ENABLE_VPC
  /// @brief See PortInBaseIf
  void commitRead(size_t consume, smoc::smoc_vpc_event_p const &readConsumeEvent)
    { smoc_register_outlet_base::commitRead(consume, readConsumeEvent); }
#else
  /// @brief See PortInBaseIf
  void commitRead(size_t consume)
    { smoc_register_outlet_base::commitRead(consume); }
#endif

  /// @brief See PortInBaseIf
  std::string getChannelName() const
    { return chan->name();}

  /// @brief See PortInBaseIf
  size_t numAvailable() const
    { return 1; }

  /// @brief See PortInBaseIf
  smoc::smoc_event &dataAvailableEvent(size_t n) {
    assert(n <= 1);
    return trueEvent;
  }

  /// @brief See PortInBaseIf
  void moreData(size_t) {}

  /// @brief See PortInBaseIf
  void lessData(size_t) {}

///// @brief See PortInBaseIf
//size_t inTokenId() const
//  { return chan->inTokenId(); }

  /// @brief See smoc_1d_port_access_if
  bool tokenIsValid(size_t n) const
    { assert(n == 0); return static_cast<smoc_register_chan<T> *>(chan)->isValid(); }

  /// @brief See smoc_1d_port_access_if
  void setLimit(size_t l) {}

  // Interfaces depending on T.

  /// @brief See smoc_port_in_if
  ring_in_type *getReadPortAccess()
    { return this; }

  /// @brief See smoc_1d_port_access_if
  return_type operator[](size_t n)
    { assert(n == 0); return static_cast<smoc_register_chan<T> *>(chan)->actualValue; }

  /// @brief See smoc_1d_port_access_if
  const return_type operator[](size_t n) const
    { assert(n == 0); return static_cast<smoc_register_chan<T> *>(chan)->actualValue; }
};

class smoc_register_entry_base {
protected:
  smoc_register_entry_base(smoc_register_chan_base *chan);

#ifdef SYSTEMOC_ENABLE_VPC
  /// @brief See PortOutBaseIf
  void commitWrite(size_t produce, smoc::Detail::VpcInterface vpcIf);
#else
  /// @brief See PortOutBaseIf
  void commitWrite(size_t produce);
#endif

  smoc_register_chan_base *chan;
  smoc_event               trueEvent;
};

template <typename T>
class smoc_register_entry
  : public smoc_register_entry_base
  , public smoc_port_out_if<T,::smoc_1d_port_access_if>
  , public smoc_1d_port_access_if<typename smoc::Detail::StorageTraitsInOut<T>::return_type>
{
  typedef smoc_register_entry<T>                        this_type;
public:
  typedef T                                             data_type;
  typedef smoc::Detail::Storage<data_type>              storage_type;
  typedef typename this_type::access_out_type           ring_out_type;
  typedef typename this_type::return_type               return_type;
  typedef smoc_port_out_if<T,::smoc_1d_port_access_if>  iface_type;

  /// @brief Constructor
  smoc_register_entry(smoc_register_chan<T> *chan)
    : smoc_register_entry_base(chan) {}

  // Interface independent of T but needed here in order to override pure virtual methods.

#ifdef SYSTEMOC_ENABLE_VPC
  /// @brief See PortOutBaseIf
  void commitWrite(size_t produce, smoc::Detail::VpcInterface vpcIf)
    { smoc_register_entry_base::commitWrite(produce, vpcIf); }
#else
  /// @brief See PortOutBaseIf
  void commitWrite(size_t produce)
    { smoc_register_entry_base::commitWrite(produce); }
#endif

  /// @brief See PortOutBaseIf
  std::string getChannelName() const
    { return chan->name();}

  /// @brief See PortOutBaseIf
  smoc::smoc_event &spaceAvailableEvent(size_t n) {
    assert(n <= 1);
    return trueEvent;
  }

  /// @brief See PortOutBaseIf
  size_t numFree() const
    { return 1; }

///// @brief See PortOutBaseIf
//size_t outTokenId() const
//  { return chan->outTokenId(); }

  /// @brief See PortOutBaseIf
  void moreSpace(size_t) {}

  /// @brief See PortOutBaseIf
  void lessSpace(size_t) {}

  /// @brief See smoc_port_access_base_if
  bool tokenIsValid(size_t n) const
    { assert(n == 0); return static_cast<smoc_register_chan<T> *>(chan)->isValid(); }

  /// @brief See smoc_port_access_base_if
  void setLimit(size_t l) {}

  // Interfaces depending on T.

  /// @brief See smoc_port_out_if
  ring_out_type *getWritePortAccess()
    { return this; }

  /// @brief See smoc_1d_port_access_if
  return_type operator[](size_t n)
    { assert(n == 0); return static_cast<smoc_register_chan<T> *>(chan)->actualValue; }

  /// @brief See smoc_1d_port_access_if
  const return_type operator[](size_t n) const
    { assert(n == 0); return static_cast<smoc_register_chan<T> *>(chan)->actualValue; }
};

template <typename T>
class smoc_register_chan
  : public smoc_register_chan_base
{
  typedef smoc_register_chan<T>             this_type;

  friend class smoc_register_outlet<T>;
  friend class smoc_register_entry<T>;
public:
  typedef T                                 data_type;
  typedef smoc::Detail::Storage<data_type>  storage_type;
  typedef smoc_register_outlet<data_type>   outlet_type;
  typedef smoc_register_entry<data_type>    entry_type;

  /// @brief Channel initializer
  class chan_init
    : public smoc_register_chan_base::chan_init {
    friend class smoc_register_chan<T>;
  private:
    //FIXME(MS): replace with signal value wrapper
    std::vector<T>  marking;
  protected:
    typedef const T &add_param_ty;
  public:
    void add(add_param_ty x) {
      assert(marking.empty());
      marking.push_back(x);
    }
  protected:
    chan_init(std::string const &name, size_t n)
      : smoc_register_chan_base::chan_init(name, n) {}
  };

  /// @brief Constructor
  smoc_register_chan(const chan_init &i)
    : smoc_register_chan_base(i)
  {
    assert(i.marking.size() <= 1);
    if (i.marking.size() > 0)
      actualValue.put(i.marking[0]);
  }
protected:
  storage_type actualValue;

  /// @brief See smoc_port_registry
  smoc::Detail::PortOutBaseIf *createEntry()
    { return new entry_type(this); }

  /// @brief See smoc_port_registry
  smoc::Detail::PortInBaseIf *createOutlet()
    { return new outlet_type(this); }

  void setChannelID( std::string sourceActor,
                             CoSupport::SystemC::ChannelId id,
                             std::string name ){
    this->actualValue.setChannelID(sourceActor, id, name);
  }

  /// @See smoc_root_chan
  virtual void doReset() {
    smoc_register_chan_base::doReset();
    actualValue.invalidate();
  }

  bool isValid() const
    { return actualValue.isValid(); }
};

template <typename T>
class smoc_register
  : public smoc_register_chan<T>::chan_init
  , public smoc::Detail::ConnectProvider<
      smoc_register<T>,
      smoc_register_chan<T> >
{
  typedef smoc_register<T>                          this_type;
  typedef typename smoc_register_chan<T>::chan_init base_type;

  friend class smoc::Detail::ConnectProvider<this_type, typename this_type::chan_type>;
public:
  typedef T                             data_type;
  typedef typename this_type::chan_type chan_type;
private:
  chan_type *chan;
public:
  /// @brief Constructor
  smoc_register()
    : base_type("", 1), chan(nullptr)
  {}

  /// @brief Constructor
  explicit smoc_register(const std::string &name)
    : base_type(name, 1), chan(nullptr)
  {}

  /// @brief Constructor
  smoc_register(const this_type &x)
    : base_type(x), chan(nullptr)
  {
    if(x.chan)
      assert(!"Can't copy initializer: Channel already created!");
  }

  this_type &operator<<(typename this_type::add_param_ty x) {
    if(chan)
      assert(!"Can't place initial token: Channel already created!");
    this->add(x);
    return *this;
  }
private:
  chan_type *getChan() {
    if (chan == nullptr)
      chan = new chan_type(*this);
    return chan;
  }

  // disable
  this_type &operator =(const this_type &);
};

} // namespace smoc

#endif // _INCLUDED_SMOC_SMOC_REGISTER_HPP
