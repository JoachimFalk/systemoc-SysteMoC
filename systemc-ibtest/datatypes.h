
/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * ----------------------------------------------------------------------------
 * datatypes.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/


#ifndef WITHOUT_SYSTEMC
# include <stdint.h>
# include <systemc.h>
#endif

#include <list>

#ifndef DATATYPES_H
#define DATATYPES_H

/** \file
 *
 * \brief user defined types, macros, enums used in several places.
 *
 * this file is included by almost all other source files.
 *
 */

//#include "ib_verbs.h"

/*******************************************************************************
 *
 * macros
 *
 */


/**
 * \brief minimum.
 */
#define MIN(A, B) ((A) < (B) ? (A) : (B))



/**
 * \brief delete and NULL object.
 */

//FIXME: QUICK HACK !!! Bug in InfiniBand
//       with preliminary freeing of messages
//       solved by disabling DELETE operations !!!

/*
#define DELETE(object)    \
do {                      \
  delete object;          \
  object = 0;             \
} while ( 0 )
*/

#define DELETE(object)    \
do {                      \
} while ( 0 )



/**
 * \brief macro to disable code when not debugging.
 *
 * everything enclosed by DBG() will disapear in non debugging builds.
 */
#ifdef DEBUG
  #define DBG(d) d
#else
  #define DBG(d) 
#endif



/**
 * \brief macro to disable output code.
 */
#ifdef OUTPUT
  #define OUT(o) o
#else
  #define OUT(o)
#endif



/// @name Color defines
//@{
/**
 * \brief print str in color (using escape sequence).
 */
#define RED(str) "\e[31;1m" #str "\e[0m"
#define GREEN(str) "\e[32;1m" #str "\e[0m"
#define BLUE(str) "\e[34;1m" #str "\e[0m"
//@}


/**
 * \brief string with colored yes/no if a == b or not
 */
#define EQUALS(a, b) "Object " #a " == " #b "? " << (a == b ? GREEN(yes) : RED(no)) \
  << std::endl

/**
 * \brief dump object to stream.
 */
#define DO(o) "Object " #o " = " << o


/**
 * \brief maximum packet payload size (bytes)
 */
#define MTU_SIZE 512

/**
 *  \brief smallest possible data packet size (bytes)
 *
 *  an IBA LOCAL ACK packet is the smallest possible "data" packet
 *  with a overall length of LRH,BTH,AETH (24 bytes) + ICRC (4 byte)
 *  = 28 bytes
 */
#define MIN_DATA_PACKET_SIZE \
        (lrh::LRH_SIZE + bth::BTH_SIZE + aeth::AETH_SIZE + icrc::ICRC_SIZE)


/*******************************************************************************
 *
 * enums
 *
 */

/**
 * \brief possible commands for scheduler.
 */
enum e_sched_action {
  SCHED_ADD,
  SCHED_ADD_READY,
  SCHED_ADD_ONESHOT,
  SCHED_REMOVE
};


/**
 *  \brief  possible tt source queues
 */
enum e_source_queue {
  NONE, ///< none specified, neutral init val
  TQ,   ///<  transmit queue
  RQ,   ///<  receive queue
  AQ    ///<  ack queue
};

/*******************************************************************************
 *
 * typedefs
 *
 */

/// @name signed and unsigned integers
//@{
typedef unsigned int  t_uint;

#ifndef WITHOUT_SYSTEMC
/// unsigned integers
typedef sc_uint<1>    t_uint1;
typedef sc_uint<2>    t_uint2;
typedef sc_uint<3>    t_uint3;
typedef sc_uint<4>    t_uint4;
typedef sc_uint<5>    t_uint5;
typedef sc_uint<7>    t_uint7;
typedef sc_uint<8>    t_uint8;
typedef sc_uint<11>   t_uint11;
typedef sc_uint<12>   t_uint12;
typedef sc_uint<16>   t_uint16;
typedef sc_uint<24>   t_uint24;
typedef t_uint        t_uint32;
//typedef sc_uint<48> t_uint48;
//typedef sc_uint<64> t_uint64;
typedef uint64_t      t_uint64;

