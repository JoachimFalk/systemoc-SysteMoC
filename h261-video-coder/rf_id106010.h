#ifndef RF_ID106010_H
#define RF_ID106010_H

SC_MODULE(rf_id106010){
sc_port<sc_fifo_in_if<bool> > port_c_in_rf2rf_id106556;
sc_port<sc_fifo_out_if<bool> > port_rf2c_rf_bm_id106524;

void rf_id106010_process();

SC_CTOR(rf_id106010){
SC_THREAD(rf_id106010_process);
}
};
#endif
