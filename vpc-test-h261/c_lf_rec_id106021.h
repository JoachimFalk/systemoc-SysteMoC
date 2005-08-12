#ifndef C_LF_REC_ID106021_H
#define C_LF_REC_ID106021_H
using namespace SystemC_VPC;

SC_MODULE(c_lf_rec_id106021){
sc_port<sc_fifo_in_if<bool> > port_lf2c_lf_rec_id106543;
sc_port<sc_fifo_out_if<bool> > port_c_lf_rec2rec_id106544;

void c_lf_rec_id106021_process();

SC_CTOR(c_lf_rec_id106021){
SC_THREAD(c_lf_rec_id106021_process);
}
};
#endif
