#include "Detector.hpp"
#include <iostream>

// AprilTag 图片路径
#define TAG_IMAGE_PATH "./tag.png"
// 魔方视频路径
#define CUBE_VIDEO_PATH "./cube.mp4"
// 识别模式：1 = AprilTag，2 = 魔方颜色
#define RUN_MODE 2

// 程序入口：实例化 Detector，按 RUN_MODE 调用对应的 detect 重载
int main()
{
    Detector detector;

    if (RUN_MODE == 1)
    {
        Mat frame = imread(TAG_IMAGE_PATH);
        if (frame.empty())
        {
            cout << "Cannot read image: " << TAG_IMAGE_PATH << endl;
            return -1;
        }
        detector.detect(frame);
        waitKey(0);
    }
    else
    {
        detector.detect(CUBE_VIDEO_PATH);
        waitKey(0);
    }

    destroyAllWindows();
    return 0;
}
