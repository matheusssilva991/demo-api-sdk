// Demo.cpp
// A C++ example of the usage of X-LIB
#include "stdafx.h"

//include X-LIB headers
#include "xsystem.h"
#include "xdevice.h"
#include "xcommand.h"

#include "xacquisition.h"
#include "xframe_transfer.h"
#include "xgig_factory.h"

#include "ixcmd_sink.h"
#include "iximg_sink.h"

#include "ximage_handler.h"
#include "xcorrection.h"

#ifdef _MSC_VER
#include "xthread_win.h"
#else
#include "xthread_liu.h"
#endif

#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <locale>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>

#ifdef _WIN64
#pragma comment(lib, "..\\lib\\x64\\XLibDllKosti.lib")
#else
#pragma comment(lib, "..\\lib\\x86\\XLibDllKosti.lib")
#endif

using namespace std;

//Global variables

XImageHandler ximg_handle; // XImageHandler object
XEvent frame_complete; // XEvent object

uint32_t frame_count = 0; // Number of frames grabed
uint32_t lost_frame_count = 0; // Number of lost frames

bool is_save = 0; // Flag for saving frames
string save_file_name; // Name of the saved file

//The allocated buffer size for the grabbed frames
#if defined(_WIN64)
uint32_t frame_buffer_size = 700;
#else
uint32_t frame_buffer_size = 400;
#endif

//Function prototypes

void displayMenu();
void clearBuffer();
string getCurrentDateStr();

//Uma classe para manipular eventos de comando do dispositivo
class CmdSink :public IXCmdSink
{
public:
	// Manipulação de erro
	// Parâmetros err_id: ID do error, err_msg_: Mensagem de erro
	void OnXError(uint32_t err_id, const char* err_msg_) override
	{
		cout << "OnXError: " << err_id << ", " << err_msg_ << endl;
	}
	// Manipulação de eventos
	// Parâmetros event_id: ID do evento, data: Dados do evento
	// Eventos: _cisTemperature; _dasTemperature1; _dasTemperature2; _dasTemperature3; _dasHumidity;
	void OnXEvent(uint32_t event_id, XHealthPara data) override
	{
		//cout << "On Event ID " << event_id << " data " << data._dasTemperature1 << endl;
	}
};

//Uma classe para manipular eventos de imagem
class ImgSink : public IXImgSink
{
	// Manipulação de erro
	// Parâmetros err_id: ID do error, err_msg_: Mensagem de erro
	void OnXError(uint32_t err_id, const char* err_msg_) override
	{
		printf("OnXERROR: %u, %s\n", err_id, err_msg_);
	}

	// Manipulação de eventos
	// Parâmetros event_id: ID do evento, data: Dados do evento
	// Eventos: XEVENT_IMG_PARSE_DATA_LOST, XEVENT_IMG_TRANSFER_BUF_FULL, XEVENT_IMG_PARSE_DM_DROP,
	//          XEVENT_IMG_PARSE_PAC_LOST, XEVENT_IMG_PARSE_MONITOR_STATUS_ERR
	void OnXEvent(uint32_t event_id, uint32_t data) override
	{
		if (XEVENT_IMG_PARSE_DATA_LOST == event_id)
		{
			lost_frame_count += data;
		}
	}

	// Manipulação de quadros prontos
	// Parâmetros image_: Ponteiro para o quadro
	void OnFrameReady(XImage* image_) override
	{
		printf("Frame %u ready, width %u, height %d,  lost line %u\n",
			frame_count++, image_->_width, image_->_height, lost_frame_count);

		if (is_save)
		{
			ximg_handle.Write(image_);
		}
	}

	// Manipulação de quadros completos
	void OnFrameComplete() override
	{
		printf("Grab complete.\n");

		if (is_save)
		{
			string txt_name = save_file_name.replace(save_file_name.find(".dat"), 4, ".txt");

			ximg_handle.SaveHeaderFile(txt_name.c_str());
			ximg_handle.CloseFile();

			is_save = 0;
		}

		frame_complete.Set();
	}

};

