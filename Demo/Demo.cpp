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

#include "ixcmd_sink.h"
#include "iximg_sink.h"

#include "ximage_handler.h"
#include "xcorrection.h"

#ifdef _MSC_VER
#include "xthread_win.h"
#else
#include "xthread_liu.h"
#endif

#include <stdio.h>
#include <iostream>
#include <conio.h>

#ifdef _WIN64
#pragma comment(lib, "..\\lib\\x64\\XLibDllKosti.lib")
#else
#pragma comment(lib, "..\\lib\\x86\\XLibDllKosti.lib")
#endif

using namespace std;

XImageHandler ximg_handle; // XImageHandler object
XEvent frame_complete; // XEvent object

uint32_t frame_count = 0; // Number of frames to grab
uint32_t lost_frame_count = 0; // Number of lost frames

bool is_save = 0; // Flag for saving frames
uint32_t max_frame_count = 0; // Number of saved frames
string save_file_name; // Name of the saved file

//The allocated buffer size for the grabbed frames
#if defined(_WIN64)
uint32_t frame_buffer_size = 700;
#else
uint32_t frame_buffer_size = 400;
#endif

void displayMenu()
{
	printf("Please choose one option from following: \n\n");

	printf("1- Find device\n");

	printf("2- Configure device\n");

	printf("3- Open device\n");

	printf("4- Send ASII command\n");

	printf("5- Grab\n");

	printf("6- Stop\n");

	printf("7- Grab and Save\n");

	printf("8- Correct file\n");

	printf("9- Close device\n");

	printf("B- Binning mode\n");

	printf("C- Cycling test\n");

	printf("G- Gain\n");

	printf("I- Integration time\n");

	printf("q- Exit program\n\n\n");
}

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
	void OnXError(uint32_t err_id, const char* err_msg_)
	{
		printf("OnXERROR: %u, %s\n", err_id, err_msg_);
	}

	void OnXEvent(uint32_t event_id, uint32_t data)
	{
		if (XEVENT_IMG_PARSE_DATA_LOST == event_id)
		{
			lost_frame_count += data;
		}
	}

	void OnFrameReady(XImage* image_)
	{
		printf("Frame %u ready, width %u, height %d,  lost line %u\n",
			frame_count++, image_->_width, image_->_height, lost_frame_count);

		if (is_save)
		{
			ximg_handle.Write(image_);
		}
	}

	void OnFrameComplete()
	{
		printf("Grab complete.\n");

		if (is_save)
		{
			string txt_name = save_file_name.replace(save_file_name.find(".dat"), 4, ".txt");

			ximg_handle.SaveHeaderFile(txt_name.c_str());
			ximg_handle.CloseFile();

			is_save = 0;
		}

		frame_complete.Set();
	}

};

CmdSink cmd_sink;
ImgSink img_sink;

