#pragma once
#include "iximg_sink.h"
#include <QObject>
#include <QMessageBox>

#include <iostream>


class QtGui;

//Uma classe para manipular eventos de imagem
class ImgSink : public IXImgSink
{

public:
	explicit ImgSink(QtGui* parent);

	// Manipula��o de erro
	// Par�metros err_id: ID do error, err_msg_: Mensagem de erro
	void OnXError(uint32_t err_id, const char* err_msg_) override;

	// Manipula��o de eventos
	// Par�metros event_id: ID do evento, data: Dados do evento
	// Eventos: XEVENT_IMG_PARSE_DATA_LOST, XEVENT_IMG_TRANSFER_BUF_FULL, XEVENT_IMG_PARSE_DM_DROP,
	//          XEVENT_IMG_PARSE_PAC_LOST, XEVENT_IMG_PARSE_MONITOR_STATUS_ERR
	void OnXEvent(uint32_t event_id, uint32_t data) override;

	// Manipula��o de quadros prontos
	// Par�metros image_: Ponteiro para o quadro
	void OnFrameReady(XImage* image_) override;

	// Manipula��o de quadros completos
	void OnFrameComplete() override;

private:
	QtGui* parent_;
};