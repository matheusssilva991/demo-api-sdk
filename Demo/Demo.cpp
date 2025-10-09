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
#include <windows.h>

#ifdef _WIN64
#pragma comment(lib, "..\\lib\\x64\\XLibDllKosti.lib")
#else
#pragma comment(lib, "..\\lib\\x86\\XLibDllKosti.lib")
#endif

#include <string>
#include <thread> 
#include <fstream>

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
uint64_t getImageAverage(string file_name);
HANDLE abrirPortaSerial(const char* porta);
void enviarComando(HANDLE hSerial, const std::string& comando);


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
		printf("\nFrame %u ready, width %u, height %d,  lost line %u\n",
			frame_count++, image_->_width, image_->_height, lost_frame_count);
		Beep( 750, 300 );

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

	int input_char;

	string offset_file;
	string gain_file;
	string img_file;

	uint64_t cmd_para = 0;

	//For cycling test
	uint32_t cycle_num = 1;
	uint32_t frame_num = 1;
	uint32_t cycle_interval = 0;
	//uint32_t cycle_it = 0;
	int32_t cycle_it = 0;

	//Arduino Connection
	const char* portaSerial = "\\\\.\\COM4";
	HANDLE hSerial = abrirPortaSerial(portaSerial);
	if (hSerial == INVALID_HANDLE_VALUE) {
		cout << "Falha na conexão com o arduino\n\n";
	}
	else {
		cout << "Conexão estabelecida com o Arduino\n\n";
	}

	displayMenu();

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

			device_count = xsystem.FindDevice();
			// device_count = 1;

			if (device_count <= 0)
			{
				cout << "Nenhum dispositivo encontrado." << endl;
				return 0;
			}

			//Get the first device
			xdevice_ptr = xsystem.GetDevice(0);
			/*xdevice_ptr = new XDevice(&xsystem);
			xdevice_ptr->SetIP("192.168.1.2");
			xdevice_ptr->SetCmdPort(3000);
			xdevice_ptr->SetImgPort(4001);
			xdevice_ptr->SetDeviceType("1412_KOSTI");
			xdevice_ptr->SetSerialNum("1234567890", 10);
			xdevice_ptr->SetMAC((uint8_t*)"123456");
			xdevice_ptr->SetFirmBuildVer(123);
			xdevice_ptr->SetFirmVer(123);*/

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

			clearBuffer();

			if (cycle_num == 1) {

				if (!ximg_handle.OpenFile(save_file_name.c_str()))
				{
					cout << "Falha ao abrir o arquivo de imagem, retornando ao menu principal" << endl;
					break;
				}

				xacquisition.Grab(frame_num);

				frame_complete.Wait();
				std::cout << "Imagem criada: " << save_file_name << endl;
			}
			else {
				uint64_t media_minima = 10000;
				string save_file_name_base = save_file_name.substr(0, save_file_name.find(".dat"));

				for (cycle_it = 0; cycle_it < cycle_num; cycle_it++)
				{

					save_file_name = save_file_name_base + "_cycle_" + to_string(cycle_it) + ".dat";

					if (!ximg_handle.OpenFile(save_file_name.c_str()))
					{
						cout << "Falha ao abrir imagem, repetindo ciclo..." << endl;
						cycle_it--;
						continue;
					}

					cout << "Ciclo " << cycle_it << " completo" << endl;
					frame_complete.WaitTime(cycle_interval);

					if (getImageAverage(save_file_name) < media_minima) {
						cout << "Imagem abaixo da media desejada" << endl;
						continue;
					}
				}

				cout << endl << "Ciclos completos" << endl;
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
			int tmp_input;

			frame_count = 0;
			lost_frame_count = 0;

			cout << "Por favor insira o número de quadros\n";
			cin >> tmp_input;
			frame_num = tmp_input <= 0 ? 1 : tmp_input;

			cout << "Por favor insira o número de ciclos\n";
			cin >> tmp_input;
			cycle_num = tmp_input <= 0 ? 1 : tmp_input;

			if (cycle_num > 1)
			{
				cout << "Por favor insira o intervalo entre os ciclos (ms)\n";
				cin >> tmp_input;
				cycle_interval = tmp_input < 0 ? 0 : tmp_input;
			}

			cout << endl;

			clearBuffer();

			break;

		//case 'A':
		//case 'a':
		//	//Isso era pra tentar barrar uma conexão caso o arduino já estivesse conectado, mas não funcionou
		//	/*if (hSerial != INVALID_HANDLE_VALUE) {
		//		cout << "Arduino já está conectado à porta serial\n\n";
		//		break;
		//	}*/

		//	hSerial = abrirPortaSerial(portaSerial);
		//	if (hSerial == INVALID_HANDLE_VALUE) {
		//		cout << "Falha na conexão com o arduino\n\n";
		//	}
		//	else {
		//		cout << "Conexão estabelecida com o Arduino\n\n";
		//	}
		//	break;

		case 'T':
		case 't': {
			
			uint64_t media = 0;
			uint32_t bad_cycles = 0;
			string local_file_name;


			//PARAMETROS
			int numeroFramesMax = 200;
			int intervaloDeEspera = 3000; //Em milissegundos
			int conscutiveErrors = 0;


			cout << "Esperando 4 segundos (segurança)" << endl;
			Sleep(4000);

			for (int i = 1; i <= numeroFramesMax; i++) {
				frame_count = 0;
				lost_frame_count = 0;
				is_save = 1;

				std::cout << endl << endl << "----------------------" << endl;
				save_file_name = ("20250904/img" + (std::to_string(i)) + ".dat");
				local_file_name = save_file_name;

				if (!ximg_handle.OpenFile(save_file_name.c_str()))
				{
					cout << "Falha ao abrir o arquivo de imagem, retornando ao menu principal" << endl;
					break;
				}

				std::cout << "Grabbing " << save_file_name << std::endl;
				xacquisition.Grab(1);
				//Sleep(3000);
				std::cout << "Shutting " << std::endl;
				//enviarComando(hSerial, "2"); //Dispara a fonte
				frame_complete.Wait();

				//Sleep(5000); //Garantir que o handler fez tudo que precisava. Talvez isso não seja necessário; testar
				ximg_handle.CloseFile(); //Fazer o imghandler liberar a imagem
				//media = getImageAverage(local_file_name);
				//std::cout << "Ciclos erro Total : " << bad_cycles << " Consecutivos: " << conscutiveErrors << endl;
				

				/*if (media < 10000) {
					i--;
					bad_cycles++;
					std::cout << endl << "----------------------" << endl;
					conscutiveErrors++;
					if (conscutiveErrors == 5) {
						std::cout << "Erro no Raios X" << endl;
						break;
					}
					continue;
				}*/
				//else {
				enviarComando(hSerial, "1"); //Gira a amostra
				//conscutiveErrors = 0;
				Sleep(1000);
				enviarComando(hSerial, "1.8");
				std::cout << endl << "----------------------" << endl;
				std::cout << "Esperando " << intervaloDeEspera << " milissegundos...";
				Sleep(intervaloDeEspera);
				//}
			}

			cout << endl << "Tomografia finalizada" << endl;
			//cout << "Ciclos ruins: " << bad_cycles << endl << endl;
			break;
		}
		
		case 'P':
		case 'p': {
			frame_count = 0;
			lost_frame_count = 0;

			//Definindo Ganho como LOW
			if (1 == xcommand.SetPara(XPARA_GAIN_RANGE, 1))
			{
				cout << "Ganho (Low) definido com sucesso\n\n";
			}
			else
			{
				cout << "Falha ao definir o ganho\n\n";
			}

			//Definindo Integration Time como 10000000 us
			if (1 == xcommand.SetPara(XPARA_FRAME_PERIOD, 10000000))
			{
				cout << "Tempo de integração (10000000 us) definido com sucesso" << endl << endl;
			}
			else
			{
				cout << "Falha ao definir o tempo de integração" << endl << endl;
			}
			
			//Definindo 1 frame, 1 numero de ciclo, x ms de cycle interval
			frame_num = 1;
			cycle_num = 1;
			//cycle_interval = 9000;

			cout << "Numero de frame por ciclo (1) definido com sucesso" << endl;
			cout << "Numero de ciclos (1) definido com sucesso" << endl;
			//cout << "Intervalo entre ciclos (9000 ms) definido com sucesso" << endl << endl;

			break;
		}

		case 'm':
		case 'M': {
			string file_name_media;
			cout << "Por favor coloque o nome do arquivo para calcular a media, *.dat \n";
			cin >> file_name_media;
			uint64_t media = getImageAverage(file_name_media);
			break;
		}

		case 'f':
		case 'F': {

			string local_file_name;

			//PARAMETROS
			int numeroFramesMax = 5;
			int intervaloDeEspera = 4000; //Em milissegundos

			cout << "Esperando 3 segundos (segurança)" << endl;
			Sleep(3000);

			for (int i = 1; i <= numeroFramesMax; i++) {
				frame_count = 0;
				is_save = 1;

				std::cout << endl << endl << "----------------------" << endl;
				save_file_name = ("20250904/flat" + (std::to_string(i)) + ".dat");
				local_file_name = save_file_name;

				if (!ximg_handle.OpenFile(save_file_name.c_str()))
				{
					cout << "Falha ao abrir o arquivo de imagem, retornando ao menu principal" << endl;
					break;
				}
				
				std::cout << "Grabbing " << save_file_name << std::endl;
				xacquisition.Grab(1);
				//TESTE Sleep(3000);
				std::cout << "Shooting " << std::endl;
				//enviarComando(hSerial, "2"); //Dispara a fonte
				frame_complete.Wait();

				//Sleep(5000); //Garantir que o handler fez tudo que precisava. Talvez isso não seja necessário; testar
				ximg_handle.CloseFile(); //Fazer o imghandler liberar a imagem
				//media = getImageAverage(local_file_name);
				//std::cout << "Ciclos erro Total : " << bad_cycles << " Consecutivos: " << conscutiveErrors << endl;
				std::cout << "Esperando " << intervaloDeEspera << " milissegundos...";
				Sleep(intervaloDeEspera);

				//if (media < 10000) {
				//	i--;
				//	bad_cycles++;
				//	std::cout << endl << "----------------------" << endl;
				//	conscutiveErrors++;
				//	if (conscutiveErrors == 5) {
				//		std::cout << "Erro no Raios X" << endl;
				//		break;
				//	}
				//	continue;
				//}
				//else {
				//	enviarComando(hSerial, "1"); //Gira a amostra
				//	conscutiveErrors = 0;
				//	Sleep(1000);
				//	enviarComando(hSerial, "7.2");
				//	std::cout << endl << "----------------------" << endl;
				//}
			}

			cout << endl << "Flats obtidos" << endl;
			//cout << "Ciclos ruins: " << bad_cycles << endl << endl;
			break;
		}

		default:
			break;

		}

	} while ((input_char != 'q'));

	xacquisition.Close();

	xcommand.Close();

	xsystem.Close();

	CloseHandle(hSerial);

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
	//cout << "A- Conectar ao arduino\n";
	cout << "T- Tomografia\n";
	cout << "M- Media\n";
	cout << "P- Configurações padrão\n";
	cout << "F- FLATS\n";

	cout << "q- Sair do programa\n\n\n";
}

