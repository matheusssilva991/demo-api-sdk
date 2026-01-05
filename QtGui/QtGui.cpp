/**
 * @file QtGui.cpp
 * @brief Implementação da interface gráfica para controle do detector X-Panel 1412i
 *
 * Esta classe fornece uma interface amigável para:
 * - Conexão e descoberta de detectores na rede
 * - Configuração de parâmetros de operação
 * - Captura de imagens individuais ou em sequência (tomografia)
 * - Monitoramento em tempo real do processo de aquisição
 *
 * @author Matheus Silva
 * @date 2024-2026
 */

#include "QtGui.h"
#include "QtDebug"
#include "QMessageBox"
#include "QFileDialog"

#include "stdafx.h"

// Cabeçalhos da API X-LIB
#include "xsystem.h"  // Gerenciamento do sistema
#include "xdevice.h"  // Controle do dispositivo
#include "xcommand.h" // Envio de comandos

#include "xacquisition.h"	 // Aquisição de imagens
#include "xframe_transfer.h" // Transferência de frames
#include "xgig_factory.h"	 // Factory de comunicação

#include "CmdSink.h" // Manipulador de eventos de comando
#include "ImgSink.h" // Manipulador de eventos de imagem

#include "ximage_handler.h" // Salvamento de imagens
#include "xcorrection.h"	// Correções de imagem

#include "xthread_win.h" // Threads para Windows

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

/**
 * @brief Tamanho do buffer alocado para frames capturados
 *
 * Buffer maior em x64 para melhor performance com mais memória disponível
 */
#if defined(_WIN64)
uint32_t frame_buffer_size = 700; // Buffer para arquitetura 64-bit
#else
uint32_t frame_buffer_size = 400; // Buffer para arquitetura 32-bit
#endif

/**
 * @brief Construtor da classe QtGui
 * @param parent Widget pai (padrão: nullptr)
 *
 * Inicializa todos os componentes da interface gráfica e conecta
 * os sinais (eventos) dos widgets aos respectivos slots (manipuladores).
 *
 * Componentes inicializados:
 * - Sistema de comunicação com detector (nullptr inicialmente)
 * - Manipuladores de eventos (CmdSink e ImgSink)
 * - Factory de comunicação Gigabit Ethernet
 * - Sistema de comando e aquisição
 */
QtGui::QtGui(QWidget *parent)
	: QMainWindow(parent),
	  xsystem(nullptr),
	  xdevice_ptr(nullptr),
	  xtransfer(),
	  xfactory(),
	  xcommand(&xfactory),
	  xacquisition(&xfactory),
	  cmd_sink(new CmdSink(this)),
	  img_sink(new ImgSink(this))
{
	ui.setupUi(this);

	// ========================================================================
	// CONEXÃO DE SINAIS E SLOTS (EVENTOS E MANIPULADORES)
	// ========================================================================

	// Botão de conexão ao host
	connect(ui.hostIpConnectBtn, SIGNAL(clicked()), this, SLOT(on_connect_btn_clicked()));

	// Seleção de dispositivo
	connect(ui.deviceSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(on_device_select_changed(int)));

	// Atualização de informações do dispositivo
	connect(ui.deviceInfoUpdateBtn, SIGNAL(clicked()), this, SLOT(on_device_info_update_btn_clicked()));

	// Seleção de arquivo de destino
	connect(ui.chooseFileNameBtn, SIGNAL(clicked()), this, SLOT(on_choose_file_name_btn_clicked()));

	// Controle de captura
	connect(ui.grabBtn, SIGNAL(clicked()), this, SLOT(on_grab_btn_clicked()));
	connect(ui.stopGrabBtn, SIGNAL(clicked()), this, SLOT(on_stop_grap_btn_clicked()));

	// Modos de operação
	connect(ui.acquisitionModeInput, SIGNAL(currentIndexChanged(int)), this, SLOT(on_acquisition_mode_changed(int)));
	connect(ui.operationModeInput, SIGNAL(currentIndexChanged(int)), this, SLOT(on_operation_mode_changed(int)));
	connect(ui.binningModeInput, SIGNAL(currentIndexChanged(int)), this, SLOT(on_binning_mode_changed(int)));
	connect(ui.gainModeInput, SIGNAL(currentIndexChanged(int)), this, SLOT(on_gain_mode_changed(int)));

	// Parâmetros de captura
	connect(ui.integrationTimeInput, SIGNAL(editingFinished()), this, SLOT(on_integration_time_changed()));
	connect(ui.numCyclesInput, SIGNAL(editingFinished()), this, SLOT(on_num_cycles_changed()));
	connect(ui.numFramesInput, SIGNAL(editingFinished()), this, SLOT(on_num_frames_changed()));
	connect(ui.cyclesIntervalInput, SIGNAL(editingFinished()), this, SLOT(on_cycles_interval_input_changed()));
}

