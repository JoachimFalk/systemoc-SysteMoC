#include "tt_ib.h"

//
// TT_IB
//

tt_ib::~tt_ib()
{
  DBG(cout << "~tt_ib" << endl);
}

void tt_ib::dump(ostream &output) const
{
  output << " transmit token:" << endl;
  output << "   type: ";
  
  switch (type) {
   
    case tt_ib::TT_ACK_INFO :
      output << "TT_ACK_INFO";
      break;
    
    case tt_ib::TT_PACKET_INFO :
      output << "TT_PACKET_INFO";
      break;
    
    case tt_ib::TT_RAW_HEADER :
      output << "TT_RAW_HEADER";
      break;
    
    case tt_ib::TT_DATA :
      output << "TT_DATA";
      break;
    
    case tt_ib::TT_RAW_DWORD :
      output << "TT_RAW_DWORD";
      break;
    
    case tt_ib::TT_MESSAGE :
      output << "TT_MESSAGE";
      break;
    
    case tt_ib::TT_LLC_INFO :
      output << "TT_LLC_INFO";
      break;
    
    default:
      output << "unknown";
  }

  output << "   buffer id: ";
  
  if ( buffer_id == -1 )  
    output << "not set";
  else 
    output << buffer_id;
  
  output << endl;
}

//
// TT_BASE_INFO
//

tt_base_info::~tt_base_info()
{
  DBG(cout << "~tt_base_info()" << endl);
}

void tt_base_info::dump(ostream &output) const
{
  this->tt_ib::dump(output);
  output << "   qpn: " << qpn << endl;
}

//
// TT_PACKET_INFO
//

void tt_packet_info::dump(ostream &output) const
{
  this->tt_base_info::dump(output);
  output << "   psn: " << psn << endl;
  output << "   packet size: " << packet_size << endl;
  output << "   seq position: ";
  switch ( seq_pos ) {
  
    case SEQ_POS_FIRST :
      output << " first" << endl;
      break;
      
    case SEQ_POS_MIDDLE :
      output << "middle" << endl;
      break;
      
    case SEQ_POS_LAST :
      output << "last" << endl;
      break;
      
    case SEQ_POS_SOLE :
      output << "sole" << endl;
      break;
      
  }
}

//
// TT_ACK_INFO
//

void tt_ack_info::dump(ostream &output) const
{
  this->tt_base_info::dump(output);
  output << "   msn: " << msn << endl;
  output << "   ack type: ";
  
  switch(ack_type) {
    case tt_ack_info::ACK :
      output << "ACK" << endl;
      break;

    case tt_ack_info::NAK :
      output << "NAK" << endl;
      break;

    default:
      output << "unknown" << endl;
  }
}

//
// TT_ACK
//

void tt_ack::dump(ostream &output) const
{
  this->tt_ack_info::dump(output);
  output << "   credits: " << credits << endl;
}

//
// TT_NAK
//
void tt_nak::dump(ostream &output) const
{
  this->tt_ack_info::dump(output);
  output << "nak type: ";

  switch(nak_type) {
  
    case tt_nak::RNR :
      output << "Receiver Not Ready";
      break;
      
    case tt_nak::PSN_SEQ_ERROR :
      output << "PSN Sequence Error";
      break;
      
    case tt_nak::INVALID_REQUEST :
      output << "Invalid Request";
      break;
      
    case tt_nak::REMOTE_ACCESS_ERROR :
      output << "Remote Access Error";
      break;
      
    case tt_nak::REMOTE_OP_ERROR :
      output << "Remote Operational Error";
      break;
      
    case tt_nak::INVALID_RD_REQUEST :
      output << "Invalid RD Request";
      break;
 
    default:
      output << "unknown";
  }
  
  output << endl;
}
  
//
// TT_RAW_DWORD
//

void tt_raw_dword::dump(ostream &output) const
{
  this->tt_ib::dump(output);
  output  << "   data:";
  
  // print buffer
  sc_uint<8> out;
  for ( t_uint i = 0 ; i < data.size() ; i++ ) {
    out = static_cast<unsigned char>(data[i]);
    output << out.to_string(SC_BIN_US, false) << " ";
  }
  output << endl;
}

//
// TT_RAW_HEADER
//
void tt_raw_header::dump(ostream &output) const
{
  this->tt_ib::dump(output);
  
  if(last) {
    output << "   LAST" << endl;
  }
  output << "   DATA:";
 
  // print buffer
  t_uint  i;
  sc_uint<8> out;
  const char* data = buffer.data();
    
  output << "    ";
  for ( i = 1 ; i < buffer.size() + 1 ; i++ ) 
  {
    out = static_cast<unsigned char>(data[i-1]);
    output << out.to_string(SC_BIN_US, false) << " ";
    if ( i % 4 == 0)
      output << std::endl << "    ";
  }
  output << endl;
}

//
// TT_DATA
//

void tt_data::dump(ostream &output) const
{
  this->tt_ib::dump(output);
  output << "   scatter/gather: address: " << buffer.v_start_addr << endl;
  output << "                   length: " << buffer.length << endl;
  
  if(last) {
    output << "   LAST" << endl;
  }
  
}

//
// TT_MESSAGE
//
void tt_message::dump(ostream &output) const
{
  this->tt_ib::dump(output);
  output << "   message type: ";
  switch(msg_type) {
    
    case tt_message::ERROR :
      output << "ERROR";
      break;

    case tt_message::NOTIFICATION :
      output << "NOTIFICATION";
      break;

    default:
      output << "unknown";
  }
  output << endl;
}

//
// TT_ERROR
//

void tt_error::dump(ostream &output) const
{
  this->tt_message::dump(output);
  output << "   error: ";
  
  switch(error_type) {
  
    case tt_error::DESTQPN_ERROR :
      output << "DestQPN Error";
      break;
    
    case tt_error::DLID_ERROR :
      output << "DLID Error";
      break;

    case tt_error::MSTORE_OP_FAILURE :
      output << "MSTORE operation failed";
      break;
    
    case tt_error::RETHCHK_ERROR :
      output << "Error in RETHCHK module";
      break;

    case tt_error::AETHCHK_ERROR :
      output << "Error in AETHCHK module";
      break;

    default:
      output << "undefined";
  }
  output << endl;
}

//
// TT_NOTIFICATION
//

void tt_notification::dump(ostream &output) const
{
  this->tt_message::dump(output);
  output << "   notification: ";
  
  switch(note_type) {
  
    case tt_notification::MSTORE_OP_SUCCESS :
      output << "MSTORE_OP_SUCCESS";
      break;

    case tt_notification::CLEAR_BUFFER :
      output << "CLEAR_BUFFER";
      break;
    
    case tt_notification::BUFFER_OFFSET :
      output << "BUFFER_OFFSET";
      break;
      
    default:
      output << "unknown";
  }

  output << endl;
}

//
// TT_BO_NOTIFICATION
//

void tt_bo_notification::dump(ostream &output) const
{
  this->tt_notification::dump(output);
  output << "   offset: " << offset << endl;
}

//
// TT_LLC_INFO
//

void tt_llc_info::dump(ostream &output) const
{
  this->tt_ib::dump(output);
  output << "   sl: " << sl << endl;
  output << "   lnh: " << lnh << endl;
  output << "   dlid: " << dlid << endl;
  output << "   slid: " << slid << " (! REMOVE !)" << endl;
  output << "   pktlen: " << pktlen << " (dwords)" << endl;
}
