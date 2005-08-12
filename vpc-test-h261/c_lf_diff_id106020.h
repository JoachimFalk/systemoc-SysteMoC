#ifndef C_LF_DIFF_ID106020_H
#define C_LF_DIFF_ID106020_H
using namespace SystemC_VPC;

SC_MODULE(c_lf_diff_id106020){
sc_port<sc_fifo_in_if<bool> > port_lf2c_lf_diff_id106541;
sc_port<sc_fifo_out_if<bool> > port_c_lf_diff2diff_id106542;

void c_lf_diff_id106020_process();

SC_CTOR(c_lf_diff_id106020){
SC_THREAD(c_lf_diff_id106020_process);
}
};
#endif
