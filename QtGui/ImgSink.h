#pragma once
#include "iximg_sink.h"
#include "ximage_handler.h"
#include "xthread_win.h"

#include <iostream>

//Uma classe para manipular eventos de imagem
class ImgSink : public IXImgSink
{

public:
	ImgSink() : lost_frame_count(0), frame_count(0), is_save(false) {}

	// Manipulação de erro
	// Parâmetros err_id: ID do error, err_msg_: Mensagem de erro
	void OnXError(uint32_t err_id, const char* err_msg_) override;

	// Manipulação de eventos
	// Parâmetros event_id: ID do evento, data: Dados do evento
	// Eventos: XEVENT_IMG_PARSE_DATA_LOST, XEVENT_IMG_TRANSFER_BUF_FULL, XEVENT_IMG_PARSE_DM_DROP,
	//          XEVENT_IMG_PARSE_PAC_LOST, XEVENT_IMG_PARSE_MONITOR_STATUS_ERR
	void OnXEvent(uint32_t event_id, uint32_t data) override;

	// Manipulação de quadros prontos
	// Parâmetros image_: Ponteiro para o quadro
	void OnFrameReady(XImage* image_) override;

	// Manipulação de quadros completos
	void OnFrameComplete() override;

	// Getters
	uint32_t GetLostFrameCount() const;
	uint32_t GetFrameCount() const;
	std::string GetSaveFileName() const;
	bool GetIsSave() const;

	// Setters
	void SetSaveFileName(const std::string& save_file_name_);
	void SetIsSave(bool is_save_);
    
	// Utilitários
	void resetCounters();

private:
	uint32_t lost_frame_count; // Contagem de quadros perdidos
	uint32_t frame_count; // Contagem de quadros
	XImageHandler ximg_handle; // Manipulador de imagem
	bool is_save; // Flag para salvar
	std::string save_file_name; // Nome do arquivo para salvar
	XEvent frame_complete; // Evento de quadro completo
};