/*
  Copyright (c) 2014, Detection Technology Inc.
  All right reserved.

  Author: Zhang Xu, 2014-4-24
 */

#ifndef IXIMG_ENGINE_H
#define IXIMG_ENGINE_H
#include "xdevice.h"
#include "xconfigure.h"
#include "ixtransfer.h"

class IXImgSink;
class XCommand;
class XPacketPool;
class XAcquisition;
class XDLL_EXPORT IXImgEngine
{
public:
     IXImgEngine();
     virtual ~IXImgEngine();

     virtual bool Open(XDevice*, uint32_t recv_buf_size,uint32_t affinity_mask =0 , uint32_t RIOBufSize= RIO_RECV_BUFF_SIZE) = 0;
     virtual void Close() = 0;
     virtual bool Start() = 0;
     virtual bool Stop() = 0;
     virtual uint32_t GetLastError() = 0;
     virtual void SetTimeout(uint32_t timeout) = 0;
     virtual void SetImgSink(IXImgSink*) = 0;
     virtual void SetCmdHandle(XCommand*) = 0;
     virtual void SetPacketPool(XPacketPool*) = 0;
     virtual bool GetIsRunning() = 0;
     virtual void AttachObserver(XAcquisition*) = 0;
	 virtual void SetFrameTransfer(IXTransfer*) = 0;
};

#endif //IXIMG_ENGINE_H
