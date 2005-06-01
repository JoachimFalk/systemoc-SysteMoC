#ifndef RLC_ID106011_H
#define RLC_ID106011_H
using namespace SystemC_VPC;

SC_MODULE(rlc_id106011){
sc_port<sc_fifo_in_if<bool> > port_c_q_rlc2rlc_id106525;
sc_port<sc_fifo_in_if<bool> > port_c_sf_rlc2rlc_id106560;
sc_port<sc_fifo_out_if<bool> > to_monitor;

void rlc_id106011_process();

SC_CTOR(rlc_id106011){
SC_THREAD(rlc_id106011_process);
}
};
#endif