void clearBuffer() {
	cin.ignore(10000, '\n');
}

HANDLE abrirPortaSerial(const char* porta) {
	HANDLE hSerial = CreateFileA(porta, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (hSerial == INVALID_HANDLE_VALUE) {
		std::cerr << "Erro ao abrir a porta serial!" << std::endl;
		return INVALID_HANDLE_VALUE;
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	if (!GetCommState(hSerial, &dcbSerialParams)) {
		std::cerr << "Erro ao obter configurações da porta serial!" << std::endl;
		CloseHandle(hSerial);
		return INVALID_HANDLE_VALUE;
	}

	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	if (!SetCommState(hSerial, &dcbSerialParams)) {
		std::cerr << "Erro ao configurar a porta serial!" << std::endl;
		CloseHandle(hSerial);
		return INVALID_HANDLE_VALUE;
	}

	return hSerial;
}

void enviarComando(HANDLE hSerial, const std::string& comando) {
	if (hSerial == INVALID_HANDLE_VALUE) return;

	std::string comandoFinal = comando + "\n";
	DWORD bytesEnviados;
	WriteFile(hSerial, comandoFinal.c_str(), comandoFinal.length(), &bytesEnviados, NULL);
	std::cout << "Comando enviado: " << comando << std::endl;
}

uint64_t getImageAverage(string file_name) {
	std::ifstream imagemDat;

	std::cout << "Arquivo de media: " << file_name << endl;

	imagemDat.open(file_name, std::ios::binary);
	if (!imagemDat.good()) {
		return 0;
	}

	uint64_t soma_total = 0;
	uint16_t valor;

	for (int y = 0; y < 1200; y++) {
		for (int x = 0; x < 1400; x++) {
			imagemDat.read(reinterpret_cast<char*>(&valor), sizeof(valor));
			soma_total += valor;
		}
	}

	imagemDat.close();

	uint64_t media = soma_total / (1400 * 1200);

	std::cout << "\nMedia de " << file_name << ": " << media << std::endl;
	if (media < 10000)
		std::cout << "Imagem abaixo da media, repetindo processo..." << std::endl;
	else
		std::cout << "Imagem igual ou acima da media" << std::endl;
	return media;
}


