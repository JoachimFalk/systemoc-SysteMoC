// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#ifndef _INCLUDED_SMOC_DETAIL_QUEUEWITHSTORAGE_HPP
#define _INCLUDED_SMOC_DETAIL_QUEUEWITHSTORAGE_HPP

#include "Storage.hpp"

#include <boost/type_traits/is_base_of.hpp>

namespace smoc { namespace Detail {

template<class T, class BASE>
class QueueWithStorageHelper: public BASE {
  typedef QueueWithStorageHelper<T, BASE> this_type;
  typedef BASE                            base_type;
public:
  typedef T                               data_type;
  typedef Storage<data_type>              storage_type;
protected:
  storage_type *storage;
protected:
  /// @brief Constructors
  QueueWithStorageHelper()
    : BASE(),
      storage(new storage_type[this->qfSize()])
    {}
  /// @brief Constructors
  template <class P1>
  QueueWithStorageHelper(const P1 &p1)
    : BASE(p1),
      storage(new storage_type[this->qfSize()])
    {}
  /// @brief Constructors
  template <class P1, class P2>
  QueueWithStorageHelper(const P1 &p1, const P2 &p2)
    : BASE(p1, p2),
      storage(new storage_type[this->qfSize()])
    {}
  /// @brief Constructors
  template <class P1, class P2, class P3>
  QueueWithStorageHelper(const P1 &p1, const P2 &p2, const P3 &p3)
    : BASE(p1, p2, p3),
      storage(new storage_type[this->qfSize()])
    {}

  /// Remove tokens given by generator g. Move remaining tokens to the right.
  /// Return number of removed tokens.
  template <class G>
  int moveTokensRight(G g) {
    int npos;
    int srcPos = g.popMax();
    int dstPos = srcPos;
    while (srcPos != -1) {
      storage[(srcPos + this->rIndex()) % this->qfSize()].invalidate();
      for (npos = g.popMax(), --srcPos; srcPos > npos; --srcPos, --dstPos) {
        storage[(dstPos + this->rIndex()) % this->qfSize()].put
          (storage[(srcPos + this->rIndex()) % this->qfSize()].get());
        storage[(srcPos + this->rIndex()) % this->qfSize()].invalidate();
      }
    }
    return dstPos+1;
  }

  /// Overload rpp from QueueRWPtr to implement destructor call for consumed
  /// tokens
  void rpp(size_t n) {
    size_t rindex = this->rIndex();
    size_t o = (std::min)(n, this->qfSize() - rindex);
    size_t p = n-o; o += rindex;
    for (;rindex < o; ++rindex)
      storage[rindex].invalidate();
    assert(p == 0 || rindex == this->qfSize());
    for (rindex = 0; rindex < p; ++rindex)
      storage[rindex].invalidate();
    base_type::rpp(n);
  }

  /// Consume tokens in the visible area given by generator g.
  template <class G>
  void rpp(G const &g)
    { base_type::rpp(moveTokensRight(g)); }

  /// Drop tokens in the visible area given by generator g. Update findex and
  /// rindex assuming that the tokens in the queue are moved to the right.
  template <class G>
  void dropRVisible(G const &g)
    { base_type::dropRVisible(moveTokensRight(g)); }

  /// Overload resetQueue from QueueRWPtr to implement destructor call for
  /// tokens on queue reset
  void resetQueue() {
    size_t rindex = this->rIndex();
    size_t o = this->rIndex() <= this->wIndex()
      ? this->wIndex() : this->qfSize();
    size_t p = this->rIndex() <= this->wIndex()
      ? 0 : this->wIndex();
    for (;rindex < o; ++rindex)
      storage[rindex].invalidate();
    assert(p == 0 || rindex == this->qfSize());
    for (rindex = 0; rindex < p; ++rindex)
      storage[rindex].invalidate();
    base_type::resetQueue();
  }

  void resize(size_t n) {
    assert(!"Implement this!");
    base_type::resize(n);
  }

