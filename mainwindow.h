#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>

class COMPortSender;
class OTD;
class MKO;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void delete_table_MKO();
    int add_string_table_MKO(int n, QString text_data, QString comm_data, QString data_hex, QString data_dec);
    int simpltst1(int x);

    void plot_point();
    void paintvalue();
    void OTDPTdata(double x,double y);
    void OTDtemd(QString data);
    void OTDerror(QString err);
    void OTDtm1(QString temp);
    void OTDtm2(QString temp);
    void OTD_fw(double x);
    void OTD_err_res(int x);
    void OTD_id();
    void status_OTD(QString data);
    void MKO_data(QString data);
    void MKO_cm_data(QString data);
    void MKO_change_ch(int x,int y);
    void statusM();
    void statusRS();
    void statusCAN();
    void startCondition();

    void on_pushButton_U1_clicked();
    void on_pushButton_U2_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_start_com6_clicked();
    void on_pushButton_start_com5_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_ctm_ch_1_clicked();
    void on_pushButton_ctm_ch_0_clicked();
    void on_pushButton_ctm_ch_2_clicked();
    void on_pushButton_ctm_ch_3_clicked();
    void on_pushButton_ctm_ch_4_clicked();
    void on_pushButton_ctm_ch_5_clicked();
    void on_pushButton_ctm_ch_6_clicked();
    void on_pushButton_ctm_ch_7_clicked();
    void on_pushButton_ctm_ch_8_clicked();
    void on_pushButton_ctm_ch_9_clicked();
    void on_pushButton_ctm_ch_10_clicked();
    void on_pushButton_ctm_ch_11_clicked();
    void on_pushButton_ctm_ch_12_clicked();
    void on_pushButton_ctm_ch_13_clicked();
    void on_pushButton_ctm_ch_14_clicked();
    void on_pushButton_ctm_ch_15_clicked();
    void on_pushButton_check_fuse_1_clicked();

    void on_pushButton_tech_fd_clicked();
    void on_pushButton_tech_hd_clicked();
    void on_tech_set_speed_clicked();
    void on_pushButton_send_tech_clicked();
    void on_tech_clear_out_3_clicked();
    void on_tech_clear_in_3_clicked();
    void on_tech_clear_buf_3_clicked();
    void on_tech_clear_out_4_clicked();
    void on_tech_clear_in_4_clicked();
    void on_tech_clear_buf_4_clicked();
    void on_pushButton_send_tech_2_clicked();
    void on_res_err_stm_clicked();
    void on_res_err_tech_clicked();
    void on_pushButton_res_stm_clicked();
    void on_pushButton_res_tech_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_check_fuse_2_clicked();
    void on_OTDPT_clicked();
    void on_OTD_reset_1_clicked();
    void on_OTD_reset_2_clicked();
    void on_OTD_meas_1_clicked();
    void on_OTD_meas_2_clicked();
    void on_OTD_nd_clicked();
    void OTD_res_st(int x);
    void on_pow_DY_osn_clicked();
    void on_pow_DY_rez_clicked();
    void on_MKO_osn_clicked();
    void on_MKO_rez_clicked();
    void on_MKO_test_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();

    void on_MKO_avt_clicked();

    void on_OTD_avt_2_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_res_otd_clicked();

    void on_res_err_otd_clicked();

signals:
    void OTD1();
    void OTD_reset1();
    void OTD_reset2();
    void OTD_meas1();
    void OTD_meas2();
    void OTD_tm1();
    void OTD_tm2();
    void OTD_nd();
    void OTD_auto(int x,int y);
    void OTD_sfw();
    void OTD_res();
    void OTD_err();
    void OTD_req();
    void MKO_stop();
    void MKO_DY(int x, int y);
    void MKO_ts(int x,int y, int z);
    void MKO_cm(int x,QString y,int z, int k);
    void MKO_cm_r(int x,int y,int z);
    void MKO_ch(int x);
    void MKO_auto(int x,int y,int adr1, int adr2);

private:
    Ui::MainWindow *ui;
    COMPortSender * m_COMPortSender;

    // TODO refactor
    QThread *threadOTD;
    OTD *myOTD;
    QThread *threadMKO;
    MKO *myMKO;

    int m_mko_kits;

    int flag_rem1 = 0;
    int flag_rem2 = 0;
    int flag_con1 = 0;
    int flag_con2 = 0;
    int k = 0;
    int flag_con3 = 0;
    int flag_con4 = 0;
    int flag_con5 = 0;
    int flag_mko_auto = 0;
    int flag_otd_auto = 0;
    int dat[1000]={0};
    int dat1[1000]={0};

};

#endif // MAINWINDOW_H
