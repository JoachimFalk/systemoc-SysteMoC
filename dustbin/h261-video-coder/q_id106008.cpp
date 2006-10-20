#include "hscd_vpc_Director.h"
#include "systemc.h"
#include "names.h"
#include "q_id106008.h"

void q_id106008::q_id106008_process()
{
while(true){
bool var_c_dct_q2q_id106518 = port_c_dct_q2q_id106518->read();
bool var_q2c_q_iq_id106519 = 0;
bool var_q2c_q_rlc_id106520 = 0;
AbstractComponent& r=Director::getInstance().getResource(Q_ID106008);
r.compute(Q_ID106008);
cout << "Q triggered!" << endl << endl;
port_q2c_q_iq_id106519->write(var_q2c_q_iq_id106519);
port_q2c_q_rlc_id106520->write(var_q2c_q_rlc_id106520);
}
}