int main(int argc, char** argv)
{
	char host_ip[20];

	if (1 == argc)
	{
		printf("Please input the IP address of the device\n");
		cin >> host_ip;
		cin.ignore();
		cout << endl;
	}

	else
	{
		memcpy(host_ip, argv[1], 20);
	}

	XSystem xsystem(host_ip);
	XDevice* xdevice_ptr = NULL;

	int32_t device_count = 0;
	char device_ip[20];

	uint16_t device_cmd_port;
	uint16_t device_img_port;

	xsystem.RegisterEventSink(&cmd_sink);

	XGigFactory xfactory;

	XCommand xcommand(&xfactory);
	xcommand.RegisterEventSink(&cmd_sink);

	XFrameTransfer xtransfer;
	xtransfer.RegisterEventSink(&img_sink);

	XAcquisition xacquisition(&xfactory);

	xacquisition.RegisterEventSink(&img_sink);
	xacquisition.RegisterFrameTransfer(&xtransfer);

	XCorrection xcorrection;

	string send_str;
	string recv_str;

	displayMenu();

	int input_char;

	string offset_file;
	string gain_file;
	string img_file;

	uint64_t cmd_para = 0;

	//For cycling test
	uint32_t cycle_num = 0;
	uint32_t cycle_frames = 0;
	uint32_t cycle_interval = 0;
	uint32_t cycle_it = 0;



	do
	{
		printf("Please input your choice: \n");
		input_char = getchar();

		switch (input_char)
		{
		case '1':

			if (!xsystem.Open())
				return 0;

			device_count = xsystem.FindDevice();

			if (device_count <= 0)
			{
				printf("No device found.\n ");
				return 0;
			}

			//Get the first device
			xdevice_ptr = xsystem.GetDevice(0);

			printf("X-GCU: IP %s, Cmd Port %d, Img Port %d\n",
				xdevice_ptr->GetIP(),
				xdevice_ptr->GetCmdPort(),
				xdevice_ptr->GetImgPort());

			break;

		case '2':
			printf("Please input device IP\n");
			cin >> device_ip;

			printf("Please input device cmd port\n");
			cin >> device_cmd_port;

			printf("Please input device img port\n");
			cin >> device_img_port;

			if (xdevice_ptr)
			{
				xdevice_ptr->SetIP(device_ip);
				xdevice_ptr->SetCmdPort(device_cmd_port);
				xdevice_ptr->SetImgPort(device_img_port);
			}

			if (1 == xsystem.ConfigureDevice(xdevice_ptr))
			{
				printf("Configure device successfully, please find device again\n");
			}
			else
				printf("Fail to configure device\n");

			break;

		case '3':
			if (xcommand.Open(xdevice_ptr))
			{
				printf("Command channel open successfully\n");

				if (xacquisition.Open(xdevice_ptr, &xcommand))
				{
					printf("Image channel open successfully\n");

				}
				else
					printf("Image channel fail to open\n");
			}

			else
				printf("Command channel fail to open\n");

			break;

		case '4':
			printf("Please enter ASCII command:\n");
			cin >> send_str;

			xcommand.SendAscCmd(send_str, recv_str);
			printf("Device feedback %s\n", recv_str.c_str());

			break;

		case '5':
			printf("Start grabbing\n");

			frame_count = 0;
			lost_frame_count = 0;

			xacquisition.Grab(0);

			break;

		case '6':
			printf("Stop grabbing\n");

			xacquisition.Stop();

			break;

		case '7':
			frame_count = 0;
			lost_frame_count = 0;

			is_save = 1;

			printf("Please input the number of frames to be saved \n");
			cin >> max_frame_count;

			if (0 == max_frame_count)
			{
				printf("The number of frames cannot be 0, return to the main menu \n");
				break;
			}

			printf("Please input the file name, *.dat\n");
			cin >> save_file_name;

			if (!ximg_handle.OpenFile(save_file_name.c_str()))
			{

				printf("Fail to open image, return to the main menu \n");
				break;
			}

			xacquisition.Grab(max_frame_count);

			break;

		case '8':

			if (!xcorrection.Open(&xcommand))
			{
				printf("Fail to open correct object, return to the main menu \n");
				break;
			}

			printf("Loading defect map...\n");

			if (!xcorrection.LoadDefectsFile())
			{
				printf("Fail to load defect map, return to the main menu \n");
				xcorrection.Close();
				break;
			}

			printf("Succeed to load defect map, please input the offset image name, *.dat \n");

			cin >> offset_file;

			if (!ximg_handle.ReadFile(offset_file.c_str()))
			{
				printf("Fail to open offset data, return to the main menu \n");

				xcorrection.Close();
				break;
			}

			if (!xcorrection.CalculateOffset(ximg_handle._images_))
			{
				printf("Fail to calculate offset, return to the main menu \n");

				xcorrection.Close();
				break;
			}

			printf("Succeed to calculate offset, please input the gain image name, *.dat \n");

			cin >> gain_file;

			if (!ximg_handle.ReadFile(gain_file.c_str()))
			{
				printf("Fail to open gain data, return to the main menu \n");

				xcorrection.Close();
				break;
			}

			if (!xcorrection.CalculateGain(ximg_handle._images_, 0))
			{
				printf("Fail to calculate gain, return to the main menu \n");

				xcorrection.Close();
				break;
			}

			printf("Succeed to calculate gain, please input the image data, *.dat \n");


			cin >> img_file;

			if (!ximg_handle.ReadFile(img_file.c_str()))
			{
				printf("Fail to open image data, return to the main menu \n");

				xcorrection.Close();
				break;
			}

			if (!xcorrection.DoCorrect(ximg_handle._images_))
			{
				printf("Fail to correct the image data, return to the main menu \n");

				xcorrection.Close();
				break;
			}

			printf("Succeed to do correct, please input the name of corrected image, *.dat  \n");

			cin >> img_file;

			xcorrection.SaveCorrectedImageFile(img_file.c_str());

			xcorrection.Close();

			break;

		case '9':
			printf("Close device\n");

			xacquisition.Close();

			xcommand.Close();

			break;

		case 'B':
		case 'b':
			printf("Please input binning mode: \n 0: Original \n 1: 2x2\n");
			cin >> cmd_para;

			if (1 == xcommand.SetPara(XPARA_BINNING_MODE, cmd_para))
			{
				printf("Succeed setting binning mode\n\n");
			}
			else
			{
				printf("Fail setting binning mode\n\n");
			}

			break;

		case 'I':
		case 'i':
			printf("Please input integration time (us)\n");
			cin >> cmd_para;

			if (1 == xcommand.SetPara(XPARA_FRAME_PERIOD, cmd_para))
			{
				printf("Succeed setting integration time\n\n");
			}
			else
			{
				printf("Fail setting integration time\n\n");
			}

			break;

		case 'G':
		case 'g':
			printf("Please input gain mode:\n 1: Low full well\n 256: High full well\n");
			cin >> cmd_para;

			if (1 == xcommand.SetPara(XPARA_GAIN_RANGE, cmd_para))
			{
				printf("Succeed setting gain\n\n");
			}
			else
			{
				printf("Fail setting gain\n\n");
			}

			break;

		case 'C':
		case 'c':
			frame_count = 0;
			lost_frame_count = 0;

			printf("Please input frame count\n");
			cin >> cycle_frames;

			printf("Please input cycle count\n");
			cin >> cycle_num;

			printf("Please input cycle interval (s)\n");
			cin >> cycle_interval;

			for (cycle_it = 0; cycle_it < cycle_num; cycle_it++)
			{
				printf("\n Starting cycle %d\n", cycle_it);

				xacquisition.Grab(cycle_frames);

				frame_complete.Wait();

				XSLEEP(cycle_interval * XSLEEP_UNIT);
			}

			printf("\nAll cycles complete \n");

			break;

		default:
			break;

		}

	} while ((input_char != 'q'));

	xacquisition.Close();

	xcommand.Close();

	xsystem.Close();

	return 1;
}

