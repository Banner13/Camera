
#pragma once

#include "abs_camera.h"

// 派生类实现
class ComCamera : public Camera {
public:
	bool SetProperty(Property property, const PropertyValue& value) override final;
	PropertyValue GetProperty(Property property) const override final;
	void SetImageAsyncCallback(CaptureCallback callback) override final {
		m_async_callback = callback;
	}

protected:
	virtual int QueryFmtList(std::vector<std::string> &list) = 0;
	virtual int QueryFrameSize(std::vector<Resolution> &list) = 0;

	CaptureCallback m_async_callback;

	// 实际使用值
	std::unordered_map<Property, PropertyValue> m_properties_;


	// 支持参数
	int m_pixelformat;
	int m_fps;
	Resolution m_resolution;

private:
	
};


