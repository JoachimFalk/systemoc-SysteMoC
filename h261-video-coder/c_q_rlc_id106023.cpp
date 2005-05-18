#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "c_q_rlc_id106023.h"

void c_q_rlc_id106023::c_q_rlc_id106023_process()
{
while(true){
bool var_q2c_q_rlc_id106547 = port_q2c_q_rlc_id106547->read();
bool var_c_q_rlc2rlc_id106548 = 0;
AbstractComponent& r=Director::getInstance().getResource(Director::C_Q_RLC_ID106023);
r.compute(Director::C_Q_RLC_ID106023);
cout << "C_Q_RLC triggered!" << endl << endl;
port_c_q_rlc2rlc_id106548->write(var_c_q_rlc2rlc_id106548);
}
}