  /// @brief Destructor
  ~QueueWithStorageHelper()
    { delete[] storage; }
};

/**
 * Provide void specialization for the special case that no elements are stored
 * in QueueWithStorageHelper.
 */
template<class BASE>
class QueueWithStorageHelper<void, BASE>: public BASE {
  typedef QueueWithStorageHelper<void, BASE>  this_type;
  typedef BASE                                base_type;
public:
  typedef void                                data_type;
  typedef void                                storage_type;
protected:
  /// @brief Constructors
  QueueWithStorageHelper()
    : BASE()
    {}
  /// @brief Constructors
  template <class P1>
  QueueWithStorageHelper(const P1 &p1)
    : BASE(p1)
    {}
  /// @brief Constructors
  template <class P1, class P2>
  QueueWithStorageHelper(const P1 &p1, const P2 &p2)
    : BASE(p1, p2)
    {}
  /// @brief Constructors
  template <class P1, class P2, class P3>
  QueueWithStorageHelper(const P1 &p1, const P2 &p2, const P3 &p3)
    : BASE(p1, p2, p3)
    {}

  /// Remove tokens given by generator g. Move remaining tokens to the right.
  /// Return number of removed tokens.
  template <class G>
  int moveTokensRight(G g)
    { return g.count(); }

  /// Drop tokens in the visible area given by generator g. Update findex and
  /// rindex assuming that the tokens in the queue are moved to the right.
  template <class G>
  void dropRVisible(G const &g)
    { base_type::dropRVisible(moveTokensRight(g)); }
};

class QueueRVWPtr;

/**
 * This class implements the storage to hold elements of type T for a ring
 * buffer queue. The base class given by the BASE template parameter must be
 * derived from one of the three queue index classes, i.e., QueueRWPtr,
 * QueueRVWPtr, or QueueFRVWPtr.
 */
template<class T, class BASE,
         bool queueHasInvisibleArea = boost::is_base_of<QueueRVWPtr, BASE>::value>
class QueueWithStorage: public QueueWithStorageHelper<T, BASE> {
  typedef QueueWithStorage<T, BASE, queueHasInvisibleArea>  this_type;
  typedef QueueWithStorageHelper<T, BASE>                   base_type;
protected:
  /// @brief Constructors
  QueueWithStorage()
    : base_type()
    {}
  /// @brief Constructors
  template <class P1>
  QueueWithStorage(const P1 &p1)
    : base_type(p1)
    {}
  /// @brief Constructors
  template <class P1, class P2>
  QueueWithStorage(const P1 &p1, const P2 &p2)
    : base_type(p1, p2)
    {}
  /// @brief Constructors
  template <class P1, class P2, class P3>
  QueueWithStorage(const P1 &p1, const P2 &p2, const P3 &p3)
    : base_type(p1, p2, p3)
    {}
};

template<class T, class BASE>
class QueueWithStorage<T, BASE, true>: public QueueWithStorageHelper<T, BASE> {
  typedef QueueWithStorage<T, BASE, true>                   this_type;
  typedef QueueWithStorageHelper<T, BASE>                   base_type;
protected:
  /// @brief Constructors
  QueueWithStorage()
    : base_type()
    {}
  /// @brief Constructors
  template <class P1>
  QueueWithStorage(const P1 &p1)
    : base_type(p1)
    {}
  /// @brief Constructors
  template <class P1, class P2>
  QueueWithStorage(const P1 &p1, const P2 &p2)
    : base_type(p1, p2)
    {}
  /// @brief Constructors
  template <class P1, class P2, class P3>
  QueueWithStorage(const P1 &p1, const P2 &p2, const P3 &p3)
    : base_type(p1, p2, p3)
    {}

  /// Drop tokens in the invisible area given by generator g. Update findex,
  /// rindex and vindex assuming that the tokens in the queue are moved to the
  /// right.
  template <class G>
  void dropRInvisible(G const &g) {
    base_type::dropRInvisible(this->moveTokensRight(DropRInvisibleWrapGenerator<G>(g, this->visibleCount())));
  }
private:
  template <class G>
  class DropRInvisibleWrapGenerator {
    G      g;
    size_t offset;
  public:
    DropRInvisibleWrapGenerator(G const &g, size_t offset): g(g), offset(offset) {}

    int popMax() {
      int result = g.popMax();
      if (result >= 0)
        result += offset;
      return result;
    }
    int count() const {
      return g.count();
    }
  };
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_QUEUEWITHSTORAGE_HPP */
