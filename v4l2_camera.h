
#pragma once

#include <map>
#include "com_camera.h"

// 派生类实现
class V4l2Camera : public ComCamera {
public:
	V4l2Camera(std::string device="/dev/video0");
	~V4l2Camera();

	bool IsRunning() override final;
	bool IsInitialized() override final;


	int Initialize() override final;
	int Start() override final;
	int Stop() override final;
	int Uninitialize() override final;

	int CaptureImage(int timeout_ms, std::vector<uint8_t> &image) override final;

	int QueryFmtList(std::vector<std::string> &list) override final;
	int QueryFrameSize(std::vector<Resolution> &list) override final;

private:
	std::string m_device;
	bool m_running;
	bool m_initialized;
	int m_fd;
	int m_buf_count;
	int m_buf_size;
	std::map<int, void*> m_mem;


	int V4L2_QueryFmtList(std::vector<std::string> &list);
	int V4L2_QueryFrameSize(std::vector<Resolution> &list);

	int V4L2_CapabilityCheck();
	int V4L2_FormatSetting(void);
	int V4L2_FPSSetting(void);
	int V4L2_RequestBuf(void);
	int V4L2_QueryBuf(void);
	int V4L2_QBuf(int index);
	int V4L2_StreamOn(void);
	int V4L2_DQBuf(int &index);
	int V4L2_StreamOff(void);
	int V4L2_DelBuf(void);
};

