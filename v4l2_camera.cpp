

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>


#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/videodev2.h>


#include "v4l2_camera.h"

using namespace std;
using namespace VideoCamera;

#ifndef __func__
#define __func___ __FUNCTION__
#endif

#define ERR_CHECK(cmd, tip)										\
	({																	\
		int __ret = cmd;												\
		if (__ret < 0) {												\
			printf("Error:%s: %s failed! %s\n\n",__func___, tip, strerror(errno));	\
		}																\
		__ret;															\
	})

#define WARN_CHECK(cmd, tip)										\
	({																	\
		int __ret = cmd;												\
		if (__ret < 0) {												\
			printf("Warning:%s: %s unsuccessful.\n",__func___, tip);					\
		}																\
		__ret;															\
	})
	

#define CLEAN(x) memset(&x, 0, sizeof(x))

#define debug 1
#define NB_BUFFER (4)

V4l2Camera::V4l2Camera(string device)
{
	int fd;
	m_device = device;
	m_buf_count = NB_BUFFER;
	const char *cdevice = device.c_str();
	if ((fd = open(cdevice, O_RDWR)) == -1) {
		perror("Can't open file\n");
		exit(1);
	}

	m_fd = fd;
	m_running = false;
	m_initialized = false;
}

V4l2Camera::~V4l2Camera()
{
	if(close(m_fd)) {
		perror("Can't close file\n");
	}
}

bool V4l2Camera::IsRunning()
{
	return m_running;
}

bool V4l2Camera::IsInitialized()
{
	return m_initialized;
}

int V4l2Camera::V4L2_QueryFmtList(vector<string> &list)
{
	int ret = 0;
	int fd = m_fd;
	// const char *cdevice = m_device.c_str();
	vector<int> pixelformat;

	struct v4l2_fmtdesc fmtdesc;
	CLEAN(fmtdesc);

	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	printf("\n");
	printf("支持的视频格式列表:\n");
	while (!WARN_CHECK(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc), "VIDIOC_ENUM_FMT")) {
		char format_str[5];
		format_str[0] = fmtdesc.pixelformat & 0xff;
		format_str[1] = (fmtdesc.pixelformat >> 8) & 0xff;
		format_str[2] = (fmtdesc.pixelformat >> 16) & 0xff;
		format_str[3] = (fmtdesc.pixelformat >> 24) & 0xff;
		format_str[4] = '\0';

		// list.push_back(string(format_str));

		printf("Index %d: %s (FourCC: %s)\n", 	fmtdesc.index, 
												fmtdesc.description, 
												format_str);
		pixelformat.push_back(fmtdesc.pixelformat);
		fmtdesc.index++;  // 增加索引以列举下一个格式
	}

	if (!pixelformat.size())
		return -1;

	// 格式检查是否支持
	if (find(pixelformat.begin(), pixelformat.end(), m_pixelformat) == pixelformat.end())
		m_pixelformat = pixelformat.front();

	return -ret;
}

int V4l2Camera::V4L2_QueryFrameSize(vector<Resolution> &list)
{
	int ret = 0;
	int fd = m_fd;
	// const char *cdevice = m_device.c_str();
	int pixelformat = m_pixelformat;
	uint32_t max_width = 0, max_height = 0;

	struct v4l2_frmsizeenum frmsize;
	CLEAN(frmsize);
	frmsize.pixel_format = pixelformat;

	printf("支持的帧大小:\n");
	while (!WARN_CHECK(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize), "VIDIOC_ENUM_FRAMESIZES")) {
		if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
			printf("宽度 = %u, 高度 = %u\n", 
				frmsize.discrete.width, frmsize.discrete.height);
		}
		max_width = max_width > frmsize.discrete.width ? max_width : frmsize.discrete.width;
		max_height = max_height > frmsize.discrete.height ? max_height : frmsize.discrete.height;
		frmsize.index++;
	}

	if (!frmsize.index)
		return -1;

	// 分辨率检查是否支持
	m_resolution.width = m_resolution.width > max_width ? max_width : m_resolution.width;
	m_resolution.height = m_resolution.height > max_width ? max_height : m_resolution.height;

	return -ret;
}

