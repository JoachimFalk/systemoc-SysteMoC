
/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * ----------------------------------------------------------------------------
 * packet_header.h
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/

#ifndef PACKET_HEADER_H
#define PACKET_HEADER_H

/** \file
 *
 * \brief packet header related datatypes and structure definitions
 *  
 *  This file is used by header generation/analysis modules of the hca.
 *  It contains strcuture definitions, datatypes and codes for ib headers.
 *  
 *  Defined headers structures: 
 *  - Acknowledgement Extended Transport Header (AETH)
 *  - Local Routing Header (LRH)
 *  - Base Transport Header (BTH)
 */

#include <datatypes.h>
#include "bit_buffer.h"


/**
 * \brief Acknowledgement Extended Transport Header
 *
 *  defines the structure and field lengths of the AETH,
 *  the sysndrome type codes and NAK error codes
 */
class aeth :
  public bit_buffer
{
  public:    
   
    /// size of AETH in bytes
    static const size_t AETH_SIZE = 4;
    
    /// type codes used for syndrome type field
    enum syndrome_type_codes {
      ACK_CODE = 0,     ///< positive ACK = 000
      RNR_NAK_CODE = 1, ///< Receive Not Ready NAK = 001
      NAK_CODE = 3      ///< negative ACK = 011
    };

    /// nak codes used for syndrome info field
    enum nak_error_codes {
      PSN_SEQ_ERROR_CODE = 0,    ///< PSN sequence error = 00000
      INV_REQ_ERROR_CODE = 1,    ///< invalid request error = 00001
      REM_ACCESS_ERROR_CODE = 2, ///< remote access error = 00010
      REM_OP_ERROR_CODE = 3,     ///< remote operational error = 00011
      INV_RD_REQ_ERROR_CODE = 4  ///< invalid rd request error = 00100
    };

    /// syndrome type_code field ( 3 bits )
    bit_buffer::bit_field<unsigned int> type_code() { 
      return bit_buffer::bit_field<unsigned int>( *this, 0, 3);
    }
    
    /// syndrome info field ( 5 bits )
    bit_buffer::bit_field<unsigned int> info() {
      return bit_field<unsigned int>( *this, 3, 5);
    }
   
    /// msn field (24 bits)
    bit_buffer::bit_field<unsigned int> msn() {
      return bit_field<unsigned int>( *this, 8, 24);
    }
  
    /// build an empty AETH (4 bytes) 
    aeth() :
      bit_buffer( aeth::AETH_SIZE )
    {}

    /// build an AETH with 4 bytes init data 
    aeth(std::string data) :
      bit_buffer( data )
    {
      assert( data.size() == aeth::AETH_SIZE );
    }
};


/**
 * \brief Local Routing Header
 * 
 * defines the structure and field lengths of the local routing header
 * see IB standard for further details
 */
class lrh :
  public bit_buffer
{
  public:    
    
    /// size of LRH in bytes
    static const size_t LRH_SIZE = 8;
    
    /// LNH - indicates what header follows the lrh
    enum lnh_types {
      LNH_RAW_ETHER   = 0,  ///< raw ether type, not used
      LNH_RAW_IPV6    = 1,  ///< raw IPv6, not used
      LNH_IBA_LOCAL   = 2,  ///< iba local packet
      LNH_IBA_GLOBAL  = 3   ///< iba global packet, not used
    };
     
    /// used version of LRH
    enum lrh_versions {
      LRH_V0 = 0            ///< only 0 defined 
    };

    /// virtual lane field ( 4 bits )
    bit_buffer::bit_field<unsigned int> vl() { 
      return bit_buffer::bit_field<unsigned int>( *this, 0, 4); 
    }
    
    /// link header version field ( 4 bits )
    bit_buffer::bit_field<unsigned int> lver() { 
      return bit_buffer::bit_field<unsigned int>( *this, 4, 4); 
    }
    
    /// service level field ( 4 bits )
    bit_buffer::bit_field<unsigned int> sl() { 
      return bit_buffer::bit_field<unsigned int>( *this, 8, 4); 
    }
    
    /// reserved1 field ( 2 bits )
    bit_buffer::bit_field<unsigned int> reserved1() { 
      return bit_buffer::bit_field<unsigned int>( *this, 12, 2); 
    }
    
    /// link next header field ( 2 bits )
    bit_buffer::bit_field<unsigned int> lnh() { 
      return bit_buffer::bit_field<unsigned int>( *this, 14, 2); 
    }
    
    /// destination local id field ( 16 bits )
    bit_buffer::bit_field<unsigned int> dlid() { 
      return bit_buffer::bit_field<unsigned int>( *this, 16, 16); 
    }
    
    /// reserved field ( 5 bits )
    bit_buffer::bit_field<unsigned int> reserved2() { 
      return bit_buffer::bit_field<unsigned int>( *this, 32, 5); 
    }
    
    /// packet length field ( 11 bits )
    bit_buffer::bit_field<unsigned int> pktlen() { 
      return bit_buffer::bit_field<unsigned int>( *this, 37, 11); 
    }
    
    /// source local id field ( 16 bits )
    bit_buffer::bit_field<unsigned int> slid() { 
      return bit_buffer::bit_field<unsigned int>( *this, 48, 16); 
    }
    
    /// construct an empty LRH (8 bytes) 
    lrh() :
      bit_buffer( lrh::LRH_SIZE )
    {}

    /// construct a LRH with 8 bytes init data 
    lrh(std::string data) :
      bit_buffer( data )
    {
      assert( data.size() == lrh::LRH_SIZE );
    }
};
 

