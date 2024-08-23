/*
Copyright (c). Detection Technology Inc.
All right reserved.

Author: Marko Kosonen, 2018-2-16
*/
#ifndef XHEALTH_PARA_H
#define XHEALTH_PARA_H
#include "xconfigure.h"

class XDLL_EXPORT XHealthPara
{
public:
	XHealthPara()
		: _v24V(0)
		, _v3V3(0)
		, _v2V5(0)
		, _v1V1(0)
		, _v3V3analog(0)
		, _v3V5(0)
		, _v1V5(0)
		, _cisTemperature(0)
		, _dasTemperature1(0)
		, _dasTemperature2(0)
		, _dasTemperature3(0)
		, _dasHumidity(0)
    {};

    ~XHealthPara()
    {};

	float _v24V;
	float _v3V3;
	float _v2V5;
	float _v1V1;
	float _v3V3analog;
	float _v3V5;
	float _v1V5;
	float _cisTemperature;
	float _dasTemperature1;
	float _dasTemperature2;
	float _dasTemperature3;
	float _dasHumidity;
};

#endif //XHEALTH_PARA_H
