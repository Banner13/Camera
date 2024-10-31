// jackZ
// need C++ 17

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace VideoCamera {
	using namespace std;

	enum class Property {
		RW_PROPERTY,
		PIXEL_FORMAT,
		RESOLUTION,
		FRAMERATE,
		EXPOSURE,
		GAIN,
		RO_PROPERTY,
		FORMAT_LIST,
	};

	// 定义分辨率结构
	struct Resolution {
		int width;
		int height;
	};

	// 使用 std::variant 来存储多种类型的属性
	using PropertyValue = variant<
									vector<string>, 
									vector<Resolution>
														>;
	
	using CaptureCallback = function<bool(const vector<uint8_t>& image)>;
}

class Camera {
public:
    using Property = VideoCamera::Property;             // 引入 Property 枚举
    using Resolution = VideoCamera::Resolution;         // 引入 Resolution 结构
    using PropertyValue = VideoCamera::PropertyValue;   // 引入 PropertyValue 类型
    using CaptureCallback = VideoCamera::CaptureCallback; // 引入 CaptureCallback 类型


	virtual ~Camera() = default;
/*
	virtual std::string getName() const = 0;

	// 新增：查询相机状态

	

*/

	virtual bool IsRunning()  = 0;
	virtual bool IsInitialized()  = 0;

	virtual int Initialize() = 0;
	virtual int Start() = 0;
	virtual int Stop() = 0;
	virtual int Uninitialize() = 0;

	// 同步读取
	virtual int CaptureImage(int timeout, std::vector<uint8_t> &image) = 0;

	// 新增：设置和获取属性的泛型接口
	virtual bool SetProperty(Property property, const PropertyValue& value) = 0;
	virtual PropertyValue GetProperty(Property property) const = 0;


	// 新增：设置捕获图像完成后的异步回调
	virtual void SetImageAsyncCallback(CaptureCallback callback) = 0;

protected:
	Camera() = default;
};





