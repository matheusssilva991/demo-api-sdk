#include "ImgSink.h"
#include <iostream>

void ImgSink::OnXError(uint32_t err_id, const char* err_msg_)
{
	std::cout << "Error ID: " << err_id << " Error Message: " << err_msg_ << std::endl;
}

void ImgSink::OnXEvent(uint32_t event_id, uint32_t data)
{
	if (XEVENT_IMG_PARSE_DATA_LOST == event_id)
	{

		this->lost_frame_count += data;
	}
}

void ImgSink::OnFrameReady(XImage* image_) 
{
	std::cout << "Frame ready" << std::endl;
	std::cout << "Width: " << image_->_width << std::endl;
	std::cout << "Height: " << image_->_height << std::endl;
	std::cout << "Lost line: " << lost_frame_count << std::endl;

	if (this->is_save)
	{
		this->ximg_handle.Write(image_);
	}
}

void ImgSink::OnFrameComplete()
{
	std::cout << "Frame complete" << std::endl;

	if (is_save)
	{
		std::string txt_name = this->save_file_name.replace(this->save_file_name.find(".dat"), 4, ".txt");

		this->ximg_handle.SaveHeaderFile(txt_name.c_str());
		this->ximg_handle.CloseFile();

		this->is_save = 0;
	}

	this->frame_complete.Set();
}

uint32_t ImgSink::GetLostFrameCount() const
{
	return this->lost_frame_count;
}

uint32_t ImgSink::GetFrameCount() const 
{
	return this->frame_count;
}

std::string ImgSink::GetSaveFileName() const 
{
	return this->save_file_name;
}

bool ImgSink::GetIsSave() const 
{
	return this->is_save;
};

void ImgSink::SetSaveFileName(const std::string& save_file_name_)
{
	this->save_file_name = save_file_name_;

}

void ImgSink::SetIsSave(bool is_save_)
{
	this->is_save = is_save_;
}

void ImgSink::resetCounters() 
{
	this->lost_frame_count = 0;
	this->frame_count = 0;
}