/*
  Copyright (c) 2004, Detection Technology Inc.
  All right reserved.
 */

#ifndef IXACQUISITION_H
#define IXACQUISITION_H

#include"xconfigure.h"

class XDevice;
class XCommand;
class IXImgSink;
class IXFactory;
class IXTransfer;

class XDLL_EXPORT IXAcquisition
{
public:
	IXAcquisition(){};
     virtual ~IXAcquisition(){};

	 virtual bool Open(XDevice* dev_, XCommand* cmd_handle_, uint32_t socket_buf_size = XUDP_RCVBUF_SIZE, uint32_t frame_buffer_size = XFRAME_NUM
	 , uint32_t affinitmask_= 0, uint32_t RIOBufSize = RIO_RECV_BUFF_SIZE) = 0;
     virtual void Close() = 0;
     virtual void RegisterEventSink(IXImgSink* img_sin_)= 0;
     virtual bool Grab(uint32_t frame_num) = 0;
     virtual bool Stop() = 0;
     virtual bool Snap()= 0;
     virtual void RegisterFrameTransfer(IXTransfer* transfer_)= 0;
     virtual void OnError() = 0;
     virtual void OnFrameComplete() = 0;
     virtual void EnableLineInfo(bool enable) = 0;
     virtual uint32_t GetLastError() = 0;
     virtual void SetFactory(IXFactory* factory_) = 0;
     virtual void SetTimeout(uint32_t time) = 0;
     virtual bool GetIsOpen() = 0;
     virtual bool GetIsGrabbing() = 0;
};
#endif //IXACQUISITION_H
