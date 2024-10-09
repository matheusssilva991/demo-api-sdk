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

private:
    Ui::QtGuiClass ui;

    CmdSink cmd_sink;
	ImgSink img_sink;
    XImageHandler ximg_handle;

    XGigFactory xfactory;
    XSystem* xsystem;
    XDevice* xdevice_ptr;
    XCommand* xcommand;
    XFrameTransfer* xtransfer;
    XAcquisition* xacquisition;

	QString file_name;
};
