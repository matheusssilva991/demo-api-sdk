#include "CmdSink.h"
#include <iostream>
#include "QtGui.h"

CmdSink::CmdSink(QtGui* parent) : parent_(parent) {}

void CmdSink::OnXError(uint32_t err_id, const char* err_msg_) 
{
	//QMessageBox::critical(parent_, "Erro", err_msg_);
	//std::cout << "OnXError: " << err_id << ", " << err_msg_ << std::endl;
}

void CmdSink::OnXEvent(uint32_t event_id, XHealthPara data)
{
	//cout << "On Event ID " << event_id << " data " << data._dasTemperature1 << endl;
}
