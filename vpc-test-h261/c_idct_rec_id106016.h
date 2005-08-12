#ifndef C_IDCT_REC_ID106016_H
#define C_IDCT_REC_ID106016_H
using namespace SystemC_VPC;

SC_MODULE(c_idct_rec_id106016){
sc_port<sc_fifo_in_if<bool> > port_idct2c_idct_rec_id106533;
sc_port<sc_fifo_out_if<bool> > port_c_idct_rec2rec_id106534;

void c_idct_rec_id106016_process();

SC_CTOR(c_idct_rec_id106016){
SC_THREAD(c_idct_rec_id106016_process);
}
};
#endif