/**
 * @brief Slot: Conecta ao host e descobre dispositivos na rede
 *
 * Acionado ao clicar no botão "Conectar". Realiza:
 * 1. Validação do endereço IP informado
 * 2. Criação do sistema de comunicação
 * 3. Registro dos manipuladores de eventos
 * 4. Abertura da conexão com o host
 * 5. Busca por dispositivos na rede
 * 6. Abertura do canal de imagem do primeiro dispositivo
 *
 * @note Exibe mensagens de erro em caso de falha na validação,
 *       conexão ou descoberta de dispositivos
 */
void QtGui::on_connect_btn_clicked()
{
	QString host_ip = ui.hostIpInput->text();
	char host_ip_c[20];

	// Valida formato do endereço IP
	if (!isValidIP(host_ip.toStdString()))
	{
		QMessageBox::warning(this, "Aviso", "Endere\u00E7o de IP inv\u00E1lido.");
		return;
	}

	std::strcpy(host_ip_c, host_ip.toStdString().c_str());

	// ====================================================================
	// CRIAÇÃO E CONFIGURAÇÃO DOS OBJETOS DA API
	// ====================================================================

	// Cria sistema de comunicação
	this->xsystem = new XSystem(host_ip_c);
	this->xsystem->RegisterEventSink(this->cmd_sink);

	// Configura sistema de comando
	this->xcommand.RegisterEventSink(this->cmd_sink);

	// Configura sistema de transferência
	this->xtransfer.RegisterEventSink(this->img_sink);

	// Configura sistema de aquisição
	this->xacquisition.RegisterEventSink(this->img_sink);
	this->xacquisition.RegisterFrameTransfer(&this->xtransfer);

	// Abre conexão com o host
	if (!this->xsystem->Open())
	{
		QMessageBox::critical(this, "Erro", "Falha ao conectar ao host " + host_ip + ".");
		return;
	}

	// Find device
	// int num_devices = this->xsystem->FindDevice();
	int num_devices = 1;
	if (num_devices <= 0)
	{
		QMessageBox::warning(this, "Aviso", "Nenhum dispositivo encontrado.");
		return;
	}

	// Set default values
	ui.integrationTimeInput->setText("10000000");
	ui.numCyclesInput->setText("0");
	ui.numFramesInput->setText("0");
	ui.cyclesIntervalInput->setText("0");
	ui.fileNameInput->setText("");

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
	ui.triggerModeInput->setDisabled(false);
	ui.binningModeInput->setDisabled(false);
	ui.gainModeInput->setDisabled(false);
	ui.fileNameInput->setDisabled(false);
	ui.chooseFileNameBtn->setDisabled(false);
	ui.grabBtn->setDisabled(false);
	ui.stopGrabBtn->setDisabled(false);
	ui.numCyclesInput->setDisabled(false);
	ui.numFramesInput->setDisabled(false);
	ui.cyclesIntervalInput->setDisabled(false);
	ui.integrationTimeInput->setDisabled(false);

	for (int i = 0; i < num_devices; i++)
	{
		ui.deviceSelect->addItem("Dispositivo " + QString::number(i + 1));
	}

	QMessageBox::information(this, "Status", "Conectado com sucesso ao host " + host_ip + ".");
}