/// signed integers
typedef sc_int<24>    t_int24;
typedef int	      t_int32;
#else
/// unsigned integers
typedef unsigned char	    t_uint1;
typedef unsigned char	    t_uint2;
typedef unsigned char	    t_uint3;
typedef unsigned char	    t_uint4;
typedef unsigned char	    t_uint5;
typedef unsigned char	    t_uint6;
typedef unsigned char	    t_uint7;
typedef unsigned char	    t_uint8;
typedef unsigned short	    t_uint9;
typedef unsigned short	    t_uint10;
typedef unsigned short	    t_uint11;
typedef unsigned short	    t_uint12;
typedef unsigned short	    t_uint13;
typedef unsigned short	    t_uint14;
typedef unsigned short	    t_uint15;
typedef unsigned short	    t_uint16;
typedef unsigned int	    t_uint24;
typedef unsigned int	    t_uint32;
typedef unsigned long long  t_uint64;

/// signed integers
typedef int		    t_int24;
typedef int		    t_int32;
#endif
//@}

/// sequence position
typedef t_uint      t_seq_pos;

/// @name sequence position consts
//@{
const t_seq_pos SEQ_POS_SOLE = 3;
const t_seq_pos SEQ_POS_FIRST = 2;
const t_seq_pos SEQ_POS_LAST = 1;
const t_seq_pos SEQ_POS_MIDDLE = 0;
//@}

/// queue pair number
// typedef t_uint24 t_qpn;
typedef t_uint24 t_qpn; 

/// packet sequence number
typedef t_int24 t_psn;

/// message sequence number
typedef t_uint24 t_msn;

/// subnet unique port identifiers (SLID, DLID)
typedef t_uint16 t_lid;

/// boolean
typedef bool t_bool;

/// credits
typedef t_uint5 t_credits;

/// mfetch/mstore send/receive buffer id
typedef t_int32 t_buf_id;
/// list of buffer ids
typedef std::list<t_buf_id> t_buf_list;

/// wqe operation type
//typedef int t_op_type;

/*******************************************************************************
 *
 * structs/simple classes (collectors)
 *
 */

/// scatter/gather buffer list entry
typedef struct {
  t_uint64  v_start_addr; ///< virtual start address
  t_uint32  length;       ///< length of this buffer
  t_uint32  l_key;        ///< L-Key associated with this buffer
} ts_scaga_buf;

/*
/// wqe part
struct ts_wqepart {
  //t_op_type     op_type;  ///< operation type
  enum ib_wr_optype  op_type;  ///< operation type
  t_msn         msn;      ///< message sequence number
  int           index;    ///< wqepart index (NEW)
  ts_scaga_buf  buffer;   ///< scatter/gather list
  t_seq_pos     seq_pos;  ///< sequence postition (first, last)
  int           msg_size_remaining;
};
*/

/// infos needed to resend packet
// TODO: don't put msn into psi! there could be tons of packets all carrying
//       the same msn and this would be waste of space. use some other scheme
//       of lookup!

/*
class packet_start_info {
public:
  t_psn      psn;            ///< infomation needed to resend *this* psn
  int        wqepart_index;  ///< index of wqepart
  t_uint32   offset;         ///< offset in indexed wqepart
  t_msn      msn;            ///< msn of wqe for this packet
  t_seq_pos  pos;            ///< position of this packet in message stream

  packet_start_info(t_psn psn, ts_wqepart *wqepart, t_uint32 offset) :
    psn(psn),
    wqepart(wqepart),
    offset(offset)
  {}

  packet_start_info()
  {}    
};
*/


/** 
 * \brief Port Information Context
 *
 * holds information about a port:
 * - its subnet unique identifier, called SLID
 * - its maximum number of data VLs it can support
 * - its number of actual used data VLs
 */

class port_info {
  
  public:

    // TODO:
    // add SLID to PortInfo

    /// maximum number of VLs
    const t_uint vl_cap;

    /// number of operational VLs
    t_uint  vl_op;

    /// link layer version used, only 0 defined since now
    t_uint4 lver;
      
    port_info(t_uint vl_cap, t_uint vl_op, t_uint4 lver = 0) :
      vl_cap(vl_cap),
      vl_op(vl_op),
      lver(lver)
    {}  
};


#ifdef OBSOLETE_CODE
/// info about a wqe (in wqm)
class rq_wqe_info {
public:
  t_msn msn;
  int   msg_size;

  rq_wqe_info(t_msn msn, int size) :
    msn(msn),
    msg_size(size)
  {}
  
  rq_wqe_info(void) :
    msn(0),
    msg_size(0)
  {}
};
#endif



#endif // DATATYPES_H

