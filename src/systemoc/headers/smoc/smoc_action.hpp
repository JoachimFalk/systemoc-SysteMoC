// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_SMOC_ACTION_HPP
#define _INCLUDED_SMOC_SMOC_ACTION_HPP

#include "detail/MemFuncCallIf.hpp"

#include <boost/intrusive_ptr.hpp>

#include <list>

namespace smoc {

class smoc_action : public std::list<
  boost::intrusive_ptr<
    Detail::MemFuncCallIf<void> > >
{
  typedef smoc_action this_type;
public:
  smoc_action()
    {}

  template<class F, class PL>
  explicit
  smoc_action(F const &f, PL const &pl)
    { push_back(new MemFuncCallImpl<F, PL>(f,pl)); }

  void operator()() const;
private:
  template<class F, class PL>
  class MemFuncCallImpl
  : public Detail::MemFuncCallIf<void> {
  private:
    F  f;
    PL pl;
  public:
    MemFuncCallImpl(const F &_f, const PL &_pl = PL())
      : f(_f), pl(_pl)
    {}

    void call() const
      { f.call(pl); }
    const char *getFuncName() const
      { return f.name; }
    const char *getCxxType() const
      { return typeid(f.func).name(); }

    Detail::ParamInfoList getParams() const {
      Detail::ParamInfoVisitor piv;
      f.paramListVisit(pl, piv);
      return piv.pil;
    }
  };

};

} // namespace smoc

namespace smoc { namespace Detail {

  smoc_action merge(smoc_action const &a, smoc_action const &b);

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_SMOC_ACTION_HPP */