void QtGui::on_device_select_changed(int index)
{
	QString selected_option = ui.deviceSelect->itemText(index);
	int device_id = selected_option.split(" ")[1].toInt() - 1;

	if (this->xdevice_ptr != nullptr)
	{
		this->xacquisition.Close();
		this->xcommand.Close();
		delete this->xdevice_ptr;
	}

	// this->xdevice_ptr = this->xsystem->GetDevice(device_id);
	this->xdevice_ptr = new XDevice(this->xsystem);
	this->xdevice_ptr->SetIP("192.168.1.2");
	this->xdevice_ptr->SetCmdPort(3000);
	this->xdevice_ptr->SetImgPort(4001);
	this->xdevice_ptr->SetDeviceType("1412_KOSTI");
	this->xdevice_ptr->SetSerialNum("1234567890", 10);
	this->xdevice_ptr->SetMAC((uint8_t *)"123456");
	this->xdevice_ptr->SetFirmBuildVer(123);
	this->xdevice_ptr->SetFirmVer(123);

	// Open acquisition connection
	if (this->xcommand.Open(this->xdevice_ptr))
	{
		// QMessageBox::information(this, "Status", "Canal de comando aberto com sucesso.");
		if (!this->xacquisition.Open(this->xdevice_ptr, &this->xcommand))
		{
			QMessageBox::critical(this, "Erro", "Falha ao abrir o canal de aquisi��o.");
		}
		else
		{
			// QMessageBox::information(this, "Status", "Canal de imagem aberto com sucesso.");
		}
	}
	else
	{
		QMessageBox::critical(this, "Erro", "Falha ao abrir o canal de comando.");
	}

	// QString mac_address(reinterpret_cast<char*>(this->xdevice_ptr->GetMAC()));
	// QString firm_ver(reinterpret_cast<char*>(this->xdevice_ptr->GetFirmVer()));
	QString cmd_port = QString::number(this->xdevice_ptr->GetCmdPort());
	QString img_port = QString::number(this->xdevice_ptr->GetImgPort());

	ui.deviceIpInput->setText(this->xdevice_ptr->GetIP());
	// ui.deviceTypeInput->setText(this->xdevice_ptr->GetDeviceType());
	ui.deviceTypeInput->setText("1412_KOSTI");
	// ui.deviceMacInput->setText(mac_address);
	ui.deviceMacInput->setText("");
	// ui.deviceFirmwareInput->setText(firm_ver);
	ui.deviceCmdPortInput->setText(cmd_port);
	ui.deviceImgPortInput->setText(img_port);
	// ui.deviceSerialInput->setText(this->xdevice_ptr->GetSerialNum());
}

void QtGui::on_device_info_update_btn_clicked()
{
	int device_id = ui.deviceSelect->currentIndex() - 1;

	QString ip = ui.deviceIpInput->text();
	QString cmd_port = ui.deviceCmdPortInput->text();
	QString img_port = ui.deviceImgPortInput->text();

	this->xdevice_ptr->SetIP(ip.toStdString().c_str());
	this->xdevice_ptr->SetCmdPort(cmd_port.toInt());
	this->xdevice_ptr->SetImgPort(img_port.toInt());

	if (1 == this->xsystem->ConfigureDevice(xdevice_ptr))
	{
		this->xdevice_ptr = this->xsystem->GetDevice(device_id);
	}
	else
	{
		QMessageBox::critical(this, "Erro", "Falha ao configurar dispositivo.");
		return;
	}
}

void QtGui::on_choose_file_name_btn_clicked()
{
	this->file_name = QFileDialog::getSaveFileName(this, "Save file", "../images", "DAT files (*.dat);;All files (*.*)");
	ui.fileNameInput->setText(this->file_name);
}

