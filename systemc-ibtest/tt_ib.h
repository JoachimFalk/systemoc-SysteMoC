
/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * -----------------------------------------------------------------------------
 * tt_ib.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/

#include <string>

#ifndef TT_IB_H
#define TT_IB_H

/** \file
 *
 * \brief transmit tokens.
 *
 * contains abstract base class ib_tt and all derived classes
 *
 */

#include "datatypes.h"

/**
 * \brief abstract base class of all transmit tokens.
 *
 */
// pure virtual (abstract class)
class tt_ib
{
  public:
    
    /// transmit token type
    enum e_tt_type {
      TT_ACK_INFO,    ///<  tt containing ack/nak information
      TT_PACKET_INFO, ///<  tt containing packet information
      TT_RAW_HEADER,  ///<  tt containing header (lrh, bth or aeth)
      TT_DATA,        ///<  tt containing payload storage information
      TT_RAW_DWORD,   ///<  tt containing 4 byte of raw data
      TT_MESSAGE,     ///<  tt containing intra-hca error/notification message
      TT_LLC_INFO     ///<  tt containing LRH generation parameters
    };

    /// indicate type of this tt
    e_tt_type type;
 
    /// buffer id of sendbuffer in mfetch or receivebuffer in mstore
    t_buf_id     buffer_id;
  
    template <class T> T *castme(void)
    {
      T *retval = dynamic_cast<T*>(this);
      assert(retval != NULL);
      return retval;
    }

    virtual ~tt_ib();

    tt_ib(e_tt_type type, t_buf_id buffer_id) :
      type(type),
      buffer_id(buffer_id) 
    {}

    virtual void dump(ostream&) const;
};



/**
 * \brief abstract base class for packet information transmit tokens
 *
 * contains basic information about data and ack packets,
 * e.g corresponding qpn
 *
 */
// pure virtual (abstract class)
class tt_base_info :
  public tt_ib
{
  public:
    
    /// queue pair number
    t_qpn   qpn;

    virtual ~tt_base_info();

    tt_base_info(e_tt_type type, t_qpn qpn, t_buf_id buffer_id ) :
      tt_ib(type, buffer_id),
      qpn(qpn)
    {}

    virtual void dump(ostream&) const;
};

/**
 * \brief packet information transmit token
 *
 * contains more detailed information about a (data) packet it describes,
 * e.g. the psn and the size.
 *
 * for acknowledgement packets it is used in combination with the
 * ack information transmit token (tt_ack_info), see below.
 *
 */

class tt_packet_info :
  public tt_base_info
{
  public:

    /// packet sequence number
    t_psn     psn;

    /// size of the packet
    t_uint    packet_size;

    /// seq pos
    t_seq_pos seq_pos;

    tt_packet_info(t_qpn qpn, t_psn psn, t_uint packet_size, t_buf_id buffer_id,
        t_seq_pos seq_pos = SEQ_POS_FIRST) :
      tt_base_info(tt_ib::TT_PACKET_INFO, qpn, buffer_id),
      psn(psn),
      packet_size(packet_size),
      seq_pos(seq_pos)
    {}

    virtual void dump(ostream&) const;
};

/**
 * \brief acknowledgement information transmit token
 *
 * contains information about acknowledgement packets, e.g
 * the msn and the acknowledgment type
 * 
 */

class tt_ack_info :
  public tt_base_info
{
  public:

    /// different acknowledgement types
    enum e_ack_type {
      ACK,      ///< positive acknowledgement
      NAK       ///< negative acknowledgement
    };
  
    /// acknowledgement type information
    e_ack_type  ack_type;
    
    /// message sequence number
    t_msn       msn;

    tt_ack_info(e_ack_type ack_type, t_qpn qpn, t_msn msn, t_buf_id buffer_id) :
      tt_base_info(tt_ib::TT_ACK_INFO, qpn, buffer_id),
      ack_type(ack_type),
      msn(msn)
    {}  

    virtual void dump(ostream&) const;
};