int V4l2Camera::V4L2_CapabilityCheck()
{
	int ret = 0;
	int fd = m_fd;
	const char *cdevice = m_device.c_str();

	struct v4l2_capability cap;
	CLEAN(cap);

	if(ERR_CHECK(ioctl(fd, VIDIOC_QUERYCAP, &cap), "VIDIOC_QUERYCAP")) 
		ret |= 0x01;

	printf("\n");
	printf("v4l2_capability.driver is %s\n", cap.driver);
	printf("v4l2_capability.card is %s\n", cap.card);
	printf("v4l2_capability.bus_info is %s\n", cap.bus_info);
	printf("v4l2_capability.version is %d\n", cap.version);
	printf("v4l2_capability.capabilities is %x\n", cap.capabilities);
	printf("v4l2_capability.device_caps is %x\n", cap.device_caps);

	if (!(V4L2_CAP_VIDEO_CAPTURE & cap.capabilities)) {
		printf("Error: %s not supported V4L2_CAP_VIDEO_CAPTURE.\n", cdevice);
		ret |= 0x02;
	}
	
	if (!(V4L2_CAP_STREAMING & cap.capabilities)) {
		printf("Warning: %s does not support V4L2_CAP_STREAMING\n", cdevice);
	}
	
	if (!(V4L2_CAP_READWRITE & cap.capabilities)) {
		printf("Warning: %s does not support V4L2_CAP_READWRITE\n", cdevice);
	}

	return -ret;
}

