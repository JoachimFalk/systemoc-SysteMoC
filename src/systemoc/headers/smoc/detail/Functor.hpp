// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_FUNCTOR_HPP
#define _INCLUDED_SMOC_DETAIL_FUNCTOR_HPP

#include <cassert>

namespace smoc { namespace Detail {

//
// ParamNode: Used to store bound parameters
//
template<class P, class PN = void>
struct ParamNode
{
  typedef P  ListHead;
  typedef PN ListTail;

  P p;
  PN pn;

  ParamNode(P _p, const PN &_pn) : p(_p), pn(_pn) {}
};

template<>
struct ParamNode<void,void>
{};

//
// MissingNode: Used to store type information for missing parameters
//
template<class M, class ML = void>
struct MissingNode
{
  typedef M  ListHead;
  typedef ML ListTail;
};

template<>
struct MissingNode<void,void>
{};

// Convert MissingList type to corresponding ParamList type, e.g.,
// MissingToParamList<MissingList>::ParamList
template<class ML, class PL = ParamNode<void> >
struct MissingToParamList;

template<class ML, class PL>
struct MissingToParamList
{
  typedef typename MissingToParamList<
    typename ML::ListTail,
    ParamNode<typename ML::ListHead, PL> >::ParamList ParamList;
};

template<class PL>
struct MissingToParamList<MissingNode<void>, PL>
{
  typedef PL                                          ParamList;
};

//
// ParamAccumulator: binds parameter via operator()
// 
template<
  template <class,class> class A,
  class F,
  bool  X  = false,
  class ML = typename F::MissingList,
  class PL = ParamNode<void>
>
struct ParamAccumulator
{
  typedef ParamAccumulator<A, F, X, ML, PL>         accumulated_type;
  typedef ParamAccumulator<A, F, X,
    typename ML::ListTail,
    ParamNode<typename ML::ListHead, PL> >  accumulated_next_type;
  
  F  f;
  PL pl;
  
  ParamAccumulator(const F& _f, const PL &_pl = PL()) : f(_f), pl(_pl) {}
  
  static
  accumulated_type build(const F &_f, const PL &_pl = PL())
    { return accumulated_type(_f, _pl); }

  typename accumulated_next_type::accumulated_type operator()(typename ML::ListHead p)
    { return accumulated_next_type::build(f, ParamNode<typename ML::ListHead, PL>(p, pl)); }
};

template<template <class,class> class A, class F, class PL>
struct ParamAccumulator<A, F, false, MissingNode<void>, PL>
{
  typedef typename A<F,PL>::type                    accumulated_type;

  static
  accumulated_type build(const F &_f, const PL &_pl = PL())
    { return accumulated_type(_f, _pl); }
};

template<template <class,class> class A, class F, class PL>
struct ParamAccumulator<A, F, true, MissingNode<void>, PL>
{
  typedef typename A<F,PL>::result_type             accumulated_type;

