/*
  Copyright (c) 2014, Detection Technology Inc.
  All rights reserved.
 */
#ifndef XDEVICE_H
#define XDEVICE_H
#include "xsystem.h"

#define ROW_NUMBER                  1100
#define COLUMN_NUMBER               1450
#define PIXEL_DEPTH                 14

#define SN_BYTE  0
#define SN_LEN   32
#define IP_BYTE  32
#define MAC_BYTE 36
#define CMD_BYTE 42
#define IMG_BYTE 44
#define TYPE_BYTE 46
#define TYPE_LEN	32

#define DT1511_KIKKA2_ID		0x10001
#define DT1511_ARMI_ID			0x10002
#define DT1412_KIKKA2_ID		0x20001
#define DT1412_KOSTI_ID			0x20002
#define DT1412_ARMI_ID			0x20003
#define DT1412_ELLI_ID			0x20004
#define DT1412_ERICA_ID			0x20005
#define DT1412_NELLI_ID			0x20006
#define DT2301_KIKKA2_ID		0x30001
#define DT2301_ARMI_ID			0x30002
#define DT2301_ARMIB_ID			0x30003
#define DT1615_ARMI_ID			0x40001
#define DT1501_ARMI_ID			0x50001
#define DT2222_DAS_ID			0x60001
#define DT3030_DAS_ID			0x70001
#define DT2923_DAS_ID			0x80001

#define SN_PATTERN_LEN 5
#define PATTERN_2301 "24465"
#define PATTERN_1412 "24117"
#define PATTERN_1511 "xx"

#define TYPE_1511_KIKKA2		"1511_KIKKA2"
#define TYPE_1511_ARMI			"1511_ARMI"
#define TYPE_1412_KIKKA2		"1412_KIKKA2"
#define TYPE_1412_KOSTI			"1412_KOSTI"
#define TYPE_1412_ARMI			"1412_ARMI"
#define TYPE_1412_ELLI			"1412_ELLI"
#define TYPE_1412_ERICA			"1412_ERICA"
#define TYPE_1412_NELLI			"1412_NELLI"
#define TYPE_2301_KIKKA2		"2301_KIKKA2"
#define TYPE_2301_ARMI			"2301_ARMI"
#define TYPE_2301_ARMIB			"2301_ARMIB"
#define TYPE_1615_ARMI			"1615_ARMI"
#define TYPE_1501_ARMI			"1501_ARMI"
#define TYPE_2222_DAS			"2222_DAS"
#define TYPE_3030_DAS			"3030_DAS"
#define TYPE_2923_DAS			"2923_DAS"
#define TYPE_UNKNOWN			"UNKNOWN"

typedef struct DeviceType {
	uint64_t type;
	const char* name;
	int linesPerPacket;
	int healthVersion;
} DeviceType;

DeviceType* GetType(int ID);
DeviceType* GetType(char* type);

/*
  XDevice class wraps the basic parameters of detector.
 */
class  XDLL_EXPORT XDevice
{
public:
XDevice(XSystem* sys_)
     :_row_number(ROW_NUMBER)
	 ,_column_number(COLUMN_NUMBER)
	 ,_pixl_depth(PIXEL_DEPTH)
     {
	_system_ = sys_;
	SetDeviceType(TYPE_UNKNOWN);
     }
XDevice()
	  :_row_number(ROW_NUMBER)
	  ,_column_number(COLUMN_NUMBER)
	  ,_pixl_depth(PIXEL_DEPTH)
	  ,_system_(NULL)
     {
     }
     ~XDevice()
     {};

