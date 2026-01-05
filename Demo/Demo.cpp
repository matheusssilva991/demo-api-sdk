/**
 * @file Demo.cpp
 * @brief Aplicação de demonstração e teste da API X-LIB para o detector X-Panel 1412i
 *
 * Esta aplicação fornece uma interface de linha de comando para:
 * - Descobrir e conectar ao detector
 * - Configurar parâmetros de operação (ganho, binning, tempo de integração)
 * - Capturar imagens individuais ou sequências (tomografia)
 * - Salvar imagens em formato .dat e .txt
 * - Realizar operações automatizadas de captura
 *
 * @author Varex Imaging / Adaptado por Matheus Silva
 * @date 2024-2026
 */

#include "stdafx.h"

// Cabeçalhos da API X-LIB - Sistema e comunicação
#include "xsystem.h"  // Gerenciamento do sistema e descoberta de dispositivos
#include "xdevice.h"  // Representação e controle do dispositivo detector
#include "xcommand.h" // Envio de comandos ao detector

// Cabeçalhos da API X-LIB - Aquisição de imagens
#include "xacquisition.h"	 // Controle do processo de aquisição
#include "xframe_transfer.h" // Transferência de frames do detector
#include "xgig_factory.h"	 // Factory para criação de objetos de comunicação

// Interfaces para manipulação de eventos
#include "ixcmd_sink.h" // Interface para eventos de comando
#include "iximg_sink.h" // Interface para eventos de imagem

// Manipulação e correção de imagens
#include "ximage_handler.h" // Salvamento de imagens em disco
#include "xcorrection.h"	// Correções de imagem (offset, ganho)

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

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================

/** @brief Manipulador para salvar imagens capturadas em disco */
XImageHandler ximg_handle;

/** @brief Evento de sincronização para sinalizar conclusão da captura */
XEvent frame_complete;

/** @brief Contador de frames capturados com sucesso na sessão atual */
uint32_t frame_count = 0;

/** @brief Contador de frames perdidos durante a transferência */
uint32_t lost_frame_count = 0;

/** @brief Flag que indica se os frames devem ser salvos em disco */
bool is_save = 0;

/** @brief Nome do arquivo onde as imagens serão salvas (formato .dat) */
string save_file_name;

/**
 * @brief Tamanho do buffer alocado para armazenar frames capturados
 *
 * Valores diferentes para 32-bit e 64-bit devido a limitações de memória.
 * Arquitetura 64-bit pode alocar buffer maior para melhor performance.
 */
#if defined(_WIN64)
uint32_t frame_buffer_size = 700; // Buffer para x64: 700 frames
#else
uint32_t frame_buffer_size = 400; // Buffer para x86: 400 frames
#endif

// ============================================================================
// PROTÓTIPOS DE FUNÇÕES
// ============================================================================

void displayMenu();
void clearBuffer();
uint64_t getImageAverage(string file_name);
HANDLE abrirPortaSerial(const char *porta);
void enviarComando(HANDLE hSerial, const std::string &comando);

// ============================================================================
// CLASSES DE MANIPULAÇÃO DE EVENTOS
// ============================================================================

/**
 * @class CmdSink
 * @brief Classe para manipular eventos de comando do dispositivo detector
 *
 * Implementa a interface IXCmdSink para receber callbacks de:
 * - Erros durante a comunicação ou execução de comandos
 * - Eventos de saúde do detector (temperatura, umidade, etc.)
 *
 * Esta classe é registrada no XSystem e XCommand para receber notificações
 * assíncronas do dispositivo.
 */
class CmdSink : public IXCmdSink
{
public:
	/**
	 * @brief Callback invocado quando ocorre um erro no sistema de comando
	 * @param err_id Código numérico identificador do erro
	 * @param err_msg_ Mensagem descritiva do erro
	 */
	void OnXError(uint32_t err_id, const char *err_msg_) override
	{
		cout << "OnXError: " << err_id << ", " << err_msg_ << endl;
	}

	/**
	 * @brief Callback invocado quando o detector reporta dados de saúde
	 * @param event_id Identificador do tipo de evento
	 * @param data Estrutura contendo parâmetros de saúde:
	 *             - _cisTemperature: Temperatura do CIS (sensor)
	 *             - _dasTemperature1/2/3: Temperaturas do DAS (sistema de aquisição)
	 *             - _dasHumidity: Umidade relativa
	 *
	 * @note Comentado para reduzir verbosidade do console
	 */
	void OnXEvent(uint32_t event_id, XHealthPara data) override
	{
		// Descomente para monitorar saúde do detector:
		// cout << "Evento ID: " << event_id << " Temp1: " << data._dasTemperature1 << endl;
	}
};

