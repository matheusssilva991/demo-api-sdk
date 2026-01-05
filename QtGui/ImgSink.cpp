/**
 * @file ImgSink.cpp
 * @brief Implementação do manipulador de eventos de aquisição de imagens
 *
 * Esta classe é o núcleo do processamento de imagens em tempo real,
 * recebendo callbacks da API X-LIB para:
 * - Erros durante captura ou transferência
 * - Eventos de status (perda de dados, buffer cheio)
 * - Frames prontos para processamento
 * - Conclusão da sequência de captura
 *
 * @author Matheus Silva
 * @date 2024-2026
 */

#include "ImgSink.h"
#include <iostream>
#include "QtGui.h"

/**
 * @brief Construtor do ImgSink
 * @param parent Ponteiro para a janela principal QtGui
 *
 * Armazena referência ao widget pai para:
 * - Acessar variáveis de estado (is_save, frame_count)
 * - Exibir mensagens de erro
 * - Atualizar interface durante captura
 */
ImgSink::ImgSink(QtGui *parent) : parent_(parent) {}

/**
 * @brief Callback invocado quando ocorre um erro no sistema de imagem
 * @param err_id Código identificador do erro
 * @param err_msg_ Mensagem descritiva do erro
 *
 * Exibe uma caixa de diálogo crítica para notificar o usuário sobre
 * erros graves que podem interromper a captura de imagens.
 *
 * Exemplos de erros:
 * - Falha na comunicação UDP
 * - Timeout na recepção de frames
 * - Erro de alocação de memória
 */
void ImgSink::OnXError(uint32_t err_id, const char *err_msg_)
{
	// Log no console para debug
	// std::cout << "Error ID: " << err_id << " Error Message: " << err_msg_ << std::endl;

	// Exibe erro ao usuário
	QMessageBox::critical(parent_, "Erro " + err_id, err_msg_);
}

/**
 * @brief Callback invocado para eventos de transferência de imagens
 * @param event_id Tipo de evento:
 *        - XEVENT_IMG_PARSE_DATA_LOST: Perda de dados
 *        - XEVENT_IMG_TRANSFER_BUF_FULL: Buffer cheio
 *        - XEVENT_IMG_PARSE_DM_DROP: Descarte de dados
 *        - XEVENT_IMG_PARSE_PAC_LOST: Perda de pacotes UDP
 *        - XEVENT_IMG_PARSE_MONITOR_STATUS_ERR: Erro de monitor
 * @param data Número de frames ou bytes perdidos
 *
 * Processa eventos de status da transferência, especialmente contabilizando
 * frames perdidos que indicam problemas de rede ou sobrecarga do sistema.
 */
void ImgSink::OnXEvent(uint32_t event_id, uint32_t data)
{
	// Contabiliza frames perdidos durante a transmissão
	if (XEVENT_IMG_PARSE_DATA_LOST == event_id)
	{
		uint32_t lost_frame_count = this->parent_->get_lost_frame_count();

		this->parent_->set_lost_frame_count(lost_frame_count + data);
	}
}

/**
 * @brief Callback invocado quando um frame está pronto para processamento
 * @param image_ Ponteiro para estrutura XImage com:
 *        - _width: Largura da imagem em pixels
 *        - _height: Altura da imagem em pixels
 *        - _data: Buffer de dados (16-bit por pixel)
 *        - _line_pitch: Bytes por linha
 *
 * Este é o callback principal de processamento de imagens. Aqui o frame:
 * 1. Tem suas informações exibidas no console
 * 2. É salvo em disco se o modo de salvamento estiver ativo
 * 3. Pode ser enviado para visualização em tempo real (futuro)
 *
 * @note O frame é válido apenas durante esta chamada. Para processar
 *       assincronamente, é necessário copiar os dados.
 */
void ImgSink::OnFrameReady(XImage *image_)
{
	// Obtém estado atual da captura
	uint32_t lost_frame_count = this->parent_->get_lost_frame_count();
	bool is_save = this->parent_->get_is_save();
	XImageHandler *ximg_handle = this->parent_->get_ximage_handler();

	// Log de informações do frame para debug
	std::cout << "Frame ready" << std::endl;
	std::cout << "Width:" << image_->_width << std::endl;
	std::cout << "Height: " << image_->_height << std::endl;
	std::cout << "Lost line: " << lost_frame_count << std::endl;

	// Salva o frame em disco se solicitado
	if (is_save)
	{
		ximg_handle->Write(image_);
	}
}

/**
 * @brief Callback invocado quando a sequência de captura é concluída
 *
 * Este callback sinaliza o fim de uma sequência de captura. Realiza:
 * 1. Geração do arquivo .txt com metadados (header)
 * 2. Fechamento do arquivo .dat de imagem
 * 3. Liberação de recursos
 * 4. Sinalização de conclusão via evento
 *
 * O arquivo .txt contém informações como:
 * - Dimensões da imagem
 * - Parâmetros de captura
 * - Data/hora da aquisição
 * - Configurações do detector
 */
void ImgSink::OnFrameComplete()
{
	// Obtém estado e recursos necessários
	bool is_save = this->parent_->get_is_save();
	XImageHandler *ximg_handle = this->parent_->get_ximage_handler();
	std::string save_file_name = this->parent_->get_save_file_name();
	XEvent *frame_complete = this->parent_->get_xevent();

	std::cout << "Frame complete" << std::endl;

	// Finaliza salvamento se estava ativo
	if (is_save)
	{
		// Gera nome do arquivo .txt substituindo extensão
		std::string txt_name = save_file_name.replace(save_file_name.find(".dat"), 4, ".txt");

		// Salva arquivo de metadados (header)
		ximg_handle->SaveHeaderFile(txt_name.c_str());

		// Fecha arquivo de imagem e libera recursos
		ximg_handle->CloseFile();

		// Desativa flag de salvamento
		is_save = 0;
	}

	// Sinaliza conclusão para threads aguardando
	frame_complete->Set();
}