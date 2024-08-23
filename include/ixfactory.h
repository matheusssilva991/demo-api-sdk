/*
  Copyright (c) 2014, Detection Technology Inc.
  All right reserved.

  Author: Zhang Xu, 2014-4-14
 */

#ifndef IXFACTORY_H
#define IXFACTORY_H
#include "xconfigure.h"
class IXCmdEngine;
class IXImgEngine;
class IXImgParse;
class XPacketPool;
/*
  Interface of factory
 */
class XDLL_EXPORT IXFactory
{
public:
     IXFactory();
     virtual ~IXFactory();

     virtual IXCmdEngine* GetCmdEngine()=0;
     virtual IXImgEngine* GetImgEngine(bool enline_info)=0;
     virtual IXImgParse*  GetImgParse(bool enline_info)=0;
     virtual XPacketPool* GetPacketPool()=0;
};
#endif //IXFACTORY_H