/**
 * @class ImgSink
 * @brief Classe para manipular eventos do processo de aquisição de imagens
 *
 * Implementa a interface IXImgSink para receber callbacks de:
 * - Erros durante a captura ou transferência de imagens
 * - Eventos de status da captura (perda de dados, buffer cheio, etc.)
 * - Frames prontos para processamento
 * - Conclusão da sequência de captura
 *
 * Esta classe é o núcleo do processamento de imagens em tempo real,
 * gerenciando o fluxo desde a captura até o salvamento em disco.
 */
class ImgSink : public IXImgSink
{
	/**
	 * @brief Callback invocado quando ocorre um erro no sistema de imagem
	 * @param err_id Código numérico identificador do erro
	 * @param err_msg_ Mensagem descritiva do erro
	 */
	void OnXError(uint32_t err_id, const char *err_msg_) override
	{
		printf("OnXERROR: %u, %s\n", err_id, err_msg_);
	}

	/**
	 * @brief Callback invocado para eventos relacionados à transferência de imagens
	 * @param event_id Identificador do tipo de evento:
	 *        - XEVENT_IMG_PARSE_DATA_LOST: Perda de dados durante parsing
	 *        - XEVENT_IMG_TRANSFER_BUF_FULL: Buffer de transferência cheio
	 *        - XEVENT_IMG_PARSE_DM_DROP: Descarte de dados no parser
	 *        - XEVENT_IMG_PARSE_PAC_LOST: Perda de pacotes UDP
	 *        - XEVENT_IMG_PARSE_MONITOR_STATUS_ERR: Erro de status do monitor
	 * @param data Dados específicos do evento (ex: número de frames perdidos)
	 */
	void OnXEvent(uint32_t event_id, uint32_t data) override
	{
		// Contabiliza frames perdidos durante a transmissão
		if (XEVENT_IMG_PARSE_DATA_LOST == event_id)
		{
			lost_frame_count += data;
		}
	}

	/**
	 * @brief Callback invocado quando um frame está pronto para processamento
	 * @param image_ Ponteiro para a estrutura XImage contendo:
	 *        - _width: Largura da imagem em pixels
	 *        - _height: Altura da imagem em pixels
	 *        - _data: Buffer com os dados de pixels (16-bit por pixel)
	 *        - _line_pitch: Bytes por linha da imagem
	 *
	 * Este é o callback principal onde o frame capturado pode ser:
	 * - Processado em tempo real
	 * - Salvo em disco
	 * - Enviado para visualização
	 */
	void OnFrameReady(XImage *image_) override
	{
		// Exibe informações do frame capturado
		printf("\nFrame %u ready, width %u, height %d,  lost line %u\n",
			   frame_count++, image_->_width, image_->_height, lost_frame_count);

		// Feedback sonoro para indicar captura (750 Hz por 300ms)
		Beep(750, 300);

		// Salva o frame em disco se o modo de salvamento estiver ativo
		if (is_save)
		{
			ximg_handle.Write(image_);
		}
	}

	/**
	 * @brief Callback invocado quando toda a sequência de captura é concluída
	 *
	 * Este callback é chamado após todos os frames solicitados serem capturados.
	 * É o momento ideal para:
	 * - Finalizar o salvamento em disco
	 * - Gerar arquivos de metadados (.txt)
	 * - Liberar recursos
	 * - Sinalizar threads aguardando a conclusão
	 */
	void OnFrameComplete() override
	{
		printf("Grab complete.\n");

		if (is_save)
		{
			// Gera arquivo .txt com metadados da imagem (header)
			string txt_name = save_file_name.replace(save_file_name.find(".dat"), 4, ".txt");

			// Salva o arquivo de cabeçalho com informações da captura
			ximg_handle.SaveHeaderFile(txt_name.c_str());

			// Fecha o arquivo de imagem e libera recursos
			ximg_handle.CloseFile();

			// Desativa o modo de salvamento
			is_save = 0;
		}

		// Sinaliza threads aguardando que a captura foi concluída
		frame_complete.Set();
	}
};

// Instâncias globais dos manipuladores de eventos
CmdSink cmd_sink; ///< Manipulador de eventos de comando
ImgSink img_sink; ///< Manipulador de eventos de imagem

/**
 * @brief Função principal do programa
 * @param argc Número de argumentos de linha de comando
 * @param argv Array de argumentos (argv[1] = IP do host, se fornecido)
 * @return 1 se bem-sucedido, 0 em caso de erro
 *
 * Inicializa o sistema, conecta ao detector e fornece um menu interativo
 * para realizar diversas operações como captura de imagens, configuração
 * de parâmetros e operações automatizadas de tomografia.
 */
