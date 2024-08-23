/*
  Copyright (c) 2014, Detection Technology Inc.
  All right reserved.

  This provide the UDP based point to point communication method.

  Author: Zhang Xu, 2014-4-14
 */

#ifndef XUDPCMD_ENGINE_H
#define XUDPCMD_ENGINE_H
#include <chrono>

#include "ixcmd_engine.h"
#include "xcrc.h"
#include "ixcmd_sink.h"
#include "xudpex_win.h"

/*
  XUDPCmdEngine class is UDP based to communicate with detector. It provides
  the CRC checking, command sending and command receiving functions.
 */

class XUDPCmdEngine : public IXCmdEngine
{
public:
     explicit XUDPCmdEngine(uint32_t timeout,
			    IXCmdSink* cmd_sink_ = NULL);
     XUDPCmdEngine();
     ~XUDPCmdEngine();

     bool Open(XDevice* device_);
     void Close();

     int32_t SendCommand(uint8_t cmd_code, uint8_t operation, uint8_t dm_id,
	              uint16_t data_size, uint8_t* send_data_,
		      uint8_t* recv_data_, uint8_t& err_code);
     void OnError(uint32_t errorCode);
     uint32_t GetLastError();
     void MakePacket(uint8_t cmd_code, uint8_t operation, uint8_t dm_id, uint16_t data_size, uint8_t* send_data_);
     int32_t SendToDevice(uint16_t dataLen);
     void SetTimeout(uint32_t timeout);
     void SetCmdSink(IXCmdSink* cmd_sink_);
	 int32_t RecvHeartbeat(uint8_t* buffer, int32_t maxLength);
private:
     XUDPCmdEngine(const XUDPCmdEngine&);
     XUDPCmdEngine& operator = (const XUDPCmdEngine&);

     int32_t ParseRecv(uint8_t* recv_data_, uint8_t& err_code);
     static uint64_t ElapsedSince(chrono::steady_clock::time_point start);
     int32_t ReceiveFromDevice(uint8_t* recv_data_, uint8_t& err_code);
     bool _is_open;
     uint32_t _timeout;
     uint32_t _last_err;
     XFastCrc _crc_check;
     XUDPSocket _udp_sock;
     IXCmdSink* _cmd_sink_;
	 XDevice *dev;
     uint8_t _send_buf[XCMD_BUF_SIZE];
     uint8_t _recv_buf[XCMD_BUF_SIZE];
     uint8_t heartbeatBuffer[XCMD_BUF_SIZE];
     uint16_t heartbeatBufferLength;
};

#endif //XUDPCMD_ENGINE_H