int V4l2Camera::V4L2_FormatSetting(void)
{
	int ret = 0;
	int fd = m_fd;
	Resolution resolution = m_resolution;
	int pixelformat = m_pixelformat;

	struct v4l2_format fmt;
	CLEAN(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = resolution.width;
	fmt.fmt.pix.height = resolution.height;
	fmt.fmt.pix.pixelformat = pixelformat;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	if (ERR_CHECK(ioctl(fd, VIDIOC_S_FMT, &fmt), "VIDIOC_S_FMT"))
		ret |= 0x01;

	printf("Set format w(%d) h(%d)\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

	if ((fmt.fmt.pix.width != resolution.width) ||
		(fmt.fmt.pix.height != resolution.width)) {
		printf("format asked unavailable get width %d height %d \n",
							fmt.fmt.pix.width, fmt.fmt.pix.height);

		resolution.width= fmt.fmt.pix.width;
		resolution.height = fmt.fmt.pix.height;
	}

	return -ret;
}


int V4l2Camera::V4L2_FPSSetting(void)
{
	int ret = 0;
	int fd = m_fd;
	int fps = m_fps;

	struct v4l2_streamparm setfps;
	CLEAN(setfps);

	setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	setfps.parm.capture.timeperframe.numerator = 1;
	setfps.parm.capture.timeperframe.denominator = fps;

	if (!WARN_CHECK(ioctl(fd, VIDIOC_S_PARM, &setfps), "VIDIOC_S_PARM"))
		printf("Set fps: %d.\n", setfps.parm.capture.timeperframe.denominator);

	return -ret;
}

int V4l2Camera::V4L2_RequestBuf(void)
{
	int ret = 0;
	int fd = m_fd;
	int count = m_buf_count;

	struct v4l2_requestbuffers rb;
	CLEAN(rb);

	rb.count = count;
	rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;

	if (ERR_CHECK(ioctl(fd, VIDIOC_REQBUFS, &rb), "VIDIOC_REQBUFS"))
		ret |= 0x01;

	return -ret;
}


int V4l2Camera::V4L2_QueryBuf(void)
{
	int ret = 0;
	int fd = m_fd;
	int count = m_buf_count;
	void *ptr;

	struct v4l2_buffer buf;
	for (int i = 0; i < count; i++) {
		CLEAN(buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (ERR_CHECK(ioctl(fd, VIDIOC_QUERYBUF, &buf), "VIDIOC_QUERYBUF"))
			return -1;

		if (debug)
			printf("length: %u offset: %u\n", buf.length, buf.m.offset);

		ptr = mmap(0 , buf.length, PROT_READ, MAP_SHARED, fd, buf.m.offset);
		if (ptr == MAP_FAILED) {
			printf("Unable to map buffer! %s\n", strerror(errno));
			return -1;
		}
		if (debug)
			printf("Buffer mapped at address %p.\n", ptr);

		m_mem[i] = ptr;
	}

	m_buf_size = buf.length;

	return -ret;
}

int V4l2Camera::V4L2_QBuf(int index)
{
	int ret = 0;
	int fd = m_fd;

	struct v4l2_buffer buf;
	CLEAN(buf);

	buf.index = index;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (ERR_CHECK(ioctl(fd, VIDIOC_QBUF, &buf), "VIDIOC_QBUF"))
		return -1;
	
	printf("Queue buffer (%d).\n", buf.index);

	return -ret;
}

int V4l2Camera::V4L2_StreamOn(void)
{
	int ret = 0;
	int fd = m_fd;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (!ERR_CHECK(ioctl(fd, VIDIOC_STREAMON, &type), "VIDIOC_STREAMON"))
		return -1;

	printf("Start Capture\n");

	return -ret;
}

int V4l2Camera::V4L2_DQBuf(int &index)
{
	int ret = 0;
	int fd = m_fd;

	struct v4l2_buffer buf;
	CLEAN(buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (ERR_CHECK(ioctl(fd, VIDIOC_DQBUF, &buf), "VIDIOC_DQBUF"))
		return -1;
	
	index = buf.index;
	printf("Dq buf OK, Get %d size %d\n", buf.index, buf.bytesused);

	return -ret;
}

int V4l2Camera::V4L2_StreamOff(void)
{
	int ret = 0;
	int fd = m_fd;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (ERR_CHECK(ioctl(fd, VIDIOC_STREAMOFF, &type), "VIDIOC_STREAMOFF"))
		return -1;

	printf("Stop Capture\n");

	return -ret;
}

int V4l2Camera::V4L2_DelBuf(void)
{
	int ret = 0;
	// int fd = m_fd;
	int count = m_buf_count;
	int buf_size = m_buf_size;
	void *ptr;

	for (int i = 0; i < count; i++) {
		ptr = m_mem[i];
		if (ERR_CHECK(munmap(ptr, buf_size), "munmap"))
			return -1;
		printf("munmap %d OK\n", i);
	}

	return -ret;
}


/*
	User API
*/

int V4l2Camera::QueryFmtList(vector<string> &list)
{
	return V4L2_QueryFmtList(list);
}

int V4l2Camera::QueryFrameSize(vector<Resolution> &list)

{
	return V4L2_QueryFrameSize(list);
}


int V4l2Camera::Initialize()
{
	// int ret = 0;
	if (V4L2_CapabilityCheck())
		goto fatal;

	if (V4L2_FormatSetting())
		goto fatal;

	V4L2_FPSSetting();

	if (V4L2_RequestBuf())
		goto fatal;

	if (V4L2_QueryBuf())
		goto fatal;

	for (int i = 0; i < m_buf_count; i++)
	{
		if (V4L2_QBuf(i))
			goto fatal;
	}

	m_initialized = true;

	return 0;

fatal:
	return -1;
}

int V4l2Camera::Start()
{
	if (m_running)
		return -1;

	m_running = true;
	if (m_async_callback) {
		m_capture_thread = thread(bind(&V4l2Camera::AsyncCaptureImage, this));
		m_capture_thread.detach();
	}
		
	return V4L2_StreamOn();
}

int V4l2Camera::Stop()
{
	int ret = V4L2_StreamOff();
	m_running = false;
	return ret;
}

int V4l2Camera::Uninitialize()
{
	//std::cout << "Camera shutdown.\n";
	m_initialized = false;
	return V4L2_DelBuf();
}

int V4l2Camera::AsyncCaptureImage()
{
	void *buf;
	int ret = 0;
	int index;
	int fd = m_fd;
	int buf_size = m_buf_size;
	int nfds = fd + 1;
	fd_set readfds;
	vector<uint8_t> image_data(buf_size, 1);

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	while (m_running && m_async_callback)
	{
		image_data.clear();
		ret = select(nfds, &readfds, NULL, NULL, NULL); 
		if (ret > 0) {
			if (FD_ISSET(fd, &readfds)) {
				if(V4L2_DQBuf(index))
					goto fatal;

				buf = m_mem[index];
				
				image_data.assign(static_cast<uint8_t*>(buf), static_cast<uint8_t*>(buf) + buf_size);

				if (V4L2_QBuf(index))
					goto fatal;

				if (m_async_callback(image_data))
					goto fatal;
			}
		} else if (ret == 0) {
			// 由于 timeout 为 NULL，此分支不会被执行
		} else {
			perror("select 出错");
		}
	}
	return 0;

fatal:
	return -1;
}

int V4l2Camera::CaptureImage(int timeout_ms, vector<uint8_t> &image)
{
	int ret;
	fd_set fds;
	int fd = m_fd;
	int buf_size = m_buf_size;
	void *buf;
	struct timeval tv;
	int index;

	// 如果设置了异步处理，则不能使用该函数
	if (m_async_callback || !m_running)
		goto fatal;
	// debug info
	/*
	struct timespec copy_start;
	struct timespec copy_end;
	uint64_t tstart;
	uint64_t tend;
	*/
	tv.tv_sec = 0;
	tv.tv_usec = timeout_ms * 1000;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	ret = select(fd + 1, &fds, NULL, NULL, &tv);
	if(!FD_ISSET(fd, &fds))
		goto fatal;

	if(V4L2_DQBuf(index))
		goto fatal;

	buf = m_mem[index];

	// clock_gettime(CLOCK_MONOTONIC, &copy_start);
	image.assign(static_cast<uint8_t*>(buf), static_cast<uint8_t*>(buf) + buf_size);
	// clock_gettime(CLOCK_MONOTONIC, &copy_end);

	/*
	tstart = copy_start.tv_sec * 1000000LL + copy_start.tv_nsec / 1000LL;
	tend = copy_end.tv_sec * 1000000LL + copy_end.tv_nsec / 1000LL;
	printf("vector copy %d ,use %lld us\n", image.size(), tend-tstart);
	*/
	if (V4L2_QBuf(index))
		goto fatal;

	return ret;

fatal:
	return -1;
}

