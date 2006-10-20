#ifndef Q_ID106008_H
#define Q_ID106008_H
using namespace SystemC_VPC;

SC_MODULE(q_id106008){
sc_port<sc_fifo_in_if<bool> > port_c_dct_q2q_id106518;
sc_port<sc_fifo_out_if<bool> > port_q2c_q_iq_id106519;
sc_port<sc_fifo_out_if<bool> > port_q2c_q_rlc_id106520;

void q_id106008_process();

SC_CTOR(q_id106008){
SC_THREAD(q_id106008_process);
}
};
#endif
