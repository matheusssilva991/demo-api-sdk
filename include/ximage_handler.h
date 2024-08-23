/*
  Copyright (c) 2017, Detection Technology Inc.
  All right reserved.

  Author: Marko Kosonen, 2017-09-07
*/

#ifndef XIMAGE_HANDLER_H
#define XIMAGE_HANDLER_H

#include "ximage.h"
#include "xdevice.h"
#include <vector>

/*
  Class XImageHandler provides basic functions to save raw format image.
  It only supports loading the images saved by it.
 */

class XDLL_EXPORT XImageHandler
{
public:
     XImageHandler();
     ~XImageHandler();

     bool OpenFile(const char* file_);
	 bool Write(XImage* image_);
	 bool Write(std::vector<XImage*>* images_);
	 bool CloseFile();

	 bool SaveHeaderFile(const char* file_);

     bool ReadFile(const char* file_);

	 XImage* _image_;
	 std::vector<XImage*>* _images_;
     uint32_t _data_size;

private:
     XImageHandler(const XImageHandler&);
     XImageHandler& operator = (const XImageHandler&);

     bool WriteTxt(XImage* image_);
	 bool ReadHeaderFile(std::string file_, XDevice* device_, uint32_t* number);
	 bool ParseHeader(std::string key, std::string value, XDevice* device_, uint32_t* number);
	 void Close();

     uint8_t _format; // 0 = raw format; 1 = txt format
	 uint32_t _image_num;
	 std::string _image_file;
	 XDevice* _device_;

	 FILE* _file_;
};
#endif
  
