
#pragma once

#include "v4l2_camera.h"

// 派生类实现
class Ov7670 : public V4l2Camera {
public:
	Ov7670(std::string device="/dev/video0");
	~Ov7670();
};


