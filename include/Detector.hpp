#ifndef HOMEWORK2_DETECTOR_HPP
#define HOMEWORK2_DETECTOR_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/imgproc.hpp>
#include <string>

using namespace cv;
using namespace std;

// 原始图像窗口名称
#define WINDOW_RAW "Image_raw"
// 二值化图像窗口名称
#define WINDOW_BINARY "Image_binary"
// 结果图像窗口名称
#define WINDOW_RESULT "result"
// 魔方展开面窗口名称
#define WINDOW_CUBE_FACE "4x4 Cube Face"

// 魔方网格行列数（4×4）
#define CUBE_GRID 4
// 单格像素边长
#define CUBE_CELL 75
// 展开后魔方面边长
#define CUBE_SIZE (CUBE_CELL * CUBE_GRID)
// 魔方二值化阈值（反二值化）
#define CUBE_BINARY_THRESH 60
// 最大轮廓最小面积
#define CUBE_MIN_AREA 2000.0
// 外圈投票采样的格子数量
#define EDGE_SAMPLE_NUM 12

// 检测器类：封装 AprilTag 识别与魔方颜色识别
class Detector
{
public:
    // 构造函数：初始化颜色表
    // @author: homework
    Detector();

    // 析构函数
    // @author: homework
    ~Detector();

    // 用于识别 AprilTag
    // @author: homework
    void detect(Mat &frame);

    // 重载函数：用于识别魔方
    // @author: homework
    void detect(const string &video_path);

private:
    Vec3b cube_colors[6];  // 存储六种颜色的 BGR 值：红、橙、黄、绿、蓝、白

    // 按 HSV 均值返回对应颜色的 BGR 值
    // @author: homework
    Vec3b Detector_ClassifyHsv(int h, int s, int v);
};

#endif //HOMEWORK2_DETECTOR_HPP