CmdSink cmd_sink;
ImgSink img_sink;

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "pt_BR.UTF-8");

	char host_ip[20];

	if (1 == argc)
	{
		cout << "Por favor, insira o IP do host" << endl;
		cin >> host_ip;
		clearBuffer();
		cout << endl;
	}

	else
	{
		memcpy(host_ip, argv[1], 20);
	}

	XSystem xsystem(host_ip);
	XDevice* xdevice_ptr = NULL;

	int32_t device_count = 0;
	char device_ip[20];

	uint16_t device_cmd_port;
	uint16_t device_img_port;

	xsystem.RegisterEventSink(&cmd_sink);

	XGigFactory xfactory;

	XCommand xcommand(&xfactory);
	xcommand.RegisterEventSink(&cmd_sink);

	XFrameTransfer xtransfer;
	xtransfer.RegisterEventSink(&img_sink);

	XAcquisition xacquisition(&xfactory);

	xacquisition.RegisterEventSink(&img_sink);
	xacquisition.RegisterFrameTransfer(&xtransfer);

	XCorrection xcorrection;

	string send_str;
	string recv_str;

	displayMenu();

	int input_char;

	string offset_file;
	string gain_file;
	string img_file;

	uint64_t cmd_para = 0;

	//For cycling test
	uint32_t cycle_num = 1;
	uint32_t cycle_frames = 1;
	uint32_t cycle_frames_interval = 0;
	uint32_t cycle_interval = 0;
	uint32_t cycle_it = 0;

	do
	{
		cout << "Por favor, escolha uma opção: ";
		input_char = getchar();
		clearBuffer();
		cout << endl;

		switch (input_char)
		{
		case '1':

			if (!xsystem.Open())
			{
				cerr << "Falha ao conectar ao host." << endl;
    			return 0;  // Termina a execução se não conseguir conectar
			}

			// device_count = xsystem.FindDevice();
			device_count = 1;

			if (device_count <= 0)
			{
				cout << "Nenhum dispositivo encontrado." << endl;
				return 0;
			}

			//Get the first device
			//xdevice_ptr = xsystem.GetDevice(0);
			xdevice_ptr = new XDevice(&xsystem);
			xdevice_ptr->SetIP("192.168.1.2");
			xdevice_ptr->SetCmdPort(5000);
			xdevice_ptr->SetImgPort(3000);
			xdevice_ptr->SetDeviceType("1412_KOSTI");
			xdevice_ptr->SetSerialNum("1234567890", 10);
			xdevice_ptr->SetMAC((uint8_t*)"123456");
			xdevice_ptr->SetFirmBuildVer(123);
			xdevice_ptr->SetFirmVer(123);

			cout << "Dispositivo encontrado: " << xdevice_ptr->GetIP() << endl;
			cout << "Porta de comando: " << xdevice_ptr->GetCmdPort() << endl;
			cout << "Porta de imagem: " << xdevice_ptr->GetImgPort() << endl;

			break;

		case '2':
			cout << "Por favor, insira o IP do dispositivo" << endl;
			cin >> device_ip;

			cout << "Por favor, insira a porta de comando do dispositivo" << endl;
			cin >> device_cmd_port;

			cout << "Por favor, insira a porta de imagem do dispositivo" << endl;
			cin >> device_img_port;

			if (xdevice_ptr)
			{
				xdevice_ptr->SetIP(device_ip);
				xdevice_ptr->SetCmdPort(device_cmd_port);
				xdevice_ptr->SetImgPort(device_img_port);
			}

			if (1 == xsystem.ConfigureDevice(xdevice_ptr))
			{
				cout << "Dispositivo configurado com sucesso, por favor, encontre o dispositivo novamente" << endl;
			}
			else
			{
				cout << "Falha ao configurar o dispositivo" << endl;
			}

			break;

		case '3':
			if (xcommand.Open(xdevice_ptr))
			{
				cout << "Canal de comando aberto com sucesso" << endl;

				if (xacquisition.Open(xdevice_ptr, &xcommand))
				{
					cout << "Canal de imagem aberto com sucesso" << endl;

				}
				else
					cout << "Falha ao abrir o canal de imagem" << endl;
			}

			else
				cout << "Falha ao abrir o canal de comando" << endl;

			break;

		case '4':
			cout << "Por favor, insira o comando ASCII" << endl;
			cin >> send_str;

			xcommand.SendAscCmd(send_str, recv_str);
			cout << "Resposta: " << recv_str << endl;

			break;

		case '5':
			cout << "Iniciando aquisição" << endl;

			frame_count = 0;
			lost_frame_count = 0;

			xacquisition.Grab(0);

			break;

		case '6':
			cout << "Parando aquisição" << endl;

			xacquisition.Stop();

			break;

		case '7':
			frame_count = 0;
			lost_frame_count = 0;

			is_save = 1;

			cout << "Por favor coloque o nome do arquivo para salvar, *.dat \n";
			cin >> save_file_name;

			save_file_name = "..//images//" + getCurrentDateStr() + "//" + save_file_name;

			if (cycle_num == 1) {

				if (!ximg_handle.OpenFile(save_file_name.c_str()))
				{
					cout << "Falha ao abrir o arquivo de imagem, retornando ao menu principal" << endl;
					break;
				}

				xacquisition.Grab(cycle_frames);

				frame_complete.WaitTime(cycle_frames_interval);

			}
			else {
				string save_file_name_base = save_file_name.substr(0, save_file_name.find(".dat"));

				for (cycle_it = 0; cycle_it < cycle_num; cycle_it++)
				{
					cout << "Ciclo " << cycle_it << " completo" << endl;

					save_file_name = "..//images//" + getCurrentDateStr() + "//" + save_file_name_base + "_cycle_" + to_string(cycle_it) + ".dat";

					if (!ximg_handle.OpenFile(save_file_name.c_str()))
					{
						cout << "Falha ao abrir o arquivo de imagem, retornando ao menu principal" << endl;
						break;
					}

					xacquisition.Grab(cycle_frames);

					frame_complete.WaitTime(cycle_frames_interval);

					XSLEEP(cycle_interval * XSLEEP_UNIT);
				}

				cout << endl << "Ciclos completos" << endl;

				xacquisition.Grab(cycle_frames);
			}

			break;

		case '8':

			if (!xcorrection.Open(&xcommand))
			{
				cout << "Falha ao abrir o canal de comando, retornando ao menu principal" << endl;
				break;
			}

			cout << "Por favor, insira o nome do arquivo de defeitos, *.dat \n";

			if (!xcorrection.LoadDefectsFile())
			{
				cout << "Falha ao carregar o mapa de defeitos, retornando ao menu principal" << endl;
				xcorrection.Close();
				break;
			}

			cout << "Por favor, insira o nome do arquivo de offset, *.dat \n";

			cin >> offset_file;

			if (!ximg_handle.ReadFile(offset_file.c_str()))
			{
				cout << "Falha ao abrir o arquivo de offset, retornando ao menu principal" << endl;

				xcorrection.Close();
				break;
			}

			if (!xcorrection.CalculateOffset(ximg_handle._images_))
			{
				cout << "Falha ao calcular o offset, retornando ao menu principal" << endl;

				xcorrection.Close();
				break;
			}

			cout << "Offset calculado com sucesso, por favor insira o nome do arquivo de ganho, *.dat \n";

			cin >> gain_file;

			if (!ximg_handle.ReadFile(gain_file.c_str()))
			{
				cout << "Falha ao abrir o arquivo de ganho, retornando ao menu principal" << endl;

				xcorrection.Close();
				break;
			}

			if (!xcorrection.CalculateGain(ximg_handle._images_, 0))
			{
				cout << "Falha ao calcular o ganho, retornando ao menu principal" << endl;

				xcorrection.Close();
				break;
			}

			cout << "Ganho calculado com sucesso, por favor insira o nome do arquivo de imagem, *.dat \n";

			cin >> img_file;

			if (!ximg_handle.ReadFile(img_file.c_str()))
			{
				cout << "Falha ao abrir o arquivo de imagem, retornando ao menu principal" << endl;

				xcorrection.Close();
				break;
			}

			if (!xcorrection.DoCorrect(ximg_handle._images_))
			{
				cout << "Falha ao corrigir a imagem, retornando ao menu principal" << endl;

				xcorrection.Close();
				break;
			}

			cout << "Por favor insira o nome do arquivo para salvar a imagem corrigida, *.dat \n";

			cin >> img_file;

			xcorrection.SaveCorrectedImageFile(img_file.c_str());

			xcorrection.Close();

			clearBuffer();

			break;

		case '9':
			cout << "Fechando o dispositivo" << endl;

			xacquisition.Close();

			xcommand.Close();

			break;

		case 'B':
		case 'b':
			cout << "Por favor insira o modo de binning: \n 0: Original \n 1: 2x2\n";
			cin >> cmd_para;

			if (1 == xcommand.SetPara(XPARA_BINNING_MODE, cmd_para))
			{
				cout << "Modo de binning definido com sucesso\n\n";
			}
			else
			{
				cout << "Falha ao definir o modo de binning\n\n";
			}

			clearBuffer();

			break;

		case 'I':
		case 'i':
			cout << "Por favor insira o tempo de integração (us)" << endl;
			cin >> cmd_para;

			if (1 == xcommand.SetPara(XPARA_FRAME_PERIOD, cmd_para))
			{
				cout << "Tempo de integração definido com sucesso" << endl << endl;
			}
			else
			{
				cout << "Falha ao definir o tempo de integração" << endl << endl;
			}

			clearBuffer();

			break;

		case 'G':
		case 'g':
			cout << "Por favor insira o modo de ganho: \n 1: Baixo ganho\n 256: Alto ganho\n";
			cin >> cmd_para;

			if (1 == xcommand.SetPara(XPARA_GAIN_RANGE, cmd_para))
			{
				cout << "Ganho definido com sucesso\n\n";
			}
			else
			{
				cout << "Falha ao definir o ganho\n\n";
			}

			clearBuffer();

			break;

		case 'C':
		case 'c':
			frame_count = 0;
			lost_frame_count = 0;

			cout << "Por favor insira o número de quadros do ciclo\n";
			cin >> cycle_frames;
			cycle_frames = cycle_frames <= 0 ? 1 : cycle_frames;

			cout << "Por favor insira o intervalo de quadros do ciclo (s)\n";
			cin >> cycle_frames_interval;
			cycle_frames_interval = cycle_frames_interval < 0 ? 1 : cycle_frames_interval;

			cout << "Por favor insira o número de ciclos\n";
			cin >> cycle_num;
			cycle_num = cycle_num <= 0 ? 1 : cycle_num;

			cout << "Por favor insira o intervalo de ciclo (s)\n";
			cin >> cycle_interval;
			cycle_interval = cycle_interval < 0 ? 1 : cycle_interval;

			cout << endl;

			clearBuffer();

			break;

		default:
			break;

		}

	} while ((input_char != 'q'));

	xacquisition.Close();

	xcommand.Close();

	xsystem.Close();

	return 1;
}

void displayMenu()
{
	cout << "Bem-vindo ao programa de demonstração do X-LIB\n\n";
	cout << "Por favor, escolha uma opção a partir das seguintes: \n\n";
	cout << "1- Encontrar dispositivo\n";
	cout << "2- Configurar dispositivo\n";
	cout << "3- Abrir dispositivo\n";
	cout << "4- Enviar comando ASII\n";
	cout << "5- Capturar\n";
	cout << "6- Parar\n";
	cout << "7- Capturar e salvar\n";
	cout << "8- Arquivo corrigido\n";
	cout << "9- Fechar dispositivo\n";
	cout << "B- Modo de binning\n";
	cout << "C- Parâmetros de ciclo\n";
	cout << "G- Ganho\n";
	cout << "I- Tempo de integração\n";
	cout << "q- Sair do programa\n\n\n";
}

void clearBuffer() {
	cin.ignore(10000, '\n');
}

string getCurrentDateStr() {
	auto now = chrono::system_clock::now();
	auto in_time_t = chrono::system_clock::to_time_t(now);

	stringstream date_stream;
	date_stream << put_time(localtime(&in_time_t), "%d%m%Y");

	return date_stream.str();
}