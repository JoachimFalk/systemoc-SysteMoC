#ifndef C_DIFF_DCT_ID106015_H
#define C_DIFF_DCT_ID106015_H
using namespace SystemC_VPC;

SC_MODULE(c_diff_dct_id106015){
sc_port<sc_fifo_in_if<bool> > port_diff2c_diff_dct_id106531;
sc_port<sc_fifo_out_if<bool> > port_c_diff_dct2dct_id106532;

void c_diff_dct_id106015_process();

SC_CTOR(c_diff_dct_id106015){
SC_THREAD(c_diff_dct_id106015_process);
}
};
#endif
