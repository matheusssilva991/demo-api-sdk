#pragma once

#include <QtWidgets/QMainWindow>
#include <QObject>
#include "ui_QtGui.h"

#include "CmdSink.h"
#include "ImgSink.h"
#include "ximage_handler.h"
#include "xsystem.h"
#include "xdevice.h"
#include "xcommand.h"
#include "xacquisition.h"
#include "xframe_transfer.h"
#include "xgig_factory.h"

class QtGui : public QMainWindow
{
    Q_OBJECT

public:
    QtGui(QWidget *parent = nullptr);
    ~QtGui();
    QString get_file_name();
    uint32_t get_frame_count();
    uint32_t get_lost_frame_count();
    bool get_is_save();
    std::string get_save_file_name();
    XImageHandler* get_ximage_handler();
    XEvent* get_xevent();
    void set_frame_count(uint32_t);
    void set_lost_frame_count(uint32_t);
    void set_is_save(bool);
    void set_save_file_name(std::string);

private slots:
	void on_connect_btn_clicked();
    void on_device_select_changed(int);
    void on_acquisition_mode_changed(int);
    void on_device_info_update_btn_clicked();
    void on_choose_file_name_btn_clicked();
	void on_operation_mode_changed(int);
    void on_grab_btn_clicked();
	void on_stop_grab_btn_clicked();
    void on_binning_mode_changed(int);
    void on_gain_mode_changed(int);
	void on_integration_time_changed();
	void on_num_cycles_changed();
    void on_cycles_interval_input_changed();
	void on_num_frames_changed();

private:
    Ui::QtGuiClass ui;
    CmdSink* cmd_sink;
	ImgSink* img_sink;
    XImageHandler ximg_handle;
	XEvent xevent;
    XGigFactory xfactory;
    XSystem* xsystem;
    XDevice* xdevice_ptr;
    XCommand xcommand;
    XFrameTransfer xtransfer;
    XAcquisition xacquisition;
	QString file_name;
    uint32_t lost_frame_count; // Contagem de quadros perdidos
    uint32_t frame_count; // Contagem de quadros
    bool is_save; // Flag para salvar
    std::string save_file_name; // Nome do arquivo para salvar
};
