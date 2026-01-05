/**
 * @file CmdSink.cpp
 * @brief Implementação do manipulador de eventos de comando do detector
 *
 * Esta classe recebe e processa callbacks da API X-LIB relacionados a:
 * - Erros de comunicação ou execução de comandos
 * - Eventos de saúde do detector (temperatura, umidade)
 *
 * @author Matheus Silva
 * @date 2024-2026
 */

#include "CmdSink.h"
#include <iostream>
#include "QtGui.h"

/**
 * @brief Construtor do CmdSink
 * @param parent Ponteiro para a janela principal QtGui
 *
 * Armazena referência ao widget pai para exibir mensagens de erro
 * ou atualizar a interface com dados de saúde do detector.
 */
CmdSink::CmdSink(QtGui *parent) : parent_(parent) {}

/**
 * @brief Callback invocado quando ocorre um erro no sistema de comando
 * @param err_id Código identificador do erro
 * @param err_msg_ Mensagem descritiva do erro
 *
 * Este método pode ser usado para:
 * - Exibir caixas de diálogo com mensagens de erro
 * - Registrar erros em arquivo de log
 * - Atualizar barra de status da interface
 *
 * @note Atualmente comentado para evitar pop-ups excessivos
 */
void CmdSink::OnXError(uint32_t err_id, const char *err_msg_)
{
	// Descomentar para exibir erros em caixa de diálogo:
	// QMessageBox::critical(parent_, "Erro", err_msg_);

	// Descomentar para log no console:
	// std::cout << "OnXError: " << err_id << ", " << err_msg_ << std::endl;
}

/**
 * @brief Callback invocado quando o detector reporta dados de saúde
 * @param event_id Identificador do tipo de evento de saúde
 * @param data Estrutura XHealthPara contendo:
 *        - _cisTemperature: Temperatura do sensor CIS
 *        - _dasTemperature1/2/3: Temperaturas do sistema DAS
 *        - _dasHumidity: Umidade relativa interna
 *
 * Este método pode ser usado para:
 * - Atualizar widgets de monitoramento na interface
 * - Disparar alarmes se temperatura exceder limites
 * - Registrar histórico de saúde do detector
 *
 * @note Atualmente comentado, mas pode ser ativado para monitoramento
 */
void CmdSink::OnXEvent(uint32_t event_id, XHealthPara data)
{
	// Exemplo de uso - exibir temperatura no console:
	// cout << "Evento de Saúde ID: " << event_id
	//      << " Temperatura DAS 1: " << data._dasTemperature1 << endl;
}
