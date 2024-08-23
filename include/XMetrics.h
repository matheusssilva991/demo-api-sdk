#pragma once
#include "xudpimg_parse.h"
#include <vector>
#include <utility>
#include <ctime>

using namespace std;

class XDLL_EXPORT XMetrics
{
public:
	XMetrics()
	{
		FrameHeader.first = 0;
		FrameHeader.second = nullptr;
	};
	~XMetrics()
	{
		Clean();
	}

	void Clean()
	{
		FrameHeader.first = 0;
		if(FrameHeader.second != nullptr)
		{
			delete FrameHeader.second;
			FrameHeader.second = nullptr;
		}
		for (int i = 0; i < FrameLines.size(); ++i)
		{
			FrameLines[i].first = 0;
			delete FrameLines[i].second;
		}
		FrameLines.clear();
	}

	pair<time_t, XHeader*> FrameHeader;
	vector< pair <time_t, XHeader* >> FrameLines;
};

