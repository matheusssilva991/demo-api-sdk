/*
  Copyright (c) 2017, Detection Technology Inc.
  All right reserved.

 */

#ifndef XANALYZE_H
#define XANALYZE_H

#include "ximage.h"
#include "xconfigure.h"
#include <vector>
class XDLL_EXPORT XAnalyze
{
public:
     XAnalyze();
     ~XAnalyze();

     bool DoAnalyze(XImage* image_);
     bool DoAnalyze(std::vector<XImage*>* images_);
     bool DoAnalyze(XImage* image_, uint32_t type);

	 bool CalcAverage(uint16_t* data_, uint32_t count);
	 bool CalcMedian(uint16_t* data_, uint32_t count);

private:
     XAnalyze(const XAnalyze&);
     XAnalyze& operator = (const XAnalyze&);
	 void Close();
     uint32_t  _img_width;
     uint32_t  _img_height;
	 uint8_t*  _img_data_;

public:
     uint32_t* _avgs_;
     uint32_t _avg;
	 uint32_t _median;
};

#endif