void QtGui::on_acquisition_mode_changed(int index)
{
	QString selected_acquisition_mode = ui.acquisitionModeInput->itemText(index);

	if (selected_acquisition_mode == "Tomografia")
	{
		ui.operationModeInput->setDisabled(false);
		ui.triggerModeInput->setDisabled(false);
		ui.binningModeInput->setDisabled(false);
		ui.gainModeInput->setDisabled(false);
		ui.integrationTimeInput->setDisabled(false);
	}
	else
	{
		ui.operationModeInput->setDisabled(true);
		ui.triggerModeInput->setDisabled(false);
		ui.binningModeInput->setDisabled(false);
		ui.gainModeInput->setDisabled(false);
		ui.numCyclesInput->setDisabled(true);
		ui.numFramesInput->setDisabled(true);
		ui.cyclesIntervalInput->setDisabled(true);
		ui.integrationTimeInput->setDisabled(false);
	}
	return;
}

void QtGui::on_operation_mode_changed(int index)
{
	QString selected_operation_mode = ui.operationModeInput->itemText(index);

	if (selected_operation_mode == QString("Cont\u00EDnuo"))
	{
		ui.numCyclesInput->setDisabled(false);
		ui.numFramesInput->setDisabled(false);
		ui.cyclesIntervalInput->setDisabled(false);
		ui.integrationTimeInput->setDisabled(false);
	}
	else
	{
		ui.numCyclesInput->setDisabled(true);
		ui.numFramesInput->setDisabled(true);
		ui.cyclesIntervalInput->setDisabled(true);
		ui.integrationTimeInput->setDisabled(true);
	}

	return;
}

void QtGui::on_binning_mode_changed(int index)
{
	QString selected_binning_mode = ui.binningModeInput->itemText(index);
	int binning_mode = selected_binning_mode.toStdString() == "Normal" ? 0 : 1;

	if (1 != this->xcommand.SetPara(XPARA_BINNING_MODE, binning_mode))
	{
		QMessageBox::critical(this, "Connection", "Falha ao definir o modo de binning");
	}
}

void QtGui::on_gain_mode_changed(int index)
{
	QString selected_gain_mode = ui.gainModeInput->itemText(index);
	int gain_mode = selected_gain_mode.toStdString() == "Alto" ? 256 : 1;

	if (this->xcommand.SetPara(XPARA_GAIN_RANGE, gain_mode) != 1)
	{
		QMessageBox::critical(this, "Erro", "Falha ao definir o modo de ganho.");
	}
}

void QtGui::on_integration_time_changed()
{
	int integration_time = ui.integrationTimeInput->text().toInt();

	if (integration_time < 0)
	{
		ui.integrationTimeInput->setText("10000000");
		QMessageBox::warning(this, "Aviso", "O tempo de integra\u00E7\u00E3o deve ser positivo.");
		return;
	}

	if (this->xcommand.SetPara(XPARA_FRAME_PERIOD, integration_time) != 1)
	{
		QMessageBox::critical(this, "Erro", "Falha ao definir o tempo de integra\u00E7\u00E3o.");
	}
}

void QtGui::on_num_cycles_changed()
{
	int num_cycles = ui.numCyclesInput->text().toInt();

	if (num_cycles < 0)
	{
		ui.numCyclesInput->setText("0");
		QMessageBox::warning(this, "Aviso", "O n\u00FAmero de ciclos deve ser positivo.");
		return;
	}
}

void QtGui::on_num_frames_changed()
{
	int num_frames = ui.numFramesInput->text().toInt();

	if (num_frames < 0)
	{
		ui.numFramesInput->setText("0");
		QMessageBox::warning(this, "Aviso", "O n\u00FAmero de frames deve ser positivo.");
		return;
	}
}

void QtGui::on_cycles_interval_input_changed()
{
	int cycle_interval = ui.cyclesIntervalInput->text().toInt();

	if (cycle_interval < 0)
	{
		ui.cyclesIntervalInput->setText("0");
		QMessageBox::warning(this, "Aviso", "O intervalo entre ciclos deve ser positivo.");
		return;
	}
}

