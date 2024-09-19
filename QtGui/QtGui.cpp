#include "QtGui.h"
#include "QtDebug"
#include "QMessageBox"

#include "xsystem.h"
#include "xdevice.h"
#include "xcommand.h"

#include "xacquisition.h"
#include "xframe_transfer.h"
#include "xgig_factory.h"

#include "CmdSink.h"
#include "ImgSink.h"

#include "ximage_handler.h"
#include "xcorrection.h"

#include "xthread_win.h"

#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <locale>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include "utils.h"

#pragma comment(lib, "..\\lib\\x64\\XLibDllKosti.lib")

using namespace std;

QtGui::QtGui(QWidget *parent)
    : QMainWindow(parent),
	xsystem(nullptr),
	xdevice_ptr(nullptr),
	xcommand(nullptr),
	xtransfer(nullptr),
	xacquisition(nullptr),
	xfactory()
{
    ui.setupUi(this);

	connect(ui.hostIpConnectBtn, SIGNAL(clicked()), this, SLOT(on_btn_connect_clicked()));
}

void QtGui::on_btn_connect_clicked() {
	QString host_ip = ui.hostIpInput->text();
	char host_ip_c[20];

	if (!isValidIP(host_ip.toStdString())) {
		QMessageBox::warning(this, "Connection", "Invalid IP address");
		return;
	}

	std::strcpy(host_ip_c, host_ip.toStdString().c_str());

	// Create objects
	this->xsystem = new XSystem(host_ip_c);
	this->xsystem->RegisterEventSink(&this->cmd_sink);

	this->xcommand = new XCommand(&this->xfactory);
	this->xcommand->RegisterEventSink(&this->cmd_sink);

	this->xtransfer = new XFrameTransfer();
	this->xtransfer->RegisterEventSink(&this->img_sink);

	this->xacquisition = new XAcquisition(&this->xfactory);

	this->xacquisition->RegisterEventSink(&this->img_sink);
	this->xacquisition->RegisterFrameTransfer(this->xtransfer);

	// Open system connection
	if (!this->xsystem->Open()) {
		QMessageBox::critical(this, "Connection", "Failed to connect to " + host_ip);
		return;
	}

	/*// Find device
	if (this->xsystem->FindDevice() <= 0) {
		QMessageBox::critical(this, "Connection", "No device found on " + host_ip);
		return;
	}

	// Get device
	this->xdevice_ptr = this->xsystem->GetDevice(0);

	// Open acquisition connection
	if (this->xcommand->Open(this->xdevice_ptr)) {
		if (!this->xacquisition->Open(this->xdevice_ptr, this->xcommand)) {
			QMessageBox::critical(this, "Connection", "Failed to open acquisition");
		}

	} else {
		cout << "Falha ao abrir o canal de comando" << endl;
	}

	// Set device info
	QString mac_address(reinterpret_cast<char*>(this->xdevice_ptr->GetMAC()));
	QString firm_ver(reinterpret_cast<char*>(this->xdevice_ptr->GetFirmVer()));
	QString cmd_port = QString::number(this->xdevice_ptr->GetCmdPort());
	QString img_port = QString::number(this->xdevice_ptr->GetImgPort());

	ui.deviceIpInput->setText(this->xdevice_ptr->GetIP());
	ui.deviceTypeInput->setText(this->xdevice_ptr->GetDeviceType());
	ui.deviceMacInput->setText(mac_address);
	ui.deviceFinwareInput->setText(firm_ver);
    ui.deviceCmdPortInput->setText(cmd_port);
	ui.deviceImgPortInput->setText(img_port);
	ui.deviceSerialInput->setText(this->xdevice_ptr->GetSerialNum());
	*/
	QMessageBox::information(this, "Connection", "Connecting to " + host_ip);

	ui.hostIpInput->setDisabled(true);
	ui.hostIpConnectBtn->setDisabled(true);
}

QtGui::~QtGui()
{}
