#include <iostream>
#include <unistd.h>

#include "ov7670.h"


using namespace std;
static int gindex = 0;

int process(const vector<uint8_t> &image)
{
	const void* buf = static_cast<const void*>(image.data());

	printf("process image size %d\n", image.size());
	char filename[64];
	snprintf(filename, sizeof(filename), "/tmp/test_%d.yuv", gindex++);
	
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
	vector<uint8_t> test(614400, 1);
	vector<string> fmt;
	vector<VideoCamera::Resolution> resolution;
	Ov7670 camera;

	camera.QueryFmtList(fmt);
	camera.QueryFrameSize(resolution);

	camera.SetImageAsyncCallback(process);

	camera.Initialize();
	camera.Start();

	sleep(10);
/*
	for (int i = 0; i < 50; i++) {
		camera.CaptureImage(1000, test);
		process(test);
	}
*/
	camera.Stop();
	camera.Uninitialize();
	
	cout << "hello, jack" << endl;
	return 0;
}