int main(int argc, char **argv)
{
	// Configura localização para suportar caracteres UTF-8 (português)
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
	XDevice *xdevice_ptr = NULL;

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

	// For cycling test
	uint32_t cycle_num = 1;
	uint32_t frame_num = 1;
	uint32_t cycle_interval = 0;
	// uint32_t cycle_it = 0;
	int32_t cycle_it = 0;

	// Arduino Connection
	const char *portaSerial = "\\\\.\\COM4";
	HANDLE hSerial = abrirPortaSerial(portaSerial);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		cout << "Falha na conexão com o arduino\n\n";
	}
	else
	{
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
				return 0; // Termina a execução se não conseguir conectar
			}

			device_count = xsystem.FindDevice();
			// device_count = 1;

			if (device_count <= 0)
			{
				cout << "Nenhum dispositivo encontrado." << endl;
				return 0;
			}

			// Get the first device
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

			if (cycle_num == 1)
			{

				if (!ximg_handle.OpenFile(save_file_name.c_str()))
				{
					cout << "Falha ao abrir o arquivo de imagem, retornando ao menu principal" << endl;
					break;
				}

				xacquisition.Grab(frame_num);

				frame_complete.Wait();
				std::cout << "Imagem criada: " << save_file_name << endl;
			}
			else
			{
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

					if (getImageAverage(save_file_name) < media_minima)
					{
						cout << "Imagem abaixo da media desejada" << endl;
						continue;
					}
				}

				cout << endl
					 << "Ciclos completos" << endl;
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
				cout << "Tempo de integração definido com sucesso" << endl
					 << endl;
			}
			else
			{
				cout << "Falha ao definir o tempo de integração" << endl
					 << endl;
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

			// case 'A':
			// case 'a':
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
		case 't':
		{

			uint64_t media = 0;
			uint32_t bad_cycles = 0;
			string local_file_name;

			// PARAMETROS
			int numeroFramesMax = 200;
			int intervaloDeEspera = 3000; // Em milissegundos
			int conscutiveErrors = 0;

			cout << "Esperando 4 segundos (segurança)" << endl;
			Sleep(4000);

			for (int i = 1; i <= numeroFramesMax; i++)
			{
				frame_count = 0;
				lost_frame_count = 0;
				is_save = 1;

				std::cout << endl
						  << endl
						  << "----------------------" << endl;
				save_file_name = ("20250904/img" + (std::to_string(i)) + ".dat");
				local_file_name = save_file_name;

				if (!ximg_handle.OpenFile(save_file_name.c_str()))
				{
					cout << "Falha ao abrir o arquivo de imagem, retornando ao menu principal" << endl;
					break;
				}

				std::cout << "Grabbing " << save_file_name << std::endl;
				xacquisition.Grab(1);
				// Sleep(3000);
				std::cout << "Shutting " << std::endl;
				// enviarComando(hSerial, "2"); //Dispara a fonte
				frame_complete.Wait();

				// Sleep(5000); //Garantir que o handler fez tudo que precisava. Talvez isso não seja necessário; testar
				ximg_handle.CloseFile(); // Fazer o imghandler liberar a imagem
				// media = getImageAverage(local_file_name);
				// std::cout << "Ciclos erro Total : " << bad_cycles << " Consecutivos: " << conscutiveErrors << endl;

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
				// else {
				enviarComando(hSerial, "1"); // Gira a amostra
				// conscutiveErrors = 0;
				Sleep(1000);
				enviarComando(hSerial, "1.8");
				std::cout << endl
						  << "----------------------" << endl;
				std::cout << "Esperando " << intervaloDeEspera << " milissegundos...";
				Sleep(intervaloDeEspera);
				//}
			}

			cout << endl
				 << "Tomografia finalizada" << endl;
			// cout << "Ciclos ruins: " << bad_cycles << endl << endl;
			break;
		}

		case 'P':
		case 'p':
		{
			frame_count = 0;
			lost_frame_count = 0;

			// Definindo Ganho como LOW
			if (1 == xcommand.SetPara(XPARA_GAIN_RANGE, 1))
			{
				cout << "Ganho (Low) definido com sucesso\n\n";
			}
			else
			{
				cout << "Falha ao definir o ganho\n\n";
			}

			// Definindo Integration Time como 10000000 us
			if (1 == xcommand.SetPara(XPARA_FRAME_PERIOD, 10000000))
			{
				cout << "Tempo de integração (10000000 us) definido com sucesso" << endl
					 << endl;
			}
			else
			{
				cout << "Falha ao definir o tempo de integração" << endl
					 << endl;
			}

			// Definindo 1 frame, 1 numero de ciclo, x ms de cycle interval
			frame_num = 1;
			cycle_num = 1;
			// cycle_interval = 9000;

			cout << "Numero de frame por ciclo (1) definido com sucesso" << endl;
			cout << "Numero de ciclos (1) definido com sucesso" << endl;
			// cout << "Intervalo entre ciclos (9000 ms) definido com sucesso" << endl << endl;

			break;
		}

		case 'm':
		case 'M':
		{
			string file_name_media;
			cout << "Por favor coloque o nome do arquivo para calcular a media, *.dat \n";
			cin >> file_name_media;
			uint64_t media = getImageAverage(file_name_media);
			break;
		}

		case 'f':
		case 'F':
		{

			string local_file_name;

			// PARAMETROS
			int numeroFramesMax = 5;
			int intervaloDeEspera = 4000; // Em milissegundos

			cout << "Esperando 3 segundos (segurança)" << endl;
			Sleep(3000);

			for (int i = 1; i <= numeroFramesMax; i++)
			{
				frame_count = 0;
				is_save = 1;

				std::cout << endl
						  << endl
						  << "----------------------" << endl;
				save_file_name = ("20250904/flat" + (std::to_string(i)) + ".dat");
				local_file_name = save_file_name;

				if (!ximg_handle.OpenFile(save_file_name.c_str()))
				{
					cout << "Falha ao abrir o arquivo de imagem, retornando ao menu principal" << endl;
					break;
				}

				std::cout << "Grabbing " << save_file_name << std::endl;
				xacquisition.Grab(1);
				// TESTE Sleep(3000);
				std::cout << "Shooting " << std::endl;
				// enviarComando(hSerial, "2"); //Dispara a fonte
				frame_complete.Wait();

				// Sleep(5000); //Garantir que o handler fez tudo que precisava. Talvez isso não seja necessário; testar
				ximg_handle.CloseFile(); // Fazer o imghandler liberar a imagem
				// media = getImageAverage(local_file_name);
				// std::cout << "Ciclos erro Total : " << bad_cycles << " Consecutivos: " << conscutiveErrors << endl;
				std::cout << "Esperando " << intervaloDeEspera << " milissegundos...";
				Sleep(intervaloDeEspera);

				// if (media < 10000) {
				//	i--;
				//	bad_cycles++;
				//	std::cout << endl << "----------------------" << endl;
				//	conscutiveErrors++;
				//	if (conscutiveErrors == 5) {
				//		std::cout << "Erro no Raios X" << endl;
				//		break;
				//	}
				//	continue;
				// }
				// else {
				//	enviarComando(hSerial, "1"); //Gira a amostra
				//	conscutiveErrors = 0;
				//	Sleep(1000);
				//	enviarComando(hSerial, "7.2");
				//	std::cout << endl << "----------------------" << endl;
				// }
			}

			cout << endl
				 << "Flats obtidos" << endl;
			// cout << "Ciclos ruins: " << bad_cycles << endl << endl;
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
	// cout << "A- Conectar ao arduino\n";
	cout << "T- Tomografia\n";
	cout << "M- Media\n";
	cout << "P- Configurações padrão\n";
	cout << "F- FLATS\n";

	cout << "q- Sair do programa\n\n\n";
}

void clearBuffer()
{
	cin.ignore(10000, '\n');
}

HANDLE abrirPortaSerial(const char *porta)
{
	HANDLE hSerial = CreateFileA(porta, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Erro ao abrir a porta serial!" << std::endl;
		return INVALID_HANDLE_VALUE;
	}

	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	if (!GetCommState(hSerial, &dcbSerialParams))
	{
		std::cerr << "Erro ao obter configurações da porta serial!" << std::endl;
		CloseHandle(hSerial);
		return INVALID_HANDLE_VALUE;
	}

	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	if (!SetCommState(hSerial, &dcbSerialParams))
	{
		std::cerr << "Erro ao configurar a porta serial!" << std::endl;
		CloseHandle(hSerial);
		return INVALID_HANDLE_VALUE;
	}

	return hSerial;
}

void enviarComando(HANDLE hSerial, const std::string &comando)
{
	if (hSerial == INVALID_HANDLE_VALUE)
		return;

	std::string comandoFinal = comando + "\n";
	DWORD bytesEnviados;
	WriteFile(hSerial, comandoFinal.c_str(), comandoFinal.length(), &bytesEnviados, NULL);
	std::cout << "Comando enviado: " << comando << std::endl;
}

uint64_t getImageAverage(string file_name)
{
	std::ifstream imagemDat;

	std::cout << "Arquivo de media: " << file_name << endl;

	imagemDat.open(file_name, std::ios::binary);
	if (!imagemDat.good())
	{
		return 0;
	}

	uint64_t soma_total = 0;
	uint16_t valor;

	for (int y = 0; y < 1200; y++)
	{
		for (int x = 0; x < 1400; x++)
		{
			imagemDat.read(reinterpret_cast<char *>(&valor), sizeof(valor));
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
