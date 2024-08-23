// Demo.cpp
// A C++ example of the usage of X-LIB

#include "stdafx.h"

//include X-LIB headers
#include "xsystem.h"
#include "xdevice.h"
#include "xcommand.h"
#include "xacquisition.h"
#include "xframe_transfer.h"
#include "xgig_factory.h"
#include "ximage.h"
#include "ximage_handler.h"
#include "ixcmd_sink.h"
#include "iximg_sink.h"
#include <stdio.h>
#include <iostream>
#include <conio.h>

#ifdef _WIN64
#pragma comment(lib, "..\\lib\\x64\\XLibDllKosti.lib")
#else
#pragma comment(lib, "..\\lib\\x86\\XLibDllKosti.lib")
#endif

using namespace std;

//The IP address of the host computer
const char host_ip[] = "192.168.0.104";

//The name of the image file
const char image_file_name[] = "test_image.dat";

//The number of the frames to grab
uint32_t frame_count = 2;

//The number of the grabbed frames
int received_frame_count = 0;

//The allocated buffer size for the grabbed frames
#if defined(_WIN64)
uint32_t frame_buffer_size = 700;
#else
uint32_t frame_buffer_size = 400;
#endif

//X-LIB objects
XSystem xsystem;
XCommand xcommand;
XGigFactory xfactory;
XFrameTransfer xtransfer;
XAcquisition xacquisition;
XImageHandler ximagehandler;
XDevice* xdevice = NULL;

//A class for handling commmand events
class CmdSink :public IXCmdSink
{
public:
	void OnXError(uint32_t err_id, const char* err_msg_)
	{
		printf("OnXError: %u, %s\n", err_id, err_msg_);
	}
	void OnXEvent(uint32_t event_id, XHealthPara data)
	{
		//printf("On Event ID %u data %f\n", event_id, data._dasTemperature1);
	}
};

//A class for handling image events
class ImgSink : public IXImgSink
{
public:
	void OnXError(uint32_t err_id, const char* err_msg_)
	{
		printf("On Error ID %u message %s\n", err_id, err_msg_);
	}
	void OnXEvent(uint32_t event_id, uint32_t data)
	{
		//printf("On Event ID %u data %u\n", event_id, data);
	}
	void OnFrameReady(XImage* image_)
	{
		printf("Frame %u ready\n", received_frame_count++);

		//Save the grapped frame to the image file
		ximagehandler.Write(image_);
	}

	void OnFrameComplete()
	{
		//Close the image file
		ximagehandler.CloseFile();

		printf("Grab completed.\n");
		printf("Press any key to exit.\n");
	}
};

CmdSink cmd_sink;
ImgSink img_sink;

int main()
{
	printf("Connecting a detector. Please wait.\n");

	//Initialize X-LIB
	xsystem.SetLocalIP(host_ip);
	if (!xsystem.Open())
	{
		printf("XSystem open failed.\n ");
		return 0;
	}
	xsystem.RegisterEventSink(&cmd_sink);

	xcommand.SetFactory(&xfactory);
	xcommand.RegisterEventSink(&cmd_sink);

	xtransfer.RegisterEventSink(&img_sink);

	xacquisition.SetFactory(&xfactory);
	xacquisition.RegisterEventSink(&img_sink);
	xacquisition.RegisterFrameTransfer(&xtransfer);

	//Find connected detectors
	int dev_num = xsystem.FindDevice();
	if (dev_num <= 0)
	{
		printf("No detector found.\n");
		return 0;
	}

	//Get the detector
	uint32_t device_index = 0;
	xdevice = xsystem.GetDevice(device_index);

	//Open connections to the detector
	if (xcommand.Open(xdevice))
	{
		printf("Command channel open successfully\n");

		if (xacquisition.Open(xdevice, &xcommand,
			/* xudp_win.h XUDP_RCVBUF_SIZE */ (8 * 1024 * 1024),
			frame_buffer_size))
		{
			printf("Image channel open successfully\n");
		}
		else
		{
			printf("Image channel fail to open\n");
			return 0;
		}
	}
	else
	{
		printf("Command channel fail to open.\n");
		return 0;
	}

	printf("Connected to a detector: IP %s, Cmd Port %d, Img Port %d\n",
		xdevice->GetIP(),
		xdevice->GetCmdPort(),
		xdevice->GetImgPort());

	//An example of the usage of GetPara function
	//Get an operation mode
	uint64_t operation_mode;
	xcommand.GetPara(XPARA_OPE_MODE, operation_mode, device_index);

	//An example of the usage of SetPara function
	//Set a frame period to 33ms
	xcommand.SetPara(XPARA_FRAME_PERIOD, 33000, device_index);

	//Open the image file
	ximagehandler.OpenFile(image_file_name);

	printf("Starting a grap of %u frames.\n", frame_count);

	//Start an image grab with desired amount of frames
	xacquisition.Grab(frame_count);

	_getch();
	printf("Closing the detector connection.\n");

	//Close X-LIB connections and free resources
	xacquisition.Close();
	xcommand.Close();
	xsystem.Close();

	return 1;
}

