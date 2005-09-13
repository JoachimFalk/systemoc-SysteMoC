#ifndef IDCT_ID106004_H
#define IDCT_ID106004_H
using namespace SystemC_VPC;

SC_MODULE(idct_id106004){
sc_port<sc_fifo_in_if<bool> > port_c_iq_idct2idct_id106509;
sc_port<sc_fifo_out_if<bool> > port_idct2c_idct_rec_id106510;

void idct_id106004_process();

SC_CTOR(idct_id106004){
SC_THREAD(idct_id106004_process);
}
};
#endif