/**
 *
 * \brief Base Transport Header
 * 
 * defines the strcuture and field lengths of the base transport header
 * see IB standard for further details
 */
class bth :
  public bit_buffer
{
  public:    
    
    /// size of BTH in bytes
    static const size_t BTH_SIZE = 12;
    
    /// used BTH version
    enum bth_version {
      BTH_V0 = 0            ///< only 0 defined 
    };

    /// BTH opcode field codes
    enum bth_opcode {
      BTH_send_first  = 0,  ///< first packet of a message
      BTH_send_middle = 1,  ///< packet of message
      BTH_send_last   = 2,  ///< last packet of a message
      BTH_send_only   = 4,  ///< first and only packet of a message
      BTH_ack         = 17  ///< ack packet
    };

    ///  opcode field ( 8 bits )
    bit_buffer::bit_field<unsigned int> opcode() { 
      return bit_buffer::bit_field<unsigned int>( *this, 0, 8); 
    }
    
    ///  solicited event field ( 1 bit )
    bit_buffer::bit_field<unsigned int> se() { 
      return bit_buffer::bit_field<unsigned int>( *this, 8, 1); 
    }
    
    ///  migration request field ( 1 bit )
    bit_buffer::bit_field<unsigned int> m() { 
      return bit_buffer::bit_field<unsigned int>( *this, 9, 1); 
    }
    
    ///  pad count field ( 2 bits )
    bit_buffer::bit_field<unsigned int> padcnt() { 
      return bit_buffer::bit_field<unsigned int>( *this, 10, 2); 
    }
    
    ///  transport header version field ( 4 bits )
    bit_buffer::bit_field<unsigned int> tver() { 
      return bit_buffer::bit_field<unsigned int>( *this, 12, 4); 
    }
    
    ///  partition key field ( 16 bits )
    bit_buffer::bit_field<unsigned int> p_key() { 
      return bit_buffer::bit_field<unsigned int>( *this, 16, 16); 
    }
    
    /// reserved field ( 8 bits )
    bit_buffer::bit_field<unsigned int> reserved1() { 
      return bit_buffer::bit_field<unsigned int>( *this, 32, 8); 
    }
    
    ///  destination QP number field ( 24 bits )
    bit_buffer::bit_field<unsigned int> dst_qpn() { 
      return bit_buffer::bit_field<unsigned int>( *this, 40, 24); 
    }
    
    /// ack request flag field ( 1 bit )
    bit_buffer::bit_field<unsigned int> a() { 
      return bit_buffer::bit_field<unsigned int>( *this, 64, 1); 
    }
    
    ///  reserved field ( 7 bits )
    bit_buffer::bit_field<unsigned int> reserved2() { 
      return bit_buffer::bit_field<unsigned int>( *this, 65, 7); 
    }
    
    /// packet sequence number field ( 24 bits )
    bit_buffer::bit_field<unsigned int> psn() { 
      return bit_buffer::bit_field<unsigned int>( *this, 72, 24); 
    }
    
    /// construct an empty BTH (12 bytes) 
    bth() :
      bit_buffer( bth::BTH_SIZE )
    {}

    /// construct a BTH with 12 bytes init data 
    bth(std::string data) :
      bit_buffer( data )
    {
      assert( data.size() == bth::BTH_SIZE );
    }
};
 

/**
 *  \brief Variant Redundancy Check (VCRC) checksum for InifiniBand packets
 *
 *  16 Bit CRC based upon HIPPI 6400 CRC with generator polynome 0x100B
 */
class vcrc :
  public bit_buffer 
{
  public:
    
    /// size of ICRC in bytes
    static const size_t VCRC_SIZE = 2;
    
    ///  invariant CRC field ( 32 bits )
    bit_buffer::bit_field<unsigned int> checksum() { 
      return bit_buffer::bit_field<unsigned int>( *this, 0, 16); 
    }
    
    /// construct an empty CRC 
    vcrc() :
      bit_buffer( vcrc::VCRC_SIZE )
    {}

    /// construct a CRC initialized with a VCRC checksum
    vcrc( std::string checksum ) :
      bit_buffer( checksum )
    {
      assert( checksum.size() == vcrc::VCRC_SIZE );
    }
};


