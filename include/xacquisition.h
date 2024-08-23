/*
  Copyright (c) 2014, Detection Technology Inc.
  All right reserved.
 */

#ifndef XACQUISITION_H
#define XACQUISITION_H

#include "ixacquisition.h"
#include "xconfigure.h"
#include "xcommand.h"
#include <vector>
class IXImgEngine;
class IXImgParse;
class XDevice;
class XPacketPool;
class IXFactory;
class IXTransfer;
class IXImgSink;

#define MAX_CPU_AMOUNT 32

class XDLL_EXPORT XAcquisition : public IXAcquisition
{
public:
     explicit XAcquisition(IXFactory* factory_, uint32_t timeout = XIMG_TIMEOUT);
	 XAcquisition();
     ~XAcquisition();

	 bool Open(XDevice* dev_, XCommand* cmd_handle_, uint32_t socket_buf_size = XUDP_RCVBUF_SIZE * 2,
		 uint32_t frame_buffer_size = XFRAME_NUM, uint32_t affinity_mask = 0, uint32_t RIOBufSize = RIO_RECV_BUFF_SIZE);
     static void GetAffinityList(std::vector<uint32_t>* list, uint32_t affinity_mask);
     static uint32_t GetNextAffinity(std::vector<uint32_t>* list);
     void Close();
     void RegisterEventSink(IXImgSink* img_sink_);
     bool Grab(uint32_t frame_num);
     bool Stop();
     bool Snap();
     void RegisterFrameTransfer(IXTransfer* transfer_);
     void OnError();
     void OnFrameComplete();
     void OnFrameComplete2();
     void EnableLineInfo(bool enable);
     uint32_t GetLastError();
     void SetFactory(IXFactory* factory_);
     void SetTimeout(uint32_t time);
     bool GetIsOpen();
     bool GetIsGrabbing();
	 XCommand* GetCmdHandle();
private:
     XAcquisition(const XAcquisition&);
     XAcquisition& operator = (const XAcquisition&); 
     bool Allocate();
     void Release();
     
     bool _is_open;
     bool _is_grabbing;
     uint32_t _last_err;
     uint32_t _timeout;
     bool _enline_info;
	 bool _exframe;

     IXImgSink* _img_sink_;
     IXImgEngine* _img_engine_;
     IXImgParse*  _img_parse_;
     XPacketPool* _pac_pool_;
     IXTransfer* _frame_transfer_;
     IXFactory*   _factory_;
	 XCommand*	 _cmd_handle_;
     XEvent _frame_complete;
};

#endif //XACQUISITION_H