     const char* GetIP()
     {
	  return (const char*) _ip;
     }
     void SetIP(const char* ip_)
     {
	  memcpy(_ip, ip_, 20);
     }
     uint8_t* GetMAC()
     {
	  return _mac;
     }
     void SetMAC(uint8_t* mac_)
     {
	  memcpy(_mac, mac_, 6);
     }
     uint16_t GetCmdPort()
     {
	  return _cmd_port;
     }
     void SetCmdPort(uint16_t port)
     {
	  _cmd_port = port;
     }
     uint16_t GetImgPort()
     {
	  return _img_port;
     }
     void SetImgPort(uint16_t port)
     {
	  _img_port = port;
     }
     char* GetSerialNum()
     {
	  return _serial_num;
     }
	 void SetSerialNum(const char* serial_num, size_t len)
	 {
		 memcpy(_serial_num, serial_num, len);
	 }
	 XSystem* GetSystem()
     {
	  return _system_;
     }
     uint32_t GetPixelNumber()
     {
	  return _row_number * _column_number;
     }
     uint32_t GetPixelDepth()
     {
	  return _pixl_depth;
     }
	 void SetPixelDepth(uint32_t pixl_depth)
	 {
		 _pixl_depth = pixl_depth;
	 }
	 uint32_t GetOPMode()
     {
	  return _op_mode;
     }
	 void SetOPMode(uint32_t op_mode)
	 {
		 _op_mode = op_mode;
	 }
	 uint32_t GetBinningMode()
	 {
		 return _binning_mode;
	 }
	 void SetBinningMode(uint32_t binning_mode)
	 {
		 _binning_mode = binning_mode;
	 }
	 uint32_t GetRowNumber()
     {
	  return _row_number;
     }
	 void SetRowNumber(uint32_t row_number)
	 {
		 _row_number = row_number;
	 }
	 uint32_t GetColumnNumber()
     {
	  return _column_number;
     }
	 void SetColumnNumber(uint32_t column_number)
	 {
		 _column_number = column_number;
	 }
	 uint32_t GetGainRange()
	 {
	  return _gain_range;
	 }
	 void SetGainRange(uint32_t gain_range)
	 {
		 _gain_range = gain_range;
	 }
	 uint32_t GetRoiEnable()
	 {
		 return _roi_enable;
	 }
	 void SetRoiEnable(uint32_t roi_enable)
	 {
		 _roi_enable = roi_enable;
	 }
	 uint32_t GetRoiRowStart()
	 {
		 return _roi_row_start;
	 }
	 void SetRoiRowStart(uint32_t roi_row_start)
	 {
		 _roi_row_start = roi_row_start;
	 }
	 uint32_t GetRoiRowEnd()
	 {
		 return _roi_row_end;
	 }
	 void SetRoiRowEnd(uint32_t roi_row_end)
	 {
		 _roi_row_end = roi_row_end;
	 }
	 uint32_t GetRoiColumnStart()
	 {
		 return _roi_column_start;
	 }
	 void SetRoiColumnStart(uint32_t roi_column_start)
	 {
		 _roi_column_start = roi_column_start;
	 }
	 uint32_t GetRoiColumnEnd()
	 {
		 return _roi_column_end;
	 }
	 void SetRoiColumnEnd(uint32_t roi_column_end)
	 {
		 _roi_column_end = roi_column_end;
	 }
	 uint32_t GetFirmVer()
	 {
		 return _firm_ver;
	 }
	 void SetFirmVer(uint32_t firm_ver)
	 {
		 _firm_ver = firm_ver;
	 }
	 uint32_t GetFirmBuildVer()
	 {
		 return _firm_build_ver;
	 }
	 void SetFirmBuildVer(uint32_t firm_build_ver)
	 {
		 _firm_build_ver = firm_build_ver;
	 }
	 int GetLinesPerPacket()
	 {
		 return _type->linesPerPacket;
	 }
	 int GetHealthVersion()
	 {
		 return _type->healthVersion;
	 }
     const char* GetDeviceType()
     {
         return _type->name;
     }

	 void SetDeviceType(const char* type);

     private:
     char	_ip[20];
     char	_serial_num[SN_LEN];
	 DeviceType *_type;
     uint8_t     _mac[6];
     uint16_t    _cmd_port;
     uint16_t    _img_port;
	 XSystem*	 _system_;
     uint32_t    _pixl_depth;
     uint32_t    _row_number;
	 uint32_t    _column_number;
	 uint32_t    _op_mode;
	 uint32_t    _binning_mode;
	 uint32_t    _gain_range;
	 uint32_t    _roi_enable;
	 uint16_t    _roi_row_start;
	 uint16_t    _roi_row_end;
	 uint16_t    _roi_column_start;
	 uint16_t    _roi_column_end;
	 uint32_t	_firm_ver;
	 uint32_t	_firm_build_ver;

};
#endif //XDEVICE_H
