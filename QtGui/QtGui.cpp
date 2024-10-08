#include "QtGui.h"
#include "QtDebug"
#include "QMessageBox"
#include "QFileDialog"

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

	connect(ui.hostIpConnectBtn, SIGNAL(clicked()), this, SLOT(on_connect_btn_clicked()));
	connect(ui.deviceInfoUpdateBtn, SIGNAL(clicked()), this, SLOT(on_device_info_update_btn_clicked()));
	connect(ui.deviceSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(on_device_select_changed(int)));
}

void QtGui::on_connect_btn_clicked() {
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

	/*// Open system connection
	if (!this->xsystem->Open()) {
		QMessageBox::critical(this, "Connection", "Failed to connect to " + host_ip);
		return;
	}

	// Find device
	int num_devices = this->xsystem->FindDevice();
	if (num_devices <= 0) {
		QMessageBox::critical(this, "Connection", "No device found on " + host_ip);
		return;
	}
	*/
	
	QMessageBox::information(this, "Connection", "Connecting to " + host_ip);

	ui.hostIpInput->setDisabled(true);
	ui.hostIpConnectBtn->setDisabled(true);

	// Enable device info
	ui.deviceSelect->setDisabled(false);
	ui.deviceCmdPortInput->setDisabled(false);
	ui.deviceImgPortInput->setDisabled(false);
	ui.deviceSerialInput->setDisabled(false);
	ui.deviceTypeInput->setDisabled(false);
	ui.deviceMacInput->setDisabled(false);
	ui.deviceFirmwareInput->setDisabled(false);
	ui.deviceIpInput->setDisabled(false);
	ui.deviceInfoUpdateBtn->setDisabled(false);

	int num_devices = 4;
	for (int i = 0; i < num_devices; i++) {
		ui.deviceSelect->addItem("Dispositivo " + QString::number(i + 1));
	}
}

void QtGui::on_device_select_changed(int index) {
	QString selected_option = ui.deviceSelect->itemText(index);
	int device_id = selected_option.split(" ")[1].toInt() - 1;

	if (this->xdevice_ptr != nullptr) {
		this->xacquisition->Close();
		this->xcommand->Close();
		delete this->xdevice_ptr;
	}

	//this->xdevice_ptr = this->xsystem->GetDevice(deviceId);

	// Open acquisition connection
	/*if (this->xcommand->Open(this->xdevice_ptr)) {
		if (!this->xacquisition->Open(this->xdevice_ptr, this->xcommand)) {
			QMessageBox::critical(this, "Connection", "Failed to open acquisition");
		}
	}
	else {
		QMessageBox::critical(this, "Connection", "Failed to open command");
	}*/

	/*QString mac_address(reinterpret_cast<char*>(this->xdevice_ptr->GetMAC()));
	QString firm_ver(reinterpret_cast<char*>(this->xdevice_ptr->GetFirmVer()));
	QString cmd_port = QString::number(this->xdevice_ptr->GetCmdPort());
	QString img_port = QString::number(this->xdevice_ptr->GetImgPort());

	ui.deviceIpInput->setText(this->xdevice_ptr->GetIP());
	ui.deviceTypeInput->setText(this->xdevice_ptr->GetDeviceType());
	ui.deviceMacInput->setText(mac_address);
	ui.deviceFirmwareInput->setText(firm_ver);
	ui.deviceCmdPortInput->setText(cmd_port);
	ui.deviceImgPortInput->setText(img_port);
	ui.deviceSerialInput->setText(this->xdevice_ptr->GetSerialNum()); */
}

void QtGui::on_device_info_update_btn_clicked() {
	int device_id = ui.deviceSelect->currentIndex() - 1;

	/*QString ip = ui.deviceIpInput->text();
	QString type = ui.deviceTypeInput->text();
	QString mac = ui.deviceMacInput->text();
	QString firmware = ui.deviceFirmwareInput->text();
	QString cmd_port = ui.deviceCmdPortInput->text();
	QString img_port = ui.deviceImgPortInput->text();
	QString serial = ui.deviceSerialInput->text();

	this->xdevice_ptr->SetIP(ip.toStdString().c_str());
	this->xdevice_ptr->SetDeviceType(type.toStdString().c_str());
	this->xdevice_ptr->SetMAC(mac.toStdString().c_str());
	this->xdevice_ptr->SetFirmVer(firmware.toStdString().c_str());
	this->xdevice_ptr->SetCmdPort(cmd_port.toInt());
	this->xdevice_ptr->SetImgPort(img_port.toInt());
	this->xdevice_ptr->SetSerialNum(serial.toStdString().c_str()); */

	if (1 == this->xsystem->ConfigureDevice(xdevice_ptr))
	{
		this->xdevice_ptr = this->xsystem->GetDevice(device_id);
	}
	else
	{
		QMessageBox::critical(this, "Connection", "Failed to configure device");
		return;
	}
}

QtGui::~QtGui()
{}
