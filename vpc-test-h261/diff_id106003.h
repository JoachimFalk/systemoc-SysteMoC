#ifndef DIFF_ID106003_H
#define DIFF_ID106003_H
using namespace SystemC_VPC;

SC_MODULE(diff_id106003){
sc_port<sc_fifo_in_if<bool> > port_c_in_diff2diff_id106507;
sc_port<sc_fifo_in_if<bool> > port_c_lf_diff2diff_id106506;
sc_port<sc_fifo_out_if<bool> > port_diff2c_diff_dct_id106508;

void diff_id106003_process();

SC_CTOR(diff_id106003){
SC_THREAD(diff_id106003_process);
}
};
#endif
