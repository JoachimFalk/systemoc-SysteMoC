//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_DETAIL_SMOC_FIFO_STORAGE_HPP
#define _INCLUDED_DETAIL_SMOC_FIFO_STORAGE_HPP

#include <systemoc/smoc_config.h>

#include <smoc/detail/DumpingInterfaces.hpp>

#include <CoSupport/String/convert.hpp>

#include "smoc_ring_access.hpp"

/**
 * This class implements the data type specific
 * channel storage operations
 *
 * This is not merged with smoc_fifo_chan because
 * we need a void specialization (only for _some_
 * methods)
 */
template<class T, class BASE>
class smoc_fifo_storage
: public BASE {
public:
  typedef T                       data_type;
  typedef smoc_storage<data_type> storage_type;

  typedef smoc_ring_access<
    typename smoc_storage_in<data_type>::storage_type,
    typename smoc_storage_in<data_type>::return_type
  > access_in_type_impl;
  typedef smoc_ring_access<
    typename smoc_storage_out<data_type>::storage_type,
    typename smoc_storage_out<data_type>::return_type
  > access_out_type_impl;

  /// @brief Channel initializer
  class chan_init: public BASE::chan_init {
  public:
    friend class smoc_fifo_storage;
    typedef T add_param_ty;

    void add(const add_param_ty &t)
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
  storage_type *storage;
  std::vector<T> initialTokens;
protected:

  /// @brief Constructor
  smoc_fifo_storage(const chan_init &i)
    : BASE(i),
      storage(new storage_type[this->fSize()]),
      initialTokens(i.marking)
  {}

  void doReset() {
    // This resets the various pointers in the queue
    // (must precede call to wpp/vpp)
    this->resetQueue();

    // place initial tokens
    assert(this->depthCount() >= initialTokens.size());
    for(size_t j = 0; j < initialTokens.size(); ++j) {
      storage[j].put(initialTokens[j]);
    }
    this->wpp(initialTokens.size());
    this->vpp(initialTokens.size());

    BASE::doReset();
  }

  /// @brief Destructor
  ~smoc_fifo_storage()
    { delete[] storage; }

  access_in_type_impl  *getReadPortAccess() {
    return new access_in_type_impl(
        storage, this->fSize(), &this->rIndex());
  }
  
  access_out_type_impl *getWritePortAccess() {
    return new access_out_type_impl(
        storage, this->fSize(), &this->wIndex());
  }
public:
#ifdef SYSTEMOC_ENABLE_SGX
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  void dumpInitalTokens(SysteMoC::Detail::IfDumpingInitialTokens *it) {
    it->setType(typeid(data_type).name());
    for (size_t n = 0; n < this->visibleCount(); ++n)
      it->addToken(CoSupport::String::asStr(storage[n].get()));
  }
#endif // SYSTEMOC_ENABLE_SGX
};

/**
 * This class implements the data type specific
 * channel storage operations (void specialization)
 */
template<class BASE>
class smoc_fifo_storage<void, BASE>
: public BASE {
public:
  typedef void data_type;

  typedef smoc_ring_access<
    typename smoc_storage_in<data_type>::storage_type,
    typename smoc_storage_in<data_type>::return_type
  > access_in_type_impl;
  typedef smoc_ring_access<
    typename smoc_storage_out<data_type>::storage_type,
    typename smoc_storage_out<data_type>::return_type
  > access_out_type_impl;

  /// @brief Channel initializer
  class chan_init: public BASE::chan_init {
  public:
    friend class smoc_fifo_storage;
    typedef size_t add_param_ty;

    void add(const add_param_ty &t)
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
  smoc_fifo_storage(const chan_init &i)
    : BASE(i),
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

  access_in_type_impl  *getReadPortAccess()
    { return new access_in_type_impl(); }
  
  access_out_type_impl *getWritePortAccess()
    { return new access_out_type_impl(); }
public:
#ifdef SYSTEMOC_ENABLE_SGX
  // FIXME: This should be protected for the SysteMoC user but accessible
  // for SysteMoC visitors
  void dumpInitalTokens(SysteMoC::Detail::IfDumpingInitialTokens *it) {
    it->setType(typeid(data_type).name());
  }
#endif // SYSTEMOC_ENABLE_SGX
};

#endif // _INCLUDED_DETAIL_SMOC_FIFO_STORAGE_HPP
