// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Christian Zebelein <christian.zebelein@fau.de>
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Martin Letras <martin.letras@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SMOC_DETAIL_FIFOSTORAGE_HPP
#define _INCLUDED_SMOC_DETAIL_FIFOSTORAGE_HPP

#include <algorithm>

#include "DumpingInterfaces.hpp"
#include "QueueWithStorage.hpp"
#include "RingAccess.hpp"

#include "../../systemoc/detail/smoc_chan_if.hpp"

#include <systemoc/smoc_config.h>

#include <CoSupport/String/convert.hpp>

namespace smoc { namespace Detail {

/**
 * This class implements the data type specific
 * channel storage operations
 *
 * This is not merged with smoc_fifo_chan because
 * we need a void specialization (only for _some_
 * methods)
 */
template<class T, class BASE>
class FifoStorage
  : public smoc::Detail::QueueWithStorage<T, BASE>
  , public RingAccess<T, typename smoc_port_in_if<T>::access_type>
  , public RingAccess<T, typename smoc_port_out_if<T>::access_type>
{
  typedef FifoStorage<T, BASE>              this_type;
  typedef smoc::Detail::QueueWithStorage<T, BASE> base_type;
public:
  typedef typename base_type::storage_type storage_type;
protected:
  typedef RingAccess<T, typename smoc_port_in_if<T>::access_type>  access_in_type_impl;
  typedef RingAccess<T, typename smoc_port_out_if<T>::access_type> access_out_type_impl;

  /// @brief Channel initializer
  class chan_init: public BASE::chan_init {
  public:
    friend class FifoStorage;
    typedef const T& add_param_ty;

    void add(add_param_ty t)
      { marking.push_back(t); }
  protected:
    template <class P1, class P2>
    chan_init(const P1 &p1, const P2 &p2)
      : BASE::chan_init(p1, p2) {}
    template <class P1, class P2, class P3>
    chan_init(const P1 &p1, const P2 &p2, const P3 &p3)
      : BASE::chan_init(p1, p2, p3) {}
  private:
    std::vector<T> marking;
  };
protected:
  std::vector<T> initialTokens;
protected:

  /// @brief Constructor
  FifoStorage(const chan_init &i)
    : base_type(i),
      access_in_type_impl(this->storage, this->qfSize(), &this->rIndex()),
      access_out_type_impl(this->storage, this->qfSize(), &this->wIndex()),
      initialTokens(i.marking)
  {}

  void doReset() {
    // This resets the various pointers in the queue
    // (must precede call to wpp/vpp)
    this->resetQueue();

    // place initial tokens
    assert(this->depthCount() >= initialTokens.size());
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceInitialTokens(this, initialTokens.size(), this->depthCount());
#endif
    for (size_t j = 0; j < initialTokens.size(); ++j) {
      this->storage[j].put(initialTokens[j]);
    }
    this->wpp(initialTokens.size());
    this->vpp(initialTokens.size());

    BASE::doReset();
  }

  access_in_type_impl  *getReadPortAccess()  { return this; }
  access_out_type_impl *getWritePortAccess() { return this; }

  void invalidateTokenInStorage(size_t x){
    for(size_t i=0; i<x ; i++){
      size_t toRemove = this->vIndex();
      size_t rindex = this->rIndex();
      size_t n = (this->qfSize() + (toRemove - rindex)) % this->qfSize(); //number of tokens to be moved
      toRemove += this->qfSize();
      for(int i=n; i>=0 ;i--){
        this->storage[toRemove % this->qfSize()]=this->storage[(toRemove-1) % this->qfSize()];
        toRemove--;
      }
    }
  }

#ifdef SYSTEMOC_ENABLE_SGX
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  void dumpInitialTokens(smoc::Detail::IfDumpingInitialTokens *it) {
    it->setType(typeid(typename this_type::data_type).name());
    for (size_t n = 0; n < this->visibleCount(); ++n)
      it->addToken(CoSupport::String::asStr(this->storage[n].get()));
  }

  void resize(size_t n) {
    this->base_type::resize(n);
    // Also update ring access
    this->access_in_type_impl::resize(this->storage, this->qfSize());
    this->access_out_type_impl::resize(this->storage, this->qfSize());
  }
#endif // SYSTEMOC_ENABLE_SGX
};

/**
 * This class implements the data type specific
 * channel storage operations (void specialization)
 */
template<class BASE>
class FifoStorage<void, BASE>
  : public smoc::Detail::QueueWithStorage<void, BASE>
  , public RingAccess<void, smoc_port_in_if<void>::access_type>
  , public RingAccess<void, smoc_port_out_if<void>::access_type>
{
  typedef FifoStorage<void, BASE>               this_type;
  typedef smoc::Detail::QueueWithStorage<void, BASE>  base_type;
public:
  typedef typename base_type::storage_type storage_type;
protected:
  typedef RingAccess<void, smoc_port_in_if<void>::access_type>  access_in_type_impl;
  typedef RingAccess<void, smoc_port_out_if<void>::access_type> access_out_type_impl;

  /// @brief Channel initializer
  class chan_init: public BASE::chan_init {
  public:
    friend class FifoStorage;
    typedef size_t add_param_ty;

    void add(add_param_ty t)
      { marking += t; }
  protected:
    template <class P1, class P2>
    chan_init(const P1 &p1, const P2 &p2)
      : BASE::chan_init(p1, p2), marking(0) {}
    template <class P1, class P2, class P3>
    chan_init(const P1 &p1, const P2 &p2, const P3 &p3)
      : BASE::chan_init(p1, p2, p3), marking(0) {}
  private:
    size_t marking;
  };
private:
  size_t initialTokens;
protected:
  /// @brief Constructor
  FifoStorage(const chan_init &i)
    : base_type(i),
      initialTokens(i.marking)
  {}

  void doReset() {
    // This resets the various pointers in the queue
    // (must precede call to wpp/vpp)
    this->resetQueue();
    
    // place initial tokens
    assert(this->depthCount() >= initialTokens);
    this->wpp(initialTokens);
    this->vpp(initialTokens);
    
    BASE::doReset();
  }

  access_in_type_impl  *getReadPortAccess()  { return this; }
  access_out_type_impl *getWritePortAccess() { return this; }

  void invalidateTokenInStorage(size_t x){
    initialTokens += x;
  }

public:
#ifdef SYSTEMOC_ENABLE_SGX
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  void dumpInitialTokens(smoc::Detail::IfDumpingInitialTokens *it) {
    it->setType(typeid(typename this_type::data_type).name());
    for (size_t n = 0; n < this->visibleCount(); ++n)
      it->addToken("");
  }
#endif // SYSTEMOC_ENABLE_SGX
};

} } // smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_FIFOSTORAGE_HPP */
