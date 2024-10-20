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
	connect(ui.chooseFileNameBtn, SIGNAL(clicked()), this, SLOT(on_choose_file_name_btn_clicked()));
	connect(ui.grabBtn, SIGNAL(clicked()), this, SLOT(on_grab_btn_clicked()));
	connect(ui.stopGrabBtn, SIGNAL(clicked()), this, SLOT(on_stop_grap_btn_clicked()));
	connect(ui.deviceSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(on_device_select_changed(int)));
	connect(ui.acquisitionModeInput, SIGNAL(currentIndexChanged(int)), this, SLOT(on_acquisition_mode_changed(int)));
	connect(ui.operationModeInput, SIGNAL(currentIndexChanged(int)), this, SLOT(on_operation_mode_changed(int)));
	connect(ui.binningModeInput, SIGNAL(currentIndexChanged(int)), this, SLOT(on_binning_mode_changed(int)));
	connect(ui.gainModeInput, SIGNAL(currentIndexChanged(int)), this, SLOT(on_gain_mode_changed(int)));
	connect(ui.integrationTimeInput, SIGNAL(editingFinished()), this, SLOT(on_integration_time_changed()));
	connect(ui.numCyclesInput, SIGNAL(editingFinished()), this, SLOT(on_num_cycles_changed()));
	connect(ui.numFramesInput, SIGNAL(editingFinished()), this, SLOT(on_num_frames_changed()));
	connect(ui.framesIntervalInput, SIGNAL(editingFinished()), this, SLOT(on_frames_interval_input_changed()));
	connect(ui.cyclesIntervalInput, SIGNAL(editingFinished()), this, SLOT(on_cycles_interval_input_changed()));
}

