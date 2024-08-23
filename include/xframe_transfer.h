/*
  Copyright (c), Detection Technology Inc.
  All right reserved.

  Author: Zhang Xu, 2014-5-5
 */

#ifndef XFRAME_TRANSFER_H
#define XFRAME_TRANSFER_H
#include "ixtransfer.h"
#include "xconfigure.h"
#include "xframe_pool.h"
#include "xacquisition.h"

class XFrameQueue;
class XMetricsQueue;
class XMetrics;


class  XDLL_EXPORT XFrameTransfer : public IXTransfer
{
public:
     XFrameTransfer();
     ~XFrameTransfer();
     friend class XAcquisition;
     
     uint32_t GetLastError();
     void RegisterEventSink(IXImgSink* img_sink_);
     bool GetIsRunning();
	 XImage* GetImage();
	 XImage* GetImage(uint32_t index);
     uint32_t GetPixelNumber();
     uint32_t GetPixelByte();
	 uint32_t GetNumFrames();
	 void PutFrameHeader(time_t time, XHeader *header);
	 void PutFrameLines(time_t time, XHeader *header);
	 void PushMetrics();
private:
	bool Open(XDevice* dev_, uint32_t data_offset = 0, uint32_t frame_buffer_size = XFRAME_NUM, uint32_t affinity_mask = 0);
     void Close();
     bool Start(uint32_t target_num);
     bool Stop();
     void PushFrame(XImage* image_);
     void PutLine(uint8_t* data_, size_t size, uint32_t data_offset);
     void FrameReady();
     void AttachObserver(XAcquisition* acq_);
	 void Reset();
private:
     XFrameTransfer(const XFrameTransfer&);
     XFrameTransfer& operator = (const XFrameTransfer&);
     static XTHREAD_CALL TransferThread(void* arg);
     uint32_t TransferThreadMember();
	 void CleanXMetrics();

     bool _is_open;
     bool _is_running;
     uint32_t _last_err;
     IXImgSink* _img_sink_;
     XAcquisition* _acquisition_;
     XFrameQueue* _frame_que_;
	 XMetricsQueue* metrics_que;
     XLock _lock;
     XFramePool _frame_pool;
	 uint32_t _frame_buffer_size;

     XThread _transfer_thread;
     XEvent _frame_ready;
     uint32_t _target_num;
     uint32_t _multi_index;
     uint32_t _frame_num;
     XImage*  _image_;
	 XMetrics *metrics;
};
#endif //XFRAME_TRANSFER_H
