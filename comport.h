#ifndef comport
#define comport

#include <QObject>
#include <QString>


void startpower();
void COMConnector5_6();
void COMConnector4();
void COMConnector8();
int stm_on_com6(int y,int x);
int stm_on_com5(int y,int x);
int stm_on_mko(int y,int x);

double ctm_data_ch(int ch);
int ctm_check_fuse(int fuse);

int tech_send(int com, int x, int y);
int tech_read(int x);
QString tech_read_buf(int x,int len);
void Remote_ON();
void Remote_OFF();
void Reset_error_com6();
void Reset_error_com5();
int readerr4I();
int readerr11I();
void com6ON();
void com6OFF();
void com5ON();
void com5OFF();
int readcom5U();
int readcom5I();
int readcom6U();
int readcom6I();
void setUIcom5(double u);
void setUIcom6(double u);
void setoverUIcom5(double u,double ii);
void setoverUIcom6(double u,double ii);
void COMClose5_6();

void COMClose4();
void COMClose8();

int id_stm();


int id_tech();
QString req_stm();


QString req_tech();
int res_err_stm();


int res_err_tech();
int res_stm();


int res_tech();
int fw_stm();


int fw_tech();

/*void COMCloseOTD();
void COMConnectorOTD();
int OTDPT();
QString OTDtemper();
void OTDres1();
void OTDres2();
void OTDmeas1();
void OTDmeas2();
void OTDtm1();
void OTDtm2();*/

#endif
