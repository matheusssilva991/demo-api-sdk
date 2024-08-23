/*
Copyright (c). Detection Technology Inc.
All right reserved.

Author: Zhang Xu, 2014-5-5
*/
#ifndef XIMAGE_H
#define XIMAGE_H
#include "xconfigure.h"
#include "xdevice.h"

class XDLL_EXPORT XImage
{
public:
    XImage()
        : _height(0)
        ,_width(0)
        ,_pixel_depth(0)
        ,_size(0)
        ,_data_offset(0)
        ,_data_(NULL)
        ,_is_allc(0)
    {};

    ~XImage()
    {
        //if(_is_allc)
        //     Free();
    };
	
    inline  uint32_t GetPixelVal(uint32_t row, uint32_t col)
    {
        if(row >= _height || col >= _width)
            return 0;
        if(!_data_)
            return 0;
        uint32_t pixel_byte = 2;
        if(_pixel_depth > 16)
            pixel_byte = 4;
        //else if(_pixel_depth <= 8)
        //pixel_byte = 1;

        uint32_t line_size = _width*pixel_byte + _data_offset;
        uint32_t pixel_value = 0;
        switch(pixel_byte)
        {
        case 1:
            pixel_value = *(uint8_t*)(_data_ + line_size*row +_data_offset
                + col*pixel_byte);
            break;
        case 2:
            pixel_value = *(uint16_t*)(_data_ + line_size*row +_data_offset
                + col*pixel_byte);
            break;
        case 4:
            pixel_value = *(uint32_t*)(_data_ + line_size*row + _data_offset
                + col*pixel_byte);
            break;
        default:
            pixel_value = *(uint16_t*)(_data_ + line_size*row +_data_offset
                + col*pixel_byte);
            break;
        }

        return pixel_value;
    }
    inline void SetPixelVal(uint32_t row, uint32_t col, uint32_t pixel_value)
    {
        if(row >= _height || col >= _width)
            return;
        if(!_data_)
            return;
        uint32_t pixel_byte = 2;
        if(_pixel_depth > 16)
            pixel_byte = 4;
        // else if(_pixel_depth <= 8)
        //pixel_byte = 1;

        uint32_t line_size = _width*pixel_byte + _data_offset;

        switch(pixel_byte)
        {
        case 1:
            *(uint8_t*)(_data_ + line_size*row +_data_offset
                + col*pixel_byte) = pixel_value;
            break;
        case 2:
            *(uint16_t*)(_data_ + line_size*row +_data_offset
                + col*pixel_byte) = pixel_value;
            break;
        case 4:
            *(uint32_t*)(_data_ + line_size*row + _data_offset
                + col*pixel_byte) = pixel_value;
            break;
        default:
            *(uint16_t*)(_data_ + line_size*row +_data_offset
                + col*pixel_byte) = pixel_value;
            break;
        }
    }
    inline uint8_t* GetLineAddr(uint32_t line_num)
    {
        if(line_num >= _height)
            return NULL;
        else
        {
            uint32_t pixel_byte = 2;
            if(_pixel_depth > 16)
                pixel_byte = 4;
            //else if(_pixel_depth <= 8)
            //    pixel_byte = 1;
            uint32_t line_size = _width*pixel_byte + _data_offset;

            return (_data_ + line_size*line_num);
        }
    }

    bool Save(const char* file_name_);
    bool OpenFile(const char* file_name_);
    bool Save();
    bool CloseFile();  

    uint32_t _height;
    uint32_t _width;
    uint32_t _pixel_depth;
    size_t _size;
    uint32_t _data_offset;
    uint8_t* _data_;
	XDevice* _device_;

private:
    XImage& operator = (const XImage& image);
    XImage(const XImage& image);

    bool _is_allc;
    FILE* _file_;

    bool SaveTxt(const char* file_name_);
    bool SaveRaw(const char* file_name_);
};

#endif //XIMAGE_H
