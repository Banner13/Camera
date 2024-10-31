#include <iostream>

#include "v4l2_camera.h"

using namespace std;
static int gindex = 0;

int process(vector<uint8_t> &image)
{
	void* buf = static_cast<void*>(image.data());

	printf("process image size %d\n", image.size());
	char filename[64];
	snprintf(filename, sizeof(filename), "test_%d.yuv", gindex++);
	
	FILE *file = fopen(filename, "wb");
	if (!file) {
		perror("Failed to open file");
		return -1;
	}
	
	size_t written = fwrite(buf, 1, image.size(), file);
	if (written != image.size()) {
		perror("Failed to write data to file");
		fclose(file);
		return -1;
	}

	printf("process ok, %d\n", written);
	
	fclose(file);
	return 0;
}

int main(void)
{
	vector<uint8_t> test;
	vector<string> fmt;
	vector<VideoCamera::Resolution> resolution;
	V4l2Camera camera;

	camera.QueryFmtList(fmt);
	camera.QueryFrameSize(resolution);

	camera.Initialize();
	camera.Start();
	camera.CaptureImage(1000, test);
	printf("main test size = %d\n", test.size());
	process(test);
	camera.Stop();
	camera.Uninitialize();
	
	cout << "hello, jack" << endl;
	return 0;
}
