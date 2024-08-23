/*
  Copyright (c) 2014, Detection Technology Inc.
  All right reserved.
 */

#ifndef XCOMMAND_H
#define XCOMMAND_H
#include "ixcmd_sink.h"
#include "xconfigure.h"
#include "xhealth_para.h"

#define XPARA_CODE                  0x00000000
#define XPARA_INIT_PARA             (XPARA_CODE + 1)
#define XPARA_SAVE_PARA             (XPARA_CODE + 2)
#define XPARA_FRAME_PERIOD          (XPARA_CODE + 3)
#define XPARA_NON_INTTIME           (XPARA_CODE + 4)
#define XPARA_OPE_MODE              (XPARA_CODE + 5)
#define XPARA_GAIN_RANGE            (XPARA_CODE + 6)
#define XPARA_EN_SCAN               (XPARA_CODE + 7)
#define XPARA_BINNING_MODE          (XPARA_CODE + 8)
#define XPARA_OUTPUT_RESOLUTION     (XPARA_CODE + 9)
#define XPARA_INPUT_TRIGGER_MODE    (XPARA_CODE + 10)
#define XPARA_EN_INPUT_TRIGGER      (XPARA_CODE + 11)
#define XPARA_OUTPUT_TRIGGER_MODE   (XPARA_CODE + 12)
#define XPARA_EN_OUTPUT_TRIGGER     (XPARA_CODE + 13)
#define XPARA_PIXEL_NUMBER          (XPARA_CODE + 14)
#define XPARA_MAXMIN_FRAME_PERIOD   (XPARA_CODE + 15)
#define XPARA_DAS_FIRM_VER          (XPARA_CODE + 16)
#define XPARA_DAS_TEST_MODE         (XPARA_CODE + 17)
#define XPARA_DAS_SERIAL            (XPARA_CODE + 18)
#define XPARA_INIT1_PARA            (XPARA_CODE + 19)
#define XPARA_EN_LED                (XPARA_CODE + 20)
#define XPARA_DAS_HEALTH            (XPARA_CODE + 21)
#define XPARA_CONFIG_FIRMWARE	    (XPARA_CODE + 22)
#define XPARA_EN_ROI			    (XPARA_CODE + 23)
#define XPARA_ROI				    (XPARA_CODE + 24)
#define XPARA_READ_OUT_PERIOD	    (XPARA_CODE + 25)
#define XPARA_READ_OUT_TIME		    (XPARA_CODE + 26)
#define XPARA_GCU_WORK_TIME			(XPARA_CODE + 27)
#define XPARA_DEVICE_TYPE           (XPARA_CODE + 28)

class IXCmdEngine;
class XASCParse;
class XDevice;
class IXFactory;
/*
  XCommand class provides command channel interface. It parse the ASCII command
  and send the hex command to detector. It provides the wrapping function
  of sending command.
 */

 class XDLL_EXPORT XCommand
{
public:
     explicit XCommand(IXFactory* factory_, uint32_t timeout = XCMD_TIMEOUT);
	 XCommand();
     ~XCommand();

     void SetTimeout(uint32_t time);
	 void SetFactory(IXFactory* factory_);
	 bool Open(XDevice* dev_);
     void ParseAndSetXDeviceType(uint64_t data);
     void ParseAndSetXDeviceType();
     void Close();
     void RegisterEventSink(IXCmdSink* cmd_sink_);
     int32_t SendAscCmd(std::string asc_send, std::string& asc_recv);
	 int32_t SendAscCmd(char* asc_send_, char* asc_recv_);
     uint32_t GetLastError();
     bool GetIsOpen();
     int32_t SendCommand(uint8_t cmd_code, uint8_t operation, uint8_t dm_id,
	              uint16_t data_size, uint8_t* send_data_,
		      uint8_t* recv_data_);

     int32_t SetPara(uint32_t para, uint64_t data, uint8_t dm_id =0);
	 int32_t SetPara(uint32_t para, std::string data, uint8_t dm_id=0);

     int32_t GetPara(uint32_t para, uint64_t &data, uint8_t dm_id=0);
	 int32_t GetPara(uint32_t para, std::string &data, uint8_t dm_id=0);
	 int32_t GetPara(uint32_t para, char* recv_data_, uint8_t dm_id=0);
	 int32_t GetHealth(float &v24V, float &v3V3, float &v2V5, float &v1V1, float &v3V3analog,
		 float &v3V5, float &v1V5, float &cisTemperature, float &dasTemperature1, float &dasTemperature2,
		 float &dasTemperature3, float &dasHumidity, uint8_t dm_id);
	 int32_t GetHealth(XHealthPara &health, uint8_t dm_id);
     int32_t ExecutePara(uint32_t para, uint64_t data=0, uint8_t dm_id=0);
	 bool StartHeartbeat();
     bool StopHeartbeat();
	 void SetHeartbeatLog(const bool enable = true);
private:
     XCommand(const XCommand&);
     XCommand& operator=(const XCommand&);
     
     bool TestConnection();
     bool Allocate();
     void Release();
     int32_t GetData(const std::string asc_recv, uint64_t& return_val);
	 static XTHREAD_CALL HeartbeatThread(void* arg);
	 uint32_t HeartbeatThreadMember();
	 int32_t ParseHealth(uint8_t *data, float &v24V, float &v3V3, float &v2V5, float &v1V1, float &v3V3analog,
		 float &v3V5, float &v1V5, float &cisTemperature, float &dasTemperature1, float &dasTemperature2,
		 float &dasTemperature3, float &dasHumidity);

	 int32_t ParseHealthVersion2(uint8_t *data, float &vccvin, float &vcc3v8, float &vcc2v5, float &cmos_temp, float &temp1, float &humid, float &temp2);
	 bool _is_open;
	 bool _is_heartbeat_running;
	 bool _is_heartbeat_log;
     uint32_t _last_err;
     uint32_t _timeout;     

     IXFactory* _factory_;
     IXCmdEngine* _cmd_engine_;
     XASCParse* _asc_parse_;
     IXCmdSink* _cmd_sink_;
     XLock _lock;
	 XThread _heartbeat_thread;
	 XDevice* _dev_;
};
#endif //XCOMMAND_H