/**
 *  \brief positive ack transmit token
 *
 *  contains additional acknowledgement information for postive acknowledgements,
 *  e.g. the credit count.
 *
 */

class tt_ack :
  public tt_ack_info
{
  public:

    /// credits
    t_credits   credits;

    /*
    tt_ack(t_qpn qpn, t_msn msn, t_credits credits) :
      tt_ack_info(tt_ack_info::ACK, qpn, msn),
      credits(credits)
    {}
    */

    tt_ack(t_qpn qpn, t_msn msn, t_uint credits_, t_buf_id buffer_id = -1) :
      tt_ack_info(tt_ack_info::ACK, qpn, msn, buffer_id)
    {
      if ( credits_ >= (1U << credits.length()) )
        credits = (1U << credits.length()) - 1;
      else
        credits = credits_;
    }

    virtual void dump(ostream&) const;
};

/**
 *  \brief negative ack transmit token
 *
 *  contains additional acknowledgement information for negative acknowledgements
 *
 */

class tt_nak :
  public tt_ack_info 
{
  public:

    /// different kind of NAKs
    enum e_nak_type {
      RNR,                  ///<  Receiver Not Ready NAK
      PSN_SEQ_ERROR,        ///<  PSN Sequence Error NAK
      INVALID_REQUEST,      ///<  Invalid Request NAK
      REMOTE_ACCESS_ERROR,  ///<  Remote Access Error NAK
      REMOTE_OP_ERROR,      ///<  Remote Operational Error NAK
      INVALID_RD_REQUEST    ///<  Invalid RD Request NAK
    };
    /// nak type
    e_nak_type    nak_type;
    
    tt_nak(t_qpn qpn, t_msn msn, e_nak_type nak_type, t_buf_id buffer_id = -1) :
      tt_ack_info(tt_ack_info::NAK, qpn, msn, buffer_id),
      nak_type(nak_type)
    {}

    virtual void dump(ostream&) const;
};


/**
 *  \brief raw data transmit token
 *
 *  contains four bytes of raw data and a flag that indicates the first
 *  transmit token of a raw data transmit token stream
 *
 */

class tt_raw_dword :
  public tt_ib
{
  public:

    static const size_t DWORD_SIZE = 4;
  
    /// raw data
    std::string data;

    /// postion last flag
    t_bool  last;

    tt_raw_dword(std::string data, t_bool last, t_buf_id buffer_id) :
      tt_ib(TT_RAW_DWORD, buffer_id),
      data(data),
      last(last)
    {
      assert( data.size() == tt_raw_dword::DWORD_SIZE );
    }

    virtual void dump(ostream&) const;
};

/**
 *  \brief raw header transmit token
 *
 *  contains a buffer that carries packet header as raw data
 *  and its size in bytes.
 *
 */

class tt_raw_header :
  public tt_ib
{
  public:

    /// buffer string
    std::string  buffer;
  
    /// last flag
    t_bool  last;
 
    tt_raw_header(t_bool last, t_buf_id buffer_id) :
      tt_ib(tt_ib::TT_RAW_HEADER, buffer_id),
      buffer(""),
      last(last)
    {}

    tt_raw_header(std::string data, t_bool last, t_buf_id buffer_id) :
      tt_ib(tt_ib::TT_RAW_HEADER, buffer_id),
      buffer(data),
      last(last)
    {}

    virtual void dump(ostream&) const;
};


/**
 *  \brief data transmit token
 *
 *  contains a scatter/gather list entry that carries the virtual start
 *  address and size of the packet's payload part.
 *  it also contains a flag that indicates the last data transmit token
 *  within a data transmit token stream
 *
 */

class tt_data :
  public tt_ib
{
  public:

    /// scatter/gather buffer
    ts_scaga_buf    buffer;

    /// position last flag
    t_bool          last;

    tt_data(t_buf_id buffer_id, t_bool last = false) :
      tt_ib(tt_ib::TT_DATA, buffer_id),
      last(last)
    {}
      

    virtual void dump(ostream&) const;
};

