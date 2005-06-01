#ifndef IQ_ID106006_H
#define IQ_ID106006_H
using namespace SystemC_VPC;

SC_MODULE(iq_id106006){
sc_port<sc_fifo_in_if<bool> > port_c_q_iq2iq_id106513;
sc_port<sc_fifo_out_if<bool> > port_iq2c_iq_idct_id106514;

void iq_id106006_process();

SC_CTOR(iq_id106006){
SC_THREAD(iq_id106006_process);
}
};
#endif