void QtGui::on_grab_btn_clicked()
{
	int acquisition_mode_index = ui.acquisitionModeInput->currentIndex();
	QString selected_acquisition_mode = ui.acquisitionModeInput->itemText(acquisition_mode_index);
	int operation_mode_index = ui.operationModeInput->currentIndex();
	QString selected_operation_mode = ui.operationModeInput->itemText(operation_mode_index);
	string save_file_name = this->file_name.toStdString();

	this->set_is_save(true);
	this->set_frame_count(0);
	this->set_lost_frame_count(0);

	if (save_file_name.find(".dat") == string::npos)
	{
		save_file_name = save_file_name + ".dat";
	}

	if (selected_acquisition_mode == "Tomografia")
	{
		if (selected_operation_mode != QString("Cont\u00EDnuo"))
		{
			this->set_save_file_name(save_file_name);

			if (!this->ximg_handle.OpenFile(save_file_name.c_str()))
			{
				QMessageBox::critical(this, "Connection", "Falha ao abrir o arquivo de imagem.");
				return;
			}

			this->xacquisition.Grab(0);
		}
		else
		{
			int cycle_num = ui.numCyclesInput->text().toInt();
			int cycle_frames = ui.numFramesInput->text().toInt();
			int cycle_interval = ui.cyclesIntervalInput->text().toInt();

			if (cycle_num == 1)
			{
				this->set_save_file_name(save_file_name);

				if (!this->ximg_handle.OpenFile(save_file_name.c_str()))
				{
					QMessageBox::critical(this, "Connection", "Falha ao abrir o arquivo de imagem.");
					return;
				}

				this->xacquisition.Grab(cycle_frames);
			}
			else
			{
				string save_file_name_base = save_file_name;

				save_file_name_base = save_file_name_base.substr(0, save_file_name_base.find(".dat"));

				for (int cycle_it = 0; cycle_it < cycle_num; cycle_it++)
				{
					this->set_is_save(true);
					this->set_frame_count(0);
					this->set_lost_frame_count(0);

					save_file_name = save_file_name_base + "_cycle_" + to_string(cycle_it) + ".dat";
					this->set_save_file_name(save_file_name);

					if (!ximg_handle.OpenFile(save_file_name.c_str()))
					{
						QMessageBox::critical(this, "Connection", "Falha ao abrir o arquivo de imagem.");
						break;
					}

					this->xacquisition.Grab(cycle_frames);

					this->xevent.WaitTime(cycle_interval);
				}

				QMessageBox::information(this, "Acquisition", "Ciclos completos.");
			}
		}
	}
	else
	{
		this->set_save_file_name(save_file_name);

		if (!this->ximg_handle.OpenFile(save_file_name.c_str()))
		{
			QMessageBox::critical(this, "Connection", "Falha ao abrir o arquivo de imagem.");
			return;
		}

		this->xacquisition.Grab(1);
	}
}

void QtGui::on_stop_grab_btn_clicked()
{
	this->xacquisition.Stop();
}

QString QtGui::get_file_name()
{
	return this->file_name;
}

uint32_t QtGui::get_frame_count()
{
	return this->frame_count;
}

uint32_t QtGui::get_lost_frame_count()
{
	return this->lost_frame_count;
}

bool QtGui::get_is_save()
{
	return this->is_save;
}

std::string QtGui::get_save_file_name()
{
	return this->save_file_name;
}

XImageHandler *QtGui::get_ximage_handler()
{
	return &ximg_handle;
}

XEvent *QtGui::get_xevent()
{
	return &this->xevent;
}

void QtGui::set_frame_count(uint32_t frame_count)
{
	this->frame_count = frame_count;
}

void QtGui::set_lost_frame_count(uint32_t lost_frame_count)
{
	this->lost_frame_count = lost_frame_count;
}

void QtGui::set_is_save(bool is_save)
{
	this->is_save = is_save;
}

void QtGui::set_save_file_name(std::string save_file_name)
{
	this->save_file_name = save_file_name;
}

QtGui::~QtGui()
{
}
