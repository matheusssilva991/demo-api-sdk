#pragma once
#include "ixcmd_sink.h"

//Uma classe para manipular eventos de comando do dispositivo
class CmdSink : public IXCmdSink
{
public:
	// Manipula��o de erro
	// Par�metros err_id: ID do error, err_msg_: Mensagem de erro
	void OnXError(uint32_t err_id, const char* err_msg_) override;

	// Manipula��o de eventos
	// Par�metros event_id: ID do evento, data: Dados do evento
	// Eventos: _cisTemperature; _dasTemperature1; _dasTemperature2; _dasTemperature3; _dasHumidity;
	void OnXEvent(uint32_t event_id, XHealthPara data) override;
};

