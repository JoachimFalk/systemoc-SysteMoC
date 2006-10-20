#ifndef C_DCT_Q_ID106014_H
#define C_DCT_Q_ID106014_H
using namespace SystemC_VPC;

SC_MODULE(c_dct_q_id106014){
sc_port<sc_fifo_in_if<bool> > port_dct2c_dct_q_id106529;
sc_port<sc_fifo_out_if<bool> > port_c_dct_q2q_id106530;

void c_dct_q_id106014_process();

SC_CTOR(c_dct_q_id106014){
SC_THREAD(c_dct_q_id106014_process);
}
};
#endif
