#ifndef DCT_ID106002_H
#define DCT_ID106002_H
using namespace SystemC_VPC;

SC_MODULE(dct_id106002){
sc_port<sc_fifo_in_if<bool> > port_c_diff_dct2dct_id106504;
sc_port<sc_fifo_out_if<bool> > port_dct2c_dct_q_id106505;

void dct_id106002_process();

SC_CTOR(dct_id106002){
SC_THREAD(dct_id106002_process);
}
};
#endif
