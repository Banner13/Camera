
#include <linux/videodev2.h>
#include "ov7670.h"


using namespace VideoCamera;

Ov7670::Ov7670(string device)
{
	m_pixelformat = V4L2_PIX_FMT_SBGGR8;
	m_fps = 15;
	m_resolution = Resolution{640, 480};
}

Ov7670::~Ov7670()
{

}




