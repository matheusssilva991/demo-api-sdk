#pragma once

#include <QtWidgets/QMainWindow>
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
	void onConnectBtnClicked();
    void onDeviceSelectChanged(int);
    void onDeviceInfoUpdateBtnClicked();

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
};