  static
  accumulated_type build(const F &_f, const PL &_pl = PL())
    { return A<F,PL>::build(_f, _pl); }
};

//
// FUNCTOR-macro: Creates Function-Objects
// 
#define CONSTRUCT(NAME, TLIST, PLIST, CONST, PCALL, PDUMP,MISSING)              \
template<class R, class T TLIST>                                                \
struct NAME<R, R (T::*)(PLIST) CONST> {                                         \
        typedef NAME<R, R (T::*)(PLIST) CONST>       this_type;                 \
        typedef MISSING                              MissingList;               \
        typedef R                                    return_type;               \
        typedef typename MissingToParamList                                     \
          <MissingList>::ParamList                   ParamList;                 \
                                                                                \
        CONST T    *obj;                                                        \
        R      (T::*func)(PLIST) CONST;                                         \
        const char *name;                                                       \
        bool        canRunInParallel;                                           \
                                                                                \
        template<class X>                                                       \
        NAME(CONST X *_obj, R (T::*_func)(PLIST) CONST,                         \
             const char *_name, bool _canRunInParallel=false)                   \
          : obj(dynamic_cast<CONST T *>(_obj)), func(_func),                    \
            name(_name), canRunInParallel(_canRunInParallel)                    \
          { assert(obj != 0 && func != 0); }                                    \
                                                                                \
        R call(const ParamList &pl) const {                                     \
                return (obj->*func)(PCALL);                                     \
        }                                                                       \
        template<class V>                                                       \
        static void paramListVisit(const ParamList &pl, const V& v = V())       \
          { PDUMP }                                                             \
        template<class V>                                                       \
        static void paramListVisit(const ParamList &pl, V& v)                   \
          { PDUMP }                                                             \
};

#define MISSING_0 MissingNode<void>
#define MISSING_1 MissingNode<TP1 , MISSING_0 >
#define MISSING_2 MissingNode<TP2 , MISSING_1 >
#define MISSING_3 MissingNode<TP3 , MISSING_2 >
#define MISSING_4 MissingNode<TP4 , MISSING_3 >

#define TEMPLATELIST_0 
#define TEMPLATELIST_1 , typename TP1 TEMPLATELIST_0
#define TEMPLATELIST_2 , typename TP2 TEMPLATELIST_1
#define TEMPLATELIST_3 , typename TP3 TEMPLATELIST_2
#define TEMPLATELIST_4 , typename TP4 TEMPLATELIST_3

#define PARAMLIST_0
#define PARAMLIST_1 TP1
#define PARAMLIST_2 TP2 , PARAMLIST_1
#define PARAMLIST_3 TP3 , PARAMLIST_2
#define PARAMLIST_4 TP4 , PARAMLIST_3

#define PARAMCALL_0
#define PARAMCALL_1 pl.p
#define PARAMCALL_2 pl.pn.p ,       PARAMCALL_1
#define PARAMCALL_3 pl.pn.pn.p ,    PARAMCALL_2
#define PARAMCALL_4 pl.pn.pn.pn.p , PARAMCALL_3

#define PARAMDUMP_0            
#define PARAMDUMP_1 v(pl.p);          PARAMDUMP_0
#define PARAMDUMP_2 v(pl.pn.p);       PARAMDUMP_1
#define PARAMDUMP_3 v(pl.pn.pn.p);    PARAMDUMP_2
#define PARAMDUMP_4 v(pl.pn.pn.pn.p); PARAMDUMP_3

template<class R, class F>
struct Functor;

CONSTRUCT(     Functor, TEMPLATELIST_0, PARAMLIST_0,      , PARAMCALL_0, PARAMDUMP_0, MISSING_0)
CONSTRUCT(     Functor, TEMPLATELIST_1, PARAMLIST_1,      , PARAMCALL_1, PARAMDUMP_1, MISSING_1)
CONSTRUCT(     Functor, TEMPLATELIST_2, PARAMLIST_2,      , PARAMCALL_2, PARAMDUMP_2, MISSING_2)
CONSTRUCT(     Functor, TEMPLATELIST_3, PARAMLIST_3,      , PARAMCALL_3, PARAMDUMP_3, MISSING_3)
CONSTRUCT(     Functor, TEMPLATELIST_4, PARAMLIST_4,      , PARAMCALL_4, PARAMDUMP_4, MISSING_4)
CONSTRUCT(     Functor, TEMPLATELIST_0, PARAMLIST_0, const, PARAMCALL_0, PARAMDUMP_0, MISSING_0)
CONSTRUCT(     Functor, TEMPLATELIST_1, PARAMLIST_1, const, PARAMCALL_1, PARAMDUMP_1, MISSING_1)
CONSTRUCT(     Functor, TEMPLATELIST_2, PARAMLIST_2, const, PARAMCALL_2, PARAMDUMP_2, MISSING_2)
CONSTRUCT(     Functor, TEMPLATELIST_3, PARAMLIST_3, const, PARAMCALL_3, PARAMDUMP_3, MISSING_3)
CONSTRUCT(     Functor, TEMPLATELIST_4, PARAMLIST_4, const, PARAMCALL_4, PARAMDUMP_4, MISSING_4)
  
template<class R, class F>
struct ConstFunctor;

CONSTRUCT(ConstFunctor, TEMPLATELIST_0, PARAMLIST_0, const, PARAMCALL_0, PARAMDUMP_0, MISSING_0)
CONSTRUCT(ConstFunctor, TEMPLATELIST_1, PARAMLIST_1, const, PARAMCALL_1, PARAMDUMP_1, MISSING_1)
CONSTRUCT(ConstFunctor, TEMPLATELIST_2, PARAMLIST_2, const, PARAMCALL_2, PARAMDUMP_2, MISSING_2)
CONSTRUCT(ConstFunctor, TEMPLATELIST_3, PARAMLIST_3, const, PARAMCALL_3, PARAMDUMP_3, MISSING_3)
CONSTRUCT(ConstFunctor, TEMPLATELIST_4, PARAMLIST_4, const, PARAMCALL_4, PARAMDUMP_4, MISSING_4)
  
#undef CONSTRUCT

#undef MISSING_0
#undef MISSING_1
#undef MISSING_2
#undef MISSING_3
#undef MISSING_4

#undef TEMPLATELIST_0
#undef TEMPLATELIST_1
#undef TEMPLATELIST_2
#undef TEMPLATELIST_3
#undef TEMPLATELIST_4

#undef PARAMLIST_0
#undef PARAMLIST_1
#undef PARAMLIST_2
#undef PARAMLIST_3
#undef PARAMLIST_4

#undef PARAMCALL_0
#undef PARAMCALL_1
#undef PARAMCALL_2
#undef PARAMCALL_3
#undef PARAMCALL_4
 
#undef PARAMDUMP_0
#undef PARAMDUMP_1
#undef PARAMDUMP_2
#undef PARAMDUMP_3
#undef PARAMDUMP_4

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_FUNCTOR_HPP */
