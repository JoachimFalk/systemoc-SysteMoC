// ============================================================================
//
//     Project: TLM WG, PVT examples
//
//     Authors: Tim Kogel, CoWare Inc.
//              Othman Fathy, Mentor Graphics
//		Trevor Wieman, Intel Corp.
//
// Description: simple PVT bus
//
// ============================================================================


#ifndef TLM_PVT_BUS_CPP
#define TLM_PVT_BUS_CPP

template <typename ADDRESS, typename DATA, tlm::tlm_data_mode DATA_MODE>
tlm_pvt_bus< ADDRESS, DATA, DATA_MODE >::tlm_pvt_bus(sc_module_name mod, 
						     unsigned int nbr_initiators, 
						     unsigned int nbr_targets):
  sc_module(mod),
  p_tlm_s("SlaveP"),
  p_tlm_m("MasterP"),
  m_index(0),
  m_previous_read_index(0),
  m_outstanding_reads(0),
  m_clk_period(1, SC_NS),
  m_fifo(),
  m_nbr_initiators(nbr_initiators),
  m_nbr_targets(nbr_targets),
  m_response_resource_busy_until(SC_ZERO_TIME)
{
}

template <typename ADDRESS, typename DATA, tlm::tlm_data_mode DATA_MODE>
void
tlm_pvt_bus<ADDRESS, DATA, DATA_MODE>::end_of_elaboration() 
{
  assert ( m_nbr_initiators == (unsigned int) p_tlm_s.size() );
  assert ( m_nbr_targets    == (unsigned int) p_tlm_m.size() );
  SC_THREAD(request_thread);
  for (int i=0; i<p_tlm_s.size();i++) {
    sensitive << p_tlm_s[i]->ok_to_peek();
#if (defined DEBUG_TLM_PVT_BUS || defined DEBUG_ALL) 
    cout << " bus_thread sensitive to interface " << i << endl;
#endif
  }
  dont_initialize();
  
  SC_THREAD(response_thread);
  for (int i=0; i<p_tlm_m.size();i++) {
    sensitive << p_tlm_m[i]->ok_to_peek();
#if (defined DEBUG_TLM_PVT_BUS || defined DEBUG_ALL) 
    cout << " response_thread sensitive to interface " << i << endl;
#endif
  }
  dont_initialize();
}