void QtGui::on_cycles_interval_input_changed() {
	int cycle_interval = ui.cyclesIntervalInput->text().toInt();

	if (cycle_interval < 0) {
		ui.cyclesIntervalInput->setText("0");
		QMessageBox::warning(this, "Connection", "O intervalo entre ciclos n\u00E3o pode ser negativo.");
		return;
	}
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

	// Set default values
	ui.integrationTimeInput->setText("10000000");
	ui.numCyclesInput->setText("1");
	ui.numFramesInput->setText("1");
	ui.cyclesIntervalInput->setText("0");
	ui.framesIntervalInput->setText("0");
	ui.fileNameInput->setText("");
	
	QMessageBox::information(this, "Connection", "Connecting to " + host_ip);

	ui.hostIpInput->setDisabled(true);
	ui.hostIpConnectBtn->setDisabled(true);

	// Enable device info
	ui.deviceSelect->setDisabled(false);
	ui.deviceCmdPortInput->setDisabled(false);
	ui.deviceImgPortInput->setDisabled(false);
	ui.deviceIpInput->setDisabled(false);
	ui.deviceInfoUpdateBtn->setDisabled(false);
	ui.acquisitionModeInput->setDisabled(false);
	ui.operationModeInput->setDisabled(false);
	ui.TriggerModeInput->setDisabled(false);
	ui.binningModeInput->setDisabled(false);
	ui.gainModeInput->setDisabled(false);
	ui.fileNameInput->setDisabled(false);
	ui.chooseFileNameBtn->setDisabled(false);
	ui.grabBtn->setDisabled(false);
	ui.stopGrabBtn->setDisabled(false);

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

void QtGui::on_choose_file_name_btn_clicked() {
	this->file_name = QFileDialog::getSaveFileName(this, "Save file", "../images", "DAT files (*.dat);;All files (*.*)");
	ui.fileNameInput->setText(this->file_name);
}

void QtGui::on_acquisition_mode_changed(int index) {
	QString selected_acquisition_mode = ui.acquisitionModeInput->itemText(index);
	int operation_mode_index = ui.operationModeInput->currentIndex();
	QString selected_operation_mode = ui.operationModeInput->itemText(operation_mode_index);

	if (selected_acquisition_mode == "Tomografia") {
		ui.operationModeInput->setDisabled(false);
		ui.TriggerModeInput->setDisabled(false);
		ui.binningModeInput->setDisabled(false);
		ui.gainModeInput->setDisabled(false);
		ui.integrationTimeInput->setDisabled(false);

		if (selected_operation_mode == QString("Cont\u00EDnuo")) {
			ui.numCyclesInput->setDisabled(true);
			ui.numFramesInput->setDisabled(true);
			ui.cyclesIntervalInput->setDisabled(true);
			ui.framesIntervalInput->setDisabled(true);
			ui.integrationTimeInput->setDisabled(true);
		}
		else {
			ui.numCyclesInput->setDisabled(false);
			ui.numFramesInput->setDisabled(false);
			ui.cyclesIntervalInput->setDisabled(false);
			ui.framesIntervalInput->setDisabled(false);
			ui.integrationTimeInput->setDisabled(false);
		}

	} else {
		ui.operationModeInput->setDisabled(true);
		ui.TriggerModeInput->setDisabled(false);
		ui.binningModeInput->setDisabled(false);
		ui.gainModeInput->setDisabled(false);
		ui.numCyclesInput->setDisabled(true);
		ui.numFramesInput->setDisabled(true);
		ui.cyclesIntervalInput->setDisabled(true);
		ui.framesIntervalInput->setDisabled(true);
		ui.integrationTimeInput->setDisabled(false);
	}
	return;
}

void QtGui::on_operation_mode_changed(int index) {
	QString selected_operation_mode = ui.operationModeInput->itemText(index);

	if (selected_operation_mode == QString("Cont\u00EDnuo")) {
		ui.numCyclesInput->setDisabled(true);
		ui.numFramesInput->setDisabled(true);
		ui.cyclesIntervalInput->setDisabled(true);
		ui.framesIntervalInput->setDisabled(true);
		ui.integrationTimeInput->setDisabled(true);
	}
	else {
		ui.numCyclesInput->setDisabled(false);
		ui.numFramesInput->setDisabled(false);
		ui.cyclesIntervalInput->setDisabled(false);
		ui.framesIntervalInput->setDisabled(false);
		ui.integrationTimeInput->setDisabled(false);
	}

	return;
}

void QtGui::on_binning_mode_changed(int index) {
	QString selected_binning_mode = ui.binningModeInput->itemText(index);
	string binning_mode = selected_binning_mode.toStdString() == "Normal" ? "0": "1";

	if (this->xcommand->SetPara(XPARA_BINNING_MODE, binning_mode) != 1)
	{
		QMessageBox::critical(this, "Connection", "Falha ao definir o modo de binning");
	}
}

void QtGui::on_gain_mode_changed(int index) {
	QString selected_gain_mode = ui.gainModeInput->itemText(index);
	string gain_mode = selected_gain_mode.toStdString() == "Alto" ? "256" : "1";

	if (this->xcommand->SetPara(XPARA_GAIN_RANGE, gain_mode) != 1)
	{
		QMessageBox::critical(this, "Connection", "Falha ao definir o modo de ganho");
	}
}

void QtGui::on_integration_time_changed() {
	int integration_time = ui.integrationTimeInput->text().toInt();

	if (integration_time < 0) {
		ui.integrationTimeInput->setText("10000000");
		QMessageBox::warning(this, "Connection", "Tempo de integra\u00E7\u00E3o deve ser positivo.");
		return;
	}

	if (this->xcommand->SetPara(XPARA_FRAME_PERIOD, integration_time) != 1)
	{
		QMessageBox::critical(this, "Connection", "Falha ao definir o tempo de integra\u00E7\u00E3o.");
	}
}

void QtGui::on_num_cycles_changed() {
	int num_cycles = ui.numCyclesInput->text().toInt();

	if (num_cycles < 0) {
		ui.numCyclesInput->setText("1");
		QMessageBox::warning(this, "Connection", "O n\u00FAmero de ciclos deve ser positivo.");
		return;
	}
}



void QtGui::on_num_frames_changed() {
	int num_frames = ui.numFramesInput->text().toInt();

	if (num_frames < 0) {
		ui.numFramesInput->setText("1");
		QMessageBox::warning(this, "Connection", "O n\u00FAmero de frames deve ser positivo.");
		return;
	}
}

void QtGui::on_frames_interval_input_changed() {
	int frames_interval = ui.framesIntervalInput->text().toInt();

	if (frames_interval < 0) {
		ui.framesIntervalInput->setText("0");
		QMessageBox::warning(this, "Connection", "O intervalo entre frames n\u00E3o pode ser negativo.");
		return;
	}
}

void QtGui::on_grab_btn_clicked() {
	int acquisition_mode_index = ui.acquisitionModeInput->currentIndex();
	QString selected_acquisition_mode = ui.acquisitionModeInput->itemText(acquisition_mode_index);
	int operation_mode_index = ui.operationModeInput->currentIndex();
	QString selected_operation_mode = ui.operationModeInput->itemText(operation_mode_index);
	string save_file_name = this->file_name.toStdString();

	this->img_sink.SetIsSave(true);
	this->img_sink.resetCounters();

	if (save_file_name.find(".dat") == string::npos)
	{
		save_file_name = save_file_name + ".dat";
	}

	if (selected_acquisition_mode == "Tomografia") {
		if (selected_operation_mode == QString("Cont\u00EDnuo")) {
			this->img_sink.SetSaveFileName(save_file_name);

			if (!this->ximg_handle.OpenFile(save_file_name.c_str()))
			{
				QMessageBox::critical(this, "Connection", "Falha ao abrir o arquivo de imagem.");
				return;
			}

			this->xacquisition->Grab(0);
		}
		else {
			int cycle_num = ui.numCyclesInput->text().toInt();
			int cycle_frames = ui.numFramesInput->text().toInt();
			int cycle_frames_interval = ui.framesIntervalInput->text().toInt();
			int cycle_interval = ui.cyclesIntervalInput->text().toInt();

			if (cycle_num == 1) {
				this->img_sink.SetSaveFileName(save_file_name);

				if (!this->ximg_handle.OpenFile(save_file_name.c_str()))
				{
					QMessageBox::critical(this, "Connection", "Falha ao abrir o arquivo de imagem.");
					return;
				}

				this->xacquisition->Grab(cycle_frames);

				this->xevent.WaitTime(cycle_frames_interval);

			}
			else {
				string save_file_name_base = save_file_name;

				save_file_name_base = save_file_name_base.substr(0, save_file_name_base.find(".dat"));

				for (int cycle_it = 0; cycle_it < cycle_num; cycle_it++)
				{
					this->img_sink.SetIsSave(true);
					this->img_sink.resetCounters();

					save_file_name = save_file_name_base + "_cycle_" + to_string(cycle_it) + ".dat";
					this->img_sink.SetSaveFileName(save_file_name);

					if (!ximg_handle.OpenFile(save_file_name.c_str()))
					{
						QMessageBox::critical(this, "Connection", "Falha ao abrir o arquivo de imagem.");
						break;
					}

					this->xacquisition->Grab(cycle_frames);

					this->xevent.WaitTime(cycle_frames_interval);

					XSLEEP(cycle_interval * XSLEEP_UNIT);
				}

				QMessageBox::information(this, "Acquisition", "Ciclos completos.");
			}
		}
	}
	else {
		this->img_sink.SetSaveFileName(save_file_name);

		if (!this->ximg_handle.OpenFile(save_file_name.c_str()))
		{
			QMessageBox::critical(this, "Connection", "Falha ao abrir o arquivo de imagem.");
			return;
		}

		this->xacquisition->Grab(1);
	}
}

void QtGui::on_stop_grab_btn_clicked() {
	this->xacquisition->Stop();
}

QtGui::~QtGui()
{}