/**
 *  \brief abstract base class message transmit token
 *  
 *  ERRORs are used to report failures to depending modules.
 *  NOTIFICATIONs are used to report the successful execution
 *  of a requested service.
 *  
 */

class tt_message :
  public tt_ib
{
  public:

    /// possible message types
    enum e_msg_type {
      NOTIFICATION,     ///< Notification message
      ERROR             ///< Error message
    };

    /// message type
    e_msg_type    msg_type;

    tt_message(e_msg_type msg_type, t_buf_id buffer_id) :
      tt_ib(tt_ib::TT_MESSAGE, buffer_id),
      msg_type(msg_type)
    {}

    virtual void dump(ostream&) const;
};

/**
 *  \brief  error transmit token
 *
 *  contains more detailed information if an error has occured
 *
 */

class tt_error :
  public tt_message
{
  public:

    /// different kinds of errors that might occur (to be extended ...)
    enum e_error_type {
      DESTQPN_ERROR,        ///< destination QP does not exist
      DLID_ERROR,           ///< packet's SLID does not match QP's DLID
      RETHCHK_ERROR,        ///< an error occured in the RETHCHK module
      AETHCHK_ERROR,        ///< an error occured in the AETHCHK module
      MSTORE_OP_FAILURE     ///< MSTORE write operation failed
    };

    /// error type
    e_error_type    error_type;

    tt_error(e_error_type error_type, t_buf_id buffer_id) :
      tt_message(tt_message::ERROR, buffer_id),
      error_type(error_type)
    {}

    virtual void dump(ostream&) const;
};

/**
 *  \brief  notification transmit token
 *
 *  transports control information between modules
 */

class tt_notification :
  public tt_message
{
  public:

    /// different kinds of notifications
    enum e_note_type {
      MSTORE_OP_SUCCESS,  ///< all data was stored successfully
      CLEAR_BUFFER,       ///< notification that buffer can be flushed
      BUFFER_OFFSET       ///< buffer offset notification, see below
    };

    /// notification type
    e_note_type   note_type;
    
    tt_notification(e_note_type note_type, t_buf_id buffer_id) :
      tt_message(tt_message::NOTIFICATION, buffer_id),
      note_type(note_type)
    {}

    virtual void dump(ostream&) const;
};


/**
 *  \brief  buffer offset notification transmit token
 *
 *  is generated by header analysis module to inform
 *  MSTORE module about overall size of packet headers
 *  in a specific receive buffer.
 *  this value has to be used as offset when data is
 *  written from receive buffer to memory
 */

class tt_bo_notification :
  public tt_notification
{
  public:

    t_uint  offset; 
  
    tt_bo_notification(t_uint offset, t_buf_id buffer_id) :
      tt_notification(tt_notification::BUFFER_OFFSET, buffer_id),
      offset(offset)
    {}

    virtual void dump(ostream&) const;
};


/**
 *  \brief LLC information transmit token
 *  
 *  contains parameters needed for LRH generation in
 *  SEND_PORT.
 */

class tt_llc_info :
  public tt_ib
{
  public:
    
    /// requested ServiceLevel of the packet described
    t_uint4   sl;

    /// identifies following header
    t_uint2   lnh;

    /// destination address
    t_lid     dlid;

    /// source address
    t_lid     slid;

    /// packet length in dwords (LRH.pktlen value)
    t_uint11  pktlen;

    // constructor
    tt_llc_info(  t_buf_id buffer_id,
                  t_uint4 sl,
                  t_uint2 lnh,
                  t_lid dlid,
                  t_lid slid,
                  t_uint11 pktlen) :
      tt_ib(tt_ib::TT_LLC_INFO, buffer_id),
      sl(sl),
      lnh(lnh),
      dlid(dlid),
      slid(slid),
      pktlen(pktlen)
    {}
      

    virtual void dump(ostream&) const;
};

static inline
ostream &operator<<(ostream &output, const tt_ib &tt)
  { tt.dump(output); return output; }
static inline
ostream &operator<<(ostream &output, const tt_ib *tt)
  { tt->dump(output); return output; }

#endif // TT_IB_H

