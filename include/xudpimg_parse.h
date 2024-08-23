/*
  Copyright (c) 2017, Detection Technology Inc.
  All right reserved.

  Author: Zhang Xu, 2014-4-28
  Author: Marko Kosonen, 2017-11-22
 */
#ifndef XUDPIMG_PARSE_H
#define XUDPIMG_PARSE_H

#include "iximg_parse.h"
#include "iximg_sink.h"
#include "xpacket_pool.h"
#include "ixtransfer.h"

 //Define the position of bytes
#define CMD             2
#define FRAME_ID        3

//Image header packet
#define LINE_STAMP      5
#define FRAME_SIZE      9
#define MONITOR_STATUS  13

//Image payload packet
#define LINE_ID         5
#define PACKET_ID       7
#define PAYLOAD_SIZE    8
#define PAYLOAD         10

//Image header packet size
#define HEADER_SIZE		24
#define TWOLINE_THRESHOLD 4700

struct XHeader
{
	uint8_t  _cmd_flag;
	uint16_t _frame_id;

	//image header
	uint32_t _line_stamp;
	uint32_t _frame_size;

	//image payload
	uint16_t _line_id;
	uint8_t  _packet_id;
	uint16_t _payload_size;
	bool _isHeader;
};

struct XLineBuf
{
     size_t _cur_pos;
     uint8_t* _line_buf_;
};

/*
  XUDPImgParse getts image pakcets form memory pool and parse each.
  Then put the real image data into frame transfer.
 */
#define LINE_BUF_SIZE 360000
#define MAX_LINE_NUM  0xFFFF
#define DEVICE_1412 1
#define DEVICE_2301 2

class XUDPImgParse : public IXImgParse
{
public:
     XUDPImgParse();
     virtual ~XUDPImgParse();

     bool Open(XDevice* dev_, uint32_t affinity_mask = 0);
     void Close();
     bool Start();
     bool Stop();

     uint32_t GetLastError();
     void SetImgSink(IXImgSink*);
     void SetPacketPool(XPacketPool*);
     bool GetIsRunning();
     void Reset();
     void SetFrameTransfer(IXTransfer* transfer_);

//private:
protected:
     XUDPImgParse(const XUDPImgParse&);
     XUDPImgParse operator = (const XUDPImgParse&);
     static XTHREAD_CALL ParseThread(void* arg);
     uint32_t ParseThreadMember();
     bool Parse();
	 void FindFirstHeader();
	 void ParseFirstLine();
	 void GetInfo(XPacket* packet_);
     void GetHeader(XPacket* packet_);
     bool IsHeader();
     virtual void Allocate();
     virtual void CopyHeadInfo();
     virtual void CopyLine();
     void DataLost();
	 void FillLostPackets(uint32_t start, uint32_t end);
     void CopyData(XPacket* packet_);

     bool _is_open;
     bool _is_running;
     uint32_t _last_err;
     uint16_t _line_num;
     uint16_t _max_line_num;
     uint16_t _packet_num;
     uint16_t _max_packet_num;
	 uint16_t _frame_num;
	 uint16_t _payload_size;
	 uint16_t _last_payload_size;

     IXImgSink* _img_sink_;
     XPacketPool* _packet_pool_;
     IXTransfer* _frame_transfer_;

     XLineBuf _line_buffer;
     XThread _parse_thread;
     struct XHeader _img_header;
	 XHeader _img_headerExt;
	 XDevice* _dev_;
	 int device_type_code;
};

#endif //XUDPIMG_PARSE_H
