/*
Copyright (c) 2017, Detection Technology Inc.
All right reserved.

*/
#ifndef XCORRECTION_H
#define XCORRECTION_H
#include "xdevice.h"
#include "xanalyze.h"
#include "xcrc.h"
#include <vector>
#include "ximage_handler.h"

#define DEFECT_MAP_COUNT 4

class XImageHandler;
class XAcquisition;
class XFrameTransfer;
class XCommand;


class XDLL_EXPORT XCorrection
{
public:
    XCorrection();
    ~XCorrection();

    bool Open();
    bool Open(XCommand* cmd_handle_);
    void Close();
    void Reset();

    bool DoCorrect(XImage* image_);
    bool DoCorrect(XImage* image_, uint32_t type);
    bool DoCorrect(std::vector<XImage*>* images_);
    bool DoCorrect(std::vector<XImage*>* images_, uint32_t type);

    bool CalculateGain(XImage* image_, uint32_t target);
    bool CalculateGain(std::vector<XImage*>* images_, uint32_t target);

    bool CalculateOffset(XImage* image_);
    bool CalculateOffset(std::vector<XImage*>* images_);

    bool LoadDefectsFile();
    bool LoadDefectsFile(const char* file_, uint32_t type);
    bool SaveDefectsFile();
    bool SaveDefectsFile(const char* file_, uint32_t type);

    bool LoadGainFile(const char* file_);
    bool LoadOffsetFile(const char* file_);

    bool SetBaseline(uint16_t baseline);

    bool SaveGainFile(const char* file_);
    bool SaveOffsetFile(const char* file_);
    bool SaveCorrectedImageFile(const char* file_);

    XImage* GetCorrectedImage();
    std::vector<XImage*>* GetCorrectedImages();

private:
    XCorrection(const XCorrection&);
    XCorrection& operator = (const XCorrection&);

    uint32_t _width;
    uint32_t _height;
    uint32_t _pixel_depth;
    uint32_t _pixel_byte;
    uint32_t _pixel_count;
    uint32_t _size;

    uint16_t _baseline;

    uint8_t* _offset_data_;
    uint8_t* _bright_data_;
    uint8_t* _gain_data_;

    XImage* _cor_image_;
    std::vector<XImage*>* _cor_images_;

    std::vector<uint16_t>* _defect_rows_;
    std::vector<uint16_t>* _defect_columns_;

    uint8_t* _defect_maps_[DEFECT_MAP_COUNT];
    uint32_t _defect_map_lengths[DEFECT_MAP_COUNT];

    uint8_t* _defect_map_;
    uint16_t _defect_map_width;
    uint16_t _defect_map_height;

    XDevice* _dev_;
    XCommand* _cmd_handle_;
    XAnalyze  _analyze;
    XImageHandler _handler;
    XFastCrc _crc_check;

    void ClearOffset();
    void ClearGain();
    void ClearCorImage();
    void ClearDefectMap();
    bool SetUsedDefectMap();

    bool CalculateOffsetInit(XImage* image_);

    bool CalculateGainInit(XImage* image_);
    bool CalculateGainCalc(uint32_t target);

    bool Correction(uint32_t type);
    bool DefectCorrection();

    bool DefectRowCorrection(uint32_t row, uint32_t column);
    bool DefectColumnCorrection(uint32_t row, uint32_t column);
    bool DefectPixelCorrection(uint32_t row, uint32_t column);

    bool IsDefectPixel(uint32_t row, uint32_t column);

    bool Convert32to16(uint32_t* from_, uint8_t* to_, uint32_t count);
    bool SaveFile(const char* file_, uint8_t* data_, uint32_t pixel_depth);

    bool CreateCorImage(XImage* image_);
};
#endif //XCORRECTION_H