/**
 *  \brief Cyclic Redundancy Check (ICRC) checksum for InifiniBand packets
 *
 *  32 Bit CRC based upon Ethernet's CRC with generator polynome 0x04C11DB7
 */
class icrc :
  public bit_buffer 
{
  public:
    
    /// size of ICRC in bytes
    static const size_t ICRC_SIZE = 4;
    
    ///  invariant CRC field ( 32 bits )
    bit_buffer::bit_field<unsigned int> checksum() { 
      return bit_buffer::bit_field<unsigned int>( *this, 0, 32); 
    }
    
    /// construct an empty CRC 
    icrc() :
      bit_buffer( icrc::ICRC_SIZE )
    {}

    /// construct a CRC initialized with a IB checksum
    icrc( std::string checksum ) :
      bit_buffer( checksum )
    {
      assert( checksum.size() == icrc::ICRC_SIZE );
    }
};


/**
 *  \brief Flow Control Packet
 *
 *  This defines the structure of an IB flow control packet.
 */
class fcp :
  public bit_buffer 
{
  public:
    
    /// size of FCP in bytes
    static const size_t FCP_SIZE = 8;
    
    /// "not applicable" value for FCCL/FCTBS field
    static const t_uint N_A = 0;

    /// flow control operand field values
    enum fcp_operand {
      NORMAL = 0,      ///< normal FlowControl packet
      INIT   = 1       ///< initial FlowControl packet
    };

    ///  operand field ( 4 bits )
    bit_buffer::bit_field<unsigned int> op() { 
      return bit_buffer::bit_field<unsigned int>( *this, 0, 4); 
    }
    
    ///  FlowControl Total Bytes Sent field ( 12 bits )
    bit_buffer::bit_field<unsigned int> fctbs() { 
      return bit_buffer::bit_field<unsigned int>( *this, 4, 12); 
    }
    
    ///  virtual lane field ( 4 bits )
    bit_buffer::bit_field<unsigned int> vl() { 
      return bit_buffer::bit_field<unsigned int>( *this, 16, 4); 
    }
    
    ///  FlowControl Credit Limit field ( 12 bits )
    bit_buffer::bit_field<unsigned int> fccl() { 
      return bit_buffer::bit_field<unsigned int>( *this, 20, 12); 
    }
    
    ///  checksum field ( 16 bits )
    bit_buffer::bit_field<unsigned int> lpcrc() { 
      return bit_buffer::bit_field<unsigned int>( *this, 32, 16); 
    }
    
    ///  reserved field ( 16 bits )
    bit_buffer::bit_field<unsigned int> reserved() { 
      return bit_buffer::bit_field<unsigned int>( *this, 48, 16); 
    }
    
    /// construct an empty CRC 
    fcp() :
      bit_buffer( fcp::FCP_SIZE )
    {}

    /// construct a CRC initialized with a IB checksum
    fcp( std::string fc_packet ) :
      bit_buffer( fc_packet )
    {
      assert( fc_packet.size() == fcp::FCP_SIZE );
    }
};


/**
 *  Maximal size of header data stored in MSTORE.
 *  LRH header is removed by RECV_PORT LRH analysis
 *
 *  this value may change in the future when GRH or 
 *  RETHs are used
 */
#define MAX_STORED_HEADER_SIZE (bth::BTH_SIZE + aeth::AETH_SIZE)

/**
 *  Max size of an IB packet header used in this simulation.
 *  May change in the future when RETHs or GRH are added.
 */
#define MAX_HEADER_SIZE (lrh::LRH_SIZE + bth::BTH_SIZE + aeth::AETH_SIZE)

template <int chunksize>
class data_chunk{
 private:
  char chunk[chunksize];
 public:
  int sourceId;
  t_uint32 length;
  t_uint64 address;

  int getChunkSize(){return chunksize;}

  t_uint32 getLength(){return length;}

  data_chunk() : length(0){}

  data_chunk( const char *buf, int sourceId, t_uint64 start, t_uint32 _length ):address(start), sourceId( sourceId ){
    for(length=0; length<chunksize && length <_length; length++){
      chunk[length]=buf[start+length];
    }
  }

  const char *getChunk(){return chunk;}
};
struct memory_request{
  static int _id_count;
  int id;
  int sourceId;
  t_uint64 address;
  t_uint32 length;

  memory_request( int sourceId, t_uint64 address, t_uint32 length ) :
    address( address ), length( length ), sourceId( sourceId ){
    
    id=_id_count++;
  }

  memory_request() : address( 0 ), length( 0 ){
    id=_id_count++;
  }
};
int memory_request::_id_count=0;


#endif // PACKER_HEADER_H

