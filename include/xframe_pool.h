/*
  Copyright (c), Detection Technology Inc.
  All right reserved.

  Author: Zhang Xu, 2014-5-5
 */
#ifndef XFRAME_POOL_H
#define XFRAME_POOL_H
#include "xconfigure.h"
#include "ximage.h"
#include "ixtransfer.h"

struct XHeader;
class XMetrics;

class  XDLL_EXPORT XFramePool
{
public:
     XFramePool(IXTransfer* transfer_);
     ~XFramePool();

     static void Free(LPVOID ptr);
	 bool Initialize(XDevice* dev_, uint32_t data_offset = 0, uint32_t frame_buffer_size = XFRAME_NUM);

     void PutLine(uint8_t* line_data_, size_t line_size, uint32_t data_offset = 0);
     void Reset();
	 void Release();
     static LPVOID Allocate(uint32_t size);
     uint32_t GetImgWidth();
     uint32_t GetPixelByte();
	 XImage* GetImage(uint32_t index);
private:
 
	 XDevice* _dev_;
     IXTransfer* _transfer_;
     uint8_t** _mem_pool_;
     uint32_t _img_width;
     uint32_t _img_height;
	 uint32_t _pixel_depth;
     uint32_t _pixel_byte;
     uint32_t _line_size;
     uint32_t _total_line_num;
     uint32_t _cur_frame_num;
	 uint32_t _cur_line_num;
	 uint32_t _frame_buffer_size;
     uint32_t _frame_size;
	 uint32_t _data_offset;
	 uint32_t _pool_size;
//     uint8_t** _line_addr_;
	 uint32_t _line_addr_num;
	 uint8_t* _align_line_;
	 uint32_t _align_line_size;
     bool     _is_init; 
	 XImage* _image_;

	 bool ReInitialize();
};
#endif //XFRAME_POOL_H