template <typename ADDRESS, typename DATA, tlm::tlm_data_mode DATA_MODE>
void
tlm_pvt_bus<ADDRESS, DATA, DATA_MODE>::request_thread() {  
  int n_active_masters;
  int active_master_id;
  int active_slave_id;
  unsigned int transfer_cycles;
  bool ok;

  while(true) { 
    while(true) { 
      n_active_masters = 0;
      active_master_id = -1; // negative value means "undefined"
      active_slave_id = -1; // negative value means "undefined"
      

      for (unsigned int i=0; i<m_nbr_initiators; i++) {
	if(p_tlm_s[i]->nb_can_peek()){
	  n_active_masters++;
	  active_master_id = i; 
	}
      }
#if (defined DEBUG_TLM_PVT_BUS || defined DEBUG_ALL) 
      if (n_active_masters) {
        cout << sc_time_stamp() << " tlm_pvt_bus(" << name() 
	     << ") request_thread, has pending requests on master port(s): ";
	for (unsigned int i=0; i < m_nbr_initiators; i++) {
	  if (p_tlm_s[i]->nb_can_peek())
	    cout << i << " ";
	}
        cout << endl;
      }
#endif
	
      if(!n_active_masters){
	break;
      }

      if (!p_tlm_s[active_master_id]->nb_peek(m_req) ) {
	cerr << "ERROR, " << sc_time_stamp() << " tlm_pvt_bus(" << name() 
	     << ") request_thread, no request from master " 
	     << active_master_id << endl;
      }

      // Distribute requests among the slaves.
      m_index = m_req.get_address() % m_nbr_targets;
      if (m_req.get_command() == READ) {
        // Only READs produce a response in this example.
	if ( m_previous_read_index != m_index ) {
          // Can handle multiple oustanding responses but only for the same
          // slave.  Once a different slave is addressed, must flush the 
          // pending responses before recording the new slave to wait on.
#if (defined DEBUG_TLM_PVT_BUS || defined DEBUG_ALL) 
	  cout << sc_time_stamp() << " tlm_pvt_bus(" << name() 
	       << ") request_thread, m_outstanding_reads :" << m_outstanding_reads
	       << ", previous slave id :" << m_previous_read_index
	       << ", current slave id :" << m_index <<endl;
#endif
	  while ( m_outstanding_reads > 0 ) {
	    wait(m_resp_event);
	  }
	  m_previous_read_index = m_index;	    
	}
	m_outstanding_reads++;

        // save initiator id for the response path
	m_fifo.write(active_master_id);

        // read request has no data
	transfer_cycles = 1;
      } else {
	transfer_cycles = m_req.get_block_size();
      }
      
      m_request_transfer_delay = transfer_cycles * m_clk_period;
#if (defined DEBUG_TLM_PVT_BUS || defined DEBUG_ALL) 
      cout << sc_time_stamp() << " tlm_pvt_bus(" << name() 
	   << ") request_thread, transfer-delay : " 
	   << m_request_transfer_delay << endl;
#endif
      
      p_tlm_m[m_index]->put(m_req);
      
      // bus resource is busy for the duration the transfer
      wait(m_request_transfer_delay);
      // after transfer is finished => release the request from the initiating master:
      ok = p_tlm_s[active_master_id]->nb_get(m_req);
      assert(ok);
    }
#if (defined DEBUG_TLM_PVT_BUS || defined DEBUG_ALL) 
    cout << sc_time_stamp() << " tlm_pvt_bus(" << name() 
	 << ") request_thread, suspend \n";
#endif
    wait();
  }
}
    

template <typename ADDRESS, typename DATA, tlm::tlm_data_mode DATA_MODE>
void
tlm_pvt_bus<ADDRESS, DATA, DATA_MODE>::response_thread() {  
  int master_id,target_id;
  sc_time now;
  bool ok;
  while(true) {
    while(true) {
      now = sc_time_stamp();
      if(p_tlm_m[m_previous_read_index]->nb_can_peek()){
	target_id = m_previous_read_index;
      } else {
	break;
      }
#if (defined DEBUG_TLM_PVT_BUS || defined DEBUG_ALL) 
      cout << now << " tlm_pvt_bus(" << name() << ") response_thread"
	   << ", get response from slave with id " << target_id << endl;
#endif 
      ok = p_tlm_m[target_id]->nb_peek(m_resp);
      assert(ok);
      
      // send the reponse to initiating master:
      master_id = m_fifo.read();
      p_tlm_s[master_id]->put(m_resp);
      
      // compute transfer delay
      m_response_transfer_delay = m_resp.get_block_size() * m_clk_period;
      m_outstanding_reads--;
#if (defined DEBUG_TLM_PVT_BUS || defined DEBUG_ALL) 
      cout << now << " tlm_pvt_bus(" << name() << ") response_thread"
	   << ", block size : " << m_resp.get_block_size()
	   << ", response_transfer_delay : " << m_response_transfer_delay 
	   << ", outstanding reads : " << m_outstanding_reads << endl;
#endif    
      
      if (m_outstanding_reads==0) {
	m_resp_event.notify();
      }
      wait(m_response_transfer_delay);
      // release target channel
      ok = p_tlm_m[target_id]->nb_get(m_resp);
      assert(ok);
    }
#if (defined DEBUG_TLM_PVT_BUS || defined DEBUG_ALL) 
    cout << now << " tlm_pvt_bus(" << name() << ") response_thread"
	 << ", suspend" << endl;
#endif 
    wait();
  }
}

#endif
